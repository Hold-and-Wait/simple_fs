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

#define DIRECTORY_ENTRY_SIZE 10
#define MAX_PATH 128
#define DEFAULT_DIR_SIZE 1
#define DEFAULT_FILE_SIZE 20
#define LBA_SIZE 10

// Functions that do not need to be in header file
void load_configs(char * buf_file_p1);
fdDir * get_free_dir();
void load_directory();
int rm_helper(fdDir * dir_stream);

// printing/debug
char prefix[] = "\nDirectory Entry >> ";

int LBA;

struct stack_util cwd_stack;    // a stack that tracks which directory we are in
struct fs_diriteminfo * dir_info;
struct fs_stat * file_stat;
fdDir * fd_table;

// supports
char * cwd_buf; // just a buffer for cwd prints
char * buf_file; // This is used to store into memory
char init_key[] = "is_init=1\0";
char meta_key[] = "00pl1fmcnfjdmxnajfncjfislf998xmmzaaaldpw93nxnpfp001337\0"; // just a random key to identify metadata blocks
Bitvector * vector;

// Some information from disk that should be loaded into memory if the file system was already initialized previously
int current_expansions = 1; // Keeps track of how many times fd_table has been expanded
int inode_index;    // increments every time make_dir is called. Guarantees each dir entry has a unique inode number
int dir_used_size; // tracks # of dir entries used
int vector_size;

/*
 * LBA[7] contains config data
 * LBA[8] - LBA[17] will contain directory structure
 */

void initializeDirectory(Bitvector * vec, int LBA_Pos) {
    LBA = LBA_Pos;
    vector = vec;
    vector_size = get_vector_size(vector);
    char * buf_file_p1 = malloc(513); // config page
    cwd_stack = create_stack(MAX_PATH);
    int is_init = dirent_check_is_init();
    if (is_init) { // load into memory
        load_configs(buf_file_p1);
    } else { // set defaults
        printf("%s First initialization. Setting default values.", prefix);
        inode_index = 1;
        dir_used_size = 0;
        buf_file_p1 = "is_init=1\ninode_index=1\ndir_used_size=0\ncurrent_expansions=1\0";
        LBAwrite(buf_file_p1, 1, 7);
    }

    stack_push(0, &cwd_stack); // Sets the root directory
    fd_table = malloc(sizeof(fdDir) * current_expansions * DIRECTORY_ENTRY_SIZE);
    dir_info = malloc(sizeof(struct fs_diriteminfo) * current_expansions * DIRECTORY_ENTRY_SIZE);
    load_directory();
}

/*
 * Checks if LBA contains initialized keyword
 * Returns:
 *  0   :   first load
 *  1   :   already initialized
 */
int dirent_check_is_init() {
    char * read_block = malloc(513);
    LBAread(read_block, 1, LBA);
    char keyword[10];
    keyword[9] = '\0';
    memcpy(keyword,read_block,9);
    if (strcmp(keyword, init_key) == 0)
        return 1;
    return 0;
}

/*
 * Loads config settings into memory
 */
void load_configs(char * buf_file_p1) {
    printf("%s Loading config values (LBA[%d]).", prefix, LBA);
    char * saveptr;
    LBAread(buf_file_p1, 1, LBA);
    char * config_values = strtok_r(buf_file_p1, "\n", &saveptr);
    while (config_values != NULL) {
        char num_val[10];
        int j = 0;
        for (int i = 0; i < strlen(config_values); i++) { // Parses values to integers
            if (atoi(&config_values[i]) > 0 || config_values[i] == '0') {
                num_val[j] = config_values[i];
                j++;
                num_val[j] = '\0';
            }
        }
        if (strstr(config_values, "inode_index") != NULL)
            inode_index = atoi(num_val);
        else if (strstr(config_values, "dir_used_size") != NULL)
            dir_used_size = atoi(num_val);
        else if (strstr(config_values, "current_expansions") != NULL)
            current_expansions = atoi(num_val);

        config_values = strtok_r(NULL, "\n", &saveptr);
    }
    printf("%s Values loaded into memory: inode_index=%d, dir_used_size=%d, current_expansions=%d", prefix, inode_index, dir_used_size, current_expansions);
}

