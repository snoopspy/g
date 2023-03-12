// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include "glog.h"
#include "base/sys/gwaitevent.h"
#include <QDir>
#include <QFile>
#include <QThread>

// ----------------------------------------------------------------------------
// GLogFile
// ----------------------------------------------------------------------------
struct G_EXPORT GLogFile : GLog {
	Q_OBJECT
	Q_PROPERTY(QString folder MEMBER folder_)

public:
	Q_INVOKABLE GLogFile(QObject* parent = nullptr);
	~GLogFile() override;

	void write(QString& msg) override;

protected:
	bool configured_{false};
	void configure();

protected:
	QString folder_{QString("log") + QDir::separator()};
	QMutex m_;
	QString fileName_;
	QFile file_;
	void changeFileName();

	struct ChangeFileNameThread : QThread {
		ChangeFileNameThread(GLogFile* logFile) : logFile_(logFile) {}
		GLogFile* logFile_;
		GStateWaitEvent swe_;
		void quit();
		void run() override;
	} thread_{this};
};
