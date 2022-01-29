#include "gdot11block.h"
#include "net/pdu/gdeauthhdr.h"
#include "net/pdu/gbeaconhdr.h"

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
	writer_->write(GBuf(pbyte(&deauthFrame), sizeof(deauthFrame)));

	if (debugLog_) {
		GBeaconHdr* beaconHdr = GBeaconHdr::check(dot11ExtHdr, packet->buf_.size_);
		QString ssid;
		int channel = 0;
		if (beaconHdr != nullptr) {
			GBeaconHdr::Tag* tag = beaconHdr->firstTag();
			void* end = packet->buf_.data_ + packet->buf_.size_;
			while(tag < end) {
				le8_t num = tag->num_;
				switch (num) {
					case GBeaconHdr::TagSsidParameterSet:
						ssid = QByteArray(pchar(tag->value()), tag->len_);
						break;
					case GBeaconHdr::TagDsParameterSet: {
						char* p = pchar(tag->value());
						channel = *p;
						break;
					}
				}
				if (ssid != "" && channel != 0)
					break;
				tag = tag->next();
				if (tag == nullptr) break;
			}
			if (ssid != "")
				qDebug() << ssid << channel;
		}
	}

	emit blocked(packet);
}

void GDot11Block::processChannelChanged(int channel) {
	qDebug() << ""; // gilgil temp 2022.01.29
	currentChannel_ = channel;
}
