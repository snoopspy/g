#pragma once

#include <GCommand>
#include <GPcapDevice>
#include <GHostMgr>
#include <GHostWatch>
#include <GHostScan>
#include <GTreeWidget>

struct G_EXPORT HostAnalyzer : GStateObj, GHostMgr::Managable {
	Q_OBJECT
	Q_PROPERTY(GObjRef pcapDevice READ getPcapDevice)
	Q_PROPERTY(GObjRef hostMgr READ getHostMgr)
	Q_PROPERTY(GObjRef hostWatch READ getHostWatch)
	Q_PROPERTY(GObjRef hostScan READ getHostScan)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	Q_INVOKABLE HostAnalyzer(QObject* parent = nullptr);
	~HostAnalyzer() override;

public:
	GObjRef getPcapDevice() { return &pcapDevice_; }
	GObjRef getHostMgr() { return &hostMgr_; }
	GObjRef getHostWatch() { return &hostWatch_; }
	GObjRef getHostScan() { return &hostScan_; }
	GObjRef getCommand() { return &command_; }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GPcapDevice pcapDevice_{this};
	GHostMgr hostMgr_{this};
	GHostWatch hostWatch_{this};
	GHostScan hostScan_{this};
	GCommand command_{this};

protected:
	// GHostMgr::Managable
	size_t hostOffset_{0};
	void hostDetected(GMac mac, GHostMgr::Value* value) override;
	void hostDeleted(GMac mac, GHostMgr::Value* value) override;

public:
	GTreeWidget* treeWidget_{nullptr};
};