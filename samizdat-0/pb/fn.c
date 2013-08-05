/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * In-model Functions
 */

#include "impl.h"

#include <stdint.h>
#include <string.h>


/*
 * Helper definitions
 */

/**
 * Regular (non-generic) function structure.
 */
typedef struct {
    /** Minimum argument count. Always `>= 0`. */
    zint minArgs;

    /**
     * Maximum argument count. Always `>= minArgs`.
     */
    zint maxArgs;

    /** C function to call. */
    zfunction function;

    /** Closure state. */
    zvalue state;

    /** The function's name, if any. Used when producing stack traces. */
    zvalue name;

    /** Id to use for ordering comparisons. */
    zint orderId;
} DatFunction;

/**
 * Gets a pointer to the value's info.
 */
static DatFunction *fnInfo(zvalue function) {
    return pbPayload(function);
}


/*
 * Exported functions
 */

/* Documented in header. */
zvalue fnFrom(zint minArgs, zint maxArgs, zfunction function, zvalue state,
        zvalue name) {
    if ((minArgs < 0) ||
        ((maxArgs != -1) && (maxArgs < minArgs))) {
        die("Invalid `minArgs` / `maxArgs`: %lld, %lld", minArgs, maxArgs);
    }

    zvalue result = pbAllocValue(DAT_Function, sizeof(DatFunction));
    DatFunction *info = fnInfo(result);

    info->minArgs = minArgs;
    info->maxArgs = (maxArgs != -1) ? maxArgs : INT64_MAX;
    info->function = function;
    info->state = state;
    info->name = name;
    info->orderId = pbOrderId();

    return result;
}


/*
 * Type binding
 */

/* Documented in header. */
static zvalue Function_call(zvalue function,
        zint argCount, const zvalue *args) {
    DatFunction *info = fnInfo(function);

    if (argCount < info->minArgs) {
        die("Too few arguments for function call: %lld, min %lld",
            argCount, info->minArgs);
    } else if (argCount > info->maxArgs) {
        die("Too many arguments for function call: %lld, max %lld",
            argCount, info->maxArgs);
    }

    return info->function(info->state, argCount, args);
}

/* Documented in header. */
static zvalue Function_debugString(zvalue state,
        zint argCount, const zvalue *args) {
    zvalue function = args[0];
    DatFunction *info = fnInfo(function);

    zvalue result = stringFromUtf8(-1, "@(Function ");

    if (info->name != NULL) {
        result = stringAdd(result, fnCall(GFN_debugString, 1, &info->name));
    } else {
        result = stringAdd(result, stringFromUtf8(-1, "(unknown)"));
    }

    result = stringAdd(result, stringFromUtf8(-1, ")"));
    return result;
}

/* Documented in header. */
static zvalue Function_gcMark(zvalue state, zint argCount, const zvalue *args) {
    zvalue function = args[0];
    DatFunction *info = fnInfo(function);

    pbMark(info->state);
    pbMark(info->name);

    return NULL;
}

/* Documented in header. */
static zvalue Function_order(zvalue state, zint argCount, const zvalue *args) {
    zvalue v1 = args[0];
    zvalue v2 = args[1];
    return (fnInfo(v1)->orderId < fnInfo(v2)->orderId) ? DAT_NEG1 : DAT_1;
}

/* Documented in header. */
void pbBindFunction(void) {
    gfnBindCore(GFN_call,        DAT_Function, Function_call);
    gfnBindCore(GFN_debugString, DAT_Function, Function_debugString);
    gfnBindCore(GFN_gcMark,      DAT_Function, Function_gcMark);
    gfnBindCore(GFN_order,       DAT_Function, Function_order);
}

/* Documented in header. */
static PbType INFO_Function = {
    .name = "Function"
};
ztype DAT_Function = &INFO_Function;
