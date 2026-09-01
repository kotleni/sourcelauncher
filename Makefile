CC = i686-w64-mingw32-gcc
WINDRES = i686-w64-mingw32-windres
CFLAGS = -O2 -Wall -mwindows
LIBS = -lcomctl32 -luser32 -lkernel32 -lshell32

TARGET = build/sourcelauncher.exe
SRC = launcher.c
RC = sourcelauncher.rc
RCOBJ = build/sourcelauncher.o

all: $(TARGET)

$(TARGET): $(SRC) $(RCOBJ) resource.h config.h
	mkdir -p build
	$(CC) $(CFLAGS) -o $@ $(SRC) $(RCOBJ) $(LIBS)

$(RCOBJ): $(RC) resource.h
	mkdir -p build
	$(WINDRES) -i $< -o $@

clean:
	rm -rf build
