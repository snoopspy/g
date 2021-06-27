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

#include <csignal>
#include <list>
#include <mutex>
#include <thread>
#include <vector>

#include "gdemon.h"
#include "gprocess.h"

// ----------------------------------------------------------------------------
// GDemonServer
// ----------------------------------------------------------------------------
struct GDemonSession;
struct GDemonServer: GDemon {
	GDemonServer();
	~GDemonServer() override;

	bool start(uint16_t port = DefaultPort);
	void exec();
	void stop();
	void wait();

	int accept_{0};

	struct GDemonSessionList : std::list<GDemonSession*> {
	protected:
		std::mutex m_;
	public:
		void lock() { m_.lock(); }
		void unlock() { m_.unlock(); }
	} sessions_;
};

// ----------------------------------------------------------------------------
// GDemonSession
// ----------------------------------------------------------------------------
struct GDemonCommand;
struct GDemonNetwork;
struct GDemonPcap;
struct GDemonNetFilter;
struct GDemonSession : GDemon {
	GDemonSession(GDemonServer* server);
	~GDemonSession() override;

	GDemonServer* server_;
	int sd_{0};

	char* recvBuf_{nullptr};
	char* sendBuf_{nullptr}; GSpinLock sendBufLock_;

	static void _run(GDemonServer* server, int new_sd);
	void run();

	GDemonCommand* command_{nullptr};
	GDemonNetwork* network_{nullptr};
	GDemonPcap* pcap_{nullptr};
	GDemonNetFilter* nf_{nullptr};
};

// ----------------------------------------------------------------------------
// GDemonCommand
// ----------------------------------------------------------------------------
struct GDemonCommand : GDemon {
	GDemonCommand(GDemonSession* session);
	~GDemonCommand() override;

	GDemonSession* session_;

	bool processCmdExecute(pchar buf, int32_t size);
	bool processCmdStart(pchar buf, int32_t size);
	bool processCmdStop(pchar buf, int32_t size);
	bool processCmdStartDetached(pchar buf, int32_t size);
};

// ----------------------------------------------------------------------------
// GDemonNetwork
// ----------------------------------------------------------------------------
struct GDemonNetwork : GDemon {
	GDemonNetwork(GDemonSession* session);
	~GDemonNetwork() override;

	GDemonSession* session_;

	static bool getMac(char* devName, uint8_t* mac);
	bool processGetInterfaceList(pchar buf, int32_t size);
	bool processGetRtm(pchar buf, int32_t size);

protected:
	static bool checkA(char* buf, RtmEntry* entry);
	static bool checkB(char* buf, RtmEntry* entry);
	static bool checkC(char* buf, RtmEntry* entry);
	static bool checkD(char* buf, RtmEntry* entry);
	static bool decodeCidr(std::string cidr, uint32_t* dst, uint32_t* mask);
	static uint32_t numberToMask(int number);
};

// ----------------------------------------------------------------------------
// GDemonPcap
// ----------------------------------------------------------------------------
struct GDemonPcap : GDemon {
	GDemonPcap(GDemonSession* session);
	~GDemonPcap() override;

	GDemonSession* session_;
	bool active_{false};
	std::thread* thread_{nullptr};
	std::string client_;
	pcap_t* pcap_{nullptr};
	int waitTimeout_;

	PcapOpenRes open(PcapOpenReq req);
	void close();
	static void _run(GDemonPcap* pcap);
	void run();

	bool processPcapOpen(pchar buf, int32_t size);
	bool processPcapClose(pchar buf, int32_t size);
	bool processPcapWrite(pchar buf, int32_t size);
};

// ----------------------------------------------------------------------------
// GDemonNetFilter
// ----------------------------------------------------------------------------
struct GDemonNetFilter : GDemon {
	GDemonNetFilter(GDemonSession* session);
	~GDemonNetFilter() override;

	GDemonSession* session_;
	bool active_{false};
	std::thread* thread_{nullptr};
	std::string client_;
	struct nfq_handle* h_{nullptr};
	struct nfq_q_handle* qh_{nullptr};
	int fd_{0};
	char* recvBuf_{nullptr};

	NfOpenRes open(NfOpenReq req);
	void close();
	static void _run(GDemonNetFilter* nf);
	void run();

	bool processNfOpen(pchar buf, int32_t size);
	bool processNfClose(pchar buf, int32_t size);
	bool processNfVerdict(pchar buf, int32_t size);

protected:
	uint32_t id_{0};
	char* packet_{nullptr};
	static int _callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg, struct nfq_data* nfad, void* data);
};
