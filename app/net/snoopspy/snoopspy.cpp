#include <GApp>
#include <GGraphWidget>
#include <GSignal>
#include <GPluginFactory>

struct G_EXPORT SnoopSpy : GStateObj {
	Q_OBJECT

public:
	SnoopSpy(QObject* parent = nullptr) : GStateObj(parent) {}

	~SnoopSpy() override {
		graph_.close();
	}

protected:
	GGraph graph_;
	GPluginFactory pluginFactory_;
	GGraphWidget graphWidget_;

	bool doOpen() override {
#ifndef Q_OS_WIN
		GSignal& signal = GSignal::instance();
		QObject::connect(&signal, &GSignal::signaled, this, &SnoopSpy::processSignal, Qt::DirectConnection);
		signal.setupAll();
#endif // Q_OS_WIN

		graph_.setFactory(&pluginFactory_);
		graphWidget_.setGraph(&graph_);

		QJsonObject& jo = GJson::instance();
		bool isFirst = jo.find("graphWidget") == jo.end();
		jo["graphWidget"] >> graphWidget_;

		graphWidget_.show();
		if (isFirst) {
			int width = graphWidget_.tabWidget_->width();
			int height = graphWidget_.tabWidget_->height() - graphWidget_.tabWidget_->tabBar()->height();
			graphWidget_.midSplitter_->setSizes(QList<int>{width / 2, width / 2});
			graphWidget_.midLeftSplitter_->setSizes(QList<int>{height / 2, height/ 2});
			graphWidget_.factoryWidget_->setColumnWidth(0, width / 2);
			graphWidget_.propWidget_->treeWidget_->setColumnWidth(0, width / 4);
		}

		return true;
	}

	bool doClose() override {
		graphWidget_.close();

		QJsonObject& jo = GJson::instance();
		jo["graphWidget"] << graphWidget_;

		return true;
	}

public slots:
	void processSignal(int signo) {
#ifdef Q_OS_WIN
		(void)signo;
#else // Q_OS_WIN
		QString str1 = GSignal::getString(signo);
		QString str2 = strsignal(signo);
		qWarning() << QString("signo=%1 signal=%2 msg=%3 _debug_gilgil=%4 _thread_gilgil=%5").arg(signo).arg(str1).arg(str2).arg(_debug_gilgil).arg(_thread_gilgil);
		graphWidget_.close();
#endif
	}
};

int main(int argc, char *argv[]) {
	GApp a(argc, argv, true, true, false);
	SnoopSpy* ss = new SnoopSpy;
	if (!ss->open())
		return -1;
	int res = a.exec();
	ss->close();
	delete ss;
	return res;
}

#include "snoopspy.moc"
