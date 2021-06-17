TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
LIBS += -lpcap
HEADERS += \
	gtrace.h \
	cpcorepcap.h \
	cpobj.h \
	cppacket.h \
	cppcap.h \
	cppcapdevice.h \
	cppcapdevicewrite.h \
	cppcapfile.h \
	cppcapfilewrite.h

SOURCES += \
	cpmain.cpp \
	gtrace.cpp \
	cpcorepcap.cpp \
	cpobj.cpp \
	cppacket.cpp \
	cppcap.cpp \
	cppcapdevice.cpp \
	cppcapdevicewrite.cpp \
	cppcapfile.cpp \
	cppcapfilewrite.cpp
