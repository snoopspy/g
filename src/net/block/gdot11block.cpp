#include "gdot11block.h"
#include "net/pdu/gdeauthhdr.h"

// ----------------------------------------------------------------------------
// GDot11Block
// ----------------------------------------------------------------------------
GDot11Block::GDot11Block(QObject* parent) : GStateObj(parent) {
}

GDot11Block::~GDot11Block() {
	close();
}

bool GDot11Block::doOpen() {
	if (writer_ == nullptr) {
		SET_ERR(GErr::ObjectIsNull, "writer must be specified");
		return false;
	}

	return true;
}

bool GDot11Block::doClose() {
	return true;
}

void GDot11Block::block(GPacket* packet) {
	if (!enabled_) return;

	GDot11ExtHdr* dot11ExtHdr = packet->dot11ExtHdr_;
	if (dot11ExtHdr == nullptr) return;

	le8_t typeSubtype = dot11ExtHdr->typeSubtype();
	if (typeSubtype != GDot11Hdr::Beacon) return;

	struct DeauthFrame {
		GRadiotapHdr radiotapHdr_;
		GDeauthHdr deauthHdr_;
	} deauthFrame;

	deauthFrame.radiotapHdr_.init();
	deauthFrame.deauthHdr_.init(dot11ExtHdr->ta());

	qDebug() << "block..."; // gilgil temp 2022.01.29
	writer_->write(GBuf(pbyte(&deauthFrame), sizeof(deauthFrame)));
	emit blocked(packet);
}

void GDot11Block::processChannelChanged(int channel) {
	qDebug() << ""; // gilgil temp 2022.01.29
	currentChannel_ = channel;
}
