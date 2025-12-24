/*
• Tasks são curtas
• Tasks não bloqueiam
• Tasks cooperam
• Sem preempção
*/

#ifndef TIMER_H
#define TIMER_H
#include <idt/idt.h>
#include <stdint.h>

struct TimerCtx_t {
    void (*func)(void);
    uint8_t init;
    uint8_t runner;
    uint32_t period;
    uint32_t remaining;
} __attribute__((packed));
struct Task_t {
    struct isr_context_t regs;
    uint8_t* stack;
    int ruuner;
    void (*func)(struct Task_t*);
};

void TimerInit();
void TimerRunTasks();
void TimerPoll();
uint32_t RegistreQuest(struct TimerCtx_t* ctx);
#endif