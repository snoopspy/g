#pragma once

#include <GCommand>
#include <GMonitorDevice>

struct G_EXPORT ProbeAnalyzer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(GObjRef monitorDevice READ getMonitorDevice)
	Q_PROPERTY(int minSignal MEMBER minSignal_)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	GObjRef getMonitorDevice() { return &monitorDevice_; }
	GObjRef getCommand() { return &command_; }
	int minSignal_{-40};

public:
	Q_INVOKABLE ProbeAnalyzer(QObject* parent = nullptr);
	~ProbeAnalyzer() override;

protected:
	bool doOpen() override;
	bool doClose() override;

protected:
	GCommand command_;
	GMonitorDevice monitorDevice_;

public slots:
	void processCaptured(GPacket* packet);

signals:
	void probeDetected(GMac mac, int8_t signal);
};
