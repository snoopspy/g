DEFINES *= GTEST
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt
INCLUDEPATH += ..
DESTDIR = $${PWD}/../bin
LIBS += -lpcap -lgtest_main -lgtest -pthread

HEADERS += \
	../gtrace.h \
	lcorepcap.h \
	lobj.h \
	lpacket.h \
	lpcap.h \
	lpcapdevice.h \
	lpcapdevicewrite.h \
	lpcapfile.h \
	lpcapfilewrite.h

SOURCES += \
	../gtrace.cpp \
	lcorepcap.cpp \
	lobj.cpp \
	lpacket.cpp \
	lpcap.cpp \
	lpcapdevice.cpp \
	lpcapdevicewrite.cpp \
	lpcapfile.cpp \
	lpcapfilewrite.cpp
