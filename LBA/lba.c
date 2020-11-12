#include "lba.h"
#include "DirectoryEntries.c"

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

struct fs_diriteminfo *fs_readdir(fdDir *dirp){
    
}

int fs_closedir(fdDir *dirp){
    printf("----fs_closedir------\n");
    printf("--------------\n");
    return 0; 

}
