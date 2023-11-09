#include "hostanalyzer.h"
#include <QToolButton>
#include <QMessageBox>

struct MyTreeWidgetItem : GTreeWidgetItem {
	GTreeWidget* treeWidget_;
	MyTreeWidgetItem(GTreeWidget* parent) : GTreeWidgetItem(parent) , treeWidget_(parent) {}

	bool operator < (const QTreeWidgetItem &other) const {
		int column = treeWidget()->sortColumn();
		switch (column) {
			case HostAnalyzer::ColumnIp:
				return uint32_t(GIp(text(HostAnalyzer::ColumnIp))) < uint32_t(GIp(other.text(HostAnalyzer::ColumnIp)));
			case HostAnalyzer::ColumnName:
				return text(1) < other.text(HostAnalyzer::ColumnName);
			case HostAnalyzer::ColumnElapsed: {
				const GTreeWidgetItem* item1 = PTreeWidgetItem(this);
				const GTreeWidgetItem* item2 = PTreeWidgetItem(&other);
				quint64 firstTs1 = item1->property("firstTs").toLongLong();
				quint64 firstTs2 = item2->property("firstTs").toLongLong();
				return firstTs1 < firstTs2;
			}
			case HostAnalyzer::ColumnAttack: {
				QTreeWidgetItem* item1 = const_cast<QTreeWidgetItem*>(dynamic_cast<const QTreeWidgetItem*>(this));
				Q_ASSERT(item1 != nullptr);
				QToolButton* toolButton1 = dynamic_cast<QToolButton*>(treeWidget_->itemWidget(item1, HostAnalyzer::ColumnAttack));

				QTreeWidgetItem* item2 = const_cast<QTreeWidgetItem*>(dynamic_cast<const QTreeWidgetItem*>(&other));
				Q_ASSERT(item2 != nullptr);
				QToolButton* toolButton2 = dynamic_cast<QToolButton*>(treeWidget_->itemWidget(item2, HostAnalyzer::ColumnAttack));

				if (toolButton1 == nullptr || toolButton2 == nullptr) return false;
				return toolButton1->text() < toolButton2->text();
			}
			default:
				qCritical() << "unreachable";
				return true;
		}
	}
};
typedef MyTreeWidgetItem *PMyTreeWidgetItem;

HostAnalyzer::HostAnalyzer(QObject* parent) : GGraph(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	hostMgr_.pcapDevice_ = &pcapDevice_;
	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &hostMgr_, &GHostMgr::manage, Qt::DirectConnection);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	pcapDevice_.readTimeout_ = -1;
	pcapDevice_.waitTimeout_ = 1000;
#endif // Q_OS_LINUX
#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
	pcapDevice_.readTimeout_ = 1000;
	pcapDevice_.waitTimeout_ = 1;
#endif

	hostWatch_.pcapDevice_ = &pcapDevice_;
	hostWatch_.hostMgr_ = &hostMgr_;

	hostScan_.pcapDevice_ = &pcapDevice_;

	arpBlock_.pcapDevice_ = &pcapDevice_;
	arpBlock_.hostMgr_ = &hostMgr_;
	arpBlock_.defaultPolicy_ = GArpBlock::Allow;

	hostDb_.hostMgr_ = &hostMgr_;
	if (!hostDb_.open()) {
		QMessageBox mb;
		QString msg = hostDb_.err->msg();
		mb.critical(nullptr, "Error", msg);
	}

	QObject::connect(&updateHostsTimer_, &QTimer::timeout, this, &HostAnalyzer::updateHosts);
	QObject::connect(&updateElapsedTimer_, &QTimer::timeout, this, &HostAnalyzer::updateElapsedTime);

	nodes_.append(&pcapDevice_);
	nodes_.append(&hostMgr_);
	nodes_.append(&hostWatch_);
	nodes_.append(&hostScan_);
	nodes_.append(&arpBlock_);
}

HostAnalyzer::~HostAnalyzer() {
	hostDb_.close();
	close();
}

