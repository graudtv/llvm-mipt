# Mercy Programming Language

Main features:
* Strong static typization
* Compiled, hence efficient
* Imperative and functional paradigms
* Generic code and metaprogramming support
* Foreign language support (C/C++)
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
Program consists of a source file in _Mercy_ language, refered to as
_translation unit_, which can be compiled into an object file and then linked
with object files or libraries coming from other programming languages.
_translation_unit_ must define ```main()``` function, which is an entry
point to the program. ```main()``` must return _void_ or _int_.

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
_declaration_ defines a variable or a function or a type alias. It can be used
either at the top level or inside of a function.
Note that "declaration" and "definition" are synonims in _Mercy_.

All declaration start with _let_ keyword:
```
let x = 100;                     // int variable declaration
let sum(x, y) = x + y;           // function declaration
let int_type = int32;            // type alias declaration
```

Type of variable is deduced based on the type of initializer.

Each function is inherently a template function. The type of the function is
deduced only during instantiation. Types of parameters are deduced based on
types of supplied arguments, and the return type is deduced based on type of
its body.

Function with multiple statements can be definied with use of curly brackets:
```
let distance(x1, x2, y1, y2) {
  let dx = x2 - x1;
  let dy = y2 - y1;
  return sqrt(dx * dx + dy * dy);
};
```
Within such syntax zero or more _return-statements_ can be used. If none used,
return type of the function is void. Otherwise, all _return-statements_ must
return the same type, which becomes the return type of the function.

Expression of trivial type (_void_ or integer) is copied when assigned to
another variable or passed to a function. Expression of non-trivial type is
assigned or passed by reference.

However, expression of trivial type can also be assigned or passed by
reference if this is requested explicitly using '&' symbol:
```
let &y = x;
let increment(&x) = x + 1;
let swap(&x, &y) { let tmp = x; x = y; y = tmp; }
```
'&' notation has no effect if assignee expression is of non-trivial type:
expression is passed by reference, as it would be without '&'. This enables the
```swap()``` function above to work with both trivial and non-trivial types.

Function can be defined on a specific domain using _when_ keyword. Multiple
definitions on non-intersecting domains are possible:
```
let fact(n) = 1                   when n == 1;
let fact(n) = fact(n - 1)         when n > 1;
```
If domains in different declarations of the same function intersect, the
program is ill-formed, no diagnostic required. If a function is called with
arguments which are not part of any domain, "domain error" is emited at
runtime and program terminates.

The last definition of a function may use _otherwise_ keyword to handle
arguments which does not belong to any of domains specified in previous
definitions of this function:
```
let select(cond, x, y) = x        when cond;
let select(cond, x, y) = y        otherwise;
```

_Grammar Synopsis_:
```
declaration
    : "let" identifier "=" expression ";"
    | "let" "&" identifier "=" expression ";"
    | "let" identifier "(" optional-function-parameter-list ")" "=" expression optional-domain ";"
    | "let" identifier "(" optional-function-parameter-list ")" "{" statement-list "}" optional-domain ";"
optional-function-parameter-list
    : function-parameter-list
    | %empty
function-parameter-list
    : function-parameter-list "," function-parameter
    | function-parameter
function-parameter
    : identifier
    | "&" identifier
statement-list
    : statement-list statement
    | statement
statement
    : declaration
    | expression ";"
    | return-statement
return-statement
    : "return" expression ";"
optional-domain
    : "when" expression
    | otherwise
    | %empty
```

##### Type system
Integers, function references and arrays are refered to as _Value_-s. Each
_Value_ is a member of some set in mathematical sence, which is refered to as
_Type_. The _Type_ defines possible operations on _Value_-s in it and semantics
of that operations.

Examples of types (math-like notation):
```
int = { 0, -1, 1, -2, 2, -3, 3, ... }
uint64 = { 0ul, 1ul, 2ul, 3ul, 4ul, ... }
array of int = { [], [0, 1, 2] , [-10, 13, 48, 99], ... }
function (int,int)->void = { /* all possible functions of this signature */ }
```

All _Type_-s are members of a single set called _Meta_:
```
Meta = { uint, uint64, array of int, function (int,int)->void, ... }
```
Basically, _Meta_ is the type of a _Type_. _Meta_ is a top-level entity,
it has no type itself.

Unlike C/C++, *Mercy* has unified grammar for operations on _Value_-s and
_Type_-s. It provides great opportunities for metaprogramming, for example
types can be passed to functions as arguments and much more. _Value_-s and
_Type_-s together are called _Generic Value_-s. _Type_-s and _Meta_ together
are called _Generic Type_.

