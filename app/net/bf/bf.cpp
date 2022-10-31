#include <QMessageBox>
#include <GApp>
#include <GJson>
#include "bfwidget.h"

int main(int argc, char *argv[])
{
	GApp a(argc, argv);
	BfWidget bw;

	QJsonObject& jo = GJson::instance();
	jo["bw"] >> bw;
	bw.plainTextEdit_->setPlainText(bw.beaconFlood_.beaconFlood_.messages_.join("\n"));

	bw.show();

	GCommand* command = &bw.beaconFlood_.command_;
	if (!command->open()) {
		QMessageBox::warning(&bw, "Error", command->err->msg());
	}

	int res = a.exec();

	command->close();

	bw.beaconFlood_.beaconFlood_.messages_ = bw.plainTextEdit_->toPlainText().split("\n");
	jo["bw"] << bw;

	return res;
}
