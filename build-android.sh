#
# arprecover
#
if true; then
	cd app/net/arprecover
	make clean
	make -j$(nproc)
	llvm-strip ../../../bin/arprecover
	cd ../../..
fi

#
# corepcap
#
if true; then
	cd app/net/corepcap
	make clean
	make -j$(nproc)
	llvm-strip ../../../bin/corepcap
	cd ../../..
fi

#
# ssdemon
#
if true; then
	cd app/net/ssdemon
	make clean
	make -j$(nproc)
	llvm-strip ../../../bin/ssdemon
	cd ../../..
fi

#
# ffce
#
if true; then
	cd app/net/ffce
	make clean
	make -j$(nproc)
	llvm-strip ../../../bin/ffce
	cd ../../..
fi

export QTDIR=/opt/Qt/6.5.3/android_armv7
export MAKEDIR=$ANDROID_NDK_ROOT/prebuilt/linux-x86_64/bin
export ANDROID_SDK_ROOT=/root/Android/Sdk
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-amd64

#
# lib
#
if true; then
	#
	# lib
	#
	cd lib
	mkdir -p build-libg
	cd build-libg
	$QTDIR/bin/qmake ../libg-gui.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	cd ../..
fi

#
# snoopspy
#
if true; then
	mkdir -p app/net/snoopspy/build
	cd app/net/snoopspy/build
	$QTDIR/bin/qmake ../snoopspy.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./android-build install
	$QTDIR/../gcc_64/bin/androiddeployqt --input android-snoopspy-deployment-settings.json --output ./android-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp android-build//build/outputs/apk/debug/android-build-debug.apk ../../../../setup/snoopspy-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# bf
#
if true; then
	mkdir -p app/net/bf/build
	cd app/net/bf/build
	$QTDIR/bin/qmake ../bf.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./android-build install
	$QTDIR/../gcc_64/bin/androiddeployqt --input android-bf-deployment-settings.json --output ./android-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp android-build//build/outputs/apk/debug/android-build-debug.apk ../../../../setup/bf-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# ch
#
if true; then
	mkdir -p app/net/ch/build
	cd app/net/ch/build
	$QTDIR/bin/qmake ../ch.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./android-build install
	$QTDIR/../gcc_64/bin/androiddeployqt --input android-ch-deployment-settings.json --output ./android-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp android-build//build/outputs/apk/debug/android-build-debug.apk ../../../../setup/ch-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# ha
#
if true; then
	mkdir -p app/net/ha/build
	cd app/net/ha/build
	$QTDIR/bin/qmake ../ha.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./android-build install
	$QTDIR/../gcc_64/bin/androiddeployqt --input android-ha-deployment-settings.json --output ./android-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp android-build//build/outputs/apk/debug/android-build-debug.apk ../../../../setup/ha-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# pa
#
if true; then
	mkdir -p app/net/pa/build
	cd app/net/pa/build
	$QTDIR/bin/qmake ../pa.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=././android-build install
	$QTDIR/../gcc_64/bin/androiddeployqt --input android-pa-deployment-settings.json --output ./android-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp android-build//build/outputs/apk/debug/android-build-debug.apk ../../../../setup/pa-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi

#
# wa
#
if true; then
	mkdir -p app/net/wa/build
	cd app/net/wa/build
	$QTDIR/bin/qmake ../wa.pro -spec android-clang CONFIG+=release
	$MAKEDIR/make -j$(nproc)
	$MAKEDIR/make INSTALL_ROOT=./android-build install
	$QTDIR/../gcc_64/bin/androiddeployqt --input android-wa-deployment-settings.json --output ./android-build --android-platform android-31 --jdk /usr/lib/jvm/java-11-openjdk-amd64 --gradle
	cp android-build//build/outputs/apk/debug/android-build-debug.apk ../../../../setup/wa-$(sed 's/"//g' ../../../../version.txt).apk
	cd ../../../..
fi
