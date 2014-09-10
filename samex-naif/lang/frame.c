// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// Execution frames
//

#include "type/Box.h"
#include "type/Map.h"
#include "type/String.h"
#include "type/Symbol.h"
#include "util.h"

#include "impl.h"


//
// Module Definitions
//

// Documented in header.
void frameInit(Frame *frame, Frame *parentFrame, zvalue parentClosure,
        zvalue vars) {
    assertValidOrNull(parentClosure);
    assertValid(vars);

    if ((parentFrame != NULL) && !parentFrame->onHeap) {
        die("Stack-allocated `parentFrame`.");
    }

    frame->parentFrame = parentFrame;
    frame->parentClosure = parentClosure;
    frame->vars = vars;
    frame->onHeap = false;
}

// Documented in header.
void frameMark(Frame *frame) {
    datMark(frame->vars);
    datMark(frame->parentClosure);  // This will mark `parentFrame`.
}

// Documented in header.
void frameDef(Frame *frame, zvalue name, zvalue box) {
    zvalue vars = frame->vars;
    zvalue newVars = collPut(vars, name, box);

    if (get_size(vars) == get_size(newVars)) {
        die("Variable already defined: %s", valDebugString(valToString(name)));
    }

    frame->vars = newVars;
}

// Documented in header.
zvalue frameGet(Frame *frame, zvalue name) {
    for (/*frame*/; frame != NULL; frame = frame->parentFrame) {
        zvalue result = get(frame->vars, name);

        if (result != NULL) {
            return result;
        }
    }

    die("Variable not defined: %s", valDebugString(valToString(name)));
}

// Documented in header.
void frameSnap(Frame *target, Frame *source) {
    assertValidOrNull(source->parentClosure);
    assertValid(source->vars);

    *target = *source;
    target->onHeap = true;
}
