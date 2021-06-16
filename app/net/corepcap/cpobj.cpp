#include "cpobj.h"

LObj::~LObj() {
	if (state_ != Closed) {
		GTRACE("State is %d. close must be called in descendant of LObj", state_);
	}
}

bool LObj::open() {
	if (state_ != Closed) {
		GTRACE("State is %d", state_);
		return false;
	}

	state_ = Opening;
	bool res = doOpen();
	if (!res) {
		state_ = Closing;
		doClose();
		state_ = Closed;
		return false;
	}

	state_ = Opened;
	return true;
}

bool LObj::close() {
	if (state_ == Closed)
		return true;

	if (!active()) {
		GTRACE("State is %d", state_);
		return false;
	}

	state_ = Closing;
	doClose();
	state_ = Closed;
	return true;
}

bool LObj::doOpen() {
	GTRACE("virtual function call");
	return false;
}

bool LObj::doClose() {
	GTRACE("virtual function call");
	return false;
}

LPacket::Result LObj::read(LPacket* packet) {
	(void)packet;
	GTRACE("virtual function call");
	return LPacket::Fail;
}

LPacket::Result LObj::write(LPacket* packet) {
	(void)packet;
	GTRACE("virtual function call");
	return LPacket::Fail;
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(Obj, openCloseTest) {
	LObj obj;
	EXPECT_FALSE(obj.open());
	EXPECT_TRUE(obj.close());
	EXPECT_EQ(obj.read(nullptr), LPacket::Fail);
	EXPECT_EQ(obj.write(nullptr), LPacket::Fail);
}

#endif // GTEST
