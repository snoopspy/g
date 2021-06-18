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
	mingw32-make -j4
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

cd setup
mkdir win
cd win
rem
rem g files
rem
copy ..\..\bin\netclient.exe . & strip netclient.exe
copy ..\..\bin\netserver.exe . & strip netserver.exe
copy ..\..\bin\snoopspy.exe . & strip snoopspy.exe
copy ..\..\bin\sscon.exe . & strip sscon.exe
copy ..\..\bin\WinDivert.dll . 
copy ..\..\bin\WinDivert64.sys . 
mkdir ss
copy ..\..\bin\ss\* ss\

rem
rem mingw files
rem
set MINGW_DIR=C:\Qt\5.15.2\mingw81_64\bin
copy %MINGW_DIR%\libgcc_s_seh-1.dll .
copy %MINGW_DIR%\libwinpthread-1.dll .
copy %MINGW_DIR%\libstdc++-6.dll .

rem
rem qt files
rem
set QT_DIR=C:\Qt\5.15.2\mingw81_64
copy %QT_DIR%\bin\Qt5Core.dll .
copy %QT_DIR%\bin\Qt5Gui.dll .
copy %QT_DIR%\bin\Qt5Widgets.dll .
mkdir platforms
copy %QT_DIR%\plugins\platforms\*.dll platforms\

rem
rem compress
rem 
tar czf ..\snoopspy-win-v.tar.gz *
cd ..
cd ..
