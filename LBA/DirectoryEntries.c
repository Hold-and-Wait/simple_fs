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

#include "LBA/lba.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/stack.h"

#define DIRECTORY_ENTRY_SIZE 100
#define MAX_PATH 128
#define DEFAULT_DIR_SIZE 1
#define DEFAULT_FILE_SIZE 20

// Functions that do not need to be in header file
int get_free_directory();
int get_inode(const char * path, int is_absolute);

struct stack_util current_directory_node;
struct stack_util current_directory_node_cpy;
struct stack_util current_directory_node_cpy2;
struct fs_diriteminfo * dir_info;
struct fs_stat * file_stat;
fdDir * fd_table;

// Index tracking and expansion
int inode_index;
int dir_used_size;

// supports
char * cwd_buf;
Bitvector * vector;

fdDir get_entry(int inode);

void initializeDirectory(Bitvector * vec) {
    vector = vec;
    fd_table = malloc(sizeof(fdDir) * DIRECTORY_ENTRY_SIZE);
    printf("STAT: %p\n", fd_table);
    current_directory_node = create_stack(MAX_PATH);
    current_directory_node_cpy = create_stack(MAX_PATH);
    current_directory_node_cpy2 = create_stack(MAX_PATH);
    file_stat = malloc(sizeof(struct fs_stat));
    inode_index = 1;
    dir_used_size = 0;

    cwd_buf = malloc(256);


    // Create root
    stack_push(0, &current_directory_node);

    //fs_mkdir("/", 0);


    printf("DIRECTORY ENTRY STRUCTURES: fdDir (%d), path_stack (%d) INITIALIZED.\n", DIRECTORY_ENTRY_SIZE, MAX_PATH);
}

// ignore mode for now
int fs_mkdir(const char *pathname, mode_t mode, int file_type) {

    int dir_change_count = 0;
    for (int i = 0; i < strlen(pathname); i++) {
        if (pathname[i] == '/')
            dir_change_count++;
    }

    int is_absolute = 0;
    int lba_pos = -1; // return value
    char * cwd_temp_buf = malloc(512);
    struct stack_util temp_stack = create_stack(MAX_PATH);
    fs_getcwd(cwd_temp_buf, 512);

    if (pathname[0] == '/') {
        is_absolute = 1;
        dir_change_count--;
    } else {
        stack_push(stack_peek(&current_directory_node), &temp_stack);
    }

    char temp_path[strlen(pathname)];
    strcpy(temp_path, pathname);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);
    int parent_node = 0;
    while (file_path != NULL) {
        int file_exists = 0;
        if (strcmp(file_path, "..") == 0) {
            stack_pop(&temp_stack);
            file_path = strtok_r(NULL, "/", &saveptr);
            continue;
        }

        fdDir temp;
        // Check if it exists
        for (int i = 0; i < dir_used_size; i++) {
            temp = fd_table[i * sizeof(fdDir)];
            if (temp.is_used && strcmp(temp.diriteminfo->d_name, file_path) == 0 && temp.parent_inode == stack_peek(&temp_stack)) {
                file_exists = 1;
                stack_push(temp.inode, &temp_stack);
                //printf("Dir change to %s\n", file_path);
                dir_change_count--;
                break;
            }
        }

        // file already exists
        if (file_exists && dir_change_count <= 0) {
            lba_pos = temp.directoryStartLocation;
            //printf("File %s already exists.\n", file_path);
        }

        // directory does not exist. i.e., dir1/dir2/file1 ---- dir2 or dir1 does not exist
        if (!file_exists && dir_change_count > 0) {
            lba_pos = -1;
            //printf("Directory %s does not exist.\n", file_path);
            break;
        }

        // directory does exist. i.e., dir1/dir2/file1 ---- dir1 and dir2 exists but file1 does not
        if (!file_exists && dir_change_count == 0) {
            int free_slot = get_free_directory();
            if (free_slot == -1) {
                return -1;
            }
            printf("FREE: %d\n", free_slot);
            int blocks_to_alloc = 0;
            if (file_type == DT_DIR)
                blocks_to_alloc = DEFAULT_DIR_SIZE;
            else if (file_type == DT_REG)
                blocks_to_alloc = DEFAULT_FILE_SIZE;

            int contig_blocks[blocks_to_alloc];
            int * arrs = get_free_blocks_index(vector, contig_blocks, blocks_to_alloc);
            for (int i = 0; i < blocks_to_alloc; ++i)
                set_bit(vector, arrs[i], 1);


            fd_table[free_slot * sizeof(fdDir)].directoryStartLocation = arrs[0];
            lba_pos = arrs[0];
            fd_table[free_slot * sizeof(fdDir)].diriteminfo = malloc(sizeof(fdDir));
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_reclen = DEFAULT_DIR_SIZE;
            fd_table[free_slot * sizeof(fdDir)].inode = inode_index;
            fd_table[free_slot * sizeof(fdDir)].parent_inode = stack_peek(&temp_stack);
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->fileType = file_type;
            stack_push(inode_index, &temp_stack);
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_reclen = 1;
            file_path[strlen(file_path)] = '\0';
            memcpy(fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_name, file_path, strlen(file_path)+1);
            printf("FILE CREATED (mkDir): File: %s, inode: %d, parent: %d\n", fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_name,
                   fd_table[free_slot * sizeof(fdDir)].inode, fd_table[free_slot * sizeof(fdDir)].parent_inode);
            inode_index++;
            dir_used_size++;
        }
        file_path = strtok_r(NULL, "/", &saveptr);
    }


    //fs_setcwd(cwd_temp_buf);
    free(cwd_temp_buf);
    return lba_pos;
}


