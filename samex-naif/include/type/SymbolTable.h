// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// `SymbolTable` class
//

#ifndef _TYPE_SYMBOL_TABLE_H_
#define _TYPE_SYMBOL_TABLE_H_

#include "type/Value.h"

/** Class value for in-model class `SymbolTable`. */
extern zvalue CLS_SymbolTable;

/** The standard empty symbol table value. */
extern zvalue EMPTY_SYMBOL_TABLE;


/**
 * Copies all the values of the given symbol table into the given result
 * array, which must be sized large enough to hold all of them. The result
 * has no particular ordering.
 */
void arrayFromSymtab(zmapping *result, zvalue symtab);

/**
 * Gets a single-mapping symbol table of the given mapping.
 */
zvalue symtabFromMapping(zmapping mapping);

/**
 * Makes a symbol table from a `zassoc`. The keys must all be symbols (of
 * course).
 */
zvalue symtabFromZassoc(zassoc ass);

/**
 * Gets the size of a symbol table.
 */
zint symtabSize(zvalue symtab);

/**
 * Returns a new symbol table with the given additional mapping. This fails
 * and returns `NULL` if the original table already has a mapping for the
 * indicated symbol.
 */
zvalue symtabWithNewMapping(zvalue symtab, zmapping mapping);

#endif
