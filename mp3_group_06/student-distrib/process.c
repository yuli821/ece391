#include "process.h"


terminal_t terminals[3];
int32_t display_tid;                     //the terminal that we are currently watching
int32_t active_tid;                        //the terminal that is currently running by scheduler
int32_t curr_pid;                         //the process id that is running on active terminal

static uint8_t cmd_array[2][1024];                 //store the command and argument

extern void flushTLB();
//file operation jump table for each type
fop_t rtc_fop = {rtc_open,rtc_read,rtc_close,rtc_write};
fop_t file_fop = {f_open, f_read, f_close, f_write};
fop_t dir_fop = {dir_open, dir_read, dir_close, dir_write};
fop_t std_in = {terminal_open, terminal_read, terminal_close, NULL};
fop_t std_out = {terminal_open, NULL, terminal_close, terminal_write};
fop_t null_fop = {NULL, NULL, NULL, NULL};

/* get_active_pcb
 * Get active process id
 * Inputs: None
 * Outputs: pointer to active PCB
 */
pcb_t* get_active_pcb() {
    return get_pcb(curr_pid);            //return the pcb pointer to the currently-running process
}

/* get_pcb
 * Get process id
 * Inputs: pid - process id
 * Outputs: pointer to PCB
 */
pcb_t* get_pcb(int32_t pid) {
    if (pid < 0 || pid >= MAX_PID) return NULL;
    return (pcb_t*) ((uint32_t)USER_MEM_START - (uint32_t)(pid+1) * (uint32_t)KERNEL_STACK_SIZE);  //pointer to a specific pcb which is represented by the input pid
}

/* parse
 * Parse the command
 * Inputs: str - pointer of the commadn string start
 * Outputs: None
 * Side Effects: Store command word by word in cmd_array
 */
void parse(const uint8_t* str) {
    int i,j;
    int str_count=0;
    for (i = 0; i < 2; i++) {                          //initialization, we have a 2 * 1024 2d array
        for (j = 0; j < 1024; j++) {
            cmd_array[i][j] = '\0';                    //initialize to null
        }
    }
    for (i = 0; i < 2; i++) {                          //loop to store the 2 words, cmd + argument
        while (str[str_count]== ' ') {                 //get rid of leading spaces
            str_count ++;
        }
        for (j = 0; j < 1024; j++) {
            if (str[str_count] == ' ') {
                cmd_array[i][j] = '\0';                //command/argument are null-terminated
                //str_count ++;
                break;
            }
            if (str[str_count] == '\0') {              //if end, return
                cmd_array[i][j] = '\0';
                return;
            }
            cmd_array[i][j] = str[str_count];          //store the input into the array
            str_count++;
        }
    }
}

/* process_init
 * Initialize process
 * Inputs: None
 * Outputs: None
 * Side Effects: Set up a PCB pointer for each process, initialize 3 terminals
 */
void process_init() {
    int i;
    for (i = 0 ; i < MAX_PID ; i++) {
        pcb_t* curr_pcb = get_pcb(i);                 //we can have a max of 6 programs running. For each pcb, initialize to inactive
        curr_pcb->active = 0;
    }
    for (i = 0 ; i < 3 ; i++) {
        terminals[i].pid = -1;             // no process is active at each terminal
        terminals[i].screen_x = 0;                    // store cursor position for each terminal
        terminals[i].screen_y = 0;
        memset(terminals[i].keyboard_buffer,0,KB_BUF_LEN+1);  //separate keyboard buffer for each terminal
        terminals[i].buffer_counter = 0;              // record the top of the keyboard buffer
        terminals[i].enter_flag = 0;                  // enter flag for terminal read
    }
    display_tid = 0;                        //we look at the first terminal at the beginning
    active_tid = -1;                          //no terminal is running at this point, scheduler is not runinng
    curr_pid = -1;                           //same as above
}

/* process_create
 * Create process
 * Inputs: command - content of command input by user
 * Outputs: -1 - fail to create the process, 0 - create process successfully
 * Side Effects: parse command into buffer, set up current PCB, do context switch
 */
