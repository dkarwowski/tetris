#ifndef _VECTORS_H_
#define _VECTORS_H_
#include <math.h>

#define PI32 3.1415926535897932384623233832795028841971693993751058209749445923078132062f

// actual vectors

typedef union {
    struct {
        r32 x, y;
    };
    struct {
        r32 w, h;
    };
    struct {
        i32 xi, yi; // NOTE(david): only use i when you KNOW they're i's
    };
    r32 e[2];
} v2;

typedef union {
    struct {
        r32 x, y, z;
    };
    struct {
        r32 r, g, b;
    };
    struct {
        r32 h, s, v;
    };
    struct {
        i32 xi, yi, zi;
    };
    r32 e[3];
} v3;

typedef union {
    struct {
        r32 x, y, z, w;
    };
    struct {
        r32 r, g, b, a;
    };
    struct {
        i32 xi, yi, zi, wi;
    };
    r32 e[4];
} v4;

// operations --------------------------------------

dk_inline v2
V2(r32 a, r32 b)
{
    v2 result = {a, b};
    return result;
}

dk_inline v3
V3(r32 a, r32 b, r32 c)
{
    v3 result = {a, b, c};
    return result;
}

dk_inline v4
V4(r32 a, r32 b, r32 c, r32 d)
{
    v4 result = {a, b, c, d};
    return result;
}

// TODO(david): so... i don't want this here. at all. 
// either switch to C++ compiling to use overloading, or create
// a separate int v2 thing...? macros...?
dk_inline v2
V2i(i32 a, i32 b)
{
    v2 result = {a, b};
    return result;
}

dk_inline v3
V3i(i32 a, i32 b, i32 c)
{
    v3 result = {a, b, c};
    return result;
}

dk_inline v4
V4i(i32 a, i32 b, i32 c, i32 d)
{
    v4 result = {a, b, c, d};
    return result;
}

dk_inline bool
equalV2(v2 a, v2 b)
{
    bool result = fequal(a.x, b.x, 0.0001) && fequal(a.y, b.y, 0.0001);
    return result;
}

dk_inline v2
hadamardV2(v2 a, v2 b)
{
    v2 result = {a.x * b.x, a.y * b.y};
    return result;
}

dk_inline v3
hadamardV3(v3 a, v3 b)
{
    v3 result = {a.x * b.x, a.y * b.y, a.z * b.z};
    return result;
}

dk_inline v4
hadamardV4(v4 a, v4 b)
{
    v4 result = {a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w};
    return result;
}

dk_inline v2
swapV2(v2 a)
{
    v2 result = {0};
    result.x = a.y;
    result.y = a.x;
    return result;
}

dk_inline r32
innerV2(v2 a, v2 b)
{
    r32 result = 0;
    result += a.x * b.x;
    result += a.y * b.y;
    return result;
}

dk_inline v2
negV2(v2 a)
{
    v2 result = {0};
    result.x = -a.x;
    result.y = -a.y;
    return result;
}

dk_inline v3
negV3(v3 a)
{
    v3 result = {0};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    return result;
}

dk_inline v4
negV4(v4 a)
{
    v4 result = {0};
    result.x = -a.x;
    result.y = -a.y;
    result.z = -a.z;
    result.w = -a.w;
    return result;
}

dk_inline v2
addV2(v2 a, v2 b)
{
    v2 result = {0};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    return result;
}

dk_inline v3
addV3(v3 a, v3 b)
{
    v3 result = {0};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    return result;
}

dk_inline v4
addV4(v4 a, v4 b)
{
    v4 result = {0};
    result.x = a.x + b.x;
    result.y = a.y + b.y;
    result.z = a.z + b.z;
    result.w = a.w + b.w;
    return result;
}

dk_inline v2
subV2(v2 a, v2 b)
{
    v2 result = addV2(a, negV2(b));
    return result;
}

dk_inline v3
subV3(v3 a, v3 b)
{
    v3 result = addV3(a, negV3(b));
    return result;
}

dk_inline v4
subV4(v4 a, v4 b)
{
    v4 result = addV4(a, negV4(b));
    return result;
}

dk_inline v2
mulV2(r32 a, v2 b)
{
    v2 result = {0};
    result.x = a * b.x;
    result.y = a * b.y;
    return result;
}

dk_inline v3
mulV3(r32 a, v3 b)
{
    v3 result = {0};
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    return result;
}

dk_inline v4
mulV4(r32 a, v4 b)
{
    v4 result = {0};
    result.x = a * b.x;
    result.y = a * b.y;
    result.z = a * b.z;
    result.w = a * b.w;
    return result;
}

dk_inline v2
divV2(r32 a, v2 b)
{
    v2 result = mulV2(1.0f/a, b);
    return result;
}

dk_inline v3
divV3(r32 a, v3 b)
{
    v3 result = mulV3(1.0f/a, b);
    return result;
}

dk_inline v4
divV4(r32 a, v4 b)
{
    v4 result = mulV4(1.0f/a, b);
    return result;
}

dk_inline v2
NormalizeV2(v2 cart)
{
    v2 result = cart;
    r32 len = sqrt(cart.x * cart.x + cart.y * cart.y);
    if (len != 0)
        result = divV2(len, cart);
    return result;
}

dk_inline v4
HSVtoRGB(v3 col)
{
    v4 result = {0};

    if (col.s <= 0)
        result = (v4){col.s, col.s, col.s, 1.0f};
    else {

        // assume that 0 <= h <= 360
        i32 section = (i32)(col.h / 60.0f);
        r32 f = col.h - section;            // fractional
        r32 p = col.v * (1 - col.s);
        r32 q = col.v * (1 - col.s * f);
        r32 t = col.v * (1 - col.s * (1 - f));

        switch(section)
        {
        case 0: 
            result = (v4){col.v, t, p, 1.0f};
            break;
        case 1:
            result = (v4){q, col.v, p, 1.0f};
            break;
        case 2:
            result = (v4){p, col.v, t, 1.0f};
            break;
        case 3:
            result = (v4){p, q, col.v, 1.0f};
            break;
        case 4:
            result = (v4){t, p, col.v, 1.0f};
            break;
        default:
            result = (v4){col.v, p, q, 1.0f};
            break;
        }
    }
    result = mulV4(255, result);

    return result;
}

#define RGBA_PARAMS(vec4) (vec4).r,(vec4).g,(vec4).b,(vec4).a

#endif
