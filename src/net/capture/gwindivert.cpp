#include "gwindivert.h"
#ifdef Q_OS_WIN

// ----------------------------------------------------------------------------
// GWinDivertLib
// ----------------------------------------------------------------------------
struct GWinDivertLib
{
	QLibrary* lib_;

public:
	typedef HANDLE (*WinDivertOpenFunc)(
		__in const char *filter,
		__in WINDIVERT_LAYER layer,
		__in INT16 priority,
		__in UINT64 flags);
	WinDivertOpenFunc WinDivertOpen{nullptr};

	typedef BOOL (*WinDivertCloseFunc)(
		__in HANDLE handle);
	WinDivertCloseFunc WinDivertClose{nullptr};

	typedef BOOL (*WinDivertSetParamFunc)(
		__in HANDLE handle,
		__in WINDIVERT_PARAM param,
		__in UINT64 value);
	WinDivertSetParamFunc WinDivertSetParam{nullptr};

	typedef BOOL (*WinDivertRecvFunc)(
		__in HANDLE handle,
		__out_opt VOID *pPacket,
		__in UINT packetLen,
		__out_opt UINT *pRecvLen,
		__out_opt WINDIVERT_ADDRESS *pAddr);
	WinDivertRecvFunc WinDivertRecv{nullptr};

	typedef BOOL (*WinDivertSendFunc)(
		__in HANDLE handle,
		__in const VOID *pPacket,
		__in UINT packetLen,
		__out_opt UINT *pSendLen,
		__in const WINDIVERT_ADDRESS *pAddr);
	WinDivertSendFunc WinDivertSend{nullptr};

public:
	bool ok{false};

private: // singleton
	GWinDivertLib() {
		lib_ = new QLibrary("WinDivert.dll");
		if (!lib_->load()) {
			QString msg = QString("load(\"WinDivert.dll\") return false error=%1(%2)").arg(lib_->errorString());
			qCritical() << msg;
		}
		if ((WinDivertOpen = WinDivertOpenFunc(lib_->resolve("WinDivertOpen"))) == nullptr)
			qCritical() << "can not find WinDivertOpen";
		if ((WinDivertClose = WinDivertCloseFunc(lib_->resolve("WinDivertClose"))) == nullptr)
			qCritical() << "can not find WinDivertClose";
		if ((WinDivertSetParam = WinDivertSetParamFunc(lib_->resolve("WinDivertSetParam"))) == nullptr)
			qCritical() << "can not find WinDivertSetParam";
		if ((WinDivertRecv = WinDivertRecvFunc(lib_->resolve("WinDivertRecv"))) == nullptr)
			qCritical() << "can not find WinDivertRecv";
		if ((WinDivertSend = WinDivertSendFunc(lib_->resolve("WinDivertSend"))) == nullptr)
			qCritical() << "can not find WinDivertSend";
		ok = true;
	}

	virtual ~GWinDivertLib() {
		if (lib_ != nullptr) {
			lib_->unload();
			delete lib_;
			lib_ = nullptr;
		}
	}

public:
	static GWinDivertLib& instance() {
		static GWinDivertLib winDivertLib;
		return winDivertLib;
	}
};

// ----------------------------------------------------------------------------
// GWinDivert
// ----------------------------------------------------------------------------
GWinDivert::GWinDivert(QObject* parent) : GCapture(parent) {
	GWinDivertLib::instance();
}

GWinDivert::~GWinDivert() {
	close();
}

