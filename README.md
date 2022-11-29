# NBASE - A BASIC interpreter (WIP)

Build deps, please take note:

* GCC</br>
* Bash

Run <b>./build.sh</b> in the directory, executable is called <b>nbase</b>
</br></br>

## Features

- [X] Line oriented
- [X] Garbage collected (partially implemented)
- [X] Prioritizes correctness over performance by now
- [X] Tokenized code (kind of)
- [ ] Probably no GOTOs allowed.
- [ ] Structured control flow constructs

</br>

## Data types

### <b>Integer: signed 32bit (C's int32_t type)</b>

Eg.:
</br>
* 1
</br>
* 666
</br>
* %5 (negative)
</br>
* 0AH, 0AX (hex integer)

### <b>Float: 32bit (C's float type)</b>

Eg.:
</br>
* 1.2
</br>
* 0AR (hex float)
</br>
* 1.2E2, 1.E+2, 2.6E-4 (exponential notation, E can be also D)
</br>
NOTE: literal number format taken from Oberon07 parser.

### <b>String</b>

* "This is a string"

</br>

## Operators, grouped from higher to lower precedence

\%
\*
\/
MOD
\^
NOT

\+
\-
LSHIFT
RSHIFT

AND
OR
XOR

\<
\<\=
\>
\>\=
\=
\<\>

</br>

## Working keywords
</br>

## PRINT

#### PRINT
#### PRINT \<expression\>
#### PRINT \<expression\>;
#### PRINT \<expression\>;\<expression\>...
</br>

## END
Terminates program execution.
</br>

## .
"Compiles" following code line into tokenized form in code area.
</br>

## RUN
Runs tokenized code.
</br>

## DUMP
Memory dump of start of code area for debugging purposes.
</br>

## LVAR
List variable descriptors for debugging purposes.</br>NOTE: variables must be reimplemented for tokenized execution, still pending.
</br>

## STAT
Dump some inner state and system limits.
