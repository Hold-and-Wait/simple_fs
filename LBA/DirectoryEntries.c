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

#define DIRECTORY_ENTRY_SIZE 50
#define MAX_PATH 128
#define DEFAULT_DIR_SIZE 10

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
int dir_index;

// supports
char * cwd_buf;
Bitvector * vector;

fdDir get_entry(int inode);

void initializeDirectory(Bitvector * vec) {
    vector = vec;
    fd_table = malloc(sizeof(fdDir) * DIRECTORY_ENTRY_SIZE);
    current_directory_node = create_stack(MAX_PATH);
    current_directory_node_cpy = create_stack(MAX_PATH);
    current_directory_node_cpy2 = create_stack(MAX_PATH);
    file_stat = malloc(sizeof(struct fs_stat));
    inode_index = 1;
    dir_index = 0;
    dir_used_size = 0;

    cwd_buf = malloc(256);

    stack_push(200, &current_directory_node_cpy);

    // Create root
    stack_push(0, &current_directory_node);

    //fs_mkdir("/", 0);


    printf("DIRECTORY ENTRY STRUCTURES: fdDir (%d), path_stack (%d) INITIALIZED.\n", DIRECTORY_ENTRY_SIZE, MAX_PATH);
}

// ignore mode for now
int fs_mkdir(const char *pathname, mode_t mode) {
    int is_absolute = 0;
    char * cwd_temp_buf = malloc(512);
    struct stack_util temp_stack = create_stack(MAX_PATH);
    fs_getcwd(cwd_temp_buf, 512);

    if (pathname[0] == '/') {
        is_absolute = 1;
    }

    char temp_path[strlen(pathname)];
    stack_push(stack_peek(&current_directory_node), &temp_stack);
    strcpy(temp_path, pathname);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);
    int file_exists = 0;
    int parent_node = 0;
    int * contig_blocks = malloc(DEFAULT_DIR_SIZE * sizeof(int));
    while (file_path != NULL) {
        for (int i = 0; i < dir_used_size; i++) {
            fdDir temp = fd_table[i * sizeof(fdDir)];
            if (temp.is_used && strcmp(temp.diriteminfo->d_name, file_path) == 0) {
                file_exists = 1;
                stack_push(temp.diriteminfo->inode, &temp_stack);
            }
        }

        if (!file_exists) {
            int free_slot = get_free_directory();
            // Call free space management here to assign the file/directory a slot/slots in lba array

            get_free_blocks_index(vector, get_vector_size(vector), contig_blocks, DEFAULT_DIR_SIZE);
            for (int i = 0; i < DEFAULT_DIR_SIZE; i++) {
                //set_bit(vector, contig_blocks[i], 1);
                //printf("--=-- %d, %d\n", contig_blocks[i], get_bit(vector, i));
            }

            fd_table[free_slot * sizeof(fdDir)].directoryStartLocation = contig_blocks[0];
            fd_table[free_slot * sizeof(fdDir)].diriteminfo = malloc(sizeof(fdDir));
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_reclen = DEFAULT_DIR_SIZE;
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->inode = inode_index;
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->parent_inode = stack_peek(&temp_stack);
            stack_push(inode_index, &temp_stack);
            fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_reclen = 1;
            memcpy(fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_name, file_path, strlen(file_path));
            printf("FILE CREATED (mkDir): File: %s, inode: %d, parent: %d\n", fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_name,
                   fd_table[free_slot * sizeof(fdDir)].diriteminfo->inode, fd_table[free_slot * sizeof(fdDir)].diriteminfo->parent_inode);
            inode_index++;
            dir_used_size++;
        }

        file_exists = 0;
        file_path = strtok_r(NULL, "/", &saveptr);
    }


    //fs_setcwd(cwd_temp_buf);
    free(cwd_temp_buf);
    free(contig_blocks);
    return 0;
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
int fs_setcwd(char *buf) {
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

/*
 * Returns the index of a free directory slot.
 *  If there is not enough space, expand.
 * Return:
 *    >= 0 : slot is free
 */
int get_free_directory() {
    int slot = -1;
    for (int i = 0; i < sizeof(fdDir) * MAX_PATH; i += sizeof(fdDir)) {
        if (!fd_table[i].is_used) {
            slot = i / sizeof(fdDir);
            fd_table[i].is_used = 1;
            break;
        }
    }

    /*TODO: Expand if slot = -1*/

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

                if (strcmp(temp.diriteminfo->d_name, file_path) == 0 && temp.diriteminfo->parent_inode == stack_peek(&current_directory_node_cpy2)) {
                    //printf("%s\n", file_path);

                    is_in_dir = 1;
                    stack_push(temp.diriteminfo->inode, &current_directory_node_cpy2);
                    ret_val = fd_table[i * sizeof(fdDir)].diriteminfo->inode;
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
        if (temp.diriteminfo->inode == inode) {
            return fd_table[i];
        }

    }

    return fd_table[-1];
}

/*
 * Prints directory table
 */
void print_table() {

    printf("\n");
    for (int i = 0; i < 58; i++) {
        if (i == 0) {
            printf("\u250F");
            continue;
        }
        if (i == 57) {
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
    printf("\n\u2503\tFILE_NAME\t \u2502\t INODE\t    \u2502   PARENT_INODE  \u2502   LBA_POS   \u2503\n");

    for (int i = 0; i < 77; i++) {

        if (i == 0) {
            printf("\u2503");
            continue;
        }

        if (i == 62) {
            printf("\u253F");
            continue;
        }

        if (i == 76) {
            printf("\u2503");
            continue;
        }

        if (i == 25 || i == 44) {
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

        printf("\u2503  %21s \u2502     %08d     \u2502     %08d    \u2502   %08ld  \u2503\n", temp.diriteminfo->d_name,
               temp.diriteminfo->inode, temp.diriteminfo->parent_inode, (long)temp.directoryStartLocation);

        for (int j = 0; j < 77; j++) {
            if (j == 0 && i == dir_used_size-1) {
                printf("\u2517");
                continue;;
            }
            if (j == 76 && i == dir_used_size-1) {
                printf("\u251B");
                continue;;
            }

            if ((j == 25 || j == 44 || j == 62) && i == dir_used_size-1) {
                printf("\u2537");
                continue;;
            }


            if (j == 76) {
                printf("\u2528");
                continue;
            }
            if (j == 0) {
                printf("\u2520");
                continue;
            }

            if (j == 25 || j == 44 || j == 62) {
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