/**************************************************************
* Class:  CSC-415
* Name: Hold & Wait
* Student ID: 920572026, 
* Project: simple shell 
*
* File: fsi.h // file system inodes header
*
* Description: 
* This is the file system's method prototypes. initializes and changes info that belong to files in the file system
*
**************************************************************/

#ifndef _FSI_H
#define _FSI_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>


#define MAX_FILEPATH_SIZE 225
#define	MAX_FILENAME_SIZE 20
#define MAX_DIRECTORY_DEPTH 10
#define MAX_NUMBER_OF_CHILDREN 64		
#define MAX_DATABLOCK_POINTERS	64
#define INVALID_DATABLOCK_POINTER -1
#define INVALID_INODE_NAME	"unused_inode"


/* In memory structure defined by linux. */

struct fsInode_dir_entry
{
    ino_t          d_ino;       /* inode number  - not used by driver*/
    off_t          d_off;       /* offset to the next dirent  - not used by driver*/
    unsigned short d_recordlen;    /* length of this record */
    unsigned char  d_type;      /* type of file; not supported by all file system types  - not used by driver*/
    char           d_name[256]; /* filename */
};

typedef enum { I_FILE, I_DIR, I_UNUSED } InodeType;

char* getInodeTypeName(char* buf, InodeType type);


typedef struct
{
		uint64_t id; // holds the index of the inode in the inodes array 
		int inUse; // holds 0 if inode is free and 1 if it is in use
		InodeType type; // holds the type of the inode I_FILE or I_DIR
		char parent[MAX_FILEPATH_SIZE];  // holds the parenr path
		char children[MAX_NUMBER_OF_CHILDREN][MAX_FILENAME_SIZE]; // an array that holds names of the children
		int numChildren; // holds number of children in a DIR
		char name[MAX_FILENAME_SIZE];  // holds the file name
		char path[MAX_FILEPATH_SIZE];  // holds the path of the file/folder
		time_t lastAccessTime; // holds time last accessed 
		time_t lastModificationTime; //  holds time last modifed 
		blkcnt_t sizeInBlocks; // holds the size of the file by block, 512 each block
		off_t sizeInBytes; // holds the size of the file in bytes
		int directBlockPointers[MAX_DATABLOCK_POINTERS]; // an array that holds pointers to the data blocks
		int numDirectBlockPointers; // holds the number of elements in the array of pointers to the data blocks


}fsi_DIR; // file system inode fsi

// SOME OF THIS FUNCTIONS might already been created in LAB folder!!! 
// Changed mfs_setcwd to return an int instead of char*
int fs_mkdir(const char *pathname, mode_t mode);
int fs_rmdir(const char *pathname);
fsi_DIR * fs_opendir(const char *fileName);
struct fsInode_dir_entryt *fs_readdir(fsi_DIR *dirptr);
int fs_closedir(fsi_DIR *dirptr);


char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdir

int fs_isFile(char * path);    //return 1 if file, 0 otherwise
int fs_isDir(char * path);        //return 1 if directory, 0 otherwise
int fs_delete(char* filename);    //removes a file

//*******************************//
void fs_init(); 
void writeInodes(); 
void fs_close(); 

void parseFilePath(const char *pathname); 
void printFilePath(); 

fsi_DIR * getInode(const char * pathname);
fsi_DIR * getFreeInode(); 
void printPWD(); // prints present working directory pwd

fsi_DIR* createInode(InodeType type,const char* path);
int checkValidityOfPath();
int setParent(fsi_DIR* parent, fsi_DIR* child);
char* getParentPath(char* buf ,const char* path);

fsi_DIR* getInodeByID(int id);


/* Writes a buffer to a provided data block, adds blockNumber to inode, updates size and timestamps
 * of inode, writes inodes to disk. Returns number of blocks written. */
int writeBufferToInode(fsi_DIR* inode, char* buffer, size_t bufSizeBytes, uint64_t blockNumber);

void freeInode(fsi_DIR* node); // free space inode




struct fsi_stat {
	dev_t     st_dev;     /* ID of device containing file - not needed by driver*/
	ino_t     st_ino;     /* inode number - not needed by driver program*/
	mode_t    st_mode;    /* protection  - not needed by driver program*/
	nlink_t   st_nlink;   /* number of hard links - not needed by driver program*/
	uid_t     st_uid;     /* user ID of owner - not needed by driver program*/
	gid_t     st_gid;     /* group ID of owner - not needed by driver program*/
	dev_t     st_rdev;    /* device ID (if special file) - not needed by driver program*/
	off_t     st_size;    /* total size, in bytes */
	blksize_t st_blksize; /* blocksize for file system I/O */
	blkcnt_t  st_blocks;  /* number of 512B blocks allocated */
	time_t    st_accesstime;   /* time of last access */
	time_t    st_modtime;   /* time of last modification */
	time_t    st_createtime;   /* time of last status change */
	
	/* add additional attributes here for your file system */
};

int fsi_stat(const char *path, struct fsi_stat *buf);



#endif