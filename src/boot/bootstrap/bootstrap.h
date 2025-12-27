#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

typedef void (*initcall_t)(void);

typedef struct {
    initcall_t fn;
    int order;
} init_entry_t;

#define REGISTER_ORDER(func, ord)                                \
    static init_entry_t __initcall_##func                        \
    __attribute__((section(".initcalls"), used)) = {             \
        .fn = func,                                              \
        .order = ord                                             \
    };

void GlobalInitEarly(void);
#endif