#define ASM          1
#include "x86_desc.h"

.globl system_call
.align 4
system_call:
    pushfl
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx

    cmpl $10, %eax
    ja err
    cmpl $1, %eax
    jb err
    call *sys_call_table(,%eax,4)
    jmp return

err:
    movl $-1, %eax
return:
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    popfl
    iret           

sys_call_table:
    .long 0x0, halt, execute, read, write, open, close, getargs, vidmap, set_handler, sigreturn

