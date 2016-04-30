#include "board.h"

/**
 * Initialize the board to all blank positions
 *
 * in:
 *  board : what should be initialized
 */
void
BoardInitialize(struct board *board)
{
    for (int i = 0; i < BOARD_HEIGHT; i++) {
        for (int j = 0; j < BOARD_WIDTH; j++) {
            board->pos[i][j] = s_COUNT;
        }
    }
}

/**
 * Get the row at a specific index from the board
 *
 * in:
 *  board : the board to grab from
 *  rowID   : the index of the row
 * out:
 *  u8 * that points to the beginning of the row
 */
static u8 *
GetRow(struct board *board, u32 rowID)
{
    ASSERT(0 <= rowID && rowID < BOARD_HEIGHT);
    return board->pos[rowID];
}

/**
 * Clear the rows over an index of a range
 * TODO(david): potentially just have a single call for each row?
 *
 * in:
 *  board    : board to clear a row in
 *  from       : index to start from
 *  to         : index of the last row to be cleared
 *  clearedRow : pointer to number of cleared rows
 */
static void
ClearRow(struct board *board, i32 from, i32 to, u32 *clearedRows)
{
    i32 copyFrom = to + 1;
    for (int i = 0; i < BOARD_HEIGHT - from; i++) {
        if (from < 0)
            continue;

        for (int j = 0; j < BOARD_WIDTH; j++) {
            if (copyFrom + i >= BOARD_HEIGHT)
                board->pos[from + i][j] = s_COUNT;
            else
                board->pos[from + i][j] = board->pos[copyFrom + i][j];
        }
    }

    (*clearedRows) += copyFrom - from;
}

/**
 * Helper function to set a piece on the board to a specific type
 *
 * in:
 *  board : the board where we want to set
 *  piece : the piece to be placed or removed
 *  type    : what should be written to these positions (not necessarily the piece's type)
 */
static void
_SetType(struct board *board, struct piece *piece, u32 type)
{
    for (int i = 0; i < 4; i++) {
        v2 pos = addV2(piece->pos, boardPieces[piece->type][piece->rot][i]);
        if (pos.x >= 0 && pos.y >= 0 && pos.x < BOARD_WIDTH && pos.y < BOARD_HEIGHT)
            board->pos[FloorToI32(pos.y)][FloorToI32(pos.x)] = type;
    }
}

/**
 * Place a piece on the board
 *
 * in:
 *  board : the board
 *  piece : piece to be placed
 */
static void
PlacePiece(struct board *board, struct piece *piece)
{
    _SetType(board, piece, piece->type);
}

/**
 * Remove a piece from the board
 *
 * in:
 *  board : the board
 *  piece : piece to be removed, position should be identical to that on the board
 */
static void
RemovePiece(struct board *board, struct piece *piece)
{
    _SetType(board, piece, s_COUNT);
}

/**
 * Check for a collision on the board with a piece and the potential position
 *
 * in:
 *  board : what to check for a collision on
 *  piece : information for how we're checking for a collision
 *  newPos  : where we're checking for collisions
 * out:
 *  whether or not we found a collision
 */
static bool
IsCollide(struct board *board, struct piece *piece, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, boardPieces[piece->type][piece->rot][i]);
        if (check.y >= BOARD_HEIGHT)
            continue;
        if (check.x <= -0.00001f || check.y <= -0.00001f || check.x >= BOARD_WIDTH)
            return true;
        if (board->pos[FloorToI32(check.y)][FloorToI32(check.x)] < s_COUNT)
            return true;
    }

    return false;
}

/**
 * Specifically check for a collision with the bottom
 * TODO(david): make collision checking more general, maybe with masking?
 *
 * in:
 *  board : what to check for a collision on
 *  piece : information for how we're checking for a collision
 *  newPos  : where we're checking for collisions
 * out:
 *  whether or not we found a collision
 */
static bool
IsCollideBottom(struct board *board, struct piece *piece, v2 newPos)
{
    for (int i = 0; i < 4; i++) {
        v2 check = addV2(newPos, boardPieces[piece->type][piece->rot][i]);
        if ((check.y <= -0.0001f) 
                || (board->pos[FloorToI32(check.y)][FloorToI32(check.x)] < s_COUNT))
            return true;
    }

    return false;
}
