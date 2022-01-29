#pragma once

#include <GMonitorDevice>
#include <GChannelHop>
#include <GCommand>

struct G_EXPORT WifiAnalyzer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int minSignal MEMBER minSignal_)
	Q_PROPERTY(int updateInterval MEMBER updateInterval_)
	Q_PROPERTY(bool ignoreOtherChannelFrame MEMBER ignoreOtherChannelFrame_)
	Q_PROPERTY(ShowType showType MEMBER showType_)
	Q_PROPERTY(GObjRef monitorDevice READ getMonitorDevice)
	Q_PROPERTY(GObjRef channelHop READ getChannelHop)
	Q_PROPERTY(GObjRef command READ getCommand)
	Q_ENUMS(ShowType)

public:
	enum ShowType {
		Average,
		Max,
		Min
	};

public:
	int minSignal_{-128};
	int updateInterval_{250};
	bool ignoreOtherChannelFrame_{true};
	ShowType showType_{Average};
	GObjRef getMonitorDevice() { return &monitorDevice_; }
	GObjRef getChannelHop() { return &channelHop_; }
	GObjRef getCommand() { return &command_; }

protected:
	int currentChannel_{0};

public:
	Q_INVOKABLE WifiAnalyzer(QObject* parent = nullptr);
	~WifiAnalyzer() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GMonitorDevice monitorDevice_;
	GCommand command_;
	GChannelHop channelHop_;

public slots:
	void processCaptured(GPacket* packet);
	void processChannelChanged(int channel);

signals:
	void detected(GMac mac, QString ssid, int channel, int signal);
};
