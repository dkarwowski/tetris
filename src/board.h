#ifndef _BOARD_H_

#define BOARD_VHEIGHT 19
#define BOARD_HEIGHT 21
#define BOARD_WIDTH 10
#define BLOCK_SIZE 55

enum shapes {
    s_I = 0,
    s_T = 1,
    s_S = 2,
    s_Z = 3,
    s_L = 4,
    s_J = 5,
    s_O = 6,

    s_COUNT = 7,
    s_GHOST = 8
};

static v2 boardPieces[s_COUNT][4][4] = {{{{-2.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}},
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

struct row {
    u8 spots[BOARD_WIDTH];
    struct row *next, *prev;
    u32 y;
};

struct board {
    struct row rows[BOARD_HEIGHT];
    struct row *first, *last;
};

struct piece {
    v2 pos;
    u32 type;
    u8 rot;
};

#define for_row(iter, first) for(struct row *iter = first; iter; iter = iter->next)
#define for_row_rev(iter, last) for(struct row *iter = last; iter; iter = iter->prev)

#define _BOARD_H_
#endif
