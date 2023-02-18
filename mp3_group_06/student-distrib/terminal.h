#ifndef _TERMINAL_H
#define _TERMINAL_H

#include "types.h"
#include "lib.h"

#define BUF_SIZE 128

// volatile int enter_flag = 0;

int terminal_open(const uint8_t* filename);

void terminal_init(void);

int terminal_close(int32_t fd);

int terminal_read(int32_t fd, void* buf, int32_t n_bytes);

int terminal_write(int32_t fd, const void* buf, int32_t n_bytes);

void copy_kb_buff(void* kb_buff, int index, int n_bytes);

#endif
