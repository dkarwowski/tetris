#include "game.h"

#include "board.c"
#include "render.c"

static v2
GetDropPos(struct game_state *state_p)
{
    i32 dropRow = 0;
    for_row_rev(row_p, GetRow(&(state_p->board), (int)state_p->dropping.pos.y)) {
        if (!IsCollide(&(state_p->board), &(state_p->dropping), V2(state_p->dropping.pos.x, (r32)row_p->y)))
            continue;
        dropRow = row_p->y + 1;
        break;
    }

    return V2(0.0f, (r32)dropRow);
}

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

        state_p->font = TTF_OpenFont("DejaVuSans.ttf", 60);
        if (state_p->font == NULL) {
            printf("failed font: %s\n", TTF_GetError());
        }

        srand(time(NULL));
        BoardInitialize(board_p);
        BoardInitialize(&(state_p->nextView));
        BoardInitialize(&(state_p->holdView));

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
            for_row(row_p, state_p->nextView.first) {
                for (int j = 0; j < BOARD_WIDTH; j++)
                    row_p->spots[j] = s_COUNT;
            }
            for_row(row_p, state_p->holdView.first) {
                for (int j = 0; j < BOARD_WIDTH; j++)
                    row_p->spots[j] = s_COUNT;
            }

            state_p->dropping = (struct piece){
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = RandLimit(s_COUNT),
                .rot = 0,
            };
            state_p->next = (struct piece) {
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = RandLimit(s_COUNT),
                .rot = 0,
            };
            state_p->hold = (struct piece) {
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = s_COUNT,
                .rot = 0,
            };

            PlaceView(&(state_p->nextView), &(state_p->next));

            PlacePiece(board_p, dropping_p);

            state_p->canHold = true;
            state_p->clearedGoal = 10;
            state_p->clearedRows = 0;
            state_p->score = 0;
            state_p->droppingLand = 0;

            v2 dropPos = GetDropPos(state_p);
            state_p->ghostPos = V2(dropping_p->pos.x, dropPos.y);
            PlaceGhost(board_p, dropping_p, state_p->ghostPos);

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
                            filled += (row_p->spots[i] < s_COUNT) ? 1 : 0;

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
                RemoveGhost(board_p, dropping_p, state_p->ghostPos);
                if (input_p->hold.endedDown && state_p->canHold && state_p->holdCount != input_p->hold.halfCount) {
                    u8 type = state_p->dropping.type;
                    if (state_p->hold.type != s_COUNT) {
                        state_p->dropping.type = state_p->hold.type;
                        state_p->dropping.pos = state_p->hold.pos;
                        state_p->dropping.rot = state_p->hold.rot;
                    } else {
                        state_p->dropping.pos = state_p->next.pos;
                        state_p->dropping.type = state_p->next.type;
                        state_p->dropping.rot = state_p->next.rot;

                        RemoveView(&(state_p->nextView), &(state_p->next));
                        state_p->next.type = RandLimit(s_COUNT);
                        PlaceView(&(state_p->nextView), &(state_p->next));
                    }
                    RemoveView(&(state_p->holdView), &(state_p->hold));
                    state_p->hold.type = type;
                    state_p->holdCount = input_p->hold.halfCount;
                    PlaceView(&(state_p->holdView), &(state_p->hold));
                    state_p->canHold = false;
                }

                // ROTATION
                if (state_p->lastRotPress != input_p->rotCW.halfCount && input_p->rotCW.endedDown) {
                    dropping_p->rot = (dropping_p->rot + 1) % 4;
                    state_p->lastRotPress = input_p->rotCW.halfCount;
                    state_p->droppingLand = 0;
                }

                for (int i = 0; i < 3; i++) {
                    for (int i = 0; i < 4; i++) {
                        v2 check = addV2(dropping_p->pos, boardPieces[dropping_p->type][dropping_p->rot][i]);
                        if ((check.x < 0.0f || check.y < 0.0f || check.x >= BOARD_WIDTH) ||
                                (GetRow(board_p, FloorToI32(check.y))->spots[FloorToI32(check.x)] != s_COUNT)) {
                            dropping_p->pos = subV2(dropping_p->pos,
                                    boardPieces[dropping_p->type][dropping_p->rot][i]);
                        }
                    }
                }

                v2 newPos = dropping_p->pos;
                // MOVE R/L
                if (!input_p->left.endedDown && !input_p->right.endedDown) {
                    state_p->moveMod = 50;
                    state_p->move = -1;
                } else if (!(state_p->move++ % state_p->moveMod)) {
                    state_p->moveMod = (state_p->moveMod <= 5) ? 3 : state_p->moveMod/3;
                    if (input_p->left.endedDown)
                        newPos = V2((r32)((int)(dropping_p->pos.x) - 1), dropping_p->pos.y);
                    if (input_p->right.endedDown)
                        newPos = V2((r32)((int)(dropping_p->pos.x) + 1), dropping_p->pos.y);
                }

                if (IsCollide(board_p, dropping_p, newPos))
                    newPos = dropping_p->pos;

                // GRAVITY -------------------------------------------------------------------------------------------
                newPos = subV2(newPos, mulV2(state_p->dropSpeed, V2(0.0f, 1.0f)));
                if (input_p->softDrop.endedDown)
                    newPos = subV2(newPos, mulV2(state_p->dropSpeed, V2(0.0f, 2.0f)));

                if (input_p->hardDrop.endedDown && state_p->lastDrop != input_p->hardDrop.halfCount) {
                    state_p->lastDrop = input_p->hardDrop.halfCount;
                    v2 dropPos = GetDropPos(state_p);
                    dropping_p->pos.y = dropPos.y + 0.1f;
                    newPos = V2(dropping_p->pos.x, dropPos.y - 0.2f);
                }

                // TODO(david): clean this up
                if (!IsCollideBottom(board_p, dropping_p, newPos)) {
                    dropping_p->pos = newPos;
                    v2 dropPos = GetDropPos(state_p);
                    state_p->ghostPos = V2(dropping_p->pos.x, dropPos.y);
                    PlaceGhost(board_p, dropping_p, state_p->ghostPos);
                    PlacePiece(board_p, dropping_p);
                    state_p->checkClear = -1;
                    state_p->droppingLand = 0;
                } else if (state_p->droppingLand++ >= 60 || input_p->hardDrop.endedDown) {
                    PlaceGhost(board_p, dropping_p, state_p->ghostPos);
                    PlacePiece(board_p, dropping_p);
                    RemoveView(&(state_p->nextView), &(state_p->next));

                    state_p->checkClear = FloorToI32(dropping_p->pos.y);

                    if (IsCollide(board_p, &(state_p->next), state_p->next.pos))
                        state_p->gameType = G_NEW_GAME;

                    Copy(dropping_p, &(state_p->next), sizeof(struct piece));
                    state_p->next.type = RandLimit(s_COUNT);
                    state_p->canHold = true;
                    state_p->droppingLand = 0;

                    v2 dropPos = GetDropPos(state_p);
                    state_p->ghostPos = V2(dropping_p->pos.x, dropPos.y);
                    PlaceView(&(state_p->nextView), &(state_p->next));
                } else {
                    dropping_p->pos.x = newPos.x;
                    v2 dropPos = GetDropPos(state_p);
                    state_p->ghostPos = V2(dropping_p->pos.x, dropPos.y);
                    PlaceGhost(board_p, dropping_p, state_p->ghostPos);
                    PlacePiece(board_p, dropping_p);
                    state_p->checkClear = -1;
                }
            }

            { // Rendering
                SDL_SetRenderDrawColor(renderer_p, 255, 0, 255, 255);
                SDL_RenderClear(renderer_p);

                int screenHeight;
                SDL_RenderGetLogicalSize(renderer_p, NULL, &screenHeight);

                SDL_SetRenderDrawColor(renderer_p, 0, 0, 0, 255);

                // RENDER DROPPING -----------------------------------------------------------------------------------
                RenderBoardPos(renderer_p, board_p, V2(20, 20));

                // RENDER PREVIEW ------------------------------------------------------------------------------------
                RenderBoardPosDim(
                        renderer_p, 
                        &(state_p->nextView), 
                        V2(40 + (BOARD_WIDTH*BLOCK_SIZE), 20 + 14 * BLOCK_SIZE), 
                        V2(5,5));

                // RENDER HOLD ---------------------------------------------------------------------------------------
                RenderBoardPosDim(
                        renderer_p, 
                        &(state_p->holdView), 
                        V2(40 + (BOARD_WIDTH*BLOCK_SIZE), 20 + 8 * BLOCK_SIZE), 
                        V2(5,5));

                // RENDER SCORE & LEFT TO CLEAR ----------------------------------------------------------------------
                char score[6];
                sprintf(score, "%5d", (i32)state_p->score);
                DK_RenderText(renderer_p, state_p->font, score, V2i(10 + (BOARD_WIDTH*BLOCK_SIZE), 200));

                char levelAndRows[10];
                sprintf(levelAndRows, "%2d - %3d",
                        (state_p->clearedGoal - 10)/2 + 1, (state_p->clearedGoal - state_p->clearedRows));
                DK_RenderText(renderer_p, state_p->font, levelAndRows, V2i(10 * (BOARD_WIDTH*BLOCK_SIZE), 200));
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
