#include <stdint.h>
#include <keyboard/keyboard.h>
#include <idt/idt.h>
#include <vga/vga.h>
#include <pic.h>
#include <locks/spinlock.h>
#include <io.h>

#define KBD_BUF_SIZE 128

static uint8_t scancodes[KBD_BUF_SIZE];
static SpinLock_t kbd_lock = SPINLOCK_INIT;
static uint8_t shift = 0, ctrl = 0, alt = 0;
static uint8_t head = 0;
static uint8_t tail = 0;

struct KeyBoardStatus_t GetKeyboardStatus() {
    struct KeyBoardStatus_t status = { .shift = shift, .ctrl = ctrl, .alt = alt };
    return status;
}


void keyboard_callback() {
    SpinLockIrq(&kbd_lock);

    uint8_t sc = inb(0x60);
    uint8_t pressed = !(sc & 0x80); // 1 se key press, 0 se release
    uint8_t key = sc & 0x7F;        // scancode puro

    // Atualiza modificadores
    switch(key) {
        case KEY_LSHIFT:
        case KEY_RSHIFT: shift = pressed; break;
        case KEY_CTRL:   ctrl  = pressed; break;
        case KEY_ALT:    alt   = pressed; break;
    }

    // Só coloca no buffer se for press e não for modificador
    if (pressed && key != KEY_LSHIFT && key != KEY_RSHIFT && key != KEY_CTRL && key != KEY_ALT) {
        uint8_t next = (head + 1) % KBD_BUF_SIZE;
        if(next != tail) {
            scancodes[head] = key;
            head = next;
        }
    }
    SpinUnlockIrq(&kbd_lock);
    PicSendEoi(1); // envia EOI para a IRQ do teclado
}

key_t KeyGet() {
    SpinLock(&kbd_lock);
    if(head == tail) {
        SpinUnlock(&kbd_lock);
        return 0;
    }
    uint8_t sc = scancodes[tail];
    tail = (tail + 1) % KBD_BUF_SIZE;
    SpinUnlock(&kbd_lock);
    return sc;
}

char ScanAscii(key_t sc)
{
    switch (sc)
    {
        // números linha superior
        case KEY_1: return shift ? '!' : '1';
        case KEY_2: return shift ? '@' : '2';
        case KEY_3: return shift ? '#' : '3';
        case KEY_4: return shift ? '$' : '4';
        case KEY_5: return shift ? '%' : '5';
        case KEY_6: return shift ? '^' : '6';
        case KEY_7: return shift ? '&' : '7';
        case KEY_8: return shift ? '*' : '8';
        case KEY_9: return shift ? '(' : '9';
        case KEY_0: return shift ? ')' : '0';
        case KEY_MINUS: return shift ? '_' : '-';
        case KEY_EQUAL: return shift ? '+' : '=';

        // letras
        case KEY_A: return shift ? 'A' : 'a';
        case KEY_B: return shift ? 'B' : 'b';
        case KEY_C: return shift ? 'C' : 'c';
        case KEY_D: return shift ? 'D' : 'd';
        case KEY_E: return shift ? 'E' : 'e';
        case KEY_F: return shift ? 'F' : 'f';
        case KEY_G: return shift ? 'G' : 'g';
        case KEY_H: return shift ? 'H' : 'h';
        case KEY_I: return shift ? 'I' : 'i';
        case KEY_J: return shift ? 'J' : 'j';
        case KEY_K: return shift ? 'K' : 'k';
        case KEY_L: return shift ? 'L' : 'l';
        case KEY_M: return shift ? 'M' : 'm';
        case KEY_N: return shift ? 'N' : 'n';
        case KEY_O: return shift ? 'O' : 'o';
        case KEY_P: return shift ? 'P' : 'p';
        case KEY_Q: return shift ? 'Q' : 'q';
        case KEY_R: return shift ? 'R' : 'r';
        case KEY_S: return shift ? 'S' : 's';
        case KEY_T: return shift ? 'T' : 't';
        case KEY_U: return shift ? 'U' : 'u';
        case KEY_V: return shift ? 'V' : 'v';
        case KEY_W: return shift ? 'W' : 'w';
        case KEY_X: return shift ? 'X' : 'x';
        case KEY_Y: return shift ? 'Y' : 'y';
        case KEY_Z: return shift ? 'Z' : 'z';

        // símbolos
        case KEY_SEMICOLON: return shift ? ':' : ';';
        case KEY_APOSTROPHE: return shift ? '"' : '\'';
        case KEY_GRAVE: return shift ? '~' : '`';
        case KEY_BACKSLASH: return shift ? '|' : '\\';
        case KEY_COMMA: return shift ? '<' : ',';
        case KEY_DOT: return shift ? '>' : '.';
        case KEY_SLASH: return shift ? '?' : '/';
        case KEY_SPACE: return ' ';  // espaço
        case KEY_LBRACKET: return shift ? '{' : '[';
        case KEY_RBRACKET: return shift ? '}' : ']';

        // Enter, Tab, Backspace
        case KEY_ENTER: return '\n';
        case KEY_TAB: return '\t';
        case KEY_BACKSPACE: return '\b';

        default: return 0; // tecla não mapeada
    }
}

extern void keyboard_handler();
void KeyboardInit()
{
    idt_set_gate(0x21,keyboard_handler,0x8E);
    PicUnmaskIrq(1);
}

#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(KeyboardInit,60);