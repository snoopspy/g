#include "gdemon.h"
#include "gtrace.h"

#ifdef __linux__
#include <sys/socket.h>
#endif // __linux__
#ifdef WIN32
#include <winsock2.h>
#endif // WIN32

// ----------------------------------------------------------------------------
// GDemon
// ----------------------------------------------------------------------------
bool GDemon::recvAll(int sd, pvoid buffer, int32_t size) {
	pchar buf = pchar(buffer);
	int32_t remain = size;
	while (remain > 0) {
		ssize_t recvLen = ::recv(sd, buf, remain, 0);
		if (recvLen == 0 || recvLen == -1) {
			GTRACE("recvLen=%zd remain=%d", recvLen, remain); // gilgil temp 2023.10.24
			return false;
		}
		buf += recvLen;
		remain -= recvLen;
	}
	return true;
}

//
// header
//
int32_t GDemon::Header::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	*pint32_t(buf) = len_; buf += sizeof(len_); size -= sizeof(len_);
	*PCmd(buf) = cmd_; buf += sizeof(cmd_); size -= sizeof(cmd_);

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::Header::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = *pint32_t(buf); buf += sizeof(len_); size -= sizeof(len_);
	cmd_ = *PCmd(buf); buf += sizeof(cmd_); size -= sizeof(cmd_);

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::Header::recv(int sd) {
	if (!recvAll(sd, pvoid(this), sizeof(Header))) {
		GTRACE("recvAll(Header) return false");
		return -1;
	}

	if (this->len_ < 0 || sizeof(Header) + len_ > MaxBufSize) {
		GTRACE("invalid header len(%d)", len_);
		return -1;
	}

	return sizeof(Header);
}

//
// command
//
int32_t GDemon::CmdExecuteReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// command_
	int32_t len = command_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, command_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdExecute;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdExecuteReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdExecute) {
		GTRACE("cmd_ is not CmdCmdExecute %d", cmd_);
		return -1;
	}

	// command_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	command_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdExecuteRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// result_
	*pbool(buf) = result_; buf += sizeof(result_); size -= sizeof(result_);

	// error_
	int32_t len = error_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, error_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdExecute;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdExecuteRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdExecute) {
		GTRACE("cmd_ is not CmdCmdExecute %d", cmd_);
		return -1;
	}

	// result_
	result_ = *pbool(buf); buf += sizeof(result_); size -= sizeof(result_);

	// error_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	error_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// command_
	int32_t len = command_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, command_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdStart;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdStart) {
		GTRACE("cmd_ is not CmdCmdStart %d", cmd_);
		return -1;
	}

	// command_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	command_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// pid_
	*pint64_t(buf) = pid_; buf += sizeof(pid_); size -= sizeof(pid_);

	// error_
	int32_t len = error_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, error_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdStart;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdStart) {
		GTRACE("cmd_ is not CmdCmdStart %d", cmd_);
		return -1;
	}

	// pid_
	pid_ = *pint64_t(buf); buf += sizeof(pid_); size -= sizeof(pid_);

	// error_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	error_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStopReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// pid_
	*pint64_t(buf) = pid_; buf += sizeof(pid_); size -= sizeof(pid_);

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdStop;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStopReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdStop) {
		GTRACE("cmd_ is not CmdCmdStop %d", cmd_);
		return -1;
	}

	// pid_
	pid_ = *pint64_t(buf); buf += sizeof(pid_); size -= sizeof(pid_);

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStopRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// result_
	*pbool(buf) = result_; buf += sizeof(result_); size -= sizeof(result_);

	// error_
	int32_t len = error_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, error_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdStop;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStopRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdStop) {
		GTRACE("cmd_ is not CmdCmdStop %d", cmd_);
		return -1;
	}

	// result_
	result_ = *pbool(buf); buf += sizeof(result_); size -= sizeof(result_);

	// error_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	error_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartDetachedReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// command_
	int32_t len = command_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, command_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdStartDetached;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartDetachedReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdStartDetached) {
		GTRACE("cmd_ is not CmdCmdStartDetached %d", cmd_);
		return -1;
	}

	// command_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	command_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartDetachedRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// result_
	*pbool(buf) = result_; buf += sizeof(result_); size -= sizeof(result_);

	// error_
	int32_t len = error_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, error_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdCmdStartDetached;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::CmdStartDetachedRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdCmdStartDetached) {
		GTRACE("cmd_ is not CmdCmdStartDetached %d", cmd_);
		return -1;
	}

	// result_
	result_ = *pbool(buf); buf += sizeof(result_); size -= sizeof(result_);

	// error_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	error_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

