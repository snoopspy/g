#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "chwidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv, true, true);
	ChWidget cw;

	QJsonObject& jo = GJson::instance();
	bool isFirst = jo.find("pw") == jo.end();
	jo["cw"] >> cw;

	cw.show();
	if (isFirst) {
		int width = cw.width();
		cw.treeWidget_->setColumnWidth(CookieHijack::ColumnMac, width * 4 / 10);
		cw.treeWidget_->setColumnWidth(CookieHijack::ColumnType, width * 1 / 10);
		cw.treeWidget_->setColumnWidth(CookieHijack::ColumnChannel, width * 1 / 10);
		cw.treeWidget_->setColumnWidth(CookieHijack::ColumnSignal, width * 4 / 10);
	}

	GCommand* command = &cw.cookieHijack_.command_;
	if (!command->open()) {
		QMessageBox::warning(&cw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	jo["cw"] << cw;

	return res;
}
