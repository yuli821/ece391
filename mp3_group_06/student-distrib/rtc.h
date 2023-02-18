#ifndef _RTC_H
#define _RTC_H

#include "types.h"
#include "lib.h"
#include "i8259.h"
#include "process.h"


#define RTC_PORT		0x70		// Port 0x70: CMOS register, the higher order bit(0x80) of Port 0x70 controls NMI, low order 7 bits of any byte sent to Port 0x70 are used to address CMOS registers.
#define RTC_DATA		0x71		// Port 0x71: read or write from/to that byte of CMOS configuration space
#define RTC_IRQ		    0x08

#define NMI_MASK        0x80
#define REG_A           0x0A
#define REG_B           0x0B
#define REG_C           0x0C

#define MAX_FREQ       1024
#define MIN_FREQ       2

/* Initialize RTC */
extern void rtc_init();

/* Set RTC frequency*/
int32_t rtc_set_freq(int32_t freq);

/* Handle RTC interrupts */
extern void rtc_handle();

/* Open RTC */
extern int32_t rtc_open(const uint8_t* filename);

/* Close RTC */
extern int32_t rtc_close(int32_t fd);

/* Read RTC */
extern int32_t rtc_read(int32_t fd, void* buf, int32_t nbytes);

/* Write RTC */
extern int32_t rtc_write(int32_t fd, const void* buf, int32_t nbytes);

/* Helper func to calculate log2(num)*/
char log2(int32_t num);

#endif /* _RTC_H */
