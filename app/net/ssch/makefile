TARGET=../../../bin/ssch
SRCS=$(wildcard *.cpp) $(wildcard ../../../src/net/demon/iw.cpp)
OBJS=$(SRCS:%.cpp=%.o)

CPPFLAGS+=-g
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
	sudo rm -f /usr/sbin/ssch

android-install:
	adb push $(TARGET) /data/local/tmp
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'cp /data/local/tmp/ssch /system/xbin'"
	adb exec-out "su -c 'chmod 755 /system/xbin/ssch'"
	adb exec-out "su -c 'mount -o ro,remount /system'"
	adb exec-out "rm /data/local/tmp/ssch"

android-uninstall:
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'rm -f /system/xbin/ssch'"
	adb exec-out "su -c 'mount -o ro,remount /system'"

test:
	@echo $(SRCS)
	@echo $(OBJS)
