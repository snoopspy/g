#include <GApp>
#include <GGraphWidget>
#include <GPluginFactory>

struct G_EXPORT SnoopSpy : GStateObj {
	Q_OBJECT

public:
	SnoopSpy(QObject* parent = nullptr) : GStateObj(parent) {}

	~SnoopSpy() override {
		graph_.close();
	}

public:
	GGraph graph_;
	GPluginFactory pluginFactory_;
	GGraphWidget graphWidget_;

protected:
	bool doOpen() override {
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
};

int main(int argc, char *argv[]) {
	GApp a(argc, argv, {"arprecover", "corepcap", "ssdemon"}, true);
	a.launchDemon(true);

	SnoopSpy* ss = new SnoopSpy;
	a.setSignalObject(&ss->graphWidget_);
	if (!ss->open())
		return -1;
	int res = a.exec();
	ss->close();
	delete ss;
	return res;
}

#include "snoopspy.moc"
