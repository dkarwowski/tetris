#include "board.h"

struct row *
GetRow(struct board *board_p, u32 rowID)
{
    ASSERT(0 <= rowID && rowID < BOARD_HEIGHT);
    for_row(iter_p, board_p->first) {
        if (rowID-- == 0)
            return iter_p;
    }

    return NULL;
}

void
ClearRow(struct board *board_p, struct row *row_p, u32 *clearedRows)
{
    if (row_p->prev)
        row_p->prev->next = row_p->next;
    if (row_p->next)
        row_p->next->prev = row_p->prev;

    struct row *prev_p = row_p->prev;

    if (row_p == board_p->first)
        board_p->first = row_p->next;

    board_p->last->next = row_p;
    row_p->prev = board_p->last;
    board_p->last = row_p;
    board_p->last->next = NULL;

    int y = (prev_p) ? prev_p->y : 0;
    prev_p = (prev_p) ? prev_p : board_p->first;
    for_row(iter_p, prev_p) {
        iter_p->y = y++;
    }

    for (int i = 0; i < BOARD_WIDTH; i++)
        row_p->spots[i] = 0;

    (*clearedRows)++;
}

void
_SetType(struct board *board_p, struct piece *piece_p, u32 type)
{
    struct row *row_p[4];

    row_p[2] = GetRow(board_p, (u32)piece_p->pos.y);
    row_p[3] = row_p[2]->next;
    row_p[1] = row_p[2]->prev;
    if (row_p[1] != NULL)
        row_p[0] = row_p[1]->prev;
    for (int i = 0; i < 4; i++) {
        int rowCount = FloorToI32(piece_p->spots[i].y + 2);
        if (row_p[rowCount] != NULL)
            row_p[rowCount]->spots[(i32)(piece_p->pos.x + piece_p->spots[i].x)] = type;
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
