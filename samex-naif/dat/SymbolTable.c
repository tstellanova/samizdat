// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

#include <stdarg.h>

#include "type/Builtin.h"
#include "type/Int.h"
#include "type/SymbolTable.h"
#include "type/define.h"
#include "util.h"

#include "impl.h"


//
// Private Definitions
//

/**
 * SymbolTable structure.
 */
typedef struct {
    /** Number of bindings in this table. */
    zint size;

    /**
     * Bindings from symbols to values, keyed off of symbol index number.
     */
    zvalue table[DAT_MAX_SYMBOLS];
} SymbolTableInfo;

/**
 * Gets a pointer to the value's info.
 */
static SymbolTableInfo *getInfo(zvalue symbolTable) {
    return datPayload(symbolTable);
}

/**
 * Allocates an instance.
 */
static zvalue allocInstance(void) {
    return datAllocValue(CLS_SymbolTable, sizeof(SymbolTableInfo));
}


//
// Exported Definitions
//

// Documented in header.
void xarrayFromSymbolTable(zvalue *result, zvalue symbolTable) {
    assertHasClass(symbolTable, CLS_SymbolTable);
    SymbolTableInfo *info = getInfo(symbolTable);

    utilCpy(zvalue, result, info->table, DAT_MAX_SYMBOLS);
}

// Documented in header.
void arrayFromSymbolTable(zmapping *result, zvalue symbolTable) {
    assertHasClass(symbolTable, CLS_SymbolTable);
    SymbolTableInfo *info = getInfo(symbolTable);
    zint size = info->size;

    for (zint i = 0, at = 0; i < DAT_MAX_SYMBOLS; i++) {
        zvalue one = info->table[i];
        if (one != NULL) {
            result[at].key = symbolFromIndex(i);
            result[at].value = one;
            at++;
        }
    }
}

// Documented in header.
zvalue symbolTableFromArgs(zvalue first, ...) {
    if (first == NULL) {
        return EMPTY_SYMBOL_TABLE;
    }

    zvalue result = allocInstance();
    SymbolTableInfo *info = getInfo(result);
    zint size = 0;
    va_list rest;

    va_start(rest, first);
    for (;;) {
        zvalue symbol = (size == 0) ? first : va_arg(rest, zvalue);

        if (symbol == NULL) {
            break;
        }

        zvalue value = va_arg(rest, zvalue);
        if (value == NULL) {
            die("Odd argument count for symbol table construction.");
        }

        zint index = symbolIndex(symbol);

        if (info->table[index] == NULL) {
            size++;
        }

        info->table[index] = value;
    }
    va_end(rest);

    info->size = size;
    return result;
}

// Documented in header.
zvalue symbolTableFromArray(zint size, zmapping *mappings) {
    if (size == 0) {
        return EMPTY_SYMBOL_TABLE;
    }

    zvalue result = allocInstance();
    SymbolTableInfo *info = getInfo(result);
    zint finalSize = 0;

    for (zint i = 0; i < size; i++) {
        zvalue key = mappings[i].key;
        zvalue value = mappings[i].value;
        zint index = symbolIndex(key);

        if (info->table[index] == NULL) {
            finalSize++;
        }

        info->table[index] = value;
    }

    info->size = finalSize;
    return result;
}

// Documented in header.
zint symbolTableSize(zvalue symbolTable) {
    assertHasClass(symbolTable, CLS_SymbolTable);
    return getInfo(symbolTable)->size;
}


//
// Class Definition
//

// Documented in header.
FUNC_IMPL_rest(SymbolTable_makeSymbolTable, args) {
    if ((argsSize & 1) != 0) {
        die("Odd argument count for symbol table construction.");
    }

    zint size = argsSize >> 1;
    zmapping mappings[size];

    for (zint i = 0, at = 0; i < size; i++, at += 2) {
        mappings[i].key = args[at];
        mappings[i].value = args[at + 1];
    }

    return symbolTableFromArray(size, mappings);
}

// Documented in header.
FUNC_IMPL_1_rest(SymbolTable_makeValueSymbolTable, first, args) {
    // Since the arguments are "stretchy" in the front instead of the
    // usual rear, we do a bit of non-obvious rearranging here.

    if (argsSize == 0) {
        return EMPTY_SYMBOL_TABLE;
    }

    zvalue value = args[argsSize - 1];
    zmapping mappings[argsSize];

    mappings[0].key = first;
    mappings[0].value = value;

    for (zint i = 1; i < argsSize; i++) {
        mappings[i].key = args[i - 1];
        mappings[i].value = value;
    }

    return symbolTableFromArray(argsSize, mappings);
}

// Documented in header.
METH_IMPL_0(SymbolTable, gcMark) {
    SymbolTableInfo *info = getInfo(ths);

    for (zint i = 0; i < DAT_MAX_SYMBOLS; i++) {
        datMark(info->table[i]);
    }

    return NULL;
}

// Documented in header.
METH_IMPL_1(SymbolTable, get, key) {
    zint index = symbolIndex(key);
    return getInfo(ths)->table[index];
}

// Documented in header.
METH_IMPL_0(SymbolTable, get_size) {
    return intFromZint(getInfo(ths)->size);
}

// Documented in header.
METH_IMPL_1(SymbolTable, totalEq, other) {
    // Note: `other` not guaranteed to be a `SymbolTable`.
    assertHasClass(other, CLS_SymbolTable);
    SymbolTableInfo *info1 = getInfo(ths);
    SymbolTableInfo *info2 = getInfo(other);

    for (zint i = 0; i < DAT_MAX_SYMBOLS; i++) {
        zvalue value1 = info1->table[i];
        zvalue value2 = info2->table[i];
        if (!valEqNullOk(value1, value2)) {
            return NULL;
        }
    }

    return ths;
}

/** Initializes the module. */
MOD_INIT(SymbolTable) {
    MOD_USE(Symbol);
    MOD_USE(OneOff);

    // Note: The `objectModel` module initializes `CLS_SymbolTable`.
    classBindMethods(CLS_SymbolTable,
        NULL,
        symbolTableFromArgs(
            METH_BIND(SymbolTable, gcMark),
            METH_BIND(SymbolTable, get),
            METH_BIND(SymbolTable, get_size),
            METH_BIND(SymbolTable, totalEq),
            NULL));

    FUN_SymbolTable_makeSymbolTable =
        datImmortalize(FUNC_VALUE(SymbolTable_makeSymbolTable));

    FUN_SymbolTable_makeValueSymbolTable =
        datImmortalize(FUNC_VALUE(SymbolTable_makeValueSymbolTable));

    EMPTY_SYMBOL_TABLE = datImmortalize(allocInstance());
}

// Documented in header.
zvalue CLS_SymbolTable = NULL;

// Documented in header.
zvalue EMPTY_SYMBOL_TABLE = NULL;

// Documented in header.
zvalue FUN_SymbolTable_makeSymbolTable;

// Documented in header.
zvalue FUN_SymbolTable_makeValueSymbolTable;