//
// network information
//
int32_t GDemon::Interface::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// index_
	*pint32_t(buf) = index_; buf += sizeof(index_); size -= sizeof(index_);

	// name_
	int32_t len = name_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, name_.data(), len); buf += len; size -= len;

	// desc_
	len = desc_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, desc_.data(), len); buf += len; size -= len;

	// mac_
	memcpy(buf, mac_, MacSize); buf += MacSize; size -= MacSize;

	// ip
	memcpy(buf, &ip_, sizeof(ip_)); buf += sizeof(ip_); size -= sizeof(ip_);

	// mask_
	memcpy(buf, &mask_, sizeof(mask_)); buf += sizeof(mask_); size -= sizeof(mask_);

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::Interface::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// index_
	index_ = *pint32_t(buf); buf += sizeof(index_); size -= sizeof(index_);
	// GTRACE("index_ = %d", index_); // gilgil temp 2021.03.09

	// name_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	name_ = std::string(buf, len); buf += len; size -= len;
	// GTRACE("name_222=%s", name_.data()); // gilgil temp 2021.03.09

	// desc_
	len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	desc_ = std::string(buf, len); buf += len; size -= len;
	// GTRACE("desc_=%s", desc_.data()); // gilgil temp 2021.03.09

	// mac_
	memcpy(mac_, buf, MacSize); buf += MacSize; size -= MacSize;
	// GTRACE("mac_222= %02x:%02x:%02x:%02x:%02x:%02x", mac_[0], mac_[1], mac_[2], mac_[3], mac_[4], mac_[5]); // gilgil temp 2021.03.09

	// ip_
	memcpy(&ip_, buf, sizeof(ip_)); buf += sizeof(ip_); size -= sizeof(ip_);

	// mask_
	memcpy(&mask_, buf, sizeof(mask_)); buf += sizeof(mask_); size -= sizeof(mask_);

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::InterfaceList::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// count
	int32_t cnt = this->size();
	*pint32_t(buf) = cnt; buf += sizeof(cnt); size -= sizeof(cnt);

	// InterfaceList
	for (Interface& intf: *this) {
		int32_t len = intf.encode(buf, size);
		if (len == -1) return -1;
		buf += len; size -= len;
	}

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::InterfaceList::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// count
	int32_t cnt = *pint32_t(buf); buf += sizeof(cnt); size -= sizeof(cnt);

	// InterfaceList
	for (int32_t i = 0; i < cnt; i++) {
		Interface intf;

		int32_t len = intf.decode(buf, size);
		if (len == -1) return -1;
		push_back(intf);
		buf += len; size -= len;
	}

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RtmEntry::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// dst_
	*puint32_t(buf) = dst_; buf += sizeof(dst_); size -= sizeof(dst_);

	// mask_
	*puint32_t(buf) = mask_; buf += sizeof(mask_); size -= sizeof(mask_);

	// gateway_
	*puint32_t(buf) = gateway_; buf += sizeof(gateway_); size -= sizeof(gateway_);

	// metric_
	*puint32_t(buf) = metric_; buf += sizeof(metric_); size -= sizeof(metric_);

	// intfName_
	int32_t len = intfName_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, intfName_.data(), len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;

}

