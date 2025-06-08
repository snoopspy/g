#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "hawidget.h"

int main(int argc, char *argv[]) {
	GApp a(argc, argv, {"arprecover", "ssdemon"}, true);
	a.launchDemon(false);

	HaWidget hw;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("hw") == jo.end();
	jo["hw"] >> hw;

	hw.show();
	if (isFirst) {
		int width = hw.width();
		hw.treeWidget_->setColumnWidth(HostAnalyzer::ColumnIp, width / 3);
		hw.treeWidget_->setColumnWidth(HostAnalyzer::ColumnName, width / 3);
	}

	GCommand* command = &hw.hostAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&hw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["hw"] << hw;

	return res;
}
