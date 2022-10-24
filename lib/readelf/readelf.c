#include <stdio.h>

#include "elfHeader.h"

int main (int argc, char ** argv){
    const char * inputFileName = argv[1];

    processElfHeader (inputFileName);

    return 0;
}
