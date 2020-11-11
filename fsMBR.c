/**************************************************************
 * Class:  CSC-415
 * Name: Jason Avina
 * Student ID: N/A
 * Project: Basic File System
 *
 * File: fsMBR.c
 * Description: This is the master boot record initialization
 * routine that also initializes the free space manager and
 * directory entries
 *
 **************************************************************

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include "fsLow.h"
#include "fsMBR.h"
#include "bitmap_vector.h"
#include "date.h"

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

int NUM_OF_FREE_BLOCKS;
int NUM_OF_BLOCKS_IN_VOLUME;



void initializeBitmapVecotr(uint64_t * volSize, uint64_t * blockSize, Bitvector * bitmap_vec); // Initialize Free Space Manager


//all functions should follow this pattern where they return 0 if success
//and -1 if fails until we have checked every aspect of the file system
//and once everything passes, then we can start reading and writing to the file
//system
int initSuperBlock(char * filename, uint64_t * volSize, uint64_t * blockSize, struct SuperBlock *sbPtr,  Bitvector * bitmap_vec){
	int startVal;
	startVal = startPartitionSystem(filename, volSize, blockSize);
	printf("here is the startVal: %d\n", startVal);
	if (startVal < 0){
		printf("Error: you did not create the volume");
		return -1;
	}



	initializeBitmapVecotr(volSize, blockSize, bitmap_vec); 		// Initialize Free Space Manager


	sbPtr->blockSize = *blockSize;
	sbPtr->VOL_SIZE_IN_BYTES = *volSize;
	sbPtr->AVAILABLE_BLOCKS = NUM_OF_FREE_BLOCKS;
	sbPtr->MBR_LOCATION_IN_VOL = 1;
	sbPtr->startBlockOfFreeSpace = 2;
	sbPtr->VOL_SIZE_IN_SECTORS = NUM_OF_BLOCKS_IN_VOLUME;
	sbPtr->DATE_ACCESSED = malloc((*blockSize)/2); // takes 64 bytes
	sbPtr->TIME_ACCESSED = malloc((*blockSize)/2); // takes 64 bytes
	getDate(sbPtr->DATE_ACCESSED);
	getTime(sbPtr->TIME_ACCESSED);



	return startVal;
}



/*
 * Initializes bitmap vector
 * Writes bytes to hard drive
 *
 */

void initializeBitmapVecotr(uint64_t * volumeSize, uint64_t * blocksize, struct Bitvector * bitmap_vec){
	int blockSize = *blocksize;
	int volSize = *volumeSize;


	bitmap_vec = create_bitvec(volSize, blockSize);		// Initializing bitmap vector


	int vect_size = get_vector_size(bitmap_vec);					// gets number of blocks
	for (int bit_pos = 0; bit_pos < vect_size; bit_pos++)  			// initialize vector with bit values of 0
		set_bit(bitmap_vec, bit_pos, 0);
	for(int bit_pos = 0; bit_pos < 100; bit_pos++) 					// Reserve First 100 Blocks
		set_bit(bitmap_vec, bit_pos, 1);


	char bit_str_array [blockSize]; 								// Needed to convert bits into chars block-size = 512 bytes

	for (int var = 0; var < sizeof(bit_str_array); var++) 			// Initialize char bit_str_array
		bit_str_array [var] = -1;



	int count = 0;
	int block_pos = 0;
	for (int loop_counter = 0; loop_counter <= vect_size; loop_counter++){

		if(count == blockSize){
			sprintf(&bit_str_array[count], "%c", '\0' );
			char* temp_buff = malloc(strlen(bit_str_array));
			strcpy(temp_buff,  bit_str_array);
			LBAwrite (temp_buff, 1, block_pos);		//LBAwrite (buff, bloks_num, block_position); buff hold data, bloks_num is the number of blocks, block_position is the starting position block
			block_pos++;
			memset (bit_str_array, 0, sizeof(temp_buff));
			count = 0;
			free(temp_buff);
			temp_buff = NULL;
		}

		bit_str_array[count] = get_bit(bitmap_vec, loop_counter) + '0'; 			// Appends bit to a string
		count++;


	}


	NUM_OF_FREE_BLOCKS = get_num_free_blocks(bitmap_vec);							// Free Blocks Record
	NUM_OF_BLOCKS_IN_VOLUME = get_vector_size(bitmap_vec);							// Total number of Blocks in Volume




}


//LBAread(mbrPtr, 1, 0); //this is checking if its already initialized
void testMBRFunction() {
	printf("MBR Test\n");
}

