// Should we combine all headers and function definitions for LBA here?

// Structs & Defs here

void testDEFunction();
void testFSMFunction();
void testMBRFunction();

// Directory Entry
int initializeDirectoryEntryTable(char * root_file);
int initializeInodeTable();
int expandDirectoryEntryTable(int num_entries);
int mkDir(char * file_path);
int rmDir(char * file_path);
int checkValidFile(char * file_name);
int setWorkingDirectory(char * path);
struct dir_entry getDirectoryEntry(char * file_name);
struct dir_entry getDirectoryEntry_node(int inode);
struct dir_entry getWorkingDirectory();

void printDirectoryTable();

struct dir_files {
    char * self_name;
    int self_inode;
};

struct dir_entry {
    char * self_name;   // .
    int self_inode;     // a unique inode number that maps filename to inode

    int parent_inode; // ..
};


/*
 * Inodes will have a unique number. Not randomly generated, just incrementing
 *  by 1 when a file is created.
 *
 * Since the directory entry will most likely be loaded in memory most of
 *  the time, we do NOT want to fill the entry with too many information.
 *  Thus, it is best to keep that information in the inode table, which
 *  will be loaded onto memory only when needed.
 */
struct inode_table {

    int month, day, year, hour, minute, second; // Date created/modified. Use built-in c function for time
    int inode; // A unique inode number
    int hard_link_count; // number of times another file references the same inode number (and logical_blocks)
                         // ^ is this needed for our file system?

    int file_size;
    char * file_type; // How would we manage this? maybe... if we have 'file.txt', take substring to the right of the
                      // period and assign it to this variable.

    int * logical_blocks; // array of block numbers that the file occupies (i.e., the root dir might occupy lba [2, 3, 4])

    struct dir_files * files_in_dir; // array of files/folders within a directory.
                                     // ^ Probably would not need this portion since each entry has a parent node already.
};
// END Directory Entry
