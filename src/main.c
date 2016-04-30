#include "main.h"

static void
ProcessKeyboardInput(game_control *oControl, game_control *nControl, bool isDown)
{
    nControl->endedDown = isDown;
    nControl->halfCount += (oControl->endedDown != nControl->endedDown) ? 1 : 0;
}

static bool
HandleEvent(SDL_Event *event, game_input *oInput, game_input *nInput)
{
    bool result = false;

    switch (event->type) {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        {
            bool wasDown = event->key.repeat != 0 || event->key.state == SDL_RELEASED;
            bool isDown  = event->key.repeat != 0 || event->key.state == SDL_PRESSED;

            if (wasDown != isDown) {
                if (event->key.keysym.sym == SDLK_LEFT)
                    ProcessKeyboardInput(&(oInput->left), &(nInput->left), isDown);
                if (event->key.keysym.sym == SDLK_RIGHT)
                    ProcessKeyboardInput(&(oInput->right), &(nInput->right), isDown);
                if (event->key.keysym.sym == SDLK_UP)
                    ProcessKeyboardInput(&(oInput->rotCW), &(nInput->rotCW), isDown);
                if (event->key.keysym.sym == SDLK_DOWN)
                    ProcessKeyboardInput(&(oInput->softDrop), &(nInput->softDrop), isDown);
                if (event->key.keysym.sym == SDLK_SPACE)
                    ProcessKeyboardInput(&(oInput->hardDrop), &(nInput->hardDrop), isDown);
                if (event->key.keysym.sym == SDLK_p)
                    ProcessKeyboardInput(&(oInput->pause), &(nInput->pause), isDown);
                if (event->key.keysym.sym == SDLK_c)
                    ProcessKeyboardInput(&(oInput->hold), &(nInput->hold), isDown);

#ifdef DEBUG
                if (event->key.keysym.sym == SDLK_r)
                    ProcessKeyboardInput(&(oInput->reload), &(nInput->reload), isDown);
#endif

                if (event->key.keysym.sym == SDLK_ESCAPE)
                    ProcessKeyboardInput(&(oInput->terminate), &(nInput->terminate), isDown);
            }

            if (event->key.keysym.sym == SDLK_ESCAPE)
                result = true;
        } break;
        case SDL_WINDOWEVENT:
        {
            if (event->window.event == SDL_WINDOWEVENT_FOCUS_LOST ||
                    event->window.event == SDL_WINDOWEVENT_HIDDEN) {
                nInput->paused = true;
                oInput->paused = true;
            }
            if (event->window.event == SDL_WINDOWEVENT_FOCUS_GAINED ||
                    event->window.event == SDL_WINDOWEVENT_SHOWN) {
                nInput->paused = false;
                oInput->paused = false;
            }
#ifdef WIN_BUILD
            if (event->window.event == SDL_WINDOWEVENT_MOVED) {
                nInput->paused = true;
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
InitializeWindowAndRenderer(SDL_Window **window, SDL_Renderer **renderer)
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
        *window = SDL_CreateWindow(
                "Tetris",
                SDL_WINDOWPOS_CENTERED,
                SDL_WINDOWPOS_CENTERED,
                SCREEN_WIDTH,
                SCREEN_HEIGHT,
                SDL_WINDOW_ALLOW_HIGHDPI
        );

        if (*window == NULL) {
            SDL_LOG("Error creating window");
            result = false;
        } else {
            *renderer = SDL_CreateRenderer(
                    *window,
                    -1,
                    SDL_RENDERER_ACCELERATED|SDL_RENDERER_PRESENTVSYNC
            );

            if (*renderer == NULL) {
                SDL_LOG("Error creating renderer");
                result = false;
            }
        }
    }

    return result;
}

static void
UnloadGame(game_lib *lib)
{
    if (lib->gameLib) {
#ifdef WIN_BUILD
        FreeLibrary(lib->gameLib);
#else
        dlclose(lib->gameLib);
#endif
    }

    lib->UpdateAndRender_fp = 0;
    lib->isValid = false;
}

static void
LoadGame(game_lib *lib)
{
#ifdef WIN_BUILD
    WIN32_FILE_ATTRIBUTE_DATA fileAttr;
    if (GetFileAttributesEx("../bin/game.dll", GetFileExInfoStandard, &fileAttr)) {
        if (CompareFileTime(&lib->lastWriteTime, &fileAttr.ftLastWriteTime) != 0) {
            lib->lastWriteTime = fileAttr.ftLastWriteTime;
#else
    struct stat fileStat;
    if (stat("../bin/libgame.so", &fileStat) == 0) {
        if (lib->ino != fileStat.st_ino) {
            lib->ino = fileStat.st_ino;
#endif
            lib->isValid = false;
        }
    }

    if (lib->isValid == false) {
        UnloadGame(lib);

#ifdef WIN_BUILD
        CopyFile("../bin/game.dll", "../bin/game-run.dll", 0);
        lib->gameLib = LoadLibraryA("../bin/game-run.dll");
        if (lib->gameLib)
            lib->UpdateAndRender_fp = (upd_and_ren *)GetProcAddress(lib->gameLib, "UpdateAndRender");
        else
            lib->UpdateAndRender_fp = 0;
#else
        lib->gameLib = dlopen("libgame.so", RTLD_LAZY);
        if (!lib->gameLib) {
            printf("failed to open: %s\n", dlerror());
            lib->UpdateAndRender_fp = 0;
        } else {
            dlerror();
            lib->UpdateAndRender_fp = (upd_and_ren *)dlsym(lib->gameLib, "UpdateAndRender");
            const char *err = dlerror();
            if (err) {
                printf("failed to open: %s\n", err);
                lib->UpdateAndRender_fp = 0;
            }
        }
#endif

        lib->isValid = (lib->UpdateAndRender_fp) ? true : false;
    }
}

int
main(int argc, char *argv[])
{
    SDL_Window *window;
    SDL_Renderer *renderer;

    if (InitializeWindowAndRenderer(&window, &renderer)) {
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
        SDL_GetWindowSize(window, &width, &height);
        SDL_RenderSetLogicalSize(renderer, width*2, height*2);

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
                gameLib.UpdateAndRender_fp(&memory, &nInput, renderer);
        }

        UnloadGame(&gameLib);
        TTF_Quit();
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();

        return 0;
    } else {
        return -1;
    }
}
