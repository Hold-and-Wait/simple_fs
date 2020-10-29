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

    int * logical_blocks;
    int inode;

    struct dir_files * f; // array of files/folders within a directory.
};
// END Directory Entry
