#include "main.h"

static void
ProcessKeyboardInput(game_control *oControl_p, game_control *nControl_p, bool isDown)
{
    nControl_p->endedDown = isDown;
    nControl_p->halfCount += (oControl_p->endedDown != nControl_p->endedDown) ? 1 : 0;
}

static bool
HandleEvent(SDL_Event *event_p, game_input *oInput_p, game_input *nInput_p)
{
    bool result = false;

    switch (event_p->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            bool wasDown = event_p->key.repeat != 0 || event_p->key.state == SDL_RELEASED;
            bool isDown  = event_p->key.repeat != 0 || event_p->key.state == SDL_PRESSED;

            if (wasDown != isDown) {
                if (event_p->key.keysym.sym == SDLK_LEFT)
                    ProcessKeyboardInput(&(oInput_p->left), &(nInput_p->left), isDown);
                if (event_p->key.keysym.sym == SDLK_RIGHT)
                    ProcessKeyboardInput(&(oInput_p->right), &(nInput_p->right), isDown);
                if (event_p->key.keysym.sym == SDLK_UP)
                    ProcessKeyboardInput(&(oInput_p->rotCW), &(nInput_p->rotCW), isDown);
                if (event_p->key.keysym.sym == SDLK_DOWN)
                    ProcessKeyboardInput(&(oInput_p->softDrop), &(nInput_p->softDrop), isDown);
                if (event_p->key.keysym.sym == SDLK_SPACE)
                    ProcessKeyboardInput(&(oInput_p->hardDrop), &(nInput_p->hardDrop), isDown);
                if (event_p->key.keysym.sym == SDLK_p)
                    ProcessKeyboardInput(&(oInput_p->pause), &(nInput_p->pause), isDown);
                if (event_p->key.keysym.sym == SDLK_c)
                    ProcessKeyboardInput(&(oInput_p->hold), &(nInput_p->hold), isDown);

#ifdef DEBUG
                if (event_p->key.keysym.sym == SDLK_r)
                    ProcessKeyboardInput(&(oInput_p->reload), &(nInput_p->reload), isDown);
#endif

                if (event_p->key.keysym.sym == SDLK_ESCAPE)
                    ProcessKeyboardInput(&(oInput_p->terminate), &(nInput_p->terminate), isDown);
            }

            if (event_p->key.keysym.sym == SDLK_ESCAPE)
                result = true;
        } break;
        case SDL_WINDOWEVENT:
        {
            if (event_p->window.event == SDL_WINDOWEVENT_FOCUS_LOST ||
                    event_p->window.event == SDL_WINDOWEVENT_HIDDEN) {
                nInput_p->paused = true;
                oInput_p->paused = true;
            }
            if (event_p->window.event == SDL_WINDOWEVENT_FOCUS_GAINED ||
                    event_p->window.event == SDL_WINDOWEVENT_SHOWN) {
                nInput_p->paused = false;
                oInput_p->paused = false;
            }
#ifdef WIN_BUILD
            if (event_p->window.event == SDL_WINDOWEVENT_MOVED) {
                nInput_p->paused = true;
            }
#endif
        } break;
        case SDL_QUIT:
        {
            result = true;
        } break;
        default:
            break;
    }

    return result;
}

/** REMOVE **/
#define SDL_LOG(msg) printf( msg ": %s\n", SDL_GetError())

static bool
InitializeWindowAndRenderer(SDL_Window **window_p, SDL_Renderer **renderer_p)
{
    bool result = true;

    if (TTF_Init() != 0) {
        printf("Error initializing TTF: %s\n", TTF_GetError());
        result = false;
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_LOG("Error initializing");
        result = false;
    } else {
        *window_p = SDL_CreateWindow(
                "Tetris",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                SDL_WINDOW_ALLOW_HIGHDPI
        );

        if (*window_p == NULL) {
            SDL_LOG("Error creating window");
            result = false;
        } else {
            *renderer_p = SDL_CreateRenderer(
                    *window_p,
                    -1,
                    SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC
            );

            if (*renderer_p == NULL) {
                SDL_LOG("Error creating renderer");
                result = false;
            }
        }
    }

    return result;
}

static void
UnloadGame(game_lib *lib_p)
{
    if (lib_p->gameLib_p) {
#ifdef WIN_BUILD
        FreeLibrary(lib_p->gameLib_p);
#else
        dlclose(lib_p->gameLib_p);
#endif
    }

    lib_p->UpdateAndRender_fp = 0;
    lib_p->isValid = false;
}

