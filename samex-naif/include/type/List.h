// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// `List` class
//

#ifndef _TYPE_LIST_H_
#define _TYPE_LIST_H_

#include "type/Core.h"
#include "type/Sequence.h"


/** Class value for in-model class `List`. */
extern zvalue CLS_List;

/** The standard value `[]`. */
extern zvalue EMPTY_LIST;

/**
 * Constructs a list of size 1 from a single given `value`.
 */
zvalue listFromValue(zvalue value);

/**
 * Constructs a list from a `zarray` (sized array of `zvalue`).
 */
zvalue listFromZarray(zarray arr);

/**
 * Gets a `zarray` of the given list. The result `elems` shares storage
 * with the `list`. As such, it is important to *not* modify the contents.
 */
zarray zarrayFromList(zvalue list);

#endif
