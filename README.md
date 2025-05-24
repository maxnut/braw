# Braw
Bugs included for free

## Usage
Run ```brawc file.braw --assemble --link```, this will spit out an executable

The compiler targets x86-64 Sys-V, so you need to run it under that

Assembling and linking stages use GNU assembler and linker. If you don't have them omit the `--assemble` and `--link` flags to only output the `.asm` file.

To include files from the standard library, either move the folder to the compiler's location or define the `BRAW_STDLIB` environment variable.

## Building
Build the project with Cmake

You need a C++ stdlib that implements std::expected, as it is used in the project

## Tests
The tests need gas to work.

If you don't have it, or simply don't want them, disable them by setting ```RUN_TESTS``` inside ```CMakeLists.txt``` to **OFF**
