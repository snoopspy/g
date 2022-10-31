TEMPLATE = subdirs
SUBDIRS += \
	arprecover \
	ha \
	netclient \
	netinfo \
	netserver \
	pcap-exam \
	pcap-findalldevs-test \
	snoopspy \
	sscon \

linux {
SUBDIRS += \
	bf \
	corepcap \
	pa \
	ssdemon \
	wa
}

win32 {
SUBDIRS += \
	getipforwardtable-test \
	getadaptersinfo-test
}
