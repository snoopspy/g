#include <iostream>
#include <GApp>
#include <GGraph>
#include <GSignal>
#include <GPluginFactory>

using namespace std;

// ----------------------------------------------------------------------------
// GSsConParam
// ----------------------------------------------------------------------------
struct GSsConParam {
	QString fileName_;

	bool parse(QStringList arguments) {
		if (arguments.count() != 2) return false;
		fileName_ = arguments.at(1);
		return true;
	}

	static void usage() {
		QString appName = QCoreApplication::applicationName();
		cout << "usage : " << qPrintable(appName) << " <filename>" << endl;
		cout << endl;
		cout << "example : " << qPrintable(appName) << " ss/test.ss" << endl;
		cout << endl;
	}
};

// ----------------------------------------------------------------------------
// SsCon
// ----------------------------------------------------------------------------
struct G_EXPORT SsCon : GStateObj {
	Q_OBJECT

public:
	SsCon(QObject* parent = nullptr) : GStateObj(parent) {}

	~SsCon() override {
		graph_.close();
	}

protected:
	GGraph graph_;
	GPluginFactory pluginFactory_;

	bool doOpen() override {
#ifndef Q_OS_WIN
		GSignal& signal = GSignal::instance();
		QObject::connect(&signal, &GSignal::signaled, this, &SsCon::processSignal, Qt::DirectConnection);
		QObject::connect(&graph_, &GStateObj::closed, this, &SsCon::processClose);
		signal.setupAll();
#endif // Q_OS_WIN

		GSsConParam param;
		if (!param.parse(QCoreApplication::arguments())) {
			GSsConParam::usage();
			return false;
		}

		if (!QFile::exists(param.fileName_)) {
			cout << "no such file " << qPrintable(param.fileName_) << endl;
			return false;
		}

		graph_.setFactory(&pluginFactory_);

		QJsonObject jo = GJson::loadFromFile(param.fileName_);
		graph_.propLoad(jo);

		if (!graph_.open()) {
			qWarning() << graph_.err;
			return false;
		}

		return true;
	}

	bool doClose() override {
		return graph_.close();
	}

public slots:
	void processSignal(int signo) {
#ifdef Q_OS_WIN
		(void)signo;
#else // Q_OS_WIN
		QString str1 = GSignal::getString(signo);
		QString str2 = strsignal(signo);
		qWarning() << QString("signo=%1 signal=%2 msg=%3 _debug_gilgil=%4 _thread_gilgil=%5").arg(signo).arg(str1).arg(str2).arg(_debug_gilgil).arg(_thread_gilgil);
		graph_.close();
#endif
	}

	void processClose() {
		QCoreApplication::quit();
	}
};

int main(int argc, char* argv[]) {
	GApp a(argc, argv);
	SsCon* sc = new SsCon;;
	if (!sc->open()) {
		delete sc;
		return -1;
	}
	int res = a.exec();
	sc->close();
	delete sc;
	return res;
}

#include "sscon.moc"
