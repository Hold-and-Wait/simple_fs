#include "LBA/lba.h"
#include <stdio.h>

int main() {
    printf("\n");

    testDEFunction();   // These functions are just to ensure that they are working.
    testMBRFunction();  // Each one prints their respective sentences.
    testFSMFunction();


    initializeDirectoryEntryTable("root");
    initializeInodeTable();

    mkDir("file1");
    setWorkingDirectory("file1");
    mkDir("file2");
    mkDir("file3");
    setWorkingDirectory("file2");
    mkDir("file4");
    setWorkingDirectory("/file1/file3");
    mkDir("file5");

    // change directory to nonexistent file
    setWorkingDirectory("/non_existent_file");






    printDirectoryTable();

    printf("\n");
    return 0;
}