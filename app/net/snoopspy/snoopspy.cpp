#include <csignal>
#include <GApp>
#include <GGraphWidget>
#include "myfactory.h"

GGraphWidget* _graphWidget{nullptr};
int exec(GApp* a, GGraph* graph, GPluginFactory* pluginFactory) {
	Q_ASSERT(graph != nullptr);

	if (pluginFactory == nullptr) {
		pluginFactory = &GPluginFactory::instance();
	}
	graph->setFactory(pluginFactory);

	_graphWidget = new GGraphWidget;
	_graphWidget->setGraph(graph);

	QJsonObject jo = GJson::loadFromFile();
	jo["graphWidget"] >> *_graphWidget;

	_graphWidget->update();
	_graphWidget->show();
	int res = a->exec();

	jo["graphWidget"] << *_graphWidget;
	delete _graphWidget;
	_graphWidget = nullptr;

	GJson::saveToFile(jo);
	return res;
}

#ifdef Q_OS_LINUX
void signalHandler(int signo) {
	const char* signal = "unknown";
	switch (signo) {
		case SIGINT: signal = "SIGINT"; break;
		case SIGILL: signal = "SIGILL"; break;
		case SIGABRT: signal = "SIGABRT"; break;
		case SIGFPE: signal = "SIGFPE"; break;
		case SIGSEGV: signal = "SIGSEGV"; break;
		case SIGTERM: signal = "SIGTERM"; break;
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
	}
	char* msg = strsignal(signo);
	qCritical() << QString("signo=%1(%2) %3").arg(signal).arg(signo).arg(msg);
	if (signo == SIGSEGV)
		exit(-1);
	qDebug() << "bef _graphWidget->close()";
	if (_graphWidget != nullptr)
		_graphWidget->close();
	qDebug() << "aft _graphWidget->close()";
}

void prepareSignal() {
	std::signal(SIGINT, signalHandler);
	std::signal(SIGILL, signalHandler);
	std::signal(SIGABRT, signalHandler);
	std::signal(SIGFPE, signalHandler);
	std::signal(SIGSEGV, signalHandler);
	std::signal(SIGTERM, signalHandler);
	std::signal(SIGHUP, signalHandler);
	std::signal(SIGQUIT, signalHandler);
	std::signal(SIGTRAP, signalHandler);
	std::signal(SIGKILL, signalHandler);
	std::signal(SIGBUS, signalHandler);
	std::signal(SIGSYS, signalHandler);
	std::signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE which can be signaled when TCP socket operation on linux
	std::signal(SIGALRM, signalHandler);
}
#endif // Q_OS_LINUX

std::string getDir(std::string argv) {
	ssize_t i = argv.length() - 1;
	while (i >= 0) {
		char& ch = argv.at(i);
		if (ch  == '/' || ch == '\\')
			return argv.substr(0, i + 1);
		i--;
	}
	return "/";
}

#include <unistd.h> // for chdir

int main(int argc, char *argv[]) {
#ifndef Q_OS_WIN
	chdir(getDir(argv[0]).data());
#endif // Q_OS_WIN
#ifdef Q_OS_LINUX
	prepareSignal();
#endif // #ifdef Q_OS_LINUX
	GApp a(argc, argv);
	GGraph graph;
	MyFactory factory;
	return exec(&a, &graph, &factory);
}
