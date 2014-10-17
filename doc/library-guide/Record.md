Samizdat Layer 0: Core Library
==============================

Record
------

<br><br>
### Class Method Definitions

#### `.new(name, data?) -> record`

Returns a record with the given name (a symbol) and optional `data` payload
value (which must be a symbol table if present). If `data` is not specified,
it defaults to `@{}` (the empty symbol table). These equivalences hold for
Samizdat source code:

```
@x{}            is equivalent to  Record.new(@x)
@(expr){}       is equivalent to  Record.new(expr)
@x(value)       is equivalent to  Record.new(@x, value)
@(expr)(value)  is equivalent to  Record.new(expr, value)
@x{key: value}  is equivalent to  Record.new(@x, @{key: value})
```

It is a fatal error (terminating the runtime) to pass for `name` anything
other than a symbol. It is also a fatal error to pass for `data` anything
other than a symbol table.

**Syntax Note:** Used in the translation of `@(type)(value)` and related forms.


<br><br>
### Method Definitions: `Record` protocol

#### `.get(symbol) -> . | void`

Returns the value mapped to the given `symbol` (a symbol) in the given
record. If there is no such mapping, then this returns void.

#### `.get_data() -> symbolTable`

Returns the data payload of the given record.

#### `.get_name() -> symbol`

Returns the name (tag) of the given record.

#### `.hasName(name) -> record`

Returns `this` if its name is as given, or void if not.


<br><br>
### Method Definitions: `Value` protocol

#### `.perEq(other) -> record | void`

Default implementation.

#### `.perOrder(other) -> int`

Default implementation.

#### `.totalEq(other) -> record | void`

Compares two records. Two records are equal if they have equal names and
equal data payloads.

#### `.totalOrder(other) -> int`

Compares two records for order. Records order by name as the major order
and data payload as minor order.


### Primitive Definitions

(none)


<br><br>
### In-Language Definitions

(none)