bool GWinDivert::doOpen() {
	if (!enabled_) return true;

	GWinDivertLib& lib = GWinDivertLib::instance();
	if (!lib.ok) {
		SET_ERR(GErr::FAIL, lib.lib_->errorString())
		return false;
	}

	handle_ = lib.WinDivertOpen(qPrintable(filter_), WINDIVERT_LAYER(layer_), priority_, flags_);
	if (handle_ == INVALID_HANDLE_VALUE) {
		DWORD lastError = GetLastError();

		QString msg;
		switch (lastError) {
			case ERROR_INVALID_PARAMETER: msg = "ERROR_INVALID_PARAMETER"; break;
			case ERROR_FILE_NOT_FOUND: msg = "ERROR_FILE_NOT_FOUND"; break;
			case ERROR_ACCESS_DENIED: msg = "ERROR_ACCESS_DENIED"; break;
			default: msg = QString("unknown error %1").arg(lastError); break;
		}
		SET_ERR(GErr::FAIL, QString("error in WinDivertOpen %1").arg(msg));
		return false;
	}

	if (!lib.WinDivertSetParam(handle_, WINDIVERT_PARAM_QUEUE_LENGTH, queueLen_)) {
		DWORD lastError = GetLastError();
		QString msg = QString("error in DivertSetParam(DIVERT_PARAM_QUEUE_LEN) %1").arg(lastError);
		SET_ERR(GErr::FAIL, msg);
		return false;
	}

	if (!lib.WinDivertSetParam(handle_, WINDIVERT_PARAM_QUEUE_TIME, queueTime_)) {
		DWORD lastError = GetLastError();
		QString msg = QString("error in DivertSetParam(WINDIVERT_PARAM_QUEUE_TIME) %1").arg(lastError);
		SET_ERR(GErr::FAIL, msg);
		return false;
	}

	Q_ASSERT(pktBuf_ == nullptr);
	pktBuf_ = new gbyte[bufSize_];

	return GCapture::doOpen();
}

bool GWinDivert::doClose() {
	if (!enabled_) return true;

	GWinDivertLib& lib = GWinDivertLib::instance();
	if (!lib.ok) {
		SET_ERR(GErr::FAIL, lib.lib_->errorString());
		return false;
	}

	if (!lib.WinDivertClose(handle_)) {
		DWORD lastError = GetLastError();
		QString msg = QString("WinDivertClose return FALSE last error=%1(0x%2)").arg(lastError).arg(QString::number(lastError, 16));
		SET_ERR(GErr::FAIL, msg);
	}
	handle_ = nullptr;

	if (pktBuf_ != nullptr) {
		delete[] pktBuf_;
		pktBuf_ = nullptr;
	}

	return GCapture::doClose();
}

GPacket::Result GWinDivert::read(GPacket* packet) {
	GWinDivertLib& lib = GWinDivertLib::instance();
	Q_ASSERT(lib.ok);

	packet->clear();
	UINT recvLen;
	BOOL res = lib.WinDivertRecv(handle_, pktBuf_, GPacket::MaxBufSize, &recvLen, &windivertAddress_);
	if (!res) {
		DWORD lastError = GetLastError();
		QString msg = QString("WinDivertRecv return FALSE last error=%1(0x%2)").arg(lastError).arg(QString::number(lastError, 16));
		SET_ERR(GErr::FAIL, msg);
		return GPacket::Fail;
	}
	packet->buf_.data_ = pktBuf_;
	packet->buf_.size_ = size_t(recvLen);
	if (int(recvLen) > bufSize_) {
		qWarning() << QString("copyLen(%1) > bufSize_(%2)").arg(recvLen).arg(bufSize_);
		return GPacket::None;
	}

	if (autoParse_) packet->parse();

	if (correctIpChecksum_) {
		GIpHdr* ipHdr = packet->ipHdr_;
		if (ipHdr != nullptr) {
			ipHdr->sum_ = ntohs(GIpHdr::calcChecksum(ipHdr));
		}
	}
	return GPacket::Ok;
}

GPacket::Result GWinDivert::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::FAIL, "not supported");
	return GPacket::Fail;
}

GPacket::Result GWinDivert::write(GPacket* packet) {
	GWinDivertLib& lib = GWinDivertLib::instance();
	Q_ASSERT(lib.ok);

	UINT sendLen;
	BOOL res = lib.WinDivertSend(handle_, packet->buf_.data_, UINT(packet->buf_.size_), &sendLen, &windivertAddress_);
	if (!res) {
		DWORD lastError = GetLastError();
		QString msg = QString("WinDivertSend return FALSE last error=%1(0x%2)").arg(lastError).arg(QString::number(lastError, 16));
		SET_ERR(GErr::FAIL, msg);
		return GPacket::Fail;
	}
	return GPacket::Ok;
}

GPacket::Result GWinDivert::relay(GPacket* packet) {
	return write(packet);
}

#endif
