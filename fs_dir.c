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
#include "fsLow.h"
#include "utils/stack.h"
#include "mfs.h"
#include "bitmap_vector.h"
#include "date.h"

//
int dir_is_init();
int dir_load_config();
//

Bitvector * bitmap;
fs_stack * cwd_stack; /* A stack that contains inode ints */
fdDir * dir_table; /* An array of directories */

char init_key[] = "init=1\n";

int config_location;
int inode_index;

void initializeDirectory(Bitvector * vec, int LBA_Pos) {
	config_location = LBA_Pos;
	bitmap = vec;
	
	if (dir_is_init()) {
		dir_load_config();
	}
	
}

int dir_is_init() {
	char config_buffer[513];
	config_buffer[512] = '\0';
	LBAread(config_buffer, 1, config_location);

	if (strstr(config_buffer, init_key) != NULL)
		return 1;
	
	char * config_write_buffer = malloc(513);
	inode_index = 1;
	snprintf(config_write_buffer, "%s\ninode_index=%d", *init_key, inode_index);
	LBAwrite(config_write_buffer, 1, config_location);
	
	return 0;
}

int dir_load_config() {
		char config_buffer[513];
		config_buffer[512] = '\0';
		LBAread(config_buffer, 1, config_location);
		
		char * saveptr;
		char * setting_val = strtok_r(config_buffer, "=\n", &saveptr);
		
		while (setting_val != NULL) {
			printf("CONF");
			setting_val = strtok_r(config_buffer, "=\n", &saveptr);
		}
		
		
	
}






























