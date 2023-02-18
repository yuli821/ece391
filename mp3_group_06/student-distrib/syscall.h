#ifndef _SYSCALL_H
#define _SYSCALL_H

#include "x86_desc.h"
#include "types.h"
#include "filesys.h"
#include "lib.h"
#include "paging.h"
#include "terminal.h"
#include "rtc.h"
#include "idt.h"

// #define MAGIC_0         	0x7f
// #define MAGIC_1         	0x45
// #define MAGIC_2         	0x4c
// #define MAGIC_3         	0x46

// #define PCB_SIZE        	sizeof(pcb_t)
// #define KERNEL_STACK_SIZE   0x2000		// 8kB
// #define MAX_PID         	10          // arbitrary
// #define MAX_FILE_SIZE   	0x400000    // max size of file image in bytes: 4MB
// #define FILE_IMG_OFF    	0x48000     // offset within the page starting from 128MB in VM space
// #define FD_ARRAY_SIZE   	8
// #define EIP_START			24			// skip 24 bytes in file img
// #define FOUR_BYTES				4
// #define PAGE_RESERVED		12			// lower 12 bits of pde reserved for paging options

// uint8_t pid_array[MAX_PID];

// typedef struct process_control_block {
//     fd_t fd_array[8];
//     int32_t parent_pid;
//     int32_t pid;
//     int32_t saved_esp;
//     int32_t saved_ebp;
//     int32_t active;
//     int32_t arg_nbytes;
//     uint8_t arg_buff[1024];
// } pcb_t;


extern void system_call(void);

extern int32_t halt(uint8_t status);
extern int32_t execute(const uint8_t* command);
extern int32_t read (int32_t fd, void* buf, int32_t nbytes);
extern int32_t write(int32_t fd, const void* buf, int32_t nbytes);
extern int32_t open(const uint8_t* filename);
extern int32_t close(int32_t fd);
extern int32_t getargs(uint8_t* buf, int32_t nbytes);
extern int32_t vidmap(uint8_t** screen_start);
extern int32_t set_handler(int32_t signum, void* handler_address);
extern int32_t sigreturn(void);

// void parse(const uint8_t* str);
// pcb_t* get_pcb(); 


#endif
