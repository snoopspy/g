#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "wawidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	WaWidget wj;

	QJsonObject& jo = GJson::instance();
	jo["wj"] >> wj;

	wj.show();

	GCommand* command = &wj.wifiJammer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&wj, "Error", command->err->msg());
		return -1;
	}

	int res = a.exec();

	command->close();

	jo["wj"] << wj;

	return res;
}
