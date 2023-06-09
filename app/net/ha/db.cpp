#include "db.h"

Db::Db(QObject* parent) : GStateObj(parent) {
}

Db::~Db() {
	close();
}

bool Db::doOpen() {
	return true;
}

bool Db::doClose() {
	return true;
}
