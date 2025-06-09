#include "gdnsblock.h"

// ----------------------------------------------------------------------------
// GDnsBlockItem
// ----------------------------------------------------------------------------
GDnsBlockItem::GDnsBlockItem(QObject* parent) : GObj(parent) {
}

GDnsBlockItem::GDnsBlockItem(QObject* parent, QString domain, QString ip) :
	GObj(parent), domain_(domain), ip_(ip) {
}

GDnsBlockItem::~GDnsBlockItem() {
}

// ----------------------------------------------------------------------------
// GDnsBlock
// ----------------------------------------------------------------------------
bool GDnsBlock::doOpen() {
	if (objectName() != "") writer_.setObjectName(objectName() + ".writer_");
	if (!writer_.open()) {
		err = writer_.err;
		return false;
	}

	myBlockItems_.clear();
	for (GObj* obj: dnsBlockItems_) {
		GDnsBlockItem* dnsBlockItem = PDnsBlockItem(obj);
		QString domain = dnsBlockItem->domain_;

		QString pattern = "^";
		while (domain != "") {
			int offset = domain.indexOf("*");
			if (offset != -1) {
				QString leftStr = domain.left(offset);
				pattern += "[A-Za-z0-9-]*" + leftStr;
				domain = domain.mid(offset + 1);
				continue;
			}
			offset = domain.indexOf(".");
			if (offset != -1) {
				QString leftStr = domain.left(offset);
				pattern += leftStr + "\\.";
				domain = domain.mid(offset + 1);
				continue;
			}
			pattern += domain;
			break;
		}

		MyBlockItem mbi;
		mbi.re_.setPattern(pattern);
		mbi.ip_ = dnsBlockItem->ip_;
		myBlockItems_.push_back(mbi);
	}

	return true;
}

bool GDnsBlock::doClose() {
	writer_.close();
	myBlockItems_.clear();

	return true;
}

void GDnsBlock::block(GPacket* packet) {
	GUdpHdr *udpHdr = packet->udpHdr_;
	if (udpHdr == nullptr)
		return;

	if (port_ != 0) {
		if (udpHdr->sport() != port_ && udpHdr->dport() != port_)
			return;
	}

	GBuf udpData = packet->udpData_;
	if (!udpData.valid())
		return;

	GDnsInfo dnsInfo;
	size_t offset = 0;
	if (!dnsInfo.decode(udpData.data_, udpData.size_, &offset)) return;
	if (dnsInfo.questions_.count() != 1) return;
	if (dnsInfo.answers_.count() != 0) return;
	if (dnsInfo.authorities_.count() != 0) return;
	if (dnsInfo.additionals_.count() != 0) return;


	QString queryName = dnsInfo.questions_.at(0).name_;
	for (MyBlockItem& mbi: myBlockItems_) {
		QRegularExpressionMatch match = mbi.re_.match(queryName);
		if (match.hasMatch()) {

			QByteArray msg;
			msg.resize(sizeof(GDnsHdr));

			GDnsHdr* dnsHdr = PDnsHdr(msg.data());
			dnsHdr->id_ = dnsInfo.dnsHdr_.id_;
			dnsHdr->flags_ = htons(0x8180);
			dnsHdr->num_q_ = htons(1);
			dnsHdr->num_answ_rr = htons(1);
			dnsHdr->num_auth_rr = htons(0);
			dnsHdr->num_addi_rr = htons(0);

			msg += dnsInfo.questions_.encode();
			msg += char('\xc0'); // Name
			msg += char('\x0c');

			msg += char('\x00'); // Type: A (Host Address) (1)
			msg += char('\x01');

			msg += char('\x00'); // Class: IN (0x0001)
			msg += char('\x01');

			msg += char('\x00'); // Time to Live : 300 (5 minutes)
			msg += char('\x00');
			msg += char('\x01');
			msg += char('\x2c');

			msg += char('\x00'); // Data length : 4
			msg += char('\x04');

			quint32 ip = htonl(mbi.ip_);
			int oldSize = msg.size();
			msg.resize(oldSize + sizeof(ip));
			memcpy(msg.data() + oldSize, &ip, sizeof(quint32));

			size_t blockLen = sizeof(GIpHdr) + sizeof(GUdpHdr) + msg.size();
			blockByteArray_.resize(blockLen);

			//
			// blockIpPacket_
			//
			gbyte* data = pbyte(blockByteArray_.data());
			blockIpPacket_.buf_.data_ = data;
			blockIpPacket_.buf_.size_ = blockLen;
			blockIpPacket_.ipHdr_ = PIpHdr(data);
			blockIpPacket_.udpHdr_ = PUdpHdr(data + sizeof(GIpHdr));
			blockIpPacket_.udpData_.data_ = data + sizeof(GIpHdr) + sizeof(GUdpHdr);
			blockIpPacket_.udpData_.size_ = msg.size();


			//
			// IP
			//
			GIpHdr* blockIpHdr = blockIpPacket_.ipHdr_;
			GIpHdr* ipHdr = packet->ipHdr_;
			Q_ASSERT(ipHdr != nullptr);
			memcpy(pvoid(blockIpHdr), pvoid(ipHdr), sizeof(GIpHdr));
			blockIpHdr->v_hlen_ = 0x45;
			blockIpHdr->tos_ = 0x44;
			blockIpHdr->tlen_ = htons(blockLen);
			blockIpHdr->ttl_ = 0x80;
			std::swap(blockIpHdr->sip_, blockIpHdr->dip_);

			//
			// UDP
			//
			GUdpHdr* blockUdpHdr = blockIpPacket_.udpHdr_;
			blockUdpHdr->sport_ = udpHdr->dport_;
			blockUdpHdr->dport_ = udpHdr->sport_;
			blockUdpHdr->len_ = htons(sizeof(GUdpHdr) + msg.size());

			//
			// Data
			//
			memcpy(blockIpPacket_.udpData_.data_, msg.data(), msg.size());

			//
			// checksum
			//
			blockIpHdr->sum_ = GIpHdr::inetCalcSum(PIpHdr(blockIpHdr));
			blockUdpHdr->sum_ = GUdpHdr::inetCalcSum(PIpHdr(blockIpHdr), blockUdpHdr);

			// write
			writer_.write(&blockIpPacket_);
			qDebug() << QString("dns changed %1 %2").arg(queryName).arg(QString(mbi.ip_));

			packet->ctrl_.block_ = true;

			emit blocked(packet);
			return;
		}
	}
}
