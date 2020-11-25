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
int dir_is_valid_path(char * path, char * dir);
//

#define PREFIX "Directory >>"
#define DEF_PATH_SIZE 128

Bitvector * bitmap;
fs_stack * cwd_stack; /* A stack that contains inode ints */
fdDir * dir_table; /* An array of directories */

char init_key[] = "init=1";

int config_location;
int inode_index;

void initializeDirectory(Bitvector * vec, int LBA_Pos) {
	config_location = LBA_Pos;
	bitmap = vec;
	
	// dir stack initialization
	cwd_stack = stack_create(DEF_PATH_SIZE);
	stack_push(0, cwd_stack);

	if (dir_is_init()) {
		dir_load_config();
	}
	char * buf;
	dir_is_valid_path("A/B/C", buf);
	
}

int dir_is_init() {
	char config_buffer[513];
	config_buffer[512] = '\0';
	LBAread(config_buffer, 1, config_location);
	
	if (strstr(config_buffer, init_key) != NULL) {
		return 1;
	}
	
	printf("%s First initialization. Setting default values.\n", "");
	char * config_write_buffer = malloc(513);
	inode_index = 1;
	snprintf(config_write_buffer, 512, "%s\ninode_index=%d", init_key, inode_index);
	LBAwrite(config_write_buffer, 1, config_location);
	free(config_write_buffer);
	return 0;
}

int dir_load_config() {
	char config_buffer[513];
	config_buffer[512] = '\0';
	LBAread(config_buffer, 1, config_location);
		
	char * saveptr;
	char * setting_val = strtok_r(config_buffer, "=\n", &saveptr);
	
	while (setting_val != NULL) {
		if (strstr(setting_val, "inode_index") != NULL) {
			setting_val = strtok_r(NULL, "=\n", &saveptr);
			inode_index = atoi(setting_val);
			if (inode_index == 0)
				return 0;
		}
		setting_val = strtok_r(NULL, "=\n", &saveptr);
	}
	
	printf("%s Successfully loaded configs into memory. inode_index=%d\n", PREFIX, inode_index);
	return 1;
}

int dir_offload_configs() {
	char * config_write_buffer = malloc(513);
	config_write_buffer[512] = '\0';
	
	snprintf(config_write_buffer, 512, "%s\ninode_index=%d", init_key, inode_index);
	LBAwrite(config_write_buffer, 1, config_location);
	
	printf("%s Successfully offloaded config values into volume.\n", PREFIX);
	free(config_write_buffer);
	return 1;
}

int dir_is_valid_path(char * path, char * dir) {
	fs_stack * stack_path_temp;
	int f_slash_counter = 0;
	
	for (int i = 0; i < strlen(path); i++) {
		if (path[i] == '/') {
			if (i == 0 || i == strlen(path)-1)
				continue;
			f_slash_counter++;
		}
	}
	
	if (path[0] == '/')
		stack_path_temp = stack_create(DEF_PATH_SIZE);
	else
		stack_path_temp = stack_copy(cwd_stack);
		
	char path_c[strlen(path)];
	strcpy(path_c, path);
	
	char * saveptr;
	char * dir_name = strtok_r(path_c, "/", &saveptr);
	
	while (dir_name != NULL) {
		printf("%s\n", dir_name);
		dir_name = strtok_r(NULL, "/", &saveptr);
	}

	return -1;
}




























