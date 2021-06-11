#include <GApp>
#include <GGraphWidget>
#include "myfactory.h"

int exec(GApp* a, GGraph* graph, GPluginFactory* pluginFactory) {
	Q_ASSERT(graph != nullptr);

	if (pluginFactory == nullptr) {
		pluginFactory = &GPluginFactory::instance();
	}
	graph->setFactory(pluginFactory);

	GGraphWidget graphWidget;
	graphWidget.setGraph(graph);

	QJsonObject jo = GJson::loadFromFile();
	jo["graphWidget"] >> graphWidget;

	graphWidget.update();
	graphWidget.show();
	int res = a->exec();

	jo["graphWidget"] << graphWidget;

	GJson::saveToFile(jo);
	return res;
}

int main(int argc, char *argv[]) {
	GApp a(argc, argv);
	GGraph graph;
	MyFactory factory;
	return exec(&a, &graph, &factory);
}
