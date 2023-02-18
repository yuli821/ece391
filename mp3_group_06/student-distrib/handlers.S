#define ASM       1
#define KB_IDT 0x21
#define RTC_IDT 0x28

#define HANDLER(name,handler,irqnum)   \
.globl name                    ;\
name:                           ;\
    pushl $0                    ;\
    pushl $irqnum               ;\
    pushal                      ;\
    call handler                ;\
    popal                       ;\
    addl $8, %esp               ;\
    iret                        

# keyboard interrupt entry index: 0x21
HANDLER(handle_keyboard,keyboard_handler, 0x21)
# rtc interrupt entry index: 0x28
HANDLER(handle_rtc,rtc_handle,0x28)
HANDLER(handle_pit,pit_handler,0x20)
