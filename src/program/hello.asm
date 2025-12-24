BITS 32
section .programs
align 16

%define SYS_PRINT 0

global __program_hello_start
__program_hello_start:
    dd 0x5047524D
    dd _entry_hello - __program_hello_start
    dd __program_hello_end - __program_hello_start
    dd "hello.BIN",0
    times 16-($-__program_hello_start-16) db 0

global _entry_hello
_entry_hello:
    mov ebx, eax ; kernel api
    push msg - __program_hello_start
    call dword [ebx + SYS_PRINT]
    add esp, 4
    ret

msg db "Hello, Wolrd",0
align 16
__program_hello_end: