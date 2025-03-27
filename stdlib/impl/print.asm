.section .note.GNU-stack,"",@progbits
.intel_syntax noprefix
.section .text
.global print_int

print_int:
    push rbp
    mov rbp, rsp
    sub rsp, 32

    mov rax, rdi
    mov rbx, rsp
    add rbx, 31
    mov byte ptr [rbx], 10
    dec rbx
    mov rcx, 10

    test rax, rax
    jns .convert
    mov dl, '-'
    mov [rbx], dl
    dec rbx
    neg rax

.convert:
    xor rdx, rdx
    div rcx
    add dl, '0'
    mov [rbx], dl
    dec rbx
    test rax, rax
    jnz .convert

    inc rbx
    mov rdx, rsp
    add rdx, 32
    sub rdx, rbx

    mov rdi, 1
    mov rsi, rbx
    mov rax, 1
    syscall

    add rsp, 32
    pop rbp
    ret

