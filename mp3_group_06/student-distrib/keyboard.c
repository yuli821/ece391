#include "keyboard.h"
#include "terminal.h"
#include "i8259.h"
#include "lib.h"
#include "process.h"

//#include <stdint.h>

uint8_t l_shift = 0;
uint8_t r_shift = 0;
uint8_t ctrl_flag = 0; 
uint8_t caps_flag = 0;
uint8_t alt_flag = 0;
//static uint32_t bs_counter = 0;

//static char keyboard_buffer[KB_BUF_LEN];
//static char* kb_buffer_ptr = keyboard_buffer;
//static uint32_t buffer_counter = 0;

//kb scancode lookup table
const char scancode_table[58][2] = {
    [0]={0x00, 0x00},// reserve
    [1]={ESC,ESC},
    [2]={'1', '!'},
    [3]={'2', '@'}, 
    [4]={'3', '#'}, 
    [5]={'4', '$'},
    [6]={'5', '%'},
    [7]={'6', '^'}, 
    [8]={'7', '&'}, 
    [9]={'8', '*'}, 
    [10]={'9', '('},
    [11]={'0', ')'},
    [12]={'-', '_'},
    [13]={'=', '+'},
    [14]={'\b','\b'},   
    [15]={'\t', '\t'},//tab
    [16]={'q','Q'},
    [17]={'w', 'W'},
    [18]={'e', 'E'},
    [19]={'r', 'R'},
    [20]={'t', 'T'},
    [21]={'y', 'Y'},
    [22]={'u', 'U'},
    [23]={'i', 'I'},
    [24]={'o', 'O'},
    [25]={'p', 'P'},
    [26]={'[', '{'},
    [27]={']', '}'},
    [28]={ENTR,ENTR},
    [29]={0x0, 0x0},
    [30]={'a', 'A'},
    [31]={'s', 'S'},
    [32]={'d', 'D'},
    [33]={'f', 'F'},
    [34]={'g', 'G'},
    [35]={'h', 'H'},
    [36]={'j', 'J'},
    [37]={'k', 'K'},
    [38]={'l', 'L'},
    [39]={';', ':'},
    [40]={'\'', '"'},
    [41]={'`', '~'},
    [42]={0x0, 0x0}, //reserve for lshift
    [43]={'\\', '|'},
    [44]={'z', 'Z'},
    [45]={'x', 'X'},
    [46]={'c', 'C'},
    [47]={'v', 'V'},
    [48]={'b', 'B'},
    [49]={'n', 'N'},
    [50]={'m', 'M'},
    [51]={',', '<'},
    [52]={'.', '>'},
    [53]={'/', '?'},
    [54]={0x0, 0x0},
    [55]={0x0, 0x0},
    [56]={0x0, 0x0},
    [57]={' ', ' '},
};

/*keyboard_init
    in: none
    out: none
    return: none
    effect: enables KB_IRQ(1) on pic

*/
void keyboard_init(void) { 
    enable_irq(KB_IRQ); 
}


