/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Safe integer functions
 *
 * Notes
 * -----
 *
 * With regard to detecting overflow, C compilers are allowed to optimize
 * overflowing operations in surprising ways. Therefore, the usual
 * recommendation for portable code is to detect overflow by inspection of
 * operands, not results. The choices for what to detect here are informed by,
 * but not exactly the same as, the CERT "AIR" recommendations.
 *
 * With regard to division and modulo, this file implements both
 * traditional truncating division and Euclidean division, each with its
 * corresponding modulo operation. These are implemented to assume the C99
 * definition of division and remainder, in particular that division is
 * truncating and that the sign of a modulo result is the same as the sign
 * of the dividend (the left-hand argument).
 *
 * This file is implemented *almost* entirely without assuming an underlying
 * int representation, not for its own sake but instead because that's what's
 * most readable (and is also not horrendously inefficient). In a couple
 * cases, though, the code does assume that ints are represented as
 * twos-complement binary values, since that makes the code easier to
 * understand.
 *
 * References:
 *
 * * [As-If Infinitely Ranged Integer Model, Second
 *   Edition](http://www.cert.org/archive/pdf/10tn008.pdf)
 *
 * * [CERT C Secure Coding
 *   Standard](https://www.securecoding.cert.org/confluence/display/
 *   seccode/CERT+C+Secure+Coding+Standard)
 *
 * * [Division and Modulus for Computer
 *   Scientists](http://legacy.cs.uu.nl/daan/download/papers/divmodnote.pdf)
 *
 * * [The Euclidean Definition of the Functions div and
 *   mod](https://biblio.ugent.be/publication/314490/file/452146.pdf)
 *
 * * [Modulo operation
 *   (Wikipedia)](http://en.wikipedia.org/wiki/Modulo_operation)
 *
 * * [Safe IOP Library](https://code.google.com/p/safe-iop/)
 */

#ifndef _UTIL_ZINT_H_
#define _UTIL_ZINT_H_

#include <stddef.h>

/*
 * Private definitions
 */

/*
 * Public definitions
 */

/**
 * Converts a `zint` to a `zchar`, detecting overflow. Returns
 * a success flag, and stores the result in the indicated pointer if
 * non-`NULL`.
 */
inline bool zcharFromZint(zchar *result, zint value) {
    if ((value < 0) || (value > ZCHAR_MAX)) {
        return false;
    }

    if (result != NULL) {
        *result = (zchar) value;
    }

    return true;
}

/**
 * Performs `abs(x)` (unary absolute value), detecting overflow. Returns
 * a success flag, and stores the result in the indicated pointer if
 * non-`NULL`.
 *
 * **Note:** The only possible overflow case is `abs(ZINT_MIN)`.
 */
inline bool zintAbs(zint *result, zint x) {
    if (x == ZINT_MIN) {
        return false;
    }

    if (result != NULL) {
        *result = (x < 0) ? -x : x;
    }

    return true;
}

/**
 * Performs `x + y`, detecting overflow. Returns a success flag, and
 * stores the result in the indicated pointer if non-`NULL`.
 */
inline bool zintAdd(zint *result, zint x, zint y) {
    // If the signs are opposite or either argument is zero, then overflow
    // is impossible. The two clauses here are for the same-sign-and-not-zero
    // cases. Each one is of the form: Given a {positive, negative} `y`,
    // what is the {largest, smallest} `x` that won't overflow?
    if (((y > 0) && (x > (ZINT_MAX - y))) ||
        ((y < 0) && (x < (ZINT_MIN - y)))) {
        return false;
    }

    if (result != NULL) {
        *result = x + y;
    }

    return true;
}

/**
 * Performs `x &&& y`. Returns `true`, and stores the result in the
 * indicated pointer if non-`NULL`. This function never fails; the success
 * flag is so that it can be used equivalently to the other similar functions
 * in this library.
 */
inline bool zintAnd(zint *result, zint x, zint y) {
    if (result != NULL) {
        *result = x & y;
    }

    return true;
}

/**
 * Performs bit extraction `(x >>> y) &&& 1`, detecting errors. Returns a
 * success flag, and stores the result in the indicated pointer if non-`NULL`.
 * For `y >= ZINT_BITS`, this returns the sign bit.
 *
 * **Note:** The only possible errors are when `y < 0`.
 */
inline bool zintBit(zint *result, zint x, zint y) {
    if (y < 0) {
        return false;
    }

    if (result != NULL) {
        if (y >= ZINT_BITS) {
            y = ZINT_BITS - 1;
        }

        *result = (x >> y) & 1;
    }

    return true;
}


/**
 * Gets the bit size (highest-order significant bit number, plus one)
 * of the given `zint`, assuming sign-extended representation. For example,
 * this is `1` for both `0` and `-1` (because both can be represented with
 * *just* a single sign bit); and this is `2` for `1` (because it requires
 * one value bit and one sign bit).
 */
inline zint zintBitSize(zint value) {
    if (value < 0) {
        value = ~value;
    }

    // "Binary-search" style implementation. Many compilers have a
    // built-in "count leading zeroes" function, but we're aiming
    // for portability here.

    zint result = 1; // +1 in that we want size, not zero-based bit number.
    uint64_t uv = (uint64_t) value; // Use `uint` to account for `-ZINT_MAX`.

    if (uv >= ((uint64_t) 1 << 32)) { result += 32; uv >>= 32; }
    if (uv >= ((uint64_t) 1 << 16)) { result += 16; uv >>= 16; }
    if (uv >= ((uint64_t) 1 << 8))  { result +=  8; uv >>=  8; }
    if (uv >= ((uint64_t) 1 << 4))  { result +=  4; uv >>=  4; }
    if (uv >= ((uint64_t) 1 << 2))  { result +=  2; uv >>=  2; }
    if (uv >= ((uint64_t) 1 << 1))  { result +=  1; uv >>=  1; }
    return result + uv;
}

/**
 * Performs `x / y` (trucated division), detecting overflow and errors.
 * Returns a success flag, and stores the result in the indicated pointer
 * if non-`NULL`.
 *
 * **Note:** The only possible overflow case is `ZINT_MIN / -1`, and the
 * only other error is division by zero.
 */
bool zintDiv(zint *result, zint x, zint y);

/**
 * Performs `x // y` (Euclidean division), detecting overflow and errors.
 * Returns a success flag, and stores the result in the indicated pointer
 * if non-`NULL`.
 *
 * **Note:** The only possible overflow case is `ZINT_MIN / -1`, and the
 * only other error is division by zero.
 */
bool zintDivEu(zint *result, zint x, zint y);

/**
 * Performs `x % y` (that is, remainder after truncated division, with the
 * result sign matching `x`), detecting overflow. Returns a success flag, and
 * stores the result in the indicated pointer if non-`NULL`.
 *
 * **Note:** This will not fail if an infinite-size int implementation
 * would succeed. In particular, `ZINT_MIN % -1` succeeds and returns `0`.
 */
bool zintMod(zint *result, zint x, zint y);

/**
 * Performs `x %% y` (that is, remainder after Euclidean division, with the
 * result sign always positive), detecting overflow. Returns a success flag,
 * and stores the result in the indicated pointer if non-`NULL`.
 *
 * **Note:** This will not fail if an infinite-size int implementation
 * would succeed. In particular, `ZINT_MIN %% -1` succeeds and returns `0`.
 */
bool zintModEu(zint *result, zint x, zint y);

/**
 * Performs `x * y`, detecting overflow. Returns a success flag, and
 * stores the result in the indicated pointer if non-`NULL`.
 */
bool zintMul(zint *result, zint x, zint y);

/**
 * Performs `-x` (unary negation), detecting overflow. Returns a success flag,
 * and stores the result in the indicated pointer if non-`NULL`.
 *
 * **Note:** The only possible overflow case is `-ZINT_MIN`.
 */
bool zintNeg(zint *result, zint x);

/**
 * Performs `!!!x` (unary bitwise complement). Returns `true`,
 * and stores the result in the indicated pointer if non-`NULL`. This
 * function never fails; the success flag is so that it can be used
 * equivalently to the other similar functions in this library.
 */
inline bool zintNot(zint *result, zint x) {
    if (result != NULL) {
        *result = ~x;
    }

    return true;
}

/**
 * Performs `x ||| y`. Returns `true`, and stores the result in the
 * indicated pointer if non-`NULL`. This function never fails; the success
 * flag is so that it can be used equivalently to the other similar functions
 * in this library.
 */
inline bool zintOr(zint *result, zint x, zint y) {
    if (result != NULL) {
        *result = x | y;
    }

    return true;
}

/**
 * Performs `sign(x)`. Returns `true`, and stores the result in the
 * indicated pointer if non-`NULL`. This function never fails; the success
 * flag is so that it can be used equivalently to the other similar functions
 * in this library.
 */
inline bool zintSign(zint *result, zint x) {
    if (result != NULL) {
        *result = (x == 0) ? 0 : ((x < 0) ? -1 : 1);
    }

    return true;
}

/**
 * Performs `x <<< y`, detecting overflow (never losing high-order bits).
 * Returns a success flag, and stores the result in the indicated pointer
 * if non-`NULL`.
 *
 * **Note:** This defines `(x <<< -y) == (x >>> y)`.
 */
bool zintShl(zint *result, zint x, zint y);

/**
 * Performs `x >>> y`, detecting overflow (never losing high-order bits).
 * Returns a success flag, and stores the result in the indicated pointer
 * if non-`NULL`.
 *
 * **Note:** This defines `(x >>> -y) == (x <<< y)`.
 */
inline bool zintShr(zint *result, zint x, zint y) {
    // We just define this in terms of `zintShl`, but note the limit test,
    // which ensures that we don't try to calculate `-ZINT_MIN` for `y`.
    return zintShl(result, x, (y <= -ZINT_BITS) ? ZINT_BITS : -y);
}

/**
 * Performs `x - y`, detecting overflow. Returns a success flag, and
 * stores the result in the indicated pointer if non-`NULL`.
 */
bool zintSub(zint *result, zint x, zint y);

/**
 * Performs `x ^^^ y`. Returns `true`, and stores the result in the
 * indicated pointer if non-`NULL`. This function never fails; the success
 * flag is so that it can be used equivalently to the other similar functions
 * in this library.
 */
inline bool zintXor(zint *result, zint x, zint y) {
    if (result != NULL) {
        *result = x ^ y;
    }

    return true;
}

#endif
