#pragma once

#include <GCommand>
#include <GMonitorDevice>

struct G_EXPORT WifiJammer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(GObjRef monitorDevice READ getMonitorDevice)
	Q_PROPERTY(bool channelHopping MEMBER channelHopping_)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	GObjRef getMonitorDevice() { return &monitorDevice_; }
	GObjRef getCommand() { return &command_; }
	bool channelHopping_{true};

public:
	Q_INVOKABLE WifiJammer(QObject* parent = nullptr);
	~WifiJammer() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GCommand command_;
	GMonitorDevice monitorDevice_;

public slots:
	void processCaptured(GPacket* packet);

signals:
	void attacked(GMac mac, QString ssid);
};
