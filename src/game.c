#include "game.h"

#include "board.c"
#include "render.c"

/**
 * Place a ghost on the board at the specified position
 *
 * in:
 *  board : board to place on
 *  piece : what's to be placed on the board
 *  pos     : v2 specifying the ghost position
 */
static void
PlaceGhost(struct board *board, struct piece *piece, v2 pos)
{
    v2 tempHold = piece->pos;
    piece->pos = pos;
    _SetType(board, piece, s_GHOST);
    piece->pos = tempHold;
}

/**
 * Remove a ghost position from the board
 *
 * in:
 *  board : where to place the piece
 *  piece : shape to be taken off of the board
 *  pos     : v2 where the current ghost piece is
 */
static void
RemoveGhost(struct board *board, struct piece *piece, v2 pos)
{
    v2 tempHold = piece->pos;
    piece->pos = pos;
    _SetType(board, piece, s_COUNT);
    piece->pos = tempHold;
}

/**
 * place a piece on a preview board instead of a normal board
 *
 * in:
 *  board : a preview board to place the piece on
 *  piece : piece to place on the board
 */
static void
PlaceView(struct board *board, struct piece *piece)
{
    v2 tempHold = piece->pos;
    piece->pos = V2(2.0f, 2.0f);
    _SetType(board, piece, piece->type);
    piece->pos = tempHold;
}

/**
 * Remove a piece from a preview board instead of a normal board
 *
 * in:
 *  board : preview board to remove the piece from
 *  piece : what's to be previewed on the board
 */
static void
RemoveView(struct board *board, struct piece *piece)
{
    RemoveGhost(board, piece, V2(2.0f, 2.0f));
}

/**
 * Check the board for its drop position. Start from the current position and
 * move down until you hit a collision
 *
 * in:
 *  state : the current game state
 * out:
 *  v2 for the block where it has no collisions
 */
static v2
GetDropPos(struct game_state *state)
{
    i32 dropRow = 0;
    for (int y = FloorToI32(state->dropping.pos.y); y >= 0; y--) {
        if (!IsCollide(&(state->board), &(state->dropping), V2(state->dropping.pos.x, (r32)y)))
            continue;
        dropRow = y + 1;
        break;
    }

    return V2(0.0f, (r32)dropRow);
}

