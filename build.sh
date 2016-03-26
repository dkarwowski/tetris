#!/bin/bash

CC=clang
APPNAME=tetris
CFLAGS="-fPIC -I/usr/local/include/SDL2 -D_THREAD_SAFE"
CFLAGS="$CFLAGS -Wall -Wno-missing-braces"
CFLAGS="$CFLAGS -O0 -DDEBUG -g -Wno-unused-function"
LIBS="-lm -L/usr/local/lib -lSDL2 -lSDL2_ttf -lSDL2_gfx"

if [[ "$(pwd)" == *"src" ]]
then
    mkdir -p ../bin
    pushd ../bin
else
    mkdir -p ./bin
    pushd ./bin
fi

if [[ "$OSTYPE" == "darwin"* ]]
then
    $CC $CFLAGS -dynamiclib -install_name @rpath/../bin/libgame.so ../src/game.c -o libgame.so $LIBS

    $CC $CFLAGS ../src/main.c -o $APPNAME -L$(pwd) -rpath $(pwd) -lgame -ldl $LIBS
else
    $CC $CFLAGS ../src/game.c -shared -o libgame.so -Wl,-soname,libgame.so $LIBS

    $CC $CFLAGS ../src/main.c -o $APPNAME -Wl,-rpath=$(pwd) -ldl $LIBS
fi

popd