int32_t GDemon::RtmEntry::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// dst_
	dst_ = *puint32_t(buf); buf += sizeof(dst_); size -= sizeof(dst_);

	// mask_
	mask_ = *puint32_t(buf); buf += sizeof(mask_); size -= sizeof(mask_);

	// dst_
	gateway_ = *puint32_t(buf); buf += sizeof(gateway_); size -= sizeof(gateway_);

	// dst_
	metric_ = *pint32_t(buf); buf += sizeof(metric_); size -= sizeof(metric_);

	// name_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	intfName_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::Rtm::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// count
	int32_t cnt = this->size();
	*pint32_t(buf) = cnt; buf += sizeof(cnt); size -= sizeof(cnt);

	// Rtm
	for (RtmEntry& rtmEntry: *this) {
		int32_t len = rtmEntry.encode(buf, size);
		if (len == -1) return -1;
		buf += len; size -= len;
	}

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}


int32_t GDemon::Rtm::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	// count
	int32_t cnt = *pint32_t(buf); buf += sizeof(cnt); size -= sizeof(cnt);

	// Rtm
	for (int32_t i = 0; i < cnt; i++) {
		RtmEntry rtmEntry;

		int32_t len = rtmEntry.decode(buf, size);
		if (len == -1) return -1;
		push_back(rtmEntry);
		buf += len; size -= len;
	}

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}


int32_t GDemon::GetInterfaceListReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = 0;
	cmd_ = CmdGetInterfaceList;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::GetInterfaceListReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; // size -= decLen;
	if (len_ != 0) {
		GTRACE("len_ is not zero %d\n", len_);
		return -1;
	}
	if (cmd_ != CmdGetInterfaceList) {
		GTRACE("cmd_ is not CmdGetInterfaceList %d", cmd_);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::GetInterfaceListRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	int32_t encLen = interfaceList_.encode(buf, size); buf += encLen; size -= encLen;
	if (encLen == -1) return -1;

	len_ = encLen;
	cmd_ = CmdGetInterfaceList;
	Header::encode(buffer, sizeof(Header)); // buf += encLen; size -= encLen;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::GetInterfaceListRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdGetInterfaceList) {
		GTRACE("cmd_ is not CmdGetInterfaceList %d", cmd_);
		return -1;
	}

	decLen = interfaceList_.decode(buf, size); buf += decLen; size -= decLen;
	if (decLen == -1) return -1;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::GetRtmReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = 0;
	cmd_ = CmdGetRtm;
	int32_t encLen = Header::encode(buf, size);	buf += encLen; size -= encLen;

	if (size < 0) {
		GTRACE(" size is %d\n", size);
		return -1;
	}
	return buf - buffer;
}


int32_t GDemon::GetRtmReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; // size -= decLen;
	if (len_ != 0) {
		GTRACE("len_ is not zero %d\n", len_);
		return -1;
	}
	if (cmd_ != CmdGetRtm) {
		GTRACE("cmd_ is not CmdGetRtm %d", cmd_);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::GetRtmRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	int32_t encLen = rtm_.encode(buf, size); buf += encLen; size -= encLen;
	if (encLen == -1) return -1;

	len_ = encLen;
	cmd_ = CmdGetRtm;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE(" size is %d\n", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::GetRtmRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdGetRtm) {
		GTRACE("cmd_ is not CmdGetRtm %d", cmd_);
		return -1;
	}

	decLen = rtm_.decode(buf, size); buf += decLen; size -= decLen;
	if (decLen == -1) return -1;

	if (size < 0) {
		GTRACE(" size is %d\n", size);
		return -1;
	}
	return buf - buffer;
}

