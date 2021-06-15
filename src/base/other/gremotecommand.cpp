#include "gremotecommand.h"

// ----------------------------------------------------------------------------
// GRemoteCommand
// ----------------------------------------------------------------------------
bool GRemoteCommand::doOpen() {
	demonClient_ = new GDemonClient(std::string(qPrintable(ip_)), port_);
	return GCommand::doOpen();
}

bool GRemoteCommand::doClose() {
	bool res = GCommand::doClose();
	if (demonClient_ != nullptr) {
		delete demonClient_;
		demonClient_ = nullptr;
	}
	return res;
}

bool GRemoteCommand::cmdExecute(QString command) {
	GDemon::CmdExecuteRes res = demonClient_->cmdExecute(qPrintable(command));
	if (!res.result_) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		return false;
	}
	return true;
}

void* GRemoteCommand::cmdStart(QString command) {
	GDemon::CmdStartRes res = demonClient_->cmdStart(qPrintable(command));
	if (res.pid_ == 0) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		return 0;
	}
	return (void*)res.pid_;
}

bool GRemoteCommand::cmdStop(void* pid) {
	if (pid == 0) return true;
	uint64_t _pid = (uint64_t)pid;
	GDemon::CmdStopRes res = demonClient_->cmdStop(_pid);
	if (!res.result_) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		return false;
	}
	return true;
}

void* GRemoteCommand::cmdStartDetached(QString command) {
	GDemon::CmdStartDetachedRes res = demonClient_->cmdStartDetached(qPrintable(command));
	if (!res.result_) {
		SET_ERR(GErr::FAIL, demonClient_->error_.data());
		return (void*)-1;
	}
	return 0;
}
