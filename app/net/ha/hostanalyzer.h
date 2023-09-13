#pragma once

#include <GCommand>
#include <GPcapDevice>
#include <GHostMgr>
#include <GHostWatch>
#include <GHostScan>
#include <GArpBlock>
#include <GTreeWidget>
#include "hostdb.h"

struct G_EXPORT HostAnalyzer : GStateObj, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef hostMgr READ getHostMgr)
	Q_PROPERTY(GObjRef hostWatch READ getHostWatch)
	Q_PROPERTY(GObjRef hostScan READ getHostScan)
	Q_PROPERTY(GObjRef arpBlock READ getArpBlock)
	Q_PROPERTY(GObjRef command READ getCommand)
	Q_PROPERTY(GObjRef hostDb READ getHostDb)

public:
	Q_INVOKABLE HostAnalyzer(QObject* parent = nullptr);
	~HostAnalyzer() override;

public:
	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getHostMgr() { return &hostMgr_; }
	GObjRef getHostWatch() { return &hostWatch_; }
	GObjRef getHostScan() { return &hostScan_; }
	GObjRef getArpBlock() { return &arpBlock_; }
	GObjRef getCommand() { return &command_; }
	GObjRef getHostDb() { return &hostDb_; }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPcapDevice pcapDevice_{this};
	GHostMgr hostMgr_{this};
	GHostWatch hostWatch_{this};
	GHostScan hostScan_{this};
	GArpBlock arpBlock_{this};
	GCommand command_{this};
	HostDb hostDb_{this};

public:
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
	GTreeWidget* treeWidget_{nullptr};

public slots:
	void toolButton_toggled(bool checked);
	void updateHosts();
	void updateElapsedTime();
};
