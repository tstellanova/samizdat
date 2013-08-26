/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"
#include "type/Callable.h"
#include "type/List.h"


/*
 * Exported Definitions
 */

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(listFilter) {
    zvalue function = args[0];
    zvalue list = args[1];
    zint size = collSize(list);
    zvalue result[size];
    zint at = 0;

    assertList(list);

    for (zint i = 0; i < size; i++) {
        zvalue elem = collNth(list, i);
        zvalue one = FUN_CALL(function, elem);

        if (one != NULL) {
            result[at] = one;
            at++;
        }
    }

    return listFromArray(at, result);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(makeList) {
    return listFromArray(argCount, args);
}