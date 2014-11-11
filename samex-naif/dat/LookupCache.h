// Copyright 2013-2014 the Samizdat Authors (Dan Bornstein et alia).
// Licensed AS IS and WITHOUT WARRANTY under the Apache License,
// Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>

//
// `LookupCache` helper class
//

#ifndef _LOOKUP_CACHE_H_
#define _LOOKUP_CACHE_H_

#include "dat.h"


/**
 * Entry in the cache. The cache is used to speed up key lookups in
 * containers (maps, tables, etc.). See `mapFind` for details.
 */
typedef struct {
    /** Map/table to look up a key in. */
    zvalue container;

    /** Key to look up. */
    zvalue key;

    /**
     * Index into the elements of the container where `key` is found, or
     * (if negative) ones-complement of the insertion point.
     */
    zint index;
} LookupCacheEntry;

/**
 * Gets the `LookupCacheEntry` for the given container/key pair.
 */
LookupCacheEntry *lookupCacheFind(zvalue container, zvalue key);

#endif