//
// pcap
//
int32_t GDemon::PcapOpenReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// client_
	int32_t len = client_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, client_.data(), len); buf += len; size -= len;

	// filter_
	len = filter_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, filter_.data(), len); buf += len; size -= len;

	// intfName_
	len = intfName_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, intfName_.data(), len); buf += len; size -= len;

	// snaplen_
	*pint32_t(buf) = snaplen_; buf += sizeof(snaplen_); size -= sizeof(snaplen_);

	// flags_
	*pint32_t(buf) = flags_; buf += sizeof(flags_); size -= sizeof(flags_);

	// readTimeout_
	*pint32_t(buf) = readTimeout_; buf += sizeof(readTimeout_); size -= sizeof(readTimeout_);

	// waitTimeout_
	*pint32_t(buf) = waitTimeout_; buf += sizeof(waitTimeout_); size -= sizeof(waitTimeout_);

	// captureThread_
	*pbool(buf) = captureThread_; buf += sizeof(captureThread_); size -= sizeof(captureThread_);

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdPcapOpen;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapOpenReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdPcapOpen) {
		GTRACE("cmd_ is not CmdPcapOpen %d", cmd_);
		return -1;
	}

	// client_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	client_ = std::string(buf, len); buf += len; size -= len;

	// filter_
	len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	filter_ = std::string(buf, len); buf += len; size -= len;

	// intfName_
	len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	intfName_ = std::string(buf, len); buf += len; size -= len;

	// snaplen_
	snaplen_ = *pint32_t(buf); buf += sizeof(snaplen_); size -= sizeof(snaplen_);

	// flags_
	flags_ = *pint32_t(buf); buf += sizeof(flags_); size -= sizeof(flags_);

	// readTimeout_
	readTimeout_ = *pint32_t(buf); buf += sizeof(readTimeout_); size -= sizeof(readTimeout_);

	// waitTimeout_
	waitTimeout_ = *pint32_t(buf); buf += sizeof(waitTimeout_); size -= sizeof(waitTimeout_);

	// captureThread_
	captureThread_ = *pbool(buf); buf += sizeof(captureThread_); size -= sizeof(captureThread_);

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapOpenRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// result_
	*pbool(buf) = result_; buf += sizeof(result_); size -= sizeof(result_);

	// dataLink_
	*pint32_t(buf) = dataLink_; buf += sizeof(dataLink_); size -= sizeof(dataLink_);

	// errBuf_
	int32_t len = errBuf_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, errBuf_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdPcapOpen;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapOpenRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdPcapOpen) {
		GTRACE("cmd_ is not CmdPcapOpen %d", cmd_);
		return -1;
	}

	// result_
	result_ = *pbool(buf); buf += sizeof(result_); size -= sizeof(result_);

	// dataLink_
	dataLink_ = *pint32_t(buf); buf += sizeof(dataLink_); size -= sizeof(dataLink_);

	// errBuf_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	errBuf_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapCloseReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = 0;
	cmd_ = CmdPcapClose;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapCloseReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdPcapClose) {
		GTRACE("cmd_ is not CmdPcapClose %d", cmd_);
		return -1;
	}

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapRead::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = sizeof(pktHdr_) + pktHdr_.caplen_;
	cmd_ = CmdPcapRead;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	// pktHdr_
	memcpy(buf, &pktHdr_, sizeof(pktHdr_)); buf += sizeof(pktHdr_); size -= sizeof(pktHdr_);

	// data_
	if (size < int(pktHdr_.caplen_)) {
		GTRACE("not enough buffer size=%d caplen=%u", size, pktHdr_.caplen_);
		return -1;
	}
	memcpy(buf, data_, pktHdr_.caplen_); buf += pktHdr_.caplen_; size -= pktHdr_.caplen_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapRead::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdPcapRead) {
		GTRACE("cmd_ is not CmdPcapRead %d", cmd_);
		return -1;
	}

	// pktHdr_
	pktHdr_ = *reinterpret_cast<PktHdr*>(buf); buf += sizeof(pktHdr_); size -= sizeof(pktHdr_);

	// data_
	if (size - int32_t(pktHdr_.caplen_) < 0) {
		GTRACE("not enough buffer size=%d caplen=%u", size, pktHdr_.caplen_);
		return -1;
	}
	data_ = puchar(buf); buf += pktHdr_.caplen_; size -= pktHdr_.caplen_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapWrite::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = sizeof(size_) + size_;
	cmd_ = CmdPcapWrite;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	// size_
	*pint32_t(buf) = size_; buf += sizeof(size_); size -= sizeof(size_);

	// data_
	if (size < size_) {
		GTRACE("not enough buffer size=%d size_=%d", size, size_);
		return -1;
	}
	memcpy(buf, data_, size_); buf += size_; size -= size_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::PcapWrite::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdPcapWrite) {
		GTRACE("cmd_ is not CmdPcapWrite %d", cmd_);
		return -1;
	}

	// size_
	size_ = *pint32_t(buf); buf += sizeof(size_); size -= sizeof(size_);

	// data_
	data_ = puchar(buf); buf += size_; size -= size_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

