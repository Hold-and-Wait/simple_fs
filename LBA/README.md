# DIRECTORY ENTRY TABLE
Author: Mark Jovero
## DIRECTORY ENTRIES
### Description
A directory entry holds the following data:
* File Name
* File inode:
  * A unique number that is assigned to a file/directory upon creation
* Parent's inode

Since the array of directory entries is loaded in memory at all times, it is best to minimize the amount of data stored in it.

## INODE ENTRIES
### Description
Each file will have an inode table. The inode table contains:
* inode
* Date of modification
* file size
* file type
* logical blocks occupied
* files in directory
  * This may not be needed since each file is already linked to a parent.
* Number of hard-links
  * This may not be needed, professor never mentioned implementing it.

The inode table holds much more bytes compared to the directory entry table. Therefore, it is best to keep the inode table stored on disk rather than loaded in memory. Each file's inode table will only be loaded when they are being accessed.

(To be implemented)
To access the inode table, a function will be responsible for taking an inode number, accessing the disk, and search/return the matching inode.

## Rundown of key functions
* initializeDirectoryEntryTable
  * Initializes the directory entry table. It takes in a root directory name as an argument.
  * This root directory will point to itself, which is the definition of the root directory.
* mkDir
  * Creates a directory.
  * This newly created directory will create a new entry in the directorey entry table. Its parent node will be the current directory's inode.
* rmDir
  * Removes directory and all its children.
  * This function is still in development
* checkValidFile
  * Checks if a file exists. Returns 0 for false, 1 for true.
  * Takes a file name as an argument.
  * The directories "." and ".." are always viewed as valid files.
* setWorkingDirectory
  * Takes a file name or path as an argument
  * If a path does not exist, it will create the directory needed.
* getWorkingDirectory
  * Returns the current working directory entry.
* getDirectoryEntry
  * Returns a directory entry.
  * It takes a file name as an argument.
* getDirectoryEntry_node
  * Returns a directory entry.
  * It takes an inode number as an argument.
* printDirectoryTable
  * Prints the directory entry table.
