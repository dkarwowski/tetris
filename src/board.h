#ifndef _BOARD_H_

#define BOARD_HEIGHT 19
#define BOARD_WIDTH 10
#define BLOCK_SIZE 55

enum shapes {
    s_none = 0,

    s_I = 1,
    s_T = 2,
    s_S = 3,
    s_Z = 4,
    s_L = 5,
    s_J = 6,
    s_O = 7
};

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
    v2 spots[4];
    u32 type;
};

#define s_I_0 {{-2.0f, 0.0f}, {-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}}
#define s_T_0 {{-1.0f, 0.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 1.0f}}
#define s_S_0 {{-1.0f, -1.0f}, {0.0f, -1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f}}
#define s_Z_0 {{-1.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}, {1.0f, -1.0f}}
#define s_L_0 {{1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}}
#define s_J_0 {{-1.0f, 1.0f}, {0.0f, 1.0f}, {0.0f, 0.0f}, {0.0f, -1.0f}}

#define for_row(iter, first) for(struct row *iter = first; iter; iter = iter->next)

#define _BOARD_H_
#endif
