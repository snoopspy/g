#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "chwidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv, true, true);
	ChWidget pw;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("pw") == jo.end();
	jo["pw"] >> pw;

	pw.show();
	if (isFirst) {
		int width = pw.width();
		pw.treeWidget_->setColumnWidth(CookieHijack::ColumnMac, width * 4 / 10);
		pw.treeWidget_->setColumnWidth(CookieHijack::ColumnType, width * 1 / 10);
		pw.treeWidget_->setColumnWidth(CookieHijack::ColumnChannel, width * 1 / 10);
		pw.treeWidget_->setColumnWidth(CookieHijack::ColumnSignal, width * 4 / 10);
	}

	GCommand* command = &pw.cookieHijack_.command_;
	if (!command->open()) {
		QMessageBox::warning(&pw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["pw"] << pw;

	return res;
}
