// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

#include "type/Object.h"
#include "type/SymbolTable.h"
#include "type/define.h"

#include "impl.h"


//
// Private Definitions
//

/**
 * Payload data for all object values.
 */
typedef struct {
    /** Data payload. */
    zvalue data;
} ObjectInfo;

/**
 * Gets the info of an object.
 */
static ObjectInfo *getInfo(zvalue obj) {
    return (ObjectInfo *) datPayload(obj);
}

/**
 * Class method to construct an instance. This is the function that's bound as
 * the class method for the `new` symbol.
 */
CMETH_IMPL_0_1(Object, new, data) {
    if (data == NULL) {
        data = EMPTY_SYMBOL_TABLE;
    } else {
        assertHasClass(data, CLS_SymbolTable);
    }

    zvalue result = datAllocValue(thsClass, sizeof(ObjectInfo));

    getInfo(result)->data = data;
    return result;
}

/**
 * Instance method to construct an instance. This is the function that's bound
 * as the instance method for the `new` symbol.
 */
METH_IMPL_0_1(Object, new, data) {
    if (data == NULL) {
        data = EMPTY_SYMBOL_TABLE;
    } else {
        assertHasClass(data, CLS_SymbolTable);
    }

    zvalue result = datAllocValue(classOf(ths), sizeof(ObjectInfo));

    getInfo(result)->data = data;
    return result;
}

/**
 * Method to get the given object's data payload. This is the function
 * that's bound as the instance method for the `access` symbol.
 */
METH_IMPL_0(Object, access) {
    return getInfo(ths)->data;
}


//
// Class Definition
//

// Documented in spec.
CMETH_IMPL_2_4(Object, subclass, name, config,
        classMethods, instanceMethods) {
    if (thsClass != CLS_Object) {
        die("Invalid parent class: %s", cm_debugString(thsClass));
    }

    if (classMethods == NULL) {
        classMethods = EMPTY_SYMBOL_TABLE;
    }

    if (instanceMethods == NULL) {
        instanceMethods = EMPTY_SYMBOL_TABLE;
    }

    zvalue accessSecret = cm_get(config, SYM(access));
    zvalue newSecret = cm_get(config, SYM(new));

    if (accessSecret != NULL) {
        instanceMethods = cm_cat(instanceMethods,
            METH_TABLE(accessSecret, FUNC_VALUE(Object_access)));
    }

    if (newSecret != NULL) {
        classMethods = cm_cat(classMethods,
            METH_TABLE(newSecret, FUNC_VALUE(class_Object_new)));
        instanceMethods = cm_cat(instanceMethods,
            METH_TABLE(newSecret, FUNC_VALUE(Object_new)));
    }

    return makeClass(name, CLS_Object, classMethods, instanceMethods);
}

// Documented in header.
METH_IMPL_0(Object, gcMark) {
    ObjectInfo *info = getInfo(ths);

    datMark(info->data);
    return NULL;
}

/** Initializes the module. */
MOD_INIT(Object) {
    MOD_USE(Value);

    // Note: This does *not* inherit from `Core`, as this class is the
    // base for all non-core classes.
    CLS_Object = makeCoreClass(SYM(Object), CLS_Value,
        METH_TABLE(
            CMETH_BIND(Object, subclass)),
        METH_TABLE(
            METH_BIND(Object, gcMark)));
}

// Documented in header.
zvalue CLS_Object = NULL;
