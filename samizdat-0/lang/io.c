/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. See the associated file "LICENSE.md" for details.
 */

#include "impl.h"
#include "util.h"

#include <stdio.h>
#include <string.h>
#include <errno.h>

enum {
    /** Maximum readable file size, in bytes. */
    MAX_FILE_SIZE = 100000
};


/*
 * Module functions
 */

/* Documented in header. */
zvalue readFile(zvalue fileName) {
    zint nameSize = datStringletUtf8Size(fileName);
    char nameUtf[nameSize + 1];

    datStringletEncodeUtf8(fileName, nameUtf);
    nameUtf[nameSize] = '\0';

    FILE *in = fopen((char *) nameUtf, "r");
    if (in == NULL) {
        die("Trouble opening file \"%s\": %s", nameUtf, strerror(errno));
    }

    char buf[MAX_FILE_SIZE];
    size_t amt = fread(buf, 1, sizeof(buf), in);

    if (ferror(in)) {
        die("Trouble reading file \"%s\": %s", nameUtf, strerror(errno));
    }

    if (!feof(in)) {
        die("Overlong file \"%s\": %s", nameUtf, strerror(errno));
    }

    fclose(in);

    return datStringletFromUtf8String(buf, amt);
}
