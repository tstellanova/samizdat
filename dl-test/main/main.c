/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "type/Map.h"
#include "type/String.h"
#include "util.h"

zvalue eval(zvalue context) {
    note("=== Hello module!");

    zvalue testStr = stringFromUtf8(-1, "TEST");
    zmapping mappings[] = {
        { testStr, testStr }
    };

    return mapFromArray(1, mappings);
}
