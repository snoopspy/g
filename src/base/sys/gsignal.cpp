#include "gsignal.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GSignal
// ----------------------------------------------------------------------------
GSignal::GSignal() {
}

GSignal::~GSignal() {
	for (Handlers::iterator it = handlers_.begin(); it != handlers_.end(); it++) {
		int signo = it.key();
		Handler handler = it.value();
		std::signal(signo, handler);
	}
}

void GSignal::signalHandler(int signo) {
	emit signaled(signo);

	Handlers::iterator it = handlers_.find(signo);
	if (it == handlers_.end()) {
		qCritical() << "can not find signal" << signo;
		return;
	}

	if (callOriginFunc_) {
		Handler handler = it.value();
		handler(signo);
	}

	if (signalOnce_) {
		Handler handler = it.value();
		std::signal(signo, handler);
		handlers_.erase(it);
	}
}

void GSignal::_signalHandler(int signo) {
	GSignal& signal = GSignal::instance();
	signal.signalHandler(signo);
}

void GSignal::setup(int signo, Handler handler) {
	if (handler == nullptr)
		handler = _signalHandler;
	Handler oldHandler = std::signal(signo, handler);
	handlers_[signo] = oldHandler;
}

void GSignal::setupAll(Handler handler) {
	setup(SIGINT, handler);
	setup(SIGILL, handler);
	setup(SIGABRT, handler);
	setup(SIGFPE, handler);
	// setup(SIGSEGV, handler); // do not touch SIGSEGV
#ifndef Q_OS_WIN
	setup(SIGTERM, handler);
	setup(SIGHUP, handler);
	setup(SIGQUIT, handler);
	setup(SIGTRAP, handler);
	setup(SIGKILL, handler);
	setup(SIGBUS, handler);
	setup(SIGSYS, handler);
	ignore(SIGPIPE); // ignore SIGPIPE which can be signaled when TCP socket operation on linux
	setup(SIGALRM, handler);
#endif
}

void GSignal::ignore(int signo) {
	std::signal(signo, SIG_IGN);
}

QString GSignal::getString(int signo) {
	QString s = "unknown";
	switch (signo) {
		case SIGINT: s = "SIGINT"; break;
		case SIGILL: s = "SIGILL"; break;
		case SIGABRT: s = "SIGABRT"; break;
		case SIGFPE: s = "SIGFPE"; break;
		case SIGSEGV: s = "SIGSEGV"; break;
		case SIGTERM: s = "SIGTERM"; break;
#ifndef Q_OS_WIN
		case SIGHUP: s = "SIGHUP"; break;
		case SIGQUIT: s = "SIGQUIT"; break;
		case SIGTRAP: s = "SIGTRAP"; break;
		case SIGKILL: s = "SIGKILL"; break;
		case SIGBUS: s = "SIGBUS"; break;
		case SIGSYS: s = "SIGSYS"; break;
		case SIGPIPE: s = "SIGPIPE"; break;
		case SIGALRM: s = "SIGALRM"; break;
		case SIGURG: s = "SIGURG"; break;
		case SIGSTOP: s = "SIGSTOP"; break;
		case SIGTSTP: s = "SIGTSTP"; break;
		case SIGCONT: s = "SIGCONT"; break;
		case SIGCHLD: s = "SIGCHLD"; break;
		case SIGTTIN: s = "SIGTTIN"; break;
		case SIGTTOU: s = "SIGTTOU"; break;
		case SIGPOLL: s = "SIGPOLL"; break;
		case SIGXCPU: s = "SIGXCPU"; break;
		case SIGXFSZ: s = "SIGXFSZ"; break;
		case SIGVTALRM: s = "SIGVTALRM"; break;
		case SIGPROF: s = "SIGPROF"; break;
		case SIGUSR1: s = "SIGUSR1"; break;
		case SIGUSR2: s = "SIGUSR2"; break;
#endif
	}
	return s;
}
