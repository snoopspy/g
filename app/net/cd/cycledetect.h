#pragma once

#include <GCycleDetect>
#include <GGraph>
#include <GPcapDevice>
#include <GPcapFile>
#include <GCommand>
#include <GTreeWidget>

struct G_EXPORT CycleDetect : GGraph {
	Q_OBJECT
	Q_PROPERTY(int updateTimeoutSec MEMBER updateTimeoutSec_)
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef pcapFile READ getPcapFile)
	Q_PROPERTY(GObjRef tcpFlowMgr READ getTcpFlowMgr)
	Q_PROPERTY(GObjRef cycleDetect READ getCycleDetect)
	Q_PROPERTY(GObjRef command READ getCommand)


public:
	int updateTimeoutSec_{1}; // 1 seconds
	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getPcapFile() { return &pcapFile_; }
	GObjRef getTcpFlowMgr() { return &tcpFlowMgr_; }
	GObjRef getCycleDetect() { return &cycleDetect_; }
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
	GCommand command_{this};

public:
	const static int ColumnSip = 0;
	const static int ColumnDip = 1;
	const static int ColumnDport = 2;

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