/*
 * Offloads config data into LBA
 */
void offload_configs() {
    char * buf_file_p1 = malloc(513); // config page
    snprintf(buf_file_p1, 512, "is_init=%d\ninode_index=%d\ndir_used_size=%d\ncurrent_expansions=%d", 1, inode_index, dir_used_size, current_expansions);
    buf_file_p1[512] = '\0';
    LBAwrite(buf_file_p1, 1, 7);
    printf("%s Config values offloaded: inode_index=%d, dir_used_size=%d, current_expansions=%d", prefix, inode_index, dir_used_size, current_expansions);
}

int get_free_inode() {
    int index_found = -1;
    for (int i = 1; i <= inode_index; i++) {
        fdDir * entry = fd_table;
        for (int j = 0; j < current_expansions * DIRECTORY_ENTRY_SIZE; j++, entry++) {
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
    return  index_found;
}

/*
 *  Create a directory entry.
 *  Return:
 *      >=0 : LBA position of the newly created directory
 *      -1  : directory not made
 */
int fs_mkdir(char *pathname, mode_t mode, int file_type) {
    struct fs_diriteminfo * dirent = malloc(sizeof(struct fs_diriteminfo));
    struct stack_util path_tracker = create_stack(10);

    int dir_count = 0; // keeps track of '/'
    for (int i = 0; i < strlen(pathname); i++) {
        if (pathname[i] == '/')
            dir_count++;
    }

    // Check for absolute path
    if (pathname[0] == '/') {
        stack_push(0, &path_tracker);
        dir_count--;
    } else {
        stack_copy(&path_tracker, &cwd_stack);
    }
    // Check if last is a /
    if (pathname[strlen(pathname)-1] == '/')
        dir_count--;

    // Iterate path
    char * saveptr;
    char * temp;
    //strcpy(temp, pathname);

    char * dir_name = strtok_r(pathname, "/", &saveptr);

    while (dir_name != NULL) {

        // dir_count > 0 indicates we are not at destination and must verify valid directory (dir1/->dir2<-/target)
        if (dir_count > 0) {
            fdDir * entry = fd_table;
            char * buf = malloc(512);
            char ent_name[strlen(dir_name)];
            for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
                if (entry->is_used) {
                    LBAread(buf, 1, entry->directoryStartLocation);
                    char * saveptr2;
                    char * kv = strtok_r(buf, "=\n", &saveptr);
                    while (kv != NULL) {
                        if (strstr(kv, dir_name) != NULL && entry->parent_inode == stack_peek(&path_tracker)) {
                            kv = strtok_r(NULL, "=\n", &saveptr2);
                            if (strcmp(kv, dir_name) == 0) {
                                stack_push(entry->inode, &path_tracker);
                                break;
                            }
                        }
                        kv = strtok_r(NULL, "=\n", &saveptr2);
                    }
                }
            }
            free(buf);
            dir_count--;
        }

        // dir_count == 0 indicates we are at last part of path (dir1/dir2/->target<-)
        if (dir_count == 0) {

            unsigned char f_type;
            // Assign slot in LBA
            int num_blocks = 0;
            if (file_type == DT_DIR) {
                num_blocks = DEFAULT_DIR_SIZE;
                f_type = 'D';
            }
            else if (file_type == DT_REG) {
                num_blocks = DEFAULT_FILE_SIZE;
                f_type = 'R';
            }
            int fb_array[num_blocks];
            int * free_blocks = get_free_blocks_index(vector, fb_array, num_blocks);
            for (int i = 0; i < num_blocks; ++i)
                set_bit(vector, free_blocks[i], 1);

            // Assign data to dir
            fdDir * newDir = get_free_dir();
            newDir->inode = inode_index;
            newDir->parent_inode = stack_peek(&cwd_stack);
            newDir->directoryStartLocation = free_blocks[0];

            // Assign metadata
            dirent->fileType = f_type;
            dirent->file_size = 0;
            dirent->d_reclen = num_blocks;
            strcpy(dirent->d_name, dir_name);
            // time
            char * date_time = malloc(25);
            char * date = malloc(10);
            char * time = malloc(10);
            getDate(date);
            getTime(time);
            snprintf(date_time, 25, "%s - %s", date, time);
            dirent->st_mod_time = date_time;
            dirent->st_create_time = date_time;

            // check uniqueness of file name
            fdDir * tableptr2 = fd_table;
            char * d_name_cpy = malloc(256);
            for (int j = 0; j < current_expansions * DIRECTORY_ENTRY_SIZE; j++, tableptr2++) {
                if (tableptr2->parent_inode == stack_peek(&path_tracker)) { // same directory folder



                }
            }


            // Prepare to write metadata to dir's first LBA
            char * buf_p1 = malloc(513);
            buf_p1[512] = '\0';
            int stat = snprintf(buf_p1, 512, "%s\nfile_name=%s\nfile_type=%c\nfile_size=%d\nblocks_occ=%d\ninode=%d\np_inode=%d\ndate_created=%s\n"
                                             "date_mod=%s\n",
                                meta_key, dirent->d_name, dirent->fileType, dirent->file_size, dirent->d_reclen, newDir->inode, newDir->parent_inode,
                                date_time, date_time);

            // Write to LBA
            LBAwrite(buf_p1, 1, newDir->directoryStartLocation);
            printf("%s mkdir: Metadata for %s(%d) written to LBA %llu", prefix, dir_name, inode_index, newDir->directoryStartLocation);
            free(dirent);
            inode_index++;
            dir_used_size++;

            char * buf2 = malloc(512);
            LBAread(buf2, 1, newDir->directoryStartLocation);
            //printf("\nCONT:\n%s\n", buf2);
            break;
        }
        dir_name = strtok_r(NULL, "/", &saveptr);
    }

    return -1;
}

struct stack_util path_tracker;
int is_use = 0;

int fs_rmdir(char *pathname) {

    if (!fs_isDir(pathname))
        return -1;

    if (!is_use) {
        stack_copy(&path_tracker, &cwd_stack);
        is_use = 1;
    }

    char name_c[strlen(pathname)];
    strcpy(name_c, pathname);
    char * saveptr;

    char * dir_name = strtok_r(name_c, "/", &saveptr);

    fdDir * entry = fd_table;
    int entry_found = 0;

    while (dir_name != NULL) {
        for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
            if (entry->is_used) {
                if (strcmp(fs_readdir(entry)->d_name, dir_name) == 0 && entry->parent_inode == stack_peek(&path_tracker)) {
                    stack_push(entry->inode, &path_tracker);
                    entry_found = 1;
                    break;
                }
            }
        }

        if (!entry_found) {
            return -1;
        }
        dir_name = strtok_r(NULL, "/", &saveptr);
    }



    if (entry_found) {
        rm_helper(entry);
        entry->is_used = 0;

        struct fs_diriteminfo * dir_info = fs_readdir(entry);

        // free up the bit
        int bit_start = entry->directoryStartLocation;
        int bit_end = bit_start + dir_info->d_reclen;

        for (int i = bit_start; i < bit_end; i++) {
            set_bit(vector, i, 0);
        }

        // Free up the LBA metadata
        char * empty_buf = malloc(512);
        LBAwrite(empty_buf, 1, entry->directoryStartLocation);
        free(empty_buf);
    }
    return 0;
}

