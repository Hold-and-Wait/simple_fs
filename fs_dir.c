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
void dir_table_expand();

int dir_is_init();
int dir_load_config();
int dir_is_valid_path(char * path, char * dir);
int dir_get_free_inode();
//

#define PREFIX "Directory >>"
#define DEF_PATH_SIZE 128
#define DEF_DIR_TABLE_SIZE 10

Bitvector * bitmap;
fs_stack * cwd_stack; /* A stack that contains inode ints */
fdDir * dir_table; /* An array of directories */

char init_key[] = "init=1";
char meta_key[] = "!&92@&fbn1337_amX";

int config_location;
int inode_index;
int num_table_expansions = 1;

void initializeDirectory(Bitvector * vec, int LBA_Pos) {
	config_location = LBA_Pos;
	bitmap = vec;

	if (dir_is_init())
        dir_load_config();

    // dir stack initialization
	cwd_stack = stack_create(DEF_PATH_SIZE);
	stack_push(0, cwd_stack);



	dir_table = malloc(sizeof(fdDir) * DEF_DIR_TABLE_SIZE * num_table_expansions);
	
	char * buf;
	fs_mkdir("ABC", DT_DIR);
	fs_mkdir("AfdBCa", DT_DIR);


}

int dir_is_init() {
	char config_buffer[513];
	config_buffer[512] = '\0';
	LBAread(config_buffer, 1, config_location);
	
	if (strstr(config_buffer, init_key) != NULL) {
		return 1;
	}
	
	printf("%s First initialization. Setting default values.\n", PREFIX);
	char * config_write_buffer = malloc(513);
	inode_index = 1;
	snprintf(config_write_buffer, 512, "%s\ninode_index=%d\ndir_exp=%d", init_key, inode_index, num_table_expansions);
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
		}
		else if (strstr(setting_val, "dir_exp") != NULL) {
			setting_val = strtok_r(NULL, "=\n", &saveptr);
			num_table_expansions = atoi(setting_val);
		}
		setting_val = strtok_r(NULL, "=\n", &saveptr);
	}
	
	printf("%s Successfully loaded configs into memory. inode_index=%d, num_table_expansions=%d\n", PREFIX, inode_index, num_table_expansions);
	return 1;
}

int dir_offload_configs() {
	char * config_write_buffer = malloc(513);
	config_write_buffer[512] = '\0';
	
	snprintf(config_write_buffer, 512, "%s\ninode_index=%d\ndir_exp=%d", init_key, inode_index, num_table_expansions);
	LBAwrite(config_write_buffer, 1, config_location);
	
	printf("%s Successfully offloaded config values into volume.\n", PREFIX);
	free(config_write_buffer);
	return 1;
}

int dir_load_dir_table() {


    return -1;
}

// TODO
int dir_is_valid_path(char * path, char * new_dir_name) {
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
		if (f_slash_counter > 0) { // validate path component
			fdDir * dir = fs_opendir(dir_name);
			if (dir == NULL) {
				printf("%s ERROR: \'%s\' is invalid. (cause: not a directory type or does not exist)\n", PREFIX, dir_name);
				return -1;
			}
			
			stack_push(dir->inode, stack_path_temp);
			
			f_slash_counter--;
		}
		else {	// can be valid/invalid last path component
			strcpy(new_dir_name, dir_name);
			return 1;
		}

		dir_name = strtok_r(NULL, "/", &saveptr);
	}

	return -1;
}

void dir_table_expand() {
	num_table_expansions++;
	dir_table = realloc(dir_table, sizeof(fdDir) * DEF_DIR_TABLE_SIZE * num_table_expansions);
	printf("%s Expanded directory table. New size: %d\n", PREFIX, DEF_DIR_TABLE_SIZE * num_table_expansions);
}

