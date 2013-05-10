Samizdat Layer 0: Core Library
==============================

Lists
-----

<br><br>
### Primitive Definitions

#### `listAdd lists* <> list`

Returns a list consisting of the concatenation of the elements
of all the argument lists, in argument order.

#### `listDelNth list n <> list`

Returns a list just like the given one, except that the `n`th
(zero-based) element is deleted, if `n` is a valid integer index into
the given list. If `n` is not a valid index (not an integer, or with
a value out of range), then this returns the original list as the
result.

#### `listInsNth list n value <> list`

Returns a list just like the given one, except that the `n`th
(zero-based) element is set to be the given value (an arbitrary
value), and all elements at or beyond index `n` in the original
are shifted up by one index.

`n` must be non-negative and less than or equal to the size of the
list. If not, it is an error (terminating the runtime).

#### `listNth list n notFound? <> . | ~.`

Returns the `n`th (zero-based) element of the given list, if `n` is
a valid integer index into the list. If `n` is not a valid index
(either an out-of-range integer, or some other value), then this
returns the `notFound` value (an arbitrary value) if supplied, or
returns void if `notFound` was not supplied.

#### `listPutNth list n value <> list`

Returns a list just like the given one, except that the `n`th
(zero-based) element is set to be the given value (an arbitrary
value), replacing whatever was at that index in the original. If
`n` is the size of the original list, then this call behaves
exactly like `listAppend`.

`n` must be non-negative and less than or equal to the size of the
list. If not, it is an error (terminating the runtime).


<br><br>
### In-Language Definitions


#### `listAppend list value <> list`

Returns a list consisting of the elements of the given
list argument followed by the given additional value.

#### `listForEach list function <> ~.`

Calls the given function on each element of the given list.
The given function is called with two arguments, namely the element
and its index number (zero-based). Always returns void.

#### `listMap list function <> list`

Maps each element of a list using a mapping function, collecting
the results into a new list. The given function is called on each
list element, with two arguments, namely the element and its index
number (zero-based).

Similar to `argsMap`, if the function returns void, then no item is
added for the corresponding element. This means the size of the
result may be smaller than the size of the argument.

See note on `stringMap` about choice of argument order.

#### `listPrepend value list <> list`

Returns a list consisting of the given first value followed by the
elements of the given list argument.

**Note:** The arguments are given in an order meant to reflect the
result (and not list-first).

#### `listReduce base list function <> . | ~.`

Reduces a list to a single value, given a base value and a reducer
function, operating in low-to-high index order (that is, this is a
left-reduce operation). The given function is called on each list
element, with three arguments: the last (or base) reduction result,
the element, and its index number (zero-based). The function result
becomes the reduction result, which is passed to the next call of
`function` or becomes the return value of the call to this function if
it was the call for the final element.

Similar to `argsReduce`, if the function returns void, then the
previously-returned non-void value (or `base` value if there has
yet to be a non-void function return) is what is passed to the
subsequent iteration and returned at the end of the call.

See note on `stringMap` about choice of argument order.