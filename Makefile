CC = clang
CFLAGS = -fPIC -I/usr/local/include/SDL2 -D_THREAD_SAFE -D_WD=\"$(shell pwd)\"
WFLAGS = -Wall -Wno-missing-braces -Wno-unused-function -O0 -DDEBUG -g
LIBS = -ldl -lm -lSDL2 -lSDL2_ttf -L/usr/local/lib 
TARGET = tetris
SRCDIR = src/
BUILDDIR = bin/

.PHONY: clean tags

default: $(TARGET)

all: default

.PRECIOUS: $(TARGET)

$(TARGET): game
	$(CC) $(CFLAGS) $(WFLAGS) $(SRCDIR)main.c -o $(BUILDDIR)$(TARGET) -Wl,-rpath='$$ORIGIN' $(LIBS)

game:
	$(CC) $(CFLAGS) $(WFLAGS) $(SRCDIR)game.c -shared -o $(BUILDDIR)libgame.so -Wl,-soname,libgame.so $(LIBS)

tags:
	ctags -R .

clean:
	-rm -rf bin/*.dSYM
	-rm -f bin/*.dylib
	-rm -f bin/*.so
	-rm -f bin/$(TARGET)
