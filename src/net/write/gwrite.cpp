#include "gwrite.h"

// ----------------------------------------------------------------------------
// GWrite
// ----------------------------------------------------------------------------
GPacket::Result GWrite::writeBuf(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}

GPacket::Result GWrite::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::VirtualFunctionCall, "virtual function call");
	return GPacket::Fail;
}
