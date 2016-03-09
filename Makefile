SHELL  = /bin/bash
TARGET = tetris

.PHONY: default all clean

default: $(TARGET)

all: default

.PRECIOUS: $(TARGET)

$(TARGET):
	./build.sh

run: $(TARGET)
	@if [ -d "./data" ]; \
	then \
		pushd data && ../bin/$(TARGET) && popd; \
	else \
		pushd ../data && ../bin/$(TARGET) && popd; \
	fi

gdb: $(TARGET)
	@if [ -d "./data" ]; \
	then \
		pushd data && gdb ../bin/$(TARGET) && popd; \
	else \
		pushd ../data && gdb ../bin/$(TARGET) && popd; \
	fi

clean:
	-rm -rf bin/*.dSYM
	-rm -f bin/*.dylib
	-rm -f bin/*.so
	-rm -f bin/$(TARGET)
