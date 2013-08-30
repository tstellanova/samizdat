/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * One-Off generics
 */

#ifndef _TYPE_ONE_OFF_H_
#define _TYPE_ONE_OFF_H_

#include "pb.h"


/** Generic `nameOf(value)`: Documented in spec. */
extern zvalue GFN_nameOf;

/** Generic `toInt(value)`: Documented in spec. */
extern zvalue GFN_toInt;

/** Generic `toNumber(value)`: Documented in spec. */
extern zvalue GFN_toNumber;

/** Generic `toString(value)`: Documented in spec. */
extern zvalue GFN_toString;

#endif