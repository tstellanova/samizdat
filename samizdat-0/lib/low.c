/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "const.h"
#include "impl.h"
#include "util.h"

#include <stddef.h>


/*
 * Helper functions
 */

/**
 * Does most of the work of `lowOrderIs`.
 */
static bool doLowOrderIs(zint argCount, const zvalue *args) {
    zorder want = datZintFromInt(args[2]);

    if ((argCount == 3) && (want == ZSAME)) {
        return datEq(args[0], args[1]);
    }

    zorder comp = datOrder(args[0], args[1]);

    return (comp == want) ||
        ((argCount == 4) && (comp == datZintFromInt(args[3])));
}


/*
 * Exported functions
 */

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(lowOrder) {
    requireExactly(argCount, 2);
    return constIntFromZint(datOrder(args[0], args[1]));
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(lowOrderIs) {
    requireRange(argCount, 3, 4);
    return doLowOrderIs(argCount, args) ? args[1] : NULL;
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(lowSize) {
    requireExactly(argCount, 1);
    return constIntFromZint(datSize(args[0]));
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(lowType) {
    requireExactly(argCount, 1);
    return constLowTypeName(args[0]);
}
