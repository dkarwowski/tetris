#include "main.h"

#define BOARD_HEIGHT 19
#define BOARD_WIDTH 10
#define BLOCK_SIZE 55

static void
DK_RenderOutlineRect(SDL_Renderer *renderer_p, SDL_Rect rect, int width)
{
    for (int i = 0; i < width; i++) {
        rect.x -= 1;
        rect.y -= 1;
        rect.w += 2;
        rect.h += 2;
        SDL_RenderDrawRect(renderer_p, &rect);
    }
}

enum shapes {
    s_none = 0,

    s_line = 1,
    s_T    = 2,
    s_s    = 3,
    s_z    = 4,
    s_l    = 5,
    s_bl   = 6
};

extern void
UpdateAndRender(game_memory *memory_p, game_input *input_p, SDL_Renderer *renderer_p)
{
    // the board
    static u32 board[BOARD_HEIGHT][BOARD_WIDTH] = {0};

    board[0][1] = s_line;
    board[0][2] = s_T;
    board[0][3] = s_l;
    board[0][4] = s_s;
    board[1][1] = s_z;
    board[1][3] = s_bl;

    { // Rendering
        SDL_SetRenderDrawColor(renderer_p, 255, 0, 255, 255);
        SDL_RenderClear(renderer_p);

        int screenHeight;
        SDL_RenderGetLogicalSize(renderer_p, NULL, &screenHeight);

        SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);

        SDL_Rect rect = {
            .x = 20,
            .y = (screenHeight - BLOCK_SIZE - 20) - (BLOCK_SIZE * (BOARD_HEIGHT - 1)),
            .w = BOARD_WIDTH * BLOCK_SIZE,
            .h = BOARD_HEIGHT * BLOCK_SIZE
        };
        DK_RenderOutlineRect(renderer_p, rect, 5);

        rect = (SDL_Rect) {
            .x = 20, 
            .y = screenHeight - BLOCK_SIZE - 20, 
            .w = BLOCK_SIZE, 
            .h = BLOCK_SIZE
        };
        for (int y = 0; y < BOARD_HEIGHT; y++) {
            for (int x = 0; x < BOARD_WIDTH; x++) {

                if ((y + x) % 2)
                    SDL_SetRenderDrawColor(renderer_p, 175, 175, 175, 255);
                else
                    SDL_SetRenderDrawColor(renderer_p, 195, 195, 195, 255);
                SDL_RenderFillRect(renderer_p, &rect);

                if (board[y][x] != s_none) {
                    switch (board[y][x]) {
                    case s_line:
                        SDL_SetRenderDrawColor(renderer_p, 255, 0, 0, 255);
                        break;
                    case s_T:
                        SDL_SetRenderDrawColor(renderer_p, 0, 255, 0, 255);
                        break;
                    case s_s:
                        SDL_SetRenderDrawColor(renderer_p, 0, 0, 255, 255);
                        break;
                    case s_z:
                        SDL_SetRenderDrawColor(renderer_p, 255, 255, 0, 255);
                        break;
                    case s_l:
                        SDL_SetRenderDrawColor(renderer_p, 0, 255, 255, 255);
                        break;
                    case s_bl:
                        SDL_SetRenderDrawColor(renderer_p, 255, 125, 0, 255);
                        break;
                    default:
                        SDL_SetRenderDrawColor(renderer_p, 255, 0, 255, 255);
                    }
                    SDL_Rect r2 = rect;
                    r2.x += 5;
                    r2.y += 5;
                    r2.w -= 10;
                    r2.h -= 10;
                    SDL_RenderFillRect(renderer_p, &r2);

                    SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);
                    DK_RenderOutlineRect(renderer_p, r2, 4);
                }

                rect.x += BLOCK_SIZE;
            }
            rect.y -= BLOCK_SIZE;
            rect.x = 20;
        }

        SDL_RenderPresent(renderer_p);
    }
}
