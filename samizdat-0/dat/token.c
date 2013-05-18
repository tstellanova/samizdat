/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "impl.h"

#include <stddef.h>


/*
 * Helper definitions
 */

/**
 * Gets a pointer to the highlet's info.
 */
static TokenInfo *highletInfo(zvalue highlet) {
    return &((DatToken *) highlet)->info;
}

/**
 * Allocates and initializes a highlet, without doing error-checking
 * on the arguments.
 */
static zvalue newToken(zvalue type, zvalue value) {
    zvalue result = datAllocValue(DAT_HIGHLET, 0, sizeof(TokenInfo));
    TokenInfo *info = highletInfo(result);

    result->size = (value == NULL) ? 0 : 1;
    info->type = type;
    info->value = value;
    return result;
}


/*
 * Module functions
 */

/* Documented in header. */
bool datTokenEq(zvalue v1, zvalue v2) {
    TokenInfo *info1 = highletInfo(v1);
    TokenInfo *info2 = highletInfo(v2);

    if (!datEq(info1->type, info2->type)) {
        return false;
    }

    return (info1->value == NULL) || datEq(info1->value, info2->value);
}

/* Documented in header. */
zorder datTokenOrder(zvalue v1, zvalue v2) {
    TokenInfo *info1 = highletInfo(v1);
    TokenInfo *info2 = highletInfo(v2);

    zorder result = datOrder(info1->type, info2->type);

    if (result != ZSAME) {
        return result;
    } else if (info1->value == NULL) {
        return (info2->value == NULL) ? ZSAME : ZLESS;
    } else if (info2->value == NULL) {
        return ZMORE;
    } else {
        return datOrder(info1->value, info2->value);
    }
}

/* Documented in header. */
void datTokenMark(zvalue value) {
    TokenInfo *info = highletInfo(value);

    datMark(info->type);
    if (info->value != NULL) {
        datMark(info->value);
    }
}


/*
 * Exported functions
 */

/* Documented in header. */
zvalue datTokenType(zvalue highlet) {
    datAssertToken(highlet);
    return highletInfo(highlet)->type;
}

/* Documented in header. */
zvalue datTokenValue(zvalue highlet) {
    datAssertToken(highlet);
    return highletInfo(highlet)->value;
}

/* Documented in header. */
zvalue datTokenFrom(zvalue type, zvalue value) {
    datAssertValid(type);

    if (value != NULL) {
        datAssertValid(value);
    }

    return newToken(type, value);
}

/* Documented in header. */
bool datTokenTypeIs(zvalue highlet, zvalue type) {
    return (datEq(datTokenType(highlet), type));
}

/* Documented in header. */
void datTokenAssertType(zvalue highlet, zvalue type) {
    if (!datTokenTypeIs(highlet, type)) {
        die("Type mismatch.");
    }
}
