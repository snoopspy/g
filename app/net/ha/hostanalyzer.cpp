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
				const GTreeWidgetItem* twi1 = PTreeWidgetItem(this);
				const GTreeWidgetItem* twi2 = PTreeWidgetItem(&other);
				quint64 firstTs1 = twi1->property("firstTime").toULongLong();
				quint64 firstTs2 = twi2->property("firstTime").toULongLong();
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
	pcapDevice_.setObjectName("pcapDevice_");
	hostMgr_.setObjectName("hostMgr_");
	hostWatch_.setObjectName("hostWatch_");
	hostScan_.setObjectName("hostScan_");
	hostDb_.setObjectName("hostDb_");
	arpBlock_.setObjectName("arpBlock_");
	command_.setObjectName("command_");
	screenSaver_.setObjectName("screenSaver_");
	webServer_.setObjectName("webServer_");

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

	hostDb_.hostMgr_ = &hostMgr_;

	arpBlock_.pcapDevice_ = &pcapDevice_;
	arpBlock_.hostMgr_ = &hostMgr_;
	arpBlock_.hostDb_ = &hostDb_;
	arpBlock_.defaultPolicy_ = GArpBlock::Allow;

	nodes_.append(&pcapDevice_);
	nodes_.append(&hostMgr_);
	nodes_.append(&hostWatch_);
	nodes_.append(&hostScan_);
	nodes_.append(&hostDb_);
	nodes_.append(&arpBlock_);
	nodes_.append(&webServer_);

	QObject::connect(&updateHostsTimer_, &QTimer::timeout, this, &HostAnalyzer::updateHosts);
	QObject::connect(&updateElapsedTimer_, &QTimer::timeout, this, &HostAnalyzer::updateElapsedTime);
}

HostAnalyzer::~HostAnalyzer() {
	close();
}

bool HostAnalyzer::doOpen() {
	bool res = GGraph::doOpen();
	if (!res) return false;

	itemOffset_ = hostMgr_.requestItems_.request(this, sizeof(Item));
	hostMgr_.managables_.insert(this);

	updateHostsTimer_.start(updateHostsTimeoutSec_ * 1000);
	updateElapsedTimer_.start(updateElapsedTimeoutSec_ * 1000);

	return true;
}

bool HostAnalyzer::doClose() {
	bool res = GGraph::doClose();

	updateHostsTimer_.stop();
	updateElapsedTimer_.stop();

	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		QToolButton* toolButton = dynamic_cast<QToolButton*>(treeWidget_->itemWidget(twi, ColumnAttack));
		Q_ASSERT(toolButton != nullptr);
		QObject::disconnect(toolButton, &QToolButton::toggled, this, &HostAnalyzer::toolButton_toggled);
		toolButton->setText("A");
		toolButton->setIcon(QIcon(":/img/play.png"));
		toolButton->setChecked(false);
		toolButton->setEnabled(false);
	}

	return res;
}

void HostAnalyzer::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	Item* item = getItem(hostValue);
	new (item) Item;

	item->state_ = Item::Created;
	item->treeWidgetItem_ = nullptr;
	item->hostValue_ = hostValue;

	GHostDb::Item dbItem;
	if (!hostDb_.selectHost(mac, &dbItem)) {
		qWarning() << QString("selectHost(%1) return false").arg(QString(mac));
	}
	GHostDb::Mode mode = dbItem.mode_;
	if (admitTimeoutSec_ != 0 && mode == GHostDb::Auto)
		item->blockTime_ = hostValue->firstTime_.tv_sec + admitTimeoutSec_;
	else
		item->blockTime_ = 0;

	{
		QMutexLocker ml(&itemMap_);
		itemMap_.insert(mac, item);
	}
}

void HostAnalyzer::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	Item* item = getItem(hostValue);
	item->~Item();

	{
		QMutexLocker ml(&itemMap_);
		itemMap_.remove(mac);
	}
}

void HostAnalyzer::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	Item* item = getItem(hostValue);
	item->state_ = Item::Changed;
}

