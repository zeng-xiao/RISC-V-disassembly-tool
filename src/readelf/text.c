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

static void rTypeInstruction(uint64_t instructionEncoding,
                             const char *instStr) {

  uint64_t funct7 = instructionEncoding & 0b1111111000000000000000000000000LL;
  uint64_t rs2 = instructionEncoding & 0b111110000000000000000000LL;
  uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
  uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
  uint64_t rd = instructionEncoding & 0b11111000000LL;

  uint64_t instruction = funct7 << 3 | funct3;
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
    break;
  case 0b0100000000LL:
    strcpy(str, "sub ");
    break;
  case 0b0000000001LL:
    strcpy(str, "sll ");
    break;
  case 0b0000000010LL:
    strcpy(str, "slt ");
    break;
  case 0b0000000011LL:
    strcpy(str, "sltu ");
    break;
  case 0b0000000100LL:
    strcpy(str, "xor ");
    break;
  case 0b0000000101LL:
    strcpy(str, "srl ");
    break;
  case 0b0100000101LL:
    strcpy(str, "sra ");
    break;
  case 0b0000000110LL:
    strcpy(str, "or ");
    break;
  case 0b0000000111LL:
    strcpy(str, "and ");
    break;

  default:
    fprintf(stderr, "Invalid instruction %ld:\n", instruction);
    abort();
  }
  strcpy(str, str_rd);
  strcpy(str, ",");
  strcpy(str, str_rs2);
  strcpy(str, ",");
  strcpy(str, str_rs1);
  instStr = str;
}

