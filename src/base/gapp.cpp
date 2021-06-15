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
	qInfo() << appName << "terminated";
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
	copyFileFromAssets("ssdemon", QFile::ReadOwner | QFile::WriteOwner | QFile::ExeOwner);
#endif // Q_OS_ANDROID

	QString ssdemonFile = "ssdemon";
	if (QFile::exists(ssdemonFile)) {
#ifdef Q_OS_ANDROID
		QString program = "su";
		QStringList arguments{"-c", QString("cd %1; export LD_LIBRARY_PATH=%2; ./%3").arg(QDir::currentPath(), QDir::currentPath() + "/../lib", ssdemonFile)};
		demon_.start(program, arguments);
#else // Q_OS_ANDROID
		QString program = "su";
		QStringList arguments{"-c", QString("cd %1; ./%2").arg(QDir::currentPath(), ssdemonFile)};
		demon_.start(program, arguments);
#endif // Q_OS_ANDROID
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
