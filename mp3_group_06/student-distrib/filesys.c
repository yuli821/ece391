#include "filesys.h"
#include "process.h"

/* filesys_init(fs_start);
 * Inputs: A pointer to the start of the filesys image
 * Return Value: none
 * Function: set bootblock_hd, inode_hd, data_hd, DENTRY_NUM, INODE_NUM, DBLOCK_NUM, also initialize temp_pcb
 */
void filesys_init(unsigned int fs_start) {
    bootblock_hd = (bootblock_t*)fs_start;
    DENTRY_NUM = bootblock_hd->dir_count;
    INODE_NUM = bootblock_hd->inodes_count;
    DBLOCK_NUM = bootblock_hd->data_count;
    inode_hd = (inode_t*)((unsigned int)fs_start+BLOCK_SIZE);    //the second block is the start of inode array
    data_hd = (data_block_t*)((unsigned int)fs_start + (INODE_NUM+1)*BLOCK_SIZE);   //find the start of the data block array
    // int i;
    // for(i = 0; i < 8; i++) {
    //     temp_pcb[i].file_op_jump_table = 0;                    //initialize everything to 0
    //     temp_pcb[i].inode = 0;
    //     temp_pcb[i].file_pos = 0;
    //     temp_pcb[i].flags = 0;
    // }
}

/* read_dentry_by_name(fname, dentry);
 * Inputs: fname: a pointer to a const string; dentry: a pointer to a directory entry
 * Return Value: 0 if successful, -1 if fail
 * Function: Given the file name that we want to find, find the directory entry in the bootblock, and fill the dentry passed in parameter
 */
int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry) {
    uint32_t i;
    uint32_t dentry_len;
    uint32_t flen = strlen((int8_t*)fname);
    if (flen > MAX_FILENAME_LEN) {                  //if the length of the parameter fname is greater than 32
        return -1;                    //set it to 32
    }
    for(i = 0; i < DENTRY_NUM; i++) {               //traverse all the dentry existed
        dentry_len = strlen((int8_t*)bootblock_hd->dentry_array[i].filename);
        if (dentry_len > MAX_FILENAME_LEN) {
            dentry_len = MAX_FILENAME_LEN;
        }
        if (flen == dentry_len){                    //only check if the file name length matches
            if(!strncmp((int8_t*)fname, (int8_t*)bootblock_hd->dentry_array[i].filename, MAX_FILENAME_LEN)) {
                read_dentry_by_index(i, dentry);    //call this function to simplify process
                return 0;
            }
        }
    }
    return -1;
}

/* read_dentry_by_index(index, dentry);
 * Inputs: index: index in the bootblock dentry_array; dentry: a pointer to a directory entry
 * Return Value: 0 if successful, -1 if fail
 * Function: Given the index of dentry that we want to find, find the directory entry in the bootblock, and fill the dentry passed in parameter
 */
int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry) {
    if (index >= DENTRY_NUM) {              //if index is outside the range of dentry, fail
        return -1;
    }
    strncpy((int8_t*)dentry->filename, (int8_t*)bootblock_hd->dentry_array[index].filename, MAX_FILENAME_LEN);
    dentry->filetype = bootblock_hd->dentry_array[index].filetype;
    dentry->inode_num = bootblock_hd->dentry_array[index].inode_num;
    return 0;
}

/* read_data(inodex, offset, buf, length);
 * Inputs: inode: index of the inode we want to read from; offset: offset in bytes from the file start, indicate where we want to start reading
 *         buf: pointer to a byte array, store the data; length: number of bytes we want to read
 * Return Value: bytes read if successful, -1 if fail
 * Function: Read length bytes of data from the file indicating by inode index, starting from offset, the data is output to buf
 */