void HostAnalyzer::updateWidgetItem(GTreeWidgetItem* twi) {
	GMac mac = twi->property("mac").toString();

	GHostDb::Item dbItem;
	if (!hostDb_.selectHost(mac, &dbItem)) {
		qWarning() << QString("selectHost(%1) return false").arg(QString(mac));
	}
	twi->setText(ColumnIp, QString(dbItem.ip_));
	QString defaultName = dbItem.getDefaultName();
	twi->setText(HostAnalyzer::ColumnName, defaultName);
	GHostDb::Mode mode = dbItem.mode_;

	GHostMgr::HostMap* hostMap = &hostMgr_.hostMap_;
	QMutexLocker ml(hostMap);
	GHostMgr::HostMap::iterator it = hostMap->find(mac);
	if (it == hostMap->end()) return;

	GHostMgr::HostValue* hostValue = it.value();
	Q_ASSERT(hostValue != nullptr);

	GArpBlock::Item* arpBlockItem = arpBlock_.getItem(hostValue);
	Q_ASSERT(arpBlockItem != nullptr);
	GArpBlock::Policy policy = arpBlockItem->policy_;

	QToolButton* toolButton = dynamic_cast<QToolButton*>(twi->treeWidget()->itemWidget(twi, ColumnAttack));
	Q_ASSERT(toolButton != nullptr);
	QObject::disconnect(toolButton, &QToolButton::toggled, this, &HostAnalyzer::toolButton_toggled);

	bool prevBlock = policy == GArpBlock::Block;
	bool block = false; // remove warning
	switch(mode) {
		case GHostDb::Auto :
			block = policy == GArpBlock::Block;
			break;
		case GHostDb::Allow :
			block = false;
			break;
		case GHostDb::Block :
			block = true;
			break;
	}

	if (block) {
		if (!prevBlock) {
			arpBlockItem->policy_ = GArpBlock::Block;
			arpBlock_.infect(arpBlockItem, GArpHdr::Request);
		}
		toolButton->setText("B");
		toolButton->setIcon(QIcon(":/img/pause.png"));
		toolButton->setChecked(true);
	} else {
		if (prevBlock) {
			arpBlockItem->policy_ = GArpBlock::Allow;
			arpBlock_.recover(arpBlockItem, GArpHdr::Request);
		}
		toolButton->setText("A");
		toolButton->setIcon(QIcon(":/img/play.png"));
		toolButton->setChecked(false);
	}
	toolButton->setEnabled(mode == GHostDb::Auto);
	QObject::connect(toolButton, &QToolButton::toggled, this, &HostAnalyzer::toolButton_toggled);
}

void HostAnalyzer::checkBlockTime(GHostMgr::HostValue* hostValue) {
	Item* haItem = getItem(hostValue);
	if (haItem->blockTime_ == 0) return;

	GArpBlock::Item* arpBlockItem = arpBlock_.getItem(hostValue);
	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();

	if (arpBlockItem->policy_ == GArpBlock::Allow) {
		if (now >= haItem->blockTime_) {
			arpBlockItem->policy_ = GArpBlock::Block;
			arpBlock_.infect(arpBlockItem, GArpHdr::Request);
			updateWidgetItem(haItem->treeWidgetItem_);
		}
	} else { // arpBlockItem->policy_ == GArpBlock::Block
		if (now < haItem->blockTime_) {
			arpBlockItem->policy_ = GArpBlock::Allow;
			arpBlock_.recover(arpBlockItem, GArpHdr::Request);
			updateWidgetItem(haItem->treeWidgetItem_);
		}
	}
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
	GHostMgr::HostMap* hostMap = &hostMgr_.hostMap_;
	QMutexLocker ml(hostMap);
	GHostMgr::HostMap::iterator it = hostMap->find(mac);
	if (it == hostMap->end()) {
		toolButton->setText("A");
		toolButton->setIcon(QIcon(":/img/play.png"));
		toolButton->setChecked(false);
		toolButton->setEnabled(false);
		return;
	}
	GHostMgr::HostValue* hostValue = it.value();
	Q_ASSERT(hostValue != nullptr);

	GArpBlock::Item* arpBlockItem = arpBlock_.getItem(hostValue);
	Q_ASSERT(arpBlockItem != nullptr);
	bool block = arpBlockItem->policy_ == GArpBlock::Block;
	bool nextBlock = !block;

	Item* haItem = getItem(hostValue);
	Q_ASSERT(haItem != nullptr);
	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();

	if (nextBlock) {
		arpBlockItem->policy_ = GArpBlock::Block;
		arpBlock_.infect(arpBlockItem, GArpHdr::Request);
		if (haItem->blockTime_ != 0)
			haItem->blockTime_ = now;
	} else {
		arpBlockItem->policy_ = GArpBlock::Allow;
		arpBlock_.recover(arpBlockItem, GArpHdr::Request);
		if (haItem->blockTime_ != 0)
			haItem->blockTime_ = now + extendTimeoutSec_;
	}

	updateWidgetItem(haItem->treeWidgetItem_);
}

