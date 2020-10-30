#include "LBA/lba.h"
#include <stdio.h>

int main() {
    printf("\n");

    testDEFunction();   // These functions are just to ensure that they are working.
    testMBRFunction();  // Each one prints their respective sentences.
    testFSMFunction();

    initializeDirectoryEntryTable();
    expandDirectoryEntryTable(3);

    mkDir("TestDir1");
    mkDir("DirectoryTest");

    printf("%s\n",getDirectory(2).self_name);

    printf("\n");
    return 0;
}