int32_t process_create(const uint8_t* command) {
    if (NULL == command) return -1;
    int i;
    int pid;
    pcb_t* curr_pcb;
    int pid_flag = 0;

    // read file image
    uint8_t eip_buf[FOUR_BYTES];
    uint32_t prog_eip;
    int bytes_read;
    dentry_t file_dentry;
    uint8_t ELF_check_buf[FOUR_BYTES];
    // saved esp and ebp
    // find the pid for the new peocess
    for (i = 0; i < MAX_PID; i++) {
      if (pid_array[i] == 0) {
        pid = i;
        pid_array[i] = 1;
        pid_flag = 1;
        break;
      }
    }
    if (pid_flag == 0) {
      return -1;
    }
    curr_pcb = get_pcb(pid);
    curr_pcb->pid = pid;
    // parse args and cpy to buffer
    parse((uint8_t*) command);
    curr_pcb->arg_nbytes = 0;
    for (i = 0; i < 1024; i++) {                               //the size of command buffer is 1024
        curr_pcb->arg_buff[i] = cmd_array[1][i];
        if (cmd_array[1][i] != '\0') {
            curr_pcb->arg_nbytes++;
        }
    }

    // executable check 1: if file can be execute(1st word: filename; rest: args)
    if (read_dentry_by_name(cmd_array[0], &file_dentry) == -1){
        pid_array[pid] = 0;
        return -1; //return -1 if the command cannot be executed
    }
    // executable check 2: check if first few bytes is ELF
    if (read_data(file_dentry.inode_num, 0, (uint8_t*)&ELF_check_buf, FOUR_BYTES) == -1){
        pid_array[pid] = 0;
        return -1;
    }
    if((ELF_check_buf[0] != MAGIC_0) || (ELF_check_buf[1] != MAGIC_1)
     || (ELF_check_buf[2] != MAGIC_2) || (ELF_check_buf[3] != MAGIC_3)){
        pid_array[pid] = 0;
        return -1; //if magic numbers don't match, the file cannot be excuted
    }


    for (i = 0 ; i < FD_ARRAY_SIZE ; i++) {
        curr_pcb->fd_array[i].f_op = null_fop;
        curr_pcb->fd_array[i].inode = 0;
        curr_pcb->fd_array[i].file_pos = 0;
        curr_pcb->fd_array[i].flags = FD_FREE;
    }
    curr_pcb->fd_array[0].f_op = std_in;
    curr_pcb->fd_array[0].inode = 0;
    curr_pcb->fd_array[0].file_pos = 0;
    curr_pcb->fd_array[0].flags = FD_IN_USE;
    curr_pcb->fd_array[1].f_op = std_out;
    curr_pcb->fd_array[1].inode = 0;
    curr_pcb->fd_array[1].file_pos = 0;
    curr_pcb->fd_array[1].flags = FD_IN_USE;
    curr_pcb->parent_pid = curr_pid;
    curr_pcb->terminal = active_tid;
    curr_pcb->vidmap = 0;
    curr_pcb->active = 1;
    curr_pcb->rtc_val = 0;
    terminals[active_tid].pid = curr_pcb->pid;

    change_paging(curr_pcb->pid);
    
    // User-level loader
    //  1. get EIP (bytes 24-27) out of file
    bytes_read = read_data(file_dentry.inode_num, EIP_START, (uint8_t*)&eip_buf, FOUR_BYTES);
    if (bytes_read == -1)
    {
        pid_array[curr_pcb->pid] = 0;
        return -1;
    }
    prog_eip = *((int32_t*)eip_buf);
    curr_pcb->eip = prog_eip;
    
    //  2. copy file to correct place in physical memory (8MB/12MB + offset)
    bytes_read = read_data(file_dentry.inode_num, 0, (uint8_t*)(USER_PROGRAM+FILE_IMG_OFF), MAX_FILE_SIZE);
    if (bytes_read == -1)
    {
        pid_array[curr_pcb->pid] = 0;
        return -1;
    }
    
    pcb_t* parent_pcb = get_pcb(curr_pid);
    if (NULL != parent_pcb) {
        register uint32_t saved_ebp asm("ebp");
        register uint32_t saved_esp asm("esp");
        parent_pcb->saved_ebp = saved_ebp;
        parent_pcb->saved_esp = saved_esp;
    }
    // Create PCB
    curr_pcb->saved_ebp = KERNEL_STACK;
    curr_pcb->saved_esp = KERNEL_STACK;
    curr_pid = curr_pcb->pid;
    //process_switch_context(curr_pcb->pid);

    // Context switch: getting to ring 3: https://wiki.osdev.org/Getting_to_Ring_3#The_TSS
    //  0. add two new GDT entries (at least) configured for ring 3.
    //  1. re-assign value to SS0 and ESP0
    tss.ss0 = KERNEL_DS;
    tss.esp0 = USER_MEM_START - curr_pcb->pid * KERNEL_STACK_SIZE - FOUR_BYTES;
    sti();

    // //  2. prepare for IRET: context switching, push to kernel stack
    asm volatile(
        "pushl $0x002B           ;"
        "pushl $0x83FFFFC        ;"
        "pushfl                  ;"
        "popl  %%eax             ;"
        "orl   $0x200, %%eax     ;"
        "pushl %%eax             ;"
        "pushl $0x0023           ;"
        "pushl %%ebx             ;"
        "iret                    "
        : /* no outputs */
        : "b" (prog_eip)
        : "eax"
    );
    return 0;
}

