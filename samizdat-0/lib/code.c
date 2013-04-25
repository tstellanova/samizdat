/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"
#include "util.h"

#include <stddef.h>
#include <string.h>


/*
 * Helper definitions
 */

/**
 * Object, that is, a function and its associated mutable state. Instances
 * of this structure are bound as the closure state as part of
 * function registration in the implementation of the primitive `object`.
 */
typedef struct {
    /** Arbitrary state value. */
    zvalue state;

    /** In-model function value. */
    zvalue function;
} Object;

/** The stringlet @"result", lazily initialized. */
static zvalue STR_RESULT = NULL;

/** The stringlet @"state", lazily initialized. */
static zvalue STR_STATE = NULL;

/**
 * Initializes the stringlet constants, if necessary.
 */
static void initCodeConsts(void) {
    if (STR_RESULT == NULL) {
        STR_RESULT = datStringletFromUtf8String(-1, "result");
        STR_STATE  = datStringletFromUtf8String(-1, "state");
    }
}

/**
 * The C function that is bound by the `object` primitive.
 */
static zvalue callObject(void *state, zint argCount, const zvalue *args) {
    Object *object = state;
    zvalue fullArgs[argCount + 1];

    fullArgs[0] = object->state;
    memcpy(fullArgs + 1, args, argCount);

    zvalue resultMaplet = langCall(object->function, argCount + 1, fullArgs);

    if (resultMaplet == NULL) {
        return NULL;
    }

    zvalue result = datMapletGet(resultMaplet, STR_RESULT);
    zvalue newState = datMapletGet(resultMaplet, STR_STATE);

    if (newState != NULL) {
        object->state = newState;
    }

    return result;
}


/*
 * Exported primitives
 */

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(apply) {
    requireAtLeast(argCount, 1);

    zvalue function = args[0];

    switch (argCount) {
        case 1: {
            // Zero-argument call.
            return langCall(function, 0, NULL);
        }
        case 2: {
            // Just a "rest" listlet.
            return langApply(function, args[1]);
        }
    }

    // The hard case: We make a flattened array of all the initial arguments
    // followed by the contents of the "rest" listlet.

    zvalue rest = args[argCount - 1];
    zint restSize = datSize(rest);

    args++;
    argCount -= 2;

    zint flatSize = argCount + restSize;
    zvalue flatArgs[flatSize];

    for (zint i = 0; i < argCount; i++) {
        flatArgs[i] = args[i];
    }

    datArrayFromListlet(flatArgs + argCount, rest);
    return langCall(function, flatSize, flatArgs);
}

/* TODO: Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(object) {
    initCodeConsts();

    requireExactly(argCount, 2);

    Object *object = zalloc(sizeof(Object));
    object->state = args[0];
    object->function = args[1];

    return langDefineFunction(callObject, object);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(sam0Tree) {
    requireExactly(argCount, 1);
    return langNodeFromProgramText(args[0]);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(sam0Eval) {
    requireExactly(argCount, 2);

    zvalue contextMaplet = args[0];
    zvalue expressionNode = args[1];
    zcontext ctx = langCtxNew();

    langCtxBindAll(ctx, contextMaplet);
    return langEvalExpressionNode(ctx, expressionNode);
}
