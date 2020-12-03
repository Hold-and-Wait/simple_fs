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
void dir_rm_helper (int inode_to_remove);
//

#define PREFIX "Directory >>"
#define DEF_PATH_SIZE 128
#define DEF_DIR_TABLE_SIZE 10

#define DEBUG_MODE 0 // Set to 1 to enable directory debugging printfs

Bitvector * bitmap;
fs_stack * cwd_stack; /* A stack that contains inode ints */
fdDir * dir_table; /* An array of directories */

char init_key[] = "init=1"; // check key to ensure directories has been initialized before
char meta_key[] = "!&92@&1337!";  // meta key, allows to find LBAs that are meta blocks

int config_location;    // stores the directory configuration location in the LBA
int inode_index;
int dir_count = 0;
int num_table_expansions = 1;
int is_initialized = 0;

/*
 * Initializes the directory.
 * Args:
 *  - An already initialized Bitvector
 *  - int, the location in the LBA that contains configuration
 */
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
	
	is_initialized = 1;
}

/*
 * Checks if the config_location in LBA contains the init key.
 * Return:
 *  1:  has already been initialized before
 *  0:  newly initialized
 */
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

/*
 * Loads the config into memory
 */
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

/*
 * Loads configs from memory into config_loc
 */
int dir_offload_configs() {
	char * config_write_buffer = malloc(513);
	config_write_buffer[512] = '\0';
	
	snprintf(config_write_buffer, 512, "%s\ninode_index=%d\ndir_exp=%d", init_key, inode_index, num_table_expansions);
	LBAwrite(config_write_buffer, 1, config_location);
	
	printf("%s Successfully offloaded config values into volume. inode_index=%d, num_table_expansions=%d\n", PREFIX, inode_index, num_table_expansions);
	free(config_write_buffer);
	return 1;
}

/*
 * Expands the directory entries table by DEF_DIR_TABLE_SIZE
 */
void dir_table_expand() {
	num_table_expansions++;
	dir_table = realloc(dir_table, sizeof(fdDir) * DEF_DIR_TABLE_SIZE * num_table_expansions);
	if (DEBUG_MODE)
	    printf("%s Expanded directory table. New size: %d\n", PREFIX, DEF_DIR_TABLE_SIZE * num_table_expansions);
}

/*
 * Moves a source directory/file into destination directory
 * Args:
 *  - char * src_directory
 *  - char * destination_directory
 * Return:
 *  1:  Successful move
 * -1:  Invalid destination
 * -2:  Invalid source
 */
