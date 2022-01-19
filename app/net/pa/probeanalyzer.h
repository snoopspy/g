#pragma once

#include <GCommand>
#include <GMonitorDevice>

struct G_EXPORT ProbeAnalyzer : GStateObj {
	Q_OBJECT
	Q_PROPERTY(int minSignal MEMBER minSignal_)
	Q_PROPERTY(GObjRef monitorDevice READ getMonitorDevice)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	GObjRef getMonitorDevice() { return &monitorDevice_; }
	int minSignal_{-128};
	GObjRef getCommand() { return &command_; }

public:
	Q_INVOKABLE ProbeAnalyzer(QObject* parent = nullptr);
	~ProbeAnalyzer() override;

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GMonitorDevice monitorDevice_;
	GCommand command_;

public slots:
	void processCaptured(GPacket* packet);

signals:
	void probeDetected(GMac mac, int signal);
};
