#define BOARD_HEIGHT 19
#define BOARD_WIDTH 10
#define BLOCK_SIZE 55

enum shapes {
    s_none = 0,

    s_line = 1,
    s_T    = 2,
    s_s    = 3,
    s_z    = 4,
    s_l    = 5,
    s_bl   = 6
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

#define for_row(iter, first) for(struct row *iter = first; iter; iter = iter->next)
