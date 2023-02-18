#ifndef _PROCESS_H_
#define _PROCESS_H_

#include "paging.h"
#include "x86_desc.h"
#include "filesys.h"
#include "lib.h"
#include "terminal.h"
#include "rtc.h"
#include "keyboard.h"
#include "idt.h"
#include "syscall.h"

#define MAGIC_0         	0x7f
#define MAGIC_1         	0x45
#define MAGIC_2         	0x4c
#define MAGIC_3         	0x46

#define PCB_SIZE        	sizeof(pcb_t)
#define KERNEL_STACK_SIZE   0x2000		// 8kB
#define MAX_PID         	6          // arbitrary
#define MAX_FILE_SIZE   	0x400000    // max size of file image in bytes: 4MB
#define FILE_IMG_OFF    	0x48000     // offset within the page starting from 128MB in VM space
#define FD_ARRAY_SIZE   	8
#define EIP_START			24			// skip 24 bytes in file img
#define FOUR_BYTES			4
#define PAGE_RESERVED		12			// lower 12 bits of pde reserved for paging options

#define CONST_VID_MEM   0xb7000
#define FOUR_KB         4096
#define KERNEL_STACK    0x83FFFFC

#define BUFF_T1 0xb9000
#define BUFF_T2 0xba000
#define BUFF_T3 0xbb000

/* file operation struct */
typedef struct file_op {
    int32_t (*open_file)(const uint8_t* fname);
    int32_t (*read_file)(int32_t fd, void* buf, int32_t nbytes);
    int32_t (*close_file)(int32_t fd);
    int32_t (*write_file)(int32_t fd, const void* buf, int32_t nbytes);
} fop_t;

/*a temprary struct of file descriptor struct*/
typedef struct file_descriptor {
    fop_t f_op;
    uint32_t inode;
    uint32_t file_pos;
    uint32_t flags;
} fd_t;


extern fop_t rtc_fop;
extern fop_t file_fop;
extern fop_t dir_fop;
extern fop_t std_in;
extern fop_t std_out;
extern fop_t null_fop;

/* PCB struct, contains info about pid, terminal, paging, etc. */
typedef struct process_control_block {
    fd_t fd_array[8];
    int32_t parent_pid;
    int32_t pid;
    int32_t saved_esp;
    int32_t saved_ebp;
    int32_t eip;
    int32_t active;
    uint32_t terminal;
    uint32_t vidmap;
    int32_t arg_nbytes;
    int32_t rtc_val;
    uint8_t arg_buff[1024];
} pcb_t;

/* terminal struct, contains info of active process id, cursor position, keyboard buffer, etc. */
typedef struct terminal {
    int32_t pid;
    int screen_x;
    int screen_y;
    uint8_t keyboard_buffer[KB_BUF_LEN+1];
    uint8_t buffer_counter;
    uint8_t enter_flag;
} terminal_t;

uint8_t pid_array[MAX_PID];
extern terminal_t terminals[3];
extern int32_t display_tid;
extern int32_t active_tid;
extern int32_t curr_pid;

void parse(const uint8_t* str);

pcb_t* get_active_pcb();
pcb_t* get_pcb(int32_t pid);

void process_init();
int32_t process_create(const uint8_t* command);
int32_t process_halt(uint8_t status);

void change_paging(int32_t pid);

void terminal_switch_active(uint32_t terminal);
void terminal_switch_display(uint32_t terminal);


#endif
