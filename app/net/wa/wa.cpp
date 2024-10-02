#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "wawidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv, {"ssdemon"}, true, true, true);
	WaWidget wa;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("ww") == jo.end();
	jo["ww"] >> wa;

	wa.show();
	if (isFirst) {
		int width = wa.width();
		wa.tableWidget_->setColumnWidth(WifiAnalyzer::ColumnMac, width / 4);
		wa.tableWidget_->setColumnWidth(WifiAnalyzer::ColumnSsid, width / 4);
		wa.tableWidget_->setColumnWidth(WifiAnalyzer::ColumnChannel, width / 12);
	}

	GCommand* command = &wa.wifiAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&wa, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["ww"] << wa;

	return res;
}
