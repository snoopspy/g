QT += widgets svg network sql
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

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

RESOURCES += $$PWD/ha.qrc

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
	ANDROID_PACKAGE_SOURCE_DIR = \
		$$PWD/android
}

DISTFILES += \
	android/AndroidManifest.xml \
	android/build.gradle \
	android/gradle.properties \
	android/gradle/wrapper/gradle-wrapper.jar \
	android/gradle/wrapper/gradle-wrapper.properties \
	android/gradlew \
	android/gradlew.bat \
	android/res/drawable-hdpi/icon.png \
	android/res/drawable-ldpi/icon.png \
	android/res/drawable-mdpi/icon.png \
	android/res/drawable-xhdpi/icon.png \
	android/res/drawable-xxhdpi/icon.png \
	android/res/drawable-xxxhdpi/icon.png \
	android/res/values/libs.xml

#------------------------------------------------------------------------------
# android
#------------------------------------------------------------------------------
android {
	deployment.files += $${G_DIR}/bin/arprecover
	deployment.files += $${G_DIR}/bin/ssdemon
	deployment.path = /assets
	INSTALLS += deployment
}
