#include <sys/stat.h>
#include <stdio.h>
#include <errno.h>
#include <libintl.h>
#include <stdarg.h>
#include <stdbool.h>

#include "elf.h"

int processElfHeader (const char * inputFileName){
    Elf64_Ehdr elfHdr;
    const char * elfClass;
    const char * elfDataEncodingForm;
    const char * elfVersion;

    FILE * fileHandle = fopen (inputFileName, "rb");

    fread (elfHdr.e_ident, EI_NIDENT, 1, fileHandle);

    if (elfHdr.e_ident[EI_MAG0] != ELFMAG0 ||
        elfHdr.e_ident[EI_MAG1] != ELFMAG1 ||
        elfHdr.e_ident[EI_MAG2] != ELFMAG2 ||
        elfHdr.e_ident[EI_MAG3] != ELFMAG3)
        return false;

    if (elfHdr.e_ident[EI_CLASS] == ELFCLASS32) 
        elfClass = "ELF32";
    else if (elfHdr.e_ident[EI_CLASS] == ELFCLASS64) 
        elfClass = "ELF64";
    else
        return false;

    if (elfHdr.e_ident[EI_DATA] == ELFDATA2LSB)
        elfDataEncodingForm = "2's complement, little endian";
    else if (elfHdr.e_ident[EI_DATA] == ELFDATA2MSB) 
        elfDataEncodingForm = "2's complement, big endian";
    else 
        return false;

    // if (elfHdr.e_ident[EI_VERSION] == e_version) elfVersion = "1";
    // else return false;
    fprintf(stderr, "\n\n");
    fprintf(stderr, "ELF Header:\n");

    fprintf(stderr, "  Magic Number:                      ");
    for (int i = 0; i < EI_NIDENT; i++)
        fprintf(stderr, "%2.2x ", elfHdr.e_ident[i]);
    fprintf(stderr, "\n");

    fprintf(stderr, "  Binary File Class:                 %s\n", elfClass);
    fprintf(stderr, "  Data Encoding Form:                %s\n", elfDataEncodingForm);
    fprintf(stderr, "\n\n");

    
    return 0;
}