int fs_mkdir(char *pathname, mode_t mode) {
	
	fs_stack * stack_path_temp;
	int f_slash_counter = 0;

	for (int i = 0; i < strlen(pathname); i++) {
		if (pathname[i] == '/') {
			if (i == 0 || i == strlen(pathname)-1)
				continue;
			f_slash_counter++;
		}
	}
	
	
	if (pathname[0] == '/')
		stack_path_temp = stack_create(DEF_PATH_SIZE);
	else
		stack_path_temp = stack_copy(cwd_stack);
		
	char path_c[strlen(pathname)];
	strcpy(path_c, pathname);
	
	char * saveptr;
	char * dir_name = strtok_r(path_c, "/", &saveptr);
	
	while (dir_name != NULL) {
		if (f_slash_counter > 0) { // validate path components
			int is_found = 0;
			if (strcmp(dir_name, "..") == 0) {
				stack_pop(stack_path_temp);
			} else {
				fdDir * dir_iter = dir_table;
				for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
					if (!dir_iter->is_used)
						continue;
					if (dir_iter->parent_inode != stack_peek(stack_path_temp))
						continue;
					
					struct fs_diriteminfo * dir_info = fs_readdir(dir_iter);
					if (strcmp(dir_info->d_name, dir_name) == 0) {
						is_found = 1;
						break;
					}
				}
			}
			if (!is_found) {
				printf("%s mkdir error: invalid path component \'%s\'.\n", PREFIX, dir_name);
				return -1;
			}
			f_slash_counter--;
		} else {	// last component may be invalid -> create dir
		
			char * meta_write_buffer = malloc(513);
			
			// Assign lba pos
			int fb_array[1];
			int * free_blocks = get_free_blocks_index(bitmap, fb_array, 1);
			for (int i = 0; i < 1; ++i)
			    set_bit(bitmap, free_blocks[i], 1);
                		
			// Load into dir table
			fdDir * dir_iter = dir_table;
            struct fs_diriteminfo * dir_meta;
			for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
				if (dir_iter->is_used > 0)
					continue;

				dir_meta = fs_readdir(dir_iter);
				
				dir_iter->is_used = 1;
				dir_iter->inode = dir_get_free_inode();
				dir_iter->parent_inode = stack_peek(stack_path_temp);
				dir_iter->directoryStartLocation = free_blocks[i];
				
				return 1;
			}

			// write to lba
			snprintf(meta_write_buffer, 513, "key=%s\nname=%s\ntype=%c\ninode=%d\npinode=%d\nsize=%d\nlbapos=%d\nblen=%d\nmdate=%s\ncdate=%s",
					meta_key, dir_meta->d_name, 'D', dir_iter->inode, dir_iter->parent_inode, 0, free_blocks[0], 1, NULL, NULL );
            LBAwrite(meta_write_buffer, 1, free_blocks[0]);
                	
                	 
			free(meta_write_buffer);
		}
		dir_name = strtok_r(NULL, "/", &saveptr);
	
	}
	printf("%s mkdir error: undefined.\n", PREFIX);
	return -1;
}

int dir_get_free_inode() {
	int index_found = -1;
	for (int i = 1; i <= inode_index; i++) {
		fdDir * entry = dir_table;
		for (int j = 0; j < num_table_expansions * DEF_DIR_TABLE_SIZE; j++, entry++) {
			if (entry->inode == i) {
				index_found = -1;
				break;
			} else {
                		index_found = i;
            		}
        	}
        	if (index_found > 0)
            		break;
    	}
    	if (index_found == -1) {
        	index_found = inode_index++;
    	}
    	if (index_found == 0) {
    		index_found = dir_get_free_inode();
    	}
    	
    	printf("!! %d\n", index_found);
	return  index_found;
}

struct fs_diriteminfo * fs_readdir(fdDir *dirp) {
	struct fs_diriteminfo * dirent_info = malloc(sizeof(struct fs_diriteminfo));
	int lba_to_read = dirp->directoryStartLocation;
	char * metadata_read = malloc(513);
	metadata_read[512] = '\0';
	
	LBAread(metadata_read, 1, lba_to_read);
	char md_read[strlen(metadata_read)];
	strcpy(md_read, metadata_read);
	
	char * saveptr;
	char * meta_val = strtok_r(md_read, "=\n", &saveptr);
	
	while (meta_val != NULL) {
		if (strstr(meta_val, "name") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			strcpy(dirent_info->d_name, meta_val);
		}
		else if (strstr(meta_val, "p_inode") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			dirent_info->parent_inode = atoi(meta_val);
		}
		else if (strstr(meta_val, "inode") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			dirent_info->inode = atoi(meta_val);
		}
		else if (strstr(meta_val, "type") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			if (strcmp(meta_val, "D"))
				dirent_info->fileType = 'D';
			if (strcmp(meta_val, "R"))
				dirent_info->fileType = 'R';
			if (strcmp(meta_val, "L"))
				dirent_info->fileType = 'L';
		}
		else if (strstr(meta_val, "b_len") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			dirent_info->d_reclen = (unsigned short) atoi(meta_val);
		}
		else if (strstr(meta_val, "f_size") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			dirent_info->file_size = (unsigned) atoi(meta_val);
		}
		meta_val = strtok_r(NULL, "=\n", &saveptr);
	}
	
	free(metadata_read);
	return dirent_info;
}

fdDir * fs_opendir(const char *name) {
	fdDir * cur_dir_ptr = dir_table;
	fdDir * dir_stream = malloc(sizeof(fdDir) * DEF_PATH_SIZE);
	fdDir * start_stream = dir_stream;
	
	struct fs_diriteminfo * dirent_info;
	
	int current_pos = 0;
	int dir_found = 0;
	
	// handle dot and dot-dot TODO

	
	cur_dir_ptr = dir_table;
	
	for (int i = 0; i < DEF_DIR_TABLE_SIZE * num_table_expansions; i++, cur_dir_ptr++) {
		dirent_info = fs_readdir(cur_dir_ptr);
		if (dirent_info->inode == 0) 
			continue;
		if (strcmp(dirent_info->d_name, name) == 0) {
			dir_found = 1;
			cur_dir_ptr = dir_table;
			for (int j = 0; j < DEF_DIR_TABLE_SIZE * num_table_expansions; j++, cur_dir_ptr++) {
				if (dirent_info->inode == cur_dir_ptr->parent_inode) {
					dir_stream = cur_dir_ptr;
					dir_stream++;
				}
			}
		}
		
		if (dir_found)
			break;
		
	}
	
	if (!dir_found)
		return NULL;
	
	dir_stream = start_stream;
	return dir_stream;
}























