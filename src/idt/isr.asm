[bits 32]
global isr_page_fault
global lapic_timer_handler
extern page_fault

isr_page_fault:
    cli                 ; PF pode reentrar, corta isso
    pusha               ; salva regs (32 bytes)
    push esp            ; passa a esp como paramentro
    call page_fault ; definindo em src/mmu/mmu.c
    add esp, 4          ; limpa args
    popa
    iretd
global pit_timer_handler
extern pit_handler
pit_timer_handler:
    cli
    pusha
    call pit_handler ; definido em src/timer/timer.c
    popa
    sti
    iretd
;global yield_timer_handler
;extern yield_handler
;yield_timer_handler:
;    cli                     ; bloqueia interrupções
;    push esp                 ; passa esp da task atual
;    call yield_handler       ; salva contexto e escolhe próxima task
;    add esp, 4               ; limpa parâmetro da pilha
;    pusha                    ; salva registradores da task selecionada
;    pushfd                   ; salva flags da task selecionada
;    popfd                    ; restaura flags
;    popa                     ; restaura registradores
;    iretd                    ; retorna para EIP/CS/FLAGS da task escolhida
global invalid_opcode_handler
extern invalid_opcode
invalid_opcode_handler:
    cli
    push 0
    pusha
    push esp
    call invalid_opcode ; definido em src/idt/idt.c
    add esp, 4
    popa
    add esp, 4
    iretd
global keyboard_handler
extern keyboard_callback
keyboard_handler:
    pusha
    call keyboard_callback ; definido em src/keyboard/keys.c
    popa
    iretd
global mouse_handler
extern mouse_callback
mouse_handler:
    cli
    pusha
    call mouse_callback
    popa
    sti
    iretd