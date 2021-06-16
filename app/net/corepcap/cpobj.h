#pragma once

#include <string>
#include "gtrace.h"
#include "cppacket.h"

struct LObj {
public:
	typedef enum {
		Closed,
		Opening,
		Opened,
		Closing
	} State;

	bool active() { return state_ == Opened; }

protected:
	State state_{Closed};

public:
	LObj() {}
	virtual ~LObj();

public:
	virtual bool open();
	virtual bool close();
	virtual LPacket::Result read(LPacket* packet);
	virtual LPacket::Result write(LPacket* packet);

protected:
	virtual bool doOpen();
	virtual bool doClose();
};
