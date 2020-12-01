/**************************************************************
 * Class:  CSC-415-1# FALL 2020
 * Name: Rigoberto Perez
 * Student ID: 902426361
 * GitHub UserID: Rig06
 * Project: File System Group Project
 *
 * File: b_io.c
 *
 * Description: The application opens, creates, reads, and writes
 * data files
 *
 *
 **************************************************************/
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "lba.h"
#include "b_io.h"
#include "fsLow.h"
#include "fsMBR.h"
#include "utils/linked_list.h"
#include "mfs.h"
//^___Libraries_&_Utilities__^

typedef struct file_INFO {
	Node *file_data;	// [ NODE-01 ]-->[ NODE-01 ]-->[ NODE-01 ]--> ...[ NODE - N ]-->[ NULL ] : Attributes in each node:  char*buffer and int size
	int file_descriptor;
	int sector_tracker;
	int unit_sector_qtity;
	char* file_name;
} F_INFO;
#define B_CHUNK_SIZE 512
#define MAX_TABLE_SIZE 20

//v______________________________________v

F_INFO open_files_stack [ MAX_TABLE_SIZE ];	// <=== F_INFO  :  Files stack [FILE-01], [FILE-02], [FILE-03], [FILE-04], [FILE-05], [FILE-06], ...[FILE-19] : indexes will act as file descriptor

//^----------Files-Stack-----------------^

int ACTIVE_FILE = 0;
int IS_STACK_INIT = 0; // zero if the array has not been initialize
int CLIENT_CALL_CNT = 0;
int F_DESCRIPTOR = 0;
int END_OF_READ_FILE = 1;
int READ_BYTES;
char *LOC_BUFFER = NULL;
int REMAINING_BYTES = 0; 		// Tracks offset (usually remainders)
int OFFSET_PTR = 0; 		// Tracks offset
int F_DATA_LBAiNIT_POS = 0;
int LINKEDLIST_LASTNODE = 1;
//^__ Global Variables--^

// ===========================================================================================
//		 Helper Routines prototypes
// ===========================================================================================
int read_long_bytes(b_io_fd f_descriptor, char * buffer, int requested_bytes);
int   reload_buffer(b_io_fd f_descriptor);
void write_to_buffer(char*buffer); // Routine to test b_open
void start_up ();
int reset_local_vars(b_io_fd f_descriptor);
// ===========================================================================================


/*
 * Returns -1 if  file does not exist
 */
