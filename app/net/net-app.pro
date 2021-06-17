TEMPLATE = subdirs
SUBDIRS += \
	netclient \
	netinfo \
	netserver \
	pcap-exam \
	pcap-findalldevs-test \
	snoopspy \
	sscon \

linux {
SUBDIRS += \
	corepcap
	ssdemon
}

win32 {
SUBDIRS += \
	getipforwardtable-test \
	getadaptersinfo-test
}
