#include "hostanalyzer.h"
#include <QToolButton>
#include <QMessageBox>

struct MyTreeWidgetItem : GTreeWidgetItem {
	MyTreeWidgetItem(GTreeWidget* parent) : GTreeWidgetItem(parent) {}

	bool operator < (const QTreeWidgetItem &other) const {
		int column = treeWidget()->sortColumn();
		switch (column) {
			case HostAnalyzer::ColumnIp:
				return uint32_t(GIp(text(HostAnalyzer::ColumnIp))) < uint32_t(GIp(other.text(HostAnalyzer::ColumnIp)));
			case HostAnalyzer::ColumnName:
				return text(1) < other.text(HostAnalyzer::ColumnName);
			case HostAnalyzer::ColumnElapsed: {
				const GTreeWidgetItem* myItem = PTreeWidgetItem(this);
				const GTreeWidgetItem* otherItem = PTreeWidgetItem(&other);
				quint64 myFirstTs = myItem->property("firstTs").toLongLong();
				quint64 otherFirstTs = otherItem->property("firstTs").toLongLong();
				return myFirstTs < otherFirstTs;
			}
			default:
				qCritical() << "unreachable";
				return true;
		}
	}
};
typedef MyTreeWidgetItem *PMyTreeWidgetItem;

HostAnalyzer::HostAnalyzer(QObject* parent) : GStateObj(parent) {
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
}

HostAnalyzer::~HostAnalyzer() {
	hostDb_.close();
	close();
}

bool HostAnalyzer::doOpen() {
	GThreadMgr::suspendStart();

	bool ok = true;
	while (true) {
		for (QObject* obj: children()) {
			GStateObj* stateObj = dynamic_cast<GStateObj*>(obj);
			if (stateObj != nullptr) {
				QObject::connect(stateObj, &GStateObj::closed, this, &HostAnalyzer::processClosed);
			}
		}

		if (!pcapDevice_.open()) {
			err = pcapDevice_.err;
			ok = false;
			break;
		}

		if (!hostMgr_.open()) {
			err = hostMgr_.err;
			ok = false;
			break;
		}
		hostOffset_ = hostMgr_.requestItems_.request(this, sizeof(Item));
		hostMgr_.managables_.insert(this);

		if (!hostWatch_.open()) {
			err = hostWatch_.err;
			ok = false;
			break;
		}

		if (!hostScan_.open()) {
			err = hostScan_.err;
			ok = false;
			break;
		}

		if (!arpBlock_.open()) {
			err = arpBlock_.err;
			ok = false;
			break;
		}

		updateHostsTimer_.start(1000); // 1 seconds
		updateElapsedTimer_.start(10000); // 10 seconds

		break;
	}

	if (ok)
		GThreadMgr::resumeStart();
	return ok;
}

bool HostAnalyzer::doClose() {
	for (QObject* obj: children()) {
		GStateObj* stateObj = dynamic_cast<GStateObj*>(obj);
		if (stateObj != nullptr) {
			QObject::disconnect(stateObj, &GStateObj::closed, this, &HostAnalyzer::processClosed);
		}
	}

	arpBlock_.close();
	pcapDevice_.close();
	hostMgr_.close();
	hostWatch_.close();
	hostScan_.close();
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		QTreeWidgetItem* item = treeWidget_->topLevelItem(i);
		QToolButton* toolButton = dynamic_cast<QToolButton*>(treeWidget_->itemWidget(item, 3));
		if (toolButton != nullptr) {
			toolButton->setText("0");
			toolButton->setIcon(QIcon(":/img/play.png"));
			toolButton->setChecked(false);
			toolButton->setEnabled(false);
		}
	}
	updateHostsTimer_.stop();
	updateElapsedTimer_.stop();
	return true;
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

					toolButton->setText("1");
					toolButton->setIcon(QIcon(":/img/pause.png"));
				} else {
					arpBlock_.recover(item, GArpHdr::Request);
					item->policy_ = GArpBlock::Allow;

					toolButton->setText("0");
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
				toolButton->setText("1");
				toolButton->setIcon(QIcon(":/img/pause.png"));
				toolButton->setChecked(true);
			} else {
				toolButton->setText("0");
				toolButton->setIcon(QIcon(":/img/play.png"));
				toolButton->setChecked(false);
			}
			treeWidget_->setItemWidget(treeWidgetItem, 3, toolButton);

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