int b_open (char * filename, int flags) {

    // .
    // ..
    // ,,
    //
	/*	// check if filename exists and it is of type DT_REG (a file)
	fdDir * directory = fs_opendir(filename);

	if (directory == NULL) { // == NULL means the file either: does not exist OR file is not a text file

		// check for flag to create a file
		if (flags == (O_CREAT | O_WRONLY) ||
				(flags == (O_CREAT | O_RDONLY)) ||
				(flags == (O_CREAT | O_RDWR)) ||
				(flags == O_CREAT)) {

			fs_mkdir(filename, flags, DT_REG);
			directory = fs_opendir(filename);


		} else { // if no O_CREAT flag, and file does not exist, ERROR
			return -1;
		}
	}


	 */


	//====================================Todo provide the following information from directory =================================================================
	F_iNode *test_file = malloc(120);
	if(test_file == NULL ){
		printf(MALLOC_ERR_MESS);
		return -1;
	}
	test_file->file_name = "test_file.txt";
	test_file->length_in_sectors = 5;  			 // including meta-data (iNode) LBA[x]: meta-data LBA[x+1]...[x + 5] : data
	test_file->file_size = 2417;       			 // 2348 bytes of actual data + 512 for meta-data block
	test_file->file_strt_position = 10;			 // LBA [ somewhere in the LBA ]
	test_file->buffer = malloc(2417); 			// For now, let's supposed that this data already exist in our LBA[]
	if(test_file->buffer == NULL ){
		printf(MALLOC_ERR_MESS);
		return -1;
	}
	write_to_buffer(test_file->buffer); //<- Look at the last lines of code (bottom of file) : Populates buffer : test_file->buffer is a single block of data of size X
	//============================================================================================================================================




	if(IS_STACK_INIT == 0) start_up();	//<-- Initialize Files Stack  ---> F_INFO open_files_stack [ MAX_TABLE_SIZE ];


	for (int var = 0; var < MAX_TABLE_SIZE; ++var) { // Finds the first index available  : Initially, all file descriptors are initialized to -1, there are 21 (0-20)
		if (open_files_stack[var].file_descriptor == -1){
			open_files_stack[F_DESCRIPTOR].file_descriptor = var;
			break;
		}
	}


	sector_info sector01;  // Creates a "type struct" with members: char*buffer and int bytes

	for (int var = 0; var < test_file->length_in_sectors; ++var) { // Load file data sectors to a LinkedList  : This data comes from LBA[x] ...LBA[n], when file exist
		sector01.buff_sector = malloc(B_CHUNK_SIZE+1);
		if(sector01.buff_sector == NULL ){
			printf(MALLOC_ERR_MESS);
			return -1;
		}
		memcpy(sector01.buff_sector, test_file->buffer + (var*B_CHUNK_SIZE), B_CHUNK_SIZE);
		sector01.sector_size = strlen(sector01.buff_sector);
		addNode(&open_files_stack[F_DESCRIPTOR].file_data, sector01); // < ===== " &buffer_stack[F_DESCRIPTOR].file_data " is a reference to the Head Node

	}



	//free(sector01.buff_sector);
	//sector01.buff_sector = NULL;

	F_DATA_LBAiNIT_POS = test_file->file_strt_position + 1; // Position of first block of current file's data (excluding meta-data block)
	open_files_stack[F_DESCRIPTOR].unit_sector_qtity = test_file->length_in_sectors;  // "open_files_stack[F_DESCRIPTOR].unit_sector_qtity" -> Stores number of sectors occupied by a file
	return open_files_stack[F_DESCRIPTOR].file_descriptor; // File descriptor is the location (index) of current file in the File Stack
}

/*
 *
 ******* Starts-up  f_iNode buffer_stack[MAX_TABLE_SIZE]
 *
 */
void start_up (){
	for (int var = 0; var < MAX_TABLE_SIZE; ++var) {
		open_files_stack[var].file_data = NULL; // This is a Node pointer
		open_files_stack[var].file_descriptor = -1;
		open_files_stack[var].sector_tracker = 0;
	}
	IS_STACK_INIT = 1; // array has been initialized
}
/*
 * Resets local variables to its initial values
 * Frees up current file descriptor
 */
int reset_local_vars(b_io_fd f_descriptor){
	READ_BYTES = 0;
	REMAINING_BYTES = 0;
	OFFSET_PTR = 0;
	END_OF_READ_FILE = 1;
	open_files_stack[f_descriptor].file_descriptor = -1;
	free(LOC_BUFFER);
	LOC_BUFFER = NULL;
	return 0;
}


/*
 * Reads Nodes from LinkedList
 * Each Node stores 512 bytes of data in a pointer
 * and int, which stores the actual length of the pointer, usuaaly 512 and less than 512 for the last node.
 * Assigns L_BUFFER with the char value from current Node;
 * And returns the the length of the pointer, usuaaly 512 and less than 512 for the last node.
 *
 */
int reload_buffer(b_io_fd f_descriptor){
	// When 0 is assigned to LINKEDLIST_LASTNODE,  this indicates there are not more nodes left to read from a file
	if(LINKEDLIST_LASTNODE == 0){ // end of file reading  :  returns 0
		return 0;
	}

	// Initial call
	Node *current = open_files_stack[f_descriptor].file_data;
	if(open_files_stack[f_descriptor].sector_tracker == 0){
		LOC_BUFFER = current->data.buff_sector;
		open_files_stack[f_descriptor].sector_tracker++;
		return current->data.sector_size; // Should always return 512 bytes whenever a file's data takes more than 1 sector
	}

	// Calls will continue happening until last byte is read
	current = getNthNode(current, open_files_stack[f_descriptor].sector_tracker);
	LOC_BUFFER = current->data.buff_sector;
	open_files_stack[f_descriptor].sector_tracker++;

	if(current->data.sector_size < B_CHUNK_SIZE) LINKEDLIST_LASTNODE = 0;

	return current->data.sector_size;
}	//End of reload_buffer

/*
 *
 */
