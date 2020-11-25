/**************************************************************
* Class:  CSC-415
* Name: Professor Bierman
* Student ID: N/A
* Project: Basic File System
*
* File: fsLowDriver.c
*
* Description: This is a demo to show how to use the fsLow
* 	routines.
*
**************************************************************/

#include "LBA/lba.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include "utils/stack.h"
#include "fsMBR.h"
#include "bitmap_vector.h"


char *filename;
/*
 *
 */
int main (int argc, char *argv[]){

	uint64_t blockSize;
	uint64_t volumeSize;
	filename = "simple_fs";
	volumeSize = 1048576;
	blockSize = 512;

	SuperBlock *sbPtr = malloc(blockSize);

	//Bitvector *bitmap_vec  = malloc(blockSize);
   	Bitvector *bitmap_vec  = create_bitvec(volumeSize, blockSize);

	// Init directory entries

	int retVal = 0;
	retVal = initSuperBlock(filename, &volumeSize, &blockSize, sbPtr, bitmap_vec); 	// Mounts volume and formats File System
											// We may pass the directory pointer too so it gets initialize
	//dir_offload_configs();
	return 0;	
}

	
