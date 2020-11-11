#ifndef FS_MBR_H
#define FS_MBR_H
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "bitmap_vector.h"


typedef struct SuperBlock {
	int startBlockOfFreeSpace;
	int startBlockOfRootDir;
	int magicNum;
	int MBR_LOCATION_IN_VOL;
	int AVAILABLE_BLOCKS;
	int VOL_SIZE_IN_BYTES;
	int VOL_SIZE_IN_SECTORS; 	// Sectors = Blocks = 512 bytes each
	int blockSize; 				// 512 Each
    char *DATE_ACCESSED;
    char *TIME_ACCESSED;

} SuperBlock;


//This initializes the super block and if it doesn't exist, creates it
int initSuperBlock(char * filename, uint64_t * volSize, uint64_t * blockSize, SuperBlock *sbPtr,  Bitvector * bitmap_vec);

#endif
