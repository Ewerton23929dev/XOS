#include <io.h>
#include <console/console.h>
#include <stdint.h>

extern uint8_t __programs_start[];
extern uint8_t __programs_end[];

#define PRG_MAGIC 0x5047524D  // "PRGM"
typedef struct {
    uint32_t magic;   // magic
    uint32_t entry;   // offset a partir do início do programa
    uint32_t size;    // tamanho TOTAL do programa em bytes
    char name[16]; // nome
} prg_header_t;
typedef void (*entry_fn_t)(void*);

void ProgramPrintUser(uint32_t offset)
{
    char* base = (char*)__programs_start;
    char* s = base + offset;

    for (int i = 0; i < 256; i++) {
        char c = s[i];
        if (c == 0) break;
        ConsolePutRaw(c);
    }
}

void* kernel_api[2] = {
    [0] = ProgramPrintUser,
    [1] = ConsolePutRaw
};

uint8_t* current_program_base = NULL;
void LoaderProgram(const char* name)
{
    uint8_t* p = __programs_start;
    while (p < __programs_end) {
        prg_header_t* hdr = (prg_header_t*)p;
        if (hdr->magic != PRG_MAGIC) return;
        if (hdr->size == 0 || hdr->size > 0x100000) return;
        if (strncmp(hdr->name, name, 16) == 0) {
            entry_fn_t entry = (entry_fn_t)(p + hdr->entry);
            current_program_base = p;
            ConsolePrint("\nRunning program...\n");
            asm volatile (
                "mov %0, %%eax \n"
                "call *%1       \n"
                :
                : "r"(kernel_api), "r"(entry)
                : "eax"
            );
            continue;
        }
        p += hdr->size;
    }
    ConsolePrint("\nProgram not found\n");
}

void CmdLoader(size_t argc, const char* argv[])
{
    LoaderProgram(argv[0]);
}

void ProgramInit()
{
    ConsoleRegisterCommand("load",CmdLoader,"Load simples programs");
}