/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "const.h"
#include "impl.h"
#include "util.h"

#include <stddef.h>

/*
 * Helper Definitions
 */

#define UNARY_PRIM(name, op) \
    PRIM_IMPL(name) { \
        zint x = zintFromInt(args[0]); \
        return intFromZint((op)); \
    } \
    extern int semicolonRequiredHere

#define BINARY_PRIM(name, op) \
    PRIM_IMPL(name) { \
        zint x = zintFromInt(args[0]); \
        zint y = zintFromInt(args[1]); \
        return intFromZint((op)); \
    } \
    extern int semicolonRequiredHere

/**
 * Division operation. This isn't a straightforward application of
 * `/`, so that division by zero can operate as specified.
 */
static zint div(zint numerator, zint denominator) {
    if (denominator == 0) {
        die("Attempt to divide by zero: %lld", numerator);
    }

    return numerator / denominator;
}

/**
 * Remainder operation. This isn't a straightforward application
 * of `%`, because:
 *
 * * We need to handle division by zero as a failure.
 *
 * * Though well specified as of C99, in practice there is
 *   variation across implementations of the `%` operator.
 */
static zint rem(zint numerator, zint denominator) {
    zint q = div(numerator, denominator);
    return numerator - (q * denominator);
}

/**
 * Modulo operation. This has to be a function, since C defines no
 * portable modulo function.
 */
static zint mod(zint numerator, zint denominator) {
    zint r = rem(numerator, denominator);
    return rem(r + denominator, denominator);
}

/**
 * Binary left-shift operation. This isn't a straightforward
 * application of `<<` to the two arguments, because:
 *
 * * The C spec demands that the right-hand argument be truncated to
 *   type `int`.
 *
 * * The C spec allows unpredictable behavior when the right-hand argument
 *   is negative. In *Samizdat Layer 0* negative shifts indicate a
 *   right-shift operation.
 *
 * * We need to preserve the sign of the result, since we are simulating
 *   infinite-width integers.
 *
 * **Note:** We limit shifts to 32 bits here, relying on this to be
 * meaning-preserving in the face of the `dat` layer's implementation
 * of the 32-bit width restriction on ints.
 */
static zint shl(zint value, zint shift) {
    if (shift == 0) {
        return value;
    } else if (shift > 0) {
        // A left shift.
        if (shift > 32) {
            shift = 32;
        }
        return value << shift;
    } else {
        // A right shift.
        shift = -shift;
        if (shift > 32) {
            shift = 32;
        }
        return value >> shift;
    }
}

/**
 * Helper for `intGet` and `intNth`, which acts like `ibit` except that
 * out-of-range indices result in void, not the sign bit.
 */
static zvalue doIntNth(zvalue value, zint n) {
    if (n < pbSize(value)) {
        return intFromZint(intGetBit(value, n));
    } else {
        return NULL;
    }
}


/*
 * Exported Definitions
 */

/* These are all documented in Samizdat Layer 0 spec. */
UNARY_PRIM(iabs,  (x >= 0) ? x : -x);
UNARY_PRIM(ineg,  -x);
UNARY_PRIM(inot,  ~x);
UNARY_PRIM(isign, (x == 0) ? 0 : ((x > 0) ? 1 : -1));

/* These are all documented in Samizdat Layer 0 spec. */
BINARY_PRIM(iadd, x + y);
BINARY_PRIM(iand, x & y);
BINARY_PRIM(ibit, zintGetBit(x, y));
BINARY_PRIM(idiv, div(x, y));
BINARY_PRIM(imod, mod(x, y));
BINARY_PRIM(imul, x * y);
BINARY_PRIM(ior,  x | y);
BINARY_PRIM(irem, rem(x, y));
BINARY_PRIM(isub, x - y);
BINARY_PRIM(ishl, shl(x, y));
BINARY_PRIM(ishr, shl(x, -y));
BINARY_PRIM(ixor, x ^ y);

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(intGet) {
    return doNthLenient(doIntNth, args[0], args[1]);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(intNth) {
    return doNthStrict(doIntNth, args[0], args[1]);
}
