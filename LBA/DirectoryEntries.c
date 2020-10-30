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
    int index_of_free_entry = 0;

    // expand directory table if not enough slots
    if (dir_index == dir_entry_size) {
        expandDirectoryEntryTable(10);
    }

    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        if (dirEntry[i].self_inode == 0 && i != 0) { // Add directory entry to free slot (inode == 0)
            dirEntry[i].self_name = file_path;
            dirEntry[i].self_inode = inode_index;
            printf("File %s (inode %d) has been created.\n", dirEntry[i].self_name, dirEntry[i].self_inode);
            inode_index++;
            return 0;
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
            return 0;
        }
    }
    printf("Could not remove %s. File not found\n", file_path);
    return -1;
}

/*
 * Prints directory table.
 */
void printDirectoryTable() {
    for (int i = 0; i < dir_entry_size * sizeof(struct dir_entry); i+=sizeof(struct dir_entry)) {
        printf("Index %lu:\tFile Name: %s, File inode: %d\n", i/sizeof(struct dir_entry), dirEntry[i].self_name, dirEntry[i].self_inode);
    }
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