int dir_move(char * src_directory, char * destination_directory) {
		
	// verify dest
	fs_stack * stack_path_temp;
	int f_slash_counter = 0;

	for (int i = 0; i < strlen(destination_directory); i++) {
		if (destination_directory[i] == '/') {
			if (i == 0 || i == strlen(destination_directory)-1)
				continue;
			f_slash_counter++;
		}
	}
	
	
	if (destination_directory[0] == '/') {
        stack_path_temp = stack_create(DEF_PATH_SIZE);
        stack_push(0, stack_path_temp);
    } else
		stack_path_temp = stack_copy(cwd_stack);
		
	char path_c[strlen(destination_directory)];
	strcpy(path_c, destination_directory);
	fdDir * destination;
	
	char * saveptr;
	char * dir_name = strtok_r(path_c, "/", &saveptr);
	int is_found = 0;
	while (dir_name != NULL) {
	    is_found = 0;
		fdDir * dir_iter = dir_table;
		for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
		    if (dir_iter->is_used != 1)
		        continue;

			struct fs_diriteminfo * meta = fs_readdir(dir_iter);
			if (strcmp(meta->d_name, dir_name) == 0 && dir_iter->parent_inode == stack_peek(stack_path_temp)) {
			    is_found = 1;
                stack_push(dir_iter->inode, stack_path_temp);
			    destination = dir_iter;
			}
		}

		if (is_found == 0) {
            if (DEBUG_MODE)
                printf("%s mv error: dest %s is invalid.\n", PREFIX, dir_name);
		    return -1;
		}

		dir_name = strtok_r(NULL, "/", &saveptr);
	}




    // verify src
    fs_stack * stack_path_temp2;
    f_slash_counter = 0;

    for (int i = 0; i < strlen(src_directory); i++) {
        if (src_directory[i] == '/') {
            if (i == 0 || i == strlen(src_directory)-1)
                continue;
            f_slash_counter++;
        }
    }


    if (src_directory[0] == '/') {
        if (strlen(src_directory) == 1)
            return -1;
        stack_path_temp2 = stack_create(DEF_PATH_SIZE);
        stack_push(0, stack_path_temp2);
    } else
        stack_path_temp2 = stack_copy(cwd_stack);

    char path_d[strlen(src_directory)];
    strcpy(path_d, src_directory);
    fdDir * src;

    char * saveptr2;
    dir_name = strtok_r(path_d, "/", &saveptr2);
    is_found = 0;
    while (dir_name != NULL) {
        is_found = 0;
        fdDir * dir_iter = dir_table;
        for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
            if (dir_iter->is_used != 1)
                continue;

            struct fs_diriteminfo * meta = fs_readdir(dir_iter);
            if (strcmp(meta->d_name, dir_name) == 0 && dir_iter->parent_inode == stack_peek(stack_path_temp2)) {
                stack_push(dir_iter->inode, stack_path_temp2);
                is_found = 1;
                src = dir_iter;
            }
        }

        if (is_found == 0) {
            if (DEBUG_MODE)
                printf("%s mv error: src %s is invalid.\n", PREFIX, dir_name);
            return -2;
        }

        dir_name = strtok_r(NULL, "/", &saveptr2);
    }

    //mv
    char * meta_write_buffer = malloc(513);
    struct fs_diriteminfo * current_meta = fs_readdir(src);

    if (destination_directory[0] == '/' && strlen(destination_directory) == 1) {
        src->parent_inode = 0;
        current_meta->parent_inode = 0;
    } else {
        src->parent_inode = destination->inode;
        current_meta->parent_inode = destination->inode;
    }

    snprintf(meta_write_buffer, 513, "key=%s\nname=%s\ntype=%c\ninode=%d\npinode=%d\nsize=%d\nlbapos=%d\nblen=%d\nmdate=%s",
             meta_key, current_meta->d_name, current_meta->fileType, src->inode, src->parent_inode, current_meta->file_size, (int)src->directoryStartLocation, current_meta->d_reclen, current_meta->st_mod_time );
    LBAwrite(meta_write_buffer, 1, src->directoryStartLocation);
    free(meta_write_buffer);

	return 1;
}

/*
 * Creates a directory entry.
 * Args:
 *  - A pathname
 *  - A mode, unused in current implementation
 * Return:
 *  -1: Uninitialized directory entries or invalid component in path
 */
int fs_mkdir(char *pathname, mode_t mode) {

	if (!is_initialized) {
        if (DEBUG_MODE)
		    printf("%s mkdir: error. (Cause: directory manager is not initialized)\n", PREFIX);
		return -1;
	}
	
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
						stack_push(dir_iter->inode, stack_path_temp);
						is_found = 1;
						break;
					}
				}
			}
			if (!is_found) {
                if (DEBUG_MODE)
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
            char * date_ = malloc(15);
            char * time_ = malloc(15);
            char * date_time = malloc(30);
            getDate(date_);
            getTime(time_);
            snprintf(date_time, 30, "%s %s", date_, time_);

			// write to lba
			snprintf(meta_write_buffer, 513, "key=%s\nname=%s\ntype=%c\ninode=%d\npinode=%d\nsize=%d\nlbapos=%d\nblen=%d\nmdate=%s",
					meta_key, dir_name, 'D', dir_iter->inode, dir_iter->parent_inode, (int)sizeof(fdDir), free_blocks[0], 1, date_time);
			LBAwrite(meta_write_buffer, 1, free_blocks[0]);

			free(date_time);
			free(date_);
			free(time_);
			free(meta_write_buffer);

            if (DEBUG_MODE)
			    printf("%s mkdir: New directory created: %s\n", PREFIX, dir_name);
			return 1;
		}
		dir_name = strtok_r(NULL, "/", &saveptr);
	
	}
    if (DEBUG_MODE)
	    printf("%s mkdir error: undefined.\n", PREFIX);
	return -1;
}

