// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

#include "type/Function.h"
#include "type/Generator.h"
#include "type/List.h"
#include "util.h"

#include "impl.h"


/*
 * Exported Definitions
 */

/* Documented in spec. */
FUN_IMPL_DECL(interpolate) {
    zvalue result = GFN_CALL(collect, args[0]);

    switch (get_size(result)) {
        case 0: return NULL;
        case 1: return nth(result, 0);
        default: {
            die("Attempt to interpolate multiple values.");
        }
    }
}

/* Documented in spec. */
FUN_IMPL_DECL(maybeValue) {
    zvalue function = args[0];
    zvalue value = FUN_CALL(function);

    return (value == NULL) ? EMPTY_LIST : listFromArray(1, &value);
}
