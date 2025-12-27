#include <boot/bootstrap/bootstrap.h>

extern init_entry_t __initcalls_start[];
extern init_entry_t __initcalls_end[];

void GlobalInitEarly(void)
{
    asm volatile("cli");

    int count = __initcalls_end - __initcalls_start;
    init_entry_t* table = __initcalls_start;

    // ordena por ordem
    for (int i = 0; i < count - 1; i++) {
        for (int j = i + 1; j < count; j++) {
            if (table[j].order < table[i].order) {
                init_entry_t tmp = table[i];
                table[i] = table[j];
                table[j] = tmp;
            }
        }
    }

    // executa
    for (int i = 0; i < count; i++) {
        table[i].fn();
    }
}