/*
 * Creates a directory entry of type REG.
 * Args:
 *  - A pathname
 *  - Length of blocks it occupies
 * Returns:
 *  -1: Uninitialized directory entries or invalid path component
 */
int fs_mkfile(char *pathname, int block_len) {

	if (!is_initialized) {
        if (DEBUG_MODE)
		    printf("%s mkdir: error. (Cause: directory maanger is not initialized)\n", PREFIX);
		return -1;
	}
	
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
						stack_push(dir_iter->inode, stack_path_temp);
						is_found = 1;
						break;
					}
				}
			}
			if (!is_found) {
                if (DEBUG_MODE)
				    printf("%s mkdir error: invalid path component \'%s\'.\n", PREFIX, dir_name);
				return -1;
			}
			f_slash_counter--;
		} else {	// last component may be invalid -> create dir
		
			char * meta_write_buffer = malloc(513);

			
			// Assign lba pos
			int fb_array[block_len];
			int * free_blocks = get_free_blocks_index(bitmap, fb_array, block_len);
			for (int i = 0; i < block_len; ++i)
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
            char * date_ = malloc(15);
            char * time_ = malloc(15);
            char * date_time = malloc(30);
            getDate(date_);
            getTime(time_);
            snprintf(date_time, 30, "%s %s", date_, time_);

			// write to lba
			snprintf(meta_write_buffer, 513, "key=%s\nname=%s\ntype=%c\ninode=%d\npinode=%d\nsize=%d\nlbapos=%d\nblen=%d\nmdate=%s",
					meta_key, dir_name, 'R', dir_iter->inode, dir_iter->parent_inode, 0, free_blocks[0], block_len, date_time);
			LBAwrite(meta_write_buffer, 1, free_blocks[0]);

            free(date_time);
            free(date_);
            free(time_);
			free(meta_write_buffer);

            if (DEBUG_MODE)
			    printf("%s new file created: %s\n", PREFIX, dir_name);
			return 0;
		}
		dir_name = strtok_r(NULL, "/", &saveptr);
	
	}
    if (DEBUG_MODE)
	    printf("%s mkdir error: undefined.\n", PREFIX);
	return -1;
}

/*
 * Returns  the current working directory
 */
char * fs_getcwd(char *buf, size_t size) {

	if (!is_initialized) {
        if (DEBUG_MODE)
		    printf("%s mkdir: error. (Cause: directory maanger is not initialized)\n", PREFIX);
		return NULL;
	}
	
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
        	}

        	tableptr++;
    	}
    	
    	buf[pos++] = '/';
    	buf[pos++] = '\0'; // null terminator
    	free(read_buf);
    	return buf;
}

/*
 * Removes  a directory.
 * Args:
 *  - A pathname
 * Return:
 *  0: pathname points to a non-directory
 * -1: Invalid path component
 */
int fs_rmdir(char *pathname) {
    if (fs_isDir(pathname) == -1)
        return 0;

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
		if (f_slash_counter >= 0) { // validate path components
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
						stack_push(dir_iter->inode, stack_path_temp);
						is_found = 1;
						if (f_slash_counter == 0) { // remove this component
							//fdDir * dir_stream = fs_opendir(dir_name);
							dir_rm_helper(dir_iter->inode);
						}
						break;
					}
				}
			}
			if (!is_found) {
                if (DEBUG_MODE)
				    printf("%s rmdir error: invalid path component \'%s\'.\n", PREFIX, dir_name);
				return -1;
			}
			f_slash_counter--;
		} 
		dir_name = strtok_r(NULL, "/", &saveptr);
	}
	
	return 0;
}

/*
 * A recursive function that  removes directories within a given directory
 * Args:
 *  - The inode number of the directory to be removed
 */
