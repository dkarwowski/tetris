#include "game.h"
#include "SDL2/SDL2_gfxPrimitives.h"

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

        state_p->font = TTF_OpenFont("DejaVuSans.ttf", 24);
        if (state_p->font == NULL) {
            printf("failed font: %s\n", TTF_GetError());
        }

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

        memory_p->isInitialized = true;
    }

    switch (state_p->gameType) {
        case G_NEW_GAME:
        {
            state_p->dropSpeed = 0.02f;
            state_p->clearedRows = 0;
            state_p->move = 0;
            state_p->moveMod = 50;
            state_p->lastRotPress = 0;
            state_p->checkClear = -1;

            for_row(row_p, board_p->first) {
                for (int j = 0; j < BOARD_WIDTH; j++)
                    row_p->spots[j] = s_COUNT;
            }

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
            state_p->hold = (struct piece) {
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = s_COUNT,
                .rot = 0,
                .floorJump = false
            };

            PlacePiece(board_p, dropping_p);

            state_p->canHold = true;
            state_p->clearedGoal = 10;
            state_p->clearedRows = 0;
            state_p->score = 0;

            state_p->gameType = G_PAUSED;
        } break;
        case G_PLAYING:
        {
            { // Update
                if (input_p->pause.endedDown && state_p->pauseCount != input_p->pause.halfCount) {
                    state_p->gameType = G_PAUSED;
                    state_p->lastPause = input_p->pause.halfCount;
                }

                if (state_p->checkClear > -1) {
                    u8 thisClear = 0;
                    struct row *row_p = GetRow(board_p, state_p->checkClear)->next;
                    while (row_p) {
                        u32 filled = 0;
                        for (int i = 0; i < BOARD_WIDTH; i++)
                            filled += (row_p->spots[i] != s_COUNT) ? 1 : 0;

                        if (filled == BOARD_WIDTH) {
                            ClearRow(board_p, row_p, &state_p->clearedRows);
                            thisClear += 1;
                        }

                        row_p = row_p->prev;
                    }

                    state_p->score += 10 * (thisClear);
                    if (state_p->clearedRows >= state_p->clearedGoal) {
                        state_p->clearedRows = 0;
                        state_p->dropSpeed += 0.005f;
                        state_p->clearedGoal += 2;
                    }
                }

                RemovePiece(board_p, dropping_p);
                if (input_p->hold.endedDown && state_p->canHold && state_p->holdCount != input_p->hold.halfCount) {
                    u8 type = state_p->dropping.type;
                    if (state_p->hold.type != s_COUNT) {
                        state_p->dropping.type = state_p->hold.type;
                        state_p->dropping.pos = state_p->hold.pos;
                    } else {
                        state_p->dropping.pos = state_p->next.pos;
                        state_p->dropping.type = state_p->next.type;

                        state_p->next.type = RandLimit(s_COUNT);
                    }
                    state_p->hold.type = type;
                    state_p->holdCount = input_p->hold.halfCount;
                    state_p->canHold = false;
                }

                // ROTATION
                if (state_p->lastRotPress != input_p->rotCW.halfCount && input_p->rotCW.endedDown) {
                    dropping_p->rot = (dropping_p->rot + 1) % 4;
                    state_p->lastRotPress = input_p->rotCW.halfCount;
                }

                for (int i = 0; i < 3; i++) {
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
                            dropping_p->pos = subV2(dropping_p->pos,
                                    board_p->pieces[dropping_p->type][dropping_p->rot][i]);
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

                if (input_p->hardDrop.endedDown && state_p->lastDrop != input_p->hardDrop.halfCount) {
                    state_p->lastDrop = input_p->hardDrop.halfCount;
                    i32 dropRow = 0;
                    for_row_rev(row_p, board_p->last) {
                        if (!IsCollide(board_p, dropping_p, V2(dropping_p->pos.x, (r32)row_p->y)))
                            continue;
                        dropRow = row_p->y + 1;
                        break;
                    }

                    dropping_p->pos.y = (r32)dropRow + 0.1f;
                    newPos = V2(dropping_p->pos.x, (r32)dropRow - 0.2f);
                }

                if (!IsCollideBottom(board_p, dropping_p, newPos)) {
                    dropping_p->pos = newPos;
                    PlacePiece(board_p, dropping_p);
                    state_p->checkClear = -1;
                } else {
                    PlacePiece(board_p, dropping_p);
                    state_p->checkClear = FloorToI32(dropping_p->pos.y);

                    if (IsCollide(board_p, &(state_p->next), state_p->next.pos))
                        state_p->gameType = G_NEW_GAME;

                    Copy(dropping_p, &(state_p->next), sizeof(struct piece));
                    state_p->next.type = RandLimit(s_COUNT);
                    state_p->canHold = true;
                }
            }

            { // Rendering
                SDL_SetRenderDrawColor(renderer_p, 255, 0, 255, 255);
                SDL_RenderClear(renderer_p);

                int screenHeight;
                SDL_RenderGetLogicalSize(renderer_p, NULL, &screenHeight);

                SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);

                // RENDER DROPPING -----------------------------------------------------------------------------------
                SDL_Rect rect = {
                    .x = 20,
                    .y = screenHeight - 20 - (BLOCK_SIZE * BOARD_VHEIGHT),
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
                        SDL_SetRenderDrawColor(renderer_p, 90, 90, 90, 255);
                        SDL_RenderFillRect(renderer_p, &rect);

                        SDL_SetRenderDrawColor(renderer_p, 70, 70, 70, 255);
                        DK_RenderInlineRect(renderer_p, rect, 2);

                        // NOTE(david): draw the pieces
                        if (row_p->spots[x] != s_COUNT) {
                            DK_RenderSpot(renderer_p, rect, DK_GetTypeColor(row_p->spots[x]));
                        }

                        rect.x += BLOCK_SIZE;
                    }
                    rect.y -= BLOCK_SIZE;
                    rect.x = 20;
                }

                // RENDER PREVIEW ------------------------------------------------------------------------------------
                rect = (SDL_Rect){
                    .x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20,
                    .y = screenHeight - 20 - (BLOCK_SIZE * BOARD_VHEIGHT),
                    .w = BLOCK_SIZE * 5,
                    .h = BLOCK_SIZE * 5
                };

                SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);
                DK_RenderOutlineRect(renderer_p, rect, 5);

                rect.y = screenHeight - 20 - (BLOCK_SIZE * (BOARD_VHEIGHT - 4));
                rect.w = BLOCK_SIZE;
                rect.h = BLOCK_SIZE;
                for (int i = 0; i < 5; i++) {
                    for (int j = 0; j < 5; j++) {
                        SDL_SetRenderDrawColor(renderer_p, 90, 90, 90, 255);
                        SDL_RenderFillRect(renderer_p, &rect);

                        SDL_SetRenderDrawColor(renderer_p, 70, 70, 70, 255);
                        DK_RenderInlineRect(renderer_p, rect, 2);

                        rect.x += BLOCK_SIZE;
                    }

                    rect.x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20;
                    rect.y -= BLOCK_SIZE;
                }

                v2 base = V2(2.0f, 2.0f);
                for (int i = 0; i < 4; i++) {
                    v2 pos = board_p->pieces[state_p->next.type][state_p->next.rot][i];
                    pos = addV2(pos, base);

                    rect.x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20 + BLOCK_SIZE * (i32)pos.x;
                    rect.y = screenHeight - 20 - (BLOCK_SIZE * (BOARD_VHEIGHT - 4)) - BLOCK_SIZE * (i32)pos.y;
                    v4 color = DK_GetTypeColor(state_p->next.type);
                    DK_RenderSpot(renderer_p, rect, color);
                }

                // RENDER HOLD ---------------------------------------------------------------------------------------
                rect = (SDL_Rect){
                    .x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20,
                    .y = screenHeight - 20 - (BLOCK_SIZE * (BOARD_VHEIGHT - 6)),
                    .w = BLOCK_SIZE * 5,
                    .h = BLOCK_SIZE * 5
                };

                SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);
                DK_RenderOutlineRect(renderer_p, rect, 5);

                rect.y = screenHeight - 20 - (BLOCK_SIZE * (BOARD_VHEIGHT - 10));
                rect.w = BLOCK_SIZE;
                rect.h = BLOCK_SIZE;
                for (int i = 0; i < 5; i++) {
                    for (int j = 0; j < 5; j++) {
                        SDL_SetRenderDrawColor(renderer_p, 90, 90, 90, 255);
                        SDL_RenderFillRect(renderer_p, &rect);

                        SDL_SetRenderDrawColor(renderer_p, 70, 70, 70, 255);
                        DK_RenderInlineRect(renderer_p, rect, 2);

                        rect.x += BLOCK_SIZE;
                    }

                    rect.x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20;
                    rect.y -= BLOCK_SIZE;
                }

                if (state_p->hold.type != s_COUNT) {
                    base = V2(2.0f, 2.0f);
                    for (int i = 0; i < 4; i++) {
                        v2 pos = board_p->pieces[state_p->hold.type][state_p->hold.rot][i];
                        pos = addV2(pos, base);

                        rect.x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20 + BLOCK_SIZE * (i32)pos.x;
                        rect.y = screenHeight - 20 - (BLOCK_SIZE * (BOARD_VHEIGHT - 10)) - BLOCK_SIZE * (i32)pos.y;
                        v4 color = DK_GetTypeColor(state_p->hold.type);
                        DK_RenderSpot(renderer_p, rect, color);
                    }
                }

                // RENDER SCORE & LEFT TO CLEAR ----------------------------------------------------------------------
                char score[6];
                sprintf(score, "%5d", (i32)state_p->score);
                SDL_Surface *tSurface_p = TTF_RenderText_Blended(
                        state_p->font, score, (SDL_Color){255, 255, 255, 255});
                SDL_Texture *tTexture_p = SDL_CreateTextureFromSurface(renderer_p, tSurface_p);
                i32 tw, th;
                SDL_QueryTexture(tTexture_p, NULL, NULL, &tw, &th);
                SDL_Rect r = {
                    .x = 20 + BOARD_WIDTH * BLOCK_SIZE + 20,
                    .y = screenHeight - 20 - (BLOCK_SIZE * (BOARD_VHEIGHT - 16)),
                    .w = tw + 10,
                    .h = th + 10
                };
                SDL_RenderCopy(renderer_p, tTexture_p, NULL, &r);

                char levelAndRows[10];
                sprintf(levelAndRows, "%2d - %3d",
                        (state_p->clearedGoal - 10)/2 + 1, (state_p->clearedGoal - state_p->clearedRows));
                tSurface_p = TTF_RenderText_Blended(
                        state_p->font, levelAndRows, (SDL_Color){255, 255, 255, 255});
                tTexture_p = SDL_CreateTextureFromSurface(renderer_p, tSurface_p);
                SDL_QueryTexture(tTexture_p, NULL, NULL, &tw, &th);
                r.y += 2*th;
                r.w = tw + 10;
                r.h = th + 10;
                SDL_RenderCopy(renderer_p, tTexture_p, NULL, &r);
                SDL_FreeSurface(tSurface_p);
                SDL_DestroyTexture(tTexture_p);
            }
        } break;
        case G_PAUSED:
        {
            if (input_p->pause.endedDown && state_p->lastPause != input_p->pause.halfCount) {
                state_p->gameType = G_PAUSED_WAIT;
            }

            SDL_SetRenderDrawColor(renderer_p, 255, 0, 255, 255);
            SDL_RenderClear(renderer_p);
        } break;
        case G_PAUSED_WAIT:
        {
            if (state_p->pauseCount++ > 60) {
                state_p->gameType = G_PLAYING;
                state_p->lastPause = input_p->pause.halfCount;
                state_p->pauseCount = 0;
            }

            SDL_SetRenderDrawColor(renderer_p, 255, 255, 0, 255);
            SDL_RenderClear(renderer_p);
        } break;
        default:
        {
        }
    }

    SDL_RenderPresent(renderer_p);
}
