#pragma once

#include <GCommand>
#include <GBeaconFlood>
#include <GPlainTextEdit>

struct G_EXPORT BeaconFlood : GStateObj {
	Q_OBJECT
	Q_PROPERTY(GObjRef beaconFlood READ getBeaconFlood)
	Q_PROPERTY(GObjRef command READ getCommand)

public:
	Q_INVOKABLE BeaconFlood(QObject* parent = nullptr);
	~BeaconFlood() override;

public:
	GObjRef getBeaconFlood() { return &beaconFlood_; }
	GObjRef getCommand() { return &command_; }

protected:
	bool doOpen() override;
	bool doClose() override;

public:
	GBeaconFlood beaconFlood_{this};
	GCommand command_{this};

public:
	GPlainTextEdit* plainTextEdit_{nullptr};
};
