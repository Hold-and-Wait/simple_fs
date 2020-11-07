#include "LBA/lba.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

    printf("\n");

    initializeDirectory();
    print_table();
    printf("\n\n");

    printf("\n\t**CREATE FILE**\n");
    fs_mkdir("file1", 2);
    print_table();

    printf("\n\n");


    printf("\n\t**CHANGE DIRECTORY TO \'file1\'**\n");
    fs_setcwd("file1");
    printf("\t**CREATE FILE \'file2\'**\n");
    fs_mkdir("file2", 2);
    print_table();

    printf("\n\n");

    printf("\n**CHANGE DIRECTORY TO NONEXISTENT FILE \'/nonexistent_file0\'**\n");
    fs_setcwd("/nonexistent_file0");
    printf("\t**CREATE FILE \'file3\'**\n");
    fs_mkdir("file3", 2);
    print_table();

    printf("\n\n");

    printf("\n**CHANGE DIRECTORY TO CONSECUTIVE NONEXISTENT FILES \'/nonexistent_file1/nonexistent_file2\'**\n");
    fs_setcwd("/nonexistent_file1/nonexistent_file2");
    printf("\t**CREATE FILE \'file4\'**\n");
    fs_mkdir("file4", 2);
    print_table();

    printf("\n\n");

    printf("\n\t**CHANGE DIRECTORY TO \'../../file1/file2\'**\n");
    fs_setcwd("../../file1/file2");
    printf("\t**CREATE FILE \'file5\'**\n");
    fs_mkdir("file5", 2);
    print_table();


    printf("\n\n");

    printf("\n\t**CHANGE DIRECTORY TO MULTIPLE EXISTING DIRs \'/file1/file2\'**\n");
    fs_setcwd("/file1/file2");
    printf("\t**CREATE FILE \'file8\'**\n");
    fs_mkdir("file8", 2);
    fs_mkdir("file9", 2);
    print_table();


    printf("\n");

    //fs_rmdir("/file1/file2");

    free_dir_mem();
    return 0;
}

/* COPY AND PASTE TO MAKEFILE TO TEST DIRECTORY ENTRIES*/
/*
ROOTNAME=LBA/testDriver
HW=
FOPTION=
RUNOPTIONS=SampleVolume 10000000 512
CC=gcc
CFLAGS= -g -I.
LIBS =pthread
DEPS = LBA/lba.h utils/stack.h
ADDOBJ= LBA/DirectoryEntries.c LBA/FreeSpaceManagement.c LBA/MasterBootRecord.c utils/stack.c
OBJ = $(ROOTNAME)$(HW)$(FOPTION).o $(ADDOBJ)

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(ROOTNAME)$(HW)$(FOPTION): $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -lm -l readline -l $(LIBS)

clean:
	rm *.o $(ROOTNAME)$(HW)$(FOPTION)

run: $(ROOTNAME)$(HW)$(FOPTION)
	./$(ROOTNAME)$(HW)$(FOPTION) $(RUNOPTIONS)
 */