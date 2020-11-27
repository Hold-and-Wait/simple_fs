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
int dir_reload();
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
int dir_count = 0;
int num_table_expansions = 1;

void initializeDirectory(Bitvector * vec, int LBA_Pos) {
	config_location = LBA_Pos;
	bitmap = vec;

	if (dir_is_init()) {
        	dir_load_config();
        	dir_table = malloc(sizeof(fdDir) * DEF_DIR_TABLE_SIZE * num_table_expansions);
        	dir_reload();
        } else {
        	dir_table = malloc(sizeof(fdDir) * DEF_DIR_TABLE_SIZE);
        }

    // dir stack initialization
	cwd_stack = stack_create(DEF_PATH_SIZE);
	stack_push(0, cwd_stack);
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
	
	printf("%s Successfully offloaded config values into volume. inode_index=%d, num_table_expansions=%d\n", PREFIX, inode_index, num_table_expansions);
	free(config_write_buffer);
	return 1;
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
					if (dir_iter->is_used != 1)
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
			dir_count++;
			if (dir_count > num_table_expansions * DEF_DIR_TABLE_SIZE)
				dir_table_expand();

			for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
				if (dir_iter->is_used == 1)
					continue;
				dir_iter->is_used = 1;
				dir_iter->inode = inode_index++;
				dir_iter->parent_inode = stack_peek(stack_path_temp);
				dir_iter->directoryStartLocation = free_blocks[0];
				
				break;
			}

			// write to lba
			snprintf(meta_write_buffer, 513, "key=%s\nname=%s\ntype=%c\ninode=%d\npinode=%d\nsize=%d\nlbapos=%d\nblen=%d\nmdate=%s\ncdate=%s",
					meta_key, dir_name, 'D', dir_iter->inode, dir_iter->parent_inode, 0, free_blocks[0], 1, "", "" );
			LBAwrite(meta_write_buffer, 1, free_blocks[0]);
                	
			free(meta_write_buffer);
			
			printf("%s mkdir: New directory created: %s\n", PREFIX, dir_name);
			return 1;
		}
		dir_name = strtok_r(NULL, "/", &saveptr);
	
	}
	printf("%s mkdir error: undefined.\n", PREFIX);
	return -1;
}

char * fs_getcwd(char *buf, size_t size) {
	fs_stack * temp = stack_copy(cwd_stack);
	fs_stack * reverse_cwd = stack_create(DEF_PATH_SIZE);
	while (stack_size(temp) > 0)
        	stack_push(stack_pop(temp), reverse_cwd);


    	int pos = 0;
    //buf[pos] = '/';
    //pos++;
    	char * read_buf = malloc(513);

	while (stack_size(reverse_cwd) > 0) {
        	fdDir * tableptr = dir_table;
        	int node = stack_pop(reverse_cwd);
        	int lba_loc = 0;
        	if (node == 0) {
            		tableptr++;
           		continue;
        	}

        	buf[pos++] = '/';

        	while (tableptr->inode != node)
            		tableptr++;

        	if (tableptr->inode == node) {
            		struct fs_diriteminfo * dirinfo = fs_readdir(tableptr);
            		for (int i = 0; i < strlen(dirinfo->d_name); i++) {
                		buf[pos++] = dirinfo->d_name[i];
            		}
            //buf[pos++] = '/';
        	}

        	tableptr++;
    	}
    	
    	buf[pos++] = '/';
    	buf[pos++] = '\0'; // null terminator
    	free(read_buf);
    	return buf;
}

int fs_setcwd(char *path) {
	if (path[0] == '/')
        	while (stack_size(cwd_stack) > 1)
           		stack_pop(cwd_stack);

    //
	int stack_push_count = 0;

    	char * saveptr;
    	char * test = malloc(strlen(path));
    	strcpy(test, path);
    	char * dir_name = strtok_r(test, "/", &saveptr);

    	while (dir_name != NULL) {

        	if (strcmp(dir_name, "..") == 0) {
            		stack_pop(cwd_stack);
            		//char * buf = malloc(MAX_PATH);
            		//fs_getcwd(buf, MAX_PATH);
            		//free(buf);
            		dir_name = strtok_r(NULL, "/", &saveptr);
            		continue;
        	}


        	int is_found = 0;
        	fdDir * entry = dir_table;
        	for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {

            		if (!entry->is_used || entry->parent_inode != stack_peek(cwd_stack))
                		continue;                                                                                        
            		// check valid
            		struct fs_diriteminfo * dirinfo = fs_readdir(entry);

           		if (strcmp(dir_name, dirinfo->d_name) == 0) {
                		stack_push(entry->inode, cwd_stack);
                		stack_push_count++;
                		is_found = 1;
            		}
        	}

		if (is_found == 0) {
			printf("%s setcwd: Error. \'%s\' was not found. (Cause: It is not a valid directory type, or it is not in path.)\n", PREFIX, dir_name);

            		// undo directory pushes to cwd_stacks
            		while (stack_push_count > 0 && stack_size(cwd_stack) > 0)
                		stack_pop(cwd_stack);
            		return -1;
        	}
        	dir_name = strtok_r(NULL, "/", &saveptr);
	}

	char * buf = malloc(256);

	fs_getcwd(buf, 256);
	printf("%s setcwd: Successful change directory to %s.\n", PREFIX, buf);
	free(buf);
	return 1;
}

