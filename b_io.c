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

//**** Libraries_&_Utilities ***********************************
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
char * delimeter = "\"\'“”‘’?:;-,—*($%)![]#/ \t\n\x0A\r";

//__Each file____________
typedef struct file_INFO {
	Node *file_data;	//<=== [ NODE-01 ]-->[ NODE-01 ]-->[ NODE-01 ]--> ...[ NODE - N ]-->[ NULL ] : Attributes in each node:  char*buffer and int size
	int file_descriptor;
	int sector_tracker;
	int sectors_qty;
	int file_size;
	int location;
	int file_selector;
	int _FLAG_; // read = 0, write = 1, READ_WRITE = 3
	char* file_name;
} F_INFO;
Bitvector *bitmap_vec_io;  //= malloc(blockSize);
#define B_CHUNK_SIZE 512
#define MAX_TABLE_SIZE 20

//************************ Files-Stack ********************************************
F_INFO open_files_stack [ MAX_TABLE_SIZE ];	// <=== F_INFO  :  Files stack [FILE-01], [FILE-02], [FILE-03], [FILE-04], [FILE-05], [FILE-06], ...[FILE-19] : indexes will act as file descriptor

//*********************  variables for writing to a file ***************************
int LOCAL_STRG_AVL_SPC = 0, LOC_STORAGE_CURSOR = 0, LOCAL_STRG_LNGTH = 0;
int STATIC_RQSTD_BYTES = 0, WRT_THIS_TO_LOCAL_BUFF = 0, STARTUP_UTILS = 0;
int caller_buff_lftover = 0, caller_bfr_cursor = 0;
sector_info sector_var_x;
int add_leftover_to_list_node = 0;
int null_char = 0;

//*********************  variables b_seek ***************************
Node *temp_list_and_file_cpy = NULL;
Node *temp_list_first_part = NULL;
Node *temp_list_second_part = NULL;

int B_SEEK_LOCK_X = 0;
int B_SEEK_SLCTR_LNGTH = 0;
int B_SEEK_LOCK_MODE03 = 0;

// ******************** Helper Routines prototypes *********************************
int read_long_bytes(b_io_fd f_descriptor, char * buffer, int requested_bytes);
int write_long_bytes(int file_d, char *caller_bfr, int bytes_to_write);
int reload_buffer(b_io_fd f_descriptor);
void start_up ();
void load_bitmap_vector();
int reset_seek_io_vars();
int reset_read_io_vars(b_io_fd f_descriptor);
int reset_write_io_vars(b_io_fd f_descriptor);
void load_to_node_list(int file_d);
int B_WRITE (int file_d, char *callers_buffer, int bytes_to_write);
//**********************************************************************************

//********************* variables for reading from a file *************************
char *LOC_BUFFER_READ = NULL,	*LOC_STRG_BUFF_WR = NULL;
int ACTVE_OPN_FILE = 0, IS_STACK_INIT = 0, END_OF_READ_FILE = 1;
int RQUSTD_BTS_CNTR = 0,  RTURND_BTS_TRCKR = 0, F_DATA_LBAiNIT_POS = 0;
int LINKEDLIST_LASTNODE = 1,  LOC_BUFFER_RMNG_BTS = 0;
int caller_bffrs_CURSOR = 0, WRT_THIS_TO_buffer = 0,  LOCAL_BUF_CURSOR = 0;

/*
 * Load storage manager
 */
void load_bitmap_vector(){
	//************ LOAD SORAGE MANAGER ****************************
	char *iNode_vector = malloc(B_CHUNK_SIZE);
	iNode_vector[B_CHUNK_SIZE] = '\0';
	LBAread(iNode_vector, 1, 2);
	//	printf("\n\n**LBA [2]\n %s\n", iNode_vector);
	//Parse FILE meta-data
	int word_counter = 0;
	char *token;
	token = strtok(iNode_vector, delimeter);
	int i = 0;
	int tempVecSize = 0;
	int bit_mapLoc = 0;
	while (token != NULL) {
		if(word_counter == 1){ // iNode location in LBA
			bit_mapLoc = atoi(token);;
			//printf("\n\n** ndex = %d, bitmap_vec->LOCATION_ID  [%d]\n", var, bitmap_vec->LOCATION_ID);

		}
		if(word_counter == 4){ // FILE SIZE : TOTAL SPACE
			tempVecSize = atoi(token);
			break;
			//printf("\n\n** ndex = %d,  bitmap_vec->size  [%d]\n", var, bitmap_vec->size );
		}
		token = strtok(NULL, delimeter);
		word_counter++;
	}
	bitmap_vec_io = create_bitvec(tempVecSize, B_CHUNK_SIZE);
	bitmap_vec_io->LOCATION_ID = bit_mapLoc;

	//************ LOAD BITMAP VECTOR **************************
	memset(iNode_vector,0 , strlen(iNode_vector));
	LBAread(iNode_vector, 1, 3);
	i = (int)strlen(iNode_vector);
	for (int var = 0; var < i; ++var) {
		int y = (int)iNode_vector[var] - '0';;
		//	printf("index at %d : y = [%d]\n",var, y);
		set_bit(bitmap_vec_io, var, y); // 0 - 99 = 1, 100 - 511 are available
	}
	for (int var = 0; var < i; ++var) {
		int x =	get_bit(bitmap_vec_io, var); // 0 - 99 = 1
		printf("Index %d  ::  x = [%d] \n", var, x);
	}


	free(iNode_vector);
	iNode_vector = NULL;


}


/*
 *
 */
