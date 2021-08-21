#include "gradiotaphdr.h"

GRadiotapHdr* GRadiotapHdr::check(gbyte* p, uint32_t size) {
	GRadiotapHdr* radiotapHdr = PRadiotapHdr(p);
	le16_t len = radiotapHdr->len_;
	// ----- gilgil temp 2020.11.06 -----
	/*
	if (len != sizeof(RadiotapHdr) && len != 18 && len != 13) { // gilgil temp
		char* end = p + size;
		GTRACE("invalid radiotap header len %u %p %p", len, (void*)radiotapHdr, (void*)end);
		dump(puchar(p), size);
		return nullptr;
	}
	*/
	// ----------------------------------
	if (len < sizeof(GRadiotapHdr) || len > size) return nullptr;
	return radiotapHdr;
}

// ----- gilgil temp 2020.11.07 -----
/*
#include <pcap.h>
#include "gqosnullhdr.h"
bool RadiotapHdr::LenghChecker::check(std::string interface) {
	if (checked_) return true;

	GTRACE("RadiotapHdr Length Checking...");
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* handle = pcap_open_live(interface.c_str(), BUFSIZ, 1, 1, errbuf);
	if (handle == nullptr) {
		GTRACE("pcap_open_live(%s) return null - %s", interface.c_str(), errbuf);
		return false;
	}

	#pragma pack(push, 1)
	struct Packet {
		RadiotapHdr radiotapHdr_;
		QosNullHdr nullHdr_;
	} packet;
	#pragma pack(pop)

	Mac addr1("00:00:00:11:11:11");
	Mac addr2("00:00:00:22:22:22");
	Mac addr3("00:00:00:33:33:33");

	packet.radiotapHdr_.ver_ = 0;
	packet.radiotapHdr_.pad_= 0;
	packet.radiotapHdr_.len_= sizeof(RadiotapHdr);
	packet.radiotapHdr_.present_= 0;

	packet.nullHdr_.ver_ = 0;
	packet.nullHdr_.type_ = Dot11Hdr::Data;
	packet.nullHdr_.subtype_ = 12;
	packet.nullHdr_.flags_ = 0;
	packet.nullHdr_.duration_ = 0;

	packet.nullHdr_.addr1_ = addr1;
	packet.nullHdr_.addr2_ = addr2;
	packet.nullHdr_.addr3_ = addr3;
	packet.nullHdr_.frag_ = 0;
	packet.nullHdr_.seq_ = 0;
	packet.nullHdr_.qosControl_ = 0;

	Clock start = Timer::now();
	Clock lastSent = Clock(Diff(0));
	while (true) {
		Clock now = Timer::now();
		Diff diff = now - start;
		if (diff >= Diff(std::chrono::seconds(8))) {
			GTRACE("can not check radio tap length(real=%u send=%d ignore=%d)", real_, send_, ignore_);
			return false;
		}
		diff = now - lastSent;
		if (diff >= Diff(std::chrono::seconds(1))) {
			int res = pcap_sendpacket(handle, (const u_char*)&packet, sizeof(Packet));
			if (res != 0) {
				GTRACE("pcap_sendpacket return %d - %s handle=%p", res, pcap_geterr(handle), handle);
				return false;
			}
			lastSent = now;
		}

		pcap_pkthdr* header;
		const u_char* packet;
		int res = pcap_next_ex(handle, &header, &packet);
		if (res == 0) continue;
		if (res == -1 || res == -2) {
			GTRACE("pcap_next_ex return %d(%s)", res, pcap_geterr(handle));
			break;
		}

		uint32_t size = header->caplen;
		RadiotapHdr* radiotapHdr = RadiotapHdr::check(pchar(packet), size);
		if (radiotapHdr == nullptr) continue;
		if (radiotapHdr->len_ == send_) continue; // my own sending packet by other process
		size -= radiotapHdr->len_;

		Dot11Hdr* dot11Hdr = Dot11Hdr::check(radiotapHdr, size);
		if (dot11Hdr == nullptr) continue;

		bool myPacket = false;
		if (dot11Hdr->typeSubtype() == Dot11Hdr::QosNull) {
			QosNullHdr* qosNullHdr = QosNullHdr::check(dot11Hdr, size);
			if (qosNullHdr != nullptr) {
				if (qosNullHdr->addr1_ == addr1 && qosNullHdr->addr2_ == addr2 && qosNullHdr->addr3_ == addr3) {
					myPacket = true;
				}
			}
		}
		le16_t len = radiotapHdr->len_;
		if (myPacket) {
			if (len != sizeof(RadiotapHdr))
				ignore_ = len;
		} else
			real_ = len;

		if (real_ != 0 && ignore_ != 0) break;
	}
	GTRACE("RadiotapHdr Length Checking completed real=%d send=%d ignore=%d", real_, send_, ignore_);
	checked_ = true;
	pcap_close(handle);
	return true;
}
*/
// -----------------------------------
