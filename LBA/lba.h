#ifndef _MFS_H
#define _MFS_H
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

#include "b_io.h"

#include <dirent.h>
#include "../bitmap_vector.h"

#define FT_REGFILE	DT_REG
#define FT_DIRECTORY DT_DIR
#define FT_LINK	DT_LNK

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif


// metadata
struct fs_diriteminfo
{
    int inode;
    int parent_inode;

    unsigned short d_reclen;    /* length of this record, aka total blocks it takes up */
    unsigned int file_size;
    char fileType;
    char *    st_mod_time;   // time modified
    char *    st_create_time;   // time modified
    char d_name[256]; 			/* filename max filename is 255 characters */
};


typedef struct
{
    int inode;
    int parent_inode;

    /*****TO DO:  Fill in this structure with what your open/read directory needs  *****/
    unsigned short	dirEntryPosition;	/*which directory entry position, like file pos */
    uint64_t	directoryStartLocation;		/*Starting LBA of directory */
    int is_used;
//
} fdDir;

// fdDir * a_directory = fs_opendir("file1");
// write, a_directory->dirEntryPosition += count;

void initializeDirectory(Bitvector * vec, int LBA_Pos);
void print_dir();
void free_dir_mem();
void offload_configs();
int dirent_check_is_init();
int is_valid_dir(char * path, char * new_dir_name);
int get_free_inode();

fdDir get_directory_entry(char * path);

int fs_mkdir(char *pathname, mode_t mode, int file_type);
int fs_rmdir(char *pathname); // todo
fdDir * fs_opendir(const char *name);
struct fs_diriteminfo *fs_readdir(fdDir *dirp);
int fs_closedir(fdDir *dirp);
int getLBAPosition(char * filepath);

char * fs_getcwd(char *buf, size_t size);
int fs_setcwd(char *buf);   //linux chdir
int fs_isFile(char * path);	//return >=0 lba position of the file, -1 otherwise
int fs_isDir(char * path);		//return 1 if directory, 0 otherwise
int fs_delete(char* filename);	//removes a file



struct fs_stat
{
    off_t     st_size;    		/* total size, in bytes */
    blksize_t st_blksize; 		/* blocksize for file system I/O */
    blkcnt_t  st_blocks;  		/* number of 512B blocks allocated */
    time_t    st_accesstime;   	/* time of last access */
    time_t    st_modtime;   	/* time of last modification */
    time_t    st_createtime;   	/* time of last status change */

    /* add additional attributes here for your file system */

    int is_init;

};

int fs_stat(const char *path, struct fs_stat *buf);

#endif