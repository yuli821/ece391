#ifndef HANDLERS_H
#define HANDLERS_H
#include "keyboard.h"
#include "rtc.h"

extern void handle_keyboard(void);
extern void handle_rtc(void);
extern void handle_pit(void);

#endif
