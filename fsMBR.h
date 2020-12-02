/**************************************************************
 * Class:  CSC-415
 * Name: Jason Avina
 * Student ID: N/A
 * Project: Basic File System
 *
 * File: fsMBR.h
 *
 * Description: This is the master boot record aka superblock
 * initialization header file
 **************************************************************/

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
#include "fsLow.h"



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



//only new file i've created in the fsMBR.c file

int beginFSInit(char * filename, uint64_t * volSize, uint64_t * blockSize, SuperBlock *sbPtr,  Bitvector * bitmap_vec);
int initSuperBlock(char * filename, uint64_t * volSize, uint64_t * blockSize);
#endif