void dir_rm_helper(int inode_to_remove) {
	char * empty_buf = malloc(513);
	empty_buf[512] = '\0';
	fdDir * dir_ptr = dir_table;
	fdDir * root_ptr;
	
	for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_ptr++) {
		if (dir_ptr->parent_inode == inode_to_remove) {
            if (DEBUG_MODE)
			    printf("Removing %d\n", dir_ptr->inode);
			// overwrite the metadata block of dir
			dir_rm_helper(dir_ptr->inode);
		}
		if (dir_ptr->inode == inode_to_remove)
			root_ptr = dir_ptr;
	}
	
	// notify bitmap
	set_bit(bitmap, root_ptr->directoryStartLocation, 0);
	
	
	root_ptr->is_used = 0;
	LBAwrite(empty_buf, 1, root_ptr->directoryStartLocation);
	
	free(empty_buf);
}

/*
 * Sets the working directory
 * Args:
 *  - A valid path
 * Return:
 *  0: Successful cwd change
 * -1: Uninitialized directory or invalid path component
 */
int fs_setcwd(char *path) {

	if (!is_initialized) {
        if (DEBUG_MODE)
		    printf("%s mkdir: error. (Cause: directory manager is not initialized)\n", PREFIX);
		return -1;
	}
	
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
            if (DEBUG_MODE)
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
    if (DEBUG_MODE)
	    printf("%s setcwd: Successful change directory to %s.\n", PREFIX, buf);
	free(buf);
	return 0;
}

/*
 * Unused
 */
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

/*
 * Reloads directories from LBA into memory
 *  by finding meta_key.
 *  Returns:
 *      - Number of directories loaded
 */
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
			
			while (meta_val != NULL) {
				if (strstr(meta_val, "pinode") != NULL) {
					meta_val = strtok_r(NULL, "=\n", &saveptr);
					dir_ptr->parent_inode = atoi(meta_val);
				}
				else if (strstr(meta_val, "inode") != NULL) {
					meta_val = strtok_r(NULL, "=\n", &saveptr);
					dir_ptr->inode = atoi(meta_val);
				}
				else if (strstr(meta_val, "lbapos") != NULL) {
					meta_val = strtok_r(NULL, "=\n", &saveptr);
					dir_ptr->directoryStartLocation = atoi(meta_val);
				}
				
				meta_val = strtok_r(NULL, "=\n", &saveptr);
			}
			
			struct fs_diriteminfo * dir_info = fs_readdir(dir_ptr);
			
			// notify bitmap
			for (int j = dir_ptr->directoryStartLocation; j < dir_info->d_reclen + dir_ptr->directoryStartLocation; j++) {
				set_bit(bitmap, j, 1);
			}
			
			
			dir_ptr++;
			loaded_dir_counter++;
		}
	}

	free(meta_buf);
	return loaded_dir_counter;
}

/*
 * Reads a directory's metadata
 */
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
			if (strcmp(meta_val, "D") == 0)
				dirent_info->fileType = 'D';
			if (strcmp(meta_val, "R") == 0)
				dirent_info->fileType = 'R';
			if (strcmp(meta_val, "L") == 0)
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
        else if (strstr(meta_val, "mdate") != NULL) {
            meta_val = strtok_r(NULL, "=\n", &saveptr);
            dirent_info->st_mod_time = meta_val;
        }
		meta_val = strtok_r(NULL, "=\n", &saveptr);
	}
	
	free(metadata_read);
	return dirent_info;
}

/*
 * Prints ls
 */
