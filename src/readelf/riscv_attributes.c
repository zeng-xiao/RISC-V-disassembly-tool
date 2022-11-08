#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

#include "dataOperate.h"

extern Elf64_Off shdrRiscv_attributesOff;
extern Elf64_Xword shdrRiscv_attributesSize;

int dumpRiscv_attributes(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Contents of section .riscv.attributes:\n");

  if (!shdrRiscv_attributesOff) {
    fprintf(stderr, "\n");
    return 0;
  }

  char *strBuffer = (char *)malloc(shdrRiscv_attributesSize);

  if (!fseek(fileHandle, shdrRiscv_attributesOff, SEEK_SET))
    fread(strBuffer, shdrRiscv_attributesSize, 1, fileHandle);

  for (int charIndex = 0; charIndex < shdrRiscv_attributesSize; charIndex++)
    if (strBuffer[charIndex] == '\0')
      fprintf(stderr, " ");
    else
      fprintf(stderr, "%c", strBuffer[charIndex]);

  closeFile(fileHandle);

  fprintf(stderr, "\n\n");

  return 0;
}
