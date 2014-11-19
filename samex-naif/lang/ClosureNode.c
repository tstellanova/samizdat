// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// `ClosureNode` class
//
// Translation of the main info of a `closure` node.

#include "const.h"
#include "type/define.h"
#include "type/Box.h"
#include "type/Jump.h"
#include "type/List.h"
#include "type/SymbolTable.h"

#include "impl.h"


//
// Private Definitions
//

/**
 * Repetition style of a formal argument.
 */
typedef enum {
    REP_NONE,
    REP_QMARK,
    REP_STAR,
    REP_PLUS
} zrepeat;

/**
 * Formal argument.
 */
typedef struct {
    /** Name (optional). */
    zvalue name;

    /** Repetition style. */
    zrepeat repeat;
} zformal;

/**
 * Payload data. This corresponds with the payload of a `closure` node, but
 * with `formals` reworked to be easier to digest.
 */
typedef struct {
    /** The `node::formals`, converted for easier use. */
    zformal formals[LANG_MAX_FORMALS];

    /** The result of `get_size(formals)`. */
    zint formalsSize;

    /** The number of actual names in `formals`, plus one for a `yieldDef`. */
    zint formalsNameCount;

    /** `node::name`. */
    zvalue name;

    /** `node::statements`. */
    zvalue statements;

    /** `node::yield`. */
    zvalue yield;

    /** `node::yieldDef`. */
    zvalue yieldDef;
} ClosureNodeInfo;

/**
 * Gets the info of a record.
 */
static ClosureNodeInfo *getInfo(zvalue value) {
    return (ClosureNodeInfo *) datPayload(value);
}

/**
 * Converts the given `formals`, storing the result in the given `info`.
 */
static void convertFormals(ClosureNodeInfo *info, zvalue formalsList) {
    zarray formals = zarrayFromList(formalsList);

    if (formals.size > LANG_MAX_FORMALS) {
        die("Too many formals: %lld", formals.size);
    }

    zvalue names[formals.size + 1];  // For detecting duplicates.
    zint nameCount = 0;

    if (info->yieldDef != NULL) {
        names[0] = info->yieldDef;
        nameCount = 1;
    }

    for (zint i = 0; i < formals.size; i++) {
        zvalue formal = formals.elems[i];
        zvalue name, repeat;
        zrepeat rep;

        symtabGet2(formal, SYM(name), &name, SYM(repeat), &repeat);

        if (name != NULL) {
            names[nameCount] = name;
            nameCount++;
        }

        if (repeat == NULL) {
            rep = REP_NONE;
        } else {
            switch (symbolEvalType(repeat)) {
                case EVAL_CH_STAR:  { rep = REP_STAR;  break; }
                case EVAL_CH_PLUS:  { rep = REP_PLUS;  break; }
                case EVAL_CH_QMARK: { rep = REP_QMARK; break; }
                default: {
                    die("Invalid repeat modifier: %s", cm_debugString(repeat));
                }
            }
        }

        info->formals[i] = (zformal) {.name = name, .repeat = rep};
    }

    // Detect duplicate formal argument names.

    if (nameCount > 1) {
        symbolSort(nameCount, names);
        for (zint i = 1; i < nameCount; i++) {
            if (names[i - 1] == names[i]) {
                die("Duplicate formal name: %s", cm_debugString(names[i]));
            }
        }
    }

    // All's well. Finish up.

    info->formalsSize = formals.size;
    info->formalsNameCount = nameCount;
}

/**
 * Creates a variable map for all the formal arguments of the given
 * function.
 */
