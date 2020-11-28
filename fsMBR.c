#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "fsMBR.h"
#include "fsLow.h"
#include "bitmap_vector.h"
#include "date.h"


#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif



#define VOL_BLK 2048 // = VOL/BLOCK num of blocks
#define START_BLK_FREE 1
#define START_BLK_ROOT 2
#define MAGICNUM 0xabba



Bitvector* bvPtr;


//all functions should follow this pattern where they return 0 if success
//and -1 if fails until we have checked every aspect of the file system
//and once everything passes, then we can start reading and writing to the file
//system
int initSuperBlock(char * fileName, uint64_t volSize, uint64_t blockSize, SuperBlock *sbPtr)
{
	int startVal;
	startVal = startPartitionSystem("SampleVolume", &volSize, &blockSize);
	printf("Here is the startVal from startPartitionSystem: %d\n", startVal);
	

	sbPtr = malloc(blockSize);		//pointer points to 512 bytes of free space
	sbPtr->sbBlockSize = blockSize; //

	LBAread(sbPtr, 1, 0); 

	if (sbPtr->magicNum != MAGICNUM) 
	{
//new volume to be initialized 
//initiize sbptr and its variables
		sbPtr->sbBlockSize = blockSize;
		sbPtr->nBlocks = (unsigned int)volSize/(unsigned int)blockSize;
		sbPtr->magicNum = MAGICNUM;
	
//write this to the LBA
		LBAwrite(sbPtr, 1, 0);
	}
//volume is set and ready to go

	return 0;
}









