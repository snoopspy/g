QT -= qt gui
CONFIG += console
DESTDIR = $${PWD}/../../../bin
INCLUDEPATH += $${PWD}/../../../src/net/iw $${PWD}/..
# LIBS += -lpcap -lnetfilter_queue # gilgil temp 2022.01.23
SOURCES += *.cpp $${PWD}/../../../src/net/iw/*.cpp $${PWD}/../../../src/net/iw/*.c $${PWD}/../gaux.cpp
HEADERS += *.h $${PWD}/../../../src/net/iw/*.h
