#ifndef _GAME_H_

#include "main.h"
#include "board.h"
#include <time.h>

// MEMORY STUFF ------------------------------------------------------------------------------------------------------

typedef struct {
    u8 *base;
    size_t size;
    size_t used;

    i32 count;
} memory_stack;

typedef struct {
    memory_stack *stack_p;
    size_t used;
} local_stack;

dk_inline void
InitMemStack(memory_stack *mStack_p, void *base_p, size_t size)
{
    mStack_p->base = (u8 *)base_p;
    mStack_p->size = size;
    mStack_p->used = 0;
    mStack_p->count = 0;
}

dk_inline void
InitSubMemStack(memory_stack *slaveStack_p, memory_stack *masterStack_p, size_t size)
{
    ASSERT(masterStack_p->used + size <= masterStack_p->size);
    InitMemStack(slaveStack_p, masterStack_p->base + masterStack_p->used, size);
    masterStack_p->used += size;
}

dk_inline void
ClearMemStack(memory_stack *mStack_p)
{
    InitMemStack(mStack_p, mStack_p->base, mStack_p->size);
}

dk_inline size_t
StackRemaining(memory_stack *mStack_p)
{
    size_t result = mStack_p->size - mStack_p->used;
    return result;
}

#define ZeroStruct(instance) ZeroSize(&instance, sizeof(instance))
#define ZeroArray(arr, count) ZeroSize((void *)array, sizeof(arr[0])*count)
dk_inline void
ZeroSize(void *base_p, size_t size)
{
    char *b_p = (char *)base_p;
    while (size--)
        *(b_p++) = 0;
}

#define PushStruct(mStack_p, type, ...) (type *)PushSize_(mStack_p, sizeof(type), ##__VA_ARGS__)
#define PushArray(mStack_p, type, count, ...) (type *)PushSize_(mStack_p, sizeof(type)*count, ##__VA_ARGS__)
dk_inline void *
PushSize_(memory_stack *mStack_p, size_t size, bool clear)
{
    ASSERT((mStack_p->used + size) <= mStack_p->size);

    void *result = mStack_p->base + mStack_p->used;
    mStack_p->used += size;

    if (clear)
        ZeroSize(result, size);

    return result;
}

dk_inline void *
PushCopy_(memory_stack *mStack_p, void *base_p, size_t size)
{
    ASSERT((mStack_p->used + size) <= mStack_p->size);

    char *dest_p = (char *)(mStack_p->base + mStack_p->used);
    char *src_p  = (char *)base_p;

    while (size--) {
        *(dest_p++) = *(src_p++);
    }

    void *result = mStack_p->base + mStack_p->used;
    mStack_p->used += size;
    return result;
}

dk_inline void
Copy(void *dest, void *src, size_t size)
{
    char *dest_p = (char *)dest;
    char *src_p  = (char *)src;

    while(size--)
        *(dest_p++) = *(src_p++);
}

dk_inline local_stack
BeginLocalStack(memory_stack *mStack_p)
{
    local_stack result;

    result.stack_p = mStack_p;
    result.used = mStack_p->used;

    return result;
}

dk_inline void
EndLocalStack(local_stack *lStack_p)
{
    memory_stack *mStack_p = lStack_p->stack_p;
    ASSERT(lStack_p->used >= mStack_p->used);

    mStack_p->used = lStack_p->used;
    ASSERT(mStack_p->count > 0);
    mStack_p->count--;
}

// GAME SHIT ---------------------------------------------------------------------------------------------------------

enum {
    G_NEW_GAME = 0,
    G_PLAYING = 1,
    G_PAUSED = 2,
    G_PAUSED_WAIT = 3,
    G_QUIT = 4,
    G_PAUSED_PLAY = 5
} G_state;

struct game_state {
    struct board board;
    struct board nextView;
    struct board holdView;
    struct piece dropping;
    struct piece next;
    struct piece hold;

    u64 score;
    u32 clearedGoal;
    u32 clearedRows;
    TTF_Font *font;

    u32 droppingLand;
    v2 ghostPos;

    r32 dropSpeed;
    i32 move;
    i32 moveMod;
    i32 checkClear;

    i32 lastRotPress;
    i32 lastDrop;

    u64 gameType;

    i32 pauseCount;
    i32 lastPause;

    bool canHold;
    i32 holdCount;

    memory_stack stack;
};

#define _GAME_H_
#endif
