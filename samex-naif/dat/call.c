// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// Function / method calling
//

#include <stdarg.h>
#include <stdio.h>   // For `asprintf`.

#include "type/Builtin.h"
#include "type/Jump.h"
#include "type/List.h"
#include "type/String.h"
#include "type/Symbol.h"
#include "type/define.h"
#include "util.h"

#include "impl.h"


//
// Private Definitions
//

/**
 * Struct used to hold the salient info for generating a stack trace.
 */
typedef struct {
    /** Target being called. */
    zvalue target;

    /** Name of method being called. */
    zvalue name;
} StackTraceEntry;

/**
 * Returns a `dup()`ed string representing `value`. The result is the chars
 * of `value` if it is a string or symbol. Otherwise, it is the result of
 * calling `.debugString()` on `value`.
 */
static char *ensureString(zvalue value) {
    if (classAccepts(CLS_String, value)) {
        // No conversion.
    } else if (classAccepts(CLS_Symbol, value)) {
        value = cm_castFrom(CLS_String, value);
    } else {
        value = METH_CALL_old(debugString, value);
    }

    return utf8DupFromString(value);
}

/**
 * This is the function that handles emitting a context string for a method
 * call, when dumping the stack.
 */
static char *callReporter(void *state) {
    StackTraceEntry *ste = state;
    char *classStr =
        ensureString(METH_CALL_old(debugSymbol, classOf(ste->target)));
    char *result;

    if (symbolEq(ste->name, SYM(call))) {
        // It's a function call (or function-like call).
        zvalue targetName = METH_CALL_old(debugSymbol, ste->target);

        if (targetName != NULL) {
            char *nameStr = ensureString(targetName);
            asprintf(&result, "%s (instance of %s)",
                nameStr, classStr);
            utilFree(nameStr);
        } else {
            asprintf(&result, "anonymous instance of %s", classStr);
        }
    } else {
        char *targetStr = cm_debugString(ste->target);
        char *nameStr = ensureString(ste->name);
        asprintf(&result, "%s.%s on %s", classStr, nameStr, targetStr);
        utilFree(targetStr);
        utilFree(nameStr);
    }

    utilFree(classStr);

    return result;
}

/**
 * Inner implementation of `funCall` and `methCall`, which does *not* do
 * argument validation.
 */
static zvalue funCall0(zvalue function, zint argCount, const zvalue *args) {
    zvalue funCls = classOf(function);
    zvalue result;

    // The first three cases are how we bottom out the recursion, instead of
    // calling `funCall0` on the `call` methods for `Builtin`, `Jump`, or
    // `Symbol`.
    if (funCls == CLS_Symbol) {
        // No call reporting setup here, as this will bottom out in a
        // `methCall()` which will do that.
        result = symbolCall(function, argCount, args);
    } else if (funCls == CLS_Builtin) {
        StackTraceEntry ste = {.target = function, .name = SYM(call)};
        UTIL_TRACE_START(callReporter, &ste);
        result = builtinCall(function, argCount, args);
        UTIL_TRACE_END();
    } else if (funCls == CLS_Jump) {
        StackTraceEntry ste = {.target = function, .name = SYM(call)};
        UTIL_TRACE_START(callReporter, &ste);
        result = jumpCall(function, argCount, args);
        UTIL_TRACE_END();
    } else {
        // The original `function` is some kind of higher layer function.
        // Use method dispatch to get to it.
        result = methCall(function, SYM(call), argCount, args);
    }

    return result;
}


//
// Module Definitions
//

// Documented in header.
zvalue symbolCall(zvalue symbol, zint argCount, const zvalue *args) {
    if (argCount < 1) {
        die("Too few arguments for symbol call.");
    }

    return methCall(args[0], symbol, argCount - 1, &args[1]);
}


//
// Exported Definitions
//

// Documented in header.
zvalue funApply(zvalue function, zvalue args) {
    zint argCount = (args == NULL) ? 0 : get_size(args);

    if (argCount == 0) {
        return funCall(function, 0, NULL);
    } else {
        zvalue argsArray[argCount];
        arrayFromList(argsArray, args);
        return funCall(function, argCount, argsArray);
    }
}

// Documented in header.
zvalue funCall(zvalue function, zint argCount, const zvalue *args) {
    if (argCount < 0) {
        die("Invalid argument count for function call: %lld", argCount);
    } else if ((argCount != 0) && (args == NULL)) {
        die("Function call argument inconsistency.");
    }

    zstackPointer save = datFrameStart();
    zvalue result = funCall0(function, argCount, args);
    datFrameReturn(save, result);
    return result;
}

// Documented in header.
zvalue methApply(zvalue target, zvalue name, zvalue args) {
    zint argCount = (args == NULL) ? 0 : get_size(args);

    if (argCount == 0) {
        return methCall(target, name, 0, NULL);
    } else {
        zvalue argsArray[argCount];
        arrayFromList(argsArray, args);
        return methCall(target, name, argCount, argsArray);
    }
}

// Documented in header.
zvalue methCall(zvalue target, zvalue name, zint argCount,
        const zvalue *args) {
    zint index = symbolIndex(name);
    zvalue cls = classOf(target);
    zvalue function = classFindMethodUnchecked(cls, index);

    if (function == NULL) {
        zvalue nameStr = cm_castFrom(CLS_String, name);
        die("Unbound method: %s.%s", cm_debugString(cls),
            cm_debugString(nameStr));
    }

    // Prepend `target` as a new first argument for a call to `function`.
    zvalue newArgs[argCount + 1];
    newArgs[0] = target;
    utilCpy(zvalue, &newArgs[1], args, argCount);

    StackTraceEntry ste = {.target = target, .name = name};
    UTIL_TRACE_START(callReporter, &ste);

    zstackPointer save = datFrameStart();
    zvalue result = funCall0(function, argCount + 1, newArgs);
    datFrameReturn(save, result);

    UTIL_TRACE_END();
    return result;
}

// Documented in header.
zvalue vaMethCall(zvalue target, zvalue name, ...) {
    zint size = 0;
    va_list rest;

    va_start(rest, name);
    for (;;) {
        if (va_arg(rest, zvalue) == NULL) {
            break;
        }
        size++;
    }
    va_end(rest);

    zvalue values[size];

    va_start(rest, name);
    for (zint i = 0; i < size; i++) {
        values[i] = va_arg(rest, zvalue);
    }
    va_end(rest);

    return methCall(target, name, size, values);
}

// Documented in header.
zvalue mustNotYield(zvalue value) {
    die("Improper yield from `noYield` expression.");
}
