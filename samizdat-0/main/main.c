/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "io.h"
#include "lang.h"
#include "lib.h"
#include "util.h"

#include <stdlib.h>


/**
 * Main driver for Samizdat Layer 0. Takes the first argument to
 * be a file name. Reads the file, and compiles it to an executable
 * parse tree in an execution context consisting of the Samizdat Layer
 * 0 core library. It evaluates that tree, which should evaluate to
 * a function. It then calls that function, passing it the remaining
 * arguments.
 */
int main(int argc, char **argv) {
    if (argc < 2) {
        die("Too few arguments.");
    }

    zvalue fileName = datStringletFromUtf8String(-1, argv[1]);
    zint argCount = argc - 2;
    zvalue args = datListletEmpty();

    for (int i = 0; i < argCount; i++) {
        zvalue arg = datStringletFromUtf8String(-1, argv[i + 2]);
        args = datListletAppend(args, arg);
    }

    zcontext ctx = libNewContext();
    zvalue programText = readFile(fileName);
    zvalue program = langNodeFromProgramText(programText);
    zvalue function = langEvalExpressionNode(ctx, program);
    zvalue result = langApply(function, args);

    if ((result != NULL) && (datType(result) == DAT_INTLET)) {
        exit((int) datIntFromIntlet(result));
    }
}
