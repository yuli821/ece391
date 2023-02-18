/* tuxctl-ioctl.c
 *
 * Driver (skeleton) for the mp2 tuxcontrollers for ECE391 at UIUC.
 *
 * Mark Murphy 2006
 * Andrew Ofisher 2007
 * Steve Lumetta 12-13 Sep 2009
 * Puskar Naha 2013
 */

#include <asm/current.h>
#include <asm/uaccess.h>

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/file.h>
#include <linux/miscdevice.h>
#include <linux/kdev_t.h>
#include <linux/tty.h>
#include <linux/spinlock.h>

#include "tuxctl-ld.h"
#include "tuxctl-ioctl.h"
#include "mtcp.h"

#define debug(str, ...) \
	printk(KERN_DEBUG "%s: " str, __FUNCTION__, ## __VA_ARGS__)

/************************ Protocol Implementation *************************/

typedef struct button{
	unsigned char first;    //first byte in MTCP_BIOC_EVT, C,B,A,Start:0-3
	unsigned char second;   //second byte in MTCP_BIOC_EVT, right, down, left, up: 0-3
} button_t;

static unsigned char LED_arg[6];   		//LED buffer for restoring
static button_t button_state;        	//button pressed
static int notBuzy = 1;        //1 means not buzy (receive ACK), 0 means buzy (haven't receive ACK)
static int BIOC_state = 0;

/*The even index represents the led alignment without decimal point, the odd index includes decimal point*/
static char binary_to_led[16 * 2]= {0xE7,0xF7,0x06,0x16,0xCB,0xDB,//0,1,2
									0x8F,0x9F,0x2E,0x3E,0xAD,0xBD,//3,4,5
									0xED,0xFD,0x86,0x96,0xEF,0xFF,//6,7,8
									0xAF,0xBF,0xEE,0xFE,0x6D,0x7D,//9,A,B
									0xE1,0xF1,0x4F,0x5F,0xE9,0xF9,//C,D,E
									0xE8,0xF8};//F

int tuxctl_ioctl_init(struct tty_struct* tty);
int tuxctl_ioctl_buttons(struct tty_struct* tty, unsigned long arg);
int tuxctl_ioctl_setLED(struct tty_struct* tty, unsigned long arg);

/* tuxctl_handle_packet()
 * IMPORTANT : Read the header for tuxctl_ldisc_data_callback() in 
 * tuxctl-ld.c. It calls this function, so all warnings there apply 
 * here as well.
 */
void tuxctl_handle_packet (struct tty_struct* tty, unsigned char* packet)
{
    unsigned a, b, c;
	unsigned char i_recover = MTCP_BIOC_ON;

    a = packet[0]; /* Avoid printk() sign extending the 8-bit */
    b = packet[1]; /* values when printing them. */
    c = packet[2];

    /*printk("packet : %x %x %x\n", a, b, c); */
	switch(a) {
		case MTCP_BIOC_EVENT:			//whenever a button is pushed under bioc mode
			button_state.first = b;		//store the received button state in to static variable
			button_state.second = c;
			return;
		case MTCP_ACK: 					//response to MTCP_BIOC_ON, MTCP_LED_SET
			notBuzy = 1;				//change the state of driver, able to send command
			return;
		case MTCP_RESET: 
			if(BIOC_state) {			//if already enabled BIOC, need to restore it after resets
				tuxctl_ldisc_put(tty, &i_recover, 1);
			}
			tuxctl_ldisc_put(tty,LED_arg,6);  //set LED
			notBuzy = 0;				//sending LED, needs to reset to 0 (set to 1 when ACK comes)
			button_state.first = 0xff;  //initialize button state
			button_state.second = 0xff;
			return;
		default: return;
	}
}

/******** IMPORTANT NOTE: READ THIS BEFORE IMPLEMENTING THE IOCTLS ************
 *                                                                            *
 * The ioctls should not spend any time waiting for responses to the commands *
 * they send to the controller. The data is sent over the serial line at      *
 * 9600 BAUD. At this rate, a byte takes approximately 1 millisecond to       *
 * transmit; this means that there will be about 9 milliseconds between       *
 * the time you request that the low-level serial driver send the             *
 * 6-byte SET_LEDS packet and the time the 3-byte ACK packet finishes         *
 * arriving. This is far too long a time for a system call to take. The       *
 * ioctls should return immediately with success if their parameters are      *
 * valid.                                                                     *
 *                                                                            *
 ******************************************************************************/
int 
tuxctl_ioctl (struct tty_struct* tty, struct file* file, 
	      unsigned cmd, unsigned long arg)
{
    switch (cmd) {
	case TUX_INIT: 
		return tuxctl_ioctl_init(tty);          //initialize tux
	case TUX_BUTTONS:
		return tuxctl_ioctl_buttons(tty,arg);   //not sending command, don't need to check notBuzy
	case TUX_SET_LED:
		if(notBuzy)								//check if we can send command
			tuxctl_ioctl_setLED(tty,arg);
			return 0;
		return 2;
	case TUX_LED_ACK:
		return 0;
	case TUX_LED_REQUEST:
		return 0;
	case TUX_READ_LED:
		return 0;
	default:
	    return -EINVAL;
    }
}

/* tuxctl_ioctl_init
 * DESCRIPTION: Initialize the tux controller
 * INPUTS: tty--line discipline processes all the input/output from a tty device
 * OUTPUTS: none
 * RETURN VALUE: return 0 on success
 * SIDE EFFECTS: Set the tux controller to led user mode; enable button interrupt-on-change mode
 */
int tuxctl_ioctl_init(struct tty_struct* tty) {  //enable led user mode and button interrupt on change
	unsigned char output;
	int i;
	for (i = 0 ; i < 6; i++) {
		LED_arg[i] = 0;
	}
	button_state.first = 0xff;          //initialize button, button is active low
	button_state.second = 0xff;
	output = MTCP_BIOC_ON;
	tuxctl_ldisc_put(tty, &output, 1);  //enable bioc
	output = MTCP_LED_USR;
	tuxctl_ldisc_put(tty, &output, 1);  //enable led user mode
	BIOC_state = 1;                     //initialize variables
	notBuzy = 0;
	return 0;
}

/* tuxctl_ioctl_buttons
 * DESCRIPTION: pass the pressed button to the user
 * INPUTS: tty--line discipline processes all the input/output from a tty device
 * 		   arg--a pointer to a 32-bit integer
 * OUTPUTS: none
 * RETURN VALUE: return 0 on success, return -EINVAL if the pointer is invalid
 * SIDE EFFECTS: Modify the last byte of the integer pointing by the arg with the button value
 */
int tuxctl_ioctl_buttons(struct tty_struct* tty, unsigned long arg) {
	unsigned long* p = (unsigned long*)arg;  
	unsigned char upper,lower,result;
	unsigned char temp1, temp2, swap;
	if (p == NULL) {
		return -EINVAL;                       //if invalid argument, return -EINVAL
	}
	upper = button_state.first;
	lower = button_state.second;
	temp1 = (lower >> 1) & 1;
	temp2 = (lower >> 2) & 1;
	swap = temp1 ^ temp2;   //XOR to swap the position of down and left
	swap = (swap << 1) | (swap << 2);
	lower ^= swap;
	result = ((lower << 4) | (upper & 0x0F)); //7:R 6:L 5:D 4:U 3:c 2:b 1:a 0:s
	copy_to_user(p, &result, 1);              //because the argument if a pointer to userspace, need translation
	return 0;
}

/* tuxctl_ioctl_setLED
 * DESCRIPTION: set the LED display on the tux
 * INPUTS: tty--line discipline processes all the input/output from a tty device
 * 		   arg--A 32-bit integer, highest byte:decimal point, second byte:LED mask, third byte:LED3 and LED2, fourth byte: LED1 and LED0
 * OUTPUTS: none
 * RETURN VALUE: return 0 on success
 * SIDE EFFECTS: Send commands to tux for setting up LED display
 */
int tuxctl_ioctl_setLED(struct tty_struct* tty, unsigned long arg) {
	unsigned char decimal;
	int i;
	unsigned char output[6];                    	  //output bytes array
	unsigned char first_arg = (arg >> (8*2)) & 0xFF;  //get the third byte of argument
	output[0] = MTCP_LED_SET;
	output[1] = 0xff;
	decimal = (arg >> (8*3));
	for (i = 0; i < 4; i++) {
		if ((first_arg & (1 << i)) != 0) {  //if the corresponding bit is 1
			unsigned int dis = (arg >> (i*4)) & 0xF;  //lower four bits after the left shift determines the hex value
			if ((decimal & (1 << i)) != 0) {  //check the decimal point
				output[2+i] = binary_to_led[2 * dis + 1];
			}else {
				output[2+i] = binary_to_led[2 * dis];
			}
		} else {
			output[2+i] = 0x00;               //if not display, send 0x00
		}
	}
	for (i = 0 ; i < 6; i++) {
		LED_arg[i] = output[i];    //update LED buffer
	}
	tuxctl_ldisc_put(tty,output,6);  //set LED
	notBuzy = 0;
	return 0;
}
