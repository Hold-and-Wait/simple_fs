#include "LBA/lba.h"
#include <stdio.h>

int main() {
    printf("\n");

    testDEFunction();   // These functions are just to ensure that they are working.
    testMBRFunction();  // Each one prints their respective sentences.
    testFSMFunction();

    initializeDirectoryEntryTable();

    mkDir("TestDir1");
    mkDir("DirectoryTest");

    rmDir("DirectoryTest");

    mkDir("TestDir2");

    printDirectoryTable();

    printf("\n");
    return 0;
}