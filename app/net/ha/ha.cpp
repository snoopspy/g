#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "hawidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	HaWidget pw;

	QJsonObject& jo = GJson::instance();
	jo["pw"] >> pw;

	pw.show();

	GCommand* command = &pw.hostAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&pw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["pw"] << pw;

	return res;
}