void b_close (int file_d){


	/*
	if(temp_list_and_file_cpy != NULL){
		for (int var = 0; var < get_list_size(temp_list_and_file_cpy); ++var) {
			B_WRITE(file_d, getNthNode(temp_list_and_file_cpy, var)->data.buff_sector, getNthNode(temp_list_and_file_cpy, var)->data.sector_size );

		}

		// Add cases for when b_seek uses mode 1 and 2
	}
	 */

/*	if(open_files_stack[file_d]._FLAG_ == 0 || open_files_stack[file_d]._FLAG_ == 3){ // 0 for reading
		open_files_stack[file_d].file_descriptor = -1;
		reset_read_io_vars(file_d);


	} else*/ if (open_files_stack[file_d]._FLAG_ == 0){

		if(LOC_BUFFER_READ != NULL && (int)strlen(LOC_BUFFER_READ) != 0){
			load_to_node_list(file_d);
		}

		// Find free space *************************************************************
		int requested_blk = get_list_size(open_files_stack[file_d].file_data) + 1;
		int f_barray[requested_blk];
		int * barray = get_free_blocks_index(bitmap_vec_io, f_barray, requested_blk);
		printf("\n\n\n {%d, %d, %d, %d, %d, %d} \n\n\n", barray[0], barray[1], barray[3], barray[4], barray[4], barray[5]);
		// Find free space *************************************************************


		printList(open_files_stack[file_d].file_data);
		open_files_stack[file_d]._FLAG_;
		open_files_stack[file_d].file_name;
		get_list_size(open_files_stack[file_d].file_data); // FILE BLOCKS LENGTH
		get_file_size(open_files_stack[file_d].file_data); // FILE SIZE (BYTES)
		open_files_stack[file_d].location = barray[0];     // LOCATION IN LBA (STARTING POSITION)

		// TOTAL BLOCKS USED BY THIS FILE: get_list_size(open_files_stack[file_d].file_data) + 1;


		fdDir * directory = fs_opendir(open_files_stack[file_d].file_name);


		struct fs_diriteminfo * updated_meta = fs_readdir(directory);
		updated_meta->file_size += open_files_stack[file_d].file_size; // increment file size
		/*
		 * If file LBA location or # blocks occupied needs to  be modified, use  this:
		 * updated_meta->d_reclen = NEW BLOCK LENGTH
		 * directory->directoryStartLocation = NEW START LOCATION
		 * ** MAKE SURE TO NOTIFY BITMAP (FREE OLD BLOCKS AND ALLOCATE NEW BLOCKS)
		 */


	//	dir_modify_meta(directory, updated_meta);







		reset_write_io_vars(file_d);
		free(sector_var_x.buff_sector);
		sector_var_x.buff_sector = NULL;
		open_files_stack[file_d].file_descriptor = -1;
		deleteList(&open_files_stack[file_d].file_data);




	}





//	printf("reset_read_io_vars( %d ) successfully closed \n\n", file_d);





}


















/*
 * Resets local variables to its initial values
 * Frees up current file descriptor
 */
int reset_read_io_vars(b_io_fd f_descriptor){
	F_DATA_LBAiNIT_POS = 0;
	ACTVE_OPN_FILE = 0;
	LOCAL_BUF_CURSOR = 0;
	END_OF_READ_FILE = 1;
	free(LOC_BUFFER_READ);
	LOC_BUFFER_READ = NULL;
	LINKEDLIST_LASTNODE = 1;
	LOC_BUFFER_RMNG_BTS = 0;
	caller_bffrs_CURSOR = 0;
	WRT_THIS_TO_buffer = 0;
	RQUSTD_BTS_CNTR = 0;
	RTURND_BTS_TRCKR = 0;
	open_files_stack[f_descriptor].sector_tracker = 0;
	open_files_stack[f_descriptor].file_selector = 0;
	return 0;
}
/*
 *
 */
int reset_write_io_vars(b_io_fd f_descriptor){
	free(LOC_STRG_BUFF_WR);
	LOC_STRG_BUFF_WR = NULL;
	LOCAL_STRG_AVL_SPC = 0;
	LOC_STORAGE_CURSOR = 0;
	LOCAL_STRG_LNGTH = 0;
	STATIC_RQSTD_BYTES = 0;
	WRT_THIS_TO_LOCAL_BUFF = 0;
	STARTUP_UTILS = 0;
	caller_buff_lftover = 0;
	caller_bfr_cursor = 0;
	add_leftover_to_list_node = 0;
	return 0;
}

/*
 *
 */
int reset_seek_io_vars(){
	B_SEEK_LOCK_X = 0;
	B_SEEK_SLCTR_LNGTH = 0;
	B_SEEK_LOCK_MODE03 = 0;
	if(temp_list_and_file_cpy != NULL) deleteList(&temp_list_and_file_cpy);
	if(temp_list_first_part != NULL) deleteList(&temp_list_first_part);
	if(temp_list_second_part != NULL) deleteList(&temp_list_second_part);

	return 0;
}

/*
 * Starts-up  f_iNode buffer_stack[MAX_TABLE_SIZE]
 */
void start_up (){
	for (int var = 0; var < MAX_TABLE_SIZE; ++var) {
		open_files_stack[var].file_data = NULL; // This is a Node pointer
		open_files_stack[var].file_descriptor = -1;
		open_files_stack[var].sector_tracker = 0;
		open_files_stack[var].file_selector = 0;
	}
	IS_STACK_INIT = 1; // array has been initialized
}

/*
 * Returns -1 if  file does not exist
 */
