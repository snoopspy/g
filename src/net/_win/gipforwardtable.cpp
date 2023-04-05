#include "gipforwardtable.h"
#include <QDebug>

// ----------------------------------------------------------------------------
// GIpForwardTable
// ----------------------------------------------------------------------------
GIpForwardTable::GIpForwardTable() {
    ipForwardTable_ = PMIB_IPFORWARDTABLE(malloc(sizeof(MIB_IPFORWARDTABLE)));
	if (ipForwardTable_ == nullptr) {
		qCritical() << "Error allocating memory";
		return;
	}

	DWORD dwSize = 0;
	if (GetIpForwardTable(ipForwardTable_, &dwSize, 0) ==
			ERROR_INSUFFICIENT_BUFFER) {
		free(ipForwardTable_);
		ipForwardTable_ = PMIB_IPFORWARDTABLE(malloc(dwSize));
		if (ipForwardTable_ == nullptr) {
			qCritical() << "Error allocating memory";
			return;
		}
	}

	DWORD dwRetVal = GetIpForwardTable(ipForwardTable_, &dwSize, 0);
	if (dwRetVal != NO_ERROR) {
		qCritical() << "GetIpForwardTable failed";
		free(ipForwardTable_);
		ipForwardTable_ = nullptr;
	}
}

GIpForwardTable::~GIpForwardTable() {
	if (ipForwardTable_ != nullptr) {
		free(ipForwardTable_);
		ipForwardTable_ = nullptr;
	}
}
