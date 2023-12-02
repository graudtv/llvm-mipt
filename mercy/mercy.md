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
    + [Translation Unit](#translation-unit)
    + [Declaration](#declaration)
    + [Expression](#expression)
    + [Type expression](#type-expression)
    + [Builtin type](#builtin-type)
    + [Function type expression](#function-type-expression)
    + [Array type expression](#array-type-expression)
    + [Block expression](#block-expression)
    + [Extern expression](#extern-expression)
    + [Constructors](#constructors)
    + [Memory management](#memory-management)
- [Full grammar](#full-grammar)
    + [Lexer grammar](#lexer-grammar)
    + [Parser grammar](#parser-grammar)
- [TODO](#todo)
    + [Strings](#strings)
    + [Operations on types](#operations-on-types)
    + [Control flow](#control-flow)

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

Expression of primitive type (see _Primitive vs non-primitive types_) is copied
when assigned to another variable or passed to a function. Expression of
non-primitive type is assigned or passed by reference (from compiler
perspective - by pointer to the object)

However, expression of primitive type can also be assigned or passed by
reference if this is requested explicitly using '&' symbol:
```
let &y = x;
let increment(&x) = x + 1;
let swap(&x, &y) = { let tmp = x; x = y; y = tmp; }
```
'&' notation has no effect if assignee expression is of non-primitive type:
expression is passed by reference, as it would be without '&'. This enables
the ```swap()``` function above to work with both primitive and non-primitive
types.

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
    : "let" identifier "=" generic-expression ";"
    | "let" "&" identifier "=" expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" generic-expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" generic-expression "when" expression ";"
optional-function-parameter-list
    : function-parameter-list
    | %empty
function-parameter-list
    : function-parameter-list "," function-parameter
    | function-parameter
function-parameter
    : identifier
    | "&" identifier
```

##### Expression
_expression_ is the result of some evaluation. There are 4 ways how an
expression can be built:
1. Use of variable, function/operator invokation
2. _type-expression_
3. _block-expression_
4. _extern-expression_

The 1st option represents usual arithmetic operations and function calls.
Syntax, semantics and precedence is the same as in C (though not all C
operators are supported)

| Operators    | Associativity |
|--------------|---------------|
| () []        | left to right |
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

Examples of _expression_-s:
```
let x = a + b; // a + b is expression
func(x, 2 + 2 * 2); // x, 2, 2 * 2, 2 + 2 * 2, func(x, 2 + 2 * 2) are expressions
```

_identifier_ used in _expression_ is accessed by reference:
```
let x = 0;
let inc_x() = { x = x + 1 };
inc_x();
print(x); // 1
```

Options 2-4 do not have equivalent in C/C++. See [type
expression](#type-expression), [block expression](#block-expression) and
[extern expression](#extern-expression) for details.

_Grammar Synopsis_:
```
expression
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
    : multiplicative-expression "*" prefix-expression
    | multiplicative-expression "/" prefix-expression
    | multiplicative-expression "%" prefix-expression
    | prefix-expression
prefix-expression
    : "!" postfix-expression
    | postfix-expression
postfix-expression
    : primary-expression "(" ")"
    | primary-expression "(" expression-list ")"
    | primary-expression "[" expression "]"
    | primary-expression
primary-expression
    : identifier
    | numeric-literal
    | "(" expression ")"
    | type-expression
    | block-expression
    | extern-expression
expression-list
    : expression-list "," expression
    | expression
```

##### Type expression
Unlike C/C++, *Mercy* has unified grammar for operations on types and normal
values. It provides great opportunities for metaprogramming, for example types
can be passed to functions as arguments and much more.

_type-expression_ builds a type. Note that this is not the only way how type
can occur in some expression: variables can be type aliases and functions can
return types. However, each type is eventually built by a sequence of
_type-expression_-s.

_Grammar Synopsis_:
```
type-expression
    : builtin-type
    | function-type-expression
    | array-type-expression
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

uintptr is a special type which can be used to exchange opaque pointers
with C code.

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
It starts with _function_type_ keyword, followed by the list of types in
parentheses:
```
let func_type1 = function_type(void, int, int);
let func_type2 = function_type(int32);
```
The first argument is function return type, the latter zero or more arguments
are function parameter types.

_Grammar Synopsis_:
```
function-type-expression
    : "function_type" "(" expression-list ")"
```

##### Array type expression
_array-type-expression_ specifies an array type:
```
let arr_t = array_type(int32);
```

_Grammar Synopsis_:
```
array-type-expression
    : "array_type" "(" expression ")"
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

##### Extern expression
Symbols defined in other object files (including ones produced from code in
another language) can be referenced with _extern-expression_. The first
argument of _extern_ is the name of the symbol to be referenced, the second
is the symbol type.

Usually _extern-expression_ is assigned to a variable using _let_. However,
it can also be used inside expressions directly.
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
    : "extern" "(" identifier "," expression ")"
```

##### Constructors
Integer type can be used as a function name to construct a type from
another integer type, i.e. perform a type cast:

```
let x = 10;              // x is int
let y = int16(x);        // y is int16
let uint_type = uint32;
let z = uint_type(x);    // z is uint32
```

Array type can be used to create an array of specific size:
```
let arr_t = array_type(int32);
let arr = arr_t(16); // array of 16 elements
```

From syntactic perspective, these are just regular function calls, so
such syntax is already supported by _expression_ hierarchy. Only special
semantic handling is required.

##### Memory management
* Any global object is alive from the invokation of main() until the end of
  execution
* Currently any non-global object is alive from the point of its
  definition until the end of the enclosing function.

Compiler may use registers, stack or heap as storage on its own choice, as
long as the requirements of *Mercy* standard are satisfied and no memory is
leaked.

Note. The requirement on life time of non-global objects highly constrains
usage of lambdas and work with memory in general. This issue will be addressed
in the future.
```
// OK, x is primitive and thus copied when returned
let foo() = { let x = 0; return x; };

// UB: dangling reference. x is destroyed at the end of boo()
let boo() = { let x = array_type(int32)(16); return x; };

// OK, x is alive until end of bar()
let bar() {
  let x = 0;
  let f() = { print(x); };
  return f();
};

// UB: returned lambda has dangling reference to x
let moo() = {
  let x = 0;
  let f() = { print(x); };
  return f;
};

```

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
    | "let" identifier "(" optional-function-parameter-list ")" "=" expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" expression "when" expression ";"
optional-function-parameter-list
    : function-parameter-list
    | %empty
function-parameter-list
    : function-parameter-list "," function-parameter
    | function-parameter
function-parameter
    : identifier
    | "&" identifier

expression
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
    : multiplicative-expression "*" prefix-expression
    | multiplicative-expression "/" prefix-expression
    | multiplicative-expression "%" prefix-expression
    | prefix-expression
prefix-expression
    : "!" postfix-expression
    | postfix-expression
postfix-expression
    : primary-expression "(" ")"
    | primary-expression "(" expression-list ")"
    | primary-expression "[" expression "]"
    | primary-expression
primary-expression
    : identifier
    | numeric-literal
    | "(" expression ")"
    | type-expression
    | block-expression
    | extern-expression
expression-list
    : expression-list "," expression
    | expression

type-expression
    : builtin-type
    | function-type-expression
    | array-type-expression
builtin-type
    : "void" | "bool"
    | "int" | "int8" | "int16" | "int32" | "int64"
    | "uint" | "uint8" | "uint16" | "uint32" | "uint64"
    | "uintptr"
function-type-expression
    : "function_type" "(" expression-list ")"
array-type-expression
    : "array_type" "(" expression ")"

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

extern-expression
    : "extern" "(" identifier "," expression ")"
```

### TODO
Thoughts and ideas on future extensions.

##### Strings
Will be added soon.

##### Operations on types
The point of expressions with types is too allow something like this:
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
