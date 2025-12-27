#include <stdint.h>
#include <idt/idt.h>
#include <timer/timer.h>
#include <io.h>
#include <memory/slab.h>
#include <locks/spinlock.h>
#include <pic.h>

#define QUEST_TIMER 900
#define MAX_TASKS 900
#define TASK_STACK_SIZE 4090
static SpinLock_t timer_lock = SPINLOCK_INIT;
static volatile uint32_t timer_count = 0;
static struct TimerCtx_t quest_arry[QUEST_TIMER];
static uint32_t quest_count = 0;

uint32_t RegistreQuest(struct TimerCtx_t* ctx)
{
    SpinLock(&timer_lock);
    if (ctx->period == 0) ctx->period = 1;
    if (ctx->remaining == 0) ctx->remaining = ctx->period;
    if (quest_count >= QUEST_TIMER) return (uint32_t)-1;
    quest_arry[quest_count] = *ctx;
    quest_arry[quest_count].init = 1;
    uint32_t id = quest_count++;
    SpinUnlock(&timer_lock);
    return id; // return offset to remove...
}

void RemoveQuest(uint32_t offset_quest)
{
    SpinLock(&timer_lock);
    quest_arry[offset_quest].init = 0;
    quest_arry[offset_quest].func = 0;
    quest_arry[offset_quest].runner = 0;
    SpinUnlock(&timer_lock);
}

// Entrada principal do ASM IRQ PIC
void pit_handler(struct isr_context_t* ctx)
{
    timer_count++;
    PicSendEoi(0);
}

static uint32_t last_tick = 0;
void TimerPoll()
{
    if (last_tick == timer_count) return;
    last_tick = timer_count;
    for (int i = 0; i < quest_count; i++) {
        if (!quest_arry[i].init) continue;
        if (--quest_arry[i].remaining == 0) {
            quest_arry[i].remaining = quest_arry[i].period;
            quest_arry[i].runner = 1;
        }
    }
}

static int last = 0;
void TimerRunTasks()
{
    for (int n = 0; n < quest_count; n++) {
        int i = (last + n) % quest_count;
        if (!quest_arry[i].func) continue;
        if (!quest_arry[i].runner) continue;
        SpinLockIrq(&timer_lock);
        if (!quest_arry[i].runner) {
            SpinUnlock(&timer_lock);
            continue;
        }
        quest_arry[i].runner = 0;
        last = i + 1;
        SpinUnlock(&timer_lock);
        quest_arry[i].func();
        break;
    }
}

extern void pit_timer_handler();
extern void yield_timer_handler();
void TimerInit()
{
    idt_set_gate(0x20, pit_timer_handler, 0x8E); 
    PicUnmaskIrq(0);
}

#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(TimerInit,50);