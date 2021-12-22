TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
LIBS += -lpcap

HEADERS += \
	ccorepcap.h \
	cobj.h \
	cpacket.h \
	cpcap.h \
	cpcapdevice.h \
	cpcapdevicewrite.h \
	cpcapfile.h \
	cpcapfilewrite.h \
	gtrace.h

SOURCES += \
	ccorepcap.cpp \
	cmain.cpp \
	cobj.cpp \
	cpacket.cpp \
	cpcap.cpp \
	cpcapdevice.cpp \
	cpcapdevicewrite.cpp \
	cpcapfile.cpp \
	cpcapfilewrite.cpp \
	gtrace.cpp
