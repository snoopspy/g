.PHONY: all lib test debug release debug-test release-test libg-d libg-gui-d libg libg-gui g-d-test g-gui-d-test g-test g-gui-test

NPROC=$(shell grep -c ^processor /proc/cpuinfo)

all: lib #test

lib: release #debug

test: debug-test release-test

debug: libg-d libg-gui-d

release: libg libg-gui

debug-test: g-d-test g-gui-d-test

release-test: g-test g-gui-test

libg-d:
	mkdir -p build-libg-d;       cd build-libg-d;        qmake ../libg.pro          CONFIG+=debug;   make -j$(NPROC)
	
libg-gui-d:
	mkdir -p build-libg-gui-d;   cd build-libg-gui-d;   qmake ../libg-gui.pro      CONFIG+=debug;   make -j$(NPROC)

libg:
	mkdir -p build-libg;         cd build-libg;         qmake ../libg.pro          CONFIG+=release; make -j$(NPROC)

libg-gui:
	mkdir -p build-libg-gui;     cd build-libg-gui;     qmake ../libg-gui.pro      CONFIG+=release; make -j$(NPROC)

g-d-test:
	mkdir -p build-g-d-test;     cd build-g-d-test;     qmake ../libg-test.pro     CONFIG+=debug;   make -j$(NPROC)

g-gui-d-test:
	mkdir -p build-g-gui-d-test; cd build-g-gui-d-test; qmake ../libg-gui-test.pro CONFIG+=debug;   make -j$(NPROC)

g-test:
	mkdir -p build-g-test;       cd build-g-test;       qmake ../libg-test.pro     CONFIG+=release; make -j$(NPROC)

g-gui-test:
	mkdir -p build-g-gui-test;   cd build-g-gui-test;   qmake ../libg-gui-test.pro CONFIG+=release; make -j$(NPROC)

clean:
	rm -rf build-libg-d
	rm -rf build-libg-gui-d
	rm -rf build-libg
	rm -rf build-libg-gui
	rm -rf build-g-d-test
	rm -rf build-g-gui-d-test
	rm -rf build-g-test
	rm -rf build-g-gui-test
	rm -rf *.pro.user

distclean: clean
	find ../bin -name "*.a" -delete
	find ../bin -name "*.so*" -delete
	find ../bin -type f -executable -name "*-test" -delete
	rm -f ../bin/g-d-test
	rm -f ../bin/g-gui-d-test
	rm -f ../bin/g-test
	rm -f ../bin/g-gui-test
