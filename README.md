# Braw
Bugs included for free

## Usage
Run ```/brawc file.braw -o build --assemble --link```, this will spit out an executable

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
