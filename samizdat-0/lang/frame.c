/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Execution frames
 */

#include "impl.h"
#include "util.h"

#include <stddef.h>


/*
 * Helper functions
 */

/**
 * Dies with a message, citing a variable name.
 */
static void dieForVariable(const char *message, zvalue name)
    __attribute__((noreturn));
static void dieForVariable(const char *message, zvalue name) {
    die("%s: %s", message, datDebugString(name));
}


/*
 * Module functions
 */

/* Documented in header. */
void frameInit(Frame *frame, Frame *parentFrame, zvalue parentClosure,
    zvalue vars) {
    frame->parentFrame = parentFrame;
    frame->parentClosure = parentClosure;
    frame->vars = vars;
}

/* Documented in header. */
void frameMark(Frame *frame) {
    pbMark(frame->vars);
    pbMark(frame->parentClosure);
}

/* Documented in header. */
void frameAdd(Frame *frame, zvalue name, zvalue value) {
    zvalue vars = frame->vars;
    zvalue newVars = datMapPut(vars, name, value);

    if (datSize(vars) == datSize(newVars)) {
        dieForVariable("Variable already defined", name);
    }

    frame->vars = newVars;
}

/* Documented in header. */
zvalue frameGet(Frame *frame, zvalue name) {
    while (frame != NULL) {
        zvalue result = datMapGet(frame->vars, name);

        if (result != NULL) {
            return result;
        }

        frame = frame->parentFrame;
    }

    dieForVariable("Variable not defined", name);
}

/* Documented in header. */
void frameSnap(Frame *target, Frame *source) {
    *target = *source;
}
