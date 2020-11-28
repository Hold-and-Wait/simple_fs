/**************************************************************
 * Class:  CSC-415
 * Name: Rigo Perez
 * Student ID: N/A
 * Project: Basic File System
 *
 * File: bitmap_vector.c
 *
 *
 **************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include "bitmap_vector.h"

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif

int NUM_OF_FREE_BLOCKS;
int NUM_OF_BLOCKS_IN_VOLUME;





/*
 * Allocates a pointer of type Bitvector with size = Bitvector
 * Allocates a pointer of type char with size = 2^n
 * Returns a pointer of type Bitvector
 */
Bitvector* create_bitvec(uint64_t n_bits, uint64_t blockSize) {

	Bitvector* vector = (Bitvector*)malloc(sizeof(Bitvector));
	vector->avalible_blocks = 0;
	int size = sizeof(Bitvector) * ceil(n_bits/8.0); 		// size in bytes, ceil(n_bits/8.0) guarantees a whole number
	vector->bits = (char*)malloc(size);						// Array of chars
	vector->size = n_bits / blockSize;						// Vector size
	return vector;
}






void initBitmapVector(uint64_t *volumesize, uint64_t *blocksize, Bitvector *bitmap_vec) {

	u_int64_t blockSize = blocksize;
	u_int64_t volSize = volumesize;
	
	int Size;
	Size = bitmap_vec->size;
	printf("Here is the size of the bitmap_vec: %u\n", Size);
//HERE IS WHERE THE CODE BREAKS - WHY? BECAUSE ITS OVERWRITIGN THE BITMAP
	for (int bit_pos = 0; bit_pos < bitmap_vec->size; bit_pos++) {
		set_bit(bitmap_vec, bit_pos, 0);
	}

	for(int bit_pos = 0; bit_pos < 100; bit_pos++) 				// Reserve First 100 Blocks
		set_bit(bitmap_vec, bit_pos, 1);

	
	char bit_str_array [blockSize]; 					// Needed to convert bits into chars block-size = 512 bytes
	for (int var = 0; var < sizeof(bit_str_array); var++) 			// Initialize char bit_str_array
		bit_str_array [var] = -1;

	int count = 0;
										// bitmap_vec->LOCATION_ID = 2
	int block_pos = get_vec_m_data_addrs(bitmap_vec)+1; 			// Bits collection start at LBA[3] with length 4
	for (int loop_counter = 0; loop_counter <= bitmap_vec->size; loop_counter++){
		if(count == blockSize){
			sprintf(&bit_str_array[count], "%c", '\0' );
			char* temp_buff = malloc(strlen(bit_str_array));
			strcpy(temp_buff,  bit_str_array);
			//printf("LBA[%d]: %s\n", block_pos, temp_buff);
			LBAwrite (temp_buff, 1, block_pos);			//LBAwrite (buff, bloks_num, block_position); buff hold data, bloks_num is the number of blocks, block_position is the starting position block
			block_pos++;
			memset (bit_str_array, 0, sizeof(temp_buff));
			count = 0;
			free(temp_buff);
			temp_buff = NULL;
		}
		bit_str_array[count] = get_bit(bitmap_vec, loop_counter) + '0'; // Appends bit to a string
		count++;
	}

	char* temp_buf = malloc(blockSize);
	strcpy(temp_buf,  VEC_LOCATION); // VEC_LOCATION is a predefined string of char
	sprintf(temp_buf +  strlen(temp_buf), "%d\n", get_vec_m_data_addrs(bitmap_vec));
	strcpy(temp_buf + strlen(temp_buf),  VEC_SIZE); // VEC_SIZE is a predefined string of char
	sprintf(temp_buf +  strlen(temp_buf), "%d\n", get_vector_size(bitmap_vec));
	strcpy(temp_buf + strlen(temp_buf),  FREE_SECTORS); // FREE_SECTORS is a predefined string of char
	sprintf(temp_buf +  strlen(temp_buf), "%d\n", get_num_free_blocks(bitmap_vec));

	LBAwrite (temp_buf, 1, get_vec_m_data_addrs(bitmap_vec));  		// Writes Bitmap-vector meta-data to LBA[2]
	free(temp_buf);
	temp_buf = NULL;


	//free_bitvector(bitmap_vec); ====> *** IMPORTANT *** remove comment after full debugging

	NUM_OF_FREE_BLOCKS = get_num_free_blocks(bitmap_vec);			// Free Blocks Record
	NUM_OF_BLOCKS_IN_VOLUME = get_vector_size(bitmap_vec);			// Total number of Blocks in Volume

}



