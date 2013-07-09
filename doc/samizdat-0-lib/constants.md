Samizdat Layer 0: Core Library
==============================

Constants
---------

The definitions in this section are all simple constant values, not
functions.

<br><br>
### Primitive Definitions

#### `false`

The boolean value false.

**Note:** Technically, this value could be defined in-language as
`false = @["boolean": 0]`. However, as a practical matter the
lowest layer of implementation needs to refer to this value, so
it makes sense to allow it to be exported as a primitive.

#### `true`

The boolean value true.

**Note:** Technically, this value could be defined in-language as
`true = @["boolean": 1]`. However, as a practical matter the
lowest layer of implementation needs to refer to this value, so
it makes sense to allow it to be exported as a primitive.


<br><br>
### In-Language Definitions

#### `null`

A value used when no other value is suitable, but when a value is
nonetheless required. It is defined as `@null`, that is, a valueless
token with type tag `"null"`.

#### `ENTITY_MAP`

Map of entity names to their string values. This is a map from strings to
strings, where the keys are XML entity names (such as `"lt" "gt" "zigrarr"`)
and the corresponding values are the strings represented by those entity
names (such as, correspondingly, `"<" ">" "⇝"`).

The set of entities is intended to track the XML spec for same, which
as of this writing can be found at
<http://www.w3.org/TR/xml-entity-names/bycodes.html>.