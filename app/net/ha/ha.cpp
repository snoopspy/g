#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "hawidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	HaWidget hw;

	QJsonObject& jo = GJson::instance();
	jo["hw"] >> hw;

	hw.show();

	GCommand* command = &hw.hostAnalyzer_.command_;
	if (!command->open()) {
		QMessageBox::warning(&hw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["hw"] << hw;

	return res;
}
