#include <QDebug>
#include "gdnsinfo.h"

// ----------------------------------------------------------------------------
// GDnsInfo::Question
// ----------------------------------------------------------------------------
QByteArray GDnsInfo::Question::encode() {
	QByteArray res;

	res = GDnsInfo::encodeName(this->name_);

	uint16_t _type = htons(this->type_);
	res.append(pchar(&_type), sizeof(uint16_t));

	uint16_t _class_ = htons(this->class_);
	res.append(pchar(&_class_), sizeof(uint16_t));

	return res;
}

bool GDnsInfo::Question::decode(u_char* udpData, size_t dataLen, size_t* offset) {
	this->name_ = GDnsInfo::decodeName(udpData, dataLen, offset);
	if (this->name_ == "") return false;

	if (*offset + sizeof(uint16_t) > dataLen) return false;
	uint16_t* _type = puint16_t(udpData + *offset);
	this->type_ = ntohs(*_type);
	*offset += sizeof(uint16_t);

	if (*offset + sizeof(uint16_t) > dataLen) return false;
	uint16_t* _class_ = puint16_t(udpData + *offset);
	this->class_ = ntohs(*_class_);
	*offset += sizeof(uint16_t);

	if (*offset > dataLen) return false;
	return true;
}

// ----------------------------------------------------------------------------
// GDnsInfo::Questions
// ----------------------------------------------------------------------------
QByteArray GDnsInfo::Questions::encode() {
	QByteArray res;
	for (Question question: *this) {
		res += question.encode();
	}
	return res;
}

bool GDnsInfo::Questions::decode(u_char* udpData, size_t dataLen, int count, size_t* offset) {
	for (int i = 0; i < count; i++) {
		Question question;
		if (!question.decode(udpData, dataLen, offset)) return false;
		push_back(question);
	}
	return true;
}

// ----------------------------------------------------------------------------
// GDnsInfo::ResourceRecord
// ----------------------------------------------------------------------------
QByteArray GDnsInfo::ResourceRecord::encode() {
	QByteArray res;

	//res = GDnsInfo::encodeName(this->name);
	res.append((char)0xC0); // gilgil temp 2014.03.22
	res.append((char)0x0C);

	uint16_t _type = htons(this->type_);
	res.append((const char*)&_type, sizeof(uint16_t));

	uint16_t _class_ = htons(this->class_);
	res.append((const char*)&_class_, sizeof(uint16_t));

	uint32_t _ttl = htonl(this->ttl_);
	res.append((const char*)&_ttl, sizeof(uint32_t));

	uint16_t _dataLength = htons(this->dataLength_);
	res.append((const char*)&_dataLength, sizeof(uint16_t));

	res += data_;

	return res;
}

bool GDnsInfo::ResourceRecord::decode(u_char* udpData, size_t dataLen, size_t* offset) {
	this->name_ = GDnsInfo::decodeName(udpData, dataLen, offset);
	if (this->name_ == "") return false;

	if (*offset + sizeof(uint16_t) > dataLen) return false;
	uint16_t* _type = puint16_t(udpData + *offset);
	this->type_ = ntohs(*_type);
	*offset += sizeof(uint16_t);

	if (*offset + sizeof(uint16_t) > dataLen) return false;
	uint16_t* _class_ = puint16_t(udpData + *offset);
	this->class_ = ntohs(*_class_);
	*offset += sizeof(uint16_t);

	if (*offset + sizeof(uint32_t) > dataLen) return false;
	uint32_t* _ttl = puint32_t(udpData + *offset);
	this->ttl_ = ntohl(*_ttl);
	*offset += sizeof(uint32_t);

	if (*offset + sizeof(uint16_t) > dataLen) return false;
	uint16_t* _dataLength = puint16_t(udpData + *offset);
	this->dataLength_= ntohs(*_dataLength);
	*offset += sizeof(uint16_t);

	if (*offset + dataLength_ > dataLen) return false;
	const char* data = const_cast<const char*>(pchar(udpData + *offset));
	this->data_ = QByteArray::fromRawData(data, this->dataLength_);
	*offset += this->dataLength_;

	return true;
}

