Samizdat Layer 0: Core Library
==============================

Ints
----

<br><br>
### Generic Function Definitions: `Value` protocol

#### `perEq(int1, int2) <> int | void`

Compares the integer values of two ints.

#### `perOrder(int1, int2) <> int`

Compares the integer values of two ints, ordering by value in the usual
manner.


<br><br>
### Generic Function Definitions: `Int` protocol

#### `abs(int) <> int`

Returns the absolute value of the given value. This is `value` itself if
it is non-negative, or `-value` if it is negative.

#### `add(int1, int2) <> int`

Returns the sum of the given values.

#### `and(int1, int2) <> int`

Returns the binary-and (intersection of all one-bits) of the given values.

#### `bit(int, n) <> int`

Returns as an int (`0` or `1`) the bit value in the first
argument at the bit position (zero-based) indicated by the second
argument. It is an error (terminating the runtime) if the second
argument is negative.

#### `bitSize(int) <> int`

Returns the number of significant bits (not bytes) in
the value when represented in twos-complement form, including a
high-order sign bit. The minimum size of an int is `1`, which
is the size of both `0` and `-1`.

#### `div(int1, int2) <> int`

Returns the quotient of the given values (first over second),
using truncated division, to yield an int result. It is an
error (terminating the runtime) if the second argument is `0`.

#### `divEu(int1, int2) <> int`

Returns the quotient of the given values (first over second),
using Euclidean division, to yield an int result. It is an
error (terminating the runtime) if the second argument is `0`.

Euclidean division differs from truncating division when given
arguments of opposite sign. The usual equivalence holds with Euclidean
division that `x == (x // y) + (x %% y)` (for `y != 0`), but the
modulo result is guaranteed to be non-negative, and this means that
division is correspondingly different.

#### `mod(int1, int2) <> int`

Returns the remainder after truncated division of the given values (first
over second). The sign of the result will always match the sign of the
first argument. It is an error (terminating the runtime) if the second
argument is `0`.

The truncated modulo operation `x % y` can be defined in terms
of truncated division as `x - (x / y) * y`.

#### `modEu(int1, int2) <> int`

Returns the remainder after Euclidean division of the given values (first
over second). The sign of the result will always be positive.
It is an error (terminating the runtime) if the second
argument is `0`.

The Euclidean modulo operation `x %% y` can be defined in terms of
Euclidean division as `x - (x // y) * y`.

#### `mul(int1, int2) <> int`

Returns the product of the given values.

#### `neg(int) <> int`

Returns the negation (same magnitude, opposite sign) of the given
value.

#### `not(int) <> int`

Returns the binary complement (all bits opposite) of the given value.

#### `or(int1, int2) <> int`

Returns the binary-or (union of all one-bits) of the given values.

#### `shl(int, shift) <> int`

Returns the first argument (an int) bit-shifted an amount indicated
by the second argument (also an int). If `shift` is positive, this
is a left-shift operation. If `shift` is negative, this is a right-shift
operation. If `shift` is `0`, this is a no-op, returning the first
argument unchanged.

**Note:** The `shift` argument is not limited in any particular way (not
masked, etc.).

#### `shr(int, shift) <> int`

Returns the first argument bit-shifted by an amount indicated by the
second argument. This is identical to `shl`, except that the sense of
positive and negative `shift` is reversed.

**Note:** Unlike some other languages, there is no unsigned right-shift
operation in *Samizdat Layer 0*. This is because ints are unlimited
in bit width, and so there is no way to define such an operation. If
you need "unsigned" operations, just operate consistently on
non-negative ints.

#### `sign(int) <> int`

Returns the sign of the given value: `-1` for negative values,
`1` for positive values, or `0` for `0`.

#### `sub(int1, int2) <> int`

Returns the difference of the given values (first minus second).

#### `xor(int1, int2) <> int`

Returns the binary-xor (bitwise not-equal) of the given values.


<br><br>
### Primitive Definitions

(none)


<br><br>
### In-Language Definitions

(none)