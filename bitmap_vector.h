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

#ifndef BITMAP_VECTOR_H
#define BITMAP_VECTOR_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>


typedef struct Bitvector{
	char* bits; // 8 bytes
	int size;   // 4 bytes
	int index;  // 4 bytes
	unsigned int avalible_blocks; // 4 bytes
}Bitvector;
/*
 *
 */
Bitvector* create_bitvec(int n_bits, uint64_t blockSize);
int get_num_free_blocks(Bitvector *vec);
void set_bit(Bitvector* vector, int index, int bit);
int get_bit(Bitvector* vec, int index);
int get_vector_size(Bitvector* vec);
int *get_free_blocks_index(Bitvector* vecctor,  int size, int *array, int requested_blk);
void free_bitvector(Bitvector *vec);
#endif
