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
#include "lba.h"
#include "utils/date.h"
#include "utils/linked_list.h"
#define B_CHUNK_SIZE 512
void write_this_to_buffer();

SuperBlock *sbPtr; //
Bitvector *bitmap_vec;  //= malloc(blockSize);

char *filename;
/*
 *
 */
int main (int argc, char *argv[]){

	uint64_t blockSize;
	uint64_t volumeSize;
	filename = "simple_fs";
	volumeSize = 1048576;
	blockSize = 512;

	sbPtr = malloc(blockSize);
	bitmap_vec = create_bitvec(volumeSize, blockSize);		// Initializing bitmap vector
	int retVal =  initSuperBlock(filename, &volumeSize, &blockSize, sbPtr, bitmap_vec); 	// Mounts volume and formats File System

		// CALL "write_this_to_buffer();" WHEN FILE SYSTEM IS MOUNTED FOR THE VERY FIRST TIME, then add a comment
	//write_this_to_buffer();

		printf("\nLBA[0T0]: retVal: %d\n\n\n\n", retVal);

	char *file01= (char*)"file01.txt";
	int read_bytes = 0;
	int flagRead = 0;
	int fd = b_open(file01, flagRead);
	char * buffer = malloc(B_CHUNK_SIZE+1);
	if(buffer == NULL ){
		printf(MALLOC_ERR_MESS);
		return -1;
	}
	printf("\n-> TEST[01] ORIGINAL FILE [ %d ] PRINTS OUT SEGMENTS OF 512 BYTES : fd = %s \n\n", fd, file01);
	//printf("\n\n\n: TEST[02] INSERTS STRING TO THE BEGINNING OF FILE in mode 0: PRINTS OUT SEGMENTS OF 512 BYTES :\n");
	do {
		read_bytes = b_read (fd, buffer, 512);//b_read (3, (char * buffer = malloc(80), 80);
		buffer[read_bytes] = '\0';
		if(read_bytes == 0)break;
		printf("BTS: %d, OUTPUT: %s\n", read_bytes, buffer);
	} while (read_bytes > 0);

	free(buffer);
	buffer = NULL;

	
	
	
	
      //  print_dir();
	//dir_offload_configs();
	
	
	free(sbPtr);
	sbPtr = NULL;

	free(bitmap_vec);
	bitmap_vec = NULL;




	// Fix bugs


	printf("\n\n\nFINISH TESTING B_IMPUT_OUTPUT INTERFACE %d\n", 100);
	printf(" EXIT WITH SUCCESS:::MESSAGE-%d::: \n\n\n", 10);
	closePartitionSystem();
	b_close(fd);
	
	
	return 0;	
}

/*
 * ===========================================================================================
 * ========================== Routine to test b_open =========================================
 * ===========================================================================================
 */

