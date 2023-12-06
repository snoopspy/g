#include "gpcapfile.h"

// ----------------------------------------------------------------------------
// GPcapFile
// ----------------------------------------------------------------------------
bool GPcapFile::doOpen() {
	if (!enabled_) return true;

	if (fileName_ == "") {
		SET_ERR(GErr::FileNameNotSpecified, "file name is not specified");
		return false;
	}

	if (loopCount_ == 0) {
		SET_ERR(GErr::ValueIsZero, "loop count can not be zero");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_offline(qPrintable(fileName_), errBuf);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::ReturnNull, errBuf);
		return false;
	}

	currentFrameNumber_ = 0;
	loopIndex_ = 0;

	frameNumberSet_.clear();
	for (QString s: frameNumbers_) {
		s = s.trimmed();
		if (s.isEmpty()) continue;
		int idx = s.indexOf("=");
		if (idx == -1) {
			int frameNumber = s.toInt();
			if (frameNumber <= 0) {
				SET_ERR(GErr::InvalidValue, QString("number(%1) must be plus number").arg(s));
				return false;
			}
			frameNumberSet_.insert(frameNumber);
		} else {
			QString s1 = s.left(idx -1);
			QString s2 = s.mid(idx + 1);
			int frameNumber1 = s1.toInt();
			int frameNumber2 = s2.toInt();
			if (frameNumber1 <= 0) {
				SET_ERR(GErr::InvalidValue, QString("number(%1) must be plus number").arg(s1));
				return false;
			}
			if (frameNumber2 <= 0) {
				SET_ERR(GErr::InvalidValue, QString("number(%1) must be plus number").arg(s2));
				return false;
			}
			if (frameNumber1 > frameNumber2) {
				SET_ERR(GErr::InvalidValue, QString("number(%1) > number2(%2)").arg(s1).arg(s2));
				return false;
			}
			for (int i = frameNumber1; i <= frameNumber2; i++)
				frameNumberSet_.insert(i);
		}
	}

	return GPcapCapture::doOpen();
}

bool GPcapFile::doClose() {
	if (!enabled_) return true;

	return GPcapCapture::doClose();
}

GPacket::Result GPcapFile::read(GPacket* packet) {
	GPacket::Result res = GPcapCapture::read(packet);
	if (res == GPacket::Eof) {
		if (loopCount_ == -1 || ++loopIndex_ < loopCount_) {
			pcap_close(pcap_);

			char errBuf[PCAP_ERRBUF_SIZE];
			pcap_ = pcap_open_offline(qPrintable(fileName_), errBuf);
			if (pcap_ == nullptr) {
				SET_ERR(GErr::ReturnNull, errBuf);
				return GPacket::Fail;
			}
			currentFrameNumber_ = 0;
			res = GPcapCapture::read(packet);
		}
	}
	if (res == GPacket::Ok) {
		if (!frameNumberSet_.isEmpty())
			if (frameNumberSet_.find(++currentFrameNumber_) == frameNumberSet_.end())
				res = GPacket::None;
	}

	return res;
}

#ifdef QT_GUI_LIB

#include "base/prop/gpropitem-filepath.h"
GPropItem* GPcapFile::propCreateItem(GPropItemParam* param) {
	if (QString(param->mpro_.name()) == "fileName") {
		GPropItemFilePath* res = new GPropItemFilePath(param);
		QStringList filters{"pcap files - *.pcap(*.pcap)", "any files - *(*)"};
		res->fd_->setNameFilters(filters);
		return res;
	}
	return GObj::propCreateItem(param);
}

#endif // QT_GUI_LIB

// ----------------------------------------------------------------------------
// GTEST
// ----------------------------------------------------------------------------
#ifdef GTEST
#include <gtest/gtest.h>

TEST(GPcapFile, noFileTest) {
	GPcapFile file;
	file.fileName_ = "no_file.pcap";

	ASSERT_FALSE(file.open());
}

#endif // GTEST
