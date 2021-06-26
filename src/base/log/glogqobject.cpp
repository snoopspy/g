#include "glogqobject.h"

// ----------------------------------------------------------------------------
// GLogQObject
// ----------------------------------------------------------------------------
void GLogQObject::write(QString& msg) {
	emit writeLogRequred(msg);
}
