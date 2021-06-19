// ----------------------------------------------------------------------------
//
// G Library
//
// http://gilgil.net
//
// Copyright (c) Gilbert Lee All rights reserved
//
// ----------------------------------------------------------------------------

#pragma once

#include "gdemon.h"

// ----------------------------------------------------------------------------
// GDemonClient
// ----------------------------------------------------------------------------
struct GDemonClient : GDemon {
	GDemonClient(std::string ip, uint16_t port);
	~GDemonClient() override;

	std::string error_;

	std::string ip_;
	uint16_t port_;

	int sd_{0};

	char readBuffer_[MaxBufSize]; // used in pcapRead

	// connection
	bool connect();
	bool disconnect();

	// command
	CmdExecuteRes cmdExecute(std::string command);
	CmdStartRes cmdStart(std::string command);
	CmdStopRes cmdStop(uint64_t pid);
	CmdStartDetachedRes cmdStartDetached(std::string command);

	// network information
	GetInterfaceListRes getInterfaceList();
	GetRtmRes getRtm();

	// pcap
	PcapOpenRes pcapOpen(std::string filter, std::string intfName, int32_t snapLen, int32_t flags, int32_t readTimeout, int32_t waitTimeout, bool captureThread);
	void pcapClose();
	PcapRead pcapRead();
	void pcapWrite(PcapWrite write);

	//

	static GDemonClient& instance(std::string ip, uint16_t port);
};
