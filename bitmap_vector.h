/**************************************************************
 * Class:  CSC-415
 * Name: Rigo Perez
 * Student ID: N/A
 * Project: Basic File System
 *
 * File: bitmap_vector.h
 *
 * Description: This is a demo to show how to use the fsLow
 * 	routines.
 *
 **************************************************************/

#ifndef BITMAP_VECTOR_H
#define BITMAP_VECTOR_H
#define VEC_SIZE "Vector Size (Number of elements): "
#define FREE_SECTORS "Available sectors: "
#define VEC_LOCATION "Location in LBA: "
#define	SELF_LOCATION_ID 2

typedef struct Bitvector{
	char* bits; // 8 bytes
	int size;   // 4 bytes
	int index;  // 4 bytes
	int LOCATION_ID;
	unsigned int avalible_blocks; // 4 bytes
}Bitvector;
/*
 *
 */
Bitvector* create_bitvec(u_int64_t n_bits, u_int64_t blockSize);
int get_num_free_blocks(Bitvector *vec);
void set_bit(Bitvector* vector, int index, int bit);
int get_bit(Bitvector* vec, int index);
int get_vector_size(Bitvector* vec);
int *get_free_blocks_index(Bitvector* vecctor, int *array, int requested_blk);
void free_bitvector(Bitvector *vec);
int get_vec_m_data_addrs(Bitvector *vec);
#endif
