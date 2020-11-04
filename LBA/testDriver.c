#include "LBA/lba.h"
#include <stdio.h>

int main() {
    printf("\n");

    initializeDirectory();

    printf("\n\t**CREATING FILES**\n");
    fs_mkdir("file1", 2);
    fs_mkdir("file2", 2);
    fs_mkdir("file3", 2);
    print_table();


    printf("\n\t**CHANGE DIRECTORY**\n");
    fs_setcwd("file2");
    fs_mkdir("file4", 2);
    fs_mkdir("file5", 2);
    print_table();


    printf("\n**CHANGE DIRECTORY TO NONEXISTENT FILE \'/nonexistent_file0\'**\n");
    fs_setcwd("/nonexistent_file0");
    fs_mkdir("file6", 2);
    print_table();


    printf("\n**CHANGE DIRECTORY TO CONSECUTIVE NONEXISTENT FILES \'/nonexistent_file1/nonexistent_file2\'**\n");
    fs_setcwd("/nonexistent_file1/nonexistent_file2");
    fs_mkdir("file7", 2);
    print_table();


    printf("\n\t**CHANGE DIRECTORY TO \'..\'**\n");
    fs_setcwd("..");
    fs_mkdir("file8", 2);
    print_table();


    printf("\n\t**CHANGE DIRECTORY TO ROOT**\n");
    fs_setcwd("/");
    fs_mkdir("file9", 2);
    print_table();



    printf("\n");
    return 0;
}