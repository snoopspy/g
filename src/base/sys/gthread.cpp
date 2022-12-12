#include "gthread.h"
#include "base/gstateobj.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GThread
// ----------------------------------------------------------------------------
void GThread::start() {
	GThreadMgr& threadMgr = GThreadMgr::instance();
	if (threadMgr.suspended_) {
		threadMgr.threads_.push_back(this);
		return;
	}
	GStateObj* stateObj = dynamic_cast<GStateObj*>(parent());
	if (stateObj == nullptr) {
		QThread::start(priority_);
		startRequired_ = false;
	} else {
		startRequired_ = true;
	}
}

bool GThread::wait(GDuration timeout) {
	bool res = QThread::wait(timeout);
	if (!res) {
		QString msg = QString("%1::wait(%2) return false").arg(metaObject()->className()).arg(timeout);
		QObject* _parent = parent();
		if (_parent != nullptr)
			msg += QString(" for (%1)").arg(_parent->metaObject()->className());
		qCritical() << msg;
		QThread::terminate();
	}
	startRequired_ = false;
	return res;
}

// ----------------------------------------------------------------------------
// GThreadMgr
// ----------------------------------------------------------------------------
void GThreadMgr::suspendStart() {
	GThreadMgr& threadMgr = instance();
	threadMgr.suspended_ = true;
}

void GThreadMgr::resumeStart() {
	GThreadMgr& threadMgr = instance();
	for (GThread* thread: threadMgr.threads_) {
		thread->QThread::start(thread->priority_);
	}
	threadMgr.threads_.clear();
	threadMgr.suspended_ = false;
}