static zvalue bindArguments(ClosureNodeInfo *info, zvalue exitFunction,
        zarray args) {
    zmapping elems[info->formalsNameCount];
    zformal *formals = info->formals;
    zint formalsSize = info->formalsSize;
    zint elemAt = 0;
    zint argAt = 0;

    for (zint i = 0; i < formalsSize; i++) {
        zvalue name = formals[i].name;
        zrepeat repeat = formals[i].repeat;
        bool ignore = (name == NULL);
        zvalue value;

        if (repeat != REP_NONE) {
            zint count;

            switch (repeat) {
                case REP_STAR: {
                    count = args.size - argAt;
                    break;
                }
                case REP_PLUS: {
                    if (argAt >= args.size) {
                        die("Function called with too few arguments "
                            "(plus argument): %lld",
                            args.size);
                    }
                    count = args.size - argAt;
                    break;
                }
                case REP_QMARK: {
                    count = (argAt >= args.size) ? 0 : 1;
                    break;
                }
                default: {
                    die("Invalid repeat enum (shouldn't happen).");
                }
            }

            value = ignore
                ? NULL
                : listFromZarray((zarray) {count, &args.elems[argAt]});
            argAt += count;
        } else if (argAt >= args.size) {
            die("Function called with too few arguments: %lld", args.size);
        } else {
            value = args.elems[argAt];
            argAt++;
        }

        if (!ignore) {
            elems[elemAt].key = name;
            elems[elemAt].value = cm_new(Result, value);
            elemAt++;
        }
    }

    if (argAt != args.size) {
        die("Function called with too many arguments: %lld > %lld",
            args.size, argAt);
    }

    if (exitFunction != NULL) {
        elems[elemAt].key = info->yieldDef;
        elems[elemAt].value = cm_new(Result, exitFunction);
        elemAt++;
    }

    return symtabFromZassoc((zassoc) {elemAt, elems});
}

/**
 * Helper that does the main work of `exnoCallClosure`, including nonlocal
 * exit binding when appropriate.
 */
static zvalue callClosureMain(zvalue node, Frame *parentFrame,
        zvalue exitFunction, zarray args) {
    ClosureNodeInfo *info = getInfo(node);

    // With the closure's frame as the parent, bind the formals and
    // nonlocal exit (if present), creating a new execution frame.

    Frame frame;
    zvalue argTable = bindArguments(info, exitFunction, args);
    frameInit(&frame, &parentFrame, /*FIXME*/ NULL, argTable);

    // Execute the statements, updating the frame as needed.
    exnoExecuteStatements(info->statements, &frame);

    // Execute the yield expression, and return the final result.
    return exnoExecute(info->yield, &frame);
}


//
// Module Definitions
//

// Documented in header.
zvalue exnoBuildClosure(zvalue node, Frame *frame) {
    // TODO! FIXME!
    die("TODO");
}

// Documented in header.
zvalue exnoCallClosure(zvalue node, Frame *frame, zarray args) {
    if (getInfo(node)->yieldDef == NULL) {
        return callClosureMain(node, frame, NULL, args);
    }

    zvalue jump = makeJump();
    jumpArm(jump);

    zvalue result = callClosureMain(node, frame, jump, args);
    jumpRetire(jump);

    return result;
}


//
// Class Definition
//

/**
 * Constructs an instance from the given (per spec) `closure` tree node.
 */
CMETH_IMPL_1(ClosureNode, new, orig) {
    zvalue result = datAllocValue(CLS_ClosureNode, sizeof(ClosureNodeInfo));
    ClosureNodeInfo *info = getInfo(result);
    zvalue formals;

    if (!recGet3(orig,
            SYM(formals),    &formals,
            SYM(statements), &info->statements,
            SYM(yield),      &info->yield)) {
        die("Invalid `closure` node.");
    }

    // These are both optional.
    recGet2(orig,
        SYM(name),     &info->name,
        SYM(yieldDef), &info->yieldDef);

    exnoConvert(&info->statements);
    exnoConvert(&info->yield);
    convertFormals(info, formals);

    return result;
}

// Documented in spec.
METH_IMPL_0(ClosureNode, debugSymbol) {
    return getInfo(ths)->name;
}

// Documented in header.
METH_IMPL_0(ClosureNode, gcMark) {
    ClosureNodeInfo *info = getInfo(ths);

    datMark(info->name);
    datMark(info->statements);
    datMark(info->yield);
    datMark(info->yieldDef);

    for (zint i = 0; i < info->formalsSize; i++) {
        datMark(info->formals[i].name);
    }

    return NULL;
}

/** Initializes the module. */
MOD_INIT(ClosureNode) {
    MOD_USE(cls);

    CLS_ClosureNode = makeCoreClass(
        symbolFromUtf8(-1, "ClosureNode"), CLS_Core,
        METH_TABLE(
            CMETH_BIND(ClosureNode, new)),
        METH_TABLE(
            METH_BIND(ClosureNode, debugSymbol),
            METH_BIND(ClosureNode, gcMark)));
}

// Documented in header.
zvalue CLS_ClosureNode = NULL;
