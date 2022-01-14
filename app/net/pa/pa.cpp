#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "probewidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	ProbeWidget pw;

	QJsonObject jo = GJson::loadFromFile();
	jo["pw"] >> pw;

	pw.show();

	GCommand* command = &pw.probeAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&pw, "Error", command->err->msg());
		return -1;
	}

	int res = a.exec();

	command->close();

	jo["pw"] << pw;
	GJson::saveToFile(jo);

	return res;
}