List of all _Type_-s:
* void, bool, int8, int16, int32, int64, uint8, uint16, uint32, uint64, string
* array of elements of type T for each _Generic Type_ T
* function with N parameters of types T1, ..., TN and return value of type RT
  for each N >=0 and for each combination of _Generic Type_-s T1, ..., TN, TR

##### Builtin type
List of builtin types:
* void, bool, int8, int16, int32, int64, uint8, uint16, uint32, uint64, string
* int, uint

_int_ and _uint_ are aliases to some _intXX_ and _uintXX_ type. Other types are
different.

Note. _Mercy_ does not allow implicit conversions, so implicit conversion from
_bool_ to _uint8_ is not possible. This can be achieved with explicit cast, if
desired. However, since _int_ and _int32_ are the same type, no explicit cast
required.

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
    | "string"
```

##### Expression
_expression_ is the result of some evaluation. The result is _Generic Value_
(see [Type system](#type-system)), i.e. each expression has a type which is
either _Type_ or _Meta_.

The most trivial form of expression is use of some _Generic Value_:
```
let x = 10; // 10 is expression of type int
let y = int32; // int32 is expression of type Meta
let str = "Hello"; // "Hello" is expression of type string
let z = x; // x is expression of type int
```

Other expression are built using function calls and operators. The syntax and
meaning is the same as in C, however not all C operators are supported.

| Operators    | Associativity |
|--------------|---------------|
| () []        | left to right |
| !            | right to left |
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

Operators have the following restrictions and semantics:
* Operands of '&&', '||', '!' must be _Value_-s of _bool_ type, result is also
  _bool_
* Operands of '<', '<=', '>', '>=' must be _Value_-s of the same integer type.
  The result has _bool_ type. Depending on whether the type is signed or
  unsigned, signed or unsigned comparison is performed
* Operands of '==', '!=' must be _Value_-s of the same integer type. The
  result has _bool_ type.
* Operands of '&', '^', '|' must be _Value_-s of the same unsigned integer
  type. The result has the same type as operands.
* Operands of '*', '+', '-', '<<' must be _Value_-s of the same integer type.
  The result has the same type.
* Operands of '/', '%', '>>' must be _Value_-s of the same integer type. The
  result has the same type. Depending on whether the type is signed or
  unsigned, signed or unsigned operation is performed (signed division vs
  signed division, signed remainder vs unsigned remainder, logical shift vs
  arithmetic shift)
* LHS operand of '()' must be either compile-time _Value_ of function type or
  builtin _Type_. The latter performs a type cast, see [type-cast](#type-cast).
* LHS Operand of '[]' must be _Value_ of some array type. Operand inside
  brackets must be _Value_ of integer type.

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
    : equality-expression "==" relational-expression
    | equality-expression "!=" relational-expression
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
    | string-literal
    | builtin-type
    | "(" expression ")"
expression-list
    : expression-list "," expression
    | expression
```

##### Type casts
Compile-time builtin _Type_ can be used as a function name to construct a type
from another integer type, i.e. perform a type cast:

```
let x = 10;              // x is int
let y = int16(x);        // y is int16
let uint_type = uint32;
let z = uint_type(x);    // z is uint32
```

From syntactic perspective, these are just regular function calls, so
such syntax is already supported by _expression_ hierarchy. Only special
semantic handling is required.

### Builtin Functions
Part of *Mercy* functionality is provided as builtin functions. Invokation and
usage syntax is exactly the same as for user-defined functions. These
functions are part of the language because it is not possible to implement
them in user code or as part of the standard library using only the base means
of the language. Instead, they actually provide such means to the user.

##### _function_type()_ function
_Synopsis_: ```let function_type(return_type, param_types...) = <function_type>;```

_function-type()_ returns a function type:
```
let func_type1 = function_type(void, int, int);
let func_type2 = function_type(int32);
```
The first argument is function return type, the latter zero or more arguments
are function parameter types. All arguments must be compile-time expressions.
The result is compile-time expression.

##### _extern()_ function
_Synopsis_: ```let extern(symbol_name, symbol_type) = <reference_to_symbol>;```

Symbols defined in other object files (including ones produced from code in
another language) can be referenced with _extern()_ function. The first
argument of _extern()_ is the name of the symbol to be referenced, the second
is the symbol type. The return value is reference to the symbol.
Both arguments must be compile-time expressions.

Usually result of _extern()_ is assigned to a variable using _let_.
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

##### _array()_ function
_Synopsis_: ```let array(values...) = <array>;```

_array()_ creates an array of supplied values. All values must be
_Generic Value_-s of the same type.

