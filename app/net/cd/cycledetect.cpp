#include "cycledetect.h"
#include <QToolButton>
#include <QMessageBox>

struct MyTreeWidgetItem : GTreeWidgetItem {
	GTreeWidget* treeWidget_;
	MyTreeWidgetItem(GTreeWidget* parent) : GTreeWidgetItem(parent) , treeWidget_(parent) {}

	bool operator < (const QTreeWidgetItem &other) const {
		int column = treeWidget()->sortColumn();
		switch (column) {
			case CycleDetect::ColumnSip:
				return uint32_t(GIp(text(CycleDetect::ColumnSip))) < uint32_t(GIp(other.text(CycleDetect::ColumnSip)));
			case CycleDetect::ColumnDip:
				return uint32_t(GIp(text(CycleDetect::ColumnDip))) < uint32_t(GIp(other.text(CycleDetect::ColumnSip)));
			case CycleDetect::ColumnDport:
				return text(CycleDetect::ColumnDport).toInt() < other.text(CycleDetect::ColumnDport).toInt();
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

	cycleDetect_.tcpFlowMgr_ = &tcpFlowMgr_;
	QObject::connect(&pcapDevice_, &GPcapDevice::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);
	QObject::connect(&pcapFile_, &GPcapFile::captured, &tcpFlowMgr_, &GTcpFlowMgr::manage, Qt::DirectConnection);

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

	QObject::connect(&cycleDetect_, &GCycleDetect::created, this, &CycleDetect::doCreated, Qt::BlockingQueuedConnection);
	QObject::connect(&cycleDetect_, &GCycleDetect::updated, this, &CycleDetect::doUpdated);
	QObject::connect(&cycleDetect_, &GCycleDetect::deleted, this, &CycleDetect::doDeleted);

	QObject::connect(&updateTimer_, &QTimer::timeout, this, &CycleDetect::updateCycleItem);
}

CycleDetect::~CycleDetect() {
	close();
}

bool CycleDetect::doOpen() {
	bool res = GGraph::doOpen();
	if (!res) return false;

	updateTimer_.start(updateTimeoutSec_ * 1000);

	return true;
}

bool CycleDetect::doClose() {
	bool res = GGraph::doClose();

	updateTimer_.stop();

	return res;
}

void CycleDetect::updateCycleItem() {
	qDebug() << "";
}

struct Obj {
	Obj()      { qDebug() << "Obj::Obj  " << pvoid(this); }
	~Obj()     { qDebug() << "Obj::~Obj " << pvoid(this); }
	void foo() { qDebug() << "foo       " << pvoid(this); }
};
typedef Obj *PObj;

void CycleDetect::doCreated(GCycleItemKey key, GCycleItem* item) {
	(void)key; (void)item;
	Obj* obj = new Obj;
	item->user_ = obj;
}

void CycleDetect::doUpdated(GCycleItemKey key, GCycleItem* item) {
	(void)key; (void)item;
	Obj* obj = PObj(item->user_);
	obj->foo();
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
