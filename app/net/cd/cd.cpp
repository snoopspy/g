#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "cdwidget.h"

int main(int argc, char *argv[]) {
	GApp a(argc, argv, {"ssdemon"}, true, false, true);
	CdWidget cw;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("cw") == jo.end();
	jo["cw"] >> cw;

	cw.show();
	if (isFirst) {
		int width = cw.width();
		cw.treeWidget_->setColumnWidth(0, width / 3);
		cw.treeWidget_->setColumnWidth(1, width / 3);
	}

	GCommand* command = &cw.cycleDetect_.command_;
	if (!command->open()) {
		QMessageBox::warning(&cw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["cw"] << cw;

	return res;
}
