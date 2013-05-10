Samizdat Layer 0: Core Library
==============================

Maps
----

<br><br>
### Primitive Definitions

#### `mapAdd maps* <> map`

Returns a map consisting of the combination of the mappings of the
argument maps. For any keys in common between the maps,
the lastmost argument's value is the one that ends up in the result.

#### `mapDel map key <> map`

Returns a map just like the one given as an argument, except that
the result does not have a binding for the key `key`. If the given
map does not have `key` as a key, then this returns the given
map as the result.

#### `mapGet map key notFound? <> . | ~.`

Returns the value mapped to the given key (an arbitrary value) in
the given map. If there is no such mapping, then this
returns the `notFound` value (an arbitrary value) if supplied,
or returns void if `notFound` was not supplied.

#### `mapHasKey map key <> boolean`

Returns `true` iff the given map has a mapping for the given key.

#### `mapNth map n notFound? <> . | ~.`

Returns the `n`th (zero-based) mapping of the given map, if `n` is
a valid integer index into the map. If `n` is not a valid index
(either an out-of-range integer, or some other value), then this
returns the `notFound` value (an arbitrary value) if supplied, or
returns void if `notFound` was not supplied.

The ordering of the mappings is by sort order of the keys.

#### `mapNthKey map n notFound? <> . | ~.`

Returns the key of the `n`th (zero-based) mapping of the given map,
if `n` is a valid integer index into the map. If `n` is not a valid index
(either an out-of-range integer, or some other value), then this
returns the `notFound` value (an arbitrary value) if supplied, or
returns void if `notFound` was not supplied.

The ordering of the mappings is by sort order of the keys.

#### `mapNthValue map n notFound? <> . | ~.`

Returns the value of the `n`th (zero-based) mapping of the given map,
if `n` is a valid integer index into the map. If `n` is not a valid index
(either an out-of-range integer, or some other value), then this
returns the `notFound` value (an arbitrary value) if supplied, or
returns void if `notFound` was not supplied.

The ordering of the mappings is by sort order of the keys.

#### `mapPut map key value <> map`

Returns a map just like the given one, except with a new binding
for `key` to `value`. The result has a replacement for the existing
binding for `key` in `map` if such a one existed, or has an
additional binding in cases where `map` didn't already bind `key`.
These two scenarios can be easily differentiated by either noting a
change in size (or not) between original and result, or by explicitly
checking for the existence of `key` in the original.


<br><br>
### In-Language Definitions

#### `mapForEach map function <> ~.`

Calls the given function for each mapping in the given map. The
function is called with two arguments, a value from the map and
its corresponding key (in that order). Always returns void.

#### `mapMap map function <> map`

Maps the values of a map using the given mapping function,
resulting in a map whose keys are the same as the given map but
whose values may differ. In key order, the function is called with
two arguments representing the binding, a value and a key (in that
order, because it is common enough to want to ignore the key). The
return value of the function becomes the bound value for the given
key in the final result.

Similar to `argsMap`, if the function returns void, then no item is
added for the corresponding element. This means the size of the
result may be smaller than the size of the argument.

See note on `stringMap` about choice of argument order.

#### `mapReduce base map function <> . | ~.`

Reduces a map to a single value, given a base value and a reducer
function, operating in key order. The given function is called on each
map binding, with three arguments: the last (or base) reduction
result, the value associated with the binding, and its key. The
function result becomes the reduction result, which is passed to the
next call of `function` or becomes the return value of the call to
this function if it was the call for the final binding.

Similar to `argsReduce`, if the function returns void, then the
previously-returned non-void value (or `base` value if there has
yet to be a non-void function return) is what is passed to the
subsequent iteration and returned at the end of the call.

See note on `stringMap` about choice of argument order.