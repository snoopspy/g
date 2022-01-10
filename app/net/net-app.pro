TEMPLATE = subdirs
SUBDIRS += \
	arprecover \
	netclient \
	netinfo \
	netserver \
	pa \
	pcap-exam \
	pcap-findalldevs-test \
	snoopspy \
	sscon \

linux {
SUBDIRS += \
	corepcap \
	ssdemon
}

win32 {
SUBDIRS += \
	getipforwardtable-test \
	getadaptersinfo-test
}
