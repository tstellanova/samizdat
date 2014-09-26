// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

#include <stdlib.h>

#include "type/Collection.h"
#include "type/Int.h"
#include "type/Record.h"
#include "type/SymbolTable.h"
#include "type/define.h"

#include "impl.h"


//
// Private Definitions
//

/** Array mapping symbol indices to record classes. */
static zvalue theClasses[DAT_MAX_SYMBOLS];


/**
 * Payload data for all records.
 */
typedef struct {
    /** Record name. */
    zvalue name;

    /** Name's symbol index. */
    zint nameIndex;

    /** Data payload. */
    zvalue data;
} RecordInfo;

/**
 * Gets the info of a record.
 */
static RecordInfo *getInfo(zvalue value) {
    return (RecordInfo *) datPayload(value);
}

/** Vestigial record class creator. */
static zvalue makeRecordClass(zvalue name) {
    // `symbolIndex` will bail if `name` isn't a symbol.
    zint index = symbolIndex(name);
    zvalue result = theClasses[index];

    if (result != NULL) {
        return result;
    }

    result = makeClass(name, CLS_Record, NULL, NULL, NULL);
    theClasses[index] = result;

    return result;
}


//
// Exported Definitions
//

// Documented in header.
zvalue dataOf(zvalue value) {
    return METH_CALL(dataOf, value);
}

// Documented in header.
zvalue makeRecord(zvalue clsOrName, zvalue data) {
    zvalue cls;
    zvalue name;

    if (hasClass(clsOrName, CLS_Symbol)) {
        name = clsOrName;
        cls = makeRecordClass(name);
    } else {
        if (!classHasParent(clsOrName, CLS_Record)) {
            die("Attempt to call `makeRecord` on an improper class.");
        }
        cls = clsOrName;
        name = get_name(cls);
    }

    if (data == NULL) {
        data = EMPTY_SYMBOL_TABLE;
    } else {
        assertHasClass(data, CLS_SymbolTable);
    }

    zvalue result = datAllocValue(cls, sizeof(RecordInfo));
    RecordInfo *info = getInfo(result);

    info->name = name;
    info->nameIndex = symbolIndex(name);
    info->data = data;

    return result;
}

// Documented in header.
bool recHasName(zvalue record, zvalue name) {
    assertHasClass(record, CLS_Record);
    return symbolEq(getInfo(record)->name, name);
}

// Documented in header.
zint recNameIndex(zvalue record) {
    assertHasClass(record, CLS_Record);
    return getInfo(record)->nameIndex;
}


//
// Class Definition
//

// Documented in header.
FUNC_IMPL_1_2(Record_makeRecord, cls, value) {
    return makeRecord(cls, value);
}

// Documented in header.
METH_IMPL_0(Record, dataOf) {
    // The `datFrameAdd()` call is because `value` might immediately become
    // garbage.
    return datFrameAdd(getInfo(ths)->data);
}

// Documented in header.
METH_IMPL_0(Record, gcMark) {
    RecordInfo *info = getInfo(ths);

    datMark(info->name);
    datMark(info->data);
    return NULL;
}

// Documented in header.
METH_IMPL_1(Record, get, key) {
    return get(getInfo(ths)->data, key);
}

// Documented in header.
METH_IMPL_0(Record, get_name) {
    return getInfo(ths)->name;
}

// Documented in header.
METH_IMPL_1(Record, hasName, name) {
    return symbolEq(getInfo(ths)->name, name) ? ths : NULL;
}

// Documented in header.
METH_IMPL_1(Record, totalEq, other) {
    // Note: `other` not guaranteed to be of same class.

    if (!haveSameClass(ths, other)) {
        die("`totalEq` called with incompatible arguments.");
    }

    return valEqNullOk(getInfo(ths)->data, getInfo(other)->data)
        ? ths : NULL;
}

// Documented in header.
METH_IMPL_1(Record, totalOrder, other) {
    // Note: `other` not guaranteed to be of same class.

    if (!haveSameClass(ths, other)) {
        die("`totalOrder` called with incompatible arguments.");
    }

    return valOrderNullOk(getInfo(ths)->data, getInfo(other)->data);
}

/** Initializes the module. */
MOD_INIT(Record) {
    MOD_USE(Data);

    SYM_INIT(dataOf);

    CLS_Record = makeCoreClass("Record", CLS_Data,
        NULL,
        symbolTableFromArgs(
            METH_BIND(Record, dataOf),
            METH_BIND(Record, gcMark),
            METH_BIND(Record, get),
            METH_BIND(Record, get_name),
            METH_BIND(Record, hasName),
            METH_BIND(Record, totalEq),
            METH_BIND(Record, totalOrder),
            NULL));

    FUN_Record_makeRecord =
        datImmortalize(FUNC_VALUE(Record_makeRecord));
}

// Documented in header.
zvalue CLS_Record = NULL;

// Documented in header.
SYM_DEF(dataOf);

// Documented in header.
zvalue FUN_Record_makeRecord = NULL;
