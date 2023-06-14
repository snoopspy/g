#include "hostanalyzer.h"
#include <QToolButton>

HostAnalyzer::HostAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	hostMgr_.pcapDevice_ = &pcapDevice_;
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

	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &hostMgr_, &GHostMgr::manage, Qt::DirectConnection);
}

HostAnalyzer::~HostAnalyzer() {
	close();
}

bool HostAnalyzer::doOpen() {
	GThreadMgr::suspendStart();

	bool ok = true;
	while (true) {
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

		if (!hostDb_.open()) {
			err = hostDb_.err;
			ok = false;
			break;
		}

		qDebug() << QObject::connect(&updateElapseTimer_, &QTimer::timeout, this, &HostAnalyzer::updateElapseTime);
		updateElapseTimer_.start(10000); // 10 seconds

		break;
	}

	if (ok)
		GThreadMgr::resumeStart();
	return ok;
}

bool HostAnalyzer::doClose() {
	arpBlock_.close();
	pcapDevice_.close();
	hostMgr_.close();
	hostWatch_.close();
	hostScan_.close();
	hostDb_.close();
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
	treeWidgetItemMap_.clear();
	updateElapseTimer_.stop();
	return true;
}

void HostAnalyzer::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	GIp ip = hostValue->ip_;
	QString defaultName = hostDb_.getDefaultName(mac, hostValue);
	struct timeval firstTs = hostValue->firstTs_;

	QMetaObject::invokeMethod(this, [this, mac, ip, defaultName, firstTs]() {
		TreeWidgetItemMap *map = &treeWidgetItemMap_;
		TreeWidgetItemMap::iterator it = map->find(mac);
		if (it == map->end()) {
			GTreeWidgetItem *item = new GTreeWidgetItem(treeWidget_);
			item->setProperty("mac", QString(mac));
			item->setProperty("firstTs", qint64(firstTs.tv_sec));

			treeWidget_->addTopLevelItem(item);

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
			treeWidget_->setItemWidget(item, 3, toolButton);

			QObject::connect(toolButton, &QToolButton::toggled, this, &HostAnalyzer::toolButton_toggled);

			it = map->insert(mac, item);
		}
		QTreeWidgetItem *item = it.value();
		item->setText(0, QString(ip));
		item->setText(1, QString(defaultName));
	}, Qt::QueuedConnection);
}

void HostAnalyzer::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)hostValue;
	if (!active()) return;

	QMetaObject::invokeMethod(this, [this, mac]() {
		TreeWidgetItemMap *map = &treeWidgetItemMap_;
		TreeWidgetItemMap::iterator it = map->find(mac);
		if (it == map->end()) {
			qWarning() << QString("can not find item(%1").arg(QString(mac));
			return;
		}
		delete it.value();
		map->erase(it);
	}, Qt::QueuedConnection);
}

void HostAnalyzer::hostChanged(GMac mac, GHostMgr::HostValue* hostValue) {
	hostCreated(mac, hostValue);
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

					toolButton->setText("1");
					toolButton->setIcon(QIcon(":/img/play.png"));
				}
				break;
			}
		}
	}
}

void HostAnalyzer::updateElapseTime() {
	qint64 now = QDateTime::currentDateTime().toSecsSinceEpoch();
	int count = treeWidget_->topLevelItemCount();
	for (int i = 0; i < count; i++) {
		QTreeWidgetItem* temp = treeWidget_->topLevelItem(i);
		GTreeWidgetItem* item = dynamic_cast<GTreeWidgetItem*>(temp);
		Q_ASSERT(item != nullptr);
		QString mac = item->property("mac").toString();
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
		item->setText(2, s);
	}
}
