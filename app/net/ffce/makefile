TARGET=../../../bin/ffce
CFILES=$(wildcard *.c)
CPPFILES=$(wildcard *.cpp) ../gaux.cpp
SRCS=$(CFILES) $(CPPFILES)
OBJS=$(CFILES:%.c=%.o)
OBJS+=$(CPPFILES:%.cpp=%.o)

CPPFLAGS+=-g -I..
CPPFLAGS+=-O2 # for release
CPPFLAGS+=-Wall -Wextra
ifdef ANDROID_NDK_ROOT
LDFLAGS+=-static-libstdc++
endif
LDLIBS+=-lpcap -pthread

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LINK.cpp) $^ $(LOADLIBES) $(LDLIBS) -o $@
ifdef ANDROID_NDK_ROOT
	termux-elf-cleaner --api-level 23 $(TARGET)
endif	

clean:
	rm -f $(TARGET) $(OBJS)

test:
	@echo $(SRCS)
	@echo $(OBJS)

