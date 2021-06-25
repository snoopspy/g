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
move app\net\arprecover\makefile app\net\arprecover\makefile.gilgil
cd app
	qmake "CONFIG+=release" 
	mingw32-make -j4
	cd ..
del app\net\arprecover\Makefile	
move app\net\arprecover\makefile.gilgil app\net\arprecover\makefile

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
copy ..\..\bin\arprecover.exe . & strip arprecover.exe
copy ..\..\bin\netclient.exe . & strip netclient.exe
copy ..\..\bin\netserver.exe . & strip netserver.exe
copy ..\..\bin\snoopspy.exe . & strip snoopspy.exe
copy ..\..\bin\sscon.exe . & strip sscon.exe
copy ..\..\bin\sslog.exe . & strip sslog.exe
copy ..\..\bin\WinDivert.dll .
copy ..\..\bin\WinDivert64.sys .
cp ../../setup/setup-win.txt .
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
set /p VERSION=<..\..\version.txt
set VERSION=%VERSION:"=%
"C:\Program Files\7-Zip\7z.exe" a ..\snoopspy-%VERSION%.zip *
cd ..
cd ..
