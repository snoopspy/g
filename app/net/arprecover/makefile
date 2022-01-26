TARGET=../../../bin/arprecover
SRCS=$(wildcard *.cpp) ../gaux.cpp
OBJS=$(SRCS:%.cpp=%.o)

CPPFLAGS+=-g -I..
CPPFLAGS+=-O2 # for release
CPPFLAGS+=-Wall -Wextra
LDLIBS+=-lpcap -pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f $(TARGET) $(OBJS)

install:
	sudo cp $(TARGET) /usr/sbin

uninstall:
	sudo rm -f /usr/sbin/arprecover

android-install:
	adb push $(TARGET) /data/local/tmp
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'cp /data/local/tmp/arprecover /system/xbin'"
	adb exec-out "su -c 'chmod 755 /system/xbin/arprecover'"
	adb exec-out "su -c 'mount -o ro,remount /system'"
	adb exec-out "rm /data/local/tmp/arprecover"

android-uninstall:
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'rm -f /system/xbin/arprecover'"
	adb exec-out "su -c 'mount -o ro,remount /system'"

test:
	@echo $(SRCS)
	@echo $(OBJS)
