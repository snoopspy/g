TEMPLATE = app
CONFIG += console c++17
CONFIG -= app_bundle
CONFIG -= qt
DESTDIR = $${PWD}/../../../bin
LIBS += -lpcap -lgtest_main -lgtest -pthread
DEFINES *= GTEST

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
	cpobj.cpp \
	cppacket.cpp \
	cppcap.cpp \
	cppcapdevice.cpp \
	cppcapdevicewrite.cpp \
	cppcapfile.cpp \
	cppcapfilewrite.cpp \
	gtrace.cpp