int rm_helper(fdDir * dir_stream) {

    if (dir_stream == NULL)
        return -1;

    fdDir * entry = fd_table;

    for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
        //printf("%d %d\n", dir_stream->inode, entry->parent_inode);
        if (entry->is_used && entry->parent_inode == dir_stream->inode) {
            printf("Removing %d\n", entry->inode);
            rm_helper(entry);

            struct fs_diriteminfo * dir_info = fs_readdir(entry);

            // free up the bit
            int bit_start = entry->directoryStartLocation;
            int bit_end = bit_start + dir_info->d_reclen;

            for (int j = bit_start; j < bit_end; j++) {
                set_bit(vector, j, 0);
            }

            entry->is_used = 0;
            char * empty_buf = malloc(512);
            LBAwrite(empty_buf, 1, entry->directoryStartLocation);
            free(empty_buf);
        }
    }

    return 0;
}

int fs_setcwd(char *path) {

    if (path[0] == '/')
        while (stack_size(&cwd_stack) > 1)
            stack_pop(&cwd_stack);

    //
    int stack_push_count = 0;

    char * saveptr;
    char * test = malloc(strlen(path));
    strcpy(test, path);
    char * dir_name = strtok_r(test, "/", &saveptr);

    while (dir_name != NULL) {

        if (strcmp(dir_name, "..") == 0) {
            stack_pop(&cwd_stack);
            char * buf = malloc(MAX_PATH);
            fs_getcwd(buf, MAX_PATH);
            free(buf);
            dir_name = strtok_r(NULL, "/", &saveptr);
            continue;
        }


        int is_found = 0;
        fdDir * entry = fd_table;
        for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {

            if (!entry->is_used || entry->parent_inode != stack_peek(&cwd_stack))
                continue;

            // check valid
            struct fs_diriteminfo * dirinfo = fs_readdir(entry);

            if (strcmp(dir_name, dirinfo->d_name) == 0 && fs_isDir(dir_name) == 1) {
                stack_push(entry->inode, &cwd_stack);
                stack_push_count++;
                is_found = 1;
            }
        }

        if (is_found == 0) {
            printf("%s setcwd: Error. \'%s\' was not found. (Cause: It is not a valid directory type, or it is not in path.)", prefix, dir_name);

            // undo directory pushes to cwd_stacks
            while (stack_push_count > 0 && stack_size(&cwd_stack) > 0)
                stack_pop(&cwd_stack);
            return 0;
        }
        dir_name = strtok_r(NULL, "/", &saveptr);
    }

    char * buf = malloc(MAX_PATH);

    fs_getcwd(buf, MAX_PATH);
    printf("%s setcwd: Successful change directory to %s.", prefix, buf);
    free(buf);
    return 1;
}

char * fs_getcwd(char *buf, size_t size) {
    struct stack_util temp;
    stack_copy(&temp, &cwd_stack);
    struct stack_util reverse_cwd = create_stack(MAX_PATH);
    while (stack_size(&temp) > 0)
        stack_push(stack_pop(&temp), &reverse_cwd);


    int pos = 0;
    //buf[pos] = '/';
    //pos++;
    char * read_buf = malloc(513);

    while (stack_size(&reverse_cwd) > 0) {
        fdDir * tableptr = fd_table;
        int node = stack_pop(&reverse_cwd);
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

/*
 * Check if a directory is valid.
 * The file given must be a child of the pop value of dir_stack.
 *  i.e., dir's parent must be pop value of dir_stack
 * Return:
 *      1   :   Valid
 *      0   :   Invalid
 */
int is_valid_dir(char * path, char * new_dir_name) {
    struct stack_util path_tracker;

    if (path[0] != '/')
        stack_copy(&path_tracker, &cwd_stack);
    else
        path_tracker = create_stack(stack_size(&cwd_stack));

    char temp_path[strlen(path)];
    strcpy(temp_path, path);
    char * saveptr;
    char * dir_name = strtok_r(temp_path, "/", &saveptr);
    char * dir;

    while (dir_name != NULL) {
        dir = dir_name;
        int found_directory = 0;
        fdDir * entry_item = fd_table;
        for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry_item++) {
            struct fs_diriteminfo * dirent = fs_readdir(entry_item);
            if (strcmp(dir_name, dirent->d_name) == 0) {
                found_directory  = 1;
                break;
            }
        }
        memcpy(new_dir_name, dir_name, strlen(dir_name)+1);

        dir_name = strtok_r(NULL, "/", &saveptr);
        if (dir_name != NULL && !found_directory)
            return 0;
    }
    return 1;
}

int recurse = 0;

/*
 * Opens dir stream of the dir item that matches name
 */
fdDir * fs_opendir(const char *name) {
    fdDir * dir_item = malloc(sizeof(fdDir));
    fdDir * initial_pos_ptr;
    struct stack_util opdir_path_track;
    stack_copy(&opdir_path_track, &cwd_stack);

    fdDir * dir_stream = malloc(sizeof(fdDir) * current_expansions * DIRECTORY_ENTRY_SIZE);

    char * name_c = strdup(name);

    if (!fs_isDir(name_c)) // cannot open a non-directory
        return NULL;

    fdDir * tableptr = fd_table;
    initial_pos_ptr = dir_stream;
    int inode_val;
    int is_found = 0;

    for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, tableptr++) {
        struct fs_diriteminfo * dirent = fs_readdir(tableptr);
        if (strcmp(dirent->d_name, name_c) == 0) {
            // add to stack to keep track of node that is removed
            if (stack_peek(&opdir_path_track) == tableptr->parent_inode) {
                inode_val = tableptr->inode;
                is_found = 1;
                break;
            }
        }
    }

    if (!is_found) {
        printf("%s opendir: Error!", prefix);
        return NULL;
    }

    tableptr = fd_table;
    for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, tableptr++) {
        if (tableptr->parent_inode == inode_val) {
            memcpy(dir_stream, tableptr, sizeof(fdDir));
            dir_stream++;
        }
    }

    return initial_pos_ptr;
}
/**
fdDir * fs_opendir(const char *name) {

    fdDir * entry = fd_table;
    char * buf = malloc(513);
    for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
        if (entry->is_used) {
            LBAread(buf, 1, entry->directoryStartLocation);
            char * file_name;

            char * saveptr;
            char * dir = strtok_r(buf, "=\n", &saveptr);

            while (dir != NULL) {

                if (strstr(dir, "file_name") != NULL) {
                    dir = strtok_r(NULL, "=\n", &saveptr);
                    file_name = malloc(strlen(dir));
                    strcpy(file_name, dir);
                    if ((strcmp(file_name, name) == 0 && entry->parent_inode == stack_peek(&cwd_stack)) || (strcmp(file_name, name) == 0 && fs_isDir(file_name))) {
                        free(file_name);
                        free(buf);
                        //printf("%s opendir: Successfully opened directory stream \'%s\'.", prefix, name);
                        return entry;
                    }

                    free(file_name);
                }
                dir = strtok_r(NULL, "=\n", &saveptr);
            }

            // get name of entry

        }
    }
    cwd_buf = malloc(MAX_PATH);
    fs_getcwd(cwd_buf, MAX_PATH);
    printf("%s opendir: Failed to open directory stream \'%s\'. (Cause: may not exist or is not in cwd \'%s\')", prefix, name, cwd_buf);
    free(cwd_buf);
    free(buf);
    return NULL;
}
*/
struct fs_diriteminfo * fs_readdir(fdDir *dirp) {
    if (dirp == NULL)
        return NULL;
    char * buf = malloc(513);
    struct fs_diriteminfo * read_dir = malloc(sizeof(struct fs_diriteminfo));

    LBAread(buf, 1, dirp->directoryStartLocation);

    char * saveptr;
    char * read_val = strtok_r(buf, "=\n", &saveptr);

    while (read_val != NULL) {
        if (strstr(read_val, "file_name") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            strcpy(read_dir->d_name, read_val);
        }
        else if (strstr(read_val, "p_inode") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->parent_inode = atoi(read_val);
        }
        else if (strstr(read_val, "inode") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->inode = atoi(read_val);
        }
        else if (strstr(read_val, "blocks_occ") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->d_reclen = atoi(read_val);
        }
        else if (strstr(read_val, "file_size") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->file_size = atoi(read_val);
        }
        else if (strstr(read_val, "file_type") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->fileType = *read_val;
        }
        else if (strstr(read_val, "date_created") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->st_create_time = read_val;
        }
        else if (strstr(read_val, "date_mod") != NULL) {
            read_val = strtok_r(NULL, "=\n", &saveptr);
            read_dir->st_mod_time = read_val;
        }
        read_val = strtok_r(NULL, "=\n", &saveptr);
    }

    //printf("%s readdir: Reading directory \'%s\'.", prefix, read_dir->d_name);
    free(buf);
    return read_dir;
}

int fs_closedir(fdDir *dirp) {
    if (dirp != NULL) {
        printf("%s closedir: Directory closed...?", prefix);
        return 1;
    }
    return 0;
}

fdDir * get_free_dir(){

    fdDir * dirptr = fd_table;
    int slot_found = -1;
    for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++) {

        if (dirptr->is_used == 0) {
            dirptr->is_used = 1;
            return dirptr;
        }
        dirptr++;
    }

    //  expand
    current_expansions++;
    fd_table = realloc(fd_table, sizeof(fdDir) * current_expansions * DIRECTORY_ENTRY_SIZE);
    dir_info = realloc(dir_info, sizeof(struct fs_diriteminfo) * current_expansions * DIRECTORY_ENTRY_SIZE);
    return get_free_dir();
}

void load_directory() {
    printf("%s Loading directories from volume...", prefix);
    int dir_load_counter = 0;
    char * buf = malloc(513);
    for (int i = 0; i < vector_size; i++) {
        LBAread(buf, 1, i);
        if (strstr(buf, meta_key) != NULL) {

            fdDir * loadDir = get_free_dir();

            int block_len = 0;
            char * saveptr;
            char * kv_pair = strtok_r(buf, "=\n", &saveptr);
            while (kv_pair != NULL) {
                if (strstr(kv_pair, "blocks_occ") != NULL){
                    kv_pair = strtok_r(NULL, "=\n", &saveptr);
                    block_len = atoi(kv_pair);
                }
                else if (strstr(kv_pair, "p_inode") != NULL) {
                    kv_pair = strtok_r(NULL, "=\n", &saveptr);
                    loadDir->parent_inode = atoi(kv_pair);
                } else if (strstr(kv_pair, "inode") != NULL) {
                    kv_pair = strtok_r(NULL, "=\n", &saveptr);
                    loadDir->inode = atoi(kv_pair);
                }
                kv_pair = strtok_r(NULL, "=\n", &saveptr);
            }

            loadDir->directoryStartLocation = i;
            loadDir->is_used = 1;
            loadDir->is_used = 1;
            loadDir->dirEntryPosition = 512;

            // set proper bits -> delete this portion when bitmap saving is implemented
            /*
            int blocks_to_alloc[block_len];
            int * blocks = get_free_blocks_index(vector, blocks_to_alloc, block_len);
            for (int i = 0; i < block_len; i++) {
                set_bit(vector, blocks[i], 1);
            }*/

            // free up the bit
            int bit_start = i;
            int bit_end = bit_start + block_len;

            for (int j = bit_start; j < bit_end; j++) {
                set_bit(vector, j, 1);
            }
            // end

            dir_load_counter++;

        }
    }
    free(buf);
    if (dir_load_counter == 0)
        printf("%s No directory entries to load.", prefix);
    else
        printf("%s Successfully loaded %d directory entries from disk.", prefix, dir_load_counter);
}

/*
 * Determines whether a path/name is a file.
 * A file is a textfile.
 * Returns:
 *      -1  :   Not a file or invalid name
 *      >=0 :   The LBA (position + 1) of the file
 */
