#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

#include "dataOperate.h"

extern Elf64_Off shdrCommentOff;
extern Elf64_Xword shdrCommentSize;

int dumpComment(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Contents of section .comment:\n");
  if (!shdrCommentOff) {
    printf("No .comment section\n");
    abort();
  }

  char *strBuffer = (char *)malloc(shdrCommentSize);

  if (!fseek(fileHandle, shdrCommentOff, SEEK_SET))
    fread(strBuffer, shdrCommentSize, 1, fileHandle);

  for (int charIndex = 0; charIndex < shdrCommentSize; charIndex++)
    if (strBuffer[charIndex] == '\0')
      fprintf(stderr, " ");
    else
      fprintf(stderr, "%c", strBuffer[charIndex]);

  closeFile(fileHandle);

  fprintf(stderr, "\n\n");
  return 0;
}
