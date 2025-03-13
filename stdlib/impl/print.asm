.intel_syntax noprefix
.section .text
.global print_int
.global print_string

print_string:
    push rbp
    mov rbp, rsp

    mov rsi, rdi
    xor rdx, rdx

count_length:
    mov al, byte [rsi + rdx]
    test al, al
    je write_string
    inc rdx
    jmp count_length       

write_string:
    inc rdx
    mov rax, 1
    mov rdi, 1
    syscall

    pop rbp
    ret

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

