[bits 32]
[extern kernel_main]   

[section .multiboot2]
align 8
header_start:
    dd 0xE85250D6           
    dd 0x0                  
    dd header_end - header_start 
    dd 0x100000000 - (0xE85250D6 + 0x0 + (header_end - header_start))
    dd 0                    
    dd 8                    
header_end:

[section .text]
global _start

; GDT mínima
gdt_start:
    dq 0                    ; Null descriptor
    dq 0x00CF9A000000FFFF   ; Code: base=0, limit=4GB, executable
    dq 0x00CF92000000FFFF   ; Data: base=0, limit=4GB, writable
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

_start:
    ; Verifica magic Multiboot2
    cmp eax, 0x36D76289
    jne no_module
    
    ; Passa MBI para kernel_main
    mov esi, ebx            ; MBI pointer em ESI (param C)
    
    cli
    lgdt [gdt_descriptor]
    jmp 0x08:flush_cs       ; Far jump para carregar CS

flush_cs:
    mov ax, 0x10            ; Data selector
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9FC00        ; Stack GRUB padrão
    
    ; Limpa BSS (zero-init)
    extern bss_start
    extern bss_end
    mov edi, bss_start
    mov ecx, bss_end
    sub ecx, edi
    shr ecx, 2              ; /4 para DWORDs
    xor eax, eax
    rep stosd
    call kernel_main        ; Chama C kernel
    jmp halt

no_module:
halt:
    cli
    hlt
    jmp halt