int b_open (char *filename, int flags) {

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


	//************ INITIALIZE STACK & PROVIDE FILE DESCRIPTOR************************
	int temp_loc_fd = 0;
	if(IS_STACK_INIT == 0) start_up();	//<--Initialize Files Stack--->F_INFO open_files_stack [ MAX_TABLE_SIZE ];


	int F_DESCRIPTOR = 0;
	for (int var = 0; var < MAX_TABLE_SIZE; ++var) { // Finds the first index available  : Initially, all file descriptors are initialized to -1, there are 21 (0-20)
		if (open_files_stack[var].file_descriptor == -1){
			temp_loc_fd = var;
			open_files_stack[var].file_descriptor = var;
			F_DESCRIPTOR = open_files_stack[var].file_descriptor;
			break;
		}
	}//****************************************************************************


	//************ CREATES AND NAMES FILE **********************************
	if (flags == 1){ // CREATES A READY TO WRITE FILE
		open_files_stack[F_DESCRIPTOR]._FLAG_ = 1; // O_RDWR = 3
		open_files_stack[F_DESCRIPTOR].file_name = filename;
		return open_files_stack[F_DESCRIPTOR].file_descriptor;
	}


	// Call directory


	//load_vitmap_vector();

	//************ IF file EXISTS ****************************
	char*iNode = malloc(B_CHUNK_SIZE/4);
	char*iNode_vector = malloc(B_CHUNK_SIZE/4);
	LBAread(iNode, 1, 100);
	//printf("\n\n**LBA 100\n %s\n", iNode);
	//Parse FILE meta-data
	int word_counter = 0;
	char *token;
	token = strtok(iNode, delimeter);
	int i;
	while (token != NULL) {
		if(word_counter == 1){ // iNode location in LBA
			i = atoi(token);
			open_files_stack[temp_loc_fd].location = i;
		}
		if(word_counter == 3){ // FILE SIZE
			i = atoi(token);
			open_files_stack[temp_loc_fd].file_size = i;
		}
		if(word_counter == 5){ // FILE FLAG
			i = atoi(token);
			open_files_stack[temp_loc_fd]._FLAG_ = i; // 0
		}
		if(word_counter == 7){ // BLOCKS NUM
			i = atoi(token);
			open_files_stack[temp_loc_fd].sectors_qty = i;
		}
		if(word_counter == 9){ // FILE NAME
			open_files_stack[temp_loc_fd].file_name = token;
			token = NULL;
			break;
		}
		token = strtok(NULL, delimeter);
		word_counter++;
	}//************ IF file EXISTS **************************



	//************ LOAD DATA FROM LBA TO A NODE LIST **********************************
	char*loc_temp_buff = malloc(B_CHUNK_SIZE+1);
	int temp_i = open_files_stack[temp_loc_fd].location + 1;// Position of first block of current file's data (excluding meta-data block)
	sector_info sector01;  // Creates a "type struct" with members: char*buffer and int bytes
	for (int var = 0; var < open_files_stack[temp_loc_fd].sectors_qty-1; ++var) { // Load file data sectors to a LinkedList  : This data comes from LBA[x] ...LBA[n], when file exist
		sector01.buff_sector = malloc(B_CHUNK_SIZE+1);
		if(sector01.buff_sector == NULL ){
			printf(MALLOC_ERR_MESS);
			return -1;
		}
		LBAread(loc_temp_buff, 1, temp_i);
		temp_i++;


		memcpy(sector01.buff_sector, loc_temp_buff, B_CHUNK_SIZE);
		sector01.buff_sector[B_CHUNK_SIZE] = '\0';
		sector01.sector_size = strlen(sector01.buff_sector);
		addNode(&open_files_stack[F_DESCRIPTOR].file_data, sector01); // < ===== " &buffer_stack[F_DESCRIPTOR].file_data " is a reference to the Head Node

	}//************ LOAD DATA FROM LBA TO A NODE LIST **********************************


	free(iNode_vector);
	iNode_vector = NULL;
	free(iNode);
	iNode = NULL;
	free(loc_temp_buff);
	loc_temp_buff = NULL;
	return open_files_stack[F_DESCRIPTOR].file_descriptor; // File descriptor is the location (index) of current file in the File Stack
}






/*
 * Reads segments of data that are larger than the standard 512-bytes sectors
 */
int read_long_bytes(b_io_fd f_descriptor, char * buffer, int requested_bytes){
	int read_bytes = 0;
	int returned_val = 0;
	WRT_THIS_TO_buffer = B_CHUNK_SIZE - LOCAL_BUF_CURSOR;
	buffer[WRT_THIS_TO_buffer] = '\0';
	memcpy(buffer, LOC_BUFFER_READ + LOCAL_BUF_CURSOR, WRT_THIS_TO_buffer); // rd 1,  2
	returned_val = strlen(buffer);
	caller_bffrs_CURSOR = returned_val; //rd 1 = 512
	WRT_THIS_TO_buffer = requested_bytes - caller_bffrs_CURSOR;
	if(returned_val < requested_bytes) { //	if(returned_val != requested_bytes && returned_val < requested_bytes) {
		do {
			read_bytes = reload_buffer(f_descriptor); // read_bytes = 512
			if(read_bytes == 0){ //Nothing left to read
				END_OF_READ_FILE = -1;
				return strlen(buffer); // Returns bytes leftover (returned quantity should be less than --> requested_bytes
			}
			if(WRT_THIS_TO_buffer >= B_CHUNK_SIZE) WRT_THIS_TO_buffer = B_CHUNK_SIZE;
			memcpy(buffer + caller_bffrs_CURSOR, LOC_BUFFER_READ, WRT_THIS_TO_buffer); //buffer + 412, LOC_BUFFER (512 - 368 = 144), ADD_THIS_TO_BUFF = 368
			//buffer[requested_bytes] = '\0';
			LOCAL_BUF_CURSOR = WRT_THIS_TO_buffer;
			LOC_BUFFER_RMNG_BTS = read_bytes - WRT_THIS_TO_buffer;
			returned_val += WRT_THIS_TO_buffer;
			caller_bffrs_CURSOR = returned_val;
			buffer[caller_bffrs_CURSOR] = '\0';
			if(caller_bffrs_CURSOR != requested_bytes){
				WRT_THIS_TO_buffer = requested_bytes - caller_bffrs_CURSOR;
			}
		} while(returned_val != requested_bytes);
	}
	WRT_THIS_TO_buffer = read_bytes - LOCAL_BUF_CURSOR;
	return (int) strlen(buffer);
}	// End read_long_bytes

/*
 *
 */
