#include "gipadapterinfo.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GIpAdapterInfo
// ----------------------------------------------------------------------------
GIpAdapterInfo::GIpAdapterInfo() {

    pAdapterInfo_ = PIP_ADAPTER_INFO(malloc(sizeof(IP_ADAPTER_INFO)));
	if (pAdapterInfo_ == nullptr) {
		qCritical() << "Error allocating memory needed to call GetAdaptersinfo";
		return;
	}

	// Make an initial call to GetAdaptersInfo to get
	// the necessary size into the ulOutBufLen variable
    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	if (GetAdaptersInfo(pAdapterInfo_, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
		free(pAdapterInfo_);
		pAdapterInfo_ = PIP_ADAPTER_INFO(malloc(ulOutBufLen));
		if (pAdapterInfo_ == nullptr) {
			qCritical() << "Error allocating memory needed to call GetAdaptersinfo";
			return;
		}
	}
	DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo_, &ulOutBufLen);
	if (dwRetVal != NO_ERROR) {
		qCritical() << "GetAdaptersInfo failed with error: " << dwRetVal;
		free(pAdapterInfo_);
		pAdapterInfo_ = nullptr;
	}
}

GIpAdapterInfo::~GIpAdapterInfo() {
	if (pAdapterInfo_ != nullptr) {
		free(pAdapterInfo_);
		pAdapterInfo_ = nullptr;
	}
}

PIP_ADAPTER_INFO GIpAdapterInfo::findByAdapterName(char* value) {
	QString name = value;
	PIP_ADAPTER_INFO p = pAdapterInfo_;
	while (p != nullptr) {
		QString adapterName = p->AdapterName;
		if (name.indexOf(adapterName) != -1)
			return p;
		p = p->Next;
	}
	return nullptr;
}

PIP_ADAPTER_INFO GIpAdapterInfo::findByComboIndex(DWORD comboIndex) {
	PIP_ADAPTER_INFO p = pAdapterInfo_;
	while (p != nullptr) {
		if (comboIndex == p->ComboIndex)
			return p;
		p = p->Next;
	}
	return nullptr;
}
