// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// Generator protocol
//

#include "type/Box.h"
#include "type/Builtin.h"
#include "type/Generator.h"
#include "type/List.h"
#include "type/define.h"

#include "impl.h"


//
// Class Definition: `Generator`
//

// Documented in spec.
FUNC_IMPL_1_opt(Generator_stdCollect, generator, function) {
    zvalue stackArr[CLS_MAX_GENERATOR_ITEMS_SOFT];
    zvalue *arr = stackArr;
    zint maxSize = CLS_MAX_GENERATOR_ITEMS_SOFT;
    zint at = 0;

    zstackPointer save = datFrameStart();
    zvalue box = cm_new(Cell);

    for (;;) {
        zvalue nextGen = METH_CALL(generator, nextValue, box);

        if (nextGen == NULL) {
            break;
        }

        zvalue one = cm_fetch(box);
        generator = nextGen;

        if (function != NULL) {
            one = FUN_CALL(function, one);
            if (one == NULL) {
                continue;
            }
        } else if (one == NULL) {
            die("Unexpected lack of result.");
        }

        if (at == maxSize) {
            if (arr == stackArr) {
                maxSize = CLS_MAX_GENERATOR_ITEMS_HARD;
                arr = utilAlloc(maxSize * sizeof(zvalue));
                utilCpy(zvalue, arr, stackArr, at);
            } else {
                die("Generator produced way too many items.");
            }
        }

        arr[at] = one;
        at++;
    }

    zvalue result = listFromZarray((zarray) {at, arr});
    datFrameReturn(save, result);

    if (arr != stackArr) {
        utilFree(arr);
    }

    return result;
}

// Documented in spec.
FUNC_IMPL_1(Generator_stdFetch, generator) {
    zvalue result;

    zvalue box = cm_new(Cell);
    zvalue nextGen = METH_CALL(generator, nextValue, box);

    if (nextGen == NULL) {
        // We were given a voided generator.
        result = NULL;
    } else {
        // We got a value out of the generator. Now need to make sure it's
        // voided.
        result = cm_fetch(box);
        if (METH_CALL(nextGen, nextValue, box) != NULL) {
            die("Generator produced second item in `fetch`.");
        }
    }

    return result;
}

// Documented in spec.
FUNC_IMPL_1_opt(Generator_stdForEach, generator, function) {
    zvalue box = cm_new(Cell);
    zvalue result = NULL;

    for (;;) {
        generator = METH_CALL(generator, nextValue, box);

        if (generator == NULL) {
            break;
        }

        if (function == NULL) {
            result = cm_fetch(box);
        } else {
            zvalue v = FUN_CALL(function, cm_fetch(box));
            if (v != NULL) {
                result = v;
            }
        }
    }

    return result;
}

/** Initializes the module. */
MOD_INIT(Generator) {
    MOD_USE(Value);

    FUN_Generator_stdCollect =
        datImmortalize(FUNC_VALUE(Generator_stdCollect));

    FUN_Generator_stdFetch = datImmortalize(FUNC_VALUE(Generator_stdFetch));

    FUN_Generator_stdForEach =
        datImmortalize(FUNC_VALUE(Generator_stdForEach));
}

// Documented in header.
zvalue FUN_Generator_stdCollect = NULL;

// Documented in header.
zvalue FUN_Generator_stdFetch = NULL;

// Documented in header.
zvalue FUN_Generator_stdForEach = NULL;
