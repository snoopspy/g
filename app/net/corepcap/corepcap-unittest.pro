TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
LIBS += -lpcap -lgtest_main -lgtest -pthread
DEFINES *= GTEST

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
	cobj.cpp \
	cpacket.cpp \
	cpcap.cpp \
	cpcapdevice.cpp \
	cpcapdevicewrite.cpp \
	cpcapfile.cpp \
	cpcapfilewrite.cpp \
	gtrace.cpp
