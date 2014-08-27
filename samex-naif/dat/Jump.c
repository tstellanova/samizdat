// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// Nonlocal jump (exit / yield) functions
//

#include "type/Jump.h"
#include "type/String.h"
#include "type/define.h"

#include "impl.h"


//
// Private Definitions
//

/**
 * Gets a pointer to the value's info.
 */
static JumpInfo *getInfo(zvalue jump) {
    return datPayload(jump);
}


//
// Module Definitions
//

// Documented in header.
zvalue jumpCall(zvalue jump, zint argCount, const zvalue *args) {
    JumpInfo *info = getInfo(jump);

    if (!info->valid) {
        die("Out-of-scope nonlocal jump.");
    }

    switch (argCount) {
        case 0:  { info->result = NULL;    break;      }
        case 1:  { info->result = args[0]; break;      }
        default: { die("Out-of-scope nonlocal jump."); }
    }

    info->valid = false;
    siglongjmp(info->env, 1);
}


//
// Exported Definitions
//

// Documented in header.
zvalue makeJump(void) {
    zvalue result = datAllocValue(CLS_Jump, sizeof(JumpInfo));
    JumpInfo *info = getInfo(result);

    info->valid = false;
    return result;
}


//
// Class Definition
//

// Documented in header.
METH_IMPL_rest(Jump, call, args) {
    return jumpCall(ths, argsSize, args);
}

// Documented in header.
METH_IMPL_0(Jump, debugString) {
    JumpInfo *info = getInfo(ths);
    zvalue validStr = info->valid ? EMPTY_STRING : stringFromUtf8(-1, "in");

    return METH_CALL(cat,
        stringFromUtf8(-1, "@(Jump "),
        validStr,
        stringFromUtf8(-1, "valid)"));
}

// Documented in header.
METH_IMPL_0(Jump, gcMark) {
    JumpInfo *info = getInfo(ths);

    datMark(info->result);
    return NULL;
}

/** Initializes the module. */
MOD_INIT(Jump) {
    MOD_USE(Value);

    CLS_Jump = makeCoreClass("Jump", CLS_Value,
        NULL,
        symbolTableFromArgs(
            METH_BIND(Jump, call),
            METH_BIND(Jump, debugString),
            METH_BIND(Jump, gcMark),
            NULL));
}

// Documented in header.
zvalue CLS_Jump = NULL;
