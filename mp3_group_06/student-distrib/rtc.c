#include "rtc.h"
#include "lib.h"
//#include <stdint.h>

//char rtc_flag;
//int curr_count;
//int max_count;

/* rtc_init
 * Initialize RTC
 * Inputs: None
 * Outputs: None
 * Side Effects: Set the default frequency, enable interrupts
 */
void rtc_init(){
    char prev;
    // select register B, and disable NMI
    outb(REG_B, RTC_PORT);

    // read the current value of register B
    prev = inb(RTC_DATA);

    // set the index again (a read will reset the index to register D)
    outb(REG_B, RTC_PORT);

    // write the previous value ORed with 0x40. This turns on bit 6(0x40) of register B
    outb(prev | 0x40, RTC_DATA);

    //virtualization
    char rate = 6;//set real frequency to 1024Hz(rate = 6)

    rate &= 0x0F;    // rate must be above 2 and not over 15

    cli();
    // set index to register A, disable NMI
    outb(REG_A, RTC_PORT);
    // get initial value of register A
    prev = inb(RTC_DATA);
    // reset index to A
    outb(REG_A, RTC_PORT);
    //write only our rate to A. Note, rate is the bottom 4 bits.
    outb((prev & 0xF0) | rate, RTC_DATA);

    //enable irq8(RTC irq) on pic
    enable_irq(RTC_IRQ);

    sti();


}

/* rtc_set_freq
 * Set virtual frequency of the RTC to the arg value
 * Inputs: freq -  the virtual frequency we want to set for RTC
 * Outputs: 0 - set freq sucessfully; -1 - fail to set the freq
 * Side Effects: No thing happen if freq is out of range(2 - 1024).
 */
int32_t rtc_set_freq(int32_t freq){
    cli();
    pcb_t* curr_pcb = get_active_pcb();
    //check if input freq is in range(2-1024)
    if ((freq < MIN_FREQ) || (freq > MAX_FREQ)) return -1;

    //set initial freq
    char log2_freq = log2(freq);// rate of this frequency(2(2) - 10(1024))

    //check input freq is the power of 2
    if (log2_freq == -1) return -1;

    //max_count = MAX_FREQ / freq / 2; //virtual rate = 1024Hz / freq(Hz)
    curr_pcb->rtc_val = MAX_FREQ / freq;

    sti();
    return 0;
}

/* rtc_handle
 * Handle RTC interrupts
 * Inputs: None
 * Outputs: None
 * Side Effects: Use test_interrupt to test RTC, if RTC is set up successfully, 
 * the screen should flash with changing garbled characters. Set rtc_flag to 1
 * when the count reach.
 */
//notice: if register C is not read after an IRQ 8, then the interrupt will not happen again. 
void rtc_handle(){
    //virtualization
    // if (curr_count == 0){
    //     rtc_flag = 1;
    //     curr_count = max_count;
    // }
    // else{
    //     rtc_flag = 0;
    //     curr_count--;
    // }
    int i,j;
    for (i = 0 ; i < MAX_PID ; i ++) {
        pcb_t* process = get_pcb(i);
        if (process == NULL) continue;
        if ((process->active == 0)) continue;
        for (j = 2; j < FD_ARRAY_SIZE ; j++) {
            if (process->fd_array[j].inode == -1) {
                process->fd_array[j].file_pos++;
                break;
            }
        }
    }

    //to be sure you get another interrupt
    outb(REG_C, RTC_PORT);	// select register C
    inb(RTC_DATA);		// just throw away contents

    // test RTC
    //test_interrupts(); 

    //send EOI
    send_eoi(RTC_IRQ);
}

/* rtc_open
 * Initialize RTC freq to 2Hz
 * Inputs: filename - file name
 * Outputs: 0 - sucess
 */
int32_t rtc_open(const uint8_t* filename){
    //rtc_init();
    rtc_set_freq(MIN_FREQ);
    return 0;
}

/* rtc_close
 * Do noting
 * Inputs: fd - file descriptor number
 * Outputs: 0 - sucess
 */
int32_t rtc_close(int32_t fd){
    rtc_set_freq(MIN_FREQ);
    return 0;
}

/* rtc_read
 * Block until the next interrupt
 * Inputs: fd - file descriptor number
 *         buf - pointer to value of frequency store in the buffer
 *         nbytes - number of bytes of the freq value(shoudle be 4 because it is int)
 * Outputs: 0 - sucess
 */
int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes){
    //rtc_flag = 0;
    //while(!rtc_flag){}
    pcb_t* curr_pcb = get_active_pcb();
    while (curr_pcb->fd_array[fd].file_pos < curr_pcb->rtc_val) {};
    curr_pcb->fd_array[fd].file_pos = 0;
    return 0;
}

/* rtc_read
 * Change rtc frequency, the input frequency must be the power of 2 and within the range of 2 - 1024
 * Inputs: fd - file descriptor number
 *         buf - pointer to value of frequency store in the buffer
 *         nbytes - number of bytes of the freq value(shoudle be 4 because it is int)
 * Outputs: 0 - sucess; -1 - fail
 */
int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes){
    //check if input args are valid
    if ((buf == NULL)||(nbytes != 4)){
        return -1;
    }

    int freq;
    
    freq = *((int*)buf); //fetch frequency value in the buffer
    
    //if return val of set_freq == -1 means set freq fail
    if (rtc_set_freq(freq) == -1){
        return -1;
    }
    return 0;
}

/* log2(num)
 * Inputs: num - frequency of the RTC
 * Outputs: count - log2(num); -1 - this numberi is not the power of 2
 * Side Effect: calculate log2(num)
 */
char log2(int32_t num) {
    char count = 0;
    while(num != 1){
        if (num % 2 != 0) return -1; //this number is not the power of 2         
        num /= 2;
        count++;
    }
    return count;
}
