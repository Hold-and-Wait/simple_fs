#include "LBA/lba.h"
#include <stdio.h>
#include <stdlib.h>

int main() {

    printf("\n");

    initializeDirectory();

    fs_mkdir("use/mkdir/to/create/new/files", 0);
    fs_setcwd("/use/setcwd/to/change/directories"); // <- will fail since setcwd is not an existing file!
    print_table();

    fs_rmdir("/use");

    print_table();
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