/* process_halt
 * Halt process, go back to parent process
 * Inputs: status - used
 * Outputs: -1 - fail to halt, 0 - halt successfully
 * Side Effects: Close any relevant FDs, restore parent data and paging
 */
int32_t process_halt(uint8_t status) {
    int i;
    pcb_t* curr_pcb = get_pcb(curr_pid);
    if (NULL == curr_pcb) return -1;
    int fl = get_exception_fl();

    // close any relevant FDs
    for(i = 0 ; i < FD_ARRAY_SIZE ; i++) {
        close(i);
        //curr_pcb->fd_array[i].flags = FD_FREE;
    }
    pid_array[curr_pcb->pid] = 0;
    //reload shell if the base shell is halted
    if (curr_pcb->parent_pid == -1) {
        curr_pcb->active = 0;
        curr_pid = -1;
        terminals[active_tid].pid = -1;
        process_create((uint8_t*)"shell");
    }
    // restore parent data
    int32_t parent = curr_pcb->parent_pid;
    curr_pcb->active = 0;
    tss.esp0 = USER_MEM_START - parent * KERNEL_STACK_SIZE - FOUR_BYTES;  //?
    // restore parent paging
    if (curr_pcb->vidmap == 1) {                                 // if the halting program uses vidmap
        pde[VIDMAP_PDE].present = 0;
        pte2[VIDMAP_PTE].present = 0;
    }
    change_paging(parent);
    curr_pid = parent;
    terminals[active_tid].pid = parent;
    curr_pcb = get_pcb(parent);
    // jump to execute return, stored_ebp->ebp, stored_esp->esp, status is in bl
    // if fl == 1, exception happend, return with 256
    // stored_ebp in edx, stored_esp in ecx, fl in edi
    asm volatile(
        "movl %%edx, %%esp           ;"
        "movl %%ecx, %%ebp           ;"
        "movl $0, %%eax              ;"
        "movb %%bl, %%al             ;"
        "cmpl $0, %%edi              ;"
        "je   return_                    ;"
        "movl $256, %%eax            ;"
"return_:    leave                       ;"
        "ret                         "
        :
        :"d"(curr_pcb->saved_esp), "c"(curr_pcb->saved_ebp), "D"(fl)
        :"cc"
    );
    return -1;                                        //never execute
}

/* change_paging
 * Change virtual memory addr for process
 * Inputs: pid - process id
 * Outputs: None
 * Side Effects: TLB is flushed
 */
