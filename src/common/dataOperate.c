#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "elf.h"

uint64_t byte_get_little_endian(const unsigned char *field, int size) {
  switch (size) {
  case 1:
    return *field;

  case 2:
    return ((unsigned int)(field[0])) | (((unsigned int)(field[1])) << 8);

  case 3:
    return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
           (((uint64_t)(field[2])) << 16);

  case 4:
    return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
           (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);

  case 5:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  case 6:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32) | (((uint64_t)(field[5])) << 40);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  case 7:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32) | (((uint64_t)(field[5])) << 40) |
             (((uint64_t)(field[6])) << 48);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  case 8:
    if (sizeof(uint64_t) == 8)
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24) |
             (((uint64_t)(field[4])) << 32) | (((uint64_t)(field[5])) << 40) |
             (((uint64_t)(field[6])) << 48) | (((uint64_t)(field[7])) << 56);
    else if (sizeof(uint64_t) == 4)
      /* We want to extract data from an 8 byte wide field and
         place it into a 4 byte wide field.  Since this is a little
         endian source we can just use the 4 byte extraction code.  */
      return ((uint64_t)(field[0])) | (((uint64_t)(field[1])) << 8) |
             (((uint64_t)(field[2])) << 16) | (((uint64_t)(field[3])) << 24);
    /* Fall through.  */

  default:
    printf("Unhandled data length: %d\n", size);
    abort();
  }
}

void byte_put_little_endian(unsigned char *field, uint64_t value, int size) {
  switch (size) {
  case 8:
    field[7] = (((value >> 24) >> 24) >> 8) & 0xff;
    field[6] = ((value >> 24) >> 24) & 0xff;
    field[5] = ((value >> 24) >> 16) & 0xff;
    field[4] = ((value >> 24) >> 8) & 0xff;
    /* Fall through.  */
  case 4:
    field[3] = (value >> 24) & 0xff;
    /* Fall through.  */
  case 3:
    field[2] = (value >> 16) & 0xff;
    /* Fall through.  */
  case 2:
    field[1] = (value >> 8) & 0xff;
    /* Fall through.  */
  case 1:
    field[0] = value & 0xff;
    break;

  default:
    printf("Unhandled data length: %d\n", size);
    abort();
  }
}

void close_file(FILE *fileHandle) {
  if (fileHandle)
    fclose(fileHandle);
  else {
    printf("fileHandle is NULL\n");
    abort();
  }
}