// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// `Collection` protocol
//
// **Note:** There is no in-model value `Collection`. There are a few
// different subsets of collection-like functionality which may eventually
// be represented as protocols or similar.
//
// **Note:** Because `collection` per se is an awkward prefix, instead the
// suggestive prefix `coll` is used.
//

#ifndef _TYPE_COLLECTION_H_
#define _TYPE_COLLECTION_H_

#include "type/Generator.h"  // Because all collections are generators.
#include "type/Generic.h"
#include "type/OneOff.h"


/** Generic `del(collection, key)`: Documented in spec. */
extern zvalue GFN_del;

/** Generic `nthMapping(collection, n)`: Documented in spec. */
extern zvalue GFN_nthMapping;

/** Generic `keyList(collection)`: Documented in spec. */
extern zvalue GFN_keyList;

/** Generic `put(collection, key, value)`: Documented in spec. */
extern zvalue GFN_put;

/** Generic `valueList(collection)`: Documented in spec. */
extern zvalue GFN_valueList;

/**
 * Calls the `del` generic.
 */
zvalue collDel(zvalue coll, zvalue key);

/**
 * Calls the `put` generic.
 */
zvalue collPut(zvalue coll, zvalue key, zvalue value);

#endif
