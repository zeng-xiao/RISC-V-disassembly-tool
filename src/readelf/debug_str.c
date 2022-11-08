#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

#include "dataOperate.h"

extern Elf64_Off shdrDebug_strOff;
extern Elf64_Xword shdrDebug_strSize;

int dumpDebug_str(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Contents of section .debug_str:\n");

  if (!shdrDebug_strOff) {
    fprintf(stderr, "\n");
    return 0;
  }

  char *strBuffer = (char *)malloc(shdrDebug_strSize);

  if (!fseek(fileHandle, shdrDebug_strOff, SEEK_SET))
    fread(strBuffer, shdrDebug_strSize, 1, fileHandle);

  for (int charIndex = 0; charIndex < shdrDebug_strSize; charIndex++)
    if (strBuffer[charIndex] == '\0')
      fprintf(stderr, " ");
    else
      fprintf(stderr, "%c", strBuffer[charIndex]);

  closeFile(fileHandle);

  fprintf(stderr, "\n\n");

  return 0;
}
