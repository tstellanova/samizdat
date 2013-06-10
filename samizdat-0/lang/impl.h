/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

/*
 * Private implementation details
 */

#ifndef _IMPL_H_
#define _IMPL_H_

#include "lang.h"

/**
 * Active execution frame. These are passed around during evaluation
 * as code executes, and can become referenced by closures that are
 * released "in the wild".
 */
typedef struct Frame {
    /** Parent closure value. May be `NULL`. */
    zvalue parentClosure;

    /** Parent frame. May be `NULL`. */
    struct Frame *parentFrame;

    /** Variables defined in this frame, as a map from names to values. */
    zvalue vars;
} Frame;

/**
 * Does gc value marking.
 */
void frameMark(Frame *frame);

/**
 * Adds a new variable to the given frame.
 */
void frameAdd(Frame *frame, zvalue name, zvalue value);

/**
 * Gets a variable's value out of the given frame.
 */
zvalue frameGet(Frame *frame, zvalue name);

/**
 * Snapshots the given frame into the given target.
 */
void frameSnap(Frame *target, Frame *source);

#endif