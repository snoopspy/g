#include "gapp.h"
#include "base/gjson.h"
#include "base/log/glogmanager.h"
#include "base/log/glogdbwin32.h"
#include "base/log/glogfile.h"
#include "base/log/glogstderr.h"
#include "base/log/glogudp.h"
#include "base/sys/gnexmon.h"
#include "net/demon/gdemon.h"

std::string getDir(std::string argv) {
	ssize_t i = argv.length() - 1;
	while (i >= 0) {
		char& ch = argv.at(i);
		if (ch == '/' || ch == '\\') {
			std::string res = argv.substr(0, i + 1);
			return res;
		}
		i--;
	}
	return "/";
}

#include <unistd.h> // for chdir

// ----------------------------------------------------------------------------
// GApp
// ----------------------------------------------------------------------------
#ifdef QT_GUI_LIB
GApp::GApp(int &argc, char** argv, bool demon, bool nexmonDemon, bool screenKeep) : QApplication(argc, argv) {
#else
GApp::GApp(int &argc, char** argv, bool demon, bool nexmonDemon, bool screenKeep) : QCoreApplication(argc, argv) {
#endif // QT_GUI_LIB

#ifndef Q_OS_ANDROID
	int res = chdir(getDir(argv[0]).data());
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

#ifdef Q_OS_ANDROID
	copyFileFromAssets("arprecover", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	copyFileFromAssets("corepcap", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	copyFileFromAssets("ssdemon", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	system("su -c '/data/data/com.termux/files/usr/bin/termux-wake-lock'");
#endif // Q_OS_ANDROID

	if (demon)
		launchDemon(&demon_, GDemon::DefaultPort);
	if (nexmonDemon) {
		QString soFileName = GNexmon::soFileName();
		launchDemon(&nexmonDemon_, GDemon::NexmonPort, soFileName);
	}

	screenKeep_ = screenKeep;
	if (screenKeep_)
		screenKeeper_.open();
}

GApp::~GApp() {
	if (demon_.state() == QProcess::Running) {
		demon_.terminate();
		demon_.waitForFinished();
	}
	if (nexmonDemon_.state() == QProcess::Running) {
		nexmonDemon_.terminate();
		nexmonDemon_.waitForFinished();
	}

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

void GApp::launchDemon(QProcess* demon, int port, QString soFileName) {
	QString ssdemonFile = "ssdemon";
	if (QFile::exists(ssdemonFile)) {
		QStringList arguments;
		arguments.append("-c");
		QString path = QDir::currentPath();
#ifdef Q_OS_ANDROID
		QString preloadStr = " ";
		if (soFileName != "")
			preloadStr = "export LD_PRELOAD=" + soFileName + ";";
		QString run = QString("export LD_LIBRARY_PATH=%1; %2 %3/%4 %5").arg(path + "/../lib", preloadStr, path, ssdemonFile, QString::number(port));
#else // Q_OS_ANDROID
		(void)port;
		(void)soFileName;
		QString run = QString("%1/%2 %3").arg(path, ssdemonFile, QString::number(port));
#endif // Q_OS_ANDROID
		arguments.append(run);
		demon->start("su", arguments);
	}
}

bool GApp::copyFileFromAssets(QString fileName, QFile::Permissions permissions) {
	QString srcFileName = QString("assets:/") + fileName;
	QString dstFileName = fileName;
	QFile srcFile(srcFileName);
	QFile dstFile(dstFileName);

	if (!srcFile.exists()) {
		qWarning() << QString("src file(%1) not exists").arg(srcFileName);
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
