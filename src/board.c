#include "board.h"

void
BoardInitialize(struct board *board_p)
{
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board_p->pos[i][j] = s_COUNT;
        }
    }
}

static u8 *
GetRow(struct board *board_p, u32 rowID)
{
    ASSERT(0 <= rowID && rowID < BOARD_HEIGHT);
    return board_p->pos[rowID];
}

static void
ClearRow(struct board *board_p, i32 from, i32 to, u32 *clearedRows)
{
    i32 copyFrom = to + 1;
    for (int i = 0; i < BOARD_HEIGHT - from; i++) {
        if (from < 0)
            continue;

        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (copyFrom + i >= BOARD_HEIGHT)
                board_p->pos[from + i][j] = s_COUNT;
            else
                board_p->pos[from + i][j] = board_p->pos[copyFrom + i][j];
        }
    }

    (*clearedRows) += copyFrom - from;
}

static void
_SetType(struct board *board_p, struct piece *piece_p, u32 type)
{
    for (int i = 0; i < 4; i++) {
        v2 pos = addV2(piece_p->pos, boardPieces[piece_p->type][piece_p->rot][i]);
        if (pos.x >= 0 && pos.y >= 0 && pos.x < BOARD_WIDTH && pos.y < BOARD_HEIGHT)
            board_p->pos[FloorToI32(pos.y)][FloorToI32(pos.x)] = type;
    }
}

static void
PlacePiece(struct board *board_p, struct piece *piece_p)
{
    _SetType(board_p, piece_p, piece_p->type);
}

static void
RemovePiece(struct board *board_p, struct piece *piece_p)
{
    _SetType(board_p, piece_p, s_COUNT);
}

static void
PlaceGhost(struct board *board_p, struct piece *piece_p, v2 pos)
{
    v2 tempHold = piece_p->pos;
    piece_p->pos = pos;
    _SetType(board_p, piece_p, s_GHOST);
    piece_p->pos = tempHold;
}

static void
RemoveGhost(struct board *board_p, struct piece *piece_p, v2 pos)
{
    v2 tempHold = piece_p->pos;
    piece_p->pos = pos;
    _SetType(board_p, piece_p, s_COUNT);
    piece_p->pos = tempHold;
}

static void
PlaceView(struct board *board_p, struct piece *piece_p)
{
    v2 tempHold = piece_p->pos;
    piece_p->pos = V2(2.0f, 2.0f);
    _SetType(board_p, piece_p, piece_p->type);
    piece_p->pos = tempHold;
}

static void
RemoveView(struct board *board_p, struct piece *piece_p)
{
    RemoveGhost(board_p, piece_p, V2(2.0f, 2.0f));
}

static bool
IsCollide(struct board *board_p, struct piece *piece_p, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, boardPieces[piece_p->type][piece_p->rot][i]);
        if (check.y >= BOARD_HEIGHT)
            continue;
        if (check.x <= -0.00001f || check.y <= -0.00001f || check.x >= BOARD_WIDTH)
            return true;
        if (board_p->pos[FloorToI32(check.y)][FloorToI32(check.x)] < s_COUNT)
            return true;
    }

    return false;
}

static bool
IsCollideBottom(struct board *board_p, struct piece *piece_p, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, boardPieces[piece_p->type][piece_p->rot][i]);
        if ((check.y <= -0.0001f) 
                || (board_p->pos[FloorToI32(check.y)][FloorToI32(check.x)] < s_COUNT))
            return true;
    }

    return false;
}

static v4
DK_GetTypeColor(u8 type)
{
    v4 result;
    switch (type) {
    case s_I:
        result = V4i(255, 0, 0, 255);
        break;
    case s_T:
        result = V4i(0, 255, 0, 255);
        break;
    case s_S:
        result = V4i(0, 0, 255, 255);
        break;
    case s_Z:
        result = V4i(255, 255, 0, 255);
        break;
    case s_L:
        result = V4i(0, 255, 255, 255);
        break;
    case s_J:
        result = V4i(255, 125, 0, 255);
        break;
    case s_O:
        result = V4i(0, 125, 255, 255);
        break;
    default:
        result = V4i(50, 50, 50, 255);
    }

    return result;
}