void HostAnalyzer::updateHosts() {
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem* treeWidgetItem = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		treeWidgetItem->setProperty("shouldBeDeleted", true);
	}

	QMutexLocker ml(&hostMgr_.hostMap_);
	for (GHostMgr::HostMap::iterator it = hostMgr_.hostMap_.begin(); it != hostMgr_.hostMap_.end(); it++) {
		GMac mac = it.key();
		GHostMgr::HostValue* hostValue = it.value();
		Item* item = getItem(hostValue);

		MyTreeWidgetItem* twi = PMyTreeWidgetItem(item->treeWidgetItem_);
		if (twi == nullptr) {
			twi = new MyTreeWidgetItem(treeWidget_);
			item->treeWidgetItem_ = twi;
			twi->setProperty("mac", QString(mac));
			twi->setProperty("firstTime", quint64(hostValue->firstTime_.tv_sec));
			treeWidget_->addTopLevelItem(twi);
			QToolButton *toolButton = new QToolButton(treeWidget_);
#ifdef Q_OS_ANDROID
			int iconSize = toolButton->iconSize().height();
			iconSize = iconSize * 3 / 2;
			toolButton->setIconSize(QSize(iconSize, iconSize));
#endif // Q_OS_ANDROID
			toolButton->setProperty("mac", QString(mac));
			toolButton->setAutoRaise(true);
			toolButton->setCheckable(true);
			treeWidget_->setItemWidget(twi, ColumnAttack, toolButton);

			QObject::connect(toolButton, &QToolButton::toggled, this, &HostAnalyzer::toolButton_toggled);

			updateWidgetItem(twi);
			item->state_ = Item::NotChanged;
		}

		Q_ASSERT(twi != nullptr);
		if (item->state_ != Item::NotChanged) {
			updateWidgetItem(twi);
			item->state_ = Item::NotChanged;
		}
		twi->setProperty("shouldBeDeleted", false);
	}

	int i = 0;
	while (i < treeWidget_->topLevelItemCount()) {
		GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		if (twi->property("shouldBeDeleted").toBool()) {
			delete twi;
			continue;
		}
		i++;
	}
}

void HostAnalyzer::updateElapsedTime() {
	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();
	int count = treeWidget_->topLevelItemCount();
	GHostMgr::HostMap* hostMap = &hostMgr_.hostMap_;
	QMutexLocker ml(hostMap);
	for (int i = 0; i < count; i++) {
		GTreeWidgetItem* twi = PTreeWidgetItem(treeWidget_->topLevelItem(i));
		Q_ASSERT(twi != nullptr);
		GMac mac = twi->property("mac").toString();
		GHostMgr::HostMap::iterator it = hostMap->find(mac);
		if (it == hostMap->end()) {
			twi->setText(ColumnElapsed, "");
			continue;
		}
		GHostMgr::HostValue* hostValue = it.value();
		Q_ASSERT(hostValue != nullptr);
		qint64 firstTime = hostValue->firstTime_.tv_sec;
		qint64 elapsed = now - firstTime;

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
		twi->setText(ColumnElapsed, s);

		checkBlockTime(hostValue);
	}
}

void HostAnalyzer::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void HostAnalyzer::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}