void dir_printShort(char * pathname,  int fllong, int  flall) {

    if (fs_isDir(pathname) == -1)
        return;

    fs_stack * stack_path_temp;
    int f_slash_counter = 0;

    for (int i = 0; i < strlen(pathname); i++) {
        if (pathname[i] == '/') {
            if (i == 0 || i == strlen(pathname)-1)
                continue;
            f_slash_counter++;
        }
    }


    if (pathname[0] == '/') {
        stack_path_temp = stack_create(DEF_PATH_SIZE);
        stack_push(0, stack_path_temp);
    } else
        stack_path_temp = stack_copy(cwd_stack);

    char path_c[strlen(pathname)];
    strcpy(path_c, pathname);

    char * saveptr;
    char * dir_name = strtok_r(path_c, "/", &saveptr);
    fdDir * dir_iter = dir_table;

    // ls in root
    if (pathname[0] == '/' && strlen(pathname) == 1) {
        fdDir * stream = fs_opendir("/");
        for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, stream++) {
            if (stream->is_used != 1 || (stream->parent_inode == 0 && stream->inode == 0))
                continue;

            if (stream->parent_inode == 0) {
                struct fs_diriteminfo * dir_meta = fs_readdir(stream);
                if (fllong)
                    printf("%s  %9u    %s   ", dir_meta->fileType=='D'?"D":"-", dir_meta->file_size, dir_meta->st_mod_time);
                printf("%s", dir_meta->d_name);
                printf("\n");
            }
        }
        return;
    }

    // ls elsewhere
    fdDir * stream = fs_opendir(pathname);
    struct fs_diriteminfo * dir_info;
    while (dir_name != NULL) {
        dir_iter = dir_table;
        int is_found = 0;
        for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
            if (dir_iter->is_used != 1)
                continue;
            dir_info = fs_readdir(dir_iter);
                if (strcmp(dir_info->d_name, dir_name) == 0 && stack_peek(stack_path_temp) == dir_iter->parent_inode) {
                    stack_push(dir_iter->inode, stack_path_temp);
                    is_found = 1;
                    break;
                }
        }
        if (is_found == 0) {
            if (DEBUG_MODE)
                printf("%s ls error: invalid path %s\n", PREFIX, dir_name);
            return;
        }
        dir_name = strtok_r(NULL, "/", &saveptr);
    }

    fdDir * cdir = dir_iter;
    dir_iter = dir_table;
    for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, dir_iter++) {
        if (dir_iter->is_used != 1)
            continue;

        dir_info = fs_readdir(dir_iter);
        if (dir_iter->parent_inode == cdir->inode) {
            struct fs_diriteminfo * dir_meta = fs_readdir(dir_iter);
            if (fllong)
                printf("%s  %9u    %s   ", dir_meta->fileType=='D'?"D":"-", dir_meta->file_size, dir_meta->st_mod_time);
            printf("%s", dir_meta->d_name);
            printf("\n");
        }
    }

}

/*
 * Opens a directory stream.
 * Return:
 *  - An array of directories that contain the children of the directory opened
 */
fdDir * fs_opendir(const char *name) {

    fdDir * dir_item = malloc(sizeof(fdDir));
    fs_stack * opdir_path_track = stack_copy(cwd_stack);

    fdDir * dir_stream = malloc(sizeof(fdDir) *num_table_expansions * DEF_DIR_TABLE_SIZE);

    char name_c[strlen(name)];
    strcpy(name_c, name);

    int inode_of_dir = fs_isDir(name_c);
    if (strlen(name_c) == 0 && name_c[0] == '/')
        inode_of_dir = 0;
    else {
        inode_of_dir = fs_isDir(name_c);
        if (inode_of_dir == -1)
            inode_of_dir = fs_isFile(name_c);
    }



    fdDir * tableptr = dir_table;
    fdDir * init_dir = dir_stream;

    if (inode_of_dir == 0) {
        dir_stream->inode = inode_of_dir;
        fdDir * entry = dir_table;
        for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {
            if (entry->parent_inode  == 0) {
                dir_stream->inode =  entry->inode;
                dir_stream->parent_inode = entry->parent_inode;
                dir_stream->is_used = 1;
                dir_stream->directoryStartLocation = entry->directoryStartLocation;
                dir_stream++;
            }
        }
    } else {

        fdDir * entry = dir_table;
        for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {
            if (entry->inode == inode_of_dir) {
                dir_stream->inode =  entry->inode;
                dir_stream->parent_inode = entry->parent_inode;
                dir_stream->is_used = 1;
                dir_stream->directoryStartLocation = entry->directoryStartLocation;
                dir_stream++;
            }
        }

    }

    return init_dir;
}

int fs_closedir(fdDir *dirp) {

    return -1;
}

/*
 * Checks if path is a valid file
 */
