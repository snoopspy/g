TARGET=../../../bin/ssdemon
SRCS=$(wildcard *.cpp) ../gaux.cpp ../../../src/net/demon/gdemon.cpp
OBJS=$(SRCS:%.cpp=%.o)

CPPFLAGS+=-g -I.. -I../../../src
CPPFLAGS+=-O2 # for release
CPPFLAGS+=-Wall -Wextra
ifdef ANDROID_NDK_ROOT
LDFLAGS+=-static-libstdc++
endif
LDLIBS+=-lpcap -pthread -lnetfilter_queue -lmnl -lnfnetlink

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@
ifdef ANDROID_NDK_ROOT
	termux-elf-cleaner --api-level 23 $(TARGET)
endif	

clean:
	rm -f $(TARGET) $(OBJS)

install:
	sudo cp $(TARGET) /usr/sbin

uninstall:
	sudo rm -f /usr/sbin/ssdemon

android-install:
	adb push $(TARGET) /data/local/tmp
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'cp /data/local/tmp/ssdemon /system/xbin'"
	adb exec-out "su -c 'chmod 755 /system/xbin/ssdemon'"
	adb exec-out "su -c 'mount -o ro,remount /system'"
	adb exec-out "rm /data/local/tmp/ssdemon"

android-uninstall:
	adb exec-out "su -c 'mount -o rw,remount /system'"
	adb exec-out "su -c 'rm -f /system/xbin/ssdemon'"
	adb exec-out "su -c 'mount -o ro,remount /system'"

test:
	@echo $(SRCS)
	@echo $(OBJS)
