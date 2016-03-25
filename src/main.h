#ifndef _MAIN_H_

#include "common.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#ifdef WIN_BUILD
#include <windows.h>
#else
#include <sys/mman.h>
#include <sys/stat.h>
#include <dlfcn.h>
#endif

#define GOAL_FPS 60

#define Kilobytes(x) (x * 1024)
#define Megabytes(x) (Kilobytes(x) * 1024)
#define Gigabytes(x) (Megabytes(x) * 1024)

#define SCREEN_WIDTH 960
#define SCREEN_HEIGHT 540

typedef struct {
    bool endedDown;
    u32 halfCount;
} game_control;

typedef struct {
    bool isConnected;

    union {
        game_control control[3];
        struct {
            game_control left;
            game_control right;
            game_control rotCW;
            game_control rotCCW;
            game_control hardDrop;
            game_control softDrop;
            game_control pause;
            game_control hold;

#ifdef DEBUG
            game_control reload;
#endif

            game_control terminate;
        };
    };

    bool paused;
    double dt;
} game_input;

typedef struct {
    bool isInitialized;

    u64 permMemSize;
    void *permMem;
    u64 tempMemSize;
    void *tempMem;
} game_memory;

#define UPDATE_AND_RENDER(name) void name(game_memory *, game_input *, SDL_Renderer *)
typedef UPDATE_AND_RENDER(upd_and_ren);

typedef struct {
    bool isValid;

#ifdef WIN_BUILD
    FILETIME lastWriteTime;
#else
    ino_t ino;
#endif

    void *gameLib_p;
    upd_and_ren *UpdateAndRender_fp;
} game_lib;

#define _MAIN_H_
#endif
