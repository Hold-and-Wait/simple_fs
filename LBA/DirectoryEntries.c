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

struct dir_entry * dirEntry;
int dir_entry_size;

void testDEFunction() {
    printf("Dir Entry Test\n");
}

/*
 * Initializes directory entry table.
 * Return:
 *      0 : Successful initialization
 *     -1 : Failure
 */
int initializeDirectoryEntryTable() {
    dir_entry_size = 20;
    dirEntry = malloc(sizeof(struct dir_entry) * 20);

    if (dirEntry == NULL) {
        printf("FAILED to initialize Directory Entry Table.\n");
        return -1;
    }

    printf("Directory Entry Table is initialized.\n");

    return 0;
}


/*
 * Expands directory entry table.
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
 * Args: A file path.
 * Return:
 *      0 : Successful creation of directory
 *     -1 : Failure
 */
int mkDir(char * file_path) {
    /*TODO*/
    return 0;
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
    return 0;
}