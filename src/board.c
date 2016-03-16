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
ClearRow(struct board *board_p, struct row *row_p)
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
}
