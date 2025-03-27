# Braw
Bugs included for free

## Usage
Run ```brawc file.braw -o build --assemble --link```, this will spit out an executable

The compiler targets x86-64 Sys-V, so you need to run it under that

Assembling and linking stages use GNU assembler and linker. If you don't have them omit the `--assemble` and `--link` flags to only output the `.asm` file.

To include files from the standard library, either move the folder to the compiler's location or define the `BRAW_STDLIB` environment variable.

## Syntax

### Functions
Functions use `fn` instead of `void`/`int`/whatever. Specify the return after
```braw
fn function(a: int, b: float) -> void {}
```

### Variables
Variables are declared like this:
```braw
let epic_variable: int;
```

### Single-line blocks
Who needs `{}` just slap a `:` instead.
```braw
fn add(a: int, b: float) -> int: return a + b;

if (a == 1): a = 2;
```

### Arrays
```braw
let buf[10]: char*;
```

## Macros

Macros are made using the `define` keyword, and called by using the $ symbol:
```braw
define CONST: 5;

fn wow() -> void {
  let a: int = $CONST;
}
```

They can also take in parameters:
```braw
define CONST(param): 5 + #param;

fn wow() -> void {
  let a: int = $CONST(5);
}
```

### Overview

They can take in parameters
- Instructions (by directly passing an instruction)
- Values (by doing `#"val"`)
- Types (by doing `#<int>`)
- Functions (by doing `#fun()`)

Parameters get assigned a values based on their kind:
- Instructions have no value
- Values have the provided value
- Types have the typename
- Functions have the function name

Parameters also have fields, accessible by doing `#param.field`:
- Types have their members (`#param.memberName`)
- Functions have their return type (`#param.returnType`) and their parameters (`#param.parameters`)

Instructions passed as a parameter can be expanded inside the macro by simply referencing them outside a macro call

For example `#param` will try to expand whatever instruction was passed to the parameter

### Constructs

There are various construct you can use to do compile-time code manipulation:
- `$concat(a, b)` which returns the concatenation of two values
- `$compare(a, b)` which returns `true` if the values are equal or `false` otherwise
- `$not(val)` which negates the boolean value
- `$and(a, b)` which ands two boolean values
- `$or(a, b)` which ors two boolean values
- `$if(val) {}` which checks if the value is true and either includes or excludes its body from the macro
- `$foreach(v : value) {}` which runs for every member of the value and includes its body in the macro each time
- `$make_variable(name, type)` which creates a variable declaration
- `$make_function(name, returnType, {parameters}) {}` which creates a function (parameters are variable declarations)
- `$make_dot(expression, value)` which creates a dot operator
- `$make_arrow(expression, value)` which creates an arrow operator

## Examples
Check the tests

## Building
Build the project with Cmake

You need a C++ stdlib that implements std::expected, as it is used in the project

## Tests
The tests need gas to work.

If you don't have it, or simply don't want them, disable them by setting ```RUN_TESTS``` inside ```CMakeLists.txt``` to **OFF**

## Todo

- [x] Pointers
- [x] Branch aware graph coloring
- [x] Arrays
- [ ] Parallelize compilation
- [x] Strings
- [ ] Variadics
- [ ] Expand the standard library (its basically nonexistant now 🔥🔥🔥)
- [ ] Stick more to conventions to make it call other language's functions
- [ ] Make it not explode every two seconds
- [ ] Debugging?