/*
 * Removes a directory, given the path of the directory to remove.
 * All children will also be removed.
 */
int fs_rmdir(const char *pathname) {
    int is_absolute = 0;
    int ret_val = 0;
    int c_n_index = 0;
    int * nodes_to_remove = malloc(MAX_PATH * sizeof(int));

    if (pathname[0] == '/') {
        stack_push(stack_peek(&current_directory_node), &current_directory_node_cpy2);
        is_absolute = 1;
    }
    // returns the inode of the directory to remove
    int inode_check = get_inode(pathname, is_absolute);
    printf("== %d\n", inode_check);

    char temp_path[strlen(pathname)];
    strcpy(temp_path, pathname);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);

    file_path = strtok_r(NULL, "/", &saveptr);

   /*
    if (inode_check > 0) { // 0 is root, can't remove root. -1 means path not found
        // inode_check is the node to remove
        // every directory where parent_inode  = inode_check must be removed
        for (int i = 0; i < dir_used_size; i++) {
            fdDir temp = fd_table[i * sizeof(fdDir)];
            if (temp.diriteminfo->parent_inode == inode_check) {
                fd_table[i * sizeof(fdDir)].is_used = 0;
                stack_push(temp.diriteminfo->inode, &current_directory_node);
                fs_rmdir(temp.diriteminfo->d_name);

            }
        }
    }
    */

    printf("REMOVED DIRECTORY (rmdir): %s\n", pathname);

    empty_stack(&current_directory_node_cpy2);
    free(nodes_to_remove);

    return ret_val;
}

/*
 * Sets the working directory.
 * Returns:
 *      0 : Success
 *     -1 : Failure
 */
int fs_setcwd(char *buf) { // "/file2/file1"
    if (buf[0] == '/') { // Change to root
        while (stack_size(&current_directory_node) > 1) {
            stack_pop(&current_directory_node);
        }
        fs_getcwd(cwd_buf, 256);
        printf("Directory change to \'%s\'\n", cwd_buf);
    }


    char temp_path[strlen(buf)];
    strcpy(temp_path, buf);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);

    while (file_path != NULL) {
        // '.' or '..'
        if (strcmp(file_path, "..") == 0) {
            stack_pop(&current_directory_node);
            fs_getcwd(cwd_buf, 256);
            printf("Directory change to \'%s\'\n", cwd_buf);
        } else if (strcmp(file_path, ".") == 0) {
            // do nothing?
        } else {

            // Check if file_path is a child of current directory
            int inode_child = get_inode(file_path, 0);
            if (inode_child >= 0) {
                stack_push(inode_child, &current_directory_node);
                fs_getcwd(cwd_buf, 256);
                printf("Directory change to \'%s\'\n", cwd_buf);
            } else { // File does not exist in directory, make a new file and change directory to it
                printf("%s does not exist.\n", file_path);
                return -1;
            }
        }

        file_path = strtok_r(NULL, "/", &saveptr);
    }

    return 0;
}

