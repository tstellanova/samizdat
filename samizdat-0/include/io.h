/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * I/O
 */

#ifndef _IO_H_
#define _IO_H_

#include "pb.h"

/**
 * Returns `getcwd()` as a string.
 */
zvalue ioFlatCwd(void);

/**
 * Gets symbolic link information about the file with the given name.
 * It the file names a symbolic link, then this returns the linked path as
 * a simple string. If the file does not name a symbolic link, this returns
 * `NULL`.
 */
zvalue ioFlatReadLink(zvalue flatPath);

/**
 * Reads the file with the given name in its entirety, interpreting
 * it as UTF-8. Returns a string (list of Unicode-representing
 * ints) of the contents.
 */
zvalue ioFlatReadFileUtf8(zvalue flatPath);

/**
 * Writes the given string to the file with the given name, encoding
 * it as UTF-8.
 */
void ioFlatWriteFileUtf8(zvalue flatPath, zvalue text);

#endif