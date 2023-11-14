#pragma once

#include <GGraph>
#include <GPcapDevice>
#include <GHostMgr>
#include <GHostWatch>
#include <GHostScan>
#include <GArpBlock>
#include <GHostDb>
#include <GCommand>
#include <GTreeWidget>

struct G_EXPORT HostAnalyzer : GGraph, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef hostMgr READ getHostMgr)
	Q_PROPERTY(GObjRef hostWatch READ getHostWatch)
	Q_PROPERTY(GObjRef hostScan READ getHostScan)
	Q_PROPERTY(GObjRef arpBlock READ getArpBlock)
	Q_PROPERTY(GObjRef hostDb READ getHostDb)
	Q_PROPERTY(GObjRef command READ getCommand)
	Q_PROPERTY(int updateHostsTimeout MEMBER updateHostsTimrout_)
	Q_PROPERTY(int updateElapsedTimeout MEMBER updateElapsedTimeout_)

public:
	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getHostMgr() { return &hostMgr_; }
	GObjRef getHostWatch() { return &hostWatch_; }
	GObjRef getHostScan() { return &hostScan_; }
	GObjRef getArpBlock() { return &arpBlock_; }
	GObjRef getHostDb() { return &hostDb_; }
	GObjRef getCommand() { return &command_; }
	int updateHostsTimrout_{1000}; // 1 seconds
	int updateElapsedTimeout_{10000}; // 10 seconds

public:
	Q_INVOKABLE HostAnalyzer(QObject* parent = nullptr);
	~HostAnalyzer() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPcapDevice pcapDevice_{this};
	GHostMgr hostMgr_{this};
	GHostWatch hostWatch_{this};
	GHostScan hostScan_{this};
	GArpBlock arpBlock_{this};
	GHostDb hostDb_{this};
	GCommand command_{this};

public:
	const static int ColumnIp = 0;
	const static int ColumnName = 1;
	const static int ColumnElapsed = 2;
	const static int ColumnAttack = 3;

	QTimer updateHostsTimer_;
	QTimer updateElapsedTimer_;

protected:
	// GHostMgr::Managable
	size_t hostOffset_{0};
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item {
		enum State {
			Created,
			Changed,
			NotChanged
		} state_;
		QTreeWidgetItem* treeWidgetItem_;
		GMac mac_;
		GIp ip_;
		QString defaultName_;
		struct timeval firstTs_;
	};
	typedef Item *PItem;
	// --------------------------------------------------------------------------

public:
	GTreeWidget* treeWidget_{nullptr}; // reference

public slots:
	void processClosed();
	void toolButton_toggled(bool checked);
	void updateHosts();
	void updateElapsedTime();
	void treeWidget_itemChanged(QTreeWidgetItem *item, int column);

public:
	void propLoad(QJsonObject jo) override;
	void propSave(QJsonObject& jo) override;
};
