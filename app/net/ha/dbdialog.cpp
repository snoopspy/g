#include "dbdialog.h"

#include <GJson>

DbDialog::DbDialog(QWidget* parent) : QDialog(parent) {
	resize(QSize(640, 480));
	setWindowTitle("Database");
}

DbDialog::~DbDialog() {
}

void DbDialog::propLoad(QJsonObject jo) {
	jo["rect"] >> GJson::rect(this);
}

void DbDialog::propSave(QJsonObject& jo) {
	jo["rect"] << GJson::rect(this);
}
