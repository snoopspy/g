#include "ganypacket.h"

// ----------------------------------------------------------------------------
// GAnyPacket
// ----------------------------------------------------------------------------
GPacket* GAnyPacket::get(GPacket::Dlt dlt) {
	switch (dlt) {
		case GPacket::Eth : return &ethPacket_;
		case GPacket::Ip : return &ipPacket_;
		case GPacket::Dot11 : return &dot11Packet_;
		case GPacket::Null : return &nullPacket_;
	}
	qWarning() << QString("invalid dlt(%1)").arg(int(dlt));
	return nullptr;
}
