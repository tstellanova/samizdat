/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"

#include <stdio.h>


/*
 * Type binding
 */

/* Documented in header. */
static zvalue Value_debugString(zint argCount, const zvalue *args) {
    zvalue value = args[0];
    zvalue type = typeOf(value);
    char addrBuf[19]; // Includes room for "0x" and "\0".

    sprintf(addrBuf, "%p", value);

    zvalue result = stringFromUtf8(-1, "@(");
    result = stringAdd(result, fnCall(GFN_debugString, 1, &type));
    result = stringAdd(result, stringFromUtf8(-1, " @ "));
    result = stringAdd(result, stringFromUtf8(-1, addrBuf));
    result = stringAdd(result, stringFromUtf8(-1, ")"));
    return result;
}

/* Documented in header. */
static zvalue Value_eq(zint argCount, const zvalue *args) {
    return NULL;
}

/* Documented in header. */
static zvalue Value_size(zint argCount, const zvalue *args) {
    return intFromZint(0);
}

/* Documented in header. */
void pbBindValue(void) {
    // Note: The type `Type` is responsible for initializing `TYPE_Value`.

    gfnBindCore(GFN_debugString, TYPE_Value, Value_debugString);
    gfnBindCore(GFN_eq,          TYPE_Value, Value_eq);
    gfnBindCore(GFN_size,        TYPE_Value, Value_size);
}

/* Documented in header. */
zvalue TYPE_Value = NULL;