int b_read (b_io_fd f_descriptor, char * buffer, int requested_bytes){
	int routine_return_val = 0;
	memcpy(buffer, " ", requested_bytes);
	if(END_OF_READ_FILE == -1){ 				// <-- No more bytes left to read
		return reset_local_vars(f_descriptor);
	}

	if(requested_bytes >= B_CHUNK_SIZE) {  // <-- Case for when the requested bytes is larger than or equal to the standard 512 bytes sector
		if(LOC_BUFFER == NULL){
			REMAINING_BYTES = B_CHUNK_SIZE;
			LOC_BUFFER = (char*)malloc(B_CHUNK_SIZE);
			if( LOC_BUFFER == NULL ){
				printf(MALLOC_ERR_MESS);
				return -1;
			}
		}
		routine_return_val = read_long_bytes(f_descriptor, buffer, requested_bytes);
		return routine_return_val;
	}

	if (ACTIVE_FILE == 0){ //   CLIENT INTITAL CALL IN THE CASE WHEN "requested_bytes" IS LESS THAN 512
		LOC_BUFFER = (char*)malloc(requested_bytes);
		if( LOC_BUFFER == NULL ){
			printf(MALLOC_ERR_MESS);
			return -1;
		}
		READ_BYTES = reload_buffer(f_descriptor);    // routine one: READ_BYTES = 512
		REMAINING_BYTES = READ_BYTES % requested_bytes;   // routine one: x_bytes_ = 32
		ACTIVE_FILE = 1;
	}

	if(READ_BYTES > requested_bytes){
		memcpy(buffer, LOC_BUFFER + (requested_bytes*CLIENT_CALL_CNT) + OFFSET_PTR, requested_bytes);
		CLIENT_CALL_CNT++;
		routine_return_val = strlen(buffer);
		READ_BYTES = READ_BYTES - routine_return_val;
	} else if (READ_BYTES < requested_bytes){ //
		memcpy(buffer, LOC_BUFFER + (requested_bytes*CLIENT_CALL_CNT) + OFFSET_PTR, REMAINING_BYTES); // Copy REMAINING_BYTES to the caller's buffer
		int bytes_count = reload_buffer(f_descriptor);
		if(bytes_count == 0){
			END_OF_READ_FILE = -1;
			return REMAINING_BYTES;
		}
		OFFSET_PTR = requested_bytes - REMAINING_BYTES;
		memcpy(buffer + REMAINING_BYTES, LOC_BUFFER, OFFSET_PTR);
		READ_BYTES = bytes_count - OFFSET_PTR;
		REMAINING_BYTES = READ_BYTES % requested_bytes;   // round one: 464 % 80 = 64
		CLIENT_CALL_CNT = 0;
		routine_return_val = strlen(buffer);
	}

	return routine_return_val;
}	// End b_read

/*
 * Reads segments of data that are larger than the standard 512-bytes sectors
 */
int read_long_bytes(b_io_fd f_descriptor, char * buffer, int requested_bytes){
	int read_bytes = 0;
	int returned_val = 0;
	memcpy(buffer, "", requested_bytes);

	if(strlen(LOC_BUFFER) != 0){   //Skips first routine call  : Then executes when there is a remaining number number of bytes in LOC_BUFFER from previous routine call
		memcpy(buffer, LOC_BUFFER + REMAINING_BYTES, OFFSET_PTR);
		OFFSET_PTR = strlen(buffer);
		REMAINING_BYTES = requested_bytes - OFFSET_PTR;
		returned_val = strlen(buffer);
	}


	do {
		read_bytes = reload_buffer(f_descriptor);
		if(read_bytes == 0 ){ //Nothing left to read
			END_OF_READ_FILE = -1;
			return strlen(buffer); // Returns bytes leftover (return quantity should be less than --> requested_bytes
		}
		memcpy(buffer + OFFSET_PTR, LOC_BUFFER, REMAINING_BYTES);
		returned_val = strlen(buffer);
		if(returned_val != requested_bytes){
			OFFSET_PTR = returned_val;
			REMAINING_BYTES = requested_bytes - returned_val;
		}
	} while(returned_val != requested_bytes);
	if(returned_val == B_CHUNK_SIZE)   memcpy(LOC_BUFFER, "", B_CHUNK_SIZE); //Will execute when the number of requested bytes is 512

	return returned_val;
}	// End read_long_bytes

