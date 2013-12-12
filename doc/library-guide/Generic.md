Samizdat Layer 0: Core Library
==============================

Generic
-------

<br><br>
### Generic Function Definitions: `Value` protocol

#### `perEq(generic, other) <> generic | void`

Calls `totEq`.

#### `perOrder(generic, other) <> int`

Calls `totOrder`.

#### `totEq(generic1, generic2) <> generic | void`

Performs an identity comparison. As generics have identity, no two
generics are ever equal, so this only yields a value if the two arguments
refer to the same generic.

#### `totOrder(generic1, generic2) <> int`

Performs an identity comparison. Generics have a consistent, transitive, and
symmetric &mdash; but arbitrary &mdash; total order.


<br><br>
### Generic Function Definitions: `Function` protocol

#### `call(generic, args*) <> . | void`

Calls the given generic function with the given arguments. `args` must
have at least one element, and the first element is what is used to
figure out which bound function to dispatch to.

#### `canCall(generic, value) <> logic`

Returns `value` if it can be passed as the first argument to the given
generic function, that is, whether `value`'s type is bound to the
generic. Returns the `value` argument if so.


<br><br>
### Generic Function Definitions: One-Offs

#### `nameOf(type) <> . | void`

Returns the name of the generic, if it has one. This is can be
an arbitrary value, which is typically (but not necessarily) a string.


<br><br>
### Primitive Definitions

#### `genericBind(generic, type, function) <> void`

Binds the given `type` to the given `function` on the given `generic`.
It is an error (terminating the runtime) if `type` has already been bound.

#### `makeRegularGeneric(name, minArgs, maxArgs?) <> generic`

Makes and returns a new generic function. `name` may be passed as `""`
(the empty string) to indicate an anonymous generic. `minArgs` is the
minimum number of arguments that must be passed to the generic when
calling; must be at least `1`, since generic dispatch is performed on
the first argument. `maxArgs` optionally lists the most arguments that
may be passed; if not passed, there is no limit. If passed it must be
`>= minArgs`.

#### `makeUnitypeGeneric(name, minArgs, maxArgs?) <> generic`

Like `makeRegularGeneric` except that the resulting generic requires
that all arguments passed to it in any given call must all be the
same type. A non-conforming call is an error that terminates the runtime.

This is the kind of generic used for functions such as
`totEq` and `cat`.


<br><br>
### In-Language Definitions

(none)