int b_read (b_io_fd file_d, char * buffer, int requested_bytes){

	//	printf("requested_bytes %d, OUTPUT: %s\n",requested_bytes, buffer);

	///	printList(open_files_stack[file_d].file_data);
	int routine_return_val = 0;
	memset(buffer,0 , strlen(buffer));
	if(END_OF_READ_FILE == -1){ // <-- No more bytes left to read from current open FILE
		/*		if(LOC_STRG_BUFF_WR != NULL && open_files_stack[file_d]._FLAG_ == 1){
			load_to_node_list(1);																	// This section of code gets executed
			//	open_files_stack[file_d].file_size = get_file_size(open_files_stack[file_d].file_data);		// when user wants to retrieve bytes of data
			//	add_leftover_to_list_node = 1;																// that has been recently added to file
			//	printf("\n\n file_d:: %d :: ",file_d);
			//	open_files_stack[1].file_size = open_files_stack[file_d].file_size;
			reset_write_io_vars(file_d);
			//printList(open_files_stack[1].file_data);
			reset_read_io_vars(file_d);
			// but has not been stored in LBA.
		}*/
		reset_write_io_vars(file_d);
		reset_read_io_vars(file_d);
		return 0;
	}
	// CHECKS FOR VALID INPUTS AND OVERFLOW OF DATA
	RQUSTD_BTS_CNTR += requested_bytes;
	if(RQUSTD_BTS_CNTR > open_files_stack[file_d].file_size && RTURND_BTS_TRCKR != 0){
		requested_bytes = (open_files_stack[file_d].file_size - RTURND_BTS_TRCKR) + 1;
		//	puts("ERROR: Invalid input...");
		END_OF_READ_FILE = -1;
	} else if (RQUSTD_BTS_CNTR > open_files_stack[file_d].file_size && RTURND_BTS_TRCKR == 0){
		requested_bytes = open_files_stack[file_d].file_size;
	} else if (requested_bytes <= 0){
		puts("ERROR: Invalid input...");
		return 0;
	}
	// BEGIN FILE READING EXECUTION
	if (ACTVE_OPN_FILE == 0){ 							 //   CLIENT INTITAL CALL IN THE CASE WHEN "requested_bytes" IS LESS THAN 512

		LOC_BUFFER_READ = (char*)malloc(B_CHUNK_SIZE + 1);
		if(LOC_BUFFER_READ == NULL ){
			printf(MALLOC_ERR_MESS);
			return -1;
		}											        // requested bytes = 100
		LOC_BUFFER_RMNG_BTS = reload_buffer(file_d);    // routine one: READ_BYTES = 512
		WRT_THIS_TO_buffer = LOC_BUFFER_RMNG_BTS;
		ACTVE_OPN_FILE = 1;
	}
	// HANDLES READINGS OF DATA THAT ARE LARGER THAN 512
	if(requested_bytes > B_CHUNK_SIZE) {  // <-- Case for when the requested bytes is larger than or equal to the standard 512 bytes sector
		routine_return_val = read_long_bytes(file_d, buffer, requested_bytes);
		RTURND_BTS_TRCKR += routine_return_val;
		open_files_stack[file_d].file_selector = RTURND_BTS_TRCKR;
		return routine_return_val;
	}
	// HANDLES READINGS OF DATA THAT ARE LESS THAN OR EQUAL TO 512
	if(requested_bytes <= LOC_BUFFER_RMNG_BTS){   // LOC_BUF_RMDR_BTS tracks remaining bytes in local buffer
		memcpy(buffer, LOC_BUFFER_READ + LOCAL_BUF_CURSOR, requested_bytes);
		LOCAL_BUF_CURSOR += requested_bytes;
		WRT_THIS_TO_buffer = LOC_BUFFER_RMNG_BTS - requested_bytes;
		buffer[requested_bytes] = '\0';
		routine_return_val = (int)strlen(buffer);
		LOC_BUFFER_RMNG_BTS -= routine_return_val;

		// HANDLES 512 bytes REQUESTS
		if(LOC_BUFFER_RMNG_BTS == 0){                            // <----------------------------------------
			LOC_BUFFER_RMNG_BTS = reload_buffer(file_d);

			if(LOC_BUFFER_RMNG_BTS < B_CHUNK_SIZE) {
				WRT_THIS_TO_buffer = LOC_BUFFER_RMNG_BTS;
				LOCAL_BUF_CURSOR = 0;						// <-- Gets ignore when requested bytes is not 512 bytes
				RTURND_BTS_TRCKR += routine_return_val;
				open_files_stack[file_d].file_selector = RTURND_BTS_TRCKR;
				return routine_return_val;
			}
			LOCAL_BUF_CURSOR = 0;  // <----------------------------------------
		}

		RTURND_BTS_TRCKR += routine_return_val;

		return routine_return_val;
	} else if (requested_bytes > LOC_BUFFER_RMNG_BTS){ //
		memcpy(buffer, LOC_BUFFER_READ + LOCAL_BUF_CURSOR, WRT_THIS_TO_buffer); // Copy REMAINDER_BTS to the caller's buffer

		int caller_temp_slctr = strlen(buffer);
		int bytes_count = reload_buffer(file_d);  // routine #5: bytes_count = 512
		LOC_BUFFER_RMNG_BTS = bytes_count;
		WRT_THIS_TO_buffer = requested_bytes - (int)strlen(buffer);

		if(bytes_count == 0){ //---------------------------------------------------------
			END_OF_READ_FILE = -1;
			open_files_stack[file_d].file_selector += (int)strlen(buffer);
			return (int)strlen(buffer);
		} //--------------------------------------------------------- Have reached the end of the open file
		memcpy(buffer + caller_temp_slctr, LOC_BUFFER_READ, WRT_THIS_TO_buffer);
		LOC_BUFFER_RMNG_BTS -= WRT_THIS_TO_buffer;
		WRT_THIS_TO_buffer = LOC_BUFFER_RMNG_BTS;
		LOCAL_BUF_CURSOR = bytes_count - LOC_BUFFER_RMNG_BTS;
		routine_return_val = strlen(buffer);
	}
	RTURND_BTS_TRCKR += routine_return_val;
	open_files_stack[file_d].file_selector = RTURND_BTS_TRCKR;
	return routine_return_val;
}// End b_read

/*
 *
 */
