#include "LBA/lba.h"
#include "mfs.h"
#include "DirectoryEntries.c"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "b_io.h"


fdDir * fs_opendir(const char *name){
    pritnf("-----fdDir-----\n"); 
    int ret = b_open(name, 0); 
    if(ret < 0 ){
        printf("---------\n");
        return NULL;
    }
    printf("--------\n");
    return getInode(name); 
}
int readdirCounter = 0;
struct fs_dirent directoryEntry;

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    printf("---------------mfs_readdir----------\n");
  
  if(readdirCounter == dirp->numChildren) {
    readdirCounter = 0;
    return NULL;
  }
  
  /* Get child inode. */
  char childPath[MAX_FILEPATH_SIZE];
  sprintf(childPath, "%s/%s", dirp->path, dirp->children[readdirCounter]);
  fdDir* child = getInode(childPath);

  /* Set properties on dirent. */
  directoryEntry.d_ino = child->id;
  strcpy(directoryEntry.d_name, child->name);

  /* Increment the counter to the next child. */
  readdirCounter++;

  printf("----------\n");
  return &directoryEntry;
    
}
struct mfs_dirent* mfs_readdir(fdDir *dirp) {
}

int fs_closedir(fdDir *dirp){
    printf("----fs_closedir------\n");
    printf("--------------\n");
    return 0; 

}


static void lookup(const char *arg)
{
    DIR *dirp;
    struct dirent *dp;


    if ((dirp = opendir(".")) == NULL) {
        perror("couldn't open '.'");
        return;
    }


    do {
        errno = 0;
        if ((dp = readdir(dirp)) != NULL) {
            if (strcmp(dp->d_name, arg) != 0)
                continue;


            (void) printf("found %s\n", arg);
            (void) closedir(dirp);
                return;


        }
    } while (dp != NULL);


    if (errno != 0)
        perror("error reading directory");
    else
        (void) printf("failed to find %s\n", arg);
    (void) closedir(dirp);
    return;
}


int main(int argc, char *argv[])
{
    int i;
    for (i = 1; i < argc; i++)
        lookup(argv[i]);
    return (0);
}

/* 
DESCRIPTION 

       The opendir() function opens a directory stream corresponding to the
       directory name, and returns a pointer to the directory stream.  The
       stream is positioned at the first entry in the directory.

       The fdopendir() function is like opendir(), but returns a directory
       stream for the directory referred to by the open file descriptor fd.
       After a successful call to fdopendir(), fd is used internally by the
       implementation, and should not otherwise be used by the application.

RETURN VALUE

       The opendir() and fdopendir() functions return a pointer to the
       directory stream.  On error, NULL is returned, and errno is set
       appropriately.
 */