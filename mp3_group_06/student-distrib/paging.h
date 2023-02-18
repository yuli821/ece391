
#ifndef _PAGING_H
#define _PAGING_H

#ifndef ASM

#include "types.h"

#define PAGE_KERNEL 0x400000       //4MB
#define PAGE_VIDEO_MEM 0xb8000
#define USER_PROGRAM 0x8000000     //128MB
#define USER_MEM_START 0x800000    //8MB
#define USER_INDEX 32              //128MB in virtual address -> 4MB page index in pde
#define VIDMAP_PDE 33
#define VIDMAP_PTE 184
#define VIDEOMEM_USER 0x84b8000

extern void paging_init();


#endif

#endif
