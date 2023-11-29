#include "gttlreplace.h"

// ----------------------------------------------------------------------------
// GTtlReplace
// ----------------------------------------------------------------------------
GTtlReplace::GTtlReplace(QObject* parent) : GStateObj(parent) {
}

GTtlReplace::~GTtlReplace() {
	close();
}

bool GTtlReplace::doOpen() {
	return true;
}

bool GTtlReplace::doClose() {
	return true;
}

void GTtlReplace::replace(GPacket* packet) {
	if (!enabled_) return;

	GIpHdr* ipHdr = packet->ipHdr_;
	if (ipHdr == nullptr) {
		emit notReplaced(packet);
		return;
	}

	if (log_)
		qInfo() << QString("replaced(%1 > %2").arg(ipHdr->ttl_).arg(ttl_);

	ipHdr->ttl_ = ttl_;
	correctChecksum_.correct(packet);
	packet->ctrl_.changed_ = true;
	emit replaced(packet);
}
