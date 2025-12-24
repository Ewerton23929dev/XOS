#ifndef CONSOLE_H
#define CONSOLE_H
#include <io.h>
#include <stdint.h>

typedef void (*CommandFunc_t)(size_t argc, const char* argv[]);
struct ConsoleComand_t {
    const char* name;
    CommandFunc_t func;
    const char* help;
};

void ConsoleInit();
void ConsolePutRaw(char c);
void ConsolePrint(const char* msg);
void ConsoleRegisterCommand(const char* name, CommandFunc_t func, const char* help);
#endif