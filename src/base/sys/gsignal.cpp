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
	setup(SIGSEGV, handler);
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
	QString signal = "unknown";
	switch (signo) {
		case SIGINT: signal = "SIGINT"; break;
		case SIGILL: signal = "SIGILL"; break;
		case SIGABRT: signal = "SIGABRT"; break;
		case SIGFPE: signal = "SIGFPE"; break;
		case SIGSEGV: signal = "SIGSEGV"; break;
		case SIGTERM: signal = "SIGTERM"; break;
#ifndef Q_OS_WIN
		case SIGHUP: signal = "SIGHUP"; break;
		case SIGQUIT: signal = "SIGQUIT"; break;
		case SIGTRAP: signal = "SIGTRAP"; break;
		case SIGKILL: signal = "SIGKILL"; break;
		case SIGBUS: signal = "SIGBUS"; break;
		case SIGSYS: signal = "SIGSYS"; break;
		case SIGPIPE: signal = "SIGPIPE"; break;
		case SIGALRM: signal = "SIGALRM"; break;
		case SIGURG: signal = "SIGURG"; break;
		case SIGSTOP: signal = "SIGSTOP"; break;
		case SIGTSTP: signal = "SIGTSTP"; break;
		case SIGCONT: signal = "SIGCONT"; break;
		case SIGCHLD: signal = "SIGCHLD"; break;
		case SIGTTIN: signal = "SIGTTIN"; break;
		case SIGTTOU: signal = "SIGTTOU"; break;
		case SIGPOLL: signal = "SIGPOLL"; break;
		case SIGXCPU: signal = "SIGXCPU"; break;
		case SIGXFSZ: signal = "SIGXFSZ"; break;
		case SIGVTALRM: signal = "SIGVTALRM"; break;
		case SIGPROF: signal = "SIGPROF"; break;
		case SIGUSR1: signal = "SIGUSR1"; break;
		case SIGUSR2: signal = "SIGUSR2"; break;
#endif
	}
	return signal;
}
