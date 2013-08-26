Samizdat Layer 0: Core Library
==============================

Boxes
-----

<br><br>
### Generic Function Definitions: `Value` protocol

#### `perEq(box1, box2) <> box | void`

Performs an identity comparison. Two boxes are only equal if they are
truly the same box.

#### `perOrder(box1, box2) <> int`

Performs an identity comparison. Boxes have a consistent, transitive, and
symmetric &mdash; but arbitrary &mdash; total order.


<br><br>
### Generic Function Definitions: `Box` protocol

#### `canStore(box) <> logic`

Returns `box` if the `box` can be stored to. Otherwise returns void.
`box` must be a box as returned by either `makeMutableBox` or `makeYieldBox`.

#### `fetch(box) <> . | void`

Gets the value inside a box, if any. If the box either is unset or has
been set to void, this returns void. `box` must be a box as returned by
either `makeMutableBox` or `makeYieldBox`.

#### `store(box, value?) <> .`

Sets the value of a box to the given value, or to void if `value` is
not supplied. This function always returns `value` (or void if `value` is
not supplied). `box` must be a box as returned by either `makeMutableBox` or
`makeYieldBox`.

It is an error (terminating the runtime) for `box` to be a yield box on
which `store` has already been called.


<br><br>
### Primitive Definitions

#### `makeMutableBox(value?) <> box`

Creates a mutable box, with optional pre-set value. The result of a call to
this function is a box which can be set any number of times using
`store`. The contents of the box are accessible by calling `fetch`.

The initial box content value is the `value` given to this function. This
is what is returned from `fetch` until `store` is called to replace it.
If `value` is not supplied, `fetch` returns void until `store` is called.

This function is meant to be the primary way to define (what amount to)
mutable variables, in that *Samizdat Layer 0* only provides immutably-bound
variables. It is hoped that this facility will be used as minimally as
possible, so as to not preclude the system from performing functional-style
optimizations.

#### `makeYieldBox() <> box`

Creates a set-once "yield box". The result of a call to this function is a
box which can be stored to at most once, using `store`. Subsequent
attempts to store to the box will fail (terminating the runtime). The
contents of the box are accessible by calling `fetch`. `fetch` returns
void until and unless `store` is called with a second argument.

This function is meant to be the primary way to capture the yielded values
from functions (such as object service functions and parser functions) which
expect to yield values by calling a function.

<br><br>
### In-Language Definitions

(none)