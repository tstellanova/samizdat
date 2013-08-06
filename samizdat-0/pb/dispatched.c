/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"

#include <stddef.h>
#include <stdio.h>
#include <string.h>


/*
 * Module definitions
 */

/* Documented in header. */
zvalue GFN_call = NULL;

/* Documented in header. */
zvalue GFN_dataOf = NULL;

/* Documented in header. */
zvalue GFN_debugString = NULL;

/* Documented in header. */
zvalue GFN_eq = NULL;

/* Documented in header. */
zvalue GFN_gcFree = NULL;

/* Documented in header. */
zvalue GFN_gcMark = NULL;

/* Documented in header. */
zvalue GFN_order = NULL;

/* Documented in header. */
zvalue GFN_sizeOf = NULL;

/* Documented in header. */
static zvalue Default_dataOf(zvalue state, zint argCount, const zvalue *args) {
    return args[0];
}

/* Documented in header. */
static zvalue Default_debugString(zvalue state,
        zint argCount, const zvalue *args) {
    char addrBuf[11]; // Includes room for "0x" and "\0".

    sprintf(addrBuf, "%p", args[0]);

    zvalue result = stringFromUtf8(-1, "@(");
    result = stringAdd(result, fnCall(GFN_debugString, 1, &args[0]->type));
    result = stringAdd(result, stringFromUtf8(-1, " @ "));
    result = stringAdd(result, stringFromUtf8(-1, addrBuf));
    result = stringAdd(result, stringFromUtf8(-1, ")"));
    return result;
}

/* Documented in header. */
static zvalue Default_eq(zvalue state, zint argCount, const zvalue *args) {
    return NULL;
}

/* Documented in header. */
static zvalue Default_sizeOf(zvalue state, zint argCount, const zvalue *args) {
    return intFromZint(0);
}

/* Documented in header. */
void pbInitCoreGenerics(void) {
    GFN_call = gfnFrom(1, 1, stringFromUtf8(-1, "call"));
    pbImmortalize(GFN_call);

    GFN_dataOf = gfnFrom(1, 1, stringFromUtf8(-1, "dataOf"));
    gfnBindCoreDefault(GFN_dataOf, Default_dataOf);
    pbImmortalize(GFN_dataOf);

    GFN_debugString = gfnFrom(1, 1, stringFromUtf8(-1, "debugString"));
    gfnBindCoreDefault(GFN_debugString, Default_debugString);
    pbImmortalize(GFN_debugString);

    GFN_eq = gfnFrom(2, 2, stringFromUtf8(-1, "eq"));
    gfnBindCoreDefault(GFN_eq, Default_eq);
    pbImmortalize(GFN_eq);

    GFN_gcFree = gfnFrom(1, 1, stringFromUtf8(-1, "gcFree"));
    pbImmortalize(GFN_gcFree);

    GFN_gcMark = gfnFrom(1, 1, stringFromUtf8(-1, "gcMark"));
    pbImmortalize(GFN_gcMark);

    GFN_order = gfnFrom(2, 2, stringFromUtf8(-1, "order"));
    pbImmortalize(GFN_order);

    GFN_sizeOf = gfnFrom(1, 1, stringFromUtf8(-1, "sizeOf"));
    gfnBindCoreDefault(GFN_sizeOf, Default_sizeOf);
    pbImmortalize(GFN_sizeOf);
}


/*
 * Exported functions
 */

/* Documented in header. */
zvalue pbDataOf(zvalue value) {
    return fnCall(GFN_dataOf, 1, &value);
}

/* Documented in header. */
char *pbDebugString(zvalue value) {
    if (value == NULL) {
        return strdup("(null)");
    }

    zvalue result = fnCall(GFN_debugString, 1, &value);
    zint size = utf8SizeFromString(result);
    char str[size + 1];

    utf8FromString(size + 1, str, result);
    return strdup(str);
}

/* Documented in header. */
bool pbEq(zvalue v1, zvalue v2) {
    pbAssertValid(v1);
    pbAssertValid(v2);

    if (v1 == v2) {
        return true;
    } else if (v1->type != v2->type) {
        return false;
    } else {
        zvalue args[2] = { v1, v2 };
        return fnCall(GFN_eq, 2, args) != NULL;
    }
}

/* Documented in header. */
bool pbNullSafeEq(zvalue v1, zvalue v2) {
    pbAssertValidOrNull(v1);
    pbAssertValidOrNull(v2);

    if (v1 == v2) {
        return true;
    } else if ((v1 == NULL) || (v2 == NULL)) {
        return false;
    } else if (v1->type != v2->type) {
        return false;
    } else {
        zvalue args[2] = { v1, v2 };
        return fnCall(GFN_eq, 2, args) != NULL;
    }
}

/* Documented in header. */
zorder pbOrder(zvalue v1, zvalue v2) {
    pbAssertValid(v1);
    pbAssertValid(v2);

    if (v1 == v2) {
        return ZSAME;
    } else if (v1->type == v2->type) {
        zvalue args[2] = { v1, v2 };
        zstackPointer save = pbFrameStart();
        zorder result = zintFromInt(fnCall(GFN_order, 2, args));
        pbFrameReturn(save, NULL);
        return result;
    } else {
        return pbOrder(v1->type, v2->type);
    }
}

/* Documented in header. */
zint pbSize(zvalue value) {
    return zintFromInt(fnCall(GFN_sizeOf, 1, &value));
}
