#include "gblock.h"

// ----------------------------------------------------------------------------
// GBlock
// ----------------------------------------------------------------------------
void GBlock::block(GPacket* packet) {
	if (!enabled_) return;
	packet->ctrl_.block_ = true;
	emit blocked(packet);
}

void GBlock::unblock(GPacket* packet) {
	if (!enabled_) return;
	packet->ctrl_.block_ = false;
	emit unblocked(packet);
}