void change_paging(int32_t pid) {                              //the process we want to change to
    if (pid < 0 || pid >= MAX_PID) return;
    pcb_t* curr_pcb = get_pcb(pid);
    if (curr_pcb == NULL) return;

    pde[USER_INDEX].present = 1;                               //switch the program paging, map the virtual user program page to the new physical memory
    pde[USER_INDEX].US = 1;                                    // we need to switch the program we are running, so we'll call this in process_create
    pde[USER_INDEX].RW = 1;                                    // we also need to change the mapping of virtual address at 0xb8000 when we implement scheduler
    pde[USER_INDEX].PWT = 0;                                   // so we'll call this in terminal_switch_active and terminal_switch_display
    pde[USER_INDEX].PCD = 0;
    pde[USER_INDEX].access = 0;
    pde[USER_INDEX].reserved2 = 0;
    pde[USER_INDEX].PS = 1;
    pde[USER_INDEX].G = 0;
    pde[USER_INDEX].avail = 0;
    pde[USER_INDEX].addr = (USER_MEM_START + pid * PAGE_KERNEL)>>PAGE_RESERVED;

    //terminal_switch_display and terminal_switch_active need to change the mapping of virtual 0xb8000
    if (active_tid == display_tid) {
        pte[VIDMAP_PTE].page_addr = VIDMAP_PTE;                            //if the terminal running and the terminal we are watching are the same terminal
        pte2[VIDMAP_PTE].page_addr = VIDMAP_PTE;                           //everything should be write to the screen, therefore virtual 0xb8000 maps to 0xb8000 in physical
    } else {
        pte[VIDMAP_PTE].page_addr = (BUFF_T1>>12) + curr_pcb->terminal;    //if they are different, virtual 0xb8000 should be mapped to the terminal buffer at b9000/ba000/bb000
        pte2[VIDMAP_PTE].page_addr = (BUFF_T1>>12) + curr_pcb->terminal;
    }
    pde[VIDMAP_PDE].present = curr_pcb->vidmap;
    pte2[VIDMAP_PTE].present = curr_pcb->vidmap;                           //restore or disable the vidmap when we switch process

    flushTLB();                                                            //flush_tlb everytime we change paging
}

/* terminal_switch_active
 * Switch to active terminal, and this terminal will run (do this by scheduling)
 * Inputs: terminal - terminal id (0, 1, 2)
 * Outputs: None
 */
void terminal_switch_active(uint32_t terminal) {                           //the terminal we want to switch to
    pcb_t* curr_pcb = get_pcb(curr_pid);                          //get the pcb of the process that is currently running
    if (curr_pcb != NULL){                                                 //check if it's the first interrupt of pit
        register uint32_t saved_ebp asm("ebp");                            //In the first interrpt, we don't have any process running, no esp and ebp
        register uint32_t saved_esp asm("esp");
        curr_pcb->saved_esp = saved_esp;                                   //store the esp and ebp
        curr_pcb->saved_ebp = saved_ebp;
    }
    //int old_term = active_tid;
    terminals[active_tid].pid = curr_pid;      //store the active process
    active_tid = terminal;                                         //update the active terminal
    curr_pid = terminals[terminal].pid;                //update the active process
    if (curr_pid == -1) {                                         //terminal's active process are initialized to -1, if -1, run shell on the three terminals
        process_create((uint8_t*)"shell");
    } else {
        tss.esp0 = USER_MEM_START - curr_pid * KERNEL_STACK_SIZE - FOUR_BYTES;  //context switch if we have all three shell running
        change_paging(curr_pid);
        // printf("%d\n", curr_pid);
        // printf("%d\n", active_tid);
        // printf("%d\n", display_tid);
        // printf("%x\n", pte[184].page_addr);
        // printf_debug("%x\n", pte[184].page_addr);
        pcb_t* curr_proc = get_pcb(curr_pid);
        asm volatile(
            "movl %0, %%esp   ;"
            "movl %1, %%ebp   ;"
            :
            :"r"(curr_proc->saved_esp), "r"(curr_proc->saved_ebp)
            :"cc"
        );
    }
}

/* terminal_switch_display
 * The terminal will display in the window (do this by user)
 * Inputs: terminal - terminal id (0, 1, 2)
 * Outputs: None
 */
void terminal_switch_display(uint32_t terminal) {
    // update_cursor_term();
    // store_cursor_term(display_tid);
    uint8_t* dest_addr = (uint8_t*) (BUFF_T1 + (display_tid<<12));     //copy the memory in physical b8000 to the corresponding buffer, 12 is the offset
    memcpy(dest_addr, (uint8_t*)CONST_VID_MEM, FOUR_KB);                         //const_vid_mem is 0xb7000, which always map to physical b8000
    display_tid = terminal;                                            //update the displayed terminal id
    change_paging(curr_pid);                                            //switch to a new process, change the mapping of virtual 0xb8000, 
    uint8_t* src_addr = (uint8_t*) (BUFF_T1 + (display_tid<<12));      //store the memory in terminal buffer back to physical video memomry
    memcpy((uint8_t*)CONST_VID_MEM, src_addr, FOUR_KB);
    // set_cursor(terminals[display_tid].screen_x, terminals[display_tid].screen_y);
    update_cursor(display_tid);                                        //restore the cursor
}
