/*
 * Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * One-Off generics
 */

#ifndef _TYPE_ONE_OFF_H_
#define _TYPE_ONE_OFF_H_

#include "dat.h"

/** Generic `cat(value, more*)`: Documented in spec. */
extern zvalue GFN_cat;

/** Generic `get(value, key)`: Documented in spec. */
extern zvalue GFN_get;

/** Generic `get_key(value)`: Documented in spec. */
extern zvalue GFN_get_key;

/** Generic `get_name(value)`: Documented in spec. */
extern zvalue GFN_get_name;

/** Generic `nth(sequence, n)`: Documented in spec. */
extern zvalue GFN_nth;

/** Generic `get_size(collection)`: Documented in spec. */
extern zvalue GFN_get_size;

/** Generic `toInt(value)`: Documented in spec. */
extern zvalue GFN_toInt;

/** Generic `toNumber(value)`: Documented in spec. */
extern zvalue GFN_toNumber;

/** Generic `toString(value)`: Documented in spec. */
extern zvalue GFN_toString;

/** Generic `get_value(value)`: Documented in spec. */
extern zvalue GFN_get_value;

/**
 * Calls the `get` generic.
 */
zvalue get(zvalue coll, zvalue key);

/**
 * Calls `get_name` on the given value.
 */
zvalue get_name(zvalue value);

/**
 * Calls `get_name` on the given value, if it is defined. If not, returns `NULL`.
 */
zvalue get_nameIfDefined(zvalue value);

/**
 * Calls `nth`, converting the given `zint` index to an `Int` value.
 */
zvalue nth(zvalue value, zint index);

/**
 * Calls `nth`, converting the given `zint` index to an `Int` value, and
 * converting a non-void return value &mdash; which must be a single-character
 * `String` &mdash; to a `zint` in the range of a `zchar`. A void return
 * value gets converted to `-1`.
 */
zint nthChar(zvalue value, zint index);

/**
 * Calls `get_size` on the given value, converting the result to a `zint`.
 */
zint get_size(zvalue value);

#endif
