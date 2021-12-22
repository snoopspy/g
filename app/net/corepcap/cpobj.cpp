#include "cpobj.h"

CObj::~CObj() {
	if (state_ != Closed) {
		GTRACE("State is %d. close must be called in descendant of CObj", state_);
	}
}

bool CObj::open() {
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

bool CObj::close() {
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

bool CObj::doOpen() {
	GTRACE("virtual function call");
	return false;
}

bool CObj::doClose() {
	GTRACE("virtual function call");
	return false;
}

CPacket::Result CObj::read(CPacket* packet) {
	(void)packet;
	GTRACE("virtual function call");
	return CPacket::Fail;
}

CPacket::Result CObj::write(CPacket* packet) {
	(void)packet;
	GTRACE("virtual function call");
	return CPacket::Fail;
}

#ifdef GTEST
#include <gtest/gtest.h>

TEST(Obj, openCloseTest) {
	CObj obj;
	EXPECT_FALSE(obj.open());
	EXPECT_TRUE(obj.close());
	EXPECT_EQ(obj.read(nullptr), CPacket::Fail);
	EXPECT_EQ(obj.write(nullptr), CPacket::Fail);
}

#endif // GTEST
