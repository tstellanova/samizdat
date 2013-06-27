Samizdat Language Guide
=======================

Appendix: *Samizdat 0* Implementation Restrictions
--------------------------------------------------

*Samizdat 0* is the system in which the full Samizdat implementation is
written. It implements several "layers" of language, with *Samizdat Layer 0*
being the most basic language, and each subsequent layer being (practically
speaking) a strict superset of the layer that it is build directly upon.

This document describes how the various layers of *Samizdat 0* differ
from the full language.

### The Layers

#### Layer 0

*Samizdat Layer 0* a "parti" of the final language layer. That is, it
embodies *just* the main thrusts of the language with very little
embellishment.

The goal is that code written in this layer be recognizably Samizdat,
even while eschewing such niceties as operators and control constructs.

#### Layer 1

The sole purpose of *Samizdat Layer 1* is to introduce parsing syntax
into the language.

#### Layer 2

*Samizdat Layer 2* adds a more complete set of syntactic constructs to
the main language, including functional operators (e.g. math operations),
control constructs, and a bit more variety in expressing literal data.

### The Restrictions

#### Ints

Ints only have a 32-bit signed range, with out-of-range arithmetic
results causing failure, not wraparound.

In the surface syntax, base 10 is the only recognized form for int
constants in Layer 0. Layer 2 introduces syntax for hexadecimal and
binary int constants.

In the Layer 0 surface syntax, underscores are not recognized as
digit spacers in int literals. Layer 2 introduces this syntax.

#### Strings

In Layer 0, the only backslash escapes that are recognized are the
trivial one-character ones. *Not* included are hexadecimal escapes,
entity escapes, `\/`, or the ignoring of newlines. In addition,
space-after-newline collapsing is *not* performed.

Handling of all of these is implemented in Layer 2.

#### Maps

In Layer 0, parenthesized comma-separated lists of keys are not
recognized. This is implemented in Layer 2.

#### Variable Definition

Only immutable variable definitions (`def name = ...`) are recognized in
Layer 0. TODO: Mutable variable definitions are implemented in Layer 2.

#### Parsing

Parsing syntax (parsing blocks and parsing operators) is not recognized at
all in Layer 0. This is implemented in Layer 1.

#### Operators

The only operators regognized in Layer 0 are:

* `-expr` &mdash; Unary negation, strictly limited to operating on ints.
* `expr*` &mdash; Interpolation.
* `expr..expr` &mdash; Ranges, but not range-with-increment (`x..y..z`),
  nor limit-exclusive ranges (`x..!y` or `x..y..!z`).
* `expr(expr, ...) { block } ...` &mdash; Function calls.
* `<> expr` &mdash; Local yield.
* `<out> exr` &mdash; Named nonlocal return.
* `return expr` &mdash; Function return.

Parsing expression operator syntax is implemented in Layer 1.
Full expression operator syntax is implemented in Layer 2.

#### Control Constructs

No control expressions are recognized (`if`, `do`, `while`, etc.) in Layer 0.
These are implemented in Layer 2.

**Note:** `fn` statements and expressions *are* recognized in Layer 0.
