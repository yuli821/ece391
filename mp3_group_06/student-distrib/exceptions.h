#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include "idt.h"

void divide_zero(void);
void debug(void);
void nonmask_interrupt(void);
void breakpoint(void);
void overflow(void);
void exceeed_bound(void);
void bad_opcode(void);
void device_busy(void);
void double_fault(void);
void coprocessor_overrun(void);
void invalid_tss(void);
void bad_segment(void);
void stack_fault(void);
void general_protection(void);
void page_fault(void);
void x87_fp(void);
void alignment_check(void);
void machine_check(void);
void simd_fp(void);
void virtualization(void);

#endif
