Samizdat Layer 0: Core Library
==============================

Constants
---------

The definitions in this section are all simple constant values, not
functions.

**Note:** This section only covers constants that are not described
elsewhere in the library documentation.

<br><br>
### Generic Function Definitions

(none)


<br><br>
### Primitive Definitions

#### `ENVIRONMENT`

A map of strings to strings, representing the (Posix or Posix-like)
environment variables passed into the program.

#### `Int`

The type of int values.

#### `List`

The type of list values.

#### `Map`

The type of map values.

#### `String`

The type of string values.

#### `Type`

The type of opaque type values.

**Note:** The type of a transparent derived type is a simple non-type
value.

#### `Uniqlet`

The type of uniqlet values.

#### `Value`

The common type of all values.


<br><br>
### In-Language Definitions

#### `null`

A value used when no other value is suitable, but when a value is
nonetheless required. It is defined as `@Null`, that is, a type-only
value with type tag `"Null"`.

#### `ENTITY_MAP`

Map of entity names to their string values. This is a map from strings to
strings, where the keys are XML entity names (such as `"lt" "gt" "zigrarr"`)
and the corresponding values are the strings represented by those entity
names (such as, correspondingly, `"<" ">" "⇝"`).

The set of entities is intended to track the XML spec for same, which
as of this writing can be found at
<http://www.w3.org/TR/xml-entity-names/bycodes.html>.

**Syntax Note:** Used during the parsing of string literal forms.