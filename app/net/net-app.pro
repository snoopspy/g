TEMPLATE = subdirs
SUBDIRS += \
	arprecover \
	ha \
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
	ssdemon \
	wa
}

win32 {
SUBDIRS += \
	getipforwardtable-test \
	getadaptersinfo-test
}
