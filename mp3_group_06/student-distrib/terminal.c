#include "terminal.h"
#include "process.h"
#define NUM_COLS    80
#define NUM_ROWS    25

static int8_t char_buf[3][BUF_SIZE];
static int bytes_copied;
//volatile int enter_flag;

/* copy_kb_buff
 * Description: Read kb buf into char buf
 * Inputs: kb_buff - pointer to kb buffer
 *         n_bytes - number of bytes copied from kb
 * Outputs: none
 * Side Effects: char_buf, bytes_copied, enter_flag changed
 */
void copy_kb_buff(void* kb_buff, int index, int n_bytes)
{
    int i;
    bytes_copied = 0;
    
    // iterate over keyboard buffer
    for (i = 0; i < n_bytes; i++)
    {
        // store current byte into read buffer
        char_buf[index][i] = *((int8_t*)kb_buff+i);
        // increment counter
        bytes_copied++;
    }
    bytes_copied++;
    // add new line to the end of buffer
    char_buf[index][bytes_copied-1] = '\n';
    //putc('\n');
    terminals[index].enter_flag = 1;
}

/* terminal_open
 * Description: Initialize variables specific to terminal
 * Inputs: filename - file name
 * Outputs: 0
 * Side Effects: none
 */
int terminal_open(const uint8_t* filename)
{
    // clear buffer
    terminal_init();
    return 0;
}

/* terminal_open
 * Description: set buf to spaces
 * Inputs: none
 * Outputs: none
 * Side Effects: buf cleared
 */
void terminal_init(void)
{
    // int i,j;
    // for (j = 0 ; j < 3 ; j++) {
    // for (i = 0; i < BUF_SIZE; i++)
    // {
    //     char_buf[j][i] = '\0';
    // }
    // bytes_copied = 0;
    // enter_flag = 0;
    // }
}

/* terminal_close
 * Description: reset variables specific to terminal
 * Inputs: fd - function descriptor
 * Outputs: 0
 * Side Effects: buf cleared
 */
int terminal_close(int32_t fd)
{
    // clear buffer
    // int i,j;
    // for (j = 0 ; j < 3 ; j++) {
    // for (i = 0; i < BUF_SIZE; i++)
    // {
    //     char_buf[j][i] = '\0';
    // }
    // }
    // bytes_copied = 0;
    // enter_flag = 0;
    return 0;
}

/* terminal_read
 * Description: read keyboard input into buf
 * Inputs: fd - function descriptor
 *         buf - pointer to argument buffer
 *         n_bytes - number of bytes to be copied 
 * Outputs: number of bytes succefully copied or -1 on failure
 * Side Effects: buf changed
 */
int terminal_read(int32_t fd, void* buf, int32_t n_bytes)
{
    int i;
    int bytes_read;
    while (terminals[active_tid].enter_flag == 0)
    {
        /* code */
    }
    
    cli();
    bytes_read = 0;

    // parameter validation
    if (buf == NULL)
    {
        return -1;
    }
    
    if (n_bytes < bytes_copied)
    {
        for (i = 0; i < n_bytes-1; i++)
        {
            // store current byte into read buffer
            *((int8_t*)buf+i) = char_buf[active_tid][i];
            // char_buf[i] = ' ';
            // increment counter
            bytes_read++;
        }
        *((int8_t*)buf+n_bytes-1) = '\n';
        bytes_read++;
    } else 
    {
        for (i = 0; i < bytes_copied; i++)
        {
            // store current byte into read buffer
            *((int8_t*)buf+i) = char_buf[active_tid][i];
            // char_buf[i] = ' ';
            // increment counter
            bytes_read++;
        }
    }
    for (i = 0; i < BUF_SIZE; i++)
    {
        char_buf[active_tid][i] = '\0';
    }
    terminals[active_tid].enter_flag = 0;
    bytes_copied = 0;
    sti();
    // return byte successfully copied
    return bytes_read;
}

/* terminal_write
 * Description: print everything in buffer (except \0) to screen
 * Inputs: fd - function descriptor
 *         buf - pointer to argument buffer
 *         n_bytes - number of bytes to be copied 
 * Outputs: Number of bytes succefully copied or -1 on failure
 * Side Effects: none
 */
int terminal_write(int32_t fd, const void* buf, int32_t n_bytes)
{
    int count_bytes;
    int i;
    char c;

    count_bytes = 0;

    // parameter validation
    if (buf == NULL)
    {
        return -1;
    }

    // iterate over argument buffer
    for (i = 0; i < n_bytes; i++)
    {
        c = *((int8_t*)buf+i);
        // do not print NULL terminator
        if (c != '\0')
        {
            // print current byte
            putc(c);
            // increment counter
            count_bytes++;
        }
        
    }
    // return byte successfully copied
    return count_bytes;
}

