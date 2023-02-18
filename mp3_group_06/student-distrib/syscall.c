#include "syscall.h"
#include "process.h"

// static uint8_t cmd_array[2][1024];
// static uint8_t curr_pid = 0;
extern void flushTLB();

/* halt
 * Description: restore parent data and paging, close any relevant FDs,
 *  set up return value and jump to execute return
 * Inputs: status - (regsiter %bl) return value of the program being halted
 * Outputs: -1 but never return to caller, instead return to execute
 * Side Effects: paging, pid_array, cur_pid, tss, memory, registers(esp, 
 *  ebp, eax) changed
 */
int32_t halt(uint8_t status) {
    cli();
    process_halt(status);
    return -1;
//     int i;
//     pcb_t* curr_pcb = get_pcb();
//     int fl = get_exception_fl();
//     //reload shell if the base shell is halted
//     if (curr_pcb->pid == 0) {
//         for (i = 0 ; i < MAX_PID ; i++) {
//             pid_array[i] = 0;
//         }
//         curr_pid = 0;
//         execute((uint8_t*)"shell");
//     }
//     // restore parent data
//     pid_array[curr_pcb->pid] = 0;
//     curr_pid = curr_pcb->parent_pid;
//     curr_pcb->active = 0;
//     tss.esp0 = USER_MEM_START - curr_pid * KERNEL_STACK_SIZE - FOUR_BYTES;  //?
//     tss.ss0 = KERNEL_DS;
//     // restore parent paging
//     pde[USER_INDEX].addr = (USER_MEM_START + curr_pid * PAGE_KERNEL)>>PAGE_RESERVED;
//     flushTLB();
//     // close any relevant FDs
//     for(i = 0 ; i < FD_ARRAY_SIZE ; i++) {
//         curr_pcb->fd_array[i].flags = FD_FREE;
//     }
//     // jump to execute return, stored_ebp->ebp, stored_esp->esp, status is in bl
//     // if fl == 1, exception happend, return with 256
//     // stored_ebp in edx, stored_esp in ecx, fl in edi
//     asm volatile(
//         "movl %%edx, %%esp           ;"
//         "movl %%ecx, %%ebp           ;"
//         "movl $0, %%eax              ;"
//         "movb %%bl, %%al             ;"
//         "cmpl $0, %%edi              ;"
//         "je   return_                    ;"
//         "movl $256, %%eax            ;"
// "return_:    leave                       ;"
//         "ret                         "
//         :
//         :"d"(curr_pcb->saved_esp), "c"(curr_pcb->saved_ebp), "D"(fl)
//         :"cc"
//     );
//     return -1;                                        //never execute
}

/* execute
 * Description: parse the command and check the validity and executability
 *   of the file, set up paging, prepare for context switching and go to user mode
 * Inputs: command - file name of the program to be executed, 
 *          and the arguments both stripped of leading spaces
 * Outputs: -1 if fails to execute the command
 *          256 if the program dies by an exception
 *          // 0-255 returned by the called program if halt is called
 * Side Effects: paging, pid_array, cur_pid, tss, memory, kernel stack changed
 */