##### _alloca()_ function
_Synopsis: ```let alloca(type, size) = <array>;```

* When used in global scope, _alloca()_ creates a global array. _size_ must be
  compile-time non-negative _Value_ of integer type. _type_ must be
  a _Generic Type_.
* When use inside function, _alloca()_ creates an array on stack. _size_ must
  be non-negative _Value_ of integer type. _type_ must be a _Generic Type_.

### Other semantical concepts
##### Compile-time expressions
Since *Mercy* is compiled statically typed language, some context may require
_Value_ or _Type_ to be known at compile-time, i.e. to be _compile-time
expression_.

Expression is compile-time expression if one of the following conditions
is satisfied:
* Expression is string or numeric literal
* Expression is builtin-type ("int", "uint", ...)
* Expression is the result of call to builtin function _function_type()_

This set may be extended in the future.

##### ODR and scope
Each declaration is visible only in certain part of the program, which is
called the _scope_ of declaration.

* Top-level declaration is called _global declaration_. It is visible for
  all following global declarations and in each function (even if function
  comes before that declaration). The latter allows global functions to call
  each other
* Function declaration opens a nested scope, in which its parameters and body
  are evaluated. Each parameter and declaration in the body is visible
  from the point of declaration until the end of function.

Declaration is looked up from the innermost to the global scope. Declaration
with the same name may occur on different layers of scope stack, but not in
the same scope. An exception is declaration of the function on certain domain,
because it is actually only a part of declaration; however parameter count and
return type deduced during instantiation must be the same.

##### Memory management
* Any global object is alive from the invokation of main() until the end of
  execution
* Any non-global object is alive from the point of its definition until the
  end of the enclosing function.

Compiler may use registers, stack or heap as storage on its own choice, as
long as the requirements of *Mercy* standard are satisfied and no memory is
leaked.

Note. The requirement on life time of non-global objects highly constrains
usage of lambdas and work with memory in general. This issue will be addressed
in the future.
```
// OK, x is trivial and thus copied when returned
let foo() = { let x = 0; return x; };

// UB: dangling reference. x is destroyed at the end of boo()
let boo() = { let x = array_type(int32)(16); return x; };

// OK, x is alive until the end of bar()
let bar() {
  let x = 0;
  let f() = { print(x); };
  f();
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
string-literal:
    "*"
```

Operators: ```* / % + - << >> < <= > >= == != & ^ | && || !```

Separators: ```( ) { } , ;```

Keywords:
* ```when```, ```otherwise```, ```return```
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
    | "let" identifier "(" optional-function-parameter-list ")" "=" expression optional-domain ";"
    | "let" identifier "(" optional-function-parameter-list ")" "{" statement-list "}" optional-domain ";"
optional-function-parameter-list
    : function-parameter-list
    | %empty
function-parameter-list
    : function-parameter-list "," function-parameter
    | function-parameter
function-parameter
    : identifier
    | "&" identifier
statement-list
    : statement-list statement
    | statement
statement
    : declaration
    | expression ";"
    | return-statement
return-statement
    : "return" expression ";"
optional-domain
    : "when" expression
    | otherwise
    | %empty

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
    : equality-expression "==" relational-expression
    | equality-expression "!=" relational-expression
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
    : postfix-expression "(" ")"
    | postfix-expression "(" expression-list ")"
    | postfix-expression "[" expression "]"
    | primary-expression
primary-expression
    : identifier
    | numeric-literal
    | string-literal
    | builtin-type
    | "(" expression ")"
expression-list
    : expression-list "," expression
    | expression

builtin-type
    : "void" | "bool"
    | "int" | "int8" | "int16" | "int32" | "int64"
    | "uint" | "uint8" | "uint16" | "uint32" | "uint64"
```

### TODO
Thoughts and ideas on future extensions.

##### Operations on strings
Basic string operations like '+', strlen, etc

##### Operations on types
The point of _Generic Value_-s is to allow something like this:
```
let sizeof(type) = type.size();
let sz = sizeof(int32); // sz = 4
```
and this (template specialization, essentially):
```
let any_signed_int = int8 | int16 | int32 | int64;
let foo(x) = smth() when x is any_signed_int;
let foo(x) = smth_else() when x is string;
```
Requires significant extensions and sufficient constexpr evaluation support

##### Control flow
Generally _when_ clause allows for writing anything using functional-like
style:
```
let maybe(cond, f) { f(); }         when cond;
let maybe(cond, f) {}               otherwise;

let say_hello() = print("Hello, world!");
let cond = (0 == 0);
maybe(cond, say_hello);
maybe(!cond, say_hello);
```
However, _if_ + _for_ + _while_ should be useful.