/*
 * Returns a string of absolute path to current directory. Assumes buf is already allocated.
 * Args:
 *      buf :   the buffer that will be written on, contains absolute path
 *      size:   Size of buffer
 */
char * fs_getcwd(char *buf, size_t size) {
    int pos = 0;
    buf[pos] = '/';
    pos++;
    while (stack_size(&current_directory_node) > 1) {
        stack_push(stack_pop(&current_directory_node), &current_directory_node_cpy);
    }

    while (stack_size(&current_directory_node_cpy) > 1) {
        int stack_val = stack_pop(&current_directory_node_cpy);
        fdDir entry = get_entry(stack_val);
        for (int i = 0; i < size; i++) {
            buf[pos] = entry.diriteminfo->d_name[i];
            if (entry.diriteminfo->d_name[i] == '\0') {
                break;
            }
            pos++;
        }

        buf[pos++] = '/';
        stack_push(stack_val, &current_directory_node); // return stack items back

    }
    buf[pos++] = '\0'; // null terminate

    return  buf;
}

/*
 * Returns a directory entry given the path.
 * Return:
 *      - A directory entry
 *      - A NULL directory entry
 */
fdDir get_directory_entry(char * path) {
    fdDir ret_val;

    char temp_path[strlen(path)];
    strcpy(temp_path, path);
    char * saveptr1;
    char * file_path = strtok_r(temp_path, "/", &saveptr1);

    while (file_path!= NULL) {

        file_path = strtok_r(NULL, "/", &saveptr1);
    }

    return fd_table[-1 * sizeof(fdDir)];
}

int exp_count = 1;
/*
 * Returns the index of a free directory slot.
 *  If there is not enough space, expand.
 * Return:
 *    >= 0 : slot is free
 */
int get_free_directory() {
    int slot = -1;
    for (int i = 0; i < sizeof(fdDir) * DIRECTORY_ENTRY_SIZE * exp_count; i += sizeof(fdDir)) {
        if (!fd_table[i].is_used) {
            slot = i / sizeof(fdDir);
            fd_table[i].is_used = 1;
            break;
        }
    }
    /*TODO: Expand if slot = -1*/
    if (slot == -1) {
    }


    return slot;
}

/*
 * Returns the inode in the CURRENT directory.
 * Return:
 *      -1 : Does not exist
 *      else: file exists, is a child of current dir
 */
int get_inode(const char * path, int is_absolute) {

    if (is_absolute && strlen(path) == 1) { // <-- This is root
        return 0;
    }

    if (is_absolute) { // absolute
        stack_push(0, &current_directory_node_cpy2);

    } else {    // relative
        stack_push(stack_peek(&current_directory_node), &current_directory_node_cpy2);
    }

    int ret_val = 0;
    char temp_path[strlen(path)];
    char * saveptr;
    strcpy(temp_path, path);
    char * file_path = strtok_r(temp_path, "/", &saveptr);

    while (file_path != NULL) {
        // Check if file exists inside current directory
        int is_in_dir = 0;
        for (int i = 0; i < dir_used_size; i++) {
            fdDir temp = fd_table[i * sizeof(fdDir)];
            if (temp.is_used) {
                //printf("%d %d\n", temp.diriteminfo->parent_inode, stack_peek(&current_directory_node_cpy2));

                if (strcmp(temp.diriteminfo->d_name, file_path) == 0 && temp.parent_inode == stack_peek(&current_directory_node_cpy2)) {
                    //printf("%s\n", file_path);

                    is_in_dir = 1;
                    stack_push(temp.inode, &current_directory_node_cpy2);
                    ret_val = fd_table[i * sizeof(fdDir)].inode;
                }
            }
        }
        if (is_in_dir == 0) {
            ret_val = -1;
            break;
        }
        file_path = strtok_r(NULL, "/", &saveptr);
    }
    //empty_stack(&current_directory_node_cpy2);
    return ret_val;
}

/*
 * Returns dir entry if inode is assigned to it.
 */
fdDir get_entry(int inode) {
    for (int i = 0; i < dir_used_size * sizeof(fdDir); i += sizeof(fdDir)) {
        fdDir temp = fd_table[i];
        if (temp.inode == inode) {
            return fd_table[i];
        }

    }

    return fd_table[-1];
}

char * temp_file_type;

/*
 * Prints directory table
 */
