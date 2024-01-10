QT += widgets svg network
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

RESOURCES += $$PWD/ha.qrc

SOURCES += \
	dbdialog.cpp \
	ha.cpp \
	hawidget.cpp \
	hostanalyzer.cpp \
	hostdialog.cpp \
	qrcodedialog.cpp \
	webserver.cpp \
	qrcode/QrCodeGenerator.cpp \
	qrcode/qrcodegen/qrcodegen.cpp


HEADERS += \
	dbdialog.h \
	hawidget.h \
	hostanalyzer.h \
	hostdialog.h \
	qrcodedialog.h \
	webserver.h \
	qrcode/QrCodeGenerator.h \
	qrcode/qrcodegen/qrcodegen.h

DISTFILES += \
	android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle.properties \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
	android/res/values/libs.xml

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
