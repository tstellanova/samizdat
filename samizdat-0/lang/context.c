/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "consts.h"
#include "impl.h"
#include "util.h"

#include <stddef.h>


/*
 * Module functions
 */

/* Documented in header. */
zcontext ctxNewChild(zcontext parent, zvalue locals) {
    zcontext ctx = zalloc(sizeof(ExecutionContext));

    ctx->locals = locals;
    ctx->parent = parent;

    return ctx;
}

/* Documented in header. */
void ctxBind(zcontext ctx, zvalue name, zvalue value) {
    if (datMapletGet(ctx->locals, name) != NULL) {
        die("Duplicate assignment.");
    }

    ctx->locals = datMapletPut(ctx->locals, name, value);
}

/* Documented in header. */
zvalue ctxGet(zcontext ctx, zvalue name) {
    zcontext zorig = ctx;
    for (/* ctx */; ctx != NULL; ctx = ctx->parent) {
        zvalue found = datMapletGet(ctx->locals, name);
        if (found != NULL) {
            return found;
        }
    }

    die("No such variable.");
}


/*
 * Exported functions
 */

/* Documented in header. */
zcontext langCtxNew(void) {
    return ctxNewChild(NULL, datMapletEmpty());
}

/* Documented in header. */
zcontext langCtxNewChild(zcontext parent) {
    return ctxNewChild(parent, datMapletEmpty());
}

/* Documented in header. */
void langCtxBind(zcontext ctx, const char *name, zvalue value) {
    ctxBind(ctx, datStringletFromUtf8String(-1, name), value);
}

/* Documented in header. */
void langCtxBindFunction(zcontext ctx, const char *name,
                      zfunction function, void *state) {
    langCtxBind(ctx, name, funDefine(function, state));
}

/* Documented in header. */
void langCtxBindAll(zcontext ctx, zvalue maplet) {
    zvalue keys = datMapletKeys(maplet);
    zint size = datSize(keys);

    for (zint i = 0; i < size; i++) {
        zvalue key = datListletNth(keys, i);
        ctxBind(ctx, key, datMapletGet(maplet, key));
    }
}

/* Documented in header. */
zvalue langMapletFromCtx(zcontext ctx) {
    if (ctx->parent == NULL) {
        return ctx->locals;
    } else {
        zvalue result = langMapletFromCtx(ctx->parent);
        return datMapletAdd(result, ctx->locals);
    }
}
