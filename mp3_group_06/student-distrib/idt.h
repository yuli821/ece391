#ifndef _IDT_H
#define _IDT_H

#ifndef ASM
#include "lib.h"
//#include "exceptions.h"

//magic numbers
#define PIT_IDT 0x20
#define KB_IDT 0x21
#define RTC_IDT 0x28
#define SYS_IDT 0x80
#define NUM_EXCEPTIONS 21

void idt_init(void);
extern void exception_handler(int vect);
uint8_t is_modifier(uint8_t scan_code);

int get_exception_fl();

#endif
#endif
