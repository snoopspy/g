#pragma once

#include <string>
#include "gtrace.h"
#include "cppacket.h"

struct CObj {
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
	CObj() {}
	virtual ~CObj();

public:
	virtual bool open();
	virtual bool close();
	virtual CPacket::Result read(CPacket* packet);
	virtual CPacket::Result write(CPacket* packet);

protected:
	virtual bool doOpen();
	virtual bool doClose();
};