int write_long_bytes(int file_d, char *caller_bfr, int bytes_to_write){
	int routine_return_val = 0;
	if(bytes_to_write == B_CHUNK_SIZE){
		WRT_THIS_TO_LOCAL_BUFF = B_CHUNK_SIZE - LOC_STORAGE_CURSOR;  // if this is the first time this function is called, TEMP_BF_CURSOR = 0;
		memcpy(LOC_STRG_BUFF_WR + LOC_STORAGE_CURSOR, caller_bfr, WRT_THIS_TO_LOCAL_BUFF); // Transfer data to local fuffer
		LOCAL_STRG_LNGTH = (int)strlen(LOC_STRG_BUFF_WR);  // = 512
		caller_bfr_cursor  = WRT_THIS_TO_LOCAL_BUFF; //357
		LOCAL_STRG_AVL_SPC -= WRT_THIS_TO_LOCAL_BUFF; // 0
		caller_buff_lftover = B_CHUNK_SIZE - WRT_THIS_TO_LOCAL_BUFF;
		LOC_STORAGE_CURSOR = LOCAL_STRG_LNGTH;
		if(LOCAL_STRG_LNGTH == B_CHUNK_SIZE){
			LOC_STRG_BUFF_WR[B_CHUNK_SIZE] = '\0';
			load_to_node_list(file_d);
			LOCAL_STRG_AVL_SPC = B_CHUNK_SIZE;
			if(caller_buff_lftover > 0){
				memcpy(LOC_STRG_BUFF_WR, caller_bfr + caller_bfr_cursor , caller_buff_lftover); // Transfer data to local fuffer
				LOCAL_STRG_LNGTH = (int)strlen(LOC_STRG_BUFF_WR);  // = X
				LOCAL_STRG_AVL_SPC -= LOCAL_STRG_LNGTH;
				LOC_STORAGE_CURSOR = LOCAL_STRG_LNGTH;
				return B_CHUNK_SIZE;
			}
			LOC_STORAGE_CURSOR = 0;
			return B_CHUNK_SIZE;
		} // TEMP_BUFFER is been freed
	}
	caller_buff_lftover = bytes_to_write - WRT_THIS_TO_LOCAL_BUFF; // 512 - 472 = 40
	caller_bfr_cursor = WRT_THIS_TO_LOCAL_BUFF; // == 472
	routine_return_val = WRT_THIS_TO_LOCAL_BUFF; // == 472
	do{
		if(caller_buff_lftover >= B_CHUNK_SIZE){
			WRT_THIS_TO_LOCAL_BUFF = B_CHUNK_SIZE;
			memcpy(LOC_STRG_BUFF_WR, caller_bfr + caller_bfr_cursor, WRT_THIS_TO_LOCAL_BUFF);
			// Load to Linked-List
			if((int)strlen(LOC_STRG_BUFF_WR) == B_CHUNK_SIZE){
				load_to_node_list(file_d);
			} // TEMP_BUFFER is been freed
			caller_buff_lftover -= B_CHUNK_SIZE;
			caller_bfr_cursor += B_CHUNK_SIZE;
			routine_return_val += B_CHUNK_SIZE;
		} else if(caller_buff_lftover < B_CHUNK_SIZE){
			WRT_THIS_TO_LOCAL_BUFF = caller_buff_lftover; //40
			memcpy(LOC_STRG_BUFF_WR, caller_bfr + caller_bfr_cursor, WRT_THIS_TO_LOCAL_BUFF); // [ 512 ], 472, 40
			LOCAL_STRG_LNGTH = (int)strlen(LOC_STRG_BUFF_WR);  // = 40
			LOC_STORAGE_CURSOR = LOCAL_STRG_LNGTH;            // = 40
			LOCAL_STRG_AVL_SPC = B_CHUNK_SIZE - LOCAL_STRG_LNGTH; // 512 - 40 = 472
			routine_return_val += LOCAL_STRG_LNGTH;
			break;
		}
	} while(routine_return_val != bytes_to_write);
	return routine_return_val;
}
/*
 *
 */
