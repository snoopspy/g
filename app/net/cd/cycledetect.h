#pragma once

#include <GCycleDetect>
#include <GGraph>
#include <GPcapDevice>
#include <GPcapFile>
#include <GPcapFileWrite>
#include <GCommand>
#include <GTreeWidget>

struct G_EXPORT CycleDetect : GGraph {
	Q_OBJECT
	Q_PROPERTY(int updateTimeoutSec MEMBER updateTimeoutSec_)
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef pcapFile READ getPcapFile)
	Q_PROPERTY(GObjRef tcpFlowMgr READ getTcpFlowMgr)
	Q_PROPERTY(GObjRef cycleDetect READ getCycleDetect)
	Q_PROPERTY(GObjRef pcapFileWrite READ getPcapFileWrite)
	Q_PROPERTY(GObjRef command READ getCommand)


public:
	int updateTimeoutSec_{1}; // 1 seconds
	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getPcapFile() { return &pcapFile_; }
	GObjRef getTcpFlowMgr() { return &tcpFlowMgr_; }
	GObjRef getCycleDetect() { return &cycleDetect_; }
	GObjRef getPcapFileWrite() { return &pcapFileWrite_; }
	GObjRef getCommand() { return &command_; }

public:
	Q_INVOKABLE CycleDetect(QObject* parent = nullptr);
	~CycleDetect() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPcapDevice pcapDevice_{this};
	GPcapFile pcapFile_{this};
	GTcpFlowMgr tcpFlowMgr_{this};
	GCycleDetect cycleDetect_{this};
	GPcapFileWrite pcapFileWrite_{this};
	GCommand command_{this};

public:
	const static int ColumnClientIp = 0;
	const static int ColumnServerIp = 1;
	const static int ColumnServerPort = 2;
	const static int ColumnTtl = 3;
	const static int ColumnCount = 4;
	const static int ColumnFirstTimeAvg = 5;
	const static int ColumnFirstTimeAvgDiff = 6;
	const static int ColumnLastTimeAvg = 7;
	const static int ColumnLastTimeAvgDiff = 8;
	const static int ColumnTxPacketsAvg = 9;
	const static int ColumnTxPacketsAvgDiff = 10;
	const static int ColumnTxBytesAvg = 11;
	const static int ColumnTxBytesAvgDiff = 12;
	const static int ColumnRxPacketsAvg = 13;
	const static int ColumnRxPacketsAvgDiff = 14;
	const static int ColumnRxBytesAvg = 15;
	const static int ColumnRxBytesAvgDiff = 16;

	QTimer updateTimer_;

public:
	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		enum State {
			Created,
			Changed,
			NotChanged
		} state_{Created};
		GTreeWidgetItem* treeWidgetItem_{nullptr};
		GCycleItem* citem_{nullptr};
		time_t blockTime_{0};
	};
	typedef Item *PItem;

	struct ItemMap : QMap<GMac, Item*>, QRecursiveMutex {
	} itemMap_;
	// --------------------------------------------------------------------------

public:
	GTreeWidget* treeWidget_{nullptr}; // reference

public slots:
	void updateCycleItem();

public slots:
	void doCreated(GCycleItemKey key, GCycleItem* item);
	void doUpdated(GCycleItemKey key, GCycleItem* item);
	void doDeleted(GCycleItemKey key, GCycleItem* item);

 public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
