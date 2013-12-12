Samizdat Layer 0: Core Library
==============================

core::Box / proto::Box
----------------------

A `Box` is a container for a single other value or for void.
In terms of value comparison, all boxes should compare by identity,
and not by "happenstance content". That is, two boxes should only be
considered "equal" if they are indistinguishable, even in the face of
calling mutating operations.

The `Box` module provides the generic functions of the `Box` protocol,
as well as related constructors and constants.


<br><br>
### Generic Function Definitions: `Box` protocol

#### `canStore(box) <> logic`

Returns `box` if the `box` can be stored to. Otherwise returns void.

#### `fetch(box) <> . | void`

Gets the value inside a box, if any. If the box either is unset or has
been set to void, this returns void.

#### `store(box, value?) <> . | void`

Sets the value of a box to the given value, or to void if `value` is
not supplied. This function always returns `value` (or void if `value` is
not supplied).

It is an error (terminating the runtime) for `box` to be a yield box on
which `store` has already been called.


<br><br>
### Primitive Definitions

#### `makeCell(value?) <> cell`

Creates a cell (a mutable box), with optional pre-set value. The result of
a call to this function is a cell which can be set any number of times using
`store`. The contents of the cell are accessible by calling `fetch`.

The initial cell content value is the `value` given to this function. This
is what is returned from `fetch` until `store` is called to replace it.
If `value` is not supplied, `fetch` returns void until `store` is called.

This function is meant to be the primary way to define (what amount to)
mutable variables, in that *Samizdat Layer 0* only provides immutably-bound
variables. It is hoped that this facility will be used as minimally as
possible, so as to not preclude the system from performing functional-style
optimizations.

#### `makePromise() <> promise`

Creates a promise (set-at-most-once box). The result of a call to this
function is a box which can be stored to at most once, using `store`.
Subsequent attempts to call `store` will fail (terminating the runtime). The
contents of the promise are accessible by calling `fetch`. `fetch` returns
void until and unless `store` is called with a second argument.

This function is meant to be the primary way to capture the yielded values
from functions (such as object service functions and parser functions) which
expect to yield values by calling a function.


<br><br>
### In-Language Definitions

#### Constant: `nullBox`

A value that represents a permanently empty (un-set, un-stored) box.
`store(nullBox, value)` is effectively a no-op. This
arrangement is done in order to make it easy to pass a box into functions
that require one, but where the box value is never needed.

It is defined as `@NullBox`.

#### `update(box, updateFunction) <> . | void`

Updates a box's value. This fetches the value out of `box` if any, passing
it as an argument to `updateFunction` (or passing no arguments if `box` was
empty). The return value from the call to `updateFunction` is then stored
back to `box`; void is stored if `updateFunction` returned void.

This function returns whatever the `store` call on `box` returns.