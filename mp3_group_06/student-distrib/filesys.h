#ifndef _FILESYS_H
#define _FILESYS_H

#include "types.h"

#define MAX_FILENAME_LEN   32              //The file name is up to 32 characters
#define BLOCK_SIZE         4096            //One block size is 4kb
#define FD_FREE            1               //The flag in fd to distinguish whether the fd is in use
#define FD_IN_USE          0

/*Dentry struct, directory entry in bootblock, 64B total*/
typedef struct directory_entries {
    uint8_t filename[32];                  //32 characters file name
    uint32_t filetype;                     //0 for RTC, 1 for directory, 2 for regular file 
    uint32_t inode_num;                    //Index in inode array
    uint8_t reserved[24];                  //reserved section is 24 bytes
} dentry_t;

/*bootblock struct, first 4kb in the file img*/
typedef struct bootblock {
    uint32_t dir_count;                    //total number of directory
    uint32_t inodes_count;                 //total number of inode
    uint32_t data_count;                   //total number of data blocks
    uint8_t reserved[52];                  //reserved section is 52 bytes
    dentry_t dentry_array[63];             //63 dentry, 1 for . directory, 62 files
} bootblock_t;

/*inode struct, contains index into data block array*/
typedef struct inode {
    uint32_t length;                       //number of data block for the file represented by the inode
    uint32_t data_block_num[1023];         //an array of data blocks
} inode_t;

/*data block struct, contain the 4kb data of a file*/
typedef struct data_block {
    uint8_t data_array[4096];              //4kb data
} data_block_t;

// typedef struct file_op {
//     int32_t (*open_file)(const uint8_t* fname);
//     int32_t (*read_file)(int32_t fd, void* buf, int32_t nbytes);
//     int32_t (*close_file)(int32_t fd);
//     int32_t (*write_file)(int32_t fd, const void* buf, int32_t nbytes);
// } fop_t;

// /*a temprary struct of file descriptor struct*/
// typedef struct file_descriptor {
//     fop_t f_op;
//     uint32_t inode;
//     uint32_t file_pos;
//     uint32_t flags;
// } fd_t;


/*a temporary pcb block to store the file descriptor, the length of fd array is 8*/
//fd_t temp_pcb[8];

bootblock_t* bootblock_hd;                 //points to the head of the bootblock
inode_t* inode_hd;                         //points to the head of the inode array
data_block_t* data_hd;                     //points to the head of the data block array

uint32_t DENTRY_NUM;                       //store the number of dentry
uint32_t INODE_NUM;                        //store the number of inodes
uint32_t DBLOCK_NUM;                       //store the number of data blocks

extern void filesys_init(unsigned int fs_start);
extern int32_t read_dentry_by_name(const uint8_t* fname, dentry_t* dentry);
extern int32_t read_dentry_by_index(uint32_t index, dentry_t* dentry);
extern int32_t read_data(uint32_t inode, uint32_t offset, uint8_t* buf, uint32_t length);

extern int32_t dir_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t dir_open(const uint8_t* fname);
extern int32_t dir_close(int32_t fd);
extern int32_t dir_write(int32_t fd, const void* buf, int32_t nbytes);

extern int32_t f_read(int32_t fd, void* buf, int32_t nbytes);
extern int32_t f_open(const uint8_t* fname);
extern int32_t f_close(int32_t fd);
extern int32_t f_write(int32_t fd, const void* buf, int32_t nbytes);


#endif
