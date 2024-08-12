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
			{
				QString leftStr = text(column);
				int left = leftStr.isEmpty() ? -1 : leftStr.toInt();
				QString rightStr = other.text(column);
				int right = rightStr.isEmpty() ? -1 : rightStr.toInt();
				return left < right;
			}
			case CycleDetect::ColumnFirstTimeAvg:
			case CycleDetect::ColumnFirstTimeAvgDiff:
			case CycleDetect::ColumnLastTimeAvg:
			case CycleDetect::ColumnLastTimeAvgDiff:
			case CycleDetect::ColumnTxPacketsAvg:
			case CycleDetect::ColumnTxPacketsAvgDiff:
			case CycleDetect::ColumnTxBytesAvg:
			case CycleDetect::ColumnTxBytesAvgDiff:
			case CycleDetect::ColumnRxPacketsAvg:
			case CycleDetect::ColumnRxPacketsAvgDiff:
			case CycleDetect::ColumnRxBytesAvg :
			case CycleDetect::ColumnRxBytesAvgDiff:
			{
				QString leftStr = text(column);
				double left = leftStr.isEmpty() ? -1 : leftStr.toDouble();
				QString rightStr = other.text(column);
				double right = rightStr.isEmpty() ? -1 : rightStr.toDouble();
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

	pcapDevice_.filter_ = "tcp";
	pcapFile_.enabled_ = false;

	cycleDetect_.tcpFlowMgr_ = &tcpFlowMgr_;

	QObject::connect(&pcapDevice_, SIGNAL(captured(GPacket*)), &pcapFileWrite_, SLOT(write(GPacket*)), Qt::DirectConnection);
	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&pcapFile_, &GPcapFile::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);

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
	twi->setFlags(twi->flags() | Qt::ItemIsEditable);
	twi->setText(ColumnClientIp, QString(key.clientIp_));
	twi->setText(ColumnServerIp, QString(key.serverIp_));
	twi->setText(ColumnServerPort, QString(QString::number(key.serverPort_)));
	twi->setText(ColumnTtl, QString(QString::number(key.ttl_)));
	twi->setText(ColumnCount, QString::number(item->firstTimes_.count()));

	if (item->firstTimes_.avg_ != -1)
		twi->setText(ColumnFirstTimeAvg, QString::number(item->firstTimes_.avg_));
	if (item->firstTimes_.avgDiff_ != -1)
		twi->setText(ColumnFirstTimeAvgDiff, QString::number(item->firstTimes_.avgDiff_));

	treeWidget_->addTopLevelItem(twi);
	item->user_ = twi;
}

void CycleDetect::doUpdated(GCycleItemKey key, GCycleItem* item) {
	(void)key;
	MyTreeWidgetItem* twi = PMyTreeWidgetItem(item->user_);
	Q_ASSERT(twi != nullptr);
	twi->setText(ColumnCount, QString::number(item->firstTimes_.count()));

	if (item->firstTimes_.avg_ != -1)
		twi->setText(ColumnFirstTimeAvg, QString::number(item->firstTimes_.avg_));
	if (item->firstTimes_.avgDiff_ != -1)
		twi->setText(ColumnFirstTimeAvgDiff, QString::number(item->firstTimes_.avgDiff_));

	if (item->lastTimes_.avg_ != -1)
		twi->setText(ColumnLastTimeAvg, QString::number(item->lastTimes_.avg_));
	if (item->lastTimes_.avgDiff_ != -1)
		twi->setText(ColumnLastTimeAvgDiff, QString::number(item->lastTimes_.avgDiff_));

	if (item->txPackets_.avg_ != -1)
		twi->setText(ColumnTxPacketsAvg, QString::number(item->txPackets_.avg_));
	if (item->txPackets_.avgDiff_ != -1)
		twi->setText(ColumnTxPacketsAvgDiff, QString::number(item->txPackets_.avgDiff_));

	if (item->txBytes_.avg_ != -1)
		twi->setText(ColumnTxBytesAvg, QString::number(item->txBytes_.avg_));
	if (item->txBytes_.avgDiff_ != -1)
		twi->setText(ColumnTxBytesAvgDiff, QString::number(item->txBytes_.avgDiff_));

	if (item->rxPackets_.avg_ != -1)
		twi->setText(ColumnRxPacketsAvg, QString::number(item->rxPackets_.avg_));
	if (item->rxPackets_.avgDiff_ != -1)
		twi->setText(ColumnRxPacketsAvgDiff, QString::number(item->rxPackets_.avgDiff_));

	if (item->rxBytes_.avg_ != -1)
		twi->setText(ColumnRxBytesAvg, QString::number(item->rxBytes_.avg_));
	if (item->rxBytes_.avgDiff_ != -1)
		twi->setText(ColumnRxBytesAvgDiff, QString::number(item->rxBytes_.avgDiff_));
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