// ----------------------------------------------------------------------------
// GDnsInfo::ResourceRecords
// ----------------------------------------------------------------------------
QByteArray GDnsInfo::ResourceRecords::encode() {
	QByteArray res;
	for (ResourceRecord record: *this) {
		res += record.encode();
	}
	return res;
}

bool GDnsInfo::ResourceRecords::decode(u_char* udpData, size_t dataLen, int count, size_t* offset) {
	for (int i = 0; i < count; i++) {
		ResourceRecord record;
		if (!record.decode(udpData, dataLen, offset)) return false;
		push_back(record);
	}
	return true;
}

// ----------------------------------------------------------------------------
// GDnsInfo
// ----------------------------------------------------------------------------
QByteArray GDnsInfo::encode() {
	QByteArray res;
	res.append((const char*)&dnsHdr_, sizeof(GDnsHdr));

	// ----- gilgil temp 2014.03.22 -----
	/*
	uint16_t _id = htons(dnsHdr.id);
	res.append((const char*)&_id, sizeof(uint16_t));

	uint16_t _flags = htons(dnsHdr.flags);
	res.append((const char*)&_flags, sizeof(uint16_t));

	uint16_t _num_q = htons(dnsHdr.num_q);
	res.append((const char*)&_num_q, sizeof(uint16_t));

	uint16_t _num_answ_rr = htons(dnsHdr.num_answ_rr);
	res.append((const char*)&_num_answ_rr, sizeof(uint16_t));

	uint16_t _num_auth_rr = htons(dnsHdr.num_auth_rr);
	res.append((const char*)&_num_auth_rr, sizeof(uint16_t));

	uint16_t _num_addi_rr = htons(dnsHdr.num_addi_rr);
	res.append((const char*)&_num_addi_rr, sizeof(uint16_t));
	*/
	// ----------------------------------

	res += questions_.encode();
	res += answers_.encode();
	res += authorities_.encode();
	res += additionals_.encode();

	return res;
}

bool GDnsInfo::decode(u_char* udpData, size_t dataLen, size_t* offset) {
	if (*offset + sizeof(GDnsHdr) > dataLen) return false;
	memcpy(&this->dnsHdr_, udpData, sizeof(GDnsHdr));
	*offset += sizeof(GDnsHdr);

	if (!questions_.decode(udpData, dataLen, dnsHdr_.q(), offset)) return false;
	if (!answers_.decode(udpData, dataLen, dnsHdr_.answ(), offset)) return false;
	if (!authorities_.decode(udpData, dataLen, dnsHdr_.auth(), offset)) return false;
	if (!additionals_.decode(udpData, dataLen, dnsHdr_.addr(), offset)) return false;

	return true;
}

QByteArray GDnsInfo::encodeName(QString name) {
	QStringList labels = name.split('.');
	int count = labels.count();
	if (count == 0) {
		qWarning() << QString("label count is zero(%1)").arg(name);
		return "";
	}

	QByteArray res;
	for (int i = 0; i < count; i++) {
		QString label = labels.at(i);
		uint8_t size = label.size();
		res.append((const char*)&size, sizeof(uint8_t));
		res += label.toLatin1();
	}
	res.append((char)0x00);

	return res;
}

QString GDnsInfo::decodeName(u_char* udpData, size_t dataLen, size_t* offset) {
	u_char* p = (u_char*)(udpData + *offset);
	QString res;
	bool first = true;
	while (true) {
		if (p - udpData > (int)dataLen) return "";
		uint8_t size = *p++;
		if (size == 0) break;

		if ((size & 0xC0) == 0xC0) {
			if (p - udpData > (int)dataLen) return "";
			size_t tempOffset = (((size_t)size & 0x3) << 8) + *p++;
			res += decodeName(udpData, dataLen, &tempOffset);
			*offset = p - udpData;
			return res;
		} else {
			if (p - udpData + size > (int)dataLen) return "";
			QByteArray label((const char*)p, (int)size);
			p += size;

			if (first) {
				res += label;
				first = false;
			} else {
				res += ".";
				res += label;
			}
		}
	}
	*offset = p - udpData;
	return res;
}
