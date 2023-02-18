#ifndef _PIT_H
#define _PIT_H

#include "types.h"
#include "process.h"
#include "lib.h"

#define CHANNEL_ZERO_PORT   0x40
#define CM_REGISTER         0x43    
#define MAX_PIT_FREQ        1193182
#define BYTE_MASK           0xff
#define PIT_IRQ             0


void pit_init();
void pit_handler();
#endif /* _PIT_H */

