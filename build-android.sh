export QTBINDIR=/opt/Qt/5.15.2/android/bin
export QMAKE_OPTION="-spec android-clang CONFIG+=release ANDROID_ABIS=armeabi-v7a"
export MAKEDIR=$ANDROID_NDK_ROOT/prebuilt/linux-x86_64/bin

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
	mkdir -p android-build
	cd android-build
	$QTBINDIR/qmake ../libg-gui.pro $QMAKE_OPTION
	$MAKEDIR/make -j$(nproc)
	cd ../..
fi

#
# app
#
if true; then
	mkdir -p app/net/snoopspy/android-build
	cd app/net/snoopspy/android-build
	$QTBINDIR/qmake ../snoopspy.pro $QMAKE_OPTION
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./android-build install
	export ANDROID_SDK_ROOT=/root/sdk && $QTBINDIR/androiddeployqt --input android-snoopspy-deployment-settings.json --output ./android-build --android-platform android-30 --jdk /usr/lib/jvm/jdk8u275-b01 --gradle
	cp android-build/build/outputs/apk/debug/android-build-debug.apk ../../../../setup/snoopspy-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi