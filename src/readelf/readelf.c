#include <stdio.h>

#include "elfHeader.h"
#include "sectionHeader.h"
#include "programHeader.h"

int main (int argc, char ** argv){
    const char * inputFileName = argv[1];

    processElfHeader (inputFileName);
    processSectionHeader (inputFileName);
    processProgramHeader (inputFileName);

    return 0;
}
