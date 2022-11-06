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

	arpBlock_.enabled_ = false;
	arpBlock_.pcapDevice_ = &pcapDevice_;
	arpBlock_.hostMgr_ = &hostMgr_;

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

		treeWidgetItemOffset_ = hostMgr_.requestItems_.request(&hostMgr_, sizeof(QTreeWidgetItem));
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
	QMetaObject::invokeMethod(this, [=]() {
		QTreeWidgetItem* treeWidgetItem = reinterpret_cast<QTreeWidgetItem*>(hostValue->mem(treeWidgetItemOffset_));
		qDebug() << "hostOffset_=" << treeWidgetItemOffset_ << "item=" << (void*)treeWidgetItem; // gilgil temp 2022.03.28
		new (treeWidgetItem) QTreeWidgetItem(QStringList{QString(hostValue->ip_), QString(mac)});
		treeWidget_->addTopLevelItem(treeWidgetItem);

		bool block = arpBlock_.defaultPolicy_ == GArpBlock::Block;

		QToolButton* toolButton = new QToolButton(treeWidget_);
		if (block) {
			toolButton->setText("1");
			//toolButton->setIcon(QIcon(":/img/pause.png"));
		} else {
			toolButton->setText("2");
			//toolButton->setIcon(QIcon(":/img/play.png"));
		}
		toolButton->setAutoRaise(true);
		treeWidget_->setItemWidget(treeWidgetItem, 3, toolButton);
	});
}

void HostAnalyzer::hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) {
	(void)mac;
	QMetaObject::invokeMethod(this, [=]() {
		QTreeWidgetItem* treeWidgetItem = reinterpret_cast<QTreeWidgetItem*>(hostValue->mem(treeWidgetItemOffset_));
		qDebug() << "hostOffset_=" << treeWidgetItemOffset_ << "item=" << (void*)treeWidgetItem; // gilgil temp 2022.03.28
		if (active())
			treeWidgetItem->~QTreeWidgetItem();
	});
}
