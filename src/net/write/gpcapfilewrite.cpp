#include "gpcapfilewrite.h"
#include <QDateTime>
#include <QDir>
#include <QFileInfo>

// ----------------------------------------------------------------------------
// GPcapFileWrite
// ----------------------------------------------------------------------------
bool GPcapFileWrite::doOpen() {
	if (!enabled_) return true;

	if (fileName_ == "") {
		SET_ERR(GErr::FileNameNotSpecified, "file name is not specified");
		return false;
	}

	int dataLink = GPacket::dltToInt(dlt_);
	pcap_ = pcap_open_dead(dataLink, snapLen_);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::ReturnNull, QString("pcap_open_dead(%1, %2)) return null").arg(dataLink, snapLen_));
		return false;
	}

	QString realFileName = fileName_;
	if (resolveFileNameByTime_) {
		QString fileName = QFileInfo(fileName_).fileName();
		QDateTime now = QDateTime::currentDateTime();
		realFileName = now.toString(fileName);
	}

	QString path = QFileInfo(fileName_).path();
	if (path != "") {
		QDir dir;
		dir.mkpath(path);
		if (resolveFileNameByTime_)
			realFileName = path + QDir::separator() + realFileName;
	}

	pcap_dumper_ = pcap_dump_open(pcap_, qPrintable(realFileName));
	if (pcap_dumper_ == nullptr) {
		SET_ERR(GErr::ReturnNull, QString("pcap_dump_open(%1)) return null").arg(realFileName));
		return false;
	}
	return true;
}

bool GPcapFileWrite::doClose() {
	if (!enabled_) return true;

	if (pcap_dumper_ != nullptr) {
		pcap_dump_close(pcap_dumper_);
		pcap_dumper_ = nullptr;
	}

	if (pcap_ != nullptr) {
		pcap_close(pcap_);
		pcap_ = nullptr;
	}
	return true;
}

GPacket::Result GPcapFileWrite::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NotSupported, "not supported");
	return GPacket::Fail;
}

GPacket::Result GPcapFileWrite::write(GPacket* packet) {
	if (!enabled_) return GPacket::Ok;

	pcap_pkthdr pkthdr;
	pkthdr.ts = packet->ts_;
	pkthdr.caplen = pkthdr.len = bpf_u_int32(packet->buf_.size_);
	pcap_dump(pbyte(pcap_dumper_), &pkthdr, packet->buf_.data_);
	emit written(packet);
	return GPacket::Ok;
}