int b_write (int file_d, char *callers_buffer, int bytes_to_write){
	int returnd_written_bts = 0;
	if(temp_list_and_file_cpy == NULL){
		returnd_written_bts = B_WRITE (file_d, callers_buffer, bytes_to_write);
		return returnd_written_bts;
	}
	int newFilepSize = 0;
	if(callers_buffer == NULL || file_d < 0  || bytes_to_write <= 0){
		puts("ERROR: Invalid input...\n Check for valid b_write() parameters.\n");
		return -1;
	}
	if(B_SEEK_LOCK_X == 1){ // ----------- Will execute when B_SEEK_LOCK_X = 1 : mode = 0
		newFilepSize = get_file_size(temp_list_and_file_cpy) + bytes_to_write;
		if(bytes_to_write > B_SEEK_SLCTR_LNGTH) bytes_to_write =  B_SEEK_SLCTR_LNGTH;
		/*
		if(B_SEEK_SLCTR_LNGTH <= B_CHUNK_SIZE){
			B_SEEK_SLCTR_LNGTH -= bytes_to_write;
			if(B_SEEK_SLCTR_LNGTH > 0){
				reset_seek_io_vars();
				B_SEEK_LOCK_X = 0;
				deleteList(&temp_list_and_file_cpy);
				temp_list_and_file_cpy = NULL;
				reset_read_io_vars(file_d);
				open_files_stack[file_d].file_size = newFilepSize ;
				return returnd_written_bts;
			}
		}else if(B_SEEK_SLCTR_LNGTH > B_CHUNK_SIZE){
			returnd_written_bts = B_WRITE (file_d, callers_buffer, bytes_to_write);
			B_SEEK_SLCTR_LNGTH -= bytes_to_write;
			returnd_written_bts +=returnd_written_bts;
			if(B_SEEK_SLCTR_LNGTH > 0) return returnd_written_bts;
		}
		if(B_SEEK_SLCTR_LNGTH == 0){}*/
		returnd_written_bts = B_WRITE (file_d, callers_buffer, bytes_to_write);
		for (int var = 0; var < get_list_size(temp_list_and_file_cpy); ++var) {
			Node *current = getNthNode(temp_list_and_file_cpy, var);
			int temp_size = current->data.sector_size;
			current->data.buff_sector[temp_size] = '\0';
			temp_size = (int)strlen(current->data.buff_sector);
			//	printf("\n*** NODE temp_size: %d ***\n", temp_size);
			B_WRITE (file_d, current->data.buff_sector, temp_size);
		}
		if((int)strlen(LOC_STRG_BUFF_WR) > 0)load_to_node_list(file_d);
		reset_seek_io_vars();
		B_SEEK_LOCK_X = 0;
		deleteList(&temp_list_and_file_cpy);
		temp_list_and_file_cpy = NULL;
		reset_read_io_vars(file_d);
		open_files_stack[file_d].file_size = newFilepSize ;
		return returnd_written_bts;
	} else 	if(B_SEEK_LOCK_X == 2){ // ----------- Will execute when B_SEEK_LOCK_X = 2 : mode = 1: inserts anywhere in the file
		/*
		int node_size = get_list_size(temp_list_first_part);
		for (int var = 0; var < node_size; ++var) {
			Node* temp_head = getNthNode(temp_list_first_part, var);
			int tempsiz = temp_head->data.sector_size;
			temp_head->data.buff_sector[tempsiz] = '\0';
			B_WRITE (file_d, temp_head->data.buff_sector, tempsiz);
		}
		 */

		/*		returnd_written_bts = B_WRITE (file_d, callers_buffer, bytes_to_write);
		node_size = get_list_size(temp_list_second_part);
		for (int var = 0; var < node_size; ++var) {
			Node* temp_head = getNthNode(temp_list_second_part, var);
			int tempsiz = temp_head->data.sector_size;
			temp_head->data.buff_sector[tempsiz] = '\0';
			B_WRITE (file_d, temp_head->data.buff_sector, tempsiz);
		}
		// load to last node
		if((int)strlen(LOC_STRG_BUFF_WR) > 0)load_to_node_list(file_d);
		deleteList(&temp_list_and_file_cpy);
		deleteList(&temp_list_second_part);
		deleteList(&temp_list_first_part);
		temp_list_second_part = NULL;
		temp_list_first_part = NULL;
		temp_list_and_file_cpy = NULL;
		reset_read_io_vars(file_d);
		reset_seek_io_vars();
		B_SEEK_LOCK_X = 0;
		open_files_stack[file_d].file_size = newFilepSize + bytes_to_write;*/
		return returnd_written_bts;
	} else if (B_SEEK_LOCK_X == 3) { // ----------- Will execute when B_SEEK_LOCK_X = 3 : mode = 2 : Will append bytes to the end of the file
		if (B_SEEK_LOCK_MODE03 == 0){
			//		reset_read_io_vars(file_d);
			int node_size = get_list_size(temp_list_and_file_cpy);
			for (int var = 0; var < node_size; ++var) {
				Node* temp_head = getNthNode(temp_list_and_file_cpy, var);
				temp_head->data.buff_sector[temp_head->data.sector_size] = '\0';
				B_WRITE (file_d, temp_head->data.buff_sector, temp_head->data.sector_size);
			}
			B_SEEK_LOCK_MODE03 = 1;
		}
		returnd_written_bts = B_WRITE (file_d, callers_buffer, bytes_to_write);
		if((int)strlen(LOC_STRG_BUFF_WR) > 0)load_to_node_list(file_d);
		deleteList(&temp_list_and_file_cpy);
		temp_list_and_file_cpy = NULL;
		reset_read_io_vars(file_d);
		reset_seek_io_vars();
		B_SEEK_LOCK_X = 0;
		open_files_stack[file_d].file_size = newFilepSize + bytes_to_write;
		return returnd_written_bts;
	}
	return returnd_written_bts;
}
/*
 *
 */
int B_WRITE (int file_d, char *callers_buffer, int bytes_to_write){
	int routine_return_val = 0;
	// Initial call
	if(STARTUP_UTILS == 0){
		LOC_STRG_BUFF_WR = malloc(B_CHUNK_SIZE +1);
		LOCAL_STRG_AVL_SPC = B_CHUNK_SIZE;
		STARTUP_UTILS = 1;
	}
	// HANDLES WRITINGS OF DATA THAT ARE LARGER THAN 512
	if(bytes_to_write >= B_CHUNK_SIZE) {  // <-- Case for when the requested bytes is larger than or equal to the standard 512 bytes sector
		routine_return_val = write_long_bytes(file_d, callers_buffer, bytes_to_write);
		return routine_return_val;
	}
	if(bytes_to_write <  LOCAL_STRG_AVL_SPC){ // each SCTOR_AVL_MEM is 512 bytes
		memcpy(LOC_STRG_BUFF_WR + LOC_STORAGE_CURSOR , callers_buffer, bytes_to_write);
		LOCAL_STRG_LNGTH = (int)strlen(callers_buffer) + LOC_STORAGE_CURSOR;
		LOC_STRG_BUFF_WR[LOCAL_STRG_LNGTH] = '\0';
		LOCAL_STRG_AVL_SPC = B_CHUNK_SIZE - LOCAL_STRG_LNGTH;
		LOC_STORAGE_CURSOR = LOCAL_STRG_LNGTH;
		routine_return_val = LOCAL_STRG_LNGTH;
		return routine_return_val;
	} else if(bytes_to_write > LOCAL_STRG_AVL_SPC) {
		// complete the first 512
		// Clear  BUFFER_WRT and set variables for the next round
		//------------Takes care of the section of the requested bytes that could not fit in previous sector---------
		WRT_THIS_TO_LOCAL_BUFF = B_CHUNK_SIZE - LOC_STORAGE_CURSOR;
		caller_bfr_cursor = WRT_THIS_TO_LOCAL_BUFF;
		memcpy(LOC_STRG_BUFF_WR + LOC_STORAGE_CURSOR, callers_buffer, WRT_THIS_TO_LOCAL_BUFF);
		LOCAL_STRG_LNGTH = (int)strlen(LOC_STRG_BUFF_WR);
		// Upload 512 bytes
		if(LOCAL_STRG_LNGTH == B_CHUNK_SIZE){
			//	printf("\n:: :: BYTES: %d, WRITEEN: %s\n", WRT_THIS_TO_LOCAL_BUFF, LOC_STRG_BUFFER);
			load_to_node_list(file_d);
		} // Free TEMP_BUFFER ---------------------------------------------------------------------------------------
		//--------Save the remaining portion of data from current request------------------
		WRT_THIS_TO_LOCAL_BUFF = (int)strlen(callers_buffer) - caller_bfr_cursor;
		memcpy(LOC_STRG_BUFF_WR, callers_buffer + caller_bfr_cursor, WRT_THIS_TO_LOCAL_BUFF);
		LOCAL_STRG_LNGTH = (int)strlen(LOC_STRG_BUFF_WR);  //
		LOC_STORAGE_CURSOR = LOCAL_STRG_LNGTH;
		LOCAL_STRG_AVL_SPC = B_CHUNK_SIZE - LOCAL_STRG_LNGTH;
		//printf("\n:::::TEMP_BUFFER_AVL_MEM:::: %s\n\n\n", LOC_STRG_BUFF_WR);
		routine_return_val = caller_bfr_cursor + WRT_THIS_TO_LOCAL_BUFF;
		return routine_return_val;
	}
	return routine_return_val;
}
/*
 * Loads 512-bytes-segments of data to a node list
 */

