#include "main.h"

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

static void
DK_RenderInlineRect(SDL_Renderer *renderer_p, SDL_Rect rect, int width)
{
    for (int i = 0; i < width; i++) {
        SDL_RenderDrawRect(renderer_p, &rect);
        rect.x += 1;
        rect.y += 1;
        rect.w -= 2;
        rect.h -= 2;
    }
}

#include "board.c"

struct piece {
    v2 pos;
    v2 spots[4];
    u32 type;
};

void
_SetType(struct board *board_p, struct piece *piece_p, u32 type)
{
    struct row *row_p = GetRow(board_p, (u32)piece_p->pos.y);
    for (int i = 0; i < 4; i++) {
        if (fequal(piece_p->spots[i].y, 0.0f, 0.0001f)) {
            row_p->spots[(i32)(piece_p->pos.x + piece_p->spots[i].x)] = type;
        } else {
            struct row *fix_p = row_p;
            while (!fequal(fix_p->y, (piece_p->pos.y + piece_p->spots[i].y), 0.001f))
                fix_p = (piece_p->spots[i].y < 0) ? row_p->prev : row_p->next;

            fix_p->spots[(i32)(piece_p->pos.x + piece_p->spots[i].x)] = type;
        }
    }
}

void
PlacePiece(struct board *board_p, struct piece *piece_p)
{
    _SetType(board_p, piece_p, piece_p->type);
}

void
RemovePiece(struct board *board_p, struct piece *piece_p)
{
    _SetType(board_p, piece_p, s_none);
}

extern void
UpdateAndRender(game_memory *memory_p, game_input *input_p, SDL_Renderer *renderer_p)
{
    // the board
    static struct board board = {0};
    static bool init = false;

    static struct piece dropping = {
        .pos = {3.0f, 3.0f},
        .spots = {{-2.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
        .type = s_line
    };

    // Initialization
    if (!init) {
        board.first = &(board.rows[0]);
        board.last  = &(board.rows[BOARD_HEIGHT - 1]);
        for (int i = 1; i < BOARD_HEIGHT; i++) {
            board.rows[i-1].next = &(board.rows[i]);
            board.rows[i].prev = &(board.rows[i-1]);
            board.rows[i].y = i;
        }

        PlacePiece(&board, &dropping);

        init = true;
    }

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
        for_row(row_p, board.first) {
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if ((row_p->y + x) % 2)
                    SDL_SetRenderDrawColor(renderer_p, 175, 175, 175, 255);
                else
                    SDL_SetRenderDrawColor(renderer_p, 195, 195, 195, 255);
                SDL_RenderFillRect(renderer_p, &rect);

                // NOTE(david): draw the pieces
                v4 color;
                if (row_p->spots[x] != s_none) {
                    switch (row_p->spots[x]) {
                    case s_line:
                        color = V4i(255, 0, 0, 255);
                        break;
                    case s_T:
                        color = V4i(0, 255, 0, 255);
                        break;
                    case s_s:
                        color = V4i(0, 0, 255, 255);
                        break;
                    case s_z:
                        color = V4i(255, 255, 0, 255);
                        break;
                    case s_l:
                        color = V4i(0, 255, 255, 255);
                        break;
                    case s_bl:
                        color = V4i(255, 125, 0, 255);
                        break;
                    default:
                        color = V4i(255, 0, 255, 255);
                    }
                    SDL_SetRenderDrawColor(renderer_p, color.r, color.g, color.b, color.a);

                    SDL_Rect r2 = rect;
                    r2.x += 4;
                    r2.y += 4;
                    r2.w -= 8;
                    r2.h -= 8;
                    SDL_RenderFillRect(renderer_p, &r2);

                    v3 hsv = RGBtoHSV(color);
                    hsv.v *= 0.3f;
                    color = HSVtoRGB(hsv);

                    SDL_SetRenderDrawColor(renderer_p, color.r, color.g, color.b, color.a);
                    DK_RenderInlineRect(renderer_p, r2, 4);
                }

                rect.x += BLOCK_SIZE;
            }
            rect.y -= BLOCK_SIZE;
            rect.x = 20;
        }

        SDL_RenderPresent(renderer_p);
    }
}
