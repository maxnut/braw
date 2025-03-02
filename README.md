# Braw
The worst programming language ever made (probably)

Bugs included for free

## Usage
1. Compile a file using: ```brawc file.braw```, this will spit out a `.asm` file

2. Assemble it (I only tested nasm 🔥)

3. ?? idk you can't do much yet

You can call it from C but only on linux x86_64. Commands i use

```
nasm -f elf64 ./build/main.asm -o ./build/main.o
gcc -m64 -no-pie -c ./build/test.c -o ./build/test.o
gcc -m64 -no-pie ./build/test.o ./build/main.o
```

## Syntax
Pretty much C. Some key differences:

### Functions
Functions use `fn` instead of `void`/`int`/whatever. Specify the return after
```braw
fn function(int a, float b) -> void {}
```

### Single-line blocks
Who needs `{}` just slap a `:` instead.
```braw
fn add(int a, int b) -> int: return a + b;

if (a == 1): a = 2;
```

## Example Code
Here’s a totally necessary and useful example:
```braw
struct Quad {
    int top;
    int bottom;
    int left;
    int right;
};

fn quad_create() -> Quad {
    Quad q;
    q.top = 0;
    q.bottom = 0;
    q.left = 0;
    q.right = 0;
    return q;
}

fn quad_get_perimeter(Quad q) -> int:
    return q.top + q.bottom + q.left + q.right;

fn yap() -> int {
    Quad quad = quad_create();
    quad.top = 1;
    quad.bottom = 2;
    quad.left = 3;
    quad.right = 4;
    return quad_get_perimeter(quad);
}
```

## Todo

- [ ] Pointers
- [ ] Arrays
- [ ] Stick more to conventions to make it call other language's functions
- [ ] Make it not explode every two seconds
