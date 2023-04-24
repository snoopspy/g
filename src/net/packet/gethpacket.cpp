#include "gethpacket.h"

// ----------------------------------------------------------------------------
// GEthPacket
// ----------------------------------------------------------------------------
void GEthPacket::parse() {
#ifdef _DEBUG
	if (parsed_) {
		qCritical() << "already parsed";
		return;
	}
#endif // _DEBUG
	ethHdr_ = PEthHdr(buf_.data_);
	switch (ethHdr_->type()) {
		case GEthHdr::Ip4: {
		// case GEthHdr::Ip6: // gilgil temp 2023.04.24
			GBuf backup = buf_;
			buf_.data_ += sizeof(GEthHdr);
			buf_.size_ -= sizeof(GEthHdr);
			GIpPacket::parse();
			buf_ = backup;
			break;
		}
		case GEthHdr::Arp:
			arpHdr_ = PArpHdr(buf_.data_ + sizeof(GEthHdr));
			break;
	}
#ifdef _DEBUG
	parsed_ = true;
#endif // _DEBUG
}
