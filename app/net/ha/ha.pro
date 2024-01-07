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
	QrCodeGenerator.cpp \
	qrcodedialog.cpp \
	qrcodegen/qrcodegen.cpp


HEADERS += \
	dbdialog.h \
	hawidget.h \
	hostanalyzer.h \
	hostdialog.h \
	QrCodeGenerator.h \
	qrcodedialog.h \
	qrcodegen/qrcodegen.h

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
