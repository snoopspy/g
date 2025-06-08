#include <iostream>
#include <GApp>
#include <GGraph>
#include <GPluginFactory>

using namespace std;

// ----------------------------------------------------------------------------
// Param
// ----------------------------------------------------------------------------
struct Param {
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
	SsCon(QObject* parent = nullptr) : GStateObj(parent) {
		QObject::connect(&graph_, &GStateObj::closed, this, &SsCon::doClosed);
	}

	~SsCon() override {
		graph_.close();
	}

protected:
	GGraph graph_;
	GPluginFactory pluginFactory_;

	bool doOpen() override {
		Param param;
		if (!param.parse(QCoreApplication::arguments())) {
			Param::usage();
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
	void doClosed() {
		QCoreApplication::quit();
	}
};

int main(int argc, char* argv[]) {
	GApp a(argc, argv, {}, true);
	a.launchDemon(true);

	SsCon* sc = new SsCon;
	a.setSignalObject(sc);
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
