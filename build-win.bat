rem
rem lib
rem
cd lib
	mkdir build-libg
	cd build-libg
	qmake ../libg.pro "CONFIG+=release"
	mingw32-make -j%NUMBER_OF_PROCESSORS%
	cd ..
	mkdir build-libg-gui
	cd build-libg-gui
	qmake ../libg-gui.pro "CONFIG+=release"
	mingw32-make -j%NUMBER_OF_PROCESSORS%
	cd ..
cd ..

rem
rem app
rem
move app\net\arprecover\makefile app\net\arprecover\makefile.gilgil
move app\net\ffce\makefile app\net\ffce\makefile.gilgil
cd app
	qmake "CONFIG+=release" 
	mingw32-make -j%NUMBER_OF_PROCESSORS%
	cd ..
del app\net\arprecover\Makefile	
move app\net\arprecover\makefile.gilgil app\net\arprecover\makefile
move app\net\ffce\makefile.gilgil app\net\ffce\makefile

rem
rem plugin
rem
cd plugin
	qmake "CONFIG+=release"
	mingw32-make -j%NUMBER_OF_PROCESSORS%
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
copy ..\..\bin\ch.exe . & strip ch.exe
copy ..\..\bin\ffce.exe . & strip ffce.exe
copy ..\..\bin\ha.exe . & strip ha.exe
copy ..\..\bin\netclient.exe . & strip netclient.exe
copy ..\..\bin\netserver.exe . & strip netserver.exe
copy ..\..\bin\snoopspy.exe . & strip snoopspy.exe
copy ..\..\bin\sscon.exe . & strip sscon.exe
copy ..\..\bin\sslog.exe . & strip sslog.exe
copy ..\..\bin\WinDivert.dll .
copy ..\..\bin\WinDivert64.sys .
mkdir cert\root
copy ..\..\bin\cert\root\* cert\root
copy ..\..\bin\cert\* cert\
mkdir ss
copy ..\..\bin\ss\* ss\
copy ..\..\setup\setup-win.txt .

rem
rem qt files
rem
set QTDIR=C:\Qt\6.5.3\mingw_64
copy %QTDIR%\bin\Qt6Core.dll .
copy %QTDIR%\bin\Qt6Gui.dll .
copy %QTDIR%\bin\Qt6Network.dll .
copy %QTDIR%\bin\Qt6Sql.dll .
copy %QTDIR%\bin\Qt6Svg.dll .
copy %QTDIR%\bin\Qt6Widgets.dll .


#
# plugins files
#
mkdir platforms
copy %QTDIR%\plugins\platforms\*.dll platforms\
mkdir tls
copy %QTDIR%\plugins\tls\*.dll tls\
mkdir plugins\sqldrivers
copy %QTDIR%\plugins\sqldrivers\qsqlite.dll plugins\sqldrivers\

rem
rem mingw files
rem
set MINGW_DIR=C:\Qt\6.5.3\mingw_64\bin
copy %QTDIR%\bin\libgcc_s_seh-1.dll .
copy %QTDIR%\bin\"libstdc++-6.dll" .
copy %QTDIR%\bin\libwinpthread-1.dll .

rem
rem compress
rem 
set /p VERSION=<..\..\version.txt
set VERSION=%VERSION:"=%
"C:\Program Files\7-Zip\7z.exe" a ..\snoopspy-win-%VERSION%.zip *
cd ..
cd ..
