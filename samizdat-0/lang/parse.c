/*
 * Copyright 2013 the Samizdat Authors (Dan Bornstein et alia).
 * Licensed AS IS and WITHOUT WARRANTY under the Apache License,
 * Version 2.0. See the associated file "LICENSE.md" for details.
 */

#include "consts.h"
#include "impl.h"
#include "util.h"

#include <stddef.h>

typedef struct {
    /** token list being parsed. */
    zvalue tokens;

    /** Size of `tokens`. */
    zint size;

    /** Current read position. */
    zint at;
} ParseState;


/*
 * Helper functions
 */

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

    zvalue result = datListletGet(state->tokens, state->at);
    state->at++;

    return result;
}

/**
 * Reads the next token if its type matches the given token's type.
 */
static zvalue readMatch(ParseState *state, zvalue token) {
    if (isEof(state)) {
        return NULL;
    }

    zvalue result = datListletGet(state->tokens, state->at);
    zvalue tokenType = hidType(token);
    zvalue resultType = hidType(result);

    if (datCompare(tokenType, resultType) != 0) {
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

/* Defined below. */
static zvalue parseAtom(ParseState *state);
static zvalue parseExpression(ParseState *state);
static zvalue parseStatements(ParseState *state);

/**
 * Parses `atom+`. Returns a listlet of parsed expressions.
 */
static zvalue parseAtomPlus(ParseState *state) {
    zvalue result = datListletEmpty();

    for (;;) {
        zvalue atom = parseAtom(state);
        if (atom == NULL) {
            break;
        }
        result = datListletAppend(result, atom);
    }

    if (datSize(result) == 0) {
        return NULL;
    }

    return result;
}

/**
 * Parses `@"(" @")"` as part of parsing a `call` node.
 */
static zvalue parseCall1(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_OPAREN) == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_CPAREN) == NULL) {
        reset(state, mark);
        return NULL;
    }

    return datListletEmpty();
}

/**
 * Parses a `call` node.
 */
static zvalue parseCall(ParseState *state) {
    zint mark = cursor(state);
    zvalue function = parseAtom(state);

    if (function == NULL) {
        return NULL;
    }

    zvalue actuals = parseCall1(state);

    if (actuals == NULL) {
        actuals = parseAtomPlus(state);
    }

    if (actuals == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue value = datMapletPut(datMapletEmpty(), STR_FUNCTION, function);
    value = datMapletPut(value, STR_ACTUALS, actuals);
    return hidPutValue(TOK_CALL, value);
}

/**
 * Parses a `formals` node.
 */
static zvalue parseFormals(ParseState *state) {
    zint mark = cursor(state);
    zvalue identifiers = datListletEmpty();

    for (;;) {
        zvalue identifier = readMatch(state, TOK_IDENTIFIER);
        if (identifier == NULL) {
            break;
        }
        identifier = hidValue(identifier);
        identifiers = datListletAppend(identifiers, identifier);
    }

    if (datSize(identifiers) != 0) {
        if (readMatch(state, TOK_CH_COLONCOLON) == NULL) {
            // We didn't find the expected `::` which means there
            // was no formals list at all. So reset the parse, but
            // still succeed with an empty formals list.
            reset(state, mark);
            identifiers = datListletEmpty();
        }
    }

    return hidPutValue(TOK_FORMALS, identifiers);
}

/**
 * Parses a `function` node.
 */
static zvalue parseFunction(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_OCURLY) == NULL) {
        return NULL;
    }

    // These always succeed.
    zvalue formals = parseFormals(state);
    zvalue statements = parseStatements(state);

    if (readMatch(state, TOK_CH_CCURLY) == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue value = datMapletPut(datMapletEmpty(), STR_FORMALS, formals);
    value = datMapletPut(value, STR_STATEMENTS, statements);
    return hidPutValue(TOK_FUNCTION, value);
}

/**
 * Parses a `uniqlet` node.
 */
static zvalue parseUniqlet(ParseState *state) {
    if (readMatch(state, TOK_CH_ATAT) == NULL) {
        return NULL;
    }

    return TOK_UNIQLET;
}

/**
 * Parses a `binding` node.
 */
