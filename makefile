.PHONY: all lib app plugin setup clean distclean

NPROC=$(shell grep -c ^processor /proc/cpuinfo)

all: lib app plugin

lib:
	cd lib && make

app:
	cd app && qmake "CONFIG+=release" && make -j$(NPROC)

plugin:
	cd plugin && qmake "CONFIG+=release" && make -j$(NPROC)

clean:
	cd lib && make clean; true
	cd app && make clean; true
	cd plugin && make clean; true
	find -type d -name 'build-*'    -exec rm -r {} \; | true
	find -type f -name '*.o'        -delete
	find -type f -name '*.pro.user' -delete

distclean: clean
	cd lib && make distclean; true
	cd app && make distclean; true
	cd plugin && make distclean; true
	find bin -type f -executable -delete
	find bin -type f -name "*.json" -delete
	find -type f -name 'Makefile*'  -delete
	rm -rf setup/linux
	rm -rf lib/android-build
	rm -rf app/net/snoopspy/android-build
	#rm -rf setup/*.gz
	#rm -rf setup/*.apk

