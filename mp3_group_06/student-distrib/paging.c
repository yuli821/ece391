#include "paging.h"
#include "x86_desc.h"

//extern void loadPageDirectory(int addr);
//extern void enablePaging();


/* paging_init
 * Initialize paging
 * Inputs: None
 * Outputs: None
 * Side Effects: Set the default page size as 4kB, enable paging
 */
void paging_init(){
    int i;
    for(i = 0 ; i < NUM_PTE; i++) {
        if (i == (PAGE_VIDEO_MEM >> 12) || i == (0xb9000 >> 12) || i == (0xba000>>12) || i == (0xbb000>>12) || i == (0xb7000>>12)) {
            pte[i].present = 1;
        }else {
            pte[i].present = 0;
        }
        pte[i].RW = 1;
        pte[i].US = 0;
        pte[i].PWT = 0;
        pte[i].PCD = 0;
        pte[i].access = 0;
        pte[i].D = 0;
        pte[i].PAT = 0;
        pte[i].G = 0;
        pte[i].avail = 0;
        if (i == (0xb7000 >> 12)) {
            pte[i].page_addr = 184;
        } else {
            pte[i].page_addr = i;
        }

        pte2[i].present = 0;
        pte2[i].RW = 1;
        pte2[i].US = 0;
        pte2[i].PWT = 0;
        pte2[i].PCD = 0;
        pte2[i].access = 0;
        pte2[i].D = 0;
        pte2[i].PAT = 0;
        pte2[i].G = 0;
        pte2[i].avail = 0;
        pte2[i].page_addr = i;
    }
    pde[0].present = 1;                           //set up for the first 4kb page, points to a page table
    pde[0].RW = 1;
    pde[0].US = 0;
    pde[0].PWT = 0;
    pde[0].PCD = 0;
    pde[0].access = 0;
    pde[0].reserved2 = 0;
    pde[0].PS = 0;                          //the page size is 4kb
    pde[0].G = 0;
    pde[0].avail = 0;
    pde[0].addr = ((unsigned long)pte >> 12);

    pde[1].present = 1;                           //set up for the kernel page
    pde[1].RW = 0;
    pde[1].US = 0;
    pde[1].PWT = 0;
    pde[1].PCD = 0;
    pde[1].access = 0;
    pde[1].reserved2 = 0;
    pde[1].PS = 1;                          //the page size is 4mb
    pde[1].G = 1;                           //kernel page?
    pde[1].avail = 0;
    pde[1].addr = PAGE_KERNEL >> 12;    //get the upper 10 bits

    for(i = 2 ; i < NUM_PDE; i++) {
        pde[i].present = 0;
        pde[i].RW = 1;
        pde[i].US = 0;
        pde[i].PWT = 0;
        pde[i].PCD = 0;
        pde[i].access = 0;
        pde[i].reserved2 = 0;
        pde[i].PS = 0;
        pde[i].G = 0;
        pde[i].avail = 0;
        if (i == VIDMAP_PDE) {
            pde[i].addr = ((unsigned long)pte2 >> 12);              //set up the new page table for vidmap
        }else{
            pde[i].addr = 0;
        }
    }
    loadPageDirectory((int)pde);
    enablePaging();
}
