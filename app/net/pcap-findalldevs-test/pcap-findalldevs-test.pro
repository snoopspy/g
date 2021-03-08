CONFIG -= qt
CONFIG += console
#------------------------------------------------------------------------------
# pcap
#------------------------------------------------------------------------------
win32 {
	INCLUDEPATH *= $${PWD}/../../../../npcap/Include
	LIBS += -L$${PWD}/../../../../npcap/Lib/x64
	LIBS *= -lwpcap -lpacket -lws2_32 -liphlpapi
}
linux : LIBS += -lpcap
DESTDIR = $${PWD}/../../../bin
SOURCES += *.cpp
