#include "LBA/lba.h"
#include "../utils/stack.h"
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

    struct stack_util stack = create_stack(10);
    stack_push("AA", &stack);
    stack_push("AA1", &stack);
    stack_push("AA12", &stack);
    printf("Peek: %s\n", stack_peek(&stack));
    stack_pop(&stack);
    printf("Peek: %s\n", stack_peek(&stack));

    printf("\n");
    return 0;
}