int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length) {
    uint32_t dblock_index;                         //store the index of the data block
    uint32_t dblock_offset;                        //store the offset within the data block
    uint32_t dblock_num;                           //store the total number of data block
    if (inode >= INODE_NUM) {                      //if the given index is outside of the range, fail
        return -1;
    }
    inode_t* curr_inode = inode_hd + inode;        //the inode pointer pointing to the inode, given the index
    uint32_t byte_count = curr_inode->length;      //the total bytes inside this file
    data_block_t* dblock;                          //pointer to the data block we want to read from
    int i;
    for(i = 0 ; i < length ; i++) {
        if ((i + offset) >= byte_count) {          //if reach the end of the file, return the bytes read
            return i;
        }
        dblock_index = (offset + i)/BLOCK_SIZE;
        dblock_offset = (offset + i)%BLOCK_SIZE;
        dblock_num = curr_inode->data_block_num[dblock_index];
        dblock = data_hd + dblock_num;
        buf[i] = dblock->data_array[dblock_offset];   //fill in the buf with the data from data block
    }
    return length;
} 

/* dir_read(fd, buf, nbytes);
 * Inputs: fd: file descriptor index in pcb; buf: the buffer we want to store the data we read; nbytes: not sure how to use here
 * Return Value: file length if successful, -1 if fail
 * Function: Read a file name in this directory, from dentry to buf
 */
int32_t dir_read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* curr_pcb = get_active_pcb();
    int32_t bytes_read;
    if (curr_pcb->fd_array[fd].flags == FD_FREE) {                //if the file is closed, fail
        return 0;
    }
    uint32_t index = curr_pcb->fd_array[fd].file_pos;             //file_pos in the pcb gives the index into dentry
    dentry_t file;
    if (read_dentry_by_index(index, &file) == 0) {      //if read_dentry_by_index is success
        strncpy((int8_t*)buf, (int8_t*)&(file.filename), MAX_FILENAME_LEN);  //copy the file name into buf
        curr_pcb->fd_array[fd].file_pos++;                        //update the file_pos
        bytes_read = strlen(buf);
        if (bytes_read > MAX_FILENAME_LEN) {
            return MAX_FILENAME_LEN;
        }
        return strlen(buf);
    } else {
        return 0;
    }
}

/* dir_open(fname);
 * Inputs: fname: file name we are looking for; fd: file descriptor index
 * Return Value: 0 if successful, -1 if fail
 * Function: set up the file descriptor in the temp pcb
 */
int32_t dir_open(const uint8_t* fname) {
    return 0;
}

/* dir_close(fd);
 * Inputs: fd: file descriptor index in pcb
 * Return Value: always return 0
 * Function: Mark the fd as free
 */
int32_t dir_close(int fd) {
    return 0;
}

/* dir_write();
 * Inputs: none
 * Return Value: -1
 * Function: Do nothing
 */
int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}

/* f_read(fd, buf, nbytes);
 * Inputs: fd: file descriptor index in pcb; buf: buffer we store the read data; nbytes: bytes we want to read
 * Return Value: return bytes read if success, 1 if fail
 * Function: Read nbytes of data from file to the buf
 */
int32_t f_read(int32_t fd, void* buf, int32_t nbytes) {
    pcb_t* curr_pcb = get_active_pcb();
    if (curr_pcb->fd_array[fd].flags == FD_FREE) {            //if fd is closed, fail
        return -1;
    }
    fd_t descriptor = curr_pcb->fd_array[fd];
    uint32_t bytes = read_data(descriptor.inode, descriptor.file_pos, (uint8_t*)buf, (uint32_t)nbytes);
    if (bytes != -1) {                              //number of data we actually read out from the file
        curr_pcb->fd_array[fd].file_pos += bytes;             //update the file_pos by bytes we read
        return bytes;
    } else {
        return -1;
    }
}

/* f_open(fname);
 * Inputs: fname: file we want to open; fd: file descriptor index in pcb
 * Return Value: return 0 if success, 1 if fail
 * Function: Set up the file descriptor
 */
int32_t f_open(const uint8_t* fname) {
    return 0;
}

/* f_close(fd);
 * Inputs: fd: file descriptor index in pcb
 * Return Value: always return 0
 * Function: Mark file as free
 */
int32_t f_close(int fd) {
    pcb_t* curr_pcb = get_active_pcb();
    curr_pcb->fd_array[fd].flags = FD_FREE;
    return 0;
}

/* f_write();
 * Inputs: none
 * Return Value: -1
 * Function: do nothing
 */
int32_t f_write(int32_t fd, const void* buf, int32_t nbytes) {
    return -1;
}
