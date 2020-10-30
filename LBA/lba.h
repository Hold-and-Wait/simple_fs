// Should we combine all headers and function definitions for LBA here?

// Structs & Defs here

void testDEFunction();
void testFSMFunction();
void testMBRFunction();

// Directory Entry
int initializeDirectoryEntryTable();
int expandDirectoryEntryTable(int num_entries);
int mkDir(char * file_path);
int rmDir(char * file_path);
struct dir_entry getDirectoryEntry(char * file_name);
struct dir_entry getDirectoryEntry_node(int inode);

void printDirectoryTable();

struct dir_files {
    char * self_name;
    int self_inode;
};

struct dir_entry {
    char * self_name;   // .
    int self_inode;     // a unique inode number that maps filename to inode

    char * parent_name; // ..
    int parent_inode;
};

// maps inode to corresponding file name.
struct inode_table {
    char * file_name;

    int month, day, year, hour, minute, second; // Date created/modified
    int inode;

    int * logical_blocks; // array of block numbers that file occupies

    struct dir_files * files_in_dir; // array of files/folders within a directory.
};
// END Directory Entry
