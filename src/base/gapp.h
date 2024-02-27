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

#ifdef QT_GUI_LIB
#include <QApplication>
#else
#include <QCoreApplication>
#endif // QT_GUI_LIB
#include <QFile>
#include <QProcess>
#include "base/gbase.h"
#include "net/process/gscreenkeeper.h"

// ----------------------------------------------------------------------------
// GApp
// ----------------------------------------------------------------------------
#ifdef QT_GUI_LIB
struct G_EXPORT GApp : QApplication {
#else
struct G_EXPORT GApp : QCoreApplication {
#endif // QT_GUI_LIB

public:
	GApp(int &argc, char** argv, bool demon = true, bool nexmonDemon = true);
	~GApp() override;

	QProcess demon_;
	QProcess nexmonDemon_;
	GScreenKeeper screenKeeper_;

	static void initLogger();
	void launchDemon(QProcess* demon, int port, QString soFileName = "");
	static bool copyFileFromAssets(QString fileName, QFile::Permissions permissions);
};