//
// netfilter
//
int32_t GDemon::NfOpenReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// client_
	int32_t len = client_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, client_.data(), len); buf += len; size -= len;

	// queueNum_
	*pint16_t(buf) = queueNum_; buf += sizeof(queueNum_); size -= sizeof(queueNum_);

	// nonBlock_
	*pbool(buf) = nonBlock_; buf += sizeof(nonBlock_); size -= sizeof(nonBlock_);

	// waitTimeout_
	*pint32_t(buf) = waitTimeout_; buf += sizeof(waitTimeout_); size -= sizeof(waitTimeout_);

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdNfOpen;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfOpenReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdNfOpen) {
		GTRACE("cmd_ is not CmdNfOpen %d", cmd_);
		return -1;
	}

	// client_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	client_ = std::string(buf, len); buf += len; size -= len;

	// queueNum_
	queueNum_ = *pint16_t(buf); buf += sizeof(queueNum_); size -= sizeof(queueNum_);

	// nonBlock_
	nonBlock_ = *pbool(buf); buf += sizeof(nonBlock_); size -= sizeof(nonBlock_);

	// waitTimeout_
	waitTimeout_ = *pint16_t(buf); buf += sizeof(waitTimeout_); size -= sizeof(waitTimeout_);

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfOpenRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// result_
	*pbool(buf) = result_; buf += sizeof(result_); size -= sizeof(result_);

	// errBuf_
	int32_t len = errBuf_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, errBuf_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdNfOpen;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfOpenRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdNfOpen) {
		GTRACE("cmd_ is not CmdNfOpen %d", cmd_);
		return -1;
	}

	// result_
	result_ = *pbool(buf); buf += sizeof(result_); size -= sizeof(result_);

	// errBuf_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	errBuf_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfCloseReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = 0;
	cmd_ = CmdNfClose;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfCloseReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdNfClose) {
		GTRACE("cmd_ is not CmdNfClose %d", cmd_);
		return -1;
	}

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfRead::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = sizeof(pktHdr_) + sizeof(id_) + pktHdr_.len_;
	cmd_ = CmdNfRead;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	// pktHdr_
	memcpy(buf, &pktHdr_, sizeof(pktHdr_)); buf += sizeof(pktHdr_); size -= sizeof(pktHdr_);

	// id_
	*puint32_t(buf) = id_; buf += sizeof(id_); size -= sizeof(id_);

	// data_
	if (size < int(pktHdr_.len_)) {
		GTRACE("not enough buffer size=%d caplen=%u", size, pktHdr_.len_);
		return -1;
	}
	memcpy(buf, data_, pktHdr_.len_); buf += pktHdr_.len_; size -= pktHdr_.len_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfRead::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdNfRead) {
		GTRACE("cmd_ is not CmdNfRead %d", cmd_);
		return -1;
	}

	// pktHdr_
	pktHdr_ = *reinterpret_cast<PktHdr*>(buf); buf += sizeof(pktHdr_); size -= sizeof(pktHdr_);

	// id_
	id_ = *puint32_t(buf); buf += sizeof(id_); size -= sizeof(id_);

	// data_
	if (size - int32_t(pktHdr_.len_) < 0) {
		GTRACE("not enough buffer size=%d caplen=%u", size, pktHdr_.len_);
		return -1;
	}
	data_ = puchar(buf); buf += pktHdr_.len_; size -= pktHdr_.len_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfVerdict::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = sizeof(id_) + sizeof(acceptVerdict_) + sizeof(mark_) + sizeof(size_) + size_;
	cmd_ = CmdNfVerdict;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	// id_;
	*puint32_t(buf) = id_; buf += sizeof(id_); size -= sizeof(id_);

	// acceptVerdict_
	*puint32_t(buf) = acceptVerdict_; buf += sizeof(acceptVerdict_); size -= sizeof(acceptVerdict_);

	// mark_
	*puint32_t(buf) = mark_; buf += sizeof(mark_); size -= sizeof(mark_);

	// size_
	*puint32_t(buf) = size_; buf += sizeof(size_); size -= sizeof(size_);

	// data_
	if (size < int32_t(size_)) {
		GTRACE("not enough buffer size=%d size_=%u", size, size_);
		return -1;
	}
	if (size_ > 0) {
		memcpy(buf, data_, size_); buf += size_; size -= size_;
	}

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::NfVerdict::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdNfVerdict) {
		GTRACE("cmd_ is not CmdNfVerdict %d", cmd_);
		return -1;
	}

	// id_
	id_ = *puint32_t(buf); buf += sizeof(id_); size -= sizeof(id_);

	// acceptVerdict_
	acceptVerdict_ = *puint32_t(buf); buf += sizeof(acceptVerdict_); size -= sizeof(acceptVerdict_);

	// mark_
	mark_ = *puint32_t(buf); buf += sizeof(mark_); size -= sizeof(mark_);

	// size_
	size_ = *puint32_t(buf); buf += sizeof(size_); size -= sizeof(size_);

	// data_
	data_ = puchar(buf); buf += size_; size -= size_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