/*keyboard_handler
    in: none
    out: none
    return: none
    effect: scans keyboard keycode and handles corresponding processor action. called on interrupt
*/
void keyboard_handler(void) { 
    cli(); //critical
        //pcb_t* curr_pcb = get_active_pcb();
        uint32_t curr_vidm=0x0b7000;
        uint8_t scancode = inb(KB_DATA);
        int i;
        //printf("%x",scancode);
        terminal_t* term = &terminals[display_tid];
        
        if (is_modifier(scancode)) {
            send_eoi(KB_IRQ);
            sti();
            return;
        }

        if (alt_flag && (scancode == F1)) {
          //printf("tswitch01");
          terminal_switch_display(0);
        } else if (alt_flag && (scancode == F2)) {
          //printf("tswitch02");
          terminal_switch_display(1);
        } else if (alt_flag && (scancode == F3)) {
          //printf("tswitch03");
          terminal_switch_display(2);
        } else if (scancode < SCAN_END) {
            //keys
            //check for valid keys
            switch(scancode_table[scancode][0]) {
                case ENTR://newline
                    //newline
                    //calls the terminal read
                    // terminal_read(0,keyboard_buffer,buffer_counter);
                    copy_kb_buff(term->keyboard_buffer, display_tid, term->buffer_counter);
                    //put_to_buffer(scancode_table[scancode][0], buffer_counter);
                    //clear_buffer((char*)(term->keyboard_buffer), term->buffer_counter); //clear the buffer
                    for (i = 0; i < term->buffer_counter; i++) {
                        term->keyboard_buffer[i] = BLANK;
                    }
                    /// temporary
                    putc2('\n',(char*)curr_vidm);
                    /// temporary  
                    term->buffer_counter = 0; //no more backspaces since on the beginning of new line
                    //^^ this is redundant just in case
                    break;
                case BS://backspace
                    //delete a char; 
                    if (term->buffer_counter > 0) {
                        if (term->keyboard_buffer[term->buffer_counter-1] =='\t') {//tab has to be handled separately for deleting 4 spoaces at a time
                            //int i;
                            for (i = 0; i < 4; i++) {
                                putc2('\b', (char*)curr_vidm);
                            }
                        } else {
                            putc2('\b', (char*)curr_vidm);//rm from screen -- lib func
                        }
                        term->buffer_counter--;//rm from buffer
                        term->keyboard_buffer[term->buffer_counter] = BLANK;
                    }
                    break;
                default: //char key
                    if (scancode == L &&
                        (ctrl_flag == 1)) {//ctrl+l
                        //clear_buffer(); //clear buffer
                        clear2(); //clear screen
                    } else if (scancode == TAB){
                        term->keyboard_buffer[term->buffer_counter] = '\t';
                        term->buffer_counter++;
                        //int i;
                        for (i =0; i < 4; i++) {
                            putc2(' ', (char*)curr_vidm);
                        }
                    }else {
                        if (term->buffer_counter < KB_BUF_LEN - 1) {
                            char new_char; //this lil thing goes to the screen
                            //display
                            if (!is_letter(scancode)) { // none alphabet, shifting check is enough
                                if (l_shift || r_shift) {
                                    new_char = scancode_table[scancode][1]; // e.g. shift+"1" is "1"
                                } else {
                                    new_char = scancode_table[scancode][0]; //"1" is "1"
                                }
                            } else { //letter; check for capslock and also shift
                                if (l_shift || r_shift || caps_flag) {
                                    new_char = scancode_table[scancode][1]; 
                                    //case 1. shift(hold) + "a" is "A"
                                    //case 2. capslock + "a" is "A"
                                } else {
                                    new_char = scancode_table[scancode][0];
                                    //"a" is "a"
                                }
                            }
                            //cp1
                            //putc(scancode_table[scancode][0]);

                            //cp2 & beyond 
                            term->keyboard_buffer[term->buffer_counter] = new_char;
                            term->buffer_counter++;
                            putc2(new_char, (char*)curr_vidm);
                           
                        }
                    }
            }
            send_eoi(KB_IRQ);
            sti();
            return;
        }
    send_eoi(KB_IRQ);
    sti();
}

/*is_letter
    tells if a scancode corresponds to an English letter
    in: uint8_t scancode
    out: none
    return: 1 for letter, 0 for not letter
    side: none 
*/
uint8_t is_letter(uint8_t scan) {
    uint32_t scancode = scan;
    return ((Q<=scancode && scancode<=P) ||
            (A<=scancode && scancode<=L) ||
            (Z<=scancode && scancode<=M));
}

/*is_modifier
    tells if a scancode corresponds to a modifier (shift, ctrl, capslc)
    in: scancode (read form kb)
    out: none
    return: True(1) for is modifier, False(0) for not
    effect: sets flag according to modifier read
*/
uint8_t is_modifier(uint8_t scan) {
    uint32_t scancode = scan;
    switch(scancode) {
        case LSHIFT:
            l_shift = 1;
            return 1;
        case (LSHIFT+RELEASE):
            //putc(';');
            l_shift = 0;
            return 1;
        case RSHIFT:
            r_shift = 1;
            return 1;
        case (RSHIFT+RELEASE):
            //putc(';');
            r_shift = 0;
            return 1;
        case CTRL:
            ctrl_flag = 1;
            return 1;
        case (CTRL+RELEASE):
            ctrl_flag = 0;
            return 1;
        case KP_M:
            return 1;
        case ALT:
            alt_flag = 1;
            return 1;
        case (ALT+RELEASE):
            alt_flag = 0;
            return 1;
        case CAPSLK:
            //putc(';');
            if (caps_flag) {
                caps_flag = 0;
            } else {
                caps_flag = 1;
            }
            return 1;
        default: return 0;
    }
}

/*put_to_buffer
    puts a char into the kb buffer
    in: char c
    out: none
    return: none
    side: the char c gets into the kb buffer; ++ the buffer count
*/
// void put_to_buffer(char* buf, char c) {
//     buf[buffer_counter] = c;
//     buffer_counter++;
// }

/*remove_from_buffer
    removes a char from kb buffer
    in: none
    out: none
    return: none
    side: removes the last char from buffer; -- the buffer count
*/
// void remove_from_buffer(char* buf) {
//     buffer_counter--;
//     buf[buffer_counter] = BLANK;
// }

/*clear_buffer
    clears the buffer by calling remove_from_buffer many times
    in: none
    out: none
    return: none
    side: kb_buffer gets empty just as my brain/beer/wallet
    (okay the cycle is stupid i think there are something else with the same effect but more elegant)
*/
void clear_buffer(char* buf, int buffer_counter) {
    int i;
    int buff_len = buffer_counter;
    for (i = 0; i < buff_len; i++) {
        buf[i] = BLANK;
    }
}
