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
int dir_index;

struct inode_table * inodeTable;
int inode_index;

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
    dir_entry_size = 50;
    dir_index = 0;
    inode_index = 1;
    dirEntry = malloc(sizeof(struct dir_entry) * dir_entry_size);

    if (dirEntry == NULL) {
        printf("FAILED to initialize Directory Entry Table.\n");
        return -1;
    }

    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        dirEntry[i].self_inode = -1;
    }

    printf("Directory Entry Table is initialized.\n");

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
 * Args: A file path.
 * Return:
 *      0 : Successful creation of directory
 *     -1 : Failure
 */
int mkDir(char * file_path) {
    /*TODO*/

    // expand directory table if not enough slots
    if (dir_index == dir_entry_size) {
        expandDirectoryEntryTable(10);
    }

    dirEntry[dir_index * sizeof(struct dir_entry)].self_name = file_path;
    dirEntry[dir_index * sizeof(struct dir_entry)].self_inode = inode_index;
    printf("File with name: %s, inode: %d has been created.\n", dirEntry[dir_index * sizeof(struct dir_entry)].self_name, dirEntry[dir_index * sizeof(struct dir_entry)].self_inode);

    dir_index++;
    inode_index++;
    return 0;
}


struct dir_entry getDirectory(int inode) {
    char * file_name = NULL;

    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_inode == inode) {
            return dirEntry[i];
        }
    }

    return dirEntry[0];
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


/*
 * *********** INODE TABLE ***********
 */

int initializeInodeTable() {
    inodeTable = malloc(sizeof(struct inode_table) * 50);

    if (inodeTable == NULL) {
        printf("FAILED to initialize inode table.\n");
        return -1;
    }

    return 0;
}

void mapInode(char * file_path, int inode) {

}