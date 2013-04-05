/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. See the associated file "LICENSE.md" for details.
 */

#include "alloc.h"
#include "impl.h"
#include "unicode.h"
#include "util.h"

#include <stdlib.h>
#include <string.h>


/*
 * Helper definitions
 */

/** The empty listlet, lazily initialized. */
static zvalue theEmptyListlet = NULL;

/**
 * Allocates a listlet of the given size.
 */
static zvalue allocListlet(zint size) {
    return samAllocValue(SAM_LISTLET, size, size * sizeof(zvalue));
}


/*
 * API Implementation
 */

/** Documented in API header. */
zvalue samListletGet(zvalue listlet, zint n) {
    samAssertListlet(listlet);
    samAssertNth(listlet, n);

    return ((SamListlet *) listlet)->elems[n];
}

/** Documented in API header. */
zvalue samListletEmpty(void) {
    if (theEmptyListlet == NULL) {
        theEmptyListlet = allocListlet(0);
    }

    return theEmptyListlet;
}

/** Documented in API header. */
zvalue samListletAppend(zvalue listlet, zvalue value) {
    samAssertListlet(listlet);

    zint oldSize = listlet->size;
    zint size = oldSize + 1;
    zvalue result = allocListlet(size);

    memcpy(((SamListlet *) result)->elems,
           ((SamListlet *) listlet)->elems,
           oldSize * sizeof(zvalue));
    ((SamListlet *) result)->elems[oldSize] = value;

    return result;
}

/** Documented in API header. */
zvalue samListletFromUtf8(const zbyte *string, zint stringBytes) {
    zint decodedSize = samUtf8DecodeStringSize(string, stringBytes);
    zvalue result = allocListlet(decodedSize);

    samUtf8DecodeStringToValues(string, stringBytes,
                                ((SamListlet *) result)->elems);
    return result;
}
