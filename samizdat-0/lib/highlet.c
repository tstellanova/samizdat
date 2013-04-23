/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"


/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(highletType) {
    requireExactly(argCount, 1);
    return datHighletType(args[0]);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(highletValue) {
    requireExactly(argCount, 1);
    return datHighletValue(args[0]);
}
