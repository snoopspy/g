rem
rem lib
rem

cd lib
	mkdir libg
	cd libg
	qmake ../libg.pro "CONFIG+=release"
	mingw32-make -j4
	cd ..
	mkdir libg-gui
	cd libg-gui
	qmake ../libg-gui.pro "CONFIG+=release"
	mingw32-make -j4
	cd ..
cd ..

rem
rem app
rem
cd app
	qmake "CONFIG+=release" 
	ingw32-make -j4
	cd ..

rem
rem plugin
rem
cd plugin
	qmake "CONFIG+=release"
	mingw32-make -j4
	cd ..

rem
rem setup
rem


