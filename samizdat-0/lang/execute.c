/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. See the associated file "LICENSE.md" for details.
 */

#include "consts.h"
#include "impl.h"
#include "util.h"

#include <stddef.h>


/**
 * Closure, that is a function and its associated state. Instances
 * of this structure are bound as the closure state as part of
 * function registration in `execFunction()`.
 */
typedef struct {
    /**
     * Parent context (which was the current context when the function
     * was defined).
     */
    zcontext parent;

    /**
     * Function definition, which includes a list of formals and the
     * statements to run.
     */
    zvalue function;
} Closure;


/*
 * Helper functions
 */

/* Defined below. */
static zvalue execExpression(zcontext ctx, zvalue expression);
static void execStatements(zcontext ctx, zvalue statements);

/**
 * The C function that is used for all registrations with function
 * registries.
 */
zvalue execClosure(void *state, zint argCount, const zvalue *args) {
    Closure *closure = state;
    zvalue formals = datMapletGet(closure->function, STR_FORMALS);
    zvalue statements = datMapletGet(closure->function, STR_STATEMENTS);
    zint formalsSize = datSize(formals);
    zvalue locals = datMapletEmpty();

    if (argCount < formalsSize) {
        die("Too few arguments to function: %lld < %lld",
            argCount, formalsSize);
    }

    for (zint i = 0; i < formalsSize; i++) {
        zvalue name = datListletGet(formals, i);
        locals = datMapletPut(locals, name, args[i]);
    }

    zcontext ctx = ctxNewChild(closure->parent, locals);
    execStatements(ctx, statements);

    zvalue result = ctx->toReturn;
    return (result == NULL) ? TOK_NULL : result;
}

/**
 * Executes a `function` form.
 */
static zvalue execFunction(zcontext ctx, zvalue function) {
    hidAssertType(function, STR_FUNCTION);

    Closure *closure = zalloc(sizeof(Closure));
    closure->parent = ctx;
    closure->function = hidValue(function);

    return funAdd(ctx->reg, execClosure, closure);
}

/**
 * Executes a `call` form.
 */
static zvalue execCall(zcontext ctx, zvalue call) {
    hidAssertType(call, STR_CALL);

    call = hidValue(call);
    zvalue function = datMapletGet(call, STR_FUNCTION);
    zvalue actuals = datMapletGet(call, STR_ACTUALS);

    zvalue id = execExpression(ctx, function);
    datAssertUniqlet(id);

    zint argCount = datSize(actuals);
    zvalue args[argCount];
    for (zint i = 0; i < argCount; i++) {
        zvalue one = datListletGet(actuals, i);
        args[i] = execExpression(ctx, one);
    }

    return funCall(ctx->reg, id, argCount, args);
}

/**
 * Executes a `uniqlet` form.
 */
static zvalue execUniqlet(zcontext ctx, zvalue uniqlet) {
    hidAssertType(uniqlet, STR_UNIQLET);

    return datUniqlet();
}

/**
 * Executes a `maplet` form.
 */
static zvalue execMaplet(zcontext ctx, zvalue maplet) {
    hidAssertType(maplet, STR_MAPLET);

    zvalue elems = hidValue(maplet);
    zint size = datSize(elems);
    zvalue result = datListletEmpty();

    for (zint i = 0; i < size; i++) {
        zvalue one = datListletGet(elems, i);
        zvalue key = datMapletGet(one, STR_KEY);
        zvalue value = datMapletGet(one, STR_VALUE);
        result = datMapletPut(result,
                              execExpression(ctx, key),
                              execExpression(ctx, value));
    }

    return result;
}

/**
 * Executes a `listlet` form.
 */
static zvalue execListlet(zcontext ctx, zvalue listlet) {
    hidAssertType(listlet, STR_LISTLET);

    zvalue elems = hidValue(listlet);
    zint size = datSize(elems);
    zvalue result = datListletEmpty();

    for (zint i = 0; i < size; i++) {
        zvalue one = datListletGet(elems, i);
        result = datListletAppend(result, execExpression(ctx, one));
    }

    return result;
}

/**
 * Executes a `varRef` form.
 */
static zvalue execVarRef(zcontext ctx, zvalue varRef) {
    hidAssertType(varRef, STR_VAR_REF);

    zvalue name = hidValue(varRef);

    for (/* ctx */; ctx != NULL; ctx = ctx->parent) {
        zvalue found = datMapletGet(ctx->locals, name);
        if (found != NULL) {
            return found;
        }
    }

    die("No such variable.");
}

/**
 * Executes a `literal` form.
 */
static zvalue execLiteral(zcontext ctx, zvalue literal) {
    hidAssertType(literal, STR_LITERAL);

    return hidValue(literal);
}

/**
 * Executes an `expression` form.
 */
static zvalue execExpression(zcontext ctx, zvalue ex) {
    if      (hidHasType(ex, STR_LITERAL))  { return execLiteral(ctx, ex);  }
    else if (hidHasType(ex, STR_VAR_REF))  { return execVarRef(ctx, ex);   }
    else if (hidHasType(ex, STR_LISTLET))  { return execListlet(ctx, ex);  }
    else if (hidHasType(ex, STR_MAPLET))   { return execMaplet(ctx, ex);   }
    else if (hidHasType(ex, STR_UNIQLET))  { return execUniqlet(ctx, ex);  }
    else if (hidHasType(ex, STR_CALL))     { return execCall(ctx, ex);     }
    else if (hidHasType(ex, STR_FUNCTION)) { return execFunction(ctx, ex); }
    else {
        die("Invalid expression type.");
    }
}

/**
 * Executes a `varDef` form.
 */
static void execVarDef(zcontext ctx, zvalue varDef) {
    hidAssertType(varDef, STR_VAR_DEF);

    zvalue nameValue = hidValue(varDef);
    zvalue name = datMapletGet(nameValue, STR_NAME);
    zvalue value = datMapletGet(nameValue, STR_VALUE);

    if (datMapletGet(ctx->locals, name) != NULL) {
        die("Duplicate assignment.");
    }

    ctx->locals = datMapletPut(ctx->locals, name, execExpression(ctx, value));
}

/**
 * Executes a `return` form.
 */
static void execReturn(zcontext ctx, zvalue returnForm) {
    hidAssertType(returnForm, STR_RETURN);

    ctx->toReturn = execExpression(ctx, hidValue(returnForm));
}

/**
 * Executes a `statements` form.
 */
static void execStatements(zcontext ctx, zvalue statements) {
    hidAssertType(statements, STR_STATEMENTS);

    statements = hidValue(statements);
    zint size = datSize(statements);

    for (zint i = 0; (i < size) && (ctx->toReturn == NULL); i++) {
        zvalue one = datListletGet(statements, i);
        zvalue type = hidType(one);
        if (hidHasType(one, STR_EXPRESSION)) {
            execExpression(ctx, one);
        } else if (hidHasType(one, STR_VAR_DEF)) {
            execVarDef(ctx, one);
        } else if (hidHasType(one, STR_RETURN)) {
            execReturn(ctx, one);
        } else {
            die("Invalid statements element.");
        }
    }
}


/*
 * Module functions
 */

/* Documented in header. */
void langExecute(zcontext ctx, zvalue code) {
    execStatements(ctx, code);
}
