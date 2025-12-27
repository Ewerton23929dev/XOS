#include <io.h>
#include <timer/timer.h>
#include <vga/vga.h>
#include <vga/cursor.h>
#include <keyboard/keyboard.h>
#include <console/console.h>
#include <locks/spinlock.h>

#define MAX_HISTORY 200

/*
screen_y: início da linha do prompt
cursor_y: quantas linhas de input já quebrou
posição real do cursor: screen_y + cursor_y
posição real do X: cursor_x + prompt_size, para quebrar, cursor_x - prompt_size
*/
struct CursorPos {
    uint32_t x;
    uint32_t y;
};

static uint32_t screen_x = 0; // global
static uint32_t screen_y = 0; // global
static uint32_t cursor_x = 0;
static uint32_t cursor_y = 0;
static SpinLock_t console_lock = SPINLOCK_INIT;
static char msg_buffer[900];
static size_t msg_index = 0;
size_t prompt_size = 0;
static char history[MAX_HISTORY][900];
static size_t history_count = 0;
static int history_index = -1;
#define MAX_ARGS 16

#define MAX_COMMANDS 20
static struct ConsoleComand_t commands[MAX_COMMANDS];
static size_t commands_count = 0;
#define MAX_ARG_LEN 128

size_t ConsoleParseArgs(char* input, char* argv[]) {
    size_t argc = 0;
    while (*input && argc < MAX_ARGS) {
        while (*input == ' ') input++;
        if (*input == '\0') break;
        argv[argc++] = input;
        while (*input && *input != ' ') input++;

        if (*input == ' ') {
            *input = '\0';
            input++;
        }
    }
    return argc;
}

static void ConsoleAddHistory(const char* cmd)
{
    if (history_count >= MAX_HISTORY) {
        for (size_t i = 1; i < MAX_HISTORY; i++)
            memset(history[i-1], 0, 900), memset(history[i-1], 0, 900), memcpy(history[i-1], history[i], 900);
        history_count = MAX_HISTORY - 1;
    }
    memset(history[history_count], 0, 900);
    size_t i = 0;
    while (cmd[i] != '\0') { history[history_count][i] = cmd[i]; i++; }
    history_count++;
    history_index = history_count;
}
static char* ConsoleGetHistory(int direction)
{
    if (history_count == 0) return NULL;

    if (direction == -1) { // seta ↑
        if (history_index > 0) history_index--;
    } else if (direction == 1) { // seta ↓
        if (history_index < (int)history_count - 1) history_index++;
        else return NULL; // além do último, retorna nada
    }

    return history[history_index];
}
void ConsoleRegisterCommand(const char* name, CommandFunc_t func, const char* help)
{
    if (commands_count >= MAX_COMMANDS) return;
    commands[commands_count].name = name;
    commands[commands_count].func = func;
    commands[commands_count].help = help;
    commands_count++;
}









