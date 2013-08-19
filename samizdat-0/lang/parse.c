/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. Details: <http://www.apache.org/licenses/LICENSE-2.0>
 */

#include "const.h"
#include "impl.h"
#include "type/List.h"
#include "type/Map.h"
#include "type/String.h"
#include "type/Type.h"
#include "type/Value.h"
#include "util.h"


/*
 * ParseState definition and functions
 */

/** State of parsing in-progress. */
typedef struct {
    /** token list being parsed. */
    zvalue tokens;

    /** Size of `tokens`. */
    zint size;

    /** Current read position. */
    zint at;
} ParseState;

/**
 * Is the parse state at EOF?
 */
static bool isEof(ParseState *state) {
    return (state->at >= state->size);
}

/**
 * Reads the next token.
 */
static zvalue read(ParseState *state) {
    if (isEof(state)) {
        return NULL;
    }

    zvalue result = listNth(state->tokens, state->at);
    state->at++;

    return result;
}

/**
 * Reads the next token if its type matches the given type.
 */
static zvalue readMatch(ParseState *state, zvalue type) {
    if (isEof(state)) {
        return NULL;
    }

    zvalue result = listNth(state->tokens, state->at);
    zvalue resultType = typeOf(result);

    if (!valEq(type, resultType)) {
        return NULL;
    }

    state->at++;

    return result;
}

/**
 * Gets the current read position.
 */
static zint cursor(ParseState *state) {
    return state->at;
}

/**
 * Resets the current read position to the given one.
 */
static void reset(ParseState *state, zint mark) {
    if (mark > state->at) {
        die("Cannot reset forward: %lld > %lld", mark, state->at);
    }

    state->at = mark;
}

/**
 * Peeks at the next token, checking against the given type.
 */
static zvalue peekMatch(ParseState *state, zvalue type) {
    zint mark = cursor(state);
    zvalue result = readMatch(state, type);

    if (result == NULL) {
        return NULL;
    }

    reset(state, mark);
    return result;
}


/*
 * Node constructors and related helpers
 */

/**
 * Makes a 0-3 mapping map. Values are allowed to be `NULL`, in
 * which case the corresponding key isn't included in the result.
 */
static zvalue mapFrom3(zvalue k1, zvalue v1, zvalue k2, zvalue v2,
                       zvalue k3, zvalue v3) {
    zvalue result = EMPTY_MAP;

    if (v1 != NULL) { result = mapPut(result, k1, v1); }
    if (v2 != NULL) { result = mapPut(result, k2, v2); }
    if (v3 != NULL) { result = mapPut(result, k3, v3); }

    return result;
}

/**
 * Makes a 0-2 mapping map.
 */
static zvalue mapFrom2(zvalue k1, zvalue v1, zvalue k2, zvalue v2) {
    return mapFrom3(k1, v1, k2, v2, NULL, NULL);
}

/**
 * Makes a 0-1 mapping map.
 */
static zvalue mapFrom1(zvalue k1, zvalue v1) {
    return mapFrom3(k1, v1, NULL, NULL, NULL, NULL);
}

/**
 * Makes a 1 element list.
 */
static zvalue listFrom1(zvalue e1) {
    return listFromArray(1, &e1);
}

/**
 * Makes a 2 element list.
 */
static zvalue listFrom2(zvalue e1, zvalue e2) {
    zvalue elems[2] = { e1, e2 };
    return listFromArray(2, elems);
}

/**
 * Appends an element to a list.
 */
