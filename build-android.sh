export QT_BIN_DIR=/opt/Qt/5.15.2/android/bin
export MAKEDIR=$ANDROID_NDK_ROOT/prebuilt/linux-x86_64/bin
export ANDROID_SDK_ROOT=/root/android/sdk
export ANDROID_HOME=/root/sdk
export JAVA_HOME=/usr/lib/jvm/java-11-openjdk-amd64

#
# arprecover
#
if true; then
	cd app/net/arprecover
	make clean
	make -j$(nproc)
	$ANDROID_STRIP_DIR/strip ../../../bin/arprecover
	cd ../../..
fi

#
# corepcap
#
if true; then
	cd app/net/corepcap
	make clean
	make -j$(nproc)
	$ANDROID_STRIP_DIR/strip ../../../bin/corepcap
	cd ../../..
fi

#
# ssdemon
#
if true; then
	cd app/net/ssdemon
	make clean
	make -j$(nproc)
	$ANDROID_STRIP_DIR/strip ../../../bin/ssdemon
	cd ../../..
fi

#
# lib
#
if true; then
	#
	# lib
	#
	cd lib
	mkdir -p temp-build
	cd temp-build
	$QT_BIN_DIR/qmake ../libg-gui.pro -spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a
	$MAKEDIR/make -j$(nproc)
	cd ../..
fi

#
# snoopspy
#
if true; then
	mkdir -p app/net/snoopspy/temp-build
	cd app/net/snoopspy/temp-build
	$QT_BIN_DIR/qmake ../snoopspy.pro -spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./temp-build install
	$QT_BIN_DIR/androiddeployqt --input android-snoopspy-deployment-settings.json --output ./temp-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp temp-build/build/outputs/apk/debug/temp-build-debug.apk ../../../../setup/snoopspy-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# bf
#
if true; then
	mkdir -p app/net/bf/temp-build
	cd app/net/bf/temp-build
	$QT_BIN_DIR/qmake ../bf.pro -spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./temp-build install
	$QT_BIN_DIR/androiddeployqt --input android-bf-deployment-settings.json --output ./temp-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp temp-build/build/outputs/apk/debug/temp-build-debug.apk ../../../../setup/bf-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# ha
#
if true; then
	mkdir -p app/net/ha/temp-build
	cd app/net/ha/temp-build
	$QT_BIN_DIR/qmake ../ha.pro -spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./temp-build install
	$QT_BIN_DIR/androiddeployqt --input android-ha-deployment-settings.json --output ./temp-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp temp-build/build/outputs/apk/debug/temp-build-debug.apk ../../../../setup/ha-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# pa
#
if true; then
	mkdir -p app/net/pa/temp-build
	cd app/net/pa/temp-build
	$QT_BIN_DIR/qmake ../pa.pro -spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./temp-build install
	$QT_BIN_DIR/androiddeployqt --input android-pa-deployment-settings.json --output ./temp-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp temp-build/build/outputs/apk/debug/temp-build-debug.apk ../../../../setup/pa-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# wa
#
if true; then
	mkdir -p app/net/wa/temp-build
	cd app/net/wa/temp-build
	$QT_BIN_DIR/qmake ../wa.pro -spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./temp-build install
	$QT_BIN_DIR/androiddeployqt --input android-wa-deployment-settings.json --output ./temp-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp temp-build/build/outputs/apk/debug/temp-build-debug.apk ../../../../setup/wa-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

