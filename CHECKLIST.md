# CheckList / RoadMap

[Monolotico](https://wiki.osdev.org/Monolithic_Kernel)
[MicroKernel](https://wiki.osdev.org/Microkernel)
[Exokernel](https://wiki.osdev.org/Exokernel)
[Modulerkernel](https://wiki.osdev.org/Modular_Kernel)

# Oque tenho no começo:

- Alocador simples via Heap
- Sistema de Quest para PIC, simples
- Sem VGA serio
- IDT simples nao pronta
- Keyboard PIC simples, sem recursos de indetificaçao de combo serio
- MMU simples...API quebrada
- GDT basica definida no boot, asm

# Oque tenho agora:

- Começo

----
# Faze QEMU:
## Oque devo fazer:

- [ ] *Melhore e fature oque voce ja tem...*
- [x] Fazer sistema de VGA API...
- [ ] Adicionar IRQ importantes na IDT...
- [ ] Controlar dados recebidos via GRUB...
- [ ] Fazer sistema de alocação com kmalloc e kfree...subistiuir sistema de HEAP
- [ ] Fazer sistema de SERIAL DEBUG...com VGA e COM1
- [ ] Adicionar API de Logging e Debug...panic, printk, outros...
- [ ] 1: *Ajuste codigo... Alocador medio, VGA, SERIAL DEBUG*
- [ ] Melhorar sistema de allocação...adicione slab e outros...
- [x] Melhorar a API da MMU
- [x] Melhorar a API do KeyBoard...saida de teclas especias e comuns, melhorar GetKey.
- [ ] Fazer multi-tasking simples...com stack e registradores com PIC ou LAPIC.
- [ ] Fazer sistema de LAPIC funcional para subistituir PIC(MANTER)...
- [ ] 2: *Ajuste codigo... Nessesario: Alocador, VGA, SERIAL DEBUG, LOGGIN, TASK Simples*
- [ ] Projetar Terminal...juntar LAPIC + TASK + VGA + BUFFER_LINE
- [ ] Expandir para tentiva de GUI em VGA...estilo Window 1.0
- [ ] Implementar um sistema de ELF ou formato proprio...algo basico
- [ ] 3: *Ajuste codigo... Nessesario: ELF, Alocador, VGA, SERIAL DEBUG, LOGGIN, TASK*
- [ ] Melhorar sistema de ELF/Formato proprio...adicione uma real API de controle...
- [ ] Implementar sistema minimo de driver ELF/Formato Proprio como driver modular...
- [ ] 4: *Ajuste codigo... Alocador, VGA, SERIAL DEBUG, LOGGIN, TASK*
- [ ] Implementar um FileSystem simples...apenas o funcional
- [ ] Implementar um [Sistema de Rede](https://wiki.osdev.org/Network_Stack) simples...apenas o funcional
- [ ] Melhorar sistema de FileSystem...adicione uma real percistencia.
- [ ] Melhorar sistema de rede...suporte a HTTP.
- [ ] 5: *Ajuste codigo... Alocador, VGA, SERIAL DEBUG, LOGGIN, TASK*
- [ ] Planejamento para SMP...adicione locks simples, spinlocks, atomic operations.
- [ ] 6: *Ajuste codigo... Alocador, VGA, SERIAL DEBUG, LOGGIN, TASK, FS*
- [ ] Implemntar [SMP](https://wiki.osdev.org/Symmetric_Multiprocessing) funcional simples...
- [ ] Fazer uma GDT seria para o kernel...atualmente e apenas simples via boot.asm
- [ ] Implementar sistema de AUDIO...simples
- [ ] Evoluir sistema de AUDIO.
