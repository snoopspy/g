#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "wawidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	WaWidget wa;

	QJsonObject& jo = GJson::instance();
	jo["wa"] >> wa;

	wa.show();

	GCommand* command = &wa.wifiAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&wa, "Error", command->err->msg());
		return -1;
	}

	int res = a.exec();

	command->close();

	jo["wa"] << wa;

	return res;
}
