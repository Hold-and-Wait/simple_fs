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
#include "bitmap_vector.h"
#include "fsMBR.h"
#include "b_io.h"
#include "LBA/lba.h"

char *filename;
/*
 *
 */
int main (int argc, char *argv[]){

	/*
			if (argc > 3){
		filename = argv[1];
		volumeSize = atoll (argv[2]);
		blockSize = atoll (argv[3]);
	else {
		printf ("Usage: fsLowDriver volumeFileName volumeSize blockSize\n");
		return -1;
		}
			}
	 */


	// Used temprary

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

    print_dir();
    offload_configs();

    free(buf3);
	buf3 = NULL;
	closePartitionSystem();

	return 0;	
}

	
