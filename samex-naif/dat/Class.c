// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

#include <stdlib.h>

#include "type/Builtin.h"
#include "type/Class.h"
#include "type/Data.h"
#include "type/Int.h"
#include "type/Jump.h"
#include "type/Object.h"
#include "type/Record.h"
#include "type/String.h"
#include "type/Symbol.h"
#include "type/SymbolTable.h"
#include "type/define.h"
#include "util.h"
#include "zlimits.h"

#include "impl.h"


//
// Private Definitions
//

/** Constants identifying class category, used when sorting classes. */
typedef enum {
    CORE_CLASS = 0,
    RECORD_CLASS,
    OPAQUE_CLASS
} ClassCategory;

/** Next class sequence number to assign. */
static zint theNextClassId = 0;

/** The `secret` value used for all core classes. */
static zvalue theCoreSecret = NULL;


/**
 * Gets a pointer to the value's info.
 */
static ClassInfo *getInfo(zvalue cls) {
    return datPayload(cls);
}

/**
 * Compare two classes for equality. Does *not* check to see if the two
 * arguments are actually classes.
 *
 * **Note:** This is just a `==` check, as the system doesn't allow for
 * two different underlying pointers to be references to the same class.
 */
static bool classEqUnchecked(zvalue cls1, zvalue cls2) {
    return (cls1 == cls2);
}

/**
 * Asserts that `value` is an instance of `Class`, that is that
 * `hasClass(value, CLS_Class)` is true.
 */
static void assertHasClassClass(zvalue value) {
    if (!classEqUnchecked(get_class(value), CLS_Class)) {
        die("Expected class Class; got %s.", valDebugString(value));
    }
}

/**
 * Initializes a class value.
 */
static void classInit(zvalue cls, zvalue name, zvalue parent, zvalue secret) {
    assertHasClass(name, CLS_Symbol);

    if (theNextClassId == DAT_MAX_CLASSES) {
        die("Too many classes!");
    } else if ((parent == NULL) && !classEqUnchecked(cls, CLS_Value)) {
        die("Every class but `Value` needs a parent.");
    }

    ClassInfo *info = getInfo(cls);
    info->parent = parent;
    info->name = name;
    info->secret = secret;
    info->classId = theNextClassId;
    info->hasSubclasses = false;

    theNextClassId++;
    datImmortalize(cls);

    if (parent != NULL) {
        getInfo(parent)->hasSubclasses = true;
    }
}

/**
 * Helper for initializing the classes built directly in this file.
 */
static void classInitHere(zvalue cls, zvalue parent, const char *name) {
    classInit(cls, symbolFromUtf8(-1, name), parent, theCoreSecret);
}

/**
 * Allocates a class value.
 */
static zvalue allocClass(void) {
    return datAllocValue(CLS_Class, sizeof(ClassInfo));
}

/**
 * Gets the category of a class (given its info), for sorting.
 */
static ClassCategory categoryOf(ClassInfo *info) {
    zvalue secret = info->secret;

    if (secret == theCoreSecret) {
        return CORE_CLASS;
    } else if (secret == NULL) {
        return RECORD_CLASS;
    } else {
        return OPAQUE_CLASS;
    }
}


//
// Module Definitions
//

// Documented in header.
void classBindMethods(zvalue cls, zvalue classMethods,
        zvalue instanceMethods) {
    ClassInfo *info = getInfo(cls);

    if (info->hasSubclasses && (info->parent != NULL)) {
        // `Value` (the only class without a parent) gets a pass on this
        // sanity check, since during during bootstrap it gains subclasses
        // before it's possible to define its methods.
        die("Cannot modify method table of a class with subclasses.");
    }

    if (info->parent != NULL) {
        // Initialize the instance method table with whatever the parent
        // defined.
        utilCpy(zvalue, info->methods, getInfo(info->parent)->methods,
            DAT_MAX_SYMBOLS);
    }

    if (classMethods != NULL) {
        die("No class methods allowed...yet.");
    }

    if (instanceMethods != NULL) {
        zint size = symbolTableSize(instanceMethods);
        zmapping methods[size];
        arrayFromSymbolTable(methods, instanceMethods);
        for (zint i = 0; i < size; i++) {
            zvalue sym = methods[i].key;
            zint index = symbolIndex(methods[i].key);
            info->methods[index] = methods[i].value;
        }
    }
}

// Documented in header.
zvalue classFindMethodUnchecked(zvalue cls, zint index) {
    return getInfo(cls)->methods[index];
}


//
// Exported Definitions
//

// Documented in header.
void assertHasClass(zvalue value, zvalue cls) {
    if (!hasClass(value, cls)) {
        die("Expected class %s; got %s.",
            valDebugString(cls), valDebugString(value));
    }
}

// Documented in header.
void classAddMethod(zvalue cls, zvalue symbol, zvalue function) {
    assertHasClassClass(cls);
    ClassInfo *info = getInfo(cls);
    zint index = symbolIndex(symbol);

    if (info->hasSubclasses) {
        die("Cannot modify method table of a class with subclasses.");
    }

    info->methods[index] = function;
}

// Documented in header.
bool classHasParent(zvalue cls, zvalue parent) {
    assertHasClassClass(cls);
    assertHasClassClass(parent);
    return classEqUnchecked(getInfo(cls)->parent, parent);
}

// Documented in header.
bool classHasSecret(zvalue cls, zvalue secret) {
    assertHasClassClass(cls);

    ClassInfo *info = getInfo(cls);

    // Note: It's important to pass `info->secret` first, so that it's the
    // one whose `totalEq` method is used. The given `secret` can't be
    // trusted to behave.
    return valEq(info->secret, secret);
}

