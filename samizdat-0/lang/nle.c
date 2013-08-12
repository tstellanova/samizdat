/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Nonlocal exit handling
 */

#include "impl.h"
#include "util.h"

#include <setjmp.h>
#include <stddef.h>


/*
 * Helper definitions
 */

/**
 * Nonlocal exit pending state. Instances of this structure are bound as
 * the closure state as part of call setup for closures that have a
 * `yieldDef`.
 */
typedef struct {
    /**
     * Whether this state is active, in that there is a valid nonlocal exit
     * that could land here.
     */
    bool active;

    /** Return value thrown here via nonlocal exit. `NULL` for void. */
    zvalue result;

    /** Jump buffer, used for nonlocal exit. */
    sigjmp_buf jumpBuf;

    /** Ordering id. */
    zint orderId;
} NonlocalExitInfo;

/**
 * Gets a pointer to the info of a nonlocal exit.
 */
static NonlocalExitInfo *nleInfo(zvalue nle) {
    return pbPayload(nle);
}

/**
 * Constructs and returns a nonlocal exit function.
 */
static zvalue newNonlocalExit(void) {
    zvalue result = pbAllocValue(TYPE_NonlocalExit, sizeof(NonlocalExitInfo));

    NonlocalExitInfo *info = nleInfo(result);
    info->active = true;
    info->result = NULL;
    info->orderId = pbOrderId();

    return result;
}


/*
 * Module functions
 */

/* Documented in header. */
zvalue nleCall(znleFunction function, void *state) {
    UTIL_TRACE_START(NULL, NULL);

    zstackPointer save = pbFrameStart();
    zvalue result;

    zvalue exitFunction = newNonlocalExit();
    NonlocalExitInfo *info = nleInfo(exitFunction);

    if (sigsetjmp(info->jumpBuf, 0) == 0) {
        // Here is where end up the first time `setjmp` returns.
        result = function(state, exitFunction);
    } else {
        // Here is where we land if and when `longjmp` is called.
        result = info->result;
        UTIL_TRACE_RESTART();
    }

    info->active = false;
    pbFrameReturn(save, result);
    UTIL_TRACE_END();

    return result;
}


/*
 * Type binding
 */

/* Documented in header. */
static zvalue NonlocalExit_call(zvalue state, zint argCount,
        const zvalue *args) {
    zvalue nle = args[0];
    NonlocalExitInfo *info = nleInfo(nle);

    if (!info->active) {
        die("Attempt to use out-of-scope nonlocal exit.");
    }

    switch (argCount) {
        case 1: {
            // Result is `NULL`. Nothing to do.
            break;
        }
        case 2: {
            // First argument is the nonlocal exit function itself. Second
            // argument, when present, is the value to yield.
            info->result = args[1];
            break;
        }
        default: {
            die("Attempt to yield multiple values from nonlocal exit.");
        }
    }

    siglongjmp(info->jumpBuf, 1);
}

/* Documented in header. */
static zvalue NonlocalExit_canCall(zvalue state, zint argCount,
        const zvalue *args) {
    zvalue nle = args[0];
    zvalue value = args[1];

    // Okay to call this with any first argument.
    return value;
}

/* Documented in header. */
static zvalue NonlocalExit_gcMark(zvalue state, zint argCount,
        const zvalue *args) {
    zvalue nle = args[0];

    pbMark(nleInfo(nle)->result);
    return NULL;
}

/* Documented in header. */
static zvalue NonlocalExit_order(zvalue state, zint argCount,
        const zvalue *args) {
    zvalue v1 = args[0];
    zvalue v2 = args[1];

    return (nleInfo(v1)->orderId < nleInfo(v2)->orderId) ? PB_NEG1 : PB_1;
}

/* Documented in header. */
void langBindNonlocalExit(void) {
    TYPE_NonlocalExit = coreTypeFromName(stringFromUtf8(-1, "NonlocalExit"));
    gfnBindCore(GFN_call,    TYPE_NonlocalExit, NonlocalExit_call);
    gfnBindCore(GFN_canCall, TYPE_NonlocalExit, NonlocalExit_canCall);
    gfnBindCore(GFN_gcMark,  TYPE_NonlocalExit, NonlocalExit_gcMark);
    gfnBindCore(GFN_order,   TYPE_NonlocalExit, NonlocalExit_order);
}

/* Documented in header. */
zvalue TYPE_NonlocalExit = NULL;