int32_t execute(const uint8_t* command) {
    cli();
    int ret = process_create(command);
    return ret;
    // cli();
    // int i;
    // pcb_t this_pcb;
    // pcb_t* pcb_addr;
    // int pid_flag = 0;

    // // read file image
    // uint8_t eip_buf[FOUR_BYTES];
    // uint32_t prog_eip;
    // int bytes_read;
    // dentry_t file_dentry;
    // uint8_t ELF_check_buf[FOUR_BYTES];
    // // saved esp and ebp
    // register uint32_t saved_ebp asm("ebp");
    // register uint32_t saved_esp asm("esp");
    
    // // parse args and cpy to buffer
    // parse(command);
    // this_pcb.arg_nbytes = 0;
    // for (i = 0; i < 1024; i++) {
    //     this_pcb.arg_buff[i] = cmd_array[1][i];
    //     if (cmd_array[1][i] != '\0') {
    //         this_pcb.arg_nbytes++;
    //     }
    // }

    // // executable check 1: if file can be execute(1st word: filename; rest: args)
    // if (read_dentry_by_name(cmd_array[0], &file_dentry) == -1){
    //     return -1; //return -1 if the command cannot be executed
    // }
    // // executable check 2: check if first few bytes is ELF
    // if (read_data(file_dentry.inode_num, 0, (uint8_t*)&ELF_check_buf, FOUR_BYTES) == -1){
    //     return -1;
    // }
    // if((ELF_check_buf[0] != MAGIC_0) || (ELF_check_buf[1] != MAGIC_1)
    //  || (ELF_check_buf[2] != MAGIC_2) || (ELF_check_buf[3] != MAGIC_3)){
    //     return -1; //if magic numbers don't match, the file cannot be excuted
    // }

    // for (i = 0; i < MAX_PID; i++) {
    //   if (pid_array[i] == 0) {
    //     this_pcb.pid = i;
    //     pid_array[i] = 1;
    //     pid_flag = 1;
    //     break;
    //   }
    // }
    // if (pid_flag == 0) {
    //   return -1;
    // }


    // for (i = 0 ; i < FD_ARRAY_SIZE ; i++) {
    //     this_pcb.fd_array[i].f_op = null_fop;
    //     this_pcb.fd_array[i].inode = 0;
    //     this_pcb.fd_array[i].file_pos = 0;
    //     this_pcb.fd_array[i].flags = FD_FREE;
    // }
    // this_pcb.fd_array[0].f_op = std_in;
    // this_pcb.fd_array[0].inode = 0;
    // this_pcb.fd_array[0].file_pos = 0;
    // this_pcb.fd_array[0].flags = FD_IN_USE;
    // this_pcb.fd_array[1].f_op = std_out;
    // this_pcb.fd_array[1].inode = 0;
    // this_pcb.fd_array[1].file_pos = 0;
    // this_pcb.fd_array[1].flags = FD_IN_USE;
    // this_pcb.parent_pid = curr_pid;
    // this_pcb.active = 1;
    
    // // Set up program paging
    // pde[USER_INDEX].P = 1;
    // pde[USER_INDEX].US = 1;
    // pde[USER_INDEX].PS = 1;
    // pde[USER_INDEX].RW = 1;
    // pde[USER_INDEX].addr = (USER_MEM_START + this_pcb.pid * PAGE_KERNEL)>>PAGE_RESERVED;//8MB + pid * 4MB
    // flushTLB();
    
    // // User-level loader
    // //  1. get EIP (bytes 24-27) out of file
    // bytes_read = read_data(file_dentry.inode_num, EIP_START, (uint8_t*)&eip_buf, FOUR_BYTES);
    // if (bytes_read == -1)
    // {
    //     pid_array[this_pcb.pid] = 0;
    //     return -1;
    // }
    // prog_eip = *((int32_t*)eip_buf);
    
    // //  2. copy file to correct place in physical memory (8MB/12MB + offset)
    // bytes_read = read_data(file_dentry.inode_num, 0, (uint8_t*)(USER_PROGRAM+FILE_IMG_OFF), MAX_FILE_SIZE);
    // if (bytes_read == -1)
    // {
    //     pid_array[this_pcb.pid] = 0;
    //     return -1;
    // }
    
    // // Create PCB
    // this_pcb.saved_ebp = saved_ebp;
    // this_pcb.saved_esp = saved_esp;
    // curr_pid = this_pcb.pid;

    // pcb_addr = get_pcb();
    // memcpy(pcb_addr, &this_pcb, PCB_SIZE);

    // // Context switch: getting to ring 3: https://wiki.osdev.org/Getting_to_Ring_3#The_TSS
    // //  0. add two new GDT entries (at least) configured for ring 3.
    // //  1. re-assign value to SS0 and ESP0
    // tss.ss0 = KERNEL_DS;
    // tss.esp0 = USER_MEM_START - this_pcb.pid * KERNEL_STACK_SIZE - FOUR_BYTES;
    // sti();

    // //  2. prepare for IRET: context switching, push to kernel stack
    // asm volatile(
    //     "pushl $0x002B           ;"
    //     "pushl $0x83FFFFC        ;"
    //     "pushfl                  ;"
    //     "popl  %%eax             ;"
    //     "orl   $0x200, %%eax     ;"
    //     "pushl %%eax             ;"
    //     "pushl $0x0023           ;"
    //     "pushl %%ebx             ;"
    //     "iret                    "
    //     : /* no outputs */
    //     : "b" (prog_eip)
    //     : "eax"
    // );
    // return 0;
}

