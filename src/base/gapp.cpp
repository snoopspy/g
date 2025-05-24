#include "gapp.h"
#include "base/gjson.h"
#include "base/log/glogmanager.h"
#include "base/log/glogdbwin32.h"
#include "base/log/glogfile.h"
#include "base/log/glogstderr.h"
#include "base/log/glogudp.h"
#include "base/sys/gnexmon.h"
#include "net/demon/gdemon.h"

#include <unistd.h> // for chdir

// ----------------------------------------------------------------------------
// GApp
// ----------------------------------------------------------------------------
#ifdef QT_GUI_LIB
GApp::GApp(int &argc, char** argv, QStringList assets, bool demon, bool nexmonDemon, bool screenKeep) : QApplication(argc, argv) {
#else
GApp::GApp(int &argc, char** argv, QStringList assets, bool demon, bool nexmonDemon, bool screenKeep) : QCoreApplication(argc, argv) {
#endif // QT_GUI_LIB

#ifndef Q_OS_ANDROID
	int res = chdir(qPrintable(QCoreApplication::applicationDirPath()));
	if (res != 0) {
		GLastErr lastErr;
		fprintf(stderr, "%s\n", qPrintable(lastErr.msg()));
	}
#endif

	initLogger();

	QString appName = QCoreApplication::applicationName();
	qInfo() << "====================================================================";
	qInfo() << appName << "G Libary" << G::version();
	qInfo() << "Copyright (c) Gilbert Lee All rights reserved";
	qInfo() << G::pcapLibVersion();

	for (QString asset: assets)
		copyFileFromAssets(asset);

	if (demon)
		launchDemon(&demon_, GDemon::DefaultPort);
	if (nexmonDemon) {
		QString preloadFileName = GNexmon::preloadFileName();
		launchDemon(&nexmonDemon_, GDemon::NexmonPort, preloadFileName);
	}

	screenKeep_ = screenKeep;
	if (screenKeep_) {
#ifdef Q_OS_ANDROID
		system("su -c '/data/data/com.termux/files/usr/bin/termux-wake-lock'");
#endif // Q_OS_ANDROID
		screenKeeper_.open();
	}
}

GApp::~GApp() {
	bool shouldKillDemon = demon_.state() == QProcess::Running;
	bool shouldKillNexmonDemon = nexmonDemon_.state() == QProcess::Running;

	if (shouldKillDemon)
		demon_.terminate();
	if (shouldKillNexmonDemon)
		nexmonDemon_.terminate();

	if (shouldKillDemon)
		demon_.waitForFinished();
	if (shouldKillNexmonDemon)
		nexmonDemon_.waitForFinished();

#ifdef Q_OS_ANDROID
	system("su -c 'pkill ssdemon'"); // ssdemon is not terminated properly on android
	system("su -c '/data/data/com.termux/files/usr/bin/termux-wake-unlock'");
#endif // Q_OS_ANDROID

	if (screenKeep_)
		screenKeeper_.close();

	QString appName = QCoreApplication::applicationName();
	qInfo() << appName << "terminated successfully";
}

void GApp::initLogger() {
	GLogManager& logManager = GLogManager::instance();
	if (QFile::exists("sslog.ss")) {
		QJsonObject jo = GJson::loadFromFile("sslog.ss");
		QJsonArray ja = jo["logs"].toArray();
		logManager.load(ja);
	} else {
#ifdef Q_OS_WIN
	logManager.push_back(new GLogDbWin32);
#endif // Q_OS_WIN
#ifdef Q_OS_LINUX
	logManager.push_back(new GLogStderr);
#endif // Q_OS_LINUX
	logManager.push_back(new GLogFile);
	logManager.push_back(new GLogUdp);
	}
}

bool GApp::copyFileFromAssets(QString fileName, QString directory, QFile::Permissions permissions) {
	QString srcFileName = QString("assets:/") + fileName;
	QString dstFileName = directory + fileName;
	QFile srcFile(srcFileName);
	QFile dstFile(dstFileName);

	if (!srcFile.exists()) {
#ifdef Q_OS_ANDROID
		qWarning() << QString("src file(%1) not exists").arg(srcFileName);
#endif // Q_OS_ANDROID
		return false;
	}

	if (dstFile.exists()) {
		bool res = dstFile.remove();
		if (!res) {
			qWarning() << QString("file remove(%1) return false").arg(dstFileName);
			return false;
		}
	}

	if (!srcFile.copy(dstFileName)) {
		qWarning() << QString("file copy(%1 %2) return false").arg(srcFileName).arg(dstFileName);
		return false;
	}
	QFile::setPermissions(fileName, permissions);

	return true;
}

bool GApp::prepareProcess(QString& program, QStringList& arguments, QString preloadFileName) {
	if (!QFile::exists(program)) return false;

	QString path = QDir::currentPath();
	qDebug() << QString("path=%1").arg(path);
#ifdef Q_OS_WIN
	program = path + QDir::separator() + program;
	return true;
#endif // Q_OS_WIN

	QString preloadStr = " ";
	if (preloadFileName != "")
		preloadStr = "export LD_PRELOAD=" + preloadFileName + "; ";
#ifdef Q_OS_ANDROID
	QString run = QString("export LD_LIBRARY_PATH=%1; %2%3/%4 %5").arg(path + "/../lib", preloadStr, path, program, arguments.join(" "));
#else // Q_OS_ANDROID
	QString run = QString("%1%2/%3 %4").arg(preloadStr, path, program, arguments.join(" "));
#endif // Q_OS_ANDROID

	QStringList newArguments;
	newArguments.append("-c");
	newArguments.append(run);
	program = "su";
	arguments = newArguments;
	return true;
}

void GApp::launchDemon(QProcess* demon, int port, QString preloadFileName) {
	QString program = "ssdemon";
	QStringList arguments;
	arguments.push_back(QString::number(port));
	if (prepareProcess(program, arguments, preloadFileName)) {
		demon->start(program, arguments);
	}
}
