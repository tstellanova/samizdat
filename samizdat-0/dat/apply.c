/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Function application
 */

#include "impl.h"


/*
 * Exported functions
 */

/* Documented in header. */
zvalue datApply(zvalue function, zvalue args) {
    zint argCount = pbSize(args);
    zvalue argsArray[argCount];

    datArrayFromList(argsArray, args);

    return datCall(function, argCount, argsArray);
}
