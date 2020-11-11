# README

### Makefile to test the functions:
Copy and paste rhe following code into your makefile folder.
```
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
  ```
  
 Then, type ```make run``` into your terminal.
