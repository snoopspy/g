#include "gcapture.h"

// ----------------------------------------------------------------------------
// GCapture
// ----------------------------------------------------------------------------
GCapture::~GCapture() {
	close();
}

bool GCapture::doOpen() {
	if (!enabled_) return true;

	if (autoRead_) {
		captureThread_.start();
	}
	return true;
}

bool GCapture::doClose() {
	if (!enabled_) return true;

	bool res = true;
	if (autoRead_) {
		captureThread_.quit();
		res = captureThread_.wait();
	}
	return res;
}

GPacket::Result GCapture::read(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}

GPacket::Result GCapture::writeBuf(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}

GPacket::Result GCapture::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}

GPacket::Result GCapture::relay(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}

GPacket::Result GCapture::drop(GPacket* packet) {
	(void)packet;
	return GPacket::Ok;
}

void GCapture::captureRun() {
	qDebug() << "beg";
	GPacket* packet = anyPacket_.get(dlt());
	PathType pt = pathType();
	while (active()) {
		GPacket::Result res = read(packet);
		if (res == GPacket::None) continue;
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		emit captured(packet);
		if (pt == InPath) {
			if (packet->ctrl_.block_)
				res = drop(packet);
			else
				res = relay(packet);
			if (res != GPacket::Ok) {
				qWarning() << "relay return " << int(res);
			}
		}
	}
	qDebug() << "end";
	emit closed();
}

// ----------------------------------------------------------------------------
// GCapture::CaptureThread
// ----------------------------------------------------------------------------
void GCapture::CaptureThread::run() {
	GCapture* capture = dynamic_cast<GCapture*>(parent());
	Q_ASSERT(capture != nullptr);
	capture->captureRun();
}
