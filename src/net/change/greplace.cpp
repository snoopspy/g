#include "greplace.h"

// ----------------------------------------------------------------------------
// GReplace
// ----------------------------------------------------------------------------
GReplace::GReplace(QObject* parent) : GStateObj(parent) {
	qDebug() << "";
}

GReplace::~GReplace() {
	qDebug() << "";
}

bool GReplace::doOpen() {
	qDebug() << "";
	return true;
}

bool GReplace::doClose() {
	qDebug() << "";
	return true;
}

void GReplace::replace(GPacket* packet) {
	(void)packet;
	qDebug() << "";
}