void print_table() {
    temp_file_type = malloc(10);

    printf("\n");
    for (int i = 0; i < 70; i++) {
        if (i == 0) {
            printf("\u250F");
            continue;
        }
        if (i == 69) {
            printf("\u2513");
            continue;
        }
        if (i >= 23 && i <= 34) {
            if (i == 23) {
                printf("[DIRECTORY ENTRIES]");
            }
        }
        printf("\u2501");
    }
    printf("\n\u2503\tFILE_NAME\t \u2502\t INODE\t    \u2502   PARENT_INODE  \u2502   LBA_POS   \u2502   F_TYPE  \u2503\n");

    for (int i = 0; i < 89; i++) {

        if (i == 0) {
            printf("\u2503");
            continue;
        }

        if (i == 88) {
            printf("\u2503");
            continue;
        }

        if (i == 25 || i == 44 || i == 62 || i == 76) {
            printf("\u253F");
            continue;
        }
        printf("\u2501");
    }

    printf("\n");
    for (int i = 0; i < dir_used_size; i++) {
        fdDir temp = fd_table[i * sizeof(fdDir)];

        if (!temp.is_used) {
            continue;
        }

        if (temp.diriteminfo->fileType == DT_DIR) {
            temp_file_type = "DT_DIR";
        } else if (temp.diriteminfo->fileType == DT_REG) {
            temp_file_type = "DT_REG";
        } else {
            temp_file_type = "DT_LNK";
        };


        printf("\u2503  %21s \u2502     %08d     \u2502     %08d    \u2502   %08ld  \u2502 \t%s  \u2503\n", temp.diriteminfo->d_name,
               temp.inode, temp.parent_inode, (long)temp.directoryStartLocation, temp_file_type);
        for (int j = 0; j < 89; j++) {
            if (j == 0 && i == dir_used_size-1) {
                printf("\u2517");
                continue;;
            }
            if (j == 88 && i == dir_used_size-1) {
                printf("\u251B");
                continue;;
            }

            if ((j == 25 || j == 44 || j == 62 || j == 76) && i == dir_used_size-1) {
                printf("\u2537");
                continue;;
            }


            if (j == 88) {
                printf("\u2528");
                continue;
            }
            if (j == 0) {
                printf("\u2520");
                continue;
            }

            if (j == 25 || j == 44 || j == 62 || j == 76) {
                printf("\u253C");
                continue;
            }

            if (i == dir_used_size-1)
                printf("\u2501");
            else
                printf("\u2500");
        }
        printf("\n");
    }
}

/*
 * Frees mallocs
 */
void free_dir_mem() {
    free(cwd_buf);
    free(fd_table);
    free(file_stat);
    rm_stack();
}

// b_functions helpers

/*
 * Returns the position of a file if it exists.
 * Otherwise, -1 is returned.
 */
int getLBAPosition(char * filepath) {
    int LBA_POS = -1;
    empty_stack(&current_directory_node_cpy2);
    stack_push(0, &current_directory_node_cpy2);

    // get the count of directory changes requested (i.e., dir1/dir2/new_file or dir1/dir2/existing_file)
    int dir_change_count = 0;
    for (int i = 0; i < strlen(filepath); i++) {
        if (filepath[i] == '/')
            dir_change_count++;
    }

    // relative path
    if (filepath[0] != '/') {
        stack_push(stack_peek(&current_directory_node), &current_directory_node_cpy2);
    } else { // don't want to count the first /
        dir_change_count--;
    }

    char temp_path[strlen(filepath)];
    strcpy(temp_path, filepath);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);

    while (file_path != NULL) {

        if (strcmp(file_path, "..") == 0) {
            stack_pop(&current_directory_node_cpy2);
        }
        for (int i = 0; i < dir_used_size; i++) {
            fdDir temp = fd_table[i * sizeof(fdDir)];

            if (temp.is_used && strcmp(temp.diriteminfo->d_name, file_path) == 0 && temp.parent_inode == stack_peek(&current_directory_node_cpy2)) {
                LBA_POS = temp.directoryStartLocation;
                stack_push(temp.inode, &current_directory_node_cpy2);
                break;
            }
        }

        if (!dir_change_count && LBA_POS) {
            LBA_POS = fs_mkdir(filepath, 0, DT_REG);
        }
        dir_change_count--;
        file_path = strtok_r(NULL, "/", &saveptr);
    }

    return LBA_POS;
}