//
// rawip
//
int32_t GDemon::RiOpenReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdRiOpen;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiOpenReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdRiOpen) {
		GTRACE("cmd_ is not CmdRiOpen %d", cmd_);
		return -1;
	}

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiOpenRes::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	buf += sizeof(Header); size -= sizeof(Header);

	// result_
	*pbool(buf) = result_; buf += sizeof(result_); size -= sizeof(result_);

	// errBuf_
	int32_t len = errBuf_.size();
	*pint32_t(buf) = int32_t(len); buf += sizeof(len); size -= sizeof(len);
	memcpy(buf, errBuf_.data(), len); buf += len; size -= len;

	len_ = buf - buffer - sizeof(Header);
	cmd_ = CmdRiOpen;
	Header::encode(buffer, sizeof(Header));

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiOpenRes::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdRiOpen) {
		GTRACE("cmd_ is not CmdRiOpen %d", cmd_);
		return -1;
	}

	// result_
	result_ = *pbool(buf); buf += sizeof(result_); size -= sizeof(result_);

	// errBuf_
	int32_t len = *pint32_t(buf); buf += sizeof(len); size -= sizeof(len);
	errBuf_ = std::string(buf, len); buf += len; size -= len;

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiCloseReq::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = 0;
	cmd_ = CmdRiClose;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiCloseReq::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdRiClose) {
		GTRACE("cmd_ is not CmdRiClose %d", cmd_);
		return -1;
	}

	if (size < 0) {
		GTRACE("decode size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiWrite::encode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	len_ = sizeof(size_) + size_;
	cmd_ = CmdRiWrite;
	int32_t encLen = Header::encode(buf, size); buf += encLen; size -= encLen;

	// size_
	*pint32_t(buf) = size_; buf += sizeof(size_); size -= sizeof(size_);

	// data_
	if (size < size_) {
		GTRACE("not enough buffer size=%d size_=%d", size, size_);
		return -1;
	}
	memcpy(buf, data_, size_); buf += size_; size -= size_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}

int32_t GDemon::RiWrite::decode(pchar buffer, int32_t size) {
	volatile pchar buf = buffer;

	int32_t decLen = Header::decode(buf, size); buf += decLen; size -= decLen;
	if (cmd_ != CmdRiWrite) {
		GTRACE("cmd_ is not CmdRiWrite %d", cmd_);
		return -1;
	}

	// size_
	size_ = *pint32_t(buf); buf += sizeof(size_); size -= sizeof(size_);

	// data_
	data_ = puchar(buf); buf += size_; size -= size_;

	if (size < 0) {
		GTRACE("size is %d", size);
		return -1;
	}
	return buf - buffer;
}
