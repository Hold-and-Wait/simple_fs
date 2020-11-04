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

// Functions that do not need to be in header file
int get_free_directory();
int get_inode(char * path);
fdDir get_directory_entry(char * path);

struct stack_util current_directory_node;
struct stack_util current_directory_node_cpy;
struct fs_diriteminfo * dir_info;
struct fs_stat * file_stat;
fdDir * fd_table;

// Index tracking and expansion
int inode_index;
int dir_used_size;
int dir_index;

fdDir get_entry(int inode);

void initializeDirectory() {

    fd_table = malloc(sizeof(fdDir) * DIRECTORY_ENTRY_SIZE);
    current_directory_node = create_stack(MAX_PATH);
    current_directory_node_cpy = create_stack(MAX_PATH);
    file_stat = malloc(sizeof(struct fs_stat));
    inode_index = 0;
    dir_index = 0;
    dir_used_size = 0;

    stack_push(200, &current_directory_node_cpy);

    // Create root
    stack_push(0, &current_directory_node);

    fs_mkdir("/", 0);


    printf("DIRECTORY ENTRY STRUCTURES: fdDir (%d), path_stack (%d) INITIALIZED.\n", DIRECTORY_ENTRY_SIZE, MAX_PATH);
}

// ignore mode for now
int fs_mkdir(const char *pathname, mode_t mode) {
    int free_slot = get_free_directory();
    fd_table[free_slot * sizeof(fdDir)].diriteminfo = malloc(sizeof(fdDir));
    fd_table[free_slot * sizeof(fdDir)].diriteminfo->inode = inode_index;
    fd_table[free_slot * sizeof(fdDir)].diriteminfo->parent_inode = stack_peek(&current_directory_node);
    fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_reclen = 1;
    memcpy(fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_name, pathname, strlen(pathname));

    printf("mkDir: File: %s, inode: %d, parent: %d\n", fd_table[free_slot * sizeof(fdDir)].diriteminfo->d_name,
           fd_table[free_slot * sizeof(fdDir)].diriteminfo->inode, fd_table[free_slot * sizeof(fdDir)].diriteminfo->parent_inode);
    inode_index++;
    dir_used_size++;
    return 0;
}

int fs_rmdir(const char *pathname) {
    int is_absolute = 0;

    if (pathname[0] == '/') {
        is_absolute = 1;
    }

    char temp_path[strlen(pathname)];
    strcpy(temp_path, pathname);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);

    while (file_path != NULL) {

    }

    return 0;
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
        printf("Directory change to root.\n");
    }


    char temp_path[strlen(buf)];
    strcpy(temp_path, buf);

    char * saveptr;
    char * file_path = strtok_r(temp_path, "/", &saveptr);

    while (file_path != NULL) {
        // '.' or '..'
        if (strcmp(file_path, "..") == 0) {
            stack_pop(&current_directory_node);
        } else if (strcmp(file_path, ".") == 0) {
            // do nothing?
        } else {

            // Check if file_path is a child of current directory
            int inode_child = get_inode(file_path);
            if (inode_child >= 0) {
                stack_push(inode_child, &current_directory_node);
            } else { // File does not exist in directory, make a new file and change directory to it
                fs_mkdir(file_path, 0);
                fs_setcwd(file_path);
            }
        }

        printf("Directory change to %s\n", file_path);
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
    buf[pos++] = '\0';

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
int get_inode(char * path) {

    if (path[0] == '/' && strlen(path) == 1) { // <-- This is root
        return 1;
    }

    int ret_val = 0;
    char temp_path[strlen(path)];
    strcpy(temp_path, path);
    char * file_path = strtok(temp_path, "/");

    while (file_path != NULL) {
        // Check if file exists inside current directory
        int is_in_dir = 0;
        for (int i = 0; i < dir_used_size; i++) {
            fdDir temp = fd_table[i * sizeof(fdDir)];
            if (temp.is_used) {
                if (strcmp(temp.diriteminfo->d_name, file_path) == 0) {
                    is_in_dir = 1;
                    ret_val = fd_table[i * sizeof(fdDir)].diriteminfo->inode;
                    break;
                }
            }
        }
        if (is_in_dir == 0) {
            printf("%s does not exist.\n", file_path);
            return -1;
        }
        file_path = strtok(NULL, "/");
    }
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
    printf("\n\t**** DIR TABLE ****\n");
    for (int i = 0; i < dir_used_size; i++) {
        fdDir temp = fd_table[i * sizeof(fdDir)];
        if (!temp.is_used)
            continue;
        printf("File: %s, inode: %d, parent_inode: %d\n", temp.diriteminfo->d_name,
               temp.diriteminfo->inode, temp.diriteminfo->parent_inode);
    }
    printf("\t******************\n\n");
}