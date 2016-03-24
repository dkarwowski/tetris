#include "game.h"

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

extern void
UpdateAndRender(game_memory *memory_p, game_input *input_p, SDL_Renderer *renderer_p)
{
    // the board
    static struct board board = {0};
    static bool init = false;

    static struct piece dropping = {
        .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 1},
        .type = s_I,
        .rot = 0,
        .floorJump = false
    };

    static r32 dropSpeed = 0.02f;
    static u32 clearedRows = 0;
    static i32 move = 0;
    static i32 moveMod = 50;

    static i32 lastRot = 0;

    // Initialization
    if (!init) {
        board.first = &(board.rows[0]);
        board.last  = &(board.rows[BOARD_HEIGHT - 1]);
        for (int i = 0; i < BOARD_HEIGHT; i++) {
            if (i != 0) {
                board.rows[i-1].next = &(board.rows[i]);
                board.rows[i].prev = &(board.rows[i-1]);
            }
            board.rows[i].y = i;

            for (int j = 0; j < BOARD_WIDTH; j++)
                board.rows[i].spots[j] = s_COUNT;
        }


        v2 boardPieces[s_COUNT][4][4] = {{{{-2.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{0.0f, -2.0f}, {0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}},
                                          {{-2.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{0.0f, -2.0f}, {0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}}},
                                         {{{-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}},
                                          {{0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 0.0f}},
                                          {{-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, -1.0f}},
                                          {{0.0f, -1.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f}}},
                                         {{{-1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f}, {1.0f, 1.0f}},
                                          {{0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, -1.0f}},
                                          {{-1.0f, -1.0f}, {0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{-1.0f, 1.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}}},
                                         {{{-1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}, {1.0f, -1.0f}},
                                          {{0.0f, 1.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f}, {-1.0f, -1.0f}},
                                          {{-1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{1.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}}},
                                         {{{0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}, {1.0f, -1.0f}},
                                          {{-1.0f, -1.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{-1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}},
                                          {{-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}}},
                                         {{{0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}, {-1.0f, -1.0f}},
                                          {{-1.0f, -1.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}},
                                          {{-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, -1.0f}}},
                                         {{{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}},
                                          {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}},
                                          {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}},
                                          {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}}};
        Copy(&(board.pieces), &boardPieces, sizeof(boardPieces));

        PlacePiece(&board, &dropping);

        init = true;
    }

    { // Update
        RemovePiece(&board, &dropping);
        // ROTATION
        if (lastRot != input_p->rotCW.halfCount && input_p->rotCW.endedDown) {
            dropping.rot = (dropping.rot + 1) % 4;
            lastRot = input_p->rotCW.halfCount;
        }

        { // fix collision with wall jump
            for (int i = 0; i < 2; i++) {
                for (int i = 0; i < 4; i++) {
                    v2 check = addV2(dropping.pos, board.pieces[dropping.type][dropping.rot][i]);
                    if ((check.x <= -0.00001f || check.y <= -0.00001f || check.x >= BOARD_WIDTH) ||
                            (GetRow(&board, FloorToI32(check.y))->spots[FloorToI32(check.x)] != s_COUNT)) {
                        if (board.pieces[dropping.type][dropping.rot][i].y < -0.0001f) {
                            if (!dropping.floorJump)
                                dropping.floorJump = true;
                            else {
                                dropping.rot = (dropping.rot + 3) % 4;
                                goto doneRot;
                            }
                        }
                        dropping.pos = subV2(dropping.pos, board.pieces[dropping.type][dropping.rot][i]);
                    }
                }
            }
        }

doneRot:;
        v2 newPos = dropping.pos;
        // MOVE R/L
        if (!input_p->left.endedDown && !input_p->right.endedDown) {
            moveMod = 50;
            move = -1;
        } else if (!(move++ % moveMod)) {
            moveMod = (moveMod <= 5) ? 3 : moveMod/2;
            if (input_p->left.endedDown)
                newPos = V2((r32)((int)(dropping.pos.x) - 1), dropping.pos.y);
            if (input_p->right.endedDown)
                newPos = V2((r32)((int)(dropping.pos.x) + 1), dropping.pos.y);
        }

        if (IsCollide(&board, &dropping, newPos))
            newPos = dropping.pos;

        // GRAVITY
        newPos = subV2(newPos, mulV2(dropSpeed, V2(0.0f, 1.0f)));
        if (input_p->softDrop.endedDown)
            newPos = subV2(newPos, mulV2(dropSpeed, V2(0.0f, 2.0f)));
        if ((!IsCollide(&board, &dropping, newPos) 
                && !fequal(newPos.y, dropping.pos.y, 0.001f)))
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
                if (row_p->spots[x] != s_COUNT) {
                    filledCount += 1;
                    switch (row_p->spots[x]) {
                    case s_I:
                        color = V4i(255, 0, 0, 255);
                        break;
                    case s_T:
                        color = V4i(0, 255, 0, 255);
                        break;
                    case s_S:
                        color = V4i(0, 0, 255, 255);
                        break;
                    case s_Z:
                        color = V4i(255, 255, 0, 255);
                        break;
                    case s_L:
                        color = V4i(0, 255, 255, 255);
                        break;
                    case s_J:
                        color = V4i(255, 125, 0, 255);
                        break;
                    case s_O:
                        color = V4i(0, 125, 255, 255);
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