void write_this_to_buffer(){
	int requested_blk = 10;
	int f_barray[requested_blk]; //
	int * barray = get_free_blocks_index(bitmap_vec, f_barray, requested_blk);
	printf("%d %d %d %d %d %d %d %d \n\n", barray[0], barray[1], barray[3], barray[4], barray[4], barray[5], barray[6], barray[7]);
	char* file_metadata = malloc(B_CHUNK_SIZE);
	char* date = malloc(B_CHUNK_SIZE/4);
	char* time_ = malloc(B_CHUNK_SIZE/4);
	getDate(date);
	getTime(time_);

	strcpy(file_metadata,  VEC_LOCATION); // VEC_LOCATION is a predefined string of char
	sprintf(file_metadata +  strlen(file_metadata), "%d\n", 100); // location
	strcpy(file_metadata + strlen(file_metadata),  "FILE_SIZE: "); // FILE SIZE is a predefined string of char
	sprintf(file_metadata +  strlen(file_metadata), "%d\n", 2417);
	strcpy(file_metadata + strlen(file_metadata),  "FLAG: "); // FLAGS
	sprintf(file_metadata +  strlen(file_metadata), "%d\n", O_RDONLY);

	strcpy(file_metadata+ strlen(file_metadata),"FILE_BLKS_NUM: "); // FILE SIZE is a predefined string of char
	sprintf(file_metadata +  strlen(file_metadata), "%d\n", 6);

	strcpy(file_metadata+ strlen(file_metadata),"FILE_NAME: f_test01.txt\n"); // FILE SIZE is a predefined string of char
	strcpy(file_metadata + strlen(file_metadata),  "DATE_CREATED: "); // date
	strcpy(file_metadata + strlen(file_metadata),  date); // date
	strcpy(file_metadata + strlen(file_metadata),  "\nTIME: "); // date
	strcpy(file_metadata + strlen(file_metadata),  time_); // date
	strcpy(file_metadata + strlen(file_metadata),  "\n\n"); // date

	LBAwrite (file_metadata, 1, 100);  						// Writes Bitmap-vector meta-data to LBA[2]
	char * buffer = malloc(B_CHUNK_SIZE);
	char str01 [] =	"The House passed the DREAM Act in 2019, but the bill went nowhere in a GOP-controlled Senate. If Democrats had been swept into the Senate majority, immigration reform was expected to be on their to-do list as the party considered ending the legislative filibuster. Now, even if Democrats are able to force a 50-50 tie by flipping two Georgia seats in runoff elections in January, they would be well short of the 60 votes needed to pass a deal. The uphill battle in Congress has immigration reform advocates urgin";
	char str02[] = "g Biden to make changes to the system through executive action, including rolling back Trump orders. Biden is expected to quickly revive the DACA program, end the Trump administration’s so-called Muslim ban and end construction on the U.S.-Mexico border wall. He is reportedly eyeing a freeze on deportations to give his administration time to issue new guidance for immigration agents. Biden also announced late last week that he would dramatically increase the refugee cap. Ron Klain, Biden’s incoming chie";
	char str03 [] = "f of staff, reiterated that addressing children brought into the country illegally as children would be one of the first actions taken by a Biden administration, saying it would be an action taken care of on Day One. Now, it seems, that era is coming to an end, raising a critical question: How much of what Trump built will remain? If Democrats had been swept into the Senate majority, immigration reform was expected to be on their to-do list as the party considered ending the legislative filibuster. Now, eve ";
	char str04 [] = "n if Democrats are able to force a 50-50 tie by flipping two Georgia seats in runoff elections in January, they would be well short of the 60 votes needed to pass a deal. Biden has not said whether he would allow individuals in the program into the U.S. As of September, roughly 24,500 people remained in the program, while fewer than 1 percent have been granted asylum. Talk of a potential agreement under Biden comes as Congress has tried and failed in recent years to clinch a deal related to the estimated 11";
	char str05 [] = " million undocumented immigrants in the United States. Some of the challenges would be logistical. Biden’s refugee resettlement plan, for example, has to contend with the fact that after nearly four years of the Trump administration strangling refugee admissions, the capacity of the existing nonprofit infrastructure to take in refugees has been severely diminished.";
	memcpy(buffer, str01, B_CHUNK_SIZE);
	LBAwrite (buffer, 1, 101);
	//printf("%s\n", buffer);
	memcpy(buffer, str02, B_CHUNK_SIZE);
	LBAwrite (buffer, 1, 102);
	//printf("%s\n", buffer);
	memcpy(buffer, str03, B_CHUNK_SIZE);
	LBAwrite (buffer, 1, 103);
	//printf("%s\n", buffer);
	memcpy(buffer, str04, B_CHUNK_SIZE);
	LBAwrite (buffer, 1, 104);
	//printf("%s\n", buffer);
	memcpy(buffer, str05, 395);
	LBAwrite (buffer, 1, 105);
	//printf("%s\n", buffer);
	//printf("\n%s\n\n\n", file_metadata);
	free(file_metadata);
	file_metadata = NULL;
	free(date);
	date = NULL;
	free(time_);
	time_ = NULL;
	int x = 100;
	char * buf3 = malloc(513);
	memset (buf3, 0, 512);
	LBAread (buf3, 1, x);
	//printf("\n\nLBA[%d]:\n\n  %s\n", x, buf3 );
	free(buf3);
	buf3 = NULL;
}

	
