#pragma once

#include <map>
#include <mutex>
#include <thread>
#include <pcap.h>

#include "ethhdr.h"
#include "arphdr.h"
#include "iphdr.h"

#include "gtrace.h"

#pragma pack(push, 1)
struct EthArpPacket {
	EthHdr ethHdr_;
	ArpHdr arpHdr_;
};
typedef EthArpPacket *PEthArpPacket;
#pragma pack(pop)

struct Network {
	Ip ip_;
	Ip mask_;
	Ip gateway_;

	bool isSameLanIp(Ip ip) { return (ip_ & mask_) == (ip & mask_); }
	Ip getAdjIp(Ip ip) { return isSameLanIp(ip) ? ip : gateway_; }
};

struct IpFlowKey {
public:
	Ip sip_;
	Ip dip_;

	IpFlowKey() {}
	IpFlowKey(Ip sip, Ip dip) : sip_(sip), dip_(dip) {}

	bool operator < (const IpFlowKey& r) const {
		if (this->sip_ < r.sip_) return true;
		if (this->sip_ > r.sip_) return false;
		if (this->dip_ < r.dip_) return true;
		return false;
	}

	IpFlowKey reverse() {
		return IpFlowKey(dip_, sip_);
	}
};

struct Flow {
	Ip senderIp_;
	Mac senderMac_;
	Ip targetIp_;
	Mac targetMac_;

	EthArpPacket recoverPacket_;

	Flow() {}
	Flow(Ip senderIp, Mac senderMac, Ip targetIp, Mac targetMac);
	void makePacket(EthArpPacket* packet, Mac myMac);
};

struct FlowMap : std::map<IpFlowKey, Flow> {
protected:
	std::mutex m_;
public:
	void lock() { m_.lock(); }
	void unlock() { m_.unlock(); }
};

struct ArpRecover {
	ArpRecover();
	virtual ~ArpRecover();

	int interval_{10}; // 10 seconds
	Network network_;
	Mac myMac_;
	std::string intfName_;
	FlowMap flowMap_;

	std::thread* sendThread_{nullptr};
	static void _sendThread(ArpRecover* arpRecover);
	void sendThread();

	bool open();
	bool exec();
	bool close();

protected:
	bool active_{false};
	pcap_t* pcap_{nullptr};
};