static void iTypeInstruction(uint64_t opcode, uint64_t instructionEncoding,
                             const char *instStr) {
  char str[80];
  char *str_rd;
  char *str_rs1;
  char *str_imm11_0;

  uint64_t imm11_0 = instructionEncoding & 0b1111111111110000000000000000000LL;
  uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
  uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
  uint64_t rd = instructionEncoding & 0b11111000000LL;

  uint64_t jalrFlag = 0;

  uint64_t funct7 = imm11_0 & 0b1111111000000000000000000000000LL;
  uint64_t shamt = imm11_0 & 0b111110000000000000000000LL;
  uint64_t shiftFlag = 0;

  uint64_t fenceFlag1 = 0;
  uint64_t fenceFlag2 = 0;
  uint64_t pred = imm11_0 & 0b1111000000000000000000000000000LL;
  uint64_t succ = imm11_0 & 0b111100000000000000000000000LL;

  uint64_t csrFlag1 = 0;
  uint64_t csrFlag2 = 0;
  uint64_t zimm4_0 = rs1;

  if (opcode == 0b1100111LL) {
    jalrFlag = 1;
    strcpy(str, "jalr ");
  }

  if (opcode == 0b0000011LL) {
    uint64_t instruction = funct3;
    switch (instruction) {
    case 0b000LL:
      strcpy(str, "lb ");
      break;
    case 0b001LL:
      strcpy(str, "lh ");
      break;
    case 0b010LL:
      strcpy(str, "lw ");
      break;
    case 0b100LL:
      strcpy(str, "lbu ");
      break;
    case 0b101LL:
      strcpy(str, "lhu ");
      break;
    default:
      fprintf(stderr, "Invalid instruction %ld:\n", instruction);
      abort();
    }
  }

  if (opcode == 0b0010011LL) {
    uint64_t instruction = funct3;
    switch (instruction) {
    case 0b000LL:
      strcpy(str, "addi ");
      break;
    case 0b010LL:
      strcpy(str, "slti ");
      break;
    case 0b011LL:
      strcpy(str, "sltiu ");
      break;
    case 0b100LL:
      strcpy(str, "xori ");
      break;
    case 0b110LL:
      strcpy(str, "ori ");
      break;
    case 0b111LL:
      strcpy(str, "andi ");
      break;
    case 0b001LL:
    case 0b101LL: {
      shiftFlag = 1;
      if (instruction == 0b001LL && funct7 == 0b0000000LL)
        strcpy(str, "slli ");
      else if (instruction == 0b101LL && funct7 == 0b0000000LL)
        strcpy(str, "srli ");
      else if (instruction == 0b101LL && funct7 == 0b0100000LL)
        strcpy(str, "srai ");
      else {
        fprintf(stderr, " Invalid instruction: funct3 = %ld, funct7 = %ld\n",
                funct3, funct7);
        abort();
      }
    } break;
    default:
      fprintf(stderr, "Invalid instruction %ld:\n", instruction);
      abort();
    }
  }

  if (opcode == 0b0001111LL) {
    uint64_t instruction = funct3;

    switch (instruction) {
    case 0b000LL:
      fenceFlag1 = 1;
      strcpy(str, "fence ");
      break;
    case 0b001LL:
      fenceFlag2 = 1;
      strcpy(str, "fence.i ");
      break;
    default:
      fprintf(stderr, "Invalid instruction %ld:\n", instruction);
      abort();
    }
  }

  if (opcode == 0b1110011LL) {
    if (funct3 == 0b000LL) {
      uint64_t instruction = imm11_0;
      switch (instruction) {

      case 0b000000000000LL:
        strcpy(str, "ecall ");
        break;
      case 0b000000100000LL:
        strcpy(str, "ebreak ");
        break;
      default:
        fprintf(stderr, "Invalid instruction %ld:\n", instruction);
        abort();
      }
    } else {
      uint64_t instruction = funct3;
      switch (instruction) {
      case 0b001LL:
      case 0b010LL:
      case 0b011LL: {
        if (instruction == 0b001LL)
          strcpy(str, "csrrw ");
        else if (instruction == 0b001LL)
          strcpy(str, "csrrs ");
        else if (instruction == 0b001LL)
          strcpy(str, "csrrc ");
        else {
          fprintf(stderr, "Invalid instruction %ld:\n", instruction);
          abort();
        }
        csrFlag1 = 1;
      } break;

      case 0b101LL:
      case 0b110LL:
      case 0b111LL: {
        csrFlag2 = 1;
        if (instruction == 0b101LL)
          strcpy(str, "csrrwi ");
        else if (instruction == 0b101LL)
          strcpy(str, "cssrrsi ");
        else if (instruction == 0b101LL)
          strcpy(str, "csrrci ");
        else {
          fprintf(stderr, "Invalid instruction %ld:\n", instruction);
          abort();
        }
      } break;
      default:
        fprintf(stderr, "Invalid instruction %ld:\n", instruction);
        abort();
      }
    }
  }

  if (jalrFlag) {
    sprintf(str_rd, "%lu", rd);
    sprintf(str_rs1, "%lu", rs1);
    sprintf(str_imm11_0, "%lu", imm11_0);

    strcpy(str, str_rd);
    strcpy(str, ",");
    strcpy(str, str_rs1);
    strcpy(str, ",");
    strcpy(str, str_imm11_0);
  } else if (shiftFlag) {
    char *str_shamt;
    sprintf(str_rd, "%lu", rd);
    sprintf(str_rs1, "%lu", rs1);
    sprintf(str_shamt, "%lu", shamt);

    strcpy(str, str_rd);
    strcpy(str, ",");
    strcpy(str, str_rs1);
    strcpy(str, ",");
    strcpy(str, str_shamt);
  } else if (fenceFlag1) {
    char *str_pred;
    char *str_succ;
    sprintf(str_pred, "%lu", pred);
    sprintf(str_succ, "%lu", succ);

    strcpy(str, str_pred);
    strcpy(str, ",");
    strcpy(str, str_succ);
  } else if (fenceFlag2) {
    ;
  } else if (csrFlag1) {
    sprintf(str_rd, "%lu", rd);
    sprintf(str_rs1, "%lu", rs1);

    strcpy(str, str_rd);
    strcpy(str, ",");
    strcpy(str, "csr");
    strcpy(str, ",");
    strcpy(str, str_rs1);
  } else if (csrFlag2) {
    char *str_zimm4_0;
    sprintf(str_rd, "%lu", rd);
    sprintf(str_rs1, "%lu", rs1);
    sprintf(str_zimm4_0, "%lu", zimm4_0);

    strcpy(str, str_rd);
    strcpy(str, ",");
    strcpy(str, "csr");
    strcpy(str, ",");
    strcpy(str, str_zimm4_0);
  }
  instStr = str;
}

static void sTypeInstruction(uint64_t instructionEncoding,
                             const char *instStr) {

  uint64_t imm11_5 = instructionEncoding & 0b1111111000000000000000000000000LL;
  uint64_t rs2 = instructionEncoding & 0b111110000000000000000000LL;
  uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
  uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
  uint64_t imm4_0 = instructionEncoding & 0b11111000000LL;

  uint64_t imm = imm11_5 << 5 | imm4_0;

  uint64_t instruction = funct3;

  char str[80];

  char *str_imm;
  sprintf(str_imm, "%lu", imm);

  char *str_rs1;
  sprintf(str_rs1, "%lu", rs1);

  char *str_rs2;
  sprintf(str_rs2, "%lu", rs2);

  switch (instruction) {
  case 0b000LL:
    strcpy(str, "sb ");
    break;
  case 0b001L:
    strcpy(str, "sh ");
    break;
  case 0b010L:
    strcpy(str, "sw ");
    break;
  default:
    fprintf(stderr, "Invalid instruction %ld:\n", instruction);
    abort();
  }
  strcpy(str, str_rs2);
  strcpy(str, ",");
  strcpy(str, str_imm);
  strcpy(str, "(");
  strcpy(str, str_rs1);
  strcpy(str, ")");
  instStr = str;
}