static zvalue listAppend(zvalue list, zvalue elem) {
    return listCat(list, listFrom1(elem));
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeInterpolate(zvalue expression) {
    return makeValue(STR_interpolate, expression);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeLiteral(zvalue value) {
    return makeValue(STR_literal, value);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeThunk(zvalue expression) {
    zvalue value = mapFrom3(
        STR_formals, EMPTY_LIST,
        STR_statements, EMPTY_LIST,
        STR_yield, expression);
    return makeValue(STR_closure, value);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeVarDef(zvalue name, zvalue value) {
    zvalue payload = mapFrom2(STR_name, name, STR_value, value);
    return makeValue(STR_varDef, payload);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeVarRef(zvalue name) {
    return makeValue(STR_varRef, name);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeCall(zvalue function, zvalue actuals) {
    if (actuals == NULL) {
        actuals = EMPTY_LIST;
    }

    zvalue value = mapFrom2(STR_function, function, STR_actuals, actuals);
    return makeValue(STR_call, value);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeCallName(zvalue name, zvalue actuals) {
    return makeCall(makeVarRef(name), actuals);
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeOptValueExpression(zvalue expression) {
    return makeCallName(STR_optValue, listFrom1(makeThunk(expression)));
}

/* Documented in Samizdat Layer 0 spec. */
static zvalue makeCallNonlocalExit(zvalue name, zvalue optExpression) {
    zvalue actuals;

    if (optExpression != NULL) {
        actuals = listFrom2(name,
            makeInterpolate(makeOptValueExpression(optExpression)));
    } else {
        actuals = listFrom1(name);
    }

    return makeCallName(STR_nonlocalExit, actuals);
}


/*
 * Parsing helper functions
 */

/* Definitions to help avoid boilerplate in the parser functions. */
#define RULE(name) parse_##name
#define DEF_PARSE(name) static zvalue RULE(name)(ParseState *state)
#define PARSE(name) RULE(name)(state)
#define PARSE_STAR(name) parseStar(RULE(name), state)
#define PARSE_PLUS(name) parsePlus(RULE(name), state)
#define PARSE_COMMA_SEQ(name) parseCommaSequence(RULE(name), state)
#define MATCH(typeOf) readMatch(state, (STR_##typeOf))
#define PEEK(typeOf) peekMatch(state, (STR_##typeOf))
#define MARK() zint mark = cursor(state); zvalue tempResult
#define RESET() do { reset(state, mark); } while (0)
#define REJECT() do { RESET(); return NULL; } while (0)
#define REJECT_IF(condition) \
    do { if ((condition)) REJECT(); } while (0)
#define MATCH_OR_REJECT(typeOf) \
    tempResult = MATCH(typeOf); \
    REJECT_IF(tempResult == NULL)
#define PARSE_OR_REJECT(name) \
    tempResult = PARSE(name); \
    REJECT_IF(tempResult == NULL)

/* Function prototype for all parser functions */
typedef zvalue (*parserFunction)(ParseState *);

/**
 * Parses `x*` for an arbitrary rule `x`. Returns a list of parsed `x` results.
 */
zvalue parseStar(parserFunction rule, ParseState *state) {
    zvalue result = EMPTY_LIST;

    for (;;) {
        zvalue one = rule(state);
        if (one == NULL) {
            break;
        }

        result = listAppend(result, one);
    }

    return result;
}

/**
 * Parses `x+` for an arbitrary rule `x`. Returns a list of parsed `x` results.
 */
zvalue parsePlus(parserFunction rule, ParseState *state) {
    MARK();

    zvalue result = parseStar(rule, state);
    REJECT_IF(valSize(result) == 0);

    return result;
}

/**
 * Parses `(x (@"," x)*)?` for an arbitrary rule `x`. Returns a list of
 * parsed `x` results.
 */
zvalue parseCommaSequence(parserFunction rule, ParseState *state) {
    zvalue item = rule(state);

    if (item == NULL) {
        return EMPTY_LIST;
    }

    zvalue result = listFrom1(item);

    for (;;) {
        MARK();

        if (! MATCH(CH_COMMA)) {
            break;
        }

        item = rule(state);
        if (item == NULL) {
            RESET();
            break;
        }

        result = listAppend(result, item);
    }

    return result;
}

/**
 * Parses zero or more semicolons. Always returns `NULL`.
 */
DEF_PARSE(optSemicolons) {
    while (MATCH(CH_SEMICOLON) != NULL) /* empty */ ;
    return NULL;
}


/*
 * Samizdat 0 Tree Grammar
 *
 * The following is a near-direct transliterations of the tree syntax
 * rules from the Samizdat Layer 0 spec.
 */

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(programBody);

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(expression);

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(voidableExpression);

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(yieldDef) {
    MARK();

    MATCH_OR_REJECT(CH_LT);
    zvalue identifier = MATCH_OR_REJECT(identifier);
    MATCH_OR_REJECT(CH_GT);

    return dataOf(identifier);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(optYieldDef) {
    zvalue result = PARSE(yieldDef);
    return (result != NULL) ? mapFrom1(STR_yieldDef, result) : EMPTY_MAP;
}

/**
 * Helper for `formal`: Parses `[@"?" @"*" @"+"]?`. Returns either the
 * parsed token payload or `NULL` to indicate that no alternate matched.
 */
DEF_PARSE(formal1) {
    MARK();

    zvalue result = NULL;

    if (result == NULL) { result = MATCH(CH_QMARK); }
    if (result == NULL) { result = MATCH(CH_STAR); }
    if (result == NULL) { result = MATCH(CH_PLUS); }

    REJECT_IF(result == NULL);

    return typeOf(result);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(formal) {
    MARK();

    zvalue name = MATCH(identifier);

    if (name != NULL) {
        name = dataOf(name);
    } else {
        // If there was no identifier, then the only valid form for a formal
        // is if this is an unnamed / unused argument.
        MATCH_OR_REJECT(CH_DOT);
    }

    zvalue repeat = PARSE(formal1); // Okay for it to be `NULL`.

    return mapFrom2(STR_name, name, STR_repeat, repeat);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(formalsList) {
    return PARSE_COMMA_SEQ(formal);
}

/**
 * Helper for `programDeclarations`: Parses the main part of the syntax.
 */
DEF_PARSE(programDeclarations1) {
    MARK();

    // Both of these are always maps (possibly empty).
    zvalue yieldDef = PARSE(optYieldDef);
    zvalue formals = PARSE(formalsList);

    if (PEEK(CH_DIAMOND) == NULL) {
        MATCH_OR_REJECT(CH_COLONCOLON);
    }

    return mapCat(mapFrom1(STR_formals, formals), yieldDef);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(programDeclarations) {
    zvalue result = NULL;

    if (result == NULL) { result = PARSE(programDeclarations1); }
    if (result == NULL) { result = mapFrom1(STR_formals, EMPTY_LIST); }

    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(program) {
    zvalue declarations = PARSE(programDeclarations); // This never fails.
    zvalue body = PARSE(programBody); // This never fails.

    return makeValue(STR_closure, mapCat(declarations, body));
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(closure) {
    MARK();

    MATCH_OR_REJECT(CH_OCURLY);

    // This always succeeds. See note in `parseProgram` above.
    zvalue result = PARSE(program);

    MATCH_OR_REJECT(CH_CCURLY);

    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(nullaryClosure) {
    MARK();

    zvalue c = PARSE_OR_REJECT(closure);

    zvalue formals = mapGet(dataOf(c), STR_formals);
    if (!valEq(formals, EMPTY_LIST)) {
        die("Invalid formal argument in code block.");
    }

    return c;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(codeOnlyClosure) {
    MARK();

    zvalue c = PARSE_OR_REJECT(nullaryClosure);

    if (mapGet(dataOf(c), STR_yieldDef) != NULL) {
        die("Invalid yield definition in code block.");
    }

    return c;
}

/**
 * Helper for `fnCommon`: Parses `(yieldDef)?` with variable definition
 * and list wrapping.
 */
DEF_PARSE(fnCommon1) {
    zvalue result = PARSE(yieldDef);

    if (result == NULL) {
        return EMPTY_LIST;
    }

    return listFrom1(makeVarDef(result, makeVarRef(STR_return)));
}

/**
 * Helper for `fnCommon`: Parses `(@identifier | [:])` with appropriate map
 * wrapping.
 */
DEF_PARSE(fnCommon2) {
    zvalue n = MATCH(identifier);

    if (n == NULL) {
        return EMPTY_MAP;
    }

    return mapFrom1(STR_name, dataOf(n));
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(fnCommon) {
    MARK();

    MATCH_OR_REJECT(fn);

    zvalue returnDef = PARSE(fnCommon1); // This never fails.
    zvalue name = PARSE(fnCommon2); // This never fails.
    MATCH_OR_REJECT(CH_OPAREN);
    zvalue formals = PARSE(formalsList); // This never fails.
    MATCH_OR_REJECT(CH_CPAREN);
    zvalue code = PARSE_OR_REJECT(codeOnlyClosure);

    zvalue codeMap = dataOf(code);
    zvalue statements =
        listCat(returnDef, mapGet(codeMap, STR_statements));

    zvalue result = mapCat(codeMap, name);
    result = mapCat(result,
        mapFrom3(
            STR_formals,    formals,
            STR_yieldDef,   STR_return,
            STR_statements, statements));
    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(fnDef) {
    MARK();

    zvalue funcMap = PARSE_OR_REJECT(fnCommon);

    if (mapGet(funcMap, STR_name) == NULL) {
        return NULL;
    }

    return makeValue(STR_fnDef, funcMap);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(fnExpression) {
    MARK();

    zvalue funcMap = PARSE_OR_REJECT(fnCommon);
    zvalue closure = makeValue(STR_closure, funcMap);

    zvalue name = mapGet(funcMap, STR_name);
    if (name == NULL) {
        return closure;
    }

    zvalue mainClosure = makeValue(
        STR_closure,
        mapFrom3(
            STR_formals,    EMPTY_LIST,
            STR_statements, listFrom1(makeValue(STR_fnDef, funcMap)),
            STR_yield,      makeVarRef(name)));

    return makeCall(mainClosure, NULL);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(int) {
    MARK();

    zvalue intval = MATCH_OR_REJECT(int);

    return makeLiteral(dataOf(intval));
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(string) {
    MARK();

    zvalue string = MATCH_OR_REJECT(string);

    return makeLiteral(dataOf(string));
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(identifierString) {
    zvalue result = NULL;

    if (result == NULL) { result = MATCH(string); }
    if (result == NULL) { result = MATCH(identifier); }
    if (result == NULL) { result = MATCH(def); }
    if (result == NULL) { result = MATCH(fn); }
    if (result == NULL) { result = MATCH(return); }
    if (result == NULL) { return NULL; }

    zvalue value = dataOf(result);
    if (value == NULL) {
        value = typeOf(result);
    }

    return makeLiteral(value);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(unadornedList) {
    return PARSE_COMMA_SEQ(voidableExpression);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(list) {
    MARK();

    MATCH_OR_REJECT(CH_OSQUARE);
    zvalue expressions = PARSE(unadornedList);
    MATCH_OR_REJECT(CH_CSQUARE);

    if (valSize(expressions) == 0) {
        return makeLiteral(EMPTY_LIST);
    } else {
        return makeCallName(STR_makeList, expressions);
    }
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(emptyMap) {
    MARK();

    MATCH_OR_REJECT(CH_OSQUARE);
    MATCH_OR_REJECT(CH_COLON);
    MATCH_OR_REJECT(CH_CSQUARE);

    return makeLiteral(EMPTY_MAP);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(keyAtom) {
    MARK();

    zvalue k = PARSE(identifierString);

    if (k != NULL) {
        if ((PEEK(CH_COLON) != NULL) || (PEEK(CH_CSQUARE) != NULL)) {
            return k;
        }
        RESET();
    }

    return PARSE(expression);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(mapKey) {
    return PARSE(keyAtom);
}

/**
 * Helper for `mapping`: Parses `mapKey @":" expression`.
 */
DEF_PARSE(mapping1) {
    MARK();

    zvalue key = PARSE_OR_REJECT(mapKey);
    MATCH_OR_REJECT(CH_COLON);
    zvalue value = PARSE_OR_REJECT(expression);

    return makeCallName(STR_makeValueMap,
        listFrom2(key, makeValue(STR_expression, value)));
}

/**
 * Helper for `mapping`: Parses a guaranteed-interpolate `expression`.
 */
DEF_PARSE(mapping2) {
    MARK();

    zvalue map = PARSE_OR_REJECT(expression);
    REJECT_IF(!hasType(map, STR_interpolate));

    return dataOf(map);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(mapping) {
    zvalue result = NULL;

    if (result == NULL) { result = PARSE(mapping1); }
    if (result == NULL) { result = PARSE(mapping2); }

    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(map) {
    MARK();

    MATCH_OR_REJECT(CH_OSQUARE);

    if (MATCH(CH_COLON)) {
        MATCH_OR_REJECT(CH_COMMA);
    }

    zvalue mappings = PARSE_COMMA_SEQ(mapping);
    zint size = valSize(mappings);
    REJECT_IF(size == 0);
    MATCH_OR_REJECT(CH_CSQUARE);

    return makeCallName(STR_mapCat, mappings);
}

/**
 * Helper for `deriv`: Parses `@"[" keyAtom (@":" expression)? @"]"`.
 */
DEF_PARSE(deriv1) {
    MARK();

    MATCH_OR_REJECT(CH_OSQUARE);

    zvalue type = PARSE_OR_REJECT(keyAtom);
    zvalue result;

    if (MATCH(CH_COLON)) {
        // Note: Strictly speaking this doesn't quite follow the spec.
        // However, there is no meaningful difference, in that the only
        // difference is *how* errors are recognized, not *whether* they
        // are.
        zvalue value = PARSE_OR_REJECT(expression);
        result = listFrom2(type, value);
    } else {
        result = listFrom1(type);
    }

    MATCH_OR_REJECT(CH_CSQUARE);

    return result;
}

/**
 * Helper for `deriv`: Parses `identifierString` returning a list of the
 * parsed value if successful.
 */
DEF_PARSE(deriv2) {
    MARK();

    zvalue result = PARSE_OR_REJECT(identifierString);

    return listFrom1(result);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(deriv) {
    MARK();

    MATCH_OR_REJECT(CH_AT);

    zvalue args = NULL;
    if (args == NULL) { args = PARSE(deriv1); }
    if (args == NULL) { args = PARSE(deriv2); }
    REJECT_IF(args == NULL);

    return makeCallName(STR_makeValue, args);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(varRef) {
    MARK();

    zvalue identifier = MATCH_OR_REJECT(identifier);

    return makeVarRef(dataOf(identifier));
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(varDef) {
    MARK();

    MATCH_OR_REJECT(def);
    zvalue identifier = MATCH_OR_REJECT(identifier);
    MATCH_OR_REJECT(CH_EQUAL);
    zvalue expression = PARSE_OR_REJECT(expression);

    zvalue name = dataOf(identifier);
    return makeVarDef(name, expression);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(parenExpression) {
    MARK();

    MATCH_OR_REJECT(CH_OPAREN);
    zvalue expression = PARSE_OR_REJECT(expression);
    MATCH_OR_REJECT(CH_CPAREN);

    return makeValue(STR_expression, expression);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(atom) {
    zvalue result = NULL;

    if (result == NULL) { result = PARSE(varRef); }
    if (result == NULL) { result = PARSE(int); }
    if (result == NULL) { result = PARSE(string); }
    if (result == NULL) { result = PARSE(list); }
    if (result == NULL) { result = PARSE(emptyMap); }
    if (result == NULL) { result = PARSE(map); }
    if (result == NULL) { result = PARSE(deriv); }
    if (result == NULL) { result = PARSE(closure); }
    if (result == NULL) { result = PARSE(parenExpression); }

    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(actualsList) {
    MARK();

    if (MATCH(CH_OPAREN)) {
        zvalue normalActuals = PARSE(unadornedList); // This never fails.
        MATCH_OR_REJECT(CH_CPAREN);
        zvalue closureActuals = PARSE_STAR(closure); // This never fails.
        return listCat(closureActuals, normalActuals);
    }

    return PARSE_PLUS(closure);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(prefixOperator) {
    // We differ from the spec here, merely returning a token if matched.
    // The corresponding `unaryExpression` code just notes the number of
    // minuses matched in order to make the right number of calls.
    return MATCH(CH_MINUS);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(postfixOperator) {
    // We differ from the spec here, returning an actuals list directly
    // or a `*` token. The corresponding `unaryExpression` code decodes
    // these as appropriate.

    zvalue result = NULL;
    if (result == NULL) { result = PARSE(actualsList); }
    if (result == NULL) { result = MATCH(CH_STAR); }
    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(unaryExpression) {
    MARK();

    zvalue prefixes = PARSE_STAR(prefixOperator);
    zvalue result = PARSE_OR_REJECT(atom);
    zvalue postfixes = PARSE_STAR(postfixOperator);

    zint size = valSize(postfixes);
    for (zint i = 0; i < size; i++) {
        zvalue one = listNth(postfixes, i);
        if (hasType(one, TYPE_List)) {
            result = makeCall(result, one);
        } else if (valEq(one, TOK_CH_STAR)) {
            result = makeInterpolate(result);
        } else {
            die("Unexpected postfix.");
        }
    }

    for (zint i = valSize(prefixes) - 1; i >= 0; i--) {
        zvalue one = listNth(prefixes, i);
        if (valEq(one, TOK_CH_MINUS)) {
            result = makeCallName(STR_ineg, listFrom1(result));
        } else {
            die("Unexpected prefix.");
        }
    }

    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(voidableExpression) {
    MARK();

    bool voidable = (MATCH(CH_AND) != NULL);
    zvalue ex = PARSE_OR_REJECT(unaryExpression);

    if (voidable) {
        return makeValue(STR_voidable, ex);
    } else {
        return ex;
    }
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(expression) {
    zstackPointer save = pbFrameStart();
    zvalue result = NULL;

    if (result == NULL) { result = PARSE(unaryExpression); }
    if (result == NULL) { result = PARSE(fnExpression); }

    pbFrameReturn(save, result);
    return result;
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(statement) {
    zstackPointer save = pbFrameStart();
    zvalue result = NULL;

    if (result == NULL) { result = PARSE(varDef); }
    if (result == NULL) { result = PARSE(fnDef); }
    if (result == NULL) { result = PARSE(expression); }

    pbFrameReturn(save, result);
    return result;
}

/**
 * Documented in Samizdat Layer 0 spec. This implementation differs from the
 * spec in that it will return `NULL` either if no diamond is present
 * or if it is a void yield. This is compensated for by matching changes to
 * the implementation of `programBody`, below.
 */
DEF_PARSE(yield) {
    MARK();

    MATCH_OR_REJECT(CH_DIAMOND);
    return PARSE(expression);
}

/**
 * Helper for `nonlocalExit`: Parses `@"<" varRef @">"`.
 */
DEF_PARSE(nonlocalExit1) {
    MARK();

    MATCH_OR_REJECT(CH_LT);
    zvalue name = PARSE_OR_REJECT(varRef);
    MATCH_OR_REJECT(CH_GT);

    return name;
}

/**
 * Helper for `nonlocalExit`: Parses `@"return"`.
 */
DEF_PARSE(nonlocalExit2) {
    MARK();

    MATCH_OR_REJECT(return);
    return makeVarRef(STR_return);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(nonlocalExit) {
    zvalue name = NULL;

    if (name == NULL) { name = PARSE(nonlocalExit1); }
    if (name == NULL) { name = PARSE(nonlocalExit2); }
    if (name == NULL) { return NULL; }

    zvalue value = PARSE(expression); // It's okay for this to be `NULL`.
    return makeCallNonlocalExit(name, value);
}

/* Documented in Samizdat Layer 0 spec. */
DEF_PARSE(programBody) {
    zvalue statements = EMPTY_LIST;
    zvalue yield = NULL; // `NULL` is ok, as it's optional.

    PARSE(optSemicolons);

    for (;;) {
        MARK();

        zvalue statement = PARSE(statement);
        if (statement == NULL) {
            break;
        }

        if (MATCH(CH_SEMICOLON) == NULL) {
            RESET();
            break;
        }

        PARSE(optSemicolons);
        statements = listAppend(statements, statement);
    }

    zvalue statement = PARSE(statement);

    if (statement == NULL) {
        statement = PARSE(nonlocalExit);
    }

    if (statement != NULL) {
        statements = listAppend(statements, statement);
    } else {
        yield = PARSE(yield);
    }

    PARSE(optSemicolons);

    return mapFrom2(STR_statements, statements, STR_yield, yield);
}


/*
 * Exported Definitions
 */

/* Documented in header. */
zvalue langTree0(zvalue program) {
    langInit();

    zvalue tokens;

    if (hasType(program, TYPE_String)) {
        tokens = langTokenize0(program);
    } else {
        tokens = program;
    }

    ParseState state = { tokens, valSize(tokens), 0 };
    zvalue result = parse_program(&state);

    if (!isEof(&state)) {
        die("Extra tokens at end of program.");
    }

    return result;
}
