#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

#include "dataOperate.h"

extern uint64_t shdrGCCcommandlineOff;
extern uint64_t shdrGCCcommandlineSize;

int dumpGCC_command_line(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Contents of section .GCC.command.line:\n");

  if (!shdrGCCcommandlineOff) {
    fprintf(stderr, "\n");
    return 0;
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