static void
LoadGame(game_lib *lib_p)
{
#ifdef WIN_BUILD
    WIN32_FILE_ATTRIBUTE_DATA fileAttr;
    if (GetFileAttributesEx("../bin/game.dll", GetFileExInfoStandard, &fileAttr)) {
        if (CompareFileTime(&lib_p->lastWriteTime, &fileAttr.ftLastWriteTime) != 0) {
            lib_p->lastWriteTime = fileAttr.ftLastWriteTime;
#else
    struct stat fileStat;
    if (stat("../bin/libgame.so", &fileStat) == 0) {
        if (lib_p->ino != fileStat.st_ino) {
            lib_p->ino = fileStat.st_ino;
#endif
            lib_p->isValid = false;
        }
    }

    if (lib_p->isValid == false) {
        UnloadGame(lib_p);

#ifdef WIN_BUILD
        CopyFile("../bin/game.dll", "../bin/game-run.dll", 0);
        lib_p->gameLib_p = LoadLibraryA("../bin/game-run.dll");
        if (lib_p->gameLib_p)
            lib_p->UpdateAndRender_fp = (upd_and_ren *)GetProcAddress(lib_p->gameLib_p, "UpdateAndRender");
        else
            lib_p->UpdateAndRender_fp = 0;
#else
        lib_p->gameLib_p = dlopen("libgame.so", RTLD_LAZY);
        if (!lib_p->gameLib_p) {
            printf("failed to open: %s\n", dlerror());
            lib_p->UpdateAndRender_fp = 0;
        } else {
            dlerror();
            lib_p->UpdateAndRender_fp = (upd_and_ren *)dlsym(lib_p->gameLib_p, "UpdateAndRender");
            const char *err = dlerror();
            if (err) {
                printf("failed to open: %s\n", err);
                lib_p->UpdateAndRender_fp = 0;
            }
        }
#endif

        lib_p->isValid = (lib_p->UpdateAndRender_fp) ? true : false;
    }
}

int
main(int argc, char *argv[])
{
    SDL_Window *window_p;
    SDL_Renderer *renderer_p;

    if (InitializeWindowAndRenderer(&window_p, &renderer_p)) {
        game_lib gameLib = {0};

        game_memory memory = {0};
        memory.permMemSize = Megabytes(64);
        memory.tempMemSize = Megabytes(256);
#ifdef WIN_BUILD
#else
        memory.permMem = mmap(
                0, memory.permMemSize + memory.tempMemSize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#endif
        memory.tempMem = (char *)(memory.permMem) + memory.permMemSize;

        game_input oInput = {0};
        game_input nInput = {0};

        int width, height;
        SDL_GetWindowSize(window_p, &width, &height);
        SDL_RenderSetLogicalSize(renderer_p, width*2, height*2);

        LoadGame(&gameLib);

        u64 prevCount = SDL_GetPerformanceCounter();
        u64 currCount = SDL_GetPerformanceCounter();
        u64 countPerSec = SDL_GetPerformanceFrequency();

        bool done = false;
        while (!done) {
            oInput = nInput;

            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                done = HandleEvent(&event, &oInput, &nInput);
            }

            if (nInput.paused) {
                currCount = SDL_GetPerformanceCounter();
                SDL_Delay(1000.0/GOAL_FPS);
                continue;
            }

#ifdef DEBUG
            if (nInput.reload.endedDown)
                LoadGame(&gameLib);
#endif

            prevCount = currCount;
            currCount = SDL_GetPerformanceCounter();

            SDL_Delay((1000.0/GOAL_FPS -
                        (((currCount - prevCount) > 0)
                         ? 1000*((currCount - prevCount)/countPerSec)
                         : 1000.0f/GOAL_FPS))*0.50f);
            do {
                currCount = SDL_GetPerformanceCounter();
            } while (countPerSec/(currCount - prevCount) > GOAL_FPS);
            nInput.dt = (double)(currCount - prevCount)/countPerSec;

            if (gameLib.UpdateAndRender_fp)
                gameLib.UpdateAndRender_fp(&memory, &nInput, renderer_p);
        }

        UnloadGame(&gameLib);
        TTF_Quit();
        SDL_DestroyRenderer(renderer_p);
        SDL_DestroyWindow(window_p);
        SDL_Quit();

        return 0;
    } else {
        return -1;
    }
}