bool HostAnalyzer::doOpen() {
	bool res = GGraph::doOpen();
	if (!res) return false;

	hostOffset_ = hostMgr_.requestItems_.request(this, sizeof(Item));
	hostMgr_.managables_.insert(this);

	updateHostsTimer_.start(updateHostsTimrout_);
	updateElapsedTimer_.start(updateElapsedTimeout_);
	return true;
}

bool HostAnalyzer::doClose() {
	bool res = GGraph::doClose();

	updateHostsTimer_.stop();
	updateElapsedTimer_.stop();
	return res;
}

void HostAnalyzer::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	Item* item = PItem(hostValue->mem(hostOffset_));
	new (item) Item;

	item->state_ = Item::Created;
	item->treeWidgetItem_ = nullptr;
	item->mac_ = mac;
	item->ip_ = hostValue->ip_;
	item->defaultName_ = hostDb_.getDefaultName(mac, hostValue);
	item->firstTs_ = hostValue->firstTs_;
}

void HostAnalyzer::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	(void)hostValue;
}

void HostAnalyzer::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	Item* item = PItem(hostValue->mem(hostOffset_));

	item->state_ = Item::Changed;
	item->mac_ = mac;
	item->ip_ = hostValue->ip_;
	item->defaultName_ = hostDb_.getDefaultName(mac, hostValue);
	item->firstTs_ = hostValue->firstTs_;
}

#include "hawidget.h"
void HostAnalyzer::processClosed() {
	qDebug() << "bef call close()"; // gilgil temp 2023.10.18
	HaWidget* haWidget = dynamic_cast<HaWidget*>(parent());
	Q_ASSERT(haWidget != nullptr);
	haWidget->tbStop_->click();
	qDebug() << "aft call close()"; // gilgil temp 2023.10.18
}

void HostAnalyzer::toolButton_toggled(bool checked) {
	(void)checked;
	QToolButton* toolButton = dynamic_cast<QToolButton*>(sender());
	Q_ASSERT(toolButton != nullptr);
	GMac mac = toolButton->property("mac").toString();
	{
		QMutexLocker ml(&arpBlock_.itemList_.m_);
		for (GArpBlock::Item* item: arpBlock_.itemList_) {
			if (mac == item->mac_) {
				bool block = item->policy_ == GArpBlock::Block;
				bool nextBlock = !block;
				if (nextBlock) {
					arpBlock_.infect(item, GArpHdr::Request);
					item->policy_ = GArpBlock::Block;

					toolButton->setText("B");
					toolButton->setIcon(QIcon(":/img/pause.png"));
				} else {
					arpBlock_.recover(item, GArpHdr::Request);
					item->policy_ = GArpBlock::Allow;

					toolButton->setText("A");
					toolButton->setIcon(QIcon(":/img/play.png"));
				}
				break;
			}
		}
	}
}

