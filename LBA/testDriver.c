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
    mkDir("file2");

    printDirectoryTable();

    setWorkingDirectory("root");
    setWorkingDirectory("root1");

    printf("\n");
    return 0;
}