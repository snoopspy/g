#include "hostdialog.h"
#include <GJson>

HostDialog::HostDialog(QWidget* parent, GHostDb* hostDb, GMac mac) : QDialog(parent), hostDb_(hostDb), mac_(mac) {
	resize(QSize(640, 480));
	setWindowTitle("Host");
	qDebug() << QString(mac);
}

HostDialog::~HostDialog() {

}

void HostDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void HostDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}
