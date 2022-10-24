#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <libintl.h>
#include <stdarg.h>

#include "../common/elf.h"


int main (int argc, char ** argv){
    struct stat statBuf;
    //FileData * fileData = NULL;

    const char * inputFileName = argv[1];

    if (stat(inputFileName, &statBuf) < 0){
        if (errno == ENOENT)
            fprintf(stderr, "No such file: %s\n", inputFileName);
        else
            fprintf(stderr, "System error message: %d\n", strerror(errno));
        return false;
    }

    fprintf(stderr, "file: %s size is %ld\n", inputFileName, statBuf.st_size);
    //fileData = calloc (1, sizeof * fileData);

    //get_file_header (FileData * fileData);


    Elf64_Ehdr eHdr;
    return 0;
}
