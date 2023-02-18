### P2 Solution

1. MTCP_BIOC_ON: When we want to enable button intterupt-on-change. this message is sent. MTCP_ACK is returned.   
MTCP_LED_SET: When the LED display is in USR mode, this command will set the user-set LED display values. MTCP_ACK is returned.

2. MTCP_ACK: When the MTC successfully completes a command, the Tux controller sends out the message.     
MTCP_BIOC_EVENT: When the button intterrupt-on-change mode is enabled and a button is either pressed or released, the device sends the message.    
MTCP_RESET: When the device re-initializes itself after a power-up, a RESET button press, or an MTCP_RESET_DEV command, the device sends the message. 

3. The tuxctl_handle_packet is called by tuxctl_ldisc_data, which is called from an interrupt context, so it cannot acquire any sleep since sleeping in an interrupt will break things.