int fs_isFile(char * path) {
    	fs_stack * cwd_temp;
    	int dir_count = 0;

    	for (int i = 0; i < strlen(path); i++) {
        	if (path[i] == '/')
            		dir_count++;
    	}

    	if (path[0] == '/') {
        	dir_count--;
        	cwd_temp = stack_create(128);
    	} else {
        	cwd_temp = stack_copy(cwd_stack);
    	}
    	if (path[strlen(path)] == '/')
        	dir_count--;

	    char * saveptr;
	    char path_c[strlen(path)];
	    strcpy(path_c, path);
    	char * dir_name = strtok_r(path_c, "/", &saveptr);
    	while (dir_name != NULL) {
        	if (dir_count == 0) { // destination
		    	fdDir * entry = dir_table;
		    	for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {
		        	if (entry->parent_inode == stack_peek(cwd_temp)) {
		            		char * buf = malloc(513);
		            		LBAread(buf, 1, entry->directoryStartLocation);

		            		char * saveptr2;
		            		char * read_value = strtok_r(buf, "=\n", &saveptr2);
		            		char * file_name;
		            		char * file_type = malloc(2);
		            		while (read_value != NULL) {
		                		if (strstr("file_name", read_value) != NULL) {
		                    			file_name = malloc(strlen(read_value));
		                    			read_value = strtok_r(NULL, "=\n", &saveptr2);
		                    			file_name = read_value;
		                		}
		                		if (strstr("file_type", read_value) != NULL) {
		                    			read_value = strtok_r(NULL, "=\n", &saveptr2);
		                    			file_type = read_value;
		                		}
		                		read_value = strtok_r(NULL, "=\n", &saveptr2);
		            		}
		            		if (strcmp(file_name, dir_name) == 0 && file_type[0] == 'R') {
		               			return entry->inode;
		            		}
		            		free(buf);
		        	}
            		}
        	} else { // check if dir is correct
            		fdDir * entry = dir_table;
            		int is_found = -1;
            		for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {
                		if (entry->parent_inode == stack_peek(cwd_temp)) {
                    			is_found = 1;
                    			stack_push(entry->inode, cwd_temp);
                    			dir_count--;
                    			break;
                		}
            		}

            	if (is_found == -1)
                	return -1;
        }
        dir_name = strtok_r(NULL, "/", &saveptr);
    }

    return -1;
}

/*
 * Similar to fs_isFile
 */
int fs_isDir(char * path) {
    	fs_stack * cwd_temp;
    	int dir_count = 0;

    	for (int i = 0; i < strlen(path); i++) {
        	if (path[i] == '/')
            	dir_count++;
    	}

    	if (path[0] == '/') {
    	    if (strlen(path) == 1)
    	        return 0;
        	dir_count--;
        	cwd_temp = stack_create(128);
        	stack_push(0, cwd_temp);
	    } else {
        	cwd_temp = stack_copy(cwd_stack);
    	}
    	if (path[strlen(path)] == '/')
        	dir_count--;


    	char temp_path[strlen(path)];
    	strcpy(temp_path, path);

    	int is_found = -1;
    	char * saveptr;
    	char * dir_name = strtok_r(temp_path, "/", &saveptr);
    	
    	while (dir_name != NULL) {
        	is_found = -1;

        	fdDir * entry = dir_table;

        	for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {

            		if (entry->is_used == 0)
                		continue;

            		struct fs_diriteminfo * direnfo = fs_readdir(entry);

            		if (direnfo == NULL)
                		continue;

                if (strcmp(direnfo->d_name, dir_name) == 0 && entry->parent_inode == stack_peek(cwd_temp) && direnfo->fileType == 'D') {
                		is_found = entry->inode;
                		stack_push(entry->inode, cwd_temp);

                        break;
            		} else {
            		}

        	}

		if (is_found == -1)
            		break;

        	dir_name = strtok_r(NULL, "/", &saveptr);
    	}

	return is_found;
}

/*
 * Removes a directory of type REG
 */
