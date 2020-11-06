#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include "fsLow.h"
#include "fsMBR.h"


#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif



#define VOL_BLK 2048 // = VOL/BLOCK num of blocks
#define START_BLK_FREE = 1;
#define START_BLK_ROOT = 2;
#define MAGICNUM = 0xabra;


struct Block {

	int blockSize;
	int numOfBlocks;
	int startBlockOfFreeSpace;
	int startBlockOfRootDir;
	int magicNum;

} SuperBlock;




//all functions should follow this pattern where they return 0 if success
//and -1 if fails until we have checked every aspect of the file system
//and once everything passes, then we can start reading and writing to the file
//system
int initSuperBlock() 
{
	uint64_t volSize;
	uint64_t blockSize;
	int startVal;
	
	startVal = startPartitionSystem("SampleVolume", &volSize, &blockSize);
	printf("here is the startVal: %d\n", startVal);
	if (startVal < 0) 
	{
		printf("Error: you did not create the volume");
		return -1;
	}
	
	struct Block* sbPtr; 
	sbPtr = malloc(blockSize);
	sbPtr->blockSize = blockSize;
}





//LBAread(mbrPtr, 1, 0); //this is checking if its already initialized


void testMBRFunction() {
    printf("MBR Test\n");
}




