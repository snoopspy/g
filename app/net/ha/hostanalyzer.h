#pragma once

#include <GGraph>
#include <GPcapDevice>
#include <GHostMgr>
#include <GHostWatch>
#include <GHostScan>
#include <GHostDb>
#include <GArpBlock>
#include <GCommand>
#include <GScreenKeeper>
#include <GScreenSaver>
#include <GTreeWidget>

struct G_EXPORT HostAnalyzer : GGraph, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(int updateHostsTimeoutSec MEMBER updateHostsTimeoutSec_)
	Q_PROPERTY(int updateElapsedTimeoutSec MEMBER updateElapsedTimeoutSec_)
	Q_PROPERTY(int extendTimeoutSec MEMBER extendTimeoutSec_)
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef hostMgr READ getHostMgr)
	Q_PROPERTY(GObjRef hostWatch READ getHostWatch)
	Q_PROPERTY(GObjRef hostScan READ getHostScan)
	Q_PROPERTY(GObjRef hostDb READ getHostDb)
	Q_PROPERTY(GObjRef arpBlock READ getArpBlock)
	Q_PROPERTY(GObjRef command READ getCommand)
	Q_PROPERTY(GObjRef screenKeeper READ getScreenKeeper)
	Q_PROPERTY(GObjRef screenSaver READ getScreenSaver)

public:
	int updateHostsTimeoutSec_{1}; // 1 seconds
	int updateElapsedTimeoutSec_{10}; // 10 seconds
	int extendTimeoutSec_{3600}; // 1 hour
	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getHostMgr() { return &hostMgr_; }
	GObjRef getHostWatch() { return &hostWatch_; }
	GObjRef getHostScan() { return &hostScan_; }
	GObjRef getHostDb() { return &hostDb_; }
	GObjRef getArpBlock() { return &arpBlock_; }
	GObjRef getCommand() { return &command_; }
	GObjRef getScreenKeeper() { return &screenKeeper_; }
	GObjRef getScreenSaver() { return &screenSaver_; }

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
	GHostDb hostDb_{this};
	GArpBlock arpBlock_{this};
	GCommand command_{this};
	GScreenKeeper screenKeeper_{this};
	GScreenSaver screenSaver_{this};

public:
	const static int ColumnIp = 0;
	const static int ColumnName = 1;
	const static int ColumnElapsed = 2;
	const static int ColumnAttack = 3;

	QTimer updateHostsTimer_;
	QTimer updateElapsedTimer_;

protected:
	// GHostMgr::Managable
	size_t itemOffset_{0};
	void hostCreated(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostDeleted(GMac mac, GHostMgr::HostValue* hostValue) override;
	void hostChanged(GMac mac, GHostMgr::HostValue* hostValue) override;

	// --------------------------------------------------------------------------
	// Item
	// --------------------------------------------------------------------------
	struct Item : GArpBlock::Item {
		enum State {
			Created,
			Changed,
			NotChanged
		} state_{Created};
		QTreeWidgetItem* treeWidgetItem_{nullptr};
		QString defaultName_;
		time_t firstTime_;
		time_t blockTime_;
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
