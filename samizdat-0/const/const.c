/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "dat.h"
#include "impl.h"
#include "type/String.h"
#include "type/Value.h"
#include "zlimits.h"


/*
 * Define globals for all of the constants.
 */

#define STR(name, str) zvalue STR_##name = NULL

#define TOK(name, str) \
    STR(name, str); \
    zvalue TOK_##name = NULL

#include "const/const-def.h"

#undef STR
#undef TOK


/*
 * Module Definitions
 */

/* Documented in header. */
void constInit(void) {
    if (STR_def != NULL) {
        return;
    }

    datInit();

    zstackPointer save = pbFrameStart();

    #define STR(name, str) \
        STR_##name = stringFromUtf8(-1, str); \
        pbImmortalize(STR_##name)

    #define TOK(name, str) \
        STR(name, str); \
        TOK_##name = makeValue(STR_##name, NULL); \
        pbImmortalize(TOK_##name)

    #include "const/const-def.h"

    generatorInit();

    pbFrameReturn(save, NULL);

    // Force a garbage collection here, mainly to get a reasonably early
    // failure if gc is broken.
    pbGc();
}