/*
 * ************************************ PENDING
 * Interface to write to LBA()
 */

int b_write (int file_d, char * buffer, int count){


	LBAwrite (buffer, 1, F_DATA_LBAiNIT_POS);
	F_DATA_LBAiNIT_POS++;


	/*	int local_fd = open_files_stack[file_d].file_descriptor;
	//printf(" *** FILE DES: %d", file_d);

	if ((local_fd < 0) || (local_fd >= MAX_TABLE_SIZE)){
		return (-1); 					//invalid file descriptor
	}
	int wrote_bytes =	LBAwrite(buffer, file_d,  count);*/

	return 0;
}








/*
 * ===========================================================================================
 * ========================== Routine to test b_open =========================================
 * ===========================================================================================
 */
void write_to_buffer(char*buffer){
	char str01 [] = "The House passed the DREAM Act in 2019, but the bill went nowhere in a GOP-controlled Senate. If Democrats had been swept into the Senate majority, immigration reform was expected to be on their to-do list as the party considered ending the legislative filibuster. Now, even if Democrats are able to force a 50-50 tie by flipping two Georgia seats in runoff elections in January, they would be well short of the 60 votes needed to pass a deal. ";
	char str02 [] = "The uphill battle in Congress has immigration reform advocates urging Biden to make changes to the system through executive action, including rolling back Trump orders. Biden is expected to quickly revive the DACA program, end the Trump administration’s so-called Muslim ban and end construction on the U.S.-Mexico border wall. He is reportedly eyeing a freeze on deportations to give his administration time to issue new guidance for immigration agents. Biden also announced late last week that he would dramatically increase the refugee cap. ";
	char str03 [] = "Ron Klain, Biden’s incoming chief of staff, reiterated that addressing children brought into the country illegally as children would be one of the first actions taken by a Biden administration, saying it would be an action taken care of on Day One. Now, it seems, that era is coming to an end, raising a critical question: How much of what Trump built will remain? ";
	char str04 [] = "If Democrats had been swept into the Senate majority, immigration reform was expected to be on their to-do list as the party considered ending the legislative filibuster. Now, even if Democrats are able to force a 50-50 tie by flipping two Georgia seats in runoff elections in January, they would be well short of the 60 votes needed to pass a deal. Biden has not said whether he would allow individuals in the program into the U.S. As of September, roughly 24,500 people remained in the program, while fewer than 1 percent have been granted asylum. ";
	char str05 [] = "Talk of a potential agreement under Biden comes as Congress has tried and failed in recent years to clinch a deal related to the estimated 11 million undocumented immigrants in the United States. Some of the challenges would be logistical. Biden’s refugee resettlement plan, for example, has to contend with the fact that after nearly four years of the Trump administration strangling refugee admissions, the capacity of the existing nonprofit infrastructure to take in refugees has been severely diminished.";

	memcpy(buffer, str01, strlen(str01));
	memcpy(buffer + strlen(buffer), str02, strlen(str02));
	memcpy(buffer + strlen(buffer), str03, strlen(str03));
	memcpy(buffer + strlen(buffer), str04, strlen(str04));
	memcpy(buffer + strlen(buffer), str05, strlen(str05));
}



/*





	// TESTING b_read() =================TEST IN MAIN===========================================


	int fd =	b_open("file01.txt", O_CREAT | O_RDONLY);

	char * buffer = malloc(514);
	int read_bytes = 0;

	do {
		read_bytes = b_read (fd, buffer, 513);//b_read (3, (char * buffer = malloc(80), 80);
		buffer[read_bytes] = '\0';
		if(read_bytes == 0)break;
		printf("\n Buffer length: %d	: %s\n", read_bytes, buffer);
		b_write (fd, buffer, read_bytes);

	} while (read_bytes > 0);



	// TESTING LBA() ============================================================

	int x = 14;
	char * buf3 = malloc(513);
	memset (buf3, 0, 512);
	LBAread (buf3, 1, x);
	printf("\n\nLBA[%d]:\n\n  %s\n", x, buf3 );

	free(sbPtr);
	sbPtr = NULL;
	free(buf3);
	buf3 = NULL;






 */



