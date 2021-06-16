TEMPLATE = subdirs
SUBDIRS += \
	corepcap \
	netclient \
	netinfo \
	netserver \
	pcap-exam \
	pcap-findalldevs-test \
	snoopspy \
	sscon \
	ssdemon
win32 {
SUBDIRS += \
	getipforwardtable-test \
	getadaptersinfo-test
}
