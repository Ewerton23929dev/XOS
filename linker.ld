ENTRY(_start)

SECTIONS {
    kernel_start = .;
    . = 1M;  /* Ser nao for 1M a MMU quebra */
    .multiboot ALIGN(8) :
    {
        KEEP(*(.multiboot2))
    }
    .text : {
        text_start = .;
        *(.text)
        text_end = .;
    }

    .data : {
        data_start = .;
        *(.data)
        data_end = .;
    }

    .bss : {
        bss_start = .;
        *(.bss)
        *(COMMON)
        bss_end = .;
    }
    .programs : ALIGN(16) {
        __programs_start = .;
        KEEP(*(.programs))
        __programs_end = .;
    }
    .gzm (NOLOAD) : ALIGN(16) {
        gzm_start = .;
        . += 256K;
        gzm_end = .;
    }
    kernel_end = .;
}