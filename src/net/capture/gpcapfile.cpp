#include "gpcapfile.h"
#include "base/prop/gpropitem-filepath.h"

// ----------------------------------------------------------------------------
// GPcapFile
// ----------------------------------------------------------------------------
bool GPcapFile::doOpen() {
	if (!enabled_) return true;

	if (fileName_ == "") {
		SET_ERR(GErr::FileNameNotSpecified, "file name is not specified");
		return false;
	}

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_ = pcap_open_offline(qPrintable(fileName_), errBuf);
	if (pcap_ == nullptr) {
		SET_ERR(GErr::ReturnNull, errBuf);
		return false;
	}

	return GPcapCapture::doOpen();
}

bool GPcapFile::doClose() {
	if (!enabled_) return true;

	return GPcapCapture::doClose();
}

#ifdef QT_GUI_LIB

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
