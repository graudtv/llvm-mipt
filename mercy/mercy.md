# Mercy Programming Language

Main features:
* Strong static typization
* Compiled, hence efficient
* Imperative and functional paradigms
* Generic code and metaprogramming support
* External C functions can be invoked
* All these features are implemented within a harmonic concise grammar

### Table of contents
- [Syntax and semantics overview](#syntax-and-semantics-overview)
    * [Translation Unit](#translation-unit)
    * [Declaration](#declaration)
    * [Generic expression](#generic-expression)
    * [Type expression](#type-expression)
    * [Builtin type](#builtin-type)
    * [Function type expression](#function-type-expression)
    * [Expression](#expression)
    * [Arithmetic expression](#arithmetic-expression)
    * [Extern expression](#extern-expression)
    * [Block expression](#block-expression)
    * [Type conversions](#type-conversions)
- [Full grammar](#full-grammar)
    * [Lexer grammar](#lexer-grammar)
    * [Parser grammar](#parser-grammar)
- [TODO](#todo)
    * [Memory and arrays](#memory-and-arrays)
    * [Strings](#strings)
    * [Operations on types](#operations-on-types)
    * [Control flow](#control-flow)

### Syntax and semantics overview
As in C/C++, program consists of one or more source files in _Mercy_ language,
refered to as _translation units_, which are compiled into object files
independently from each other. To produce an executable, object files are
passed to the regular linker together with object files coming from other
programming languages. The entry point to the program is ```main()```
function.

##### Translation Unit
_translation-unit_ consists of zero or more _declaration_-s, which define
global entities.

_Grammar Synopsis_:
```
translation-unit
    : declaration-list
    | %empty
declaration-list
    : declaration-list declaration
    | declaration
```

##### Declaration
_declaration_ defines a variable, a function or a type alias. It can be used
either at the top level or in a _block_expression_.
Note that "declaration" and "definition" are synonims in _Mercy_.

All declaration start with _let_ keyword:
```
let x = 100;                     // int variable declaration
let sum(x, y) = x + y;           // function declaration
let int_type = int32;            // type alias declaration
```

Type of variable is deduced based on the type of initializer.

Type of function is deduced based on the parameter count and the type of
initializer, the latter defining the return type of the function.
Each function is inherently a template function.

Functions can take arguments by reference, and reference to a variable can
be declared:
```
let &y = x;
let increment(&x) = x + 1;
```

Function can be defined on a specific domain using _when_ keyword. Multiple
definitions on non-intersecting domains are possible:
```
let fact(n) = 1                   when n == 1;
let fact(n) = fact(n - 1)         when n > 1;
```
If domains in different declarations of the same function intersect, the
program is ill-formed, no diagnostic required. If a function is called with
arguments which are not part of any domain, a runtime error is emited and
program terminates.

_Grammar Synopsis_:
```
declaration
    : "let" identifier "=" expression ";"
    | "let" "&" identifier "=" expression ";"
    | "let" identifier "=" type-expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" generic-expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" generic-expression "when" expression ";"
optional-function-parameter-list
    : function-paramer-list
    | %empty
function-parameter-list
    : function-parameter-list "," function-parameter
    | function-parameter
function-parameter
    : identifier
    | "&" identifier
```

##### Generic expression
In some contexts types can be used as usual expressions, e.g. they can be
passed to a function as an argument. So there is a concept of
_generic-expression_, which is either a _type-expression_ or _expression_.

_Grammar Synopsis_:
```
generic-expression
    : type-expression
    | expression
```

##### Type expression
_type-exprssion_ is either a _builtin type_ or a _function type expression_.

_Grammar Synopsis_:
```
type-expression
    : builtin-type
    | function-type-expression
```

##### Builtin type
List of builtin types:
* void
* bool
* int, int8, int16, int32, int64
* uint, uint8, uint16, uint32, uint64
* uintptr

_int_ and _uint_ are aliases to some _intXX_ and _uintXX_ type. Other types are
different.

Note. _Mercy_ does not allow implicit conversions, so implicit conversion from
_bool_ to _uint8_ is not possible. This can be achieved with explicit cast,
if desired. However, since _int_ and _int32_ (or _int64_, depending on
architecture) are the same type, no explicit cast required.

Each builtin type has an associated keyword, e.g.:
```
let int_type = int32;
let uint_type = uint32;
```

_Grammar Synopsis_:
```
builtin-type
    : "void" | "bool"
    | "int" | "int8" | "int16" | "int32" | "int64"
    | "uint" | "uint8" | "uint16" | "uint32" | "uint64"
    | "uintptr"
```

##### Function type expression
_function-type-expression_ specifies a function type, i.e. function signature.
It starts with _function_type_ keyword, followed by the list of types:
```
let func_type1 = function_type(void, int, int);
let func_type2 = function_type(int32);
```
The first argument is function return type, the latter zero or more arguments
are function parameter types.

_Grammar Synopsis_:
```
function-type-expression
    : "function_type" "(" type-list ")"
type-list
    : type-list type-expression
    | type-expression
```

##### Expression

_expression_ is either an _arithmetic-expression_ or _block-expression_.

_Grammar Synopsis_:
```
expression
    : arithmetic-expression
    | block-expression
```

##### Arithmetic expression

_arithemetic-expression_ defines regular arithmetic operations and function
calls. Syntax, semantics and precedence is the same as in C (though not all C
operators are supported)

| Operators    | Associativity |
|--------------|---------------|
| ()           | left to right |
| * / %        | left to right |
| + -          | left to right |
| << >>        | left to right |
| < <= > >     | left to right |
| == !=        | left to right |
| &            | left to right |
| ^            | left to right |
| \|           | left to right |
| &&           | left to right |
| \|\|         | left to right |

Examples:
```
let x = a + b;
func(x, 2 + 2 * 2);
```

_Grammar Synopsis_:
```
arithmetic-expression
    : logical-or-expression
logical-or-expression
    : logical-or-expression "||" logical-and-expression
    | logical-and-expression
logical-and-expression
    : logical-and-expression "&&" or-expression
    | or-expression
or-expression
    : or-expression "|" xor-expression
    | xor-expression
xor-expression
    : xor-expression "^" and-expression
    | and-expression
and-expression
    : and-expression "&" equality-expression
    | equality-expression
equality-expression
    : equality-expresion "==" relational-expression
    : equality-expresion "!=" relational-expression
    | relational-expression
relational-expression
    : relational-expression "<" shift-expression
    | relational-expression ">" shift-expression
    | relational-expression "<=" shift-expression
    | relational-expression ">=" shift-expression
    | shift-expression
shift-expression
    : shift-expression "<<" additive-expression
    | shift-expression ">> additive-expression
    | additive-expression
additive-expression
    : additive-expression "+" multiplicative-expression
    | additive-expression "-" multiplicative-expression
    | multiplicative-expression
multiplicative-expression
    : multiplicative-expression "*" primary-expression
    | multiplicative-expression "/" primary-expression
    | multiplicative-expression "%" primary-expression
    | prefix-expression
prefix-expression
    : "!" postfix-expression
    | postfix-expression
postfix-expression
    : primary-expression "(" ")"
    | primary-expression "(" function-argument-list ")"
    | primary-expression
primary-expression
    : identifier
    | numeric-literal
    | extern-expression
    | "(" expression ")"
```

##### Extern expression
Symbols defined in other object files (including ones produced from code in
another language) can be referenced with _extern expression_. The first
argument of _extern_ is the name of the symbol to be referenced, the second
is the symbol type.

Usually _extern expression_ is assigned to a variable using _let_. Though
it has a high precedence and can also be used inside expressions.
```
let print_hex = extern(_print_hex, function_type(void, int32));
let var = extern(_var, int32);
print_hex(var);

let &var_ref = extern(_var, int32);
var_ref *= 2;

let v = 2 + extern(_var, int32) * 2
```


If _extern_ references a function, the domain of the callee is constrained by
the types specified in _extern expression_ as if a _when_ clause was used.

If type specified in _extern expression_ does not match the actual type of
the symbol, behaviour is undefined.

_Grammar Synopsis_:
```
extern-expression
    : "extern" "(" identifier type-expression ")"
```

##### Block expression
_block-expression_ is a list of zero or more _statement_-s enclosed into
curly brackets. _block-expression_ is executed statement by statement and
yields the result of the expression under the first encountered _return_
during execution.  All _return_-s must return expressions of the same type. If
there is no _return_, return type of the _block-expression_ is _void_.

This is most useful in function declarations:
```
let dist(x1, x2, y1, y2) = {
  let dx = x2 - x1;
  let dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
};
```

_Grammar Synopsis_:
```
block-expression
    : "{" "}"
    | "{" statement-list "}"
statement-list
    : statement-list statement
    | statement
statement
    : declaration
    | expression
    | "return" expression
```

##### Type conversions
Name of a type can be used as a function name to perform a type cast on 
expression:

```
let x = 10;              // x is int
let y = int16(x);        // y is int16
let uint_type = uint32;
let z = uint_type(x);    // z is uint32
```

From syntactic perspective, this is just a regular function call, so
such syntax is already supported by _expression_ hierarchy. Only special
semantic handling is required.


### Full grammar
##### Lexer grammar
Non-trivial terminals:
```
identifier:
    [_a-zA-Z][_a-zA-Z0-9]*
numeric-literal:
    [+-]?[1-9][0-9]*            ; decimal number
    0x[0-9]+                    ; hex number
    0[0-9]+                     ; octal number
```

Operators: ```* / % + - << >> < <= > >= == != & ^ | && ||```

Separators: ```( ) { } , ;```

Keywords:
* ```function_type```, ```extern```, ```return```
* ```void```, ```bool```
* ```int```, ```int8```, ```int16```, ```int32```, ```int64```
* ```uint```, ```uint8```, ```uint16```, ```uint32```, ```uint64```

##### Parser grammar

```
translation-unit
    : declaration-list
    | %empty
declaration-list
    : declaration-list declaration
    | declaration

declaration
    : "let" identifier "=" expression ";"
    | "let" "&" identifier "=" expression ";"
    | "let" identifier "=" type-expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" generic-expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" generic-expression "when" expression ";"

optional-function-parameter-list
    : function-paramer-list
    | %empty
function-parameter-list
    : function-parameter-list "," function-parameter
    | function-parameter
function-parameter
    : identifier
    | "&" identifier

generic-expression
    : type-expression
    | expression

type-expression
    : builtin-type
    | function-type-expression

builtin-type
    : "void" | "bool"
    | "int" | "int8" | "int16" | "int32" | "int64"
    | "uint" | "uint8" | "uint16" | "uint32" | "uint64"
    | "uintptr"

function-type-expression
    : "function_type" "(" type-list ")"
type-list
    : type-list type-expression
    | type-expression

expression
    : arithmetic-expression
    | block-expression

arithmetic-expression
    : logical-or-expression
logical-or-expression
    : logical-or-expression "||" logical-and-expression
    | logical-and-expression
logical-and-expression
    : logical-and-expression "&&" or-expression
    | or-expression
or-expression
    : or-expression "|" xor-expression
    | xor-expression
xor-expression
    : xor-expression "^" and-expression
    | and-expression
and-expression
    : and-expression "&" equality-expression
    | equality-expression
equality-expression
    : equality-expresion "==" relational-expression
    : equality-expresion "!=" relational-expression
    | relational-expression
relational-expression
    : relational-expression "<" shift-expression
    | relational-expression ">" shift-expression
    | relational-expression "<=" shift-expression
    | relational-expression ">=" shift-expression
    | shift-expression
shift-expression
    : shift-expression "<<" additive-expression
    | shift-expression ">> additive-expression
    | additive-expression
additive-expression
    : additive-expression "+" multiplicative-expression
    | additive-expression "-" multiplicative-expression
    | multiplicative-expression
multiplicative-expression
    : multiplicative-expression "*" primary-expression
    | multiplicative-expression "/" primary-expression
    | multiplicative-expression "%" primary-expression
    | prefix-expression
prefix-expression
    : "!" postfix-expression
    | postfix-expression
postfix-expression
    : primary-expression "(" ")"
    | primary-expression "(" function-argument-list ")"
    | primary-expression
primary-expression
    : identifier
    | numeric-literal
    | extern-expression
    | "(" expression ")"

function-argument-list
    : function-argument-list "," generic-expression
    | generic-expression

extern-expression
    : "extern" "(" identifier type-expression ")"

block-expression
    : "{" "}"
    | "{" statement-list "}"
statement-list
    : statement-list statement
    | statement
statement
    : declaration
    | expression
    | "return" expression

```


### TODO
Thoughts and ideas on future extensions.

##### Memory and arrays
Generally, _Mercy_ is fully capable of working with memory and data structures
without any additional language support: these details can be implemented in C
and imported with _extern_ (potentially automatically, as part of _Mercy_
standard library):
```
let new_array = extern("_new_array", function_type(uintptr, uint));
let del_array = extern("_del_array", function_type(void, uintptr));
let array_set = extern("_array_set", function_type(uintptr, uint, int));
let array_get = extern("_array_get", function_type(int, uint));

let arr = new_array(ARRAY_SIZE);
array_set(arr, idx, value);
let elem = array_get(arr, idx);
del_array(arr);
```

However, some syntactic sugar would be useful, such as '[]' operator and
convenient array initialization. These will be addressed in the future and will
be possibly implemented using OOP paradigm (kind of like numpy in Python).

##### Strings
Definetely useful, will be added soon.

##### Operations on types
The point of _type-expressions_ is too allow something like this:
```
let sizeof(type) = type.size;
let sz = sizeof(int32); // sz = 4
```
and this (template specialization, essentially):
```
let foo(x) = smth() when (x is int32) || (x is int64);
let foo(x) = smth_else() when x is string;
```

##### Control flow
Generally _when_ clause allows for writing anything using functional style:
```
let select(cond, x, y) = x        when cond;
let select(cond, x, y) = y        when !cond;

print(select(foo(), boo(), bar(goo())));
```
However, _if_ + _for_ + _while_ should be useful.
