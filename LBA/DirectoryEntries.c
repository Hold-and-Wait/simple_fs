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

struct dir_entry * dirEntry;
int dir_entry_size;
int dir_index;

struct inode_table * inodeTable;
int inode_index;

char * current_working_dir_path;
struct dir_entry current_working_dir_entry;
struct stack_util dir_path_stack; // Holds array of dir entries that current working directory is in.

void testDEFunction() {
    printf("Dir Entry Test\n");
}

/*
 * *********** WORKING DIR MANAGEMENT ***********
 */

/*
 * Tokenizes a string (i.e., 'root/path1/path2/file0')
 *  into an array.
 */
void tokenizeDirectoryPath(char * file_path) {
    /* TODO */
    int slash_count = 0;
    for (int i = 0; i < strlen(file_path); i++) {
        if (strcmp(&file_path[i], "/") == 0) {
            slash_count++;
        }
    }
}


/*
 * Return: The current directory entry.
 */
struct dir_entry getWorkingDirectory() {
    return current_working_dir_entry;
}

/*
 * Sets the working directory. (Change directory)
 * Return:
 *      0 : Successful change
 *     -1 : Failure
 */
int setWorkingDirectory(char * path) {
    int is_absolute_path = 0;
    if (path[0] == '/') {
        is_absolute_path = 1;
    }

    const char * delim = "/";
    char path_temp[10000]; // temp array used for strtok
    strcpy(path_temp, path);
    char * file_name = strtok(path_temp, delim);
    while (file_name != NULL) {

        if (!checkValidFile(file_name)) {
            printf("File %s not found.\n", file_name);
            return -1;
        }

        // check absolute path (i.e., /root/file1/...)
        if (is_absolute_path) {
            while (stack_size(&dir_path_stack)  > 1) { // pop until root
                stack_pop(&dir_path_stack);
            }
            is_absolute_path = 0;
        }

        // . or ..
        if (strcmp(file_name, ".") == 0 || strcmp(file_name, "..") == 0) {
            if (strcmp(file_name, "..") == 0) {
                current_working_dir_entry = getDirectoryEntry_node(getWorkingDirectory().parent_inode);
                if (stack_size(&dir_path_stack) > 1) { // prevents popping of root
                    stack_pop(&dir_path_stack);
                }
            }
        }

        // root/file1/.../
        else {

            // check if requested directory change inode is a child of current directory
            if (getDirectoryEntry(file_name).parent_inode == getWorkingDirectory().self_inode) {
                current_working_dir_entry = getDirectoryEntry(file_name);
                stack_push(getWorkingDirectory().self_inode, &dir_path_stack);
            } else {
                printf("\'%s\' is not a child of \'%s\'. Directory change not possible.\n", file_name, getWorkingDirectory().self_name);
                return -1;
            }

        }
        printf("Directory change to \'%s\'.\n", getDirectoryEntry_node(stack_peek(&dir_path_stack)).self_name);
        file_name = strtok(NULL, delim);
    }
    return -1;
}


/*
 * *********** INODE TABLE ***********
 */


int inode_table_size;

/*
 * Initializes inode table to 50 entries.
 * Return:
 *      0 : Successful initialization
 *     -1 : Failure
 */
int initializeInodeTable() {
    inode_table_size = 50;
    inodeTable = malloc(sizeof(struct inode_table) * inode_table_size);

    if (inodeTable == NULL) {
        printf("FAILED to initialize inode table.\n");
        return -1;
    }

    return 0;
}


/*
 * *********** Dir Entry TABLE ***********
 */

int is_entry_table_init = 0;
/*
 * Initializes directory entry table.
 * Return:
 *      0 : Successful initialization
 *     -1 : Failure
 */
int initializeDirectoryEntryTable(char * root_file) {
    dir_entry_size = 50;
    dir_index = 0;
    inode_index = 1; // 0 means free. For some reason, I am unable to set inode value in dirEntry to -1.
    dirEntry = malloc(sizeof(struct dir_entry) * dir_entry_size);

    if (dirEntry == NULL) {
        printf("FAILED to initialize Directory Entry Table.\n");
        return -1;
    }
    dir_path_stack = create_stack(10);
    mkDir(root_file);
    current_working_dir_entry = getDirectoryEntry(root_file);
    printf("Directory Entry Table is initialized. Root directory: %s\n", current_working_dir_entry.self_name);

    return 0;
}


