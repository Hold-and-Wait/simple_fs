#include <stdio.h>
#include "b_io.h"

/*
 * Returns -1 if  file does not exist
 */
int b_open (char * filename, int flags) {
    // check if filename exists and it is of type DT_REG (a file)
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

    // Return the writable/readable portion of the file
    return directory->directoryStartLocation + 1; // add 1 because we do not want to read/write the metadata block
}

// for b_seek, b_write and b_read, make sure to update directory->dirEntryPosition += count, how ever many bytes were read/written