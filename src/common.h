#ifndef _COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef uint8_t     u8;
typedef uint16_t    u16;
typedef uint32_t    u32;
typedef uint64_t    u64;

typedef int8_t      i8;
typedef int16_t     i16;
typedef int32_t     i32;
typedef int64_t     i64;

typedef char        byte;

typedef float       r32;
typedef double      r64;

typedef intptr_t    iptr;
typedef uintptr_t   uptr;

#ifdef DEBUG
#include <assert.h>
#define ASSERT(exp) assert(exp)
#define dk_inline static
#else
#define ASSERT(exp)
#define dk_inline static inline
#endif

#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

#define MIN(x, y) ((x <= y) ? x : y)
#define MAX(x, y) ((x >= y) ? x : y)

#define fequal(a, b, eps) (fabs(a - b) < eps)

#include "vectors.h"
#include "intrinsics.h"

#define _COMMON_H_
#endif

