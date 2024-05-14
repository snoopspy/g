QT += widgets network
include(../../../g.pri)
DESTDIR = $${PWD}/../../../bin
SOURCES += *.cpp
HEADERS += *.h
FORMS += widget.ui
android: include($${ANDROID_SDK_ROOT}/android_openssl/openssl.pri)