// Documented in header.
zint classIndex(zvalue cls) {
    assertHasClassClass(cls);
    return getInfo(cls)->classId;
}

// Documented in header.
zint get_classIndex(zvalue value) {
    return getInfo(get_class(value))->classId;
}

// Documented in header.
bool hasClass(zvalue value, zvalue cls) {
    assertHasClassClass(cls);

    for (zvalue valueCls = get_class(value);
            valueCls != NULL;
            valueCls = getInfo(valueCls)->parent) {
        if (classEqUnchecked(valueCls, cls)) {
            return true;
        }
    }

    return false;
}

// Documented in header.
bool haveSameClass(zvalue value, zvalue other) {
    return classEqUnchecked(get_class(value), get_class(other));
}

// Documented in header.
zvalue makeClass(zvalue name, zvalue parent, zvalue secret,
        zvalue classMethods, zvalue instanceMethods) {
    assertHasClassClass(parent);

    zvalue result = allocClass();
    ClassInfo *info = getInfo(result);
    classInit(result, name, parent, secret);
    classBindMethods(result, classMethods, instanceMethods);

    return result;
}

// Documented in header.
zvalue makeCoreClass(const char *name, zvalue parent,
        zvalue classMethods, zvalue instanceMethods) {
    return makeClass(symbolFromUtf8(-1, name), parent, theCoreSecret,
        classMethods, instanceMethods);
}


//
// Class Definition
//

// Documented in header.
METH_IMPL_0(Class, debugString) {
    ClassInfo *info = getInfo(ths);
    zvalue extraString;

    if (info->secret == theCoreSecret) {
        return valToString(info->name);
    } else if (info->secret != NULL) {
        extraString = stringFromUtf8(-1, " : opaque");
    } else if (classEqUnchecked(info->parent, CLS_Record)) {
        extraString = EMPTY_STRING;
    } else {
        die("Shouldn't happen: opaque class without secret.");
    }

    return METH_CALL(cat,
        stringFromUtf8(-1, "@@("),
        METH_CALL(debugString, valToString(info->name)),
        extraString,
        stringFromUtf8(-1, ")"));
}

// Documented in header.
METH_IMPL_0(Class, gcMark) {
    ClassInfo *info = getInfo(ths);

    datMark(info->name);
    datMark(info->secret);

    for (zint i = 0; i < DAT_MAX_SYMBOLS; i++) {
        datMark(info->methods[i]);
    }

    return NULL;
}

// Documented in header.
METH_IMPL_0(Class, get_name) {
    return getInfo(ths)->name;
}

// Documented in header.
METH_IMPL_0(Class, get_parent) {
    return getInfo(ths)->parent;
}

// Documented in header.
METH_IMPL_1(Class, totalOrder, other) {
    if (ths == other) {
        // Easy case to avoid decomposition and detailed tests.
        return INT_0;
    }

    assertHasClassClass(other);  // Note: Not guaranteed to be a `Class`.
    ClassInfo *info1 = getInfo(ths);
    ClassInfo *info2 = getInfo(other);
    zvalue name1 = info1->name;
    zvalue name2 = info2->name;
    ClassCategory cat1 = categoryOf(info1);
    ClassCategory cat2 = categoryOf(info2);

    // Compare categories for major order.

    if (cat1 < cat2) {
        return INT_NEG1;
    } else if (cat1 > cat2) {
        return INT_1;
    }

    // Compare names for minor order.

    zorder nameOrder = valZorder(name1, name2);
    if (nameOrder != ZSAME) {
        return intFromZint(nameOrder);
    }

    // Names are the same. The order is not defined given two different
    // opaque classes.

    return (cat1 == OPAQUE_CLASS) ? NULL : INT_0;
}

/**
 * Define `objectModel` as a module, as separate from the `Class` class.
 */
MOD_INIT(objectModel) {
    // Make sure that the "fake" header is sized the same as the real one.
    if (DAT_HEADER_SIZE != sizeof(DatHeader)) {
        die("Mismatched value header size: should be %lu", sizeof(DatHeader));
    }

    CLS_Class = allocClass();
    CLS_Class->cls = CLS_Class;

    CLS_Value       = allocClass();
    CLS_Symbol      = allocClass();
    CLS_SymbolTable = allocClass();
    CLS_Builtin     = allocClass();

    theCoreSecret = anonymousSymbolFromUtf8(-1, "coreSecret");
    datImmortalize(theCoreSecret);

    classInitHere(CLS_Class,       CLS_Value, "Class");
    classInitHere(CLS_Value,       NULL,      "Value");
    classInitHere(CLS_Symbol,      CLS_Value, "Symbol");
    classInitHere(CLS_SymbolTable, CLS_Value, "SymbolTable");
    classInitHere(CLS_Builtin,     CLS_Value, "Builtin");
}

/** Initializes the module. */
MOD_INIT(Class) {
    MOD_USE(Value);

    SYM_INIT(get_name);
    SYM_INIT(get_parent);

    // Note: The `objectModel` module (directly above) initializes `CLS_Class`.
    classBindMethods(CLS_Class,
        NULL,
        symbolTableFromArgs(
            METH_BIND(Class, debugString),
            METH_BIND(Class, gcMark),
            METH_BIND(Class, get_name),
            METH_BIND(Class, get_parent),
            METH_BIND(Class, totalOrder),
            NULL));
}

// Documented in header.
zvalue CLS_Class = NULL;

// Documented in header.
SYM_DEF(get_name);

// Documented in header.
SYM_DEF(get_parent);