static void bTypeInstruction(uint64_t instructionEncoding,
                             const char *instStr) {

  uint64_t imm12 = instructionEncoding & 0b1000000000000000000000000000000LL;
  uint64_t imm10_5 = instructionEncoding & 0b111111000000000000000000000000LL;
  uint64_t rs2 = instructionEncoding & 0b111110000000000000000000LL;
  uint64_t rs1 = instructionEncoding & 0b1111100000000000000LL;
  uint64_t funct3 = instructionEncoding & 0b11100000000000LL;
  uint64_t imm4_1 = instructionEncoding & 0b11110000000LL;
  uint64_t imm11 = instructionEncoding & 0b1000000LL;

  uint64_t imm = imm4_1 << 1 | imm10_5 << 5 | imm11 << 11 | imm12 << 12;

  uint64_t instruction = funct3;
  char str[80];

  char *str_imm;
  sprintf(str_imm, "%lu", imm);

  char *str_rs1;
  sprintf(str_rs1, "%lu", rs1);

  char *str_rs2;
  sprintf(str_rs2, "%lu", rs2);

  switch (instruction) {
  case 0b000LL:
    strcpy(str, "beq ");
    break;
  case 0b001LL:
    strcpy(str, "bne ");
    break;
  case 0b100LL:
    strcpy(str, "blt ");
    break;
  case 0b101LL:
    strcpy(str, "bge ");
    break;
  case 0b110LL:
    strcpy(str, "bltu ");
    break;
  case 0b111LL:
    strcpy(str, "bgeu ");
    break;
  default:
    fprintf(stderr, "Invalid instruction %ld:\n", instruction);
    abort();
  }

  strcpy(str, str_rs1);
  strcpy(str, ",");
  strcpy(str, str_rs2);
  strcpy(str, ",");
  strcpy(str, str_imm);

  instStr = str;
}

static void uTypeInstruction(uint64_t opcode, uint64_t instructionEncoding,
                             const char *instStr) {
  uint64_t imm31_12 =
      instructionEncoding & 0b11111111111111111111000000000000LL;
  uint64_t rd = instructionEncoding & 0b11111000000LL;

  char str[80];

  if (opcode == 0b0110111LL)
    strcpy(str, "lui ");
  else if (opcode == 0b0010111LL)
    strcpy(str, "auipc ");

  char *str_rd;
  sprintf(str_rd, "%lu", rd);

  char *str_imm31_12;
  sprintf(str_imm31_12, "%lu", imm31_12);

  strcpy(str, str_rd);
  strcpy(str, ",");
  strcpy(str, str_imm31_12);
  instStr = str;
}

static void jTypeInstruction(uint64_t instructionEncoding,
                             const char *instStr) {

  uint64_t imm20 = instructionEncoding & 0b10000000000000000000000000000000LL;
  uint64_t imm10_1 = instructionEncoding & 0b111111111100000000000000000000LL;
  uint64_t imm11 = instructionEncoding & 0b10000000000000000000LL;
  uint64_t imm19_12 = instructionEncoding & 0b1111111100000000000LL;
  uint64_t rd = instructionEncoding & 0b11111000000LL;

  uint64_t imm = imm10_1 << 1 | imm11 << 11 | imm19_12 << 12 | imm20 << 20;
  char str[80];

  char *str_rd;
  sprintf(str_rd, "%lu", rd);

  char *str_imm;
  sprintf(str_imm, "%lu", imm);

  strcpy(str, str_rd);
  strcpy(str, ",");
  strcpy(str, str_imm);
  instStr = str;
}

static void decode(uint64_t instructionEncoding, unsigned char C) {
  const char *instStr;

  if (C == 4) {
    uint64_t opcode = instructionEncoding & 0b1111111LL;
    switch (opcode) {
    case 0b0110011LL: // R
      rTypeInstruction(instructionEncoding, instStr);
      break;

    case 0b1100111LL: // I Type
    case 0b0000011LL:
    case 0b0010011LL:
    case 0b0001111LL:
    case 0b1110011LL:
      iTypeInstruction(opcode, instructionEncoding, instStr);
      break;

    case 0b0100011LL: // S Type
      sTypeInstruction(instructionEncoding, instStr);
      break;

    case 0b1100011LL: // B Type
      bTypeInstruction(instructionEncoding, instStr);
      break;

    case 0b0110111LL: // U Type
    case 0b0010111LL:
      uTypeInstruction(opcode, instructionEncoding, instStr);
      break;

    case 0b1101111LL: // J Type
      jTypeInstruction(instructionEncoding, instStr);
      break;

    default:
      fprintf(stderr, "Invalid instruction %ld:\n", instructionEncoding);
      abort();
    }

    instructionEncoding += uncompressionInstruction;
    instructionIndex += uncompressionInstruction;
  } else {
    instructionEncoding += compressionInstruction;
    instructionIndex += compressionInstruction;
  }

  fprintf(stderr, "%04lx:     %lx    %s:\n", instructionIndex,
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
