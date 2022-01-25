#include "gcommand.h"

// ----------------------------------------------------------------------------
// GCommandItem
// ----------------------------------------------------------------------------
GCommandItem::GCommandItem(QObject* parent) : GObj(parent) {
}

GCommandItem::GCommandItem(QObject* parent, QStringList commands, GCommandType commandType) :
	GObj(parent), commands_(commands), commandType_(commandType) {
}

GCommandItem::~GCommandItem() {
	int count = processList_.count();
	if (count != 0) {
		qCritical() << "process list count is " << count;
	}
}

// ----------------------------------------------------------------------------
// GCommand
// ----------------------------------------------------------------------------
bool GCommand::doOpen() {
#ifdef Q_OS_ANDROID
	demonClient_ = new GDemonClient("127.0.0.1", GDemon::DefaultPort);
#endif

	bool res = true;

	for (GObj* obj: openCommands_) {
		GCommandItem* item = PCommandItem(obj);
		for (QString& command: item->commands_) {
			if (command == "") continue;
			qDebug() << command;

			void* pid = nullptr;
			switch (item->commandType_) {
				case GCommandItem::Execute: {
					res = cmdExecute(command);
					break;
				}
				case GCommandItem::StartStop: {
					pid = cmdStart(command);
					if (pid == (void*)-1) res = false;
					else item->processList_.push_back(pid);
					break;
				}
				case GCommandItem::StartDetach: {
					pid = cmdStartDetached(command);
					if (pid == (void*)-1) res = false;
					else item->processList_.push_back(pid);
					break;
				}
			}
			if (!res) break;
		}
	}

	if (!res) {
		stopCommands();
#ifdef Q_OS_ANDROID
		if (demonClient_ != nullptr) {
			delete demonClient_;
			demonClient_ = nullptr;
		}
#endif
	}

	return res;
}

bool GCommand::doClose() {
	bool res = true;

	stopCommands();

	for (GObj* obj: closeCommands_) {
		GCommandItem* item = PCommandItem(obj);
		for (QString& command: item->commands_) {
			if (command == "") continue;
			qDebug() << command;

			switch (item->commandType_) {
				case GCommandItem::Execute: {
					res = cmdExecute(command);
					break;
				}
				case GCommandItem::StartStop: {
					QString msg = QString("Invalid commandType %1 for '%2'").arg(item->commandType_).arg(command);
					SET_ERR(GErr::Fail, msg);
					res = false;
					break;
				}
				case GCommandItem::StartDetach:{
					QString msg = QString("Invalid commandType %1 for '%2'").arg(item->commandType_).arg(command);
					SET_ERR(GErr::Fail, msg);
					res = false;
					break;
				}
			}
			if (!res) break;
		}
	}

#ifdef Q_OS_ANDROID
	if (demonClient_ != nullptr) {
		delete demonClient_;
		demonClient_ = nullptr;
	}
#endif

	return res;
}

bool GCommand::split(QString command, QString& program, QStringList &arguments) {
	arguments = QProcess::splitCommand(command);
	if (arguments.count() == 0) {
		SET_ERR(GErr::Fail, "count is zero");
		return false;
	}
	program = arguments.at(0);
	arguments.removeAt(0);
	return true;
}

bool GCommand::stopCommands() {
	bool res = true;
	for (GObj* obj: openCommands_) {
		GCommandItem* item = PCommandItem(obj);
		for (QList<void*>::iterator it = item->processList_.begin(); it != item->processList_.end();) {
			void* pid = *it;
			if (!cmdStop(pid))
				res = false;
			it = item->processList_.erase(it);
		}
	}
	return res;
}

#ifdef Q_OS_ANDROID
bool GCommand::cmdExecute(QString command) {
	if (demonClient_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "demonClinet is null");
		return false;
	}

	GDemon::CmdExecuteRes res = demonClient_->cmdExecute(qPrintable(command));
	if (!res.result_) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		return false;
	}
	return true;
}

void* GCommand::cmdStart(QString command) {
	if (demonClient_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "demonClinet is null");
		return 0;
	}

	GDemon::CmdStartRes res = demonClient_->cmdStart(qPrintable(command));
	if (res.pid_ == 0) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		return 0;
	}
	return (void*)res.pid_;
}

bool GCommand::cmdStop(void* pid) {
	if (demonClient_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "demonClinet is null");
		return false;
	}

	if (pid == 0) return true;
	uint64_t _pid = (uint64_t)pid;
	GDemon::CmdStopRes res = demonClient_->cmdStop(_pid);
	if (!res.result_) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		return false;
	}
	return true;
}

void* GCommand::cmdStartDetached(QString command) {
	if (demonClient_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "demonClinet is null");
		return 0;
	}

	GDemon::CmdStartDetachedRes res = demonClient_->cmdStartDetached(qPrintable(command));
	if (!res.result_) {
		SET_ERR(GErr::Fail, demonClient_->error_.data());
		return (void*)-1;
	}
	return 0;
}
#else
bool GCommand::cmdExecute(QString command) {
	QString program;
	QStringList arguments;
	if (!split(command, program, arguments)) return false;
	int res = QProcess::execute(program, arguments);
	if (res != 0) {
		SET_ERR(GErr::Fail, QString("QProcess::execute(%1, %2) return %3").arg(program, arguments.join(' ')).arg(res));
		return false;
	}
	return true;
}

void* GCommand::cmdStart(QString command) {
	QString program;
	QStringList arguments;
	if (!split(command, program, arguments)) return (void*)-1;
	QProcess* process = new QProcess;
	process->start(program, arguments);
	return process;
}

bool GCommand::cmdStop(void* pid) {
	if (pid == nullptr) return true;
	QProcess* process = reinterpret_cast<QProcess*>(pid);
	if (dynamic_cast<QProcess*>(process) == nullptr) {
		SET_ERR(GErr::Fail, QString("invalid process pointer(0x%1)").arg(QString::number(uintptr_t(pid), 16)));
		return false;
	}
	delete process;
	return true;
}

void* GCommand::cmdStartDetached(QString command) {
	QString program;
	QStringList arguments;
	if (!split(command, program, arguments)) return (void*)-1;
	QProcess* process = new QProcess;
	bool res = process->startDetached(program, arguments);
	if (!res) {
		SET_ERR(GErr::Fail, process->errorString());
		delete process;
		return (void*)-1;
	}
	return process;
}
#endif

