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
		cw.treeWidget_->setColumnWidth(ChWidget::ColumnHost, width / 2);
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
