#include "cycledetect.h"
#include <QToolButton>
#include <QMessageBox>

struct MyTreeWidgetItem : GTreeWidgetItem {
	GTreeWidget* treeWidget_;
	MyTreeWidgetItem(GTreeWidget* parent) : GTreeWidgetItem(parent) , treeWidget_(parent) {}

	bool operator < (const QTreeWidgetItem &other) const {
		int column = treeWidget()->sortColumn();
		switch (column) {
			case CycleDetect::ColumnClientIp:
			case CycleDetect::ColumnServerIp:
				return uint32_t(GIp(text(column))) < uint32_t(GIp(other.text(column)));
			case CycleDetect::ColumnServerPort:
			case CycleDetect::ColumnTtl:
			case CycleDetect::ColumnCount:
			case CycleDetect::ColumnFirstTime:
			case CycleDetect::ColumnLastTime:
			case CycleDetect::ColumnTxPackets:
			case CycleDetect::ColumnTxBytes:
			case CycleDetect::ColumnRxPackets:
			case CycleDetect::ColumnRxBytes: {
				QString leftStr = text(column);
				int left = leftStr.isEmpty() ? -1 : leftStr.toInt();
				QString rightStr = other.text(column);
				int right = rightStr.isEmpty() ? -1 : rightStr.toInt();
				return left < right;
			}
			default:
				qCritical() << "unreachable";
				return true;
		}
	}
};
typedef MyTreeWidgetItem *PMyTreeWidgetItem;

CycleDetect::CycleDetect(QObject* parent) : GGraph(parent) {
	pcapDevice_.setObjectName("pcapDevice_");
	pcapFile_.setObjectName("pcapFile");
	tcpFlowMgr_.setObjectName("tcpFlowMgr_");
	cycleDetect_.setObjectName("cycleDetect");
	command_.setObjectName("command_");

	pcapFile_.enabled_ = false;
	tcpFlowMgr_.rstTimeout_ = 1;
	tcpFlowMgr_.finTimeout_ = 1;

	cycleDetect_.tcpFlowMgr_ = &tcpFlowMgr_;

	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&pcapFile_, &GPcapFile::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&pcapDevice_, SIGNAL(captured(GPacket*)), &pcapFileWrite_, SLOT(write(GPacket*)), Qt::DirectConnection);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
	pcapDevice_.readTimeout_ = -1;
	pcapDevice_.waitTimeout_ = 1000;
#endif // Q_OS_LINUX
#if defined(Q_OS_WIN) || defined(Q_OS_ANDROID)
	pcapDevice_.readTimeout_ = 1000;
	pcapDevice_.waitTimeout_ = 1;
#endif

	nodes_.append(&pcapDevice_);
	nodes_.append(&pcapFile_);
	nodes_.append(&tcpFlowMgr_);
	nodes_.append(&cycleDetect_);
	nodes_.append(&pcapFileWrite_);

	QObject::connect(&updateTimer_, &QTimer::timeout, this, &CycleDetect::updateCycleItem);
}

CycleDetect::~CycleDetect() {
	close();
}

bool CycleDetect::doOpen() {
	QObject::connect(&cycleDetect_, &GCycleDetect::created, this, &CycleDetect::doCreated, Qt::BlockingQueuedConnection);
	QObject::connect(&cycleDetect_, &GCycleDetect::updated, this, &CycleDetect::doUpdated, Qt::BlockingQueuedConnection);
	QObject::connect(&cycleDetect_, &GCycleDetect::deleted, this, &CycleDetect::doDeleted, Qt::BlockingQueuedConnection);

	bool res = GGraph::doOpen();
	if (!res) return false;

	// updateTimer_.start(updateTimeoutSec_ * 1000); // gilgil temp

	return true;
}

bool CycleDetect::doClose() {
	QObject::disconnect(&cycleDetect_, &GCycleDetect::created, this, &CycleDetect::doCreated);
	QObject::disconnect(&cycleDetect_, &GCycleDetect::updated, this, &CycleDetect::doUpdated);
	QObject::disconnect(&cycleDetect_, &GCycleDetect::deleted, this, &CycleDetect::doDeleted);

	QObject::connect(&cycleDetect_, &GCycleDetect::created, this, &CycleDetect::doCreated, Qt::DirectConnection);
	QObject::connect(&cycleDetect_, &GCycleDetect::updated, this, &CycleDetect::doUpdated, Qt::DirectConnection);
	QObject::connect(&cycleDetect_, &GCycleDetect::deleted, this, &CycleDetect::doDeleted, Qt::DirectConnection);

	bool res = GGraph::doClose();

	QObject::disconnect(&cycleDetect_, &GCycleDetect::created, this, &CycleDetect::doCreated);
	QObject::disconnect(&cycleDetect_, &GCycleDetect::updated, this, &CycleDetect::doUpdated);
	QObject::disconnect(&cycleDetect_, &GCycleDetect::deleted, this, &CycleDetect::doDeleted);

	updateTimer_.stop();

	return res;
}

void CycleDetect::updateCycleItem() {
	// qDebug() << ""; // gilgil temp 2024.08.10
}

void CycleDetect::doCreated(GCycleItemKey key, GCycleItem* item) {
	MyTreeWidgetItem* twi = new MyTreeWidgetItem(treeWidget_);
	twi->setText(ColumnClientIp, QString(key.clientIp_));
	twi->setText(ColumnServerIp, QString(key.serverIp_));
	twi->setText(ColumnServerPort, QString(QString::number(key.serverPort_)));
	twi->setText(ColumnTtl, QString(QString::number(key.ttl_)));
	twi->setText(ColumnCount, QString::number(item->firstTimes_.count()));
	if (item->firstTimes_.diffAvg_ != -1)
		twi->setText(ColumnFirstTime, QString::number(item->firstTimes_.diffAvg_));
	treeWidget_->addTopLevelItem(twi);
	item->user_ = twi;
}

void CycleDetect::doUpdated(GCycleItemKey key, GCycleItem* item) {
	(void)key;
	MyTreeWidgetItem* twi = PMyTreeWidgetItem(item->user_);
	Q_ASSERT(twi != nullptr);
	twi->setText(ColumnCount, QString::number(item->firstTimes_.count()));
	if (item->firstTimes_.diffAvg_ != -1)
		twi->setText(ColumnFirstTime, QString::number(item->firstTimes_.diffAvg_));
	if (item->lastTimes_.diffAvg_ != -1)
		twi->setText(ColumnLastTime, QString::number(item->lastTimes_.diffAvg_));
	if (item->txPackets_.diffAvg_ != -1)
		twi->setText(ColumnTxPackets, QString::number(item->txPackets_.diffAvg_));
	if (item->txBytes_.diffAvg_ != -1)
		twi->setText(ColumnTxBytes, QString::number(item->txBytes_.diffAvg_));
	if (item->rxPackets_.diffAvg_ != -1)
		twi->setText(ColumnRxPackets, QString::number(item->rxPackets_.diffAvg_));
	if (item->rxBytes_.diffAvg_ != -1)
		twi->setText(ColumnRxBytes, QString::number(item->rxBytes_.diffAvg_));
}

void CycleDetect::doDeleted(GCycleItemKey key, GCycleItem* item) {
	(void)key; (void)item;
}

void CycleDetect::propLoad(QJsonObject jo) {
	GProp::propLoad(jo);
}

void CycleDetect::propSave(QJsonObject& jo) {
	GProp::propSave(jo);
}
