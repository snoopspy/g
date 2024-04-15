#include <GApp>
#include <GJson>
#include <GPropWidget>
#include "dept.h"

int exec(GApp* a, GObj* obj) {
	GPropWidget propWidget;
	propWidget.setObject(obj);

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("object") == jo.end();

	jo["object"] >> *obj;
	jo["propWidget"] >> propWidget;

	propWidget.update();
	propWidget.show();
	if (isFirst) {
		int width = propWidget.width();
		propWidget.treeWidget_->setColumnWidth(0, width / 2);
	}

	int res = a->exec();

	jo["object"] << *obj;
	jo["propWidget"] << propWidget;

	return res;
}

int main(int argc, char *argv[]) {
	GApp a(argc, argv);
	Dept dept;
	return exec(&a, &dept);
}
