#include "hostanalyzer.h"
#include <QToolButton>

HostAnalyzer::HostAnalyzer(QObject* parent) : GStateObj(parent) {
#ifdef Q_OS_ANDROID
	command_.openCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
	command_.closeCommands_.push_back(new GCommandItem(this, QStringList{"su -c \"nexutil -m0\""}));
#endif

	hostMgr_.pcapDevice_ = &pcapDevice_;

	hostWatch_.pcapDevice_ = &pcapDevice_;
	hostWatch_.hostMgr_ = &hostMgr_;

	hostScan_.pcapDevice_ = &pcapDevice_;

	arpBlock_.pcapDevice_ = &pcapDevice_;
	arpBlock_.hostMgr_ = &hostMgr_;
	arpBlock_.defaultPolicy_ = GArpBlock::Allow;

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

		itemOffset_ = hostMgr_.requestItems_.request(this, sizeof(QTreeWidgetItem**));
		hostMgr_.managables_.insert(this);

		break;
	}

	GThreadMgr::resumeStart();
	return ok;
}

bool HostAnalyzer::doClose() {
	arpBlock_.close();
	pcapDevice_.close();
	hostMgr_.close();
	hostWatch_.close();
	hostScan_.close();
	return true;
}

void HostAnalyzer::hostCreated(GMac mac, GHostMgr::HostValue* hostValue) {
	GIp ip = hostValue->ip_;
	QTreeWidgetItem** item = reinterpret_cast<QTreeWidgetItem**>(hostValue->mem(itemOffset_));
	QMetaObject::invokeMethod(this, [this, mac, ip, item]() {
		QTreeWidgetItem* treeWidgetItem = new QTreeWidgetItem(QStringList{QString(ip), QString(mac)});
		treeWidget_->addTopLevelItem(treeWidgetItem);

		QToolButton* toolButton = new QToolButton(treeWidget_);
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

		*item = treeWidgetItem;
	});
}

void HostAnalyzer::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	QTreeWidgetItem** item = reinterpret_cast<QTreeWidgetItem**>(hostValue->mem(itemOffset_));
	QMetaObject::invokeMethod(this, [this, item]() {
		if (active())
			(*item)->~QTreeWidgetItem();
	});
}

void HostAnalyzer::toolButton_toggled(bool checked) {
	(void)checked;
	QToolButton* toolButton = dynamic_cast<QToolButton*>(sender());
	GMac mac = toolButton->property("mac").toString();
	{
		QMutexLocker ml(&arpBlock_.itemList_.m_);
		for (GArpBlock::Item* item: arpBlock_.itemList_) {
			if (mac == item->mac_) {
				qDebug() << "matched"; // gilgil temp
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
