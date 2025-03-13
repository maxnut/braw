.intel_syntax noprefix
.section .text
.global syscall

syscall:
    mov rax, rdi    # Syscall number
    mov rdi, rsi    # First argument
    mov rsi, rdx    # Second argument
    mov rdx, rcx    # Third argument
    mov r10, r8     # Fourth argument
    mov r8,  r9     # Fifth argument
    mov r9,  [rsp+8] # Sixth argument (if any, from stack)
    syscall         # Invoke syscall
    ret             # Return