/* read
 * Description: reads data from the keyboard, a file, device (RTC), or directory
    into the argument buf
 * Inputs: fd - function descriptor
 *         buf - buffer to store data being read
 *         nbytes - number of bytes to read
 * Outputs: number of bytes successfully read or
 *          -1 if fd unavailable or buf invalid
 * Side Effects: buf changed
 */
int32_t read(int32_t fd, void* buf, int32_t nbytes) {
    sti();
    pcb_t* curr_pcb;
    int bytes_read;
    curr_pcb = get_active_pcb();
    if (fd < 0 || fd > (FD_ARRAY_SIZE - 1)) {
        return -1;
    }
    if (fd == 1) {
        return -1;
    }
    if (buf == NULL) {
        return -1;
    }
    if (curr_pcb->fd_array[fd].flags == FD_FREE) {
        return -1;
    }
    bytes_read = curr_pcb->fd_array[fd].f_op.read_file(fd, buf, nbytes);
    return bytes_read;
}

/* write
 * Description: write data to the terminal or a device (RTC)
 * Inputs: fd - function descriptor
 *         buf - data to be written
 *         nbytes - number of bytes to be written
 * Outputs: number of bytes successfully written or
 *          -1 if fd unavailable or buf invalid
 * Side Effects: video memory or rtc changed
 */
int32_t write(int32_t fd, const void* buf, int32_t nbytes) {
    pcb_t* curr_pcb;
    int bytes_written;
    curr_pcb = get_active_pcb();
    if (fd < 1 || fd > (FD_ARRAY_SIZE-1)) {
        return -1;
    }
    if (buf == NULL) {
        return -1;
    }
    if (curr_pcb->fd_array[fd].flags == FD_FREE) {
        return -1;
    }
    bytes_written = curr_pcb->fd_array[fd].f_op.write_file(fd, buf, nbytes);
    return bytes_written;
}

/* open
 * Description:  find the directory entry corresponding to the named file,
 *  allocate an unused file descriptor, and set up any data necessary to 
 *  handle the given type of file (directory, RTC device, or regular file)
 * Inputs: filename - file to open
 * Outputs: index of allocated file descriptor into fd array or
 *          -1 If file not exist or no descriptors are free
 * Side Effects: memory at curr_pcb regarding fd array changed
 */
int32_t open(const uint8_t* filename) {
    dentry_t d;
    pcb_t* curr_pcb = get_active_pcb();
    int i;
    //uint8_t garbage;
    if (filename == NULL) {
        return -1;
    }
    if (read_dentry_by_name(filename, &d)!= 0){
        return -1; //return -1 if the command cannot be executed
    }
    for (i = 2 ; i < FD_ARRAY_SIZE ; i++) {
        if (curr_pcb->fd_array[i].flags == FD_FREE) {
            curr_pcb->fd_array[i].file_pos = 0;
            curr_pcb->fd_array[i].flags = FD_IN_USE;
            if (d.filetype == 0) {
                curr_pcb->fd_array[i].f_op = rtc_fop;
                curr_pcb->fd_array[i].inode = -1;
                //rtc_open(&garbage);
            }
            else if (d.filetype == 1){
                curr_pcb->fd_array[i].f_op = dir_fop;
                curr_pcb->fd_array[i].inode = 0;
            }
            else {
                curr_pcb->fd_array[i].f_op = file_fop;
                curr_pcb->fd_array[i].inode = d.inode_num;
            }
            return i;
        }
    }
    return -1;
}

/* close
 * Description:  closes the specified file descriptor and 
 *   makes it available for return from later calls
 * Inputs: fd - file descriptor
 * Outputs: 0 for sucess or -1 for failure
 * Side Effects: memory at curr_pcb regarding fd array changed
 */
