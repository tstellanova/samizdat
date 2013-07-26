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
 * Does the type assertions that are part of `coreOrder` and `coreOrderIs`.
 */
static void coreOrderTypeCheck(zvalue v1, zvalue v2) {
    ztype ztype1 = datType(v1);
    ztype ztype2 = datType(v2);

    if (ztype1 != ztype2) {
        die("Mismatched core types.");
    }

    if (ztype1 == DAT_DERIV) {
        zvalue type1 = constTypeOf(v1);
        zvalue type2 = constTypeOf(v2);

        if (!datEq(type1, type2)) {
            die("Mismatched derived types.");
        } else if (constIsCoreTypeName(type1)) {
            die("Oddball derived value.");
        }
    }
}

/**
 * Does most of the work of `coreOrderIs` and `totalOrderIs`.
 */
static bool doOrderIs(zint argCount, const zvalue *args) {
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
PRIM_IMPL(coreOrder) {
    requireExactly(argCount, 2);

    zvalue arg0 = args[0];
    zvalue arg1 = args[1];

    coreOrderTypeCheck(arg0, arg1);
    return constIntFromZint(datOrder(arg0, arg1));
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(coreOrderIs) {
    requireRange(argCount, 3, 4);

    zvalue arg0 = args[0];
    zvalue arg1 = args[1];

    coreOrderTypeCheck(arg0, arg1);
    return doOrderIs(argCount, args) ? arg1 : NULL;
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(coreSizeOf) {
    requireExactly(argCount, 1);
    return constIntFromZint(datSize(args[0]));
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(dataOf) {
    requireExactly(argCount, 1);
    return constDataOf(args[0]);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(isCoreValue) {
    requireExactly(argCount, 1);

    zvalue value = args[0];
    return (datType(value) != DAT_DERIV) ? value : NULL;
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(totalOrder) {
    requireExactly(argCount, 2);
    return constIntFromZint(datOrder(args[0], args[1]));
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(totalOrderIs) {
    requireRange(argCount, 3, 4);
    return doOrderIs(argCount, args) ? args[1] : NULL;
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(typeOf) {
    requireExactly(argCount, 1);
    return constTypeOf(args[0]);
}
