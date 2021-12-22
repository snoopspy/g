TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
LIBS += -lpcap

HEADERS += \
	cpcorepcap.h \
	cpobj.h \
	cppacket.h \
	cppcap.h \
	cppcapdevice.h \
	cppcapdevicewrite.h \
	cppcapfile.h \
	cppcapfilewrite.h \
	gtrace.h

SOURCES += \
	cpcorepcap.cpp \
	cpmain.cpp \
	cpobj.cpp \
	cppacket.cpp \
	cppcap.cpp \
	cppcapdevice.cpp \
	cppcapdevicewrite.cpp \
	cppcapfile.cpp \
	cppcapfilewrite.cpp \
	gtrace.cpp
