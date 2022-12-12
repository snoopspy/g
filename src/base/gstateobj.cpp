#include "gstateobj.h"
#include "base/sys/gthread.h"

// ----------------------------------------------------------------------------
// GStageObj
// ----------------------------------------------------------------------------
GStateObj::~GStateObj() {
	if (state_ != Closed) {
		qCritical() << QString("%1 State is %2. close must be called in descendant of GStateObj").arg(metaObject()->className()).arg(int(state_));
	}
}

bool GStateObj::active() {
	return state_ == Opened;
}

bool GStateObj::open() {
	if (state_ != Closed) {
		SET_ERR(GErr::NotClosedState, QString("%1 State is %2").arg(metaObject()->className()).arg(int(state_)));
		return false;
	}
	err.clear();

	state_ = Opening;
	bool res = doOpen();
	if (!res) {
		state_ = Closing;
		doClose();
		state_ = Closed;
		return false;
	}

	state_ = Opened;

	for (QObject* obj: children()) {
		GThread* thread = dynamic_cast<GThread*>(obj);
		if (thread != nullptr) {
			if (thread->startRequired_) {
				thread->QThread::start(thread->priority_);
				thread->startRequired_ = false;
			}
		}
	}

	emit opened();
	return true;
}

bool GStateObj::close() {
	if (state_ == Closed)
		return true;

	if (!active()) {
		SET_ERR(GErr::NotOpenedState, QString("%1 State is %2").arg(metaObject()->className()).arg(int(state_)));
		return false;
	}

	state_ = Closing;
	doClose();
	state_ = Closed;
	emit closed();
	return true;
}

bool GStateObj::doOpen() {
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return false;
}

bool GStateObj::doClose() {
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return false;
}
