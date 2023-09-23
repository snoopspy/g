export QTDIR=/opt/Qt/5.15.2/gcc_64

make

cd setup

rm -rf linux
mkdir -p linux
cd linux

#
# g files 
#
cp ../../bin/arprecover .; strip arprecover
cp ../../bin/bf .; strip bf
cp ../../bin/corepcap .; strip corepcap
cp ../../bin/ha .; strip ha
cp ../../bin/netclient .; strip netclient
cp ../../bin/netserver .; strip netserver
cp ../../bin/pa .; strip pa
cp ../../bin/snoopspy .; strip snoopspy
cp ../../bin/sscon .; strip sscon
cp ../../bin/sslog .; strip sslog
cp ../../bin/wa .; strip wa
cp ../../setup/setup-linux.sh .
mkdir ss
cp ../../bin/ss/* ss/

#
# qt files
#
cp $QTDIR/lib/libicudata.so.56 .
cp $QTDIR/lib/libicui18n.so.56 .
cp $QTDIR/lib/libicuuc.so.56 .
cp $QTDIR/lib/libQt5Core.so.5 .
cp $QTDIR/lib/libQt5DBus.so.5 .
cp $QTDIR/lib/libQt5Gui.so.5 .
cp $QTDIR/lib/libQt5Network.so.5 .
cp $QTDIR/lib/libQt5Sql.so.5 .
cp $QTDIR/lib/libQt5Widgets.so.5 .
cp $QTDIR/lib/libQt5XcbQpa.so.5 .

#
# platforms files
#
mkdir -p platforms
cp $QTDIR/plugins/platforms/* platforms/
mkdir -p sqldrivers
cp $QTDIR/plugins/sqldrivers/libqsqlite.so sqldrivers/

#
# compress
#
tar czf ../snoopspy-$(sed 's/"//g' ../../version.txt).tar.gz *
cd ..
cd ..