int dir_get_free_inode() {
    	int index_found = -1;
    	for (int i = 1; i < inode_index; i++) {
        	fdDir * entry = dir_table;
        	for (int j = 0; j < num_table_expansions * DEF_DIR_TABLE_SIZE; j++, entry++) {
        		if (entry->is_used > 0)
        			continue;
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
    		inode_index++;
        	index_found = inode_index;
    	}
    return  index_found;
}

int dir_reload() {

	int loaded_dir_counter = 0;

	int vec_size = get_vector_size(bitmap);
	char * meta_buf = malloc(513);
	meta_buf[512] = '\0';
	
	fdDir * dir_ptr = dir_table;
	
	for (int i = 0; i < vec_size; i++) {
		LBAread(meta_buf, 1, i);
		if (strstr(meta_buf, meta_key) != NULL) {
			char * saveptr;
			char * meta_val = strtok_r(meta_buf, "=\n", &saveptr);
			
			dir_count++;
			if (dir_count > num_table_expansions * DEF_DIR_TABLE_SIZE)
				dir_table_expand();
			
			dir_ptr->is_used = 1;
			dir_ptr->directoryStartLocation = i;
			
			// notify bitmap
			set_bit(bitmap, i, 1);
			
			while (meta_val != NULL) {
				if (strstr(meta_val, "pinode") != NULL) {
					meta_val = strtok_r(NULL, "=\n", &saveptr);
					dir_ptr->parent_inode = atoi(meta_val);
				}
				else if (strstr(meta_val, "inode") != NULL) {
					meta_val = strtok_r(NULL, "=\n", &saveptr);
					dir_ptr->inode = atoi(meta_val);
				}
				
				meta_val = strtok_r(NULL, "=\n", &saveptr);
			}
			
			dir_ptr++;
			loaded_dir_counter++;
		}
	}

	free(meta_buf);
	return loaded_dir_counter;
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
		else if (strstr(meta_val, "pinode") != NULL) {
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
		else if (strstr(meta_val, "blen") != NULL) {
			meta_val = strtok_r(NULL, "=\n", &saveptr);
			dirent_info->d_reclen = (unsigned short) atoi(meta_val);
		}
		else if (strstr(meta_val, "size") != NULL) {
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

void print_dir() {
    char * buf = malloc(513);
    printf("%s Printing Directory Entries Table:\n", PREFIX);

    // PRINT TABLE HEADERS
    printf(" ");
    for (int i = 0; i <= 101; i++) {
        if (i == 0) {
            printf("\u250f");
            continue;
        }
        if (i == 101) {
            printf("\u2513");
            continue;
        }
        if (i == 20 || i == 35 || i == 50 || i == 65 || i == 80 || i == 94) {
            printf("\u2533");
            continue;
        }
        printf("\u2501");
    }
    printf("\n");
    printf(" \u2503     file_name     \u2503");
    printf("    inode     \u2503");
    printf(" parent_inode \u2503");
    printf("   lba_pos    \u2503");
    printf("  block_size  \u2503");
    printf("  file_size  \u2503");
    printf(" type \u2503\n ");
    for (int i = 0; i <= 101; i++) {
        if (i == 0) {
            printf("\u2523");
            continue;
        }
        if (i == 101) {
            printf("\u252b");
            continue;
        }
        if (i == 20 || i == 35 || i == 50 || i == 65 || i == 80 || i == 94) {
            printf("\u2547");
            continue;
        }
        printf("\u2501");
    }
    printf("\n ");

    // END HEADER

    // DATA ENTRY
    fdDir * entry = dir_table;
    for (int i = 0; i < DEF_DIR_TABLE_SIZE * num_table_expansions; i++, entry++) {
    	if (entry->is_used <= 0)
    		continue;
        if (entry->is_used) {

            if (entry->directoryStartLocation <= config_location || entry->inode < 0 || entry->parent_inode < 0  || entry->inode > 999999 || entry->parent_inode > 999999)
                continue;

            char * buf = malloc(512);
            LBAread(buf, 1, entry->directoryStartLocation);

            char file_name[256];
            char file_type[2];
            char file_type_long[5];
            int block_size;
            char * saveptr;
            char * f_data = strtok_r(buf, "=\n", &saveptr);

            while (f_data != NULL) {

                if (strstr(f_data, "name") != NULL) {
                    f_data = strtok_r(NULL, "=\n", &saveptr);
                    strcpy(file_name, f_data);
                }if (strstr(f_data, "type") != NULL) {
                    f_data = strtok_r(NULL, "=\n", &saveptr);
                    strcpy(file_type, f_data);
                }
                if (strstr(f_data, "blen") != NULL) {
                    f_data = strtok_r(NULL, "=\n", &saveptr);
                    block_size = atoi(f_data);
                }

                f_data = strtok_r(NULL, "=\n", &saveptr);
            }

            if (strcmp(file_type, "R") == 0) {
                strcpy(file_type_long, "REG");
            }else if (strcmp(file_type, "D") == 0) {
                strcpy(file_type_long, "DIR");
            }

            char * file_name_trunc = malloc(18);
            if (strlen(file_name) > 15) {
                snprintf(file_name, 18, "%.14s...", file_name);
            }

            printf("\u2503%18.*s \u2502   %08d   \u2502   %08d   \u2502   %08lu   \u2502   %08d   \u2502  %08d   \u2502  %s \u2503\n ",
                   18,file_name, entry->inode, entry->parent_inode, (unsigned long) entry->directoryStartLocation, block_size, fs_readdir(entry)->file_size, file_type_long);

            free(file_name_trunc);
            free(buf);
        }
    }
    // END DATA
    // LAST ROW
    for (int i = 0; i <= 101; i++) {
        if (i == 0) {
            printf("\u2517");
            continue;
        }
        if (i == 101) {
            printf("\u251b");
            continue;
        }
        if (i == 20 || i == 35 || i == 50 || i == 65 || i == 80 || i == 94) {
            printf("\u2537");
            continue;
        }

        printf("\u2501");
    }
    
    printf("\n");
    // END TABLE

    free(buf);
}




















