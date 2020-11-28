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
#include "fsLow.h"
#include "fsMBR.h"
#include "bitmap_vector.h"


int main (int argc, char *argv[])
{

	char * fileName;
	uint64_t volumeSize;
	uint64_t blockSize;
    int retVal;
    
	if (argc > 3)
		{
		fileName = argv[1];
		volumeSize = atoll (argv[2]);
		blockSize = atoll (argv[3]);
		}
	else
		{
		printf ("Usage: fsLowDriver volumeFileName volumeSize blockSize\n");
		return -1;
		}
	
	
	
	
	
	
	
	//CREATING THE SUPERBLOCK
	struct Block* sbPtr; 			//creating a pointer to superblock struct
	retVal = initSuperBlock(fileName, volumeSize, blockSize, sbPtr);
	printf("Opened %s, Volume Size: %llu;  BlockSize: %llu; Return %d\n", fileName, (ull_t)volumeSize, (ull_t)blockSize, retVal);
	
	//CHECKING TO SEE IF WE CAN READ AND WRITE TO THE SUPERBLOCK
	char * buf = malloc(blockSize *2);
	char * buf2 = malloc(blockSize *2);
	memset (buf, 0, blockSize*2);
	strcpy (buf, "Now is the time for all good people to come to the aid of their countrymen\n");
	strcpy (&buf[blockSize+10], "Four score and seven years ago our fathers brought forth onto this continent a new nation\n");
	LBAwrite (buf, 2, 0);
	LBAwrite (buf, 2, 3);
	LBAread (buf2, 2, 0);
	if (memcmp(buf, buf2, blockSize*2)==0)
		{
		printf("Read/Write worked\n");
		}
	else
	{
		printf("FAILURE on Write/Read\n");
	}
	if (retVal < 0) 
	{
		printf("Error: you did not create the volume");
		return -1;
	}

	free (buf);
	free(buf2);
	
	
	
	
	
	
	//CREATING THE BITVECTOR
	Bitvector* bitmap_vec  = create_bitvec(volumeSize, blockSize);
	
	//INITIALIZING THE BITVECTOR
	initBitmapVector(volumeSize, blockSize, bitmap_vec);
	
	//TESTING BITMAP
	printf("\nLBA[1]:\n");
	printf("SUPERBLOCK STATS:\n");

	printf("File Type (Magic NUmber): %s\n",   "Undefined");
	printf("Volume Size: %d BYTES\n",  sbPtr->VOL_SIZE_IN_BYTES);
	printf("MBR LBA Index Position: %d\n", 	sbPtr->MBR_LOCATION_IN_VOL);
	printf("Free Space Manager Index Position: %d, Length: %d\n", 2, 5);
	printf("Volume Size (in sector units): %d\n", sbPtr->VOL_SIZE_IN_SECTORS);
	printf("Available Free Space (in sector units): %d\n", sbPtr->AVAILABLE_BLOCKS);
	printf("Directory Manager Index Position: %d, Length: %s\n", 8, "Undefined");
	printf("Date Created: %s, Time: %s\n", sbPtr->DATE_ACCESSED, sbPtr->TIME_ACCESSED);

	printf("\nLBA[2]:");
	printf("\nBITVECTOR STATS:\n");
	printf("Vector Size: %d blocks. \n", get_vector_size(bitmap_vec));
	printf("Free space in blocks unit: %d blocks.\n\n", get_num_free_blocks(bitmap_vec));


	// TESTING LBA
	char * buf3 = malloc(513);
	memset (buf3, 0, blockSize);
	LBAread (buf3, 1, 0);
	printf("\nLBA[3]: %d,  buf3* s = %s\n\n", (int)strlen(buf3), buf3 ); // Prints content located at LBA[0], remember this is juts a test
									     // LBA[0] will be used for the boot block

	// TEST DIR
    	//initializeDirectory(bitmap_vec, 7);

    	//print_dir();

    	//fs_mkdir("test1", 1, DT_DIR);
    	//b_open("dir3", O_CREAT | O_RDONLY);

    	//offload_configs();
    	// END TEST

    free(buf3);
	buf3 = NULL;
	closePartitionSystem();
	
	

	closePartitionSystem();

	return 0;	
}
	
	