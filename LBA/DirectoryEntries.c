/**************************************************************
* Class:  CSC-415
* Name: Mark Jovero
* Student ID:   916691664
* Project: Basic File System
*
* File: DirectoryEntries.c
*
* Description: This file is responsible for managing directory entries.
*   It also manages inode tables and maps each inode to their
*   corresponding directory/file.
*
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../fsLow.h"
#include "../utils/stack.h"
#include "lba.h"
#include "../bitmap_vector.h"
#include "../date.h"

Bitvector * bitmap;
fs_stack * cwd_stack; /* A stack that contains inode ints */
fdDir * dir_table; /* An array of directories */

int inode_index;

