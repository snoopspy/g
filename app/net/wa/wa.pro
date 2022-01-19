QT += widgets
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

SOURCES += \
	wa.cpp \
	wifianalyzer.cpp \
	wawidget.cpp

HEADERS += \
	wifianalyzer.h \
	wawidget.h
