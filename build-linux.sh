make

#
# setup
#
mkdir -p setup
cd setup
rm -rf linux
mkdir -p linux
cd linux

#
# g files 
#
cp ../../bin/arprecover .; strip arprecover
cp ../../bin/bf .; strip bf
cp ../../bin/ch .; strip ch
cp ../../bin/corepcap .; strip corepcap
cp ../../bin/ffce .; strip ffce
cp ../../bin/ha .; strip ha
cp ../../bin/netclient .; strip netclient
cp ../../bin/netserver .; strip netserver
cp ../../bin/pa .; strip pa
cp ../../bin/snoopspy .; strip snoopspy
cp ../../bin/sscon .; strip sscon
cp ../../bin/sslog .; strip sslog
cp ../../bin/wa .; strip wa
mkdir -p cert/root
cp ../../bin/cert/root/* cert/root/
cp ../../bin/cert/* cert/
mkdir -p ss
cp ../../bin/ss/* ss/
cp ../../setup/setup-linux.sh .
cp ../../setup/run .

#
# qt files
#
export QTDIR=/opt/Qt/6.5.3/gcc_64
cp $QTDIR/lib/libicudata.so.56 .
cp $QTDIR/lib/libicui18n.so.56 .
cp $QTDIR/lib/libicuuc.so.56 .
cp $QTDIR/lib/libQt6Core.so.6 .
cp $QTDIR/lib/libQt6DBus.so.6 .
cp $QTDIR/lib/libQt6Gui.so.6 .
cp $QTDIR/lib/libQt6Network.so.6 .
cp $QTDIR/lib/libQt6OpenGL.so.6 .
cp $QTDIR/lib/libQt6OpenGL.so.6 .
cp $QTDIR/lib/libQt6Sql.so.6 .
cp $QTDIR/lib/libQt6Svg.so.6 .
cp $QTDIR/lib/libQt6Widgets.so.6 .
cp $QTDIR/lib/libQt6XcbQpa.so.6 .

#
# plugins files
#
mkdir -p platforms
cp $QTDIR/plugins/platforms/* platforms/
mkdir -p tls
cp $QTDIR/plugins/tls/* tls/
mkdir -p plugins/sqldrivers
cp $QTDIR/plugins/sqldrivers/libqsqlite.so plugins/sqldrivers/

#
# compress
#
tar czf ../snoopspy-linux-$(sed 's/"//g' ../../version.txt).tar.gz *
cd ..
cd ..
