/**************************************************************
 * Class:  CSC-415-0#
 * Name:
 * Student ID:
 * Project: File System Project
 *
 * File: b_io.h
 *
 * Description: Interface of basic I/O functions
 *
 **************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
typedef int b_io_fd;

#define MALLOC_ERR_MESS " ** Failed allocation of memory **"

typedef struct file_breakdown {
	char *buff_sector;
	int sector_size;
} sector_info;


int b_open (char * filename, int flags);
int b_read (int fd, char * buffer, int count);
int b_write (int fd, char * buffer, int count);
int b_seek (int fd, off_t offset, int whence);
void b_close (int fd);

#endif