int fs_isFile(char * path) {
    struct stack_util cwd_temp;
    int dir_count = 0;

    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '/')
            dir_count++;
    }

    if (path[0] == '/') {
        dir_count--;
        cwd_temp = create_stack(MAX_PATH);
    } else {
        stack_copy(&cwd_temp, &cwd_stack);
    }
    if (path[strlen(path)] == '/')
        dir_count--;

    char * saveptr;
    char * dir_name = strtok_r(path, "/", &saveptr);
    while (dir_name != NULL) {
        if (dir_count == 0) { // destination
            fdDir * entry = fd_table;
            for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
                if (entry->parent_inode == stack_peek(&cwd_temp)) {
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
                        return entry->directoryStartLocation+1;
                    }
                    free(buf);
                }
            }
        } else { // check if dir is correct
            fdDir * entry = fd_table;
            int is_found = -1;
            for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
                if (entry->parent_inode == stack_peek(&cwd_temp)) {
                    is_found = 1;
                    stack_push(entry->inode, &cwd_temp);
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

int fs_isDir(char * path) {
    struct stack_util cwd_temp;
    int dir_count = 0;

    for (int i = 0; i < strlen(path); i++) {
        if (path[i] == '/')
            dir_count++;
    }

    if (path[0] == '/') {
        dir_count--;
        cwd_temp = create_stack(MAX_PATH);
    } else {
        stack_copy(&cwd_temp, &cwd_stack);
    }
    if (path[strlen(path)] == '/')
        dir_count--;
    int is_found = -1;
    char * saveptr;
    char * dir_name = strtok_r(path, "/", &saveptr);
    while (dir_name != NULL) {
        is_found = -1;

        fdDir * entry = fd_table;

        for (int i = 0; i < DIRECTORY_ENTRY_SIZE * current_expansions; i++, entry++) {

            if (entry->is_used == 0)
                continue;

            struct fs_diriteminfo * direnfo = fs_readdir(entry);

            if (direnfo == NULL)
                continue;

            if (strcmp(direnfo->d_name, dir_name) == 0 && direnfo->fileType == 'D') {
                is_found = 1;
                stack_push(entry->inode, &cwd_temp);
                break;
            }

        }

        if (is_found == -1)
            break;

        dir_name = strtok_r(NULL, "/", &saveptr);
    }

    return is_found;
}

int fs_delete(char* filename) {

    return 0;
}

void print_dir() {
    char * buf = malloc(513);
    printf("%s Printing Directory Entries Table:\n", prefix);

    // PRINT TABLE HEADERS
    printf(" ");
    for (int i = 0; i <= 90; i++) {
        if (i == 0) {
            printf("\u250f");
            continue;
        }
        if (i == 90) {
            printf("\u2513");
            continue;
        }
        if (i == 20 || i == 35 || i == 50 || i == 65 || i == 80) {
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
    printf("  type   \u2503\n ");
    for (int i = 0; i <= 90; i++) {
        if (i == 0) {
            printf("\u2523");
            continue;
        }
        if (i == 90) {
            printf("\u252b");
            continue;
        }
        if (i == 20 || i == 35 || i == 50 || i == 65 || i == 80) {
            printf("\u2547");
            continue;
        }
        printf("\u2501");
    }
    printf("\n ");

    // END HEADER

    // DATA ENTRY
    fdDir * entry = fd_table;
    for (int i = 0; i < current_expansions * DIRECTORY_ENTRY_SIZE; i++, entry++) {
        if (entry->is_used) {

            if (entry->directoryStartLocation <= LBA || entry->inode < 0 || entry->parent_inode < 0  || entry->inode > 999999 || entry->parent_inode > 999999)
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

                if (strstr(f_data, "file_name") != NULL) {
                    f_data = strtok_r(NULL, "=\n", &saveptr);
                    strcpy(file_name, f_data);
                }if (strstr(f_data, "file_type") != NULL) {
                    f_data = strtok_r(NULL, "=\n", &saveptr);
                    strcpy(file_type, f_data);
                }
                if (strstr(f_data, "blocks_occ") != NULL) {
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

            printf("\u2503%18.*s \u2502   %08d   \u2502   %08d   \u2502   %08llu   \u2502   %08d   \u2502   %s   \u2503\n ",
                   18,file_name, entry->inode, entry->parent_inode, entry->directoryStartLocation, block_size, file_type_long);

            free(file_name_trunc);
            free(buf);
        }
    }
    // END DATA
    // LAST ROW
    for (int i = 0; i <= 90; i++) {
        if (i == 0) {
            printf("\u2517");
            continue;
        }
        if (i == 90) {
            printf("\u251b");
            continue;
        }
        if (i == 20 || i == 35 || i == 50 || i == 65 || i == 80) {
            printf("\u2537");
            continue;
        }

        printf("\u2501");
    }
    // END TABLE

    free(buf);
}

int fs_stat(const char *path, struct fs_stat *buf) {
    char * temp_path = strcpy(temp_path, path);

    return 0;
}
