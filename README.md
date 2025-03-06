# Braw
The worst programming language ever made (probably)

Bugs included for free

## Usage
1. Compile a file using: ```brawc file.braw```, this will spit out a `.asm` file

2. Assemble it (default syntax is gas, use ```-a nasm``` to use nasm syntax)

3. ?? idk you can't do much yet

You can call it from C but only on linux x86_64. Commands i use

```
nasm -f elf64 ./build/main.asm -o ./build/main.o
gcc -m64 -no-pie -c ./build/test.c -o ./build/test.o
gcc -m64 -no-pie ./build/test.o ./build/main.o
```

## Syntax

### Functions
Functions use `fn` instead of `void`/`int`/whatever. Specify the return after
```braw
fn function(int a, float b) -> void {}
```

### Variables
Variables are declared like this:
```braw
let epic_variable: int;
```

### Single-line blocks
Who needs `{}` just slap a `:` instead.
```braw
fn add(int a, int b) -> int: return a + b;

if (a == 1): a = 2;
```

## Examples
Check the tests.

## Tests
The tests need gas to work.

If you don't have it, or simply don't want them, disable them by setting ```RUN_TESTS``` inside ```CMakeLists.txt``` to **OFF**

## Todo

- [ ] Pointers
- [ ] Arrays
- [ ] Stick more to conventions to make it call other language's functions
- [ ] Make it not explode every two seconds
- [ ] Debugging?