int fs_delete(char * filename) {
    int inode_of_file = fs_isFile(filename); // -1 if invalid, inode # if valid
    if (inode_of_file == -1)
        return 0;

    fdDir * entry = dir_table;

    for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {
        if (entry->is_used == 0)
            continue;

        if (entry->inode == inode_of_file) {

            struct fs_diriteminfo * dir_info = fs_readdir(entry);

            // notify bitmap to free up file
            for (int j = entry->directoryStartLocation; j < dir_info->d_reclen; j++)
                set_bit(bitmap, j, 0);

            // remove file's metadata block
            char * empty_buf = malloc(512);
            LBAwrite(empty_buf, 1, entry->directoryStartLocation);
            free(empty_buf);

            // remove from table
            entry->is_used = 0;


            return 1;
        }
    }

    return 0;
}

/*
 * Modifies the meta data of directory
 * Args:
 *  - dir: the directory to be modified
 *  - updated meta: The new meta of the directory
 */
void dir_modify_meta(fdDir * dir, struct fs_diriteminfo * updated_meta) {
    char * meta_write_buffer = malloc(513);
    struct fs_diriteminfo * current_meta = fs_readdir(dir);

    // update lba location, if needed
    fdDir * tableptr = dir_table;
    for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, tableptr++) {
        if (tableptr->inode == dir->inode) {
            tableptr->directoryStartLocation = dir->directoryStartLocation;
        }
    }

    char * date_ = malloc(15);
    char * time_ = malloc(15);
    char * date_time = malloc(30);
    getDate(date_);
    getTime(time_);
    snprintf(date_time, 30, "%s %s", date_, time_);

    current_meta->file_size += updated_meta->file_size;
    snprintf(meta_write_buffer, 513, "key=%s\nname=%s\ntype=%c\ninode=%d\npinode=%d\nsize=%d\nlbapos=%d\nblen=%d\nmdate=%s",
             meta_key, current_meta->d_name, 'R', dir->inode, dir->parent_inode, current_meta->file_size, (int)dir->directoryStartLocation, updated_meta->d_reclen, date_time );
    LBAwrite(meta_write_buffer, 1, dir->directoryStartLocation);

    free(date_time);
    free(date_);
    free(time_);
    free(meta_write_buffer);
}

int fs_stat(const char *path, struct fs_stat * buf) {
	char path_c[strlen(path)];
	strcpy(path_c, path);
	
	fs_stack * cwd_temp;
    	int dir_count = 0;

    	for (int i = 0; i < strlen(path_c); i++) {
        	if (path[i] == '/')
            		dir_count++;
    	}

    	if (path_c[0] == '/') {
        	dir_count--;
        	cwd_temp = stack_create(128);
    	} else {
        	cwd_temp = stack_copy(cwd_stack);
    	}
    	if (path_c[strlen(path_c)] == '/')
        	dir_count--;

	char * saveptr;
	int is_found;
	
	struct fs_diriteminfo * dir_info;
	fdDir * entry = dir_table;
	
    	char * dir_name = strtok_r(path_c, "/", &saveptr);
    	while (dir_name != NULL) {
    		is_found = 0;
    		entry = dir_table;
    		
        	for (int i = 0; i < num_table_expansions * DEF_DIR_TABLE_SIZE; i++, entry++) {
        		dir_info = fs_readdir(entry);
        		if (strcmp(dir_name, dir_info->d_name) == 0 && entry->parent_inode == stack_peek(cwd_temp)) {
        			stack_push(entry->parent_inode, cwd_temp);
        			is_found = 1;
        			break;
        		}
        	}
        	if (is_found == 0) {
        		printf("%s fs_stat error: invalid path: \'%s\'.\n", PREFIX, dir_name);
        		return 0;
        	}
        	dir_name = strtok_r(NULL, "/", &saveptr);
    	}
    	
    	buf->st_size = dir_info->file_size;
    	buf->st_blksize = 512;
    	buf->st_blocks = dir_info->d_reclen;
    	
    	return 1;
}

/*
 * Prints all the directories
 */
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
        if (entry->is_used == 1) {

            if (entry->directoryStartLocation <= config_location || entry->directoryStartLocation > 9999999 || entry->inode < 0 || entry->parent_inode < 0  || entry->inode > 999999 || entry->parent_inode > 999999)
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





















