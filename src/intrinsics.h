#ifndef _INTRINSICS_H_
#define _INTRINSICS_H_
#include "common.h"

dk_inline i32
FloorToI32(r32 a)
{
    i32 result = (i32)floorf(a);
    return result;
}

dk_inline r32
FloorToR32(r32 a)
{
    r32 result = floorf(a);
    return result;
}

dk_inline i32
RoundToI32(r32 a)
{
    i32 result = lroundf(a);
    return result;
}

#endif
