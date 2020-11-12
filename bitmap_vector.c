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
#include "bitmap_vector.h"
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