int32_t close(int32_t fd) {
    if (fd < 2 || fd > (FD_ARRAY_SIZE-1)) {
        return -1;
    }
    pcb_t* curr_pcb = get_active_pcb();
    if (curr_pcb->fd_array[fd].flags == FD_FREE) {
        return -1;
    }
    curr_pcb->fd_array[fd].flags = FD_FREE;
    curr_pcb->fd_array[fd].file_pos = 0;
    curr_pcb->fd_array[fd].inode = 0;
    return 0;
}

/* getargs
 * Description: read theprogram's command line args into 
                a user-level buffer
 * Inputs: buf - buffer to store data being read from PCB
 *         nbytes - number of bytes to read
 * Outputs: 0 if number of bytes successfully read or
 *          -1 if number of args stored in PCE is smaller than 
            number of bytes to read or buf invalid
 * Side Effects: buf changed
 */
int32_t getargs(uint8_t* buf, int32_t nbytes) {
    int i;
    pcb_t* this_pcb;
    this_pcb = get_active_pcb();
    if (this_pcb->arg_nbytes > nbytes - 1 || this_pcb->arg_buff[0] == '\0') {
        return -1;
    }
    for (i = 0; i < 1024; i++) {
        if (this_pcb->arg_buff[i] == '\0'){
            buf[i] = '\0';
            break;
        }
        buf[i] = this_pcb->arg_buff[i];
    }
    return 0;
}

/* vidmap
 * Description: map the text-mode video memory into user space 
                at a preset-set virtual addr
 * Inputs: screen_start - start addr of virtual video memory location
 * Outputs: 0 if successfully map the video memory into user space
 *          -1 if location invalid
 * Side Effects: mapping changed
 */
int32_t vidmap(uint8_t** screen_start) {
    if (screen_start == NULL) {
        return -1;
    }
    if (screen_start == (uint8_t**)PAGE_KERNEL) {
        return -1;
    }
    pcb_t* curr_pcb = get_active_pcb();
    curr_pcb->vidmap = 1;
    pde[VIDMAP_PDE].present = 1;
    pde[VIDMAP_PDE].US = 1;
    pde[VIDMAP_PDE].G = 1;
    pte2[VIDMAP_PTE].present = 1;
    pte2[VIDMAP_PTE].US = 1;
    pte2[VIDMAP_PTE].G = 1;
    pte2[VIDMAP_PTE].page_addr = 0xb8;                            //video memory in physical memory
    flushTLB();
    *screen_start = (uint8_t*)VIDEOMEM_USER;
    return 0;
}

int32_t set_handler(int32_t signum, void* handler_address) {
    return -1;
}

int32_t sigreturn(void) {
    return -1;
}

/* parse
 * Description: split the command into program name and arguments
 *  and store them into cmd_array
 * Inputs: str - command passed into execute
 * Outputs: none
 * Side Effects: cmd_array changed
 */
// void parse(const uint8_t* str) {
//     int i,j;
//     int str_count=0;
//     for (i = 0; i < 2; i++) {
//         for (j = 0; j < 1024; j++) {
//             cmd_array[i][j] = '\0';
//         }
//     }
//     for (i = 0; i < 2; i++) {
//         while (str[str_count]== ' ') {
//             str_count ++;
//         }
//         for (j = 0; j < 1024; j++) {
//             if (str[str_count] == ' ') {
//                 cmd_array[i][j] = '\0';     //null-terminated
//                 //str_count ++;
//                 break;
//             }
//             if (str[str_count] == '\0') {
//                 cmd_array[i][j] = '\0';
//                 return;
//             }
//             cmd_array[i][j] = str[str_count];
//             str_count++;
//         }
//     }
// }

/* get_pcb
 * Description:  get the address of current process control block
 *  in memory given current process id
 * Inputs: none
 * Outputs: pointer to the start of current pcb
 * Side Effects: none
 */
// pcb_t* get_pcb() {
//     uint32_t temp;
//     temp = (uint32_t)USER_MEM_START - (uint32_t)(curr_pid+1) * (uint32_t)KERNEL_STACK_SIZE;
//     return (pcb_t *)temp;
// }
