/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Private implementation details
 */

#ifndef _IMPL_H_
#define _IMPL_H_

#include "dat.h"
#include "util.h"


/**
 * Flag indicating whether module has been initialized.
 */
extern bool datInitialized;

/**
 * Generic `call(value)`: Somewhat-degenerate generic for dispatching to
 * a function call mechanism (how meta). Only defined for types `Function`
 * and `Generic`. When called, argument count and pointer will have been
 * checked, but the argument count may not match what's expected by the
 * target function. The `state` argument is always passed as the function
 * or generic value itself.
 */
extern zvalue GFN_call;

/**
 * Generic `debugString(value)`: Returns a minimal string form of the
 * given value. Notably, functions and generics include their names.
 * The default implementation returns strings of the form
 * `#(TypeName @ address)`.
 */
extern zvalue GFN_debugString;

/**
 * Generic `gcMark(value)`: Does GC marking for the given value.
 */
extern zvalue GFN_gcMark;

/**
 * Generic `gcFree(value)`: Does GC freeing for the given value. This is
 * to do immediate pre-mortem freeing of value contents.
 */
extern zvalue GFN_gcFree;

/**
 * Clears the contents of the map lookup cache.
 */
void datMapClearCache(void);


/*
 * Initialization functions
 */

// Per-type generic binding.
void datBindBox(void);
void datBindDeriv(void);
void datBindList(void);
void datBindMap(void);
void datBindUniqlet(void);

#endif
