#define ASM     1

#define EXCEPTION(name, id)         \
.globl  name                        ;\
.align  4                           ;\
name:                               ;\
    pushfl                          ;\
    pushal                          ;\
    pushl   $id                     ;\
    call    exception_handler       ;\
    addl    $4, %esp                ;\
    popal                           ;\
    popfl                           ;\
    iret

#define EXCEPTION_DUMMY(name, id)         \
.globl  name                        ;\
.align  4                           ;\
name:                               ;\
    pushl   $0                      ;\
    pushfl                          ;\
    pushal                          ;\
    pushl   $id                     ;\
    call    exception_handler       ;\
    addl    $4, %esp                ;\
    popal                           ;\
    popfl                           ;\
    addl    $4, %esp                ;\
    iret

EXCEPTION(divide_zero,0x00);
EXCEPTION(debug,0x01);
EXCEPTION(nonmask_interrupt,0x02);
EXCEPTION(breakpoint,0x03);
EXCEPTION(overflow,0x04);
EXCEPTION(exceeed_bound,0x05);
EXCEPTION(bad_opcode,0x06);
EXCEPTION(device_busy,0x07);
EXCEPTION_DUMMY(double_fault,0x08);
EXCEPTION(coprocessor_overrun,0x09);
EXCEPTION(invalid_tss,0x0A);
EXCEPTION(bad_segment,0x0B);
EXCEPTION(stack_fault,0x0C);
EXCEPTION(general_protection,0x0D);
EXCEPTION(page_fault,0x0E);
EXCEPTION_DUMMY(x87_fp,0x10);
EXCEPTION(alignment_check,0x11);
EXCEPTION_DUMMY(machine_check,0x12);
EXCEPTION_DUMMY(simd_fp,0x13);
EXCEPTION_DUMMY(virtualization,0x14);