/*
 * @parameter pointer of type Bitvector
 * returns number of free blocks
 */
int get_num_free_blocks(Bitvector *vec){
	return vec->avalible_blocks;
}
/*
 *@parameter 1. pointer of type Bitvector
 *@parameter 2. index is a position in the list
 *@parameter 3. bit: either 0 | 1
 * Include source reference
 */
void set_bit(Bitvector* vector, int index, int bit) {
	if(bit == 0) vector->avalible_blocks ++;
	if(bit == 1) vector->avalible_blocks--;
	vector->index = index;
	int byte = index >> 3; 						// == index / 2^3
	int n = sizeof(index)*8-3;
	int offset = ((unsigned) index << n) >> n;
	if (bit) {
		vector->bits[byte] |= 1 << (7-offset);
	} else {
		vector->bits[byte] &= ~(1 << (7-offset));

	}
}
/*
 * Takes Bitvector pointer & index/position
 * Returns 1 when sector is not available
 * Returns 0 when sector is available
 */
int get_bit(Bitvector* vec, int index) {
	int byte = index >> 3;
	int n = sizeof(index)*8-3;
	int offset = ((unsigned) index << n) >> n;
	if (vec->bits[byte] & (1 << (7- offset))) {
		return 1;
	} else {
		return 0;
	}
}
/*
 * Include checks
 * @parameter 1. vector is a pointer of type Bitvector
 * @parameter 2. array is a pointer to an array of integers of size N
 * @parameter 3. requested_blk is an integer which value represents
 * a set of requested unused sectors
 * Checks for valid entries
 * Returns an array pointer which values represent the position of free sectors
 *
 *  example:
 *
 *  int array[N]; Where N is the number of free requested blocks
 *
 *  int * ptr = get_free_blocks_index(bitmap_vec, array, N);
 *
 *  Retriving ptr values:
 *
 *  int X = ptr[i]; ...
 *
 */
int *get_free_blocks_index(Bitvector *vector, int *array, int requested_blk){
	if(requested_blk > vector->size || requested_blk < 1)return NULL;

	int avble_blk_ctr = 0;
	int arr_index = 0;
		// array size = requested_blk
	for (int i = 0; i < vector->size; i++) {
		if( get_bit(vector, i) == 0){ 			// Finds the first 0
			for ( int bit_position = i; bit_position < (i + requested_blk); ++bit_position) { // look for an X contiguous blocks
				if( get_bit(vector, bit_position) == 0) {		// If bit == 0 increment avble_blk_ctr, increment arr_index
					array[arr_index] = bit_position;
					arr_index++;
					avble_blk_ctr++;
					//printf("Free Block at positon LBA[%d] =  \n",  array[i]);

				}

				if(get_bit(vector, bit_position) == 1){		// If not contiguous blocks found, reset variables and start a new search
					i = bit_position;
					arr_index = 0;
					avble_blk_ctr = 0;
					break;
				}
			}
			if(avble_blk_ctr == requested_blk) break;
		}
	}
	return array;
}

/*
 * Returns the size of vector
 */
int get_vector_size(Bitvector* vec){
	return vec->size;
}
/*
 * @parameter pointer to Bitvector
 *  releases allocated memory
 */
void free_bitvector(Bitvector *vec) {
	free(vec->bits);
	vec->bits = NULL;
	free(vec);
	vec = NULL;
}
/*
 * Returns position of Bitvector meta-data in the LBA
 */
int get_vec_m_data_addrs(Bitvector *vec){
	vec->LOCATION_ID = SELF_LOCATION_ID;
	return vec->LOCATION_ID;
}