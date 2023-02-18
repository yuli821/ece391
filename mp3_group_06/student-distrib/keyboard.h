#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "types.h"
//#include <stdint.h>
#include "lib.h"

#define ESC         0x27
#define BS          0x08
#define ENTR        0x28

//modifiers
#define LSHIFT      0x2A
#define RSHIFT      0X36
#define CAPSLK      0x3A
#define CTRL        0x1D
#define RELEASE     0x80

#define KB_IRQ      0X01
#define KB_DATA     0X60
#define KB_CMD      0x64
#define KB_BUF_LEN  128
#define SCAN_BEGIN 0
#define SCAN_END 58

#define Q 16
#define P 25
#define A 30
#define L 38
#define Z 44
#define M 50
#define BLANK 0x00
#define TAB 15
#define ALT 56
#define KP_M 55
#define F1 0x3B
#define F2 0x3C
#define F3 0x3D

uint8_t is_modifier(uint8_t scancode);
uint8_t is_letter(uint8_t scancode);

extern void keyboard_init(void);
extern void keyboard_handler(void);

void put_to_buffer(char* buf, char c);
void remove_from_buffer(char* buf);

void clear_buffer(char* buf, int buffer_counter);
#endif
