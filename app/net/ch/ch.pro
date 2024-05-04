QT += widgets network
include(../../../g.pri)
DESTDIR = $$PWD/../../../bin

SOURCES += \
	ch.cpp \
	chwidget.cpp \
	cookiehijack.cpp \
	dbdialog.cpp \
	webserver.cpp

HEADERS += \
	chwidget.h \
	cookiehijack.h \
	dbdialog.h \
	webserver.h

RESOURCES += $${PWD}/ch.qrc

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
	android/res/values/libs.xml

#------------------------------------------------------------------------------
# android
#------------------------------------------------------------------------------
android {
	deployment.files += $${G_DIR}/bin/arprecover
	deployment.files += $${G_DIR}/bin/ffce
	deployment.files += $${G_DIR}/bin/ssdemon
	deployment.files += $${G_DIR}/bin/cert/root/root.crt
	deployment.files += $${G_DIR}/bin/cert/root/root.key
	deployment.files += $${G_DIR}/bin/cert/default.crt
	deployment.files += $${G_DIR}/bin/cert/default.key
	deployment.path = /assets
	INSTALLS += deployment

	include(/root/Android/Sdk/android_openssl/openssl.pri)
}
