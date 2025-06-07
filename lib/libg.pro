TEMPLATE = lib
CONFIG += staticlib
QT -= gui
QT += sql
CONFIG += G_BUILD
DEFINES += G_BUILD
include(../g.pri)
TARGET = $${G_NAME}
DESTDIR = $${PWD}/../bin
include(libg-files.pri)
