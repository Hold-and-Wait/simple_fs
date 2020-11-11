/**************************************************************
 * Class:  CSC-415
 * Name: Rigo Perez
 * Student ID: N/A
 * Project: Basic File System
 *
 * File: fsLowDriver.c
 *
 * Description: This is a demo to show how to use the fsLow
 * 	routines.
 *
 **************************************************************/
#include "bitmap_vector.h"
/*
 *
 */
Bitvector* create_bitvec(int n_bits, uint64_t blockSize) {


	//printf("SIzeOfBitVector = %d", (int)sizeof(bitvector));


	Bitvector* vector = (Bitvector*)malloc(sizeof(Bitvector));
	vector->avalible_blocks = 0;
	int size = sizeof(Bitvector) * ceil(n_bits/8.0); 		// size in bytes
	vector->size = n_bits / blockSize;
	vector->bits = (char*)malloc(size);
	return vector;
}
/*
 *
 */
int get_num_free_blocks(Bitvector *vec){
	return vec->avalible_blocks;
}
/*
 *
 */
void free_bitvector(Bitvector *vec) {
	free(vec->bits);
	free(vec);
}
/*
 *
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
 *
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
 *
 */
int *get_free_blocks_index(Bitvector* vecctor,  int vec_size, int *array, int requested_blk){
	int avble_blk_ctr = 0;
	int arr_index = 0;

		// array size = requested_blk
	for (int i = 0; i < vec_size; i++) {
		if( get_bit(vecctor, i) == 0){ 			// Finds the first 0
			for ( int bit_position = i; bit_position < (i + requested_blk); ++bit_position) { // look for an X contiguous blocks
				if( get_bit(vecctor, bit_position) == 0) {		// If bit == 0 increment avble_blk_ctr, increment arr_index
					array[arr_index] = bit_position;
					arr_index++;
					avble_blk_ctr++;
				}
				if(get_bit(vecctor, bit_position) == 1){		// If not contiguous blocks found, reset variables and start a new search
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
