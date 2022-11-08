#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf.h"

#include "dataOperate.h"

#include "instructions.def"

extern Elf64_Off shdrTextOff;
extern Elf64_Xword shdrTextSize;

static unsigned char compressionInstruction = 2;
static unsigned char uncompressionInstruction = 4;

static uint64_t instructionIndex = 0;

static void instructionStr(uint64_t instruction, uint64_t rs1, uint64_t rs2,
                           uint64_t rd, const char *instStr) {

  char str[80];

  char *str_rd;
  sprintf(str_rd, "%lu", rd);

  char *str_rs1;
  sprintf(str_rs1, "%lu", rs1);

  char *str_rs2;
  sprintf(str_rs2, "%lu", rs2);

  switch (instruction) {
  case 0b0000000000LL:
    strcpy(str, "add ");
    strcpy(str, str_rd);
    strcpy(str, " ");
    strcpy(str, str_rs2);
    strcpy(str, " ");
    strcpy(str, str_rs1);
    instStr = str;
    break;
  case 0b0100000000LL:
    instStr = "sub";
    break;
  case 0b0000000001LL:
    instStr = "sll";
    break;
  case 0b0000000010LL:
    instStr = "slt";
    break;
  case 0b0000000011LL:
    instStr = "sltu";
    break;
  case 0b0000000100LL:
    instStr = "xor";
    break;
  case 0b0000000101LL:
    instStr = "srl";
    break;
  case 0b0100000101LL:
    instStr = "sra";
    break;
  case 0b0000000110LL:
    instStr = "or";
    break;
  case 0b0000000111LL:
    instStr = "and";
    break;

  default:
    fprintf(stderr, "Invalid instruction %ld:\n", instruction);
    abort();
  }
}

static void decode(uint64_t instructionEncoding, unsigned char C) {
  const char *instStr;

  if (C == 4) {
    uint64_t opcode = instructionEncoding & 0b1111111LL;
    switch (opcode) {
    case 0b0110011LL: // R
    {
      uint64_t funct7 =
          instructionEncoding & 0b1111111000000000000000000000000LL;
      uint64_t rs2 = instructionEncoding & 0b111110000000000000000000LL;
      uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
      uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
      uint64_t rd = instructionEncoding & 0b11111000000LL;

      instructionStr(funct7 << 3 | funct3, rs1, rs2, rd, instStr);
    } break;

    case 0b0000011LL: // I Type
    {
      uint64_t imm11_0 =
          instructionEncoding & 0b1111111111110000000000000000000LL;
      uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
      uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
      uint64_t rd = instructionEncoding & 0b11111000000LL;
    } break;

    case 0b0100011LL: // S Type
    {
      uint64_t imm11_5 =
          instructionEncoding & 0b1111111000000000000000000000000LL;
      uint64_t rs2 = instructionEncoding & 0b111110000000000000000000LL;
      uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
      uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
      uint64_t imm4_0 = instructionEncoding & 0b11111000000LL;
    } break;

    case 0b1100011LL: // B Type
    {
      uint64_t imm12 =
          instructionEncoding & 0b1000000000000000000000000000000LL;
      uint64_t imm10_5 =
          instructionEncoding & 0b111111000000000000000000000000LL;
      uint64_t rs2 = instructionEncoding & 0b111110000000000000000000LL;
      uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
      uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
      uint64_t imm4_1 = instructionEncoding & 0b11110000000LL;
      uint64_t imm11 = instructionEncoding & 0b1000000LL;
    } break;

    case 0b0110111LL: // U Type
    {
      uint64_t imm31_12 =
          instructionEncoding & 0b11111111111111111111000000000000LL;
      uint64_t rd = instructionEncoding & 0b11111000000LL;
    } break;

    case 0b1101111LL: // J Type
    {
      uint64_t imm20 =
          instructionEncoding & 0b10000000000000000000000000000000LL;
      uint64_t imm10_1 =
          instructionEncoding & 0b111111111100000000000000000000LL;
      uint64_t imm11 = instructionEncoding & 0b10000000000000000000LL;
      uint64_t imm19_12 = instructionEncoding & 0b1111111100000000000LL;
      uint64_t rd = instructionEncoding & 0b11111000000LL;
    } break;

    default:
      fprintf(stderr, "Invalid instruction %ld:\n", instructionEncoding);
      abort();
    }

    instructionEncoding += uncompressionInstruction;
  } else {
    instructionEncoding += compressionInstruction;
  }

  fprintf(stderr, "%04lx     %ld    %s:\n", instructionIndex,
          instructionEncoding, instStr);
}

int disassembleText(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Disassembly of section .text:\n");

  if (!shdrTextOff) {
    fprintf(stderr, "\n");
    return 0;
  }

  char *strBuffer = (char *)malloc(shdrTextSize);
  char *strBufferEnd = strBuffer + shdrTextSize;

  if (!fseek(fileHandle, shdrTextOff, SEEK_SET))
    fread(strBuffer, shdrTextSize, 1, fileHandle);

  while (strBuffer != strBufferEnd) {
    uint64_t instructionEncoding =
        byte_get_little_endian(strBuffer, uncompressionInstruction);
    if (!(~instructionEncoding & 3LL)) {
      decode(instructionEncoding, uncompressionInstruction);
      strBuffer += uncompressionInstruction;
    } else {
      decode(instructionEncoding, compressionInstruction);
      strBuffer += compressionInstruction;
    }
  }

  closeFile(fileHandle);

  fprintf(stderr, "\n\n");
  return 0;
}
