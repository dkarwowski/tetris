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
        row_p->spots[i] = s_COUNT;

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
        int rowCount = FloorToI32(board_p->pieces[piece_p->type][piece_p->rot][i].y + 2);
        if (row_p[rowCount] != NULL)
            row_p[rowCount]->spots[(i32)(piece_p->pos.x + board_p->pieces[piece_p->type][piece_p->rot][i].x)] = type;
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
    _SetType(board_p, piece_p, s_COUNT);
}

bool
IsCollide(struct board *board_p, struct piece *piece_p, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, board_p->pieces[piece_p->type][piece_p->rot][i]);
        if (check.y >= BOARD_HEIGHT)
            continue;
        if (check.x <= -0.00001f || check.y <= -0.00001f || check.x >= BOARD_WIDTH)
            return true;
        if (GetRow(board_p, FloorToI32(check.y))->spots[FloorToI32(check.x)] != s_COUNT)
            return true;
    }

    return false;
}

bool
IsCollideBottom(struct board *board_p, struct piece *piece_p, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, board_p->pieces[piece_p->type][piece_p->rot][i]);
        if ((check.y <= -0.0001f) 
                || (GetRow(board_p, FloorToI32(check.y))->spots[FloorToI32(check.x)] != s_COUNT))
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
        result = V4i(255, 0, 255, 255);
    }

    return result;
}

static void
DK_RenderSpot(SDL_Renderer *renderer_p, SDL_Rect rect, v4 color)
{
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