static void ConsolePrompt() {
    const char* prompt = "[KERNEL] >> ";
    size_t prompt_len = 0;
    struct VgaLine_t line;
    screen_y += cursor_y;
    cursor_y = 0;
    cursor_x = 0;
    screen_x = 0;

    VGAGetPhyLine(screen_y, &line);
    for (size_t i = 0; ; i++) {
        char c = prompt[i];
        if (c == '\0') break;
        VGALineWriteCaracter(i, &line, c, 0x0F);
        prompt_len++;
    }
    prompt_size = prompt_len;
    VGACursorSetPose(screen_y, prompt_size);
}
static void ConsoleClearScreen() {
    for (uint32_t y = 0; y < VGA_H; y++) {
        struct VgaLine_t line;
        VGAGetPhyLine(y, &line);
        for (uint32_t x = 0; x < VGA_W; x++) {
            VGALineWriteCaracter(x, &line, ' ', 0x0F);
        }
    }
    screen_x = 0;
    screen_y = 0;
    cursor_x = 0;
    cursor_y = 0;
    ConsolePrompt();
}
void ConsolePutRaw(char c)
{
    if (c == '\n') {
        screen_x = 0;
        screen_y++;
        VGACursorSetPose(screen_y,screen_x);
        return;
    }
    if (screen_y + cursor_y >= VGA_H) {
        ConsoleClearScreen();
    }

    if (screen_x >= VGA_W) {
        screen_y++;
        screen_x = 0;
        VGACursorSetPose(screen_y,screen_x);
        return;
    }
    if (screen_y >= VGA_H) {
        screen_y = VGA_H - 1;
    }
    struct VgaLine_t line;
    VGAGetPhyLine(screen_y,&line);
    VGALineWriteCaracter(screen_x,&line,c,0x0F);
    screen_x++;
    VGACursorSetPose(screen_y,screen_x);
}
void ConsoleInputChar(char c) {
    if (cursor_x + prompt_size >= VGA_W) {
        cursor_x = 0;
        cursor_y++;
        if (screen_y + cursor_y >= VGA_H) return; // limite de tela
    }
    if (screen_y + cursor_y >= VGA_H) {
        ConsoleClearScreen();
    }

    struct VgaLine_t line;
    VGAGetPhyLine(screen_y + cursor_y, &line);
    VGALineWriteCaracter(cursor_x + prompt_size, &line, c, 0x0F);
    cursor_x++;
    VGACursorSetPose(screen_y + cursor_y, cursor_x + prompt_size);
}
void ConsolePrint(const char* msg)
{
    for (int i = 0; msg[i] != '\0'; i++) {
        ConsolePutRaw(msg[i]);
    }
}
static struct CursorPos CalcCursorPos(uint32_t cursor_x, uint32_t cursor_y) {
    struct CursorPos pos;
    uint32_t total_index = cursor_x + cursor_y * VGA_W;
    pos.y = screen_y + (total_index / VGA_W);
    pos.x = (total_index % VGA_W) + (cursor_y == 0 ? prompt_size : 0);
    if (pos.y >= VGA_H) pos.y = VGA_H - 1;
    return pos;
}
static void ConsoleBackspace() {
    if (msg_index == 0) return;

    msg_index--;
    cursor_x--; // atualiza posição interna do buffer

    struct CursorPos pos = CalcCursorPos(cursor_x, cursor_y);
    struct VgaLine_t line;
    VGAGetPhyLine(pos.y, &line);
    VGALineWriteCaracter(pos.x, &line, ' ', 0x0F); // apaga caractere
    VGACursorSetPose(pos.y, pos.x);
}
void ConsoleTrace()
{
    SpinLock(&console_lock);
    key_t tecla = KeyGet();
    if (tecla == 0) {
        SpinUnlock(&console_lock);
        return;
    }
    if (tecla == KEY_UP) {
        char* cmd = ConsoleGetHistory(-1);
        if (cmd) {
            while (cursor_x > 0) ConsoleBackspace();
            size_t i = 0;
            while (cmd[i] != '\0') {
                ConsoleInputChar(cmd[i]);
                msg_buffer[i] = cmd[i];
                i++;
            }
            msg_index = strlen(cmd);
        }
        SpinUnlock(&console_lock);
        return;
    }
    if (tecla == KEY_DOWN) {
        char* cmd = ConsoleGetHistory(1);
        if (cmd) {
            while (cursor_x > 0) ConsoleBackspace();
            size_t i = 0;
            while (cmd[i] != '\0') {
                ConsoleInputChar(cmd[i]);
                msg_buffer[i] = cmd[i];
                i++;
            }
            msg_index = strlen(cmd);
        }
        SpinUnlock(&console_lock);
        return;
    }
    if (tecla == KEY_ENTER){
        if (msg_index == 0) {
            SpinUnlock(&console_lock);
            return;
        }
        if (msg_buffer[0] == 0 && msg_buffer[0] == ' ') {
            SpinUnlock(&console_lock);
            return;
        }
        VGACursorVisible(0);
        msg_buffer[msg_index] = '\0'; // delimitador
        ConsoleAddHistory(msg_buffer);
        char* argv[MAX_ARGS];
        size_t argc = ConsoleParseArgs(msg_buffer,argv);
        if (argc == 0) return;
        for (size_t i = 0; i < commands_count; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                commands[i].func(argc-1,(const char**)&argv[1]);
                break;
            }
        }
        screen_y += 1;
        cursor_x = 0;
        cursor_y = 0;
        msg_index = 0; // reseta
        ConsolePrompt();
        VGACursorVisible(1);
        SpinUnlock(&console_lock);
        return;
    }
    if (tecla == KEY_BACKSPACE) {
        ConsoleBackspace();
        SpinUnlock(&console_lock);
        return;
    }
    if (msg_index >= 900) {
        SpinUnlock(&console_lock);
        return;
    }
    char c = ScanAscii(tecla);
    ConsoleInputChar(c);
    msg_buffer[msg_index] = c;
    msg_index++;
    SpinUnlock(&console_lock);
}

static void CmdPrint(size_t argc, const char* argv[]) {
    if (argc < 0) return;
    ConsolePrint("\n");
    for (size_t i = 0; i < argc; i++) {
        ConsolePrint(argv[i]);
        if (i < argc - 1) ConsolePrint(" ");
    }
}
static void CmdHelp(size_t argc, const char* argv[]) {
    ConsolePrint("\n");
    if (argc > 0) {
        for (size_t i = 0; i < commands_count; i++) {
            if (strcmp(argv[0], commands[i].name) == 0) {
                ConsolePrint(commands[i].help);
                return;
            }
        }
    }
    for (size_t i = 0; i < commands_count; i++) {
        ConsolePrint(commands[i].name);
        ConsolePrint(" - ");
        ConsolePrint(commands[i].help);
        ConsolePrint("\n");
    }
}
static void CmdClear(size_t argc, const char* argv[]) {
    ConsoleClearScreen();
}
#include <cpu/cpu.h>
static void CmdReboot(size_t argc, const char* argv[])
{
    CpuReboot();
}
static void CmdShow(size_t argc, const char* argv[])
{
    if (argc == 0 ||strcmp(argv[0],"help") == 0) {
        ConsolePrint("\ncpu - Show name cpu");
        return;
    }
    if (strcmp(argv[0],"cpu") == 0) {
        ConsolePrint("\n");
        CpuPrintVendor();
        return;
    }
    ConsolePrint("\nDont have this device/module!");
    return;
}
static void CmdOff(size_t argc,const char* argv[])
{
    CpuPowerOff();
}
void ConsoleInit()
{
    ConsolePrint("XOS 0.0.98 - Shell Interative\n");
    ConsolePrompt();
    struct TimerCtx_t console;
    console.func = ConsoleTrace;
    console.period = 4;
    console.remaining = 2;
    RegistreQuest(&console);
    ConsoleRegisterCommand("print",CmdPrint,"Print a message");
    ConsoleRegisterCommand("help",CmdHelp,"Show this help");
    ConsoleRegisterCommand("clear", CmdClear, "Clear the screen");
    ConsoleRegisterCommand("reboot",CmdReboot,"Reboot your system now");
    ConsoleRegisterCommand("show",CmdShow,"Show data machine");
    ConsoleRegisterCommand("poweroff",CmdOff,"Poweroff your pc!");
}

#include <boot/bootstrap/bootstrap.h>
REGISTER_ORDER(ConsoleInit,80);