void load_to_node_list(int file_d){
	int malloc_size = (int)strlen(LOC_STRG_BUFF_WR);
	sector_var_x.buff_sector = malloc(malloc_size+1);
	if(sector_var_x.buff_sector == NULL ){
		printf(MALLOC_ERR_MESS);
		exit(EXIT_FAILURE);
	}
	memcpy(sector_var_x.buff_sector, LOC_STRG_BUFF_WR, LOCAL_STRG_LNGTH);
	sector_var_x.sector_size = LOCAL_STRG_LNGTH;
	sector_var_x.buff_sector[LOCAL_STRG_LNGTH] = '\0';
	addNode(&open_files_stack[file_d].file_data, sector_var_x);
	if(open_files_stack[file_d]._FLAG_ == 1){
		open_files_stack[file_d].file_size+=sector_var_x.sector_size;
		open_files_stack[file_d].sectors_qty++;
	}
	memset(LOC_STRG_BUFF_WR, 0, B_CHUNK_SIZE);

}
/*
 *
 */
int b_seek(int file_d, signed int selector, int mode){ // mode: 0 = beginning of file, 1 = current position, 2 = end of file (reads backward with negative selector values)
	sector_info sector_x;
	char * loc_temp_buff;
	int read_bytes = 0;
	loc_temp_buff = malloc(selector);
	int this_local_bff_offset = RQUSTD_BTS_CNTR;
	B_SEEK_SLCTR_LNGTH = selector; // Saves user b_seek requested bytes

	// CASE: 1 Reading
	if( open_files_stack[file_d]._FLAG_ == 0 || open_files_stack[file_d]._FLAG_ == 3){ // B_SEEK() READ
		if (mode == 0){ // Jumps selector to a specific position within current open file <----
			reset_read_io_vars(file_d);
			read_bytes = b_read (file_d, loc_temp_buff, selector);
		} else if (mode == 1){
			read_bytes = b_read (file_d, loc_temp_buff, selector);
			read_bytes = this_local_bff_offset + selector;
		} else if (mode == 2){
			int temp_fl_size = open_files_stack[file_d].file_size;
			int temp_bts_cnt = temp_fl_size + selector;
			read_bytes = b_read (file_d, loc_temp_buff, temp_bts_cnt);
		} //------------------------------------------------------------------
	}
	// CASE 2:  writing flag = 1
	if( open_files_stack[file_d]._FLAG_ == 1){ 								// B_SEEK() WRITE : 3-modes: 0 , 1, 2
		reset_read_io_vars(file_d);
		int temp_list_i = get_list_size(open_files_stack[file_d].file_data);//	B_SEEK_SLCTR_LNGTH = selector;
		for (int var = 0; var < temp_list_i; ++var) {
			Node *temp_head = getNthNode(open_files_stack[file_d].file_data, var);	// Make a copy of current open file
			sector_x.buff_sector = malloc(temp_head->data.sector_size + 1);
			sector_x.buff_sector[temp_head->data.sector_size] = '\0';
			memcpy(sector_x.buff_sector, temp_head->data.buff_sector, temp_head->data.sector_size);
			sector_x.sector_size = temp_head->data.sector_size;
			if(sector_x.sector_size <= 0 || (int)strlen(temp_head->data.buff_sector) == 0)break;
			addNode(&temp_list_and_file_cpy, sector_x);
		}
		deleteList(&open_files_stack[file_d].file_data);
		open_files_stack[file_d].file_data = NULL;
		//case 1: mode = 0
		if (mode == 0){ // Append at the beginning of file when selector is less than 512
			reset_read_io_vars(file_d);
			//printList(temp_list_and_file_cpy);

			B_SEEK_LOCK_X = 1;
			return B_SEEK_SLCTR_LNGTH;
			//case: 1
		}
		//case 1: mode = 1 =========================================================
		if (mode == 1){ // Append at any position within current open file
			int num_sectors = this_local_bff_offset / B_CHUNK_SIZE;
			int sector_remainin = this_local_bff_offset % B_CHUNK_SIZE;
			Node *current_;
			if(num_sectors == 0){
				current_ = getNthNode(temp_list_and_file_cpy, 0);
				sector_x.buff_sector = malloc(selector);
				sector_x.buff_sector[selector] = '\0';
				memcpy(sector_x.buff_sector, current_->data.buff_sector, this_local_bff_offset);
				sector_x.sector_size = (int)strlen(sector_x.buff_sector);
				addNode(&temp_list_first_part, sector_x);
				// Save second part of current open file
				int this_sector_cursor = B_CHUNK_SIZE - this_local_bff_offset;
				sector_x.buff_sector = malloc(this_sector_cursor);
				sector_x.buff_sector[this_sector_cursor] = '\0';
				memcpy(sector_x.buff_sector, current_->data.buff_sector + this_local_bff_offset, this_sector_cursor);
				//printf("\n\n  this_sector_cursor: %d,  sector_x.buff_sector  %s\n", this_sector_cursor, sector_x.buff_sector);
				reset_write_io_vars(file_d);
				B_WRITE(file_d, sector_x.buff_sector, this_sector_cursor);
				num_sectors++;
				//	printf("\n\n num_sectors = %d,  X = %d this_sector_cursor %d  , LOC_STRG_BUFF_WR = %s\n\n", num_sectors, x, this_sector_cursor, LOC_STRG_BUFF_WR);
				for (int var = num_sectors; var < get_list_size(temp_list_and_file_cpy); ++var) {
					current_ = getNthNode(temp_list_and_file_cpy, var);
					if((int)strlen(current_->data.buff_sector) <= 0 ) break;
					B_WRITE(file_d, current_->data.buff_sector, current_->data.sector_size);
				}
				int num_nodes =  get_list_size(open_files_stack[file_d].file_data);
				for (int var = 0; var < num_nodes; ++var) {
					Node *temp_head = getNthNode(open_files_stack[file_d].file_data, var);
					sector_x.buff_sector = malloc(temp_head->data.sector_size + 1);
					sector_x.sector_size = temp_head->data.sector_size;
					memcpy(sector_x.buff_sector, temp_head->data.buff_sector, sector_x.sector_size);
					addNode(&temp_list_second_part, sector_x);
				}
				//	printList(temp_list_second_part);                              //========================= PRINT
				reset_write_io_vars(file_d);
				reset_read_io_vars(file_d);
				B_SEEK_LOCK_X = 2;
				return get_file_size(temp_list_first_part) + B_SEEK_SLCTR_LNGTH;
			} else if (num_sectors > 0){ // MODE 1 insert in a posi5ion larger beyond 512 Append at any position within current open file when selector is larger than 512
				int _index_ = 0;
				for (int var = 0; var < num_sectors; ++var) {
					current_ = getNthNode(temp_list_and_file_cpy, var);
					sector_x.buff_sector = malloc(B_CHUNK_SIZE);
					sector_x.buff_sector[B_CHUNK_SIZE] = '\0';
					memcpy(sector_x.buff_sector, current_->data.buff_sector, B_CHUNK_SIZE);
					sector_x.sector_size = B_CHUNK_SIZE;
					addNode(&temp_list_first_part, sector_x);
					_index_ = var;
				}
				_index_++;
				current_ = getNthNode(temp_list_and_file_cpy, _index_);
				sector_x.buff_sector = malloc(sector_remainin);
				sector_x.buff_sector[sector_remainin] = '\0';
				memcpy(sector_x.buff_sector, current_->data.buff_sector, sector_remainin);
				sector_x.sector_size = sector_remainin;
				addNode(&temp_list_first_part, sector_x);
				//======================================= PRINT FIRST PART
				//reset_read_io_vars(file_d);
				reset_write_io_vars(file_d);
				int local_buff_cursor = sector_remainin;
				int local_buff_remain = B_CHUNK_SIZE-local_buff_cursor;
				sector_x.buff_sector = malloc(local_buff_remain);
				sector_x.buff_sector[local_buff_remain] = '\0';
				memcpy(sector_x.buff_sector, current_->data.buff_sector + local_buff_cursor, local_buff_remain);
				B_WRITE(file_d, sector_x.buff_sector, local_buff_remain);
				_index_++;
				int list_size = get_list_size(temp_list_and_file_cpy);
				for (int var = _index_; var < list_size; ++var) {
					Node *temp_head = getNthNode(temp_list_and_file_cpy, var);
					//	printf("\n num_sectors = %s\n\n\n",  temp_head->data.buff_sector);
					int buff_sz = temp_head->data.sector_size;
					temp_head->data.buff_sector[buff_sz] = '\0';
					buff_sz = (int)strlen(temp_head->data.buff_sector);
					B_WRITE(file_d, temp_head->data.buff_sector, buff_sz);
				}
				load_to_node_list(file_d);
				printList(open_files_stack[file_d].file_data);
				//  printList(open_files_stack[file_d].file_data);
				//   printList(temp_list_second_part);
				int num_nodes =  get_list_size(open_files_stack[file_d].file_data);
				// Make a deep copy of second part of file
				for (int var = 0; var < num_nodes; ++var) {
					Node *temp_head = getNthNode(open_files_stack[file_d].file_data, var);
					sector_x.buff_sector = malloc(temp_head->data.sector_size + 1);
					sector_x.sector_size = temp_head->data.sector_size;
					sector_x.buff_sector[temp_head->data.sector_size] = '\0';
					//printf("\n\n temp_head->data.sector_size %d :\n\n", temp_head->data.sector_size);
					memcpy(sector_x.buff_sector, temp_head->data.buff_sector, sector_x.sector_size);
					addNode(&temp_list_second_part, sector_x);
				}
				//======================================= PRINT Second PART
				deleteList(&open_files_stack[file_d].file_data);
				open_files_stack[file_d].file_data = NULL;
				reset_write_io_vars(file_d);
				reset_read_io_vars(file_d);
				B_SEEK_LOCK_X = 2;
				//printf("\n****************************************************************> \n");
				//printList(temp_list_first_part);
				//printList(temp_list_second_part);
				//printf("\n ****************************************************************> *****> \n\n\n");
				read_bytes = get_file_size(temp_list_first_part)+ B_SEEK_SLCTR_LNGTH;
				return read_bytes;
			}
		}
		//case: 2
		if (mode == 2){   // Appends bytes at a position X bytes backward/forward from the end of file
			int selector_pos = get_file_size(temp_list_and_file_cpy) + selector;
			reset_write_io_vars(file_d);
			reset_read_io_vars(file_d);
			B_SEEK_LOCK_X = 3;
			return selector_pos;
		}
	}
	free(loc_temp_buff);
	loc_temp_buff = NULL;
	return read_bytes;
}








/*
 * Reads Nodes from LinkedList
 * Each Node stores 512 bytes of data in a pointer
 * and int, which stores the actual length of the pointer, usuaaly 512 and less than 512 for the last node.
 * Assigns L_BUFFER with the char value from current Node;
 * And returns the the length of the pointer, usually 512 and potentially less than 512 for the last node.
 *
 */
int reload_buffer(b_io_fd f_descriptor){
	// When 0 is assigned to LINKEDLIST_LASTNODE,  this indicates there are not more nodes left to read from a file
	if(LINKEDLIST_LASTNODE == 0){ // end of file reading  :  returns 0
		return 0;
	}
	// Initial call
	Node *current = open_files_stack[f_descriptor].file_data;
	current = getNthNode(current, open_files_stack[f_descriptor].sector_tracker);
	strcpy(LOC_BUFFER_READ, current->data.buff_sector);
	open_files_stack[f_descriptor].sector_tracker++;
	if(current->data.sector_size < B_CHUNK_SIZE) LINKEDLIST_LASTNODE = 0;
	return current->data.sector_size;
}	//End of reload_buffer