static zvalue parseBinding(ParseState *state) {
    zint mark = cursor(state);
    zvalue key = parseAtom(state);

    if (key == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_EQUAL) == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue value = parseAtom(state);

    if (value == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue binding = datMapletPut(datMapletEmpty(), STR_KEY, key);
    return datMapletPut(binding, STR_VALUE, value);
}

/**
 * Parses a `maplet` node.
 */
static zvalue parseMaplet(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_AT) == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_OSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue bindings = datListletEmpty();

    for (;;) {
        zvalue binding = parseBinding(state);

        if (binding == NULL) {
            break;
        }

        bindings = datListletAppend(bindings, binding);
    }

    if (datSize(bindings) == 0) {
        reset(state, mark);
        return NULL;
    }

    if (readMatch(state, TOK_CH_CSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    return hidPutValue(TOK_MAPLET, bindings);
}

/**
 * Parses an `emptyMaplet` node.
 */
static zvalue parseEmptyMaplet(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_AT) == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_OSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    if (readMatch(state, TOK_CH_EQUAL) == NULL) {
        reset(state, mark);
        return NULL;
    }

    if (readMatch(state, TOK_CH_CSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    return hidPutValue(TOK_LITERAL, datMapletEmpty());
}

/**
 * Parses a `listlet` node.
 */
static zvalue parseListlet(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_AT) == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_OSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue atoms = parseAtomPlus(state);

    if (atoms == NULL) {
        reset(state, mark);
        return NULL;
    }

    if (readMatch(state, TOK_CH_CSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    return hidPutValue(TOK_LISTLET, atoms);
}

/**
 * Parses an `emptyListlet` node.
 */
static zvalue parseEmptyListlet(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_AT) == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_OSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    if (readMatch(state, TOK_CH_CSQUARE) == NULL) {
        reset(state, mark);
        return NULL;
    }

    return hidPutValue(TOK_LITERAL, datListletEmpty());
}

/**
 * Parses a `stringlet` node.
 */
static zvalue parseStringlet(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_AT) == NULL) {
        return NULL;
    }

    zvalue string = readMatch(state, TOK_STRING);

    if (string == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue value = hidValue(string);
    return hidPutValue(TOK_LITERAL, value);
}

/**
 * Parses an `integer` node.
 */
static zvalue parseInteger(ParseState *state) {
    zvalue integer = readMatch(state, TOK_INTEGER);

    if (integer == NULL) {
        return NULL;
    }

    return hidPutValue(TOK_LITERAL, integer);
}

/**
 * Parses an `intlet` node.
 */
static zvalue parseIntlet(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_AT) == NULL) {
        return NULL;
    }

    zvalue integer = readMatch(state, TOK_INTEGER);

    if (integer == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue value = hidValue(integer);
    return hidPutValue(TOK_LITERAL, value);
}

/**
 * Parses a `varRef` node.
 */
static zvalue parseVarRef(ParseState *state) {
    zvalue identifier = readMatch(state, TOK_IDENTIFIER);

    if (identifier == NULL) {
        return NULL;
    }

    zvalue name = hidValue(identifier);
    return hidPutValue(TOK_VAR_REF, name);
}

/**
 * Parses a `varDef` node.
 */
static zvalue parseVarDef(ParseState *state) {
    zint mark = cursor(state);
    zvalue identifier = readMatch(state, TOK_IDENTIFIER);

    if (identifier == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_EQUAL) == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue expression = parseExpression(state);

    if (expression == NULL) {
        reset(state, mark);
        return NULL;
    }

    zvalue name = hidValue(identifier);
    zvalue value = datMapletPut(datMapletEmpty(), STR_NAME, name);
    value = datMapletPut(value, STR_VALUE, expression);
    return hidPutValue(TOK_VAR_DEF, value);
}

/**
 * Parses a `parenExpression` node.
 */
static zvalue parseParenExpression(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_OPAREN) == NULL) {
        return NULL;
    }

    zvalue expression = parseExpression(state);

    if (expression == NULL) {
        reset(state, mark);
        return NULL;
    }

    if (readMatch(state, TOK_CH_CPAREN) == NULL) {
        reset(state, mark);
        return NULL;
    }

    return expression;
}

/**
 * Parses an `atom` node.
 */
static zvalue parseAtom(ParseState *state) {
    zvalue result = NULL;

    if (result == NULL) { result = parseVarRef(state); }
    if (result == NULL) { result = parseIntlet(state); }
    if (result == NULL) { result = parseInteger(state); }
    if (result == NULL) { result = parseStringlet(state); }
    if (result == NULL) { result = parseEmptyListlet(state); }
    if (result == NULL) { result = parseListlet(state); }
    if (result == NULL) { result = parseEmptyMaplet(state); }
    if (result == NULL) { result = parseMaplet(state); }
    if (result == NULL) { result = parseUniqlet(state); }
    if (result == NULL) { result = parseFunction(state); }
    if (result == NULL) { result = parseParenExpression(state); }

    return result;
}

/**
 * Parses an `expression` node.
 */
static zvalue parseExpression(ParseState *state) {
    zvalue result = NULL;

    if (result == NULL) { result = parseCall(state); }
    if (result == NULL) { result = parseAtom(state); }

    return result;
}

/**
 * Parses a `return` node.
 */
static zvalue parseReturn(ParseState *state) {
    zint mark = cursor(state);

    if (readMatch(state, TOK_CH_CARET) == NULL) {
        return NULL;
    }

    zvalue expression = parseExpression(state);

    if (expression == NULL) {
        reset(state, mark);
        return NULL;
    }

    return hidPutValue(TOK_RETURN, expression);
}

/**
 * Parses a `statement` node.
 */
static zvalue parseStatement(ParseState *state) {
    zint mark = cursor(state);
    zvalue result = NULL;

    if (result == NULL) { result = parseVarDef(state); }
    if (result == NULL) { result = parseExpression(state); }
    if (result == NULL) { result = parseReturn(state); }

    if (result == NULL) {
        return NULL;
    }

    if (readMatch(state, TOK_CH_SEMICOLON)) {
        return result;
    } else {
        reset(state, mark);
        return NULL;
    }
}

/**
 * Parses a `statements` node.
 */
static zvalue parseStatements(ParseState *state) {
    zvalue result = datListletEmpty();

    for (;;) {
        zvalue statement = parseStatement(state);
        if (statement == NULL) {
            break;
        }
        result = datListletAppend(result, statement);
    }

    return hidPutValue(TOK_STATEMENTS, result);
}


/*
 * Module functions
 */

/* Documented in header. */
zvalue parse(zvalue tokens) {
    ParseState state = { tokens, datSize(tokens), 0 };
    zvalue result = parseStatements(&state);

    if (!isEof(&state)) {
        die("Extra tokens at end of program.");
    }

    return result;
}
