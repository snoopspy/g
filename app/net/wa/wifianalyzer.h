#pragma once

#include <GMonitorDevice>
#include <GChannelHop>
#include <GCommand>

struct G_EXPORT WifiAnalyzer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int minSignal MEMBER minSignal_)
	Q_PROPERTY(int updateInterval MEMBER updateInterval_)
	Q_PROPERTY(bool channelHopping MEMBER channelHopping_)
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
	int updateInterval_{1000};
	bool channelHopping_{true};
	ShowType showType_{Average};
	GObjRef getMonitorDevice() { return &monitorDevice_; }
	GObjRef getChannelHop() { return &channelHop_; }
	GObjRef getCommand() { return &command_; }

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

signals:
	void detected(GMac mac, QString ssid, int signal);
};
