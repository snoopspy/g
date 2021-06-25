#include "gpcappipe.h"

struct pcap_hdr_t {
	uint32_t magic_number; /* magic number */
	uint16_t version_major; /* major version number */
	uint16_t version_minor;/* minor version number */
	int32_t  thiszone;  /* GMT to local correction */
	uint32_t sigfigs; /* accuracy of timestamps */
	uint32_t snaplen; /* max length of captured packets, in octets */
	uint32_t network; /* data link type */
};

struct packet_hdr_t {
	uint32_t ts_sec; /* timestamp seconds */
	uint32_t ts_usec; /* timestamp microseconds */
	uint32_t incl_len; /* number of octets of packet saved in file */
	uint32_t orig_len; /* actual length of packet */
};

// ----------------------------------------------------------------------------
// GPcapPipe
// ----------------------------------------------------------------------------
GPcapPipe::GPcapPipe(QObject* parent) : GCapture(parent) {
	QString path = "/data/data/com.snoopspy/files";
	QString preloadStr = " ";
	if (QFile::exists("/system/lib/libfakeioctl.so"))
		preloadStr = " export LD_PRELOAD=libfakeioctl.so; ";
	command_ = QString("adb exec-out su -c \"cd %1; export LD_LIBRARY_PATH=%2/../lib;%3./corepcap dev wlan0 -f '' file -\"").arg(path, path, preloadStr);
	qDebug() << command_;
}

GPcapPipe::~GPcapPipe() {
	close();
}

bool GPcapPipe::doOpen() {
	if (!enabled_) return true;

	QStringList arguments = QProcess::splitCommand(command_);
	if (arguments.count() == 0) {
		SET_ERR(GErr::FAIL, "argument size is zero");
		return false;
	}
	QString program = arguments.at(0);
	arguments.removeAt(0);

	process_= new QProcess;
	process_->start(program, arguments, QProcess::ReadOnly);
	if (!process_->waitForStarted()) {
		SET_ERR(GErr::FAIL, "waitForStarted return false");
		return false;
	}

	pcap_hdr_t hdr;
	qint64 recvLen = recvAll(pchar(&hdr), sizeof(hdr));
	if (recvLen != sizeof(hdr))	return false;

	int dataLink =hdr.network;
	dlt_ = GPacket::intToDlt(dataLink);

	bool res = GCapture::doOpen();
	process_->moveToThread(&thread_);

	Q_ASSERT(recvBuf_ == nullptr);
	recvBuf_ = new gbyte[bufSize_];

	removeCrLastBytesBuffered_ = false;
	removeCrLastBytes_ = 0;

	return res;
}

bool GPcapPipe::doClose() {
	if (!enabled_) return true;

	GCapture::doClose();

	if (process_ != nullptr) {
		process_->terminate();
		process_->waitForFinished();
		delete process_;
		process_ = nullptr;
	}

	if (recvBuf_ != nullptr) {
		delete[] recvBuf_;
		recvBuf_ = nullptr;
	}

	return false; // gilgil temp
}

GPacket::Result GPcapPipe::read(GPacket* packet) {
	packet_hdr_t pktHdr;
	qint64 recvLen = recvAll(pchar(&pktHdr), sizeof(pktHdr));
	if (recvLen != sizeof(pktHdr))
		return GPacket::Fail;

	if (pktHdr.incl_len != pktHdr.orig_len)
		qWarning() << QString("incl_len(%1) and orig_len(%2) is not same").arg(QString::number(pktHdr.incl_len, 16)).arg(QString::number(pktHdr.orig_len, 16));

	qint64 len = pktHdr.incl_len;
	if (int(len) > bufSize_) {
		qWarning() << QString("len(%1) > bufSize_(%2)").arg(len).arg(bufSize_);
		return GPacket::Fail;
	}
	recvLen = recvAll(pchar(recvBuf_), len);
	if (recvLen != len)
		return GPacket::Fail;

	packet->clear();
	packet->ts_.tv_sec = pktHdr.ts_sec;
	packet->ts_.tv_usec = pktHdr.ts_usec;
	packet->buf_ = GBuf(recvBuf_, len);
	if (autoParse_)
		packet->parse();
	return GPacket::Ok;
}

GPacket::Result GPcapPipe::write(GBuf buf) {
	(void)buf;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

GPacket::Result GPcapPipe::write(GPacket* packet) {
	(void)packet;
	SET_ERR(GErr::NOT_SUPPORTED, "not supported");
	return GPacket::Fail;
}

qint64 GPcapPipe::recvAll(char *data, size_t size) {
	char* p = data;
	qint64 remain = size;

	bool debug = false; // gilgil temp 2012.06.17
	while (true) {
		if (state_ != Opening && state_ != Opened) return -1;

		qint64 recvLen;
		if (removeCr_ && removeCrLastBytesBuffered_) {
			removeCrLastBytesBuffered_ = false;
			*p = removeCrLastBytes_;
			recvLen = 1;
		} else {
			qint64 available = process_->bytesAvailable();
			if (debug) qDebug() << QString("bytesAvailable return %1 size=%2 remain=%3").arg(available).arg(size).arg(remain);
			if (available < qint64(remain)) {
				bool ready = process_->waitForReadyRead(readTimeout_);
				if (debug) qDebug() << QString("waitForReadyRead(%1) return %2 size=%3 remain=%4").arg(readTimeout_).arg(ready).arg(size).arg(remain);
				if (!ready) {
					QProcess::ProcessState state = process_->state();
					if (debug) qDebug() << QString("proce state is %1").arg(int(state));
					if (state != QProcess::Running) {
						qDebug() << QString("process state is %1").arg(int(state));
						return -1;
					}
					continue;
				}
			}

			recvLen = process_->read(p, remain);
			if (debug) qDebug() << QString("read return %1 size=%2 remain=%3").arg(recvLen).arg(size).arg(remain);
			if (recvLen == 0) {
				qWarning() << "process_->read return zero"; QThread::sleep(1); // gilgil temp 2021.06.17
				debug = true;
				continue;
			}
			if (recvLen == -1) {
				SET_ERR(GErr::FAIL, QString("process_->read return %1").arg(recvLen));
				return -1;
			}
		}

		if (removeCr_) {
			char* begin = p;
			char* end = begin + recvLen - 1;
			while (begin < end) {
				uint16_t two = *reinterpret_cast<uint16_t*>(begin);
				if (ntohs(two) == 0x0d0a) { // \r\n
					char* shift = begin;
					while (shift < end) {
						*shift = *(shift + 1);
						shift++;
					}
					recvLen--;
				}
				begin++;
			}
			if (p[recvLen - 1] == 0x0d) {
				if (process_->bytesAvailable() > 0) {
					char c;
					if (process_->read(&c, 1) == 1) {
						if (c == 0x0a) {
							p[recvLen - 1] = 0x0a;
							removeCrLastBytesBuffered_ = false;
						} else {
							removeCrLastBytesBuffered_ = true;
							removeCrLastBytes_ = c;
						}
					}
				}
			}
		}

		p += recvLen;
		remain -= recvLen; Q_ASSERT(remain >= 0);
		if (remain == 0) break;
	}
	return size;
}
