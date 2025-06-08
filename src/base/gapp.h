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
	GApp(int &argc, char** argv, QStringList assets = {}, bool screenKeep = false);
	~GApp() override;
	void launchDemon(bool nexmon);

protected:
	QProcess demon_;
	QProcess nexmonDemon_;
	static void initLogger();
	void launchDemon(QProcess* demon, int port, QString preloadFileName = "");

public:
	static bool copyFileFromAssets(QString fileName, QString directory = "", QFile::Permissions permissions = QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	static bool prepareProcess(QString& program, QStringList& arguments, QString preloadFileName = "");

protected:
	bool screenKeep_;
	GScreenKeeper screenKeeper_;
};
