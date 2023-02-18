#include "pit.h"

/* pit_init
 * Initialize PIT
 * Inputs: None
 * Outputs: None
 * Side Effects: Set the default frequency to 100 Hz
 */
void pit_init() {
    
    //frequency = 1193182 / reload_value Hz
    uint32_t desired_freq = 100;
    uint32_t reload_value = MAX_PIT_FREQ / desired_freq;

    //seperate the reload_value into two parts
    uint8_t low = (uint8_t) (reload_value & BYTE_MASK);
    uint8_t high = (uint8_t) ((reload_value >> 8) & BYTE_MASK);

    //set freq: channel 0, lobyte/hibyte, rate generator
    outb(0x36, CM_REGISTER);

    //send reload value in two parts
    outb(low, CHANNEL_ZERO_PORT);
    outb(high, CHANNEL_ZERO_PORT);

    enable_irq(PIT_IRQ);
}

/* pit_handle
 * Handle PIT interrupts
 * Inputs: None
 * Outputs: None
 * Side Effects: when receive an interrupt by PIT, switch to the active terminal
 */
void pit_handler() {
    cli();
    send_eoi(PIT_IRQ);
    int32_t new_term = (active_tid + 1) % 3;
    terminal_switch_active(new_term);
    sti();
    // cli();
    // send_eoi(PIT_IRQ);
    // pcb_t* curr_pcb;
    // if (term_count != 0) {
    //     curr_pcb = get_pcb();                     //get pcb for the process we are going to leave
    //     register uint32_t ebp asm("ebp");                //get current esp and ebp
    //     register uint32_t esp asm("esp");
    //     terminal_array[curr_active_term].esp = esp;
    //     terminal_array[curr_active_term].ebp = ebp;
    //     terminal_array[curr_active_term].esp0 = tss.esp0;
    //     terminal_array[curr_active_term].process_pcb = curr_pcb;
    //     curr_pcb->saved_esp = esp;                       //update and store the esp and ebp for current process
    //     curr_pcb->saved_ebp = ebp;
    //     curr_active_term = (curr_active_term+1) % 3;
    //     //changeMapping(curr_active_term);
    // }
    // if (term_count < 3) {
    //     term_count++;
    //     execute((uint8_t*)"shell");
    // }
    // uint32_t new_esp = terminal_array[curr_active_term].esp;
    // uint32_t new_ebp = terminal_array[curr_active_term].ebp;
    // curr_pcb = terminal_array[curr_active_term].process_pcb;
    // tss.ss0 = KERNEL_DS;
    // tss.esp0 = terminal_array[curr_active_term].esp0;
    // pde[USER_INDEX].addr = (USER_MEM_START + curr_pcb->pid * PAGE_KERNEL)>>PAGE_RESERVED;//8MB + pid * 4MB
    // flushTLB();
    // sti();
    // asm volatile(
    //     "movl %0, %%esp   ;"
    //     "movl %1, %%ebp   ;"
    //     :
    //     :"r"(new_esp), "r"(new_ebp)
    //     :"cc"
    // );
}
