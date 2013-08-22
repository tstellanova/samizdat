/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"
#include "type/List.h"
#include "type/Map.h"
#include "type/Value.h"
#include "util.h"


/*
 * Exported Definitions
 */

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(makeValueMap) {
    zint size = argCount - 1;
    zvalue value = args[size];

    if (size == 0) {
        return EMPTY_MAP;
    }

    zmapping mappings[size];

    for (zint i = 0; i < size; i++) {
        mappings[i].key = args[i];
        mappings[i].value = value;
    }

    return mapFromArray(size, mappings);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(mapKeys) {
    zvalue map = args[0];
    zint size = collSize(map);
    zmapping mappings[size];
    zvalue arr[size];

    arrayFromMap(mappings, map);

    for (zint i = 0; i < size; i++) {
        arr[i] = mappings[i].key;
    }

    return listFromArray(size, arr);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(mappingKey) {
    return mappingKey(args[0]);
}

/* Documented in Samizdat Layer 0 spec. */
PRIM_IMPL(mappingValue) {
    return mappingValue(args[0]);
}