/*
 * Expands directory entry table.
 * Args: The number of entries to expand by. (i.e. 10 would add 10 to the size.)
 * Return:
 *      0 : Successful expansion
 *     -1 : Failure
 */
int expandDirectoryEntryTable(int num_entries) {
    dir_entry_size += num_entries;
    dirEntry = realloc(dirEntry, dir_entry_size);

    if (dirEntry == NULL) {
        printf("FAILED to expand Directory Entry Table.\n");
        return -1;
    }
    printf("Directory Entry Table is expanded (%d).\n", dir_entry_size);

    return 0;
}


/*
 * Makes a new folder.
 * Args: A file path of new folder and the parent of the folder.
 * Return:
 *      0 : Successful creation of directory
 *     -1 : Failure
 */
int mkDir(char * file_name) {
    int index_of_free_entry = 0;

    // expand directory table if not enough slots
    if (dir_index == dir_entry_size) {
        expandDirectoryEntryTable(10);
    }

    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_inode == 0) { // Add directory entry to free slot (inode == 0)
            dirEntry[i].self_name = file_name;
            dirEntry[i].self_inode = inode_index;
            inode_index++;
            dirEntry[i].parent_inode = getWorkingDirectory().self_inode;
            if (!is_entry_table_init) {
                dirEntry[i].parent_inode = dirEntry[i].self_inode;
                current_working_dir_entry = dirEntry[i];
                stack_push(dirEntry[i].self_inode, &dir_path_stack);
                is_entry_table_init = 1;
            }
            break;
        }
    }

    return 0;
}

/*
 * Checks if a given file is valid. (NOTE: '.' and '..' are both considered valid files!)
 * Args: A file name
 * Return:
 *      0 : Invalid file name, does not exist
 *      1: Valid file name
 */
int checkValidFile(char * file_name) {
    if (strcmp(file_name, ".") || strcmp(file_name, "..")) {
        return 1;
    }
    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_name != NULL && strcmp(dirEntry[i].self_name, file_name) == 0) {
            return 1;
        }
    }
    return 0;
}


/*
 * Returns a directory given file name.
 * Args: file name
 * Return:
 *      - a directory entry if file name is valid
 *      - a null directory entry if file name does not exist
 */
struct dir_entry getDirectoryEntry(char * file_name) {

    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_name != NULL && strcmp(dirEntry[i].self_name, file_name) == 0) {
            return dirEntry[i];
        }
    }
    printf("Query of file %s has no match.\n", file_name);
    return dirEntry[-1];
}


/*
 * Returns a directory given inode number.
 * Args: inode number
 * Return:
 *      - a directory entry if inode maps to file
 *      - a null directory entry if inode is not mapped
 */
struct dir_entry getDirectoryEntry_node(int inode) {
    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_inode == inode) {
            return dirEntry[i];
        }
    }
    printf("Query of inode %d has no match.\n", inode);
    return dirEntry[-1];
}


/*
 * Removes a directory as well as all files and directories
 *  and memory.
 * Args: A file path.
 * Return:
 *      0 : Successful deletion of directory.
 *     -1 : Failure
 */
int rmDir(char * file_path) {
    /*TODO*/
    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_name != NULL && strcmp(dirEntry[i].self_name, file_path) == 0) {
            printf("Removed %s (inode %d)\n", dirEntry[i].self_name, dirEntry[i].self_inode);
            dirEntry[i].self_name = NULL;
            dirEntry[i].self_inode = 0;
            break;
        }
    }

    printf("Could not remove %s. File not found\n", file_path);
    return -1;
}

/*
 * Prints directory table.
 */
void printDirectoryTable() {
    printf("** Directory Entry Table**\n");
    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_name != NULL) {
            printf("Index %lu: File Name: %s, File inode: %d\n", i / sizeof(struct dir_entry), dirEntry[i].self_name,
                   dirEntry[i].self_inode);
            printf("\tParent: %d\n\n", dirEntry[i].parent_inode);
        }
    }
}
