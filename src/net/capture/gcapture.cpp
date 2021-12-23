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
		// ----- by gilgil 2009.08.31 -----
		//
		// There can be a case that even if thread starts,
		// state remains not not Opened(still Opening) state.
		// So set state_ into Opened before open thread.
		//
		state_ = Opened;
		thread_.start();
		// --------------------------------
	}
	return true;
}

bool GCapture::doClose() {
	if (!enabled_) return true;

	bool res = true;
	if (autoRead_) {
		thread_.quit();
		res = thread_.wait();
	}
	return res;
}

GPacket::Result GCapture::read(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}

GPacket::Result GCapture::write(GBuf buf) {
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

void GCapture::run() {
	qDebug() << "GCapture::run() beg"; // gilgil temp 2017.11.25

	GEthPacket ethPacket;
	GIpPacket ipPacket;
	GDot11Packet dot11Packet;
	GNullPacket nullPacket;

	GPacket* packet;
	switch(dlt()) {
		case GPacket::Eth: packet = &ethPacket; break;
		case GPacket::Ip: packet = &ipPacket; break;
		case GPacket::Dot11: packet = &dot11Packet; break;
		case GPacket::Null: default: packet = &nullPacket; break;
	}

	PathType pt = pathType();
	while (active()) {
		GPacket::Result res = read(packet);
		if (res == GPacket::None) continue;
		if (res == GPacket::Eof || res == GPacket::Fail) break;
		emit captured(packet);
		if (pt == InPath) {
			if (packet->ctrl.block_)
				res = drop(packet);
			else
				res = relay(packet);
			if (res != GPacket::Ok) {
				qWarning() << "relay return " << int(res);
			}
		}
	}
	qDebug() << "GCapture::run() end"; // gilgil temp 2017.11.25
	emit closed();
}
