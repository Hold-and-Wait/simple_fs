
#include "fsLow.h"

#ifndef uint64_t
typedef u_int64_t uint64_t;
#endif
#ifndef uint32_t
typedef u_int32_t uint32_t;
#endif
typedef unsigned long long ull_t;





#include "fsLow.h"
#include "bitmap_vector.h"
#include "date.h"





typedef struct Block {

	uint64_t sbBlockSize;
	int blockSize; 	
	uint64_t nBlocks;
	int MBR_LOCATION_IN_VOL;
	int AVAILABLE_BLOCKS;
	int VOL_SIZE_IN_BYTES;
	int VOL_SIZE_IN_SECTORS; 	// Sectors = Blocks = 512 bytes each
	int startBlockOfFreeSpace;
	int startBlockOfRootDir;
	int magicNum;
	char *DATE_ACCESSED;
    char *TIME_ACCESSED;

} SuperBlock;






int initSuperBlock(char* fileName, u_int64_t volSize, u_int64_t blockSize, SuperBlock *sbPtr);

