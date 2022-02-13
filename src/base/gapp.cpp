#include "gapp.h"
#include "base/gjson.h"
#include "base/graph/ggraphwidget.h"
#include "base/log/glogmanager.h"
#include "base/log/glogdbwin32.h"
#include "base/log/glogfile.h"
#include "base/log/glogstderr.h"
#include "base/log/glogudp.h"
#include "base/sys/gnexmon.h"

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
GApp::GApp(int &argc, char** argv) : QApplication(argc, argv) {
#else
GApp::GApp(int &argc, char** argv) : QCoreApplication(argc, argv) {
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
	qInfo() << G::pcap_lib_version();

	launchDemon();
}

GApp::~GApp() {
	demon_.terminate();
	demon_.waitForFinished();
#ifdef Q_OS_ANDROID
	system("su -c 'pkill ssdemon'"); // ssdemon is not terminated properly on android
#endif // Q_OS_ANDROID

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

void GApp::launchDemon() {
#ifdef Q_OS_ANDROID
	copyFileFromAssets("arprecover", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	copyFileFromAssets("corepcap", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
	copyFileFromAssets("ssdemon", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
#endif // Q_OS_ANDROID

	QString ssdemonFile = "ssdemon";
	if (QFile::exists(ssdemonFile)) {
		QStringList arguments;
		arguments.append("-c");
		QString path = QDir::currentPath();
#ifdef Q_OS_ANDROID
		QString preloadStr = " ";
		QString soFileName = GNexmon::soFileName();
		if (soFileName != "")
			preloadStr = "export LD_PRELOAD=" + soFileName + ";";
		QString run = QString("export LD_LIBRARY_PATH=%1; %2 %3/%4").arg(path + "/../lib", preloadStr, path, ssdemonFile);
#else // Q_OS_ANDROID
		QString run = QString("%1/%2").arg(path, ssdemonFile);
#endif // Q_OS_ANDROID
		arguments.append(run);
		qDebug() << arguments;
		demon_.start("su", arguments);
	}
}

bool GApp::copyFileFromAssets(QString fileName, QFile::Permissions permissions) {
	QString sourceFileName = QString("assets:/") + fileName;
	QFile sFile(sourceFileName);
	QFile dFile(fileName);
	if (!dFile.exists()) {
		if (!sFile.exists()) {
			qWarning() << QString("src file(%1) not exists").arg(sourceFileName);
			return false;
		}

		if (!sFile.copy(fileName)) {
			qWarning() << QString("file copy(%1) return false").arg(fileName);
			return false;
		}
		QFile::setPermissions(fileName, permissions);
	}
	return true;
}
