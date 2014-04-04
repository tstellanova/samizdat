/*
 * Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Initialization
 */

#include "const.h"
#include "impl.h"
#include "type/Type.h"

#include <stddef.h>
#include <string.h>



/*
 * Module Definitions
 */

/* Documented in header. */
zevalType langTypeMap[DAT_MAX_TYPES];

/* This provides the non-inline version of this function. */
extern zevalType get_evalType(zvalue node);

/** Initializes the module. */
MOD_INIT(lang) {
    MOD_USE(const);
    MOD_USE(Box);
    MOD_USE(Closure);
    MOD_USE(Generator);
    MOD_USE(Map);

    memset(langTypeMap, 0, sizeof(langTypeMap));
    langTypeMap[typeIndex(TYPE_apply)]         = EVAL_apply;
    langTypeMap[typeIndex(TYPE_call)]          = EVAL_call;
    langTypeMap[typeIndex(TYPE_closure)]       = EVAL_closure;
    langTypeMap[typeIndex(TYPE_jump)]          = EVAL_jump;
    langTypeMap[typeIndex(TYPE_literal)]       = EVAL_literal;
    langTypeMap[typeIndex(TYPE_varBind)]       = EVAL_varBind;
    langTypeMap[typeIndex(TYPE_varDef)]        = EVAL_varDef;
    langTypeMap[typeIndex(TYPE_varDefMutable)] = EVAL_varDefMutable;
    langTypeMap[typeIndex(TYPE_varRef)]        = EVAL_varRef;
}