void HostAnalyzer::updateHosts() {
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem* treeWidgetItem = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		treeWidgetItem->setProperty("shouldBeDeleted", true);
	}

	QMutexLocker ml(&hostMgr_.hostMap_.m_);
	for (GHostMgr::HostMap::iterator it = hostMgr_.hostMap_.begin(); it != hostMgr_.hostMap_.end(); it++) {
		GMac mac = it.key();
		GHostMgr::HostValue* hostValue = it.value();
		Item* item = PItem(hostValue->mem(hostOffset_));

		MyTreeWidgetItem* treeWidgetItem = PMyTreeWidgetItem(item->treeWidgetItem_);
		if (treeWidgetItem == nullptr) {
			QObject::disconnect(treeWidget_, &QTreeWidget::itemChanged, this, &HostAnalyzer::treeWidget_itemChanged);
			treeWidgetItem = new MyTreeWidgetItem(treeWidget_);
			QObject::connect(treeWidget_, &QTreeWidget::itemChanged, this, &HostAnalyzer::treeWidget_itemChanged);
			treeWidgetItem->setFlags(treeWidgetItem->flags()  | Qt::ItemIsEditable);
			treeWidgetItem->setProperty("mac", QString(mac));
			treeWidgetItem->setProperty("firstTs", qint64(item->firstTs_.tv_sec));
			treeWidget_->addTopLevelItem(treeWidgetItem);

			QToolButton *toolButton = new QToolButton(treeWidget_);
			bool block = arpBlock_.defaultPolicy_ == GArpBlock::Block;
			toolButton->setAutoRaise(true);
			toolButton->setCheckable(true);
			toolButton->setProperty("mac", QString(mac));
			if (block) {
				toolButton->setText("B");
				toolButton->setIcon(QIcon(":/img/pause.png"));
				toolButton->setChecked(true);
			} else {
				toolButton->setText("A");
				toolButton->setIcon(QIcon(":/img/play.png"));
				toolButton->setChecked(false);
			}
			treeWidget_->setItemWidget(treeWidgetItem, ColumnAttack, toolButton);

			QObject::connect(toolButton, &QToolButton::toggled, this, &HostAnalyzer::toolButton_toggled);
			item->treeWidgetItem_ = treeWidgetItem;
		}

		Q_ASSERT(treeWidgetItem != nullptr);
		if (item->state_ != Item::NotChanged) {
			QObject::disconnect(treeWidget_, &QTreeWidget::itemChanged, this, &HostAnalyzer::treeWidget_itemChanged);
			treeWidgetItem->setText(ColumnIp, QString(item->ip_));
			treeWidgetItem->setText(ColumnName, QString(item->defaultName_));
			QObject::connect(treeWidget_, &QTreeWidget::itemChanged, this, &HostAnalyzer::treeWidget_itemChanged);
			item->state_ = Item::NotChanged;
		}
		treeWidgetItem->setProperty("shouldBeDeleted", false);
	}

	int i = 0;
	while (i < treeWidget_->topLevelItemCount()) {
		GTreeWidgetItem* treeWidgetItem = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		if (treeWidgetItem->property("shouldBeDeleted").toBool()) {
			delete treeWidgetItem;
			continue;
		}
		i++;
	}
}

void HostAnalyzer::updateElapsedTime() {
	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem* item = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		Q_ASSERT(item != nullptr);
		qint64 first = item->property("firstTs").toLongLong();
		qint64 elapsed = now - first;

		qint64 days = elapsed / 86400;
		elapsed %= 86400;
		qint64 hours = elapsed / 3600;
		elapsed %= 3600;
		qint64 minutes = elapsed / 60;
		elapsed %= 60;
		qint64 seconds = elapsed;
		QString s;
		if (days != 0) s = QString("%1d ").arg(days);
		if (hours != 0) s += QString("%1h ").arg(hours);
		if (minutes != 0) s += QString("%1m ").arg(minutes);
		s += QString("%1s").arg(seconds);
		QObject::disconnect(treeWidget_, &QTreeWidget::itemChanged, this, &HostAnalyzer::treeWidget_itemChanged);
		item->setText(ColumnElapsed, s);
		QObject::connect(treeWidget_, &QTreeWidget::itemChanged, this, &HostAnalyzer::treeWidget_itemChanged);
	}
}

void HostAnalyzer::treeWidget_itemChanged(QTreeWidgetItem *item, int column) {
	if (column != ColumnName) return;
	GTreeWidgetItem* twi = PTreeWidgetItem(item);
	GMac mac = twi->property("mac").toString();
	QString alias = item->text(column);
	hostDb_.updateAlias(mac, alias);
}

void HostAnalyzer::propLoad(QJsonObject jo) {
	qDebug() << ""; // gilgil temp 2023.11.09
	GProp::propLoad(jo);
}

void HostAnalyzer::propSave(QJsonObject& jo) {
	qDebug() << ""; // gilgil temp 2023.11.09
	GProp::propSave(jo);
}
