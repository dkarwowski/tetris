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

bool
IsCollide(struct board *board_p, struct piece *piece_p, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, piece_p->spots[i]);
        if (check.x <= 0.0f || check.y <= 0.0f || check.x >= BOARD_WIDTH)
            return true;
        if (GetRow(board_p, FloorToI32(check.y))->spots[FloorToI32(check.x)] != s_none)
            return true;
    }

    return false;
}

extern void
UpdateAndRender(game_memory *memory_p, game_input *input_p, SDL_Renderer *renderer_p)
{
    // the board
    static struct board board = {0};
    static bool init = false;

    static struct piece dropping = {
        .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 1},
        .spots = {{-2.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
        .type = s_line
    };

    static r32 dropSpeed = 0.02f;
    static u32 clearedRows = 0;

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

    { // Update
        RemovePiece(&board, &dropping);
        v2 newPos = dropping.pos;
        if (input_p->left.endedDown)
            newPos = subV2(newPos, mulV2(0.15f, V2(1.0f, 0.0f)));
        if (input_p->right.endedDown)
            newPos = addV2(newPos, mulV2(0.15f, V2(1.0f, 0.0f)));

        if (IsCollide(&board, &dropping, newPos))
            newPos = dropping.pos;

        newPos = subV2(newPos, mulV2(dropSpeed, V2(0.0f, 1.0f)));
        if ((!IsCollide(&board, &dropping, newPos) 
                && !fequal(newPos.y, dropping.pos.y, 0.001f)) || (newPos.y >= 0.0f))
            dropping.pos = newPos;

        PlacePiece(&board, &dropping);
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
        bool toClear[BOARD_HEIGHT] = {0};
        for_row(row_p, board.first) {
            u32 filledCount = 0;
            for (int x = 0; x < BOARD_WIDTH; x++) {
                if ((row_p->y + x) % 2)
                    SDL_SetRenderDrawColor(renderer_p, 175, 175, 175, 255);
                else
                    SDL_SetRenderDrawColor(renderer_p, 195, 195, 195, 255);
                SDL_RenderFillRect(renderer_p, &rect);

                // NOTE(david): draw the pieces
                v4 color;
                if (row_p->spots[x] != s_none) {
                    filledCount += 1;
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

            if (filledCount == BOARD_HEIGHT)
                toClear[((iptr)(row_p)-(iptr)&(board.rows))/sizeof(struct row)] = true;
        }

        for (int i = 0; i < BOARD_HEIGHT; i++) {
            if (toClear[i])
                ClearRow(&board, &(board.rows[i]), &clearedRows);
        }

        SDL_RenderPresent(renderer_p);
    }
}
