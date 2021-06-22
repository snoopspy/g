#include "gapp.h"
#include "base/gjson.h"
#include "base/graph/ggraphwidget.h"
#include "base/log/glogmanager.h"
#include "base/log/glogdbwin32.h"
#include "base/log/glogfile.h"
#include "base/log/glogstderr.h"
#include "base/log/glogudp.h"

// ----------------------------------------------------------------------------
// GApp
// ----------------------------------------------------------------------------
#ifdef QT_GUI_LIB
GApp::GApp(int &argc, char** argv) : QApplication(argc, argv) {
#else
GApp::GApp(int &argc, char** argv) : QCoreApplication(argc, argv) {
#endif // QT_GUI_LIB
	initLogger();

	QString appName = QCoreApplication::applicationName();
	qInfo() << "====================================================================";
	qInfo() << appName << G::version();
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
#ifdef Q_OS_ANDROID
	logManager.push_back(new GLogFile);
#endif // Q_OS_ANDROID
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
		QString run = QString("cd %1; export LD_LIBRARY_PATH=%2; export LD_PRELOAD=libfakeioctl.so; ./%3").arg(path, path + "/../lib", ssdemonFile)};
#else // Q_OS_ANDROID
		QString run = QString("cd %1; ./%2").arg(path, ssdemonFile);
#endif // Q_OS_ANDROID
		arguments.append(run);
		qDebug() << arguments; // gilgil temp 2021.06.23
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
