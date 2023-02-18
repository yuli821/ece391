#include "idt.h"
#include "exceptions.h"
#include "x86_desc.h"
#include "lib.h"
#include "handlers.h"
#include "syscall.h"

static int exception_flag = 0;

//exception message table for display lookup
const char* exception_titles[] = {
    [0]="divide_zero",
    [1]="debug",
    [2]="nonmask_interrupt",
    [3]="breakpoint",
    [4]="overflow",
    [5]="exceeed_bound",
    [6]="bad_opcode",
    [7]="device_busy",
    [8]="double_fault",
    [9]="coprocessor_overrun",
    [10]="invalid_tss",
    [11]="bad_segment",
    [12]="stack_fault", 
    [13]="general_protection",
    [14]="page_fault",
    [15]="reserve",
    [16]="x87_fp",
    [17]="alignment_check",
    [18]="machine_check",
    [19]="simd_fp",
    [20]="virtualization",
    [21]="control_protection",
    [22]="hypervisor_injection",
    [23]="vmm_communication",
    [24]="security",
    [25]="triple_fault"
};

/*idt_init
    in: none
    out: none
    return: none
    effect: populates the idt table and fills it to idt_desc_prt
*/
void idt_init(void) {
    int i;
    for (i = 0; i < NUM_VEC; i++) {
      
      // Boundary check
        if (i < NUM_EXCEPTIONS && i != 15)
        {
            idt[i].present = 1;
        }
        else {
            idt[i].present = 0;
        }
      
        // idt[i].present = 1;
        idt[i].dpl = 0;
        idt[i].size = 1;
        //reserve bits = 01110 for interrupt gate
        idt[i].reserved4 = 0;   
        idt[i].reserved3 = 0; 
        idt[i].reserved2 = 1;
        idt[i].reserved1 = 1;
        idt[i].reserved0 = 0;
        //lower reg high to low
        idt[i].seg_selector = KERNEL_CS;

        //remains to set the offests.
    }

    //exception offsets
    SET_IDT_ENTRY(idt[0], divide_zero);
    SET_IDT_ENTRY(idt[1], debug);
    SET_IDT_ENTRY(idt[2], nonmask_interrupt);
    SET_IDT_ENTRY(idt[3], breakpoint);
    SET_IDT_ENTRY(idt[4], overflow);
    SET_IDT_ENTRY(idt[5], exceeed_bound);
    SET_IDT_ENTRY(idt[6], bad_opcode);
    SET_IDT_ENTRY(idt[7], device_busy);
    SET_IDT_ENTRY(idt[8], double_fault);
    SET_IDT_ENTRY(idt[9], coprocessor_overrun);
    SET_IDT_ENTRY(idt[10],invalid_tss);
    SET_IDT_ENTRY(idt[11],bad_segment);
    SET_IDT_ENTRY(idt[12],stack_fault);
    SET_IDT_ENTRY(idt[13],general_protection);
    SET_IDT_ENTRY(idt[14],page_fault);
    SET_IDT_ENTRY(idt[16],x87_fp);
    SET_IDT_ENTRY(idt[17],alignment_check);
    SET_IDT_ENTRY(idt[18],machine_check);
    SET_IDT_ENTRY(idt[19],simd_fp);
    SET_IDT_ENTRY(idt[20],virtualization);
    //other interrupts

    //reserve bits = 01110 for interrupt gate

    // Keyboard
    idt[KB_IDT].reserved3 = 1;
    idt[KB_IDT].present = 1;
    SET_IDT_ENTRY(idt[KB_IDT], handle_keyboard);
    // RTC
    idt[RTC_IDT].reserved3 = 1;
    idt[RTC_IDT].present = 1;
    SET_IDT_ENTRY(idt[RTC_IDT], handle_rtc);
    //PIT
    idt[PIT_IDT].reserved3 = 1;
    idt[PIT_IDT].present = 1;
    SET_IDT_ENTRY(idt[PIT_IDT], handle_pit);

    idt[SYS_IDT].reserved3 = 0;
    idt[SYS_IDT].present = 1;
    idt[SYS_IDT].dpl = 3;

    SET_IDT_ENTRY(idt[SYS_IDT], system_call);

    lidt(idt_desc_ptr);
}


/*exception_handler
    in: exception vector
    out: none
    return: none
    effect: prints exception message and a kitten. holds afterward
*/
void exception_handler(int vect) {

    exception_flag = 1;

    printf("Vector: %x, Exception: %s\n", vect, exception_titles[vect]);

    //while(1);

}

int get_exception_fl() {
    return exception_flag;
}

