/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"
#include "lib.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask = 0xff; /* IRQs 0-7  */
uint8_t slave_mask = 0xff;  /* IRQs 8-15 */
/* i8259_init
 * Initialize the 8259 PIC
 * Inputs: None
 * Outputs: None
 * Side Effects: Mask all interrupts and tell PIC it is in cascade mode
 */
void i8259_init(void) {
    // primary PIC
    outb(ICW1, MASTER_8259_PORT); //first PIC port,  tells the PIC that it is being initialized, that it should use edge-triggered input signals, that it is operating in cascade mode
    outb(ICW2_MASTER, MASTER_DATA);
    outb(ICW3_MASTER, MASTER_DATA);
    outb(ICW4, MASTER_DATA);

    // secondary PIC
    outb(ICW1, SLAVE_8259_PORT);
    outb(ICW2_SLAVE, SLAVE_DATA);
    outb(ICW3_SLAVE, SLAVE_DATA);
    outb(ICW4, SLAVE_DATA);

    //mask all interrupts
    outb(0xff, MASTER_DATA);
    outb(0xff, SLAVE_DATA);

    enable_irq(SLAVE_IRQ); 
}

/* enable_irq
 * Enable (unmask) the specified IRQ.
 * Inputs: irq_num: primary PIC(0 - 7), secondary (8 - 15) 
 * Outputs: None
 */
void enable_irq(uint32_t irq_num) {
    //check if the irq num is valid

    if((irq_num < 0) | (irq_num > 15)) return; // the irq number should within range (0-15)

    uint16_t port;
 
    if(irq_num < MAX_MASTER_IRQ_NUM) {
        port = MASTER_DATA;
        master_mask &= ~(1 << irq_num);
        outb(master_mask, MASTER_DATA);
    } else {
        port = SLAVE_DATA;
        slave_mask &= ~(1 << (irq_num - MAX_MASTER_IRQ_NUM));         
        outb(slave_mask, SLAVE_DATA);     
    }

}

/* disable_irq
 * Disable (mask) the specified IRQ.
 * Inputs: irq_num: primary PIC(0 - 7), secondary (8 - 15) 
 * Outputs: None
 */
void disable_irq(uint32_t irq_num) {
    //check if the irq num is valid
    if((irq_num < 0) | (irq_num > 15)) return; // the irq number should within range (0-15)

    uint16_t port;
 
    // assign the correspond data port
    if(irq_num< MAX_MASTER_IRQ_NUM) {
        port = MASTER_DATA;
        master_mask |= 1 << irq_num;
        outb(master_mask, MASTER_DATA);       
    } 
    else {
        port = SLAVE_DATA;
        slave_mask |= 1 << (irq_num - MAX_MASTER_IRQ_NUM); 
        outb(slave_mask, SLAVE_DATA);     
    }
}

/* send_eoi
 * Send end-of-interrupt signal for the specified IRQ
 * Inputs: irq_num: primary PIC(0 - 7), secondary (8 - 15) 
 * Outputs: None
 */
void send_eoi(uint32_t irq_num) {
    //printf("hello eoi");
    //check if the irq num is valid
    if((irq_num < 0) | (irq_num > 15)) return;// the irq number should within range (0-15)

    if(irq_num< MAX_MASTER_IRQ_NUM) {
        outb((EOI | irq_num), MASTER_8259_PORT);
    } 
    else {
        outb((EOI | SLAVE_IRQ), MASTER_8259_PORT);
        outb((EOI | (irq_num - 8)), SLAVE_8259_PORT);
    }
}
