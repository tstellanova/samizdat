Samizdat Layer 0
================

Token Syntax
------------

The following is a BNF/PEG-like description of the tokens. A program
is tokenized by matching the top `file` rule, which results in a
listlet of all the tokens.

```
file ::= (whitespace* token)* whitespace* ;
# result: @[token*]

token ::= punctuation | integer | string | identifier ;
# result: same as whichever alternate was picked.

punctuation ::=
    "@@" | # result: [:@"@@":]
    "::" | # result: [:@"::":]
    "<>" | # result: [:@"<>":]
    "@"  | # result: [:@"@":]
    ":"  | # result: [:@":":]
    "*"  | # result: [:@"*":]
    ";"  | # result: [:@";":]
    "="  | # result: [:@"=":]
    "-"  | # result: [:@"-":]
    "?"  | # result: [:@"?":]
    "<"  | # result: [:@"<":]
    ">"  | # result: [:@">":]
    "{"  | # result: [:@"{":]
    "}"  | # result: [:@"}":]
    "("  | # result: [:@"(":]
    ")"  | # result: [:@")":]
    "["  | # result: [:@"[":]
    "]"    # result: [:@"]":]
;

integer ::= ("0".."9")+ ;
# result: [:@integer <intlet>:]

string ::= "\"" (~("\\"|"\"") | ("\\" ("\\"|"\""|"n")))* "\"" ;
# result: [:@string <stringlet>:]

identifier ::=
    ("_" | "a".."z" | "A".."Z") ("_" | "a".."z" | "A".."Z" | "0".."9")* ;
# result: [:@identifier <stringlet>:]

whitespace ::= " " | "\n" | "#" (~("\n"))* "\n" ;
# result: n/a; automatically ignored.
```