extern void
UpdateAndRender(game_memory *memory, game_input *input, SDL_Renderer *renderer)
{
    struct game_state *state = memory->permMem;
    struct board *board = &(state->board);
    struct piece *dropping = &(state->dropping);

    // Initialization
    if (!memory->isInitialized) {
        InitMemStack(
                &state->stack,
                (void *)((iptr)memory->permMem + sizeof(struct game_state)),
                memory->permMemSize - sizeof(struct game_state)
        );

        char fontName[128];
        strcpy(fontName, _WD);
        strcat(fontName, "/data/DejaVuSans.ttf");
        state->font = TTF_OpenFont(fontName, 55);
        if (state->font == NULL) {
            printf("failed font: %s\n", TTF_GetError());
        }

        srand(time(NULL));
        BoardInitialize(board);
        BoardInitialize(&(state->nextView));
        BoardInitialize(&(state->holdView));

        memory->isInitialized = true;
    }

    switch (state->gameType) {
        case G_NEW_GAME:
        {
            state->dropSpeed = 0.05f;
            state->clearedRows = 0;
            state->move = 0;
            state->moveMod = 50;
            state->lastRotPress = 0;
            state->checkClear = -1;

            BoardInitialize(board);
            BoardInitialize(&(state->nextView));
            BoardInitialize(&(state->holdView));

            state->dropping = (struct piece){
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = RandLimit(s_COUNT),
                .rot = 0,
            };
            state->next = (struct piece) {
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = RandLimit(s_COUNT),
                .rot = 0,
            };
            state->hold = (struct piece) {
                .pos = {(r32)(BOARD_WIDTH)/2.0f, BOARD_HEIGHT - 2},
                .type = s_COUNT,
                .rot = 0,
            };

            PlaceView(&(state->nextView), &(state->next));

            PlacePiece(board, dropping);

            state->canHold = true;
            state->clearedGoal = 10;
            state->clearedRows = 0;
            state->score = 0;
            state->droppingLand = 0;

            v2 dropPos = GetDropPos(state);
            state->ghostPos = V2(dropping->pos.x, dropPos.y);
            PlaceGhost(board, dropping, state->ghostPos);

            DK_RenderGame(renderer, state);
            DK_RenderTextCenter(
                    renderer, state->font, "New Game", V2(20+(BOARD_WIDTH*BLOCK_SIZE/2), SCREEN_HEIGHT));
            SDL_RenderPresent(renderer);

            state->gameType = G_PAUSED_WAIT;
        } break;
        case G_PLAYING:
        {
            { // Update
                if (input->pause.endedDown && state->pauseCount != input->pause.halfCount) {
                    state->gameType = G_PAUSED;
                    state->lastPause = input->pause.halfCount;
                }

                // CLEAR ROWS -----------------------------------------------------------------------------------------
                if (state->checkClear > -1) {
                    int thisClear = 0;
                    int toClear[BOARD_HEIGHT] = {0};

                    for (int i = 0; i < BOARD_HEIGHT; i++) {
                        for (int j = 0; j < BOARD_WIDTH; j++) {
                            toClear[i] += (board->pos[i][j] != s_COUNT) ? 1 : 0;
                        }
                    }
                    int clearTo = -1;
                    for (int i = BOARD_HEIGHT - 1; i >= 0; i--) {
                        if (clearTo >= 0 && toClear[i] != BOARD_WIDTH) {
                            ClearRow(board, i + 1, clearTo, &state->clearedRows);
                            thisClear += clearTo - i;
                            clearTo = -1;
                        } else if (clearTo < 0 && toClear[i] == BOARD_WIDTH) {
                            clearTo = i;
                        }
                    }
                    if (clearTo >= 0)
                        ClearRow(board, 0, clearTo, &state->clearedRows);

                    state->score += 10 * (thisClear);
                    if (state->clearedRows >= state->clearedGoal) {
                        state->clearedRows -= state->clearedGoal;
                        state->dropSpeed += 0.005f;
                        state->clearedGoal += 2;
                    }
                }

                RemovePiece(board, dropping);
                RemoveGhost(board, dropping, state->ghostPos);
                // HOLD PIECE -----------------------------------------------------------------------------------------
                if (input->hold.endedDown && state->canHold && state->holdCount != input->hold.halfCount) {
                    u8 type = state->dropping.type;
                    if (state->hold.type != s_COUNT) {
                        state->dropping.type = state->hold.type;
                        state->dropping.pos = state->hold.pos;
                        state->dropping.rot = state->hold.rot;
                        RemoveView(&(state->holdView), &(state->hold));
                    } else {
                        state->dropping.pos = state->next.pos;
                        state->dropping.type = state->next.type;
                        state->dropping.rot = state->next.rot;

                        RemoveView(&(state->nextView), &(state->next));
                        state->next.type = RandLimit(s_COUNT);
                        PlaceView(&(state->nextView), &(state->next));
                    }
                    state->hold.type = type;
                    state->holdCount = input->hold.halfCount;
                    PlaceView(&(state->holdView), &(state->hold));
                    state->canHold = false;
                }

                // ROTATION -------------------------------------------------------------------------------------------
                if (state->lastRotPress != input->rotCW.halfCount && input->rotCW.endedDown) {
                    dropping->rot = (dropping->rot + 1) % 4;
                    state->lastRotPress = input->rotCW.halfCount;
                    state->droppingLand = 0;
                }

                for (int i = 0; i < 3; i++) {
                    for (int i = 0; i < 4; i++) {
                        v2 check = addV2(dropping->pos, boardPieces[dropping->type][dropping->rot][i]);
                        if ((check.x < 0.0f || check.y < 0.0f || check.x >= BOARD_WIDTH) ||
                                (board->pos[FloorToI32(check.y)][FloorToI32(check.x)] != s_COUNT)) {
                            dropping->pos = subV2(dropping->pos,
                                    boardPieces[dropping->type][dropping->rot][i]);
                        }
                    }
                }

                v2 newPos = dropping->pos;
                // MOVE R/L -------------------------------------------------------------------------------------------
                if (!input->left.endedDown && !input->right.endedDown) {
                    state->moveMod = 50;
                    state->move = -1;
                } else if (!(state->move++ % state->moveMod)) {
                    state->moveMod = (state->moveMod <= 5) ? 3 : state->moveMod/3;
                    if (input->left.endedDown)
                        newPos = V2((r32)((int)(dropping->pos.x) - 1), dropping->pos.y);
                    if (input->right.endedDown)
                        newPos = V2((r32)((int)(dropping->pos.x) + 1), dropping->pos.y);
                }

                if (IsCollide(board, dropping, newPos))
                    newPos = dropping->pos;

                // GRAVITY -------------------------------------------------------------------------------------------
                newPos = subV2(newPos, mulV2(state->dropSpeed, V2(0.0f, 1.0f)));
                if (input->softDrop.endedDown)
                    newPos = subV2(newPos, mulV2(state->dropSpeed, V2(0.0f, 2.0f)));

                if (input->hardDrop.endedDown && state->lastDrop != input->hardDrop.halfCount) {
                    state->lastDrop = input->hardDrop.halfCount;
                    v2 dropPos = GetDropPos(state);
                    dropping->pos.y = dropPos.y + 0.1f;
                    newPos = V2(dropping->pos.x, dropPos.y - 0.2f);
                }

                // TODO(david): clean this up
                if (!IsCollideBottom(board, dropping, newPos)) {
                    dropping->pos = newPos;
                    v2 dropPos = GetDropPos(state);
                    state->ghostPos = V2(dropping->pos.x, dropPos.y);
                    PlaceGhost(board, dropping, state->ghostPos);
                    PlacePiece(board, dropping);
                    state->checkClear = -1;
                    state->droppingLand = 0;
                } else if (state->droppingLand++ >= 60 || input->hardDrop.endedDown) {
                    PlaceGhost(board, dropping, state->ghostPos);
                    PlacePiece(board, dropping);
                    RemoveView(&(state->nextView), &(state->next));

                    state->checkClear = FloorToI32(dropping->pos.y);

                    if (IsCollide(board, &(state->next), state->next.pos))
                        state->gameType = G_NEW_GAME;

                    Copy(dropping, &(state->next), sizeof(struct piece));
                    state->next.type = RandLimit(s_COUNT);
                    state->canHold = true;
                    state->droppingLand = 0;

                    v2 dropPos = GetDropPos(state);
                    state->ghostPos = V2(dropping->pos.x, dropPos.y);
                    PlaceView(&(state->nextView), &(state->next));
                } else {
                    dropping->pos.x = newPos.x;
                    v2 dropPos = GetDropPos(state);
                    state->ghostPos = V2(dropping->pos.x, dropPos.y);
                    PlaceGhost(board, dropping, state->ghostPos);
                    PlacePiece(board, dropping);
                    state->checkClear = -1;
                }
            }

            { // Rendering
                DK_RenderGame(renderer, state);

                SDL_RenderPresent(renderer);
            }
        } break;
        case G_PAUSED:
        {
            DK_RenderGame(renderer, state);

            DK_RenderTextCenter(
                    renderer, state->font, "Paused", V2(20+(BOARD_WIDTH*BLOCK_SIZE/2), SCREEN_HEIGHT));
            state->gameType = G_PAUSED_WAIT;

            SDL_RenderPresent(renderer);
        } break;
        case G_PAUSED_WAIT:
        {
            if (input->pause.endedDown && state->lastPause != input->pause.halfCount) {
                state->gameType = G_PAUSED_PLAY;
            }
        } break;
        case G_PAUSED_PLAY:
        {
            if (state->pauseCount++ > 120) {
                state->gameType = G_PLAYING;
                state->lastPause = input->pause.halfCount;
                state->pauseCount = 0;
            }
            char count[6];
            sprintf(count, "%2d...", 10 - (i32)(state->pauseCount/12));

            DK_RenderGame(renderer, state);
            DK_RenderTextCenter(renderer, state->font, count, V2(20+(BOARD_WIDTH*BLOCK_SIZE/2), SCREEN_HEIGHT));
            SDL_RenderPresent(renderer);
        } break;
        default:
        {
        }
    }
}
