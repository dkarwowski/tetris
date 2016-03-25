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
    struct game_state *state_p = memory_p->permMem;
    struct board *board_p = &(state_p->board);
    struct piece *dropping_p = &(state_p->dropping);

    // Initialization
    if (!memory_p->isInitialized) {
        InitMemStack(
                &state_p->stack,
                (void *)((iptr)memory_p->permMem + sizeof(struct game_state)),
                memory_p->permMemSize - sizeof(struct game_state)
        );

        state_p->dropSpeed = 0.02f;
        state_p->clearedRows = 0;
        state_p->move = 0;
        state_p->moveMod = 50;
        state_p->lastRotPress = 0;
        state_p->checkClear = -1;

        srand(time(NULL));
        board_p->first = &(board_p->rows[0]);
        board_p->last  = &(board_p->rows[BOARD_HEIGHT - 1]);
        for (int i = 0; i < BOARD_HEIGHT; i++) {
            if (i != 0) {
                board_p->rows[i-1].next = &(board_p->rows[i]);
                board_p->rows[i].prev = &(board_p->rows[i-1]);
            }
            board_p->rows[i].y = i;

            for (int j = 0; j < BOARD_WIDTH; j++)
                board_p->rows[i].spots[j] = s_COUNT;
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
                                          {{-1.0f, 1.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
                                          {{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}},
                                          {{-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, -1.0f}}},
                                         {{{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}},
                                          {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}},
                                          {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}},
                                          {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}}}};
        Copy(&(board_p->pieces), &boardPieces, sizeof(boardPieces));

        state_p->dropping = (struct piece){
            .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
            .type = RandLimit(s_COUNT),
            .rot = 0,
            .floorJump = false
        };
        state_p->next = (struct piece) {
            .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
            .type = RandLimit(s_COUNT),
            .rot = 0,
            .floorJump = false
        };

        PlacePiece(board_p, dropping_p);

        memory_p->isInitialized = true;
    }

    { // Update
        if (state_p->checkClear > -1) {
            struct row *row_p = GetRow(board_p, state_p->checkClear)->next;
            while (row_p && row_p->y > (state_p->checkClear - 3)) {
                u32 filled = 0;
                for (int i = 0; i < BOARD_WIDTH; i++)
                    filled += (row_p->spots[i] != s_COUNT) ? 1 : 0;
                
                if (filled == BOARD_WIDTH)
                    ClearRow(board_p, row_p, &state_p->clearedRows);

                row_p = row_p->prev;
            }
        }

        RemovePiece(board_p, dropping_p);
        // ROTATION
        if (state_p->lastRotPress != input_p->rotCW.halfCount && input_p->rotCW.endedDown) {
            dropping_p->rot = (dropping_p->rot + 1) % 4;
            state_p->lastRotPress = input_p->rotCW.halfCount;
        }

        for (int i = 0; i < 2; i++) {
            for (int i = 0; i < 4; i++) {
                v2 check = addV2(dropping_p->pos, board_p->pieces[dropping_p->type][dropping_p->rot][i]);
                if ((check.x < 0.0f || check.y < 0.0f || check.x >= BOARD_WIDTH) ||
                        (GetRow(board_p, FloorToI32(check.y))->spots[FloorToI32(check.x)] != s_COUNT)) {
                    if (board_p->pieces[dropping_p->type][dropping_p->rot][i].y < -0.0001f) {
                        if (!dropping_p->floorJump) {
                            dropping_p->floorJump = true;
                        } else {
                            dropping_p->rot = (dropping_p->rot + 3) % 4;
                            break;
                        }
                    }
                    dropping_p->pos = subV2(dropping_p->pos, board_p->pieces[dropping_p->type][dropping_p->rot][i]);
                }
            }
        }

        v2 newPos = dropping_p->pos;
        // MOVE R/L
        if (!input_p->left.endedDown && !input_p->right.endedDown) {
            state_p->moveMod = 50;
            state_p->move = -1;
        } else if (!(state_p->move++ % state_p->moveMod)) {
            state_p->moveMod = (state_p->moveMod <= 5) ? 3 : state_p->moveMod/2;
            if (input_p->left.endedDown)
                newPos = V2((r32)((int)(dropping_p->pos.x) - 1), dropping_p->pos.y);
            if (input_p->right.endedDown)
                newPos = V2((r32)((int)(dropping_p->pos.x) + 1), dropping_p->pos.y);
        }

        if (IsCollide(board_p, dropping_p, newPos))
            newPos = dropping_p->pos;

        // GRAVITY
        newPos = subV2(newPos, mulV2(state_p->dropSpeed, V2(0.0f, 1.0f)));
        if (input_p->softDrop.endedDown)
            newPos = subV2(newPos, mulV2(state_p->dropSpeed, V2(0.0f, 2.0f)));

        if (!IsCollideBottom(board_p, dropping_p, newPos)) {
            dropping_p->pos = newPos;
            PlacePiece(board_p, dropping_p);
            state_p->checkClear = -1;
        } else {
            PlacePiece(board_p, dropping_p);
            state_p->checkClear = FloorToI32(dropping_p->pos.y);

            Copy(dropping_p, &(state_p->next), sizeof(struct piece));
            state_p->next.type = RandLimit(s_COUNT);
            state_p->next.rot = RandLimit(4);
        }
    }

    { // Rendering
        SDL_SetRenderDrawColor(renderer_p, 255, 0, 255, 255);
        SDL_RenderClear(renderer_p);

        int screenHeight;
        SDL_RenderGetLogicalSize(renderer_p, NULL, &screenHeight);

        SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);

        SDL_Rect rect = {
            .x = 20,
            .y = (screenHeight - BLOCK_SIZE - 20) - (BLOCK_SIZE * (BOARD_VHEIGHT - 1)),
            .w = BOARD_WIDTH * BLOCK_SIZE,
            .h = BOARD_VHEIGHT * BLOCK_SIZE
        };
        DK_RenderOutlineRect(renderer_p, rect, 5);

        rect = (SDL_Rect) {
            .x = 20,
            .y = screenHeight - BLOCK_SIZE - 20,
            .w = BLOCK_SIZE,
            .h = BLOCK_SIZE
        };
        for_row(row_p, board_p->first) {
            if (row_p->y >= BOARD_VHEIGHT)
                break;

            for (int x = 0; x < BOARD_WIDTH; x++) {
                if ((row_p->y + x) % 2)
                    SDL_SetRenderDrawColor(renderer_p, 175, 175, 175, 255);
                else
                    SDL_SetRenderDrawColor(renderer_p, 195, 195, 195, 255);
                SDL_RenderFillRect(renderer_p, &rect);

                // NOTE(david): draw the pieces
                v4 color;
                if (row_p->spots[x] != s_COUNT) {
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
        }

        SDL_RenderPresent(renderer_p);
    }
}
