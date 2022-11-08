#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

#include "dataOperate.h"

extern Elf64_Off shdrGCCcommandlineOff;
extern Elf64_Xword shdrGCCcommandlineSize;

int dumpGCCcommandline(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Contents of section .GCC.command.line:\n");
  if (!shdrGCCcommandlineOff) {
    printf("No .GCC.command.line section\n");
    abort();
  }

  char *strBuffer = (char *)malloc(shdrGCCcommandlineSize);

  if (!fseek(fileHandle, shdrGCCcommandlineOff, SEEK_SET))
    fread(strBuffer, shdrGCCcommandlineSize, 1, fileHandle);

  for (int charIndex = 0; charIndex < shdrGCCcommandlineSize; charIndex++)
    if (strBuffer[charIndex] == '\0')
      fprintf(stderr, " ");
    else
      fprintf(stderr, "%c", strBuffer[charIndex]);

  closeFile(fileHandle);

  fprintf(stderr, "\n\n");
  return 0;
}
