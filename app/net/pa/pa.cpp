#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "pawidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv, true, true);
	PaWidget pw;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("pw") == jo.end();
	jo["pw"] >> pw;

	pw.show();
	if (isFirst) {
		int width = pw.width();
		pw.tableWidget_->setColumnWidth(0, width / 2);
	}

	GCommand* command = &pw.probeAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&pw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["pw"] << pw;

	return res;
}
