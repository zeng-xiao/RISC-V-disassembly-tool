#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf.h"

#include "dataOperate.h"

#include "instructions.def"

extern uint64_t shdrTextOff;
extern uint64_t shdrTextSize;

static uint8_t compressionInstruction = 2;
static uint8_t uncompressionInstruction = 4;

static int32_t instructionIndex = 0;

static const char *registerAbiName[32] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

static void rTypeInstruction(int32_t instructionEncoding, const char *instStr) {
  int32_t funct7 = instructionEncoding & 0b1111111000000000000000000000000;
  uint32_t rs2 = instructionEncoding & 0b111110000000000000000000;
  uint32_t rs1 = instructionEncoding & 0b1111100000000000000;
  uint32_t funct3 = instructionEncoding & 0b11100000000000;
  uint32_t rd = instructionEncoding & 0b11111000000;

  int32_t instruction = funct7 << 3 | funct3;
  char str[80];

  switch (instruction) {
  case 0b0000000000:
    strcat(str, "add   ");
    break;
  case 0b0100000000:
    strcat(str, "sub   ");
    break;
  case 0b0000000001:
    strcat(str, "sll   ");
    break;
  case 0b0000000010:
    strcat(str, "slt   ");
    break;
  case 0b0000000011:
    strcat(str, "sltu  ");
    break;
  case 0b0000000100:
    strcat(str, "xor   ");
    break;
  case 0b0000000101:
    strcat(str, "srl   ");
    break;
  case 0b0100000101:
    strcat(str, "sra   ");
    break;
  case 0b0000000110:
    strcat(str, "or    ");
    break;
  case 0b0000000111:
    strcat(str, "and   ");
    break;

  default:
    fprintf(stderr, "Invalid instruction %u:\n", instruction);
    abort();
  }
  strcat(str, registerAbiName[rd]);
  strcat(str, ",");
  strcat(str, registerAbiName[rs2]);
  strcat(str, ",");
  strcat(str, registerAbiName[rs1]);
  instStr = str;
}

static const char *iTypeInstruction(int32_t opcode,
                                    int32_t instructionEncoding) {
  char *str = calloc(80, sizeof(uint8_t));
  char str_imm11_0[3];

  int32_t imm11_0 = instructionEncoding >> 20;
  uint32_t rs1 = (instructionEncoding >> 15) & 0b11111;
  uint32_t funct3 = (instructionEncoding >> 12) & 0b111;
  uint32_t rd = (instructionEncoding >> 7) & 0b11111;

  int32_t jalrFlag = 0;

  int32_t funct7 = imm11_0 & 0b1111111000000000000000000000000;
  int32_t shamt = imm11_0 & 0b111110000000000000000000;
  int32_t shiftFlag = 0;

  int32_t fenceFlag1 = 0;
  int32_t fenceFlag2 = 0;
  int32_t pred = imm11_0 & 0b1111000000000000000000000000000;
  int32_t succ = imm11_0 & 0b111100000000000000000000000;

  int32_t csrFlag1 = 0;
  int32_t csrFlag2 = 0;
  int32_t zimm4_0 = rs1;

  if (opcode == 0b1100111LL) {
    jalrFlag = 1;
    strcat(str, "jalr  ");
  }

  if (opcode == 0b0000011LL) {
    uint32_t instruction = funct3;
    switch (instruction) {
    case 0b000:
      strcat(str, "lb ");
      break;
    case 0b001:
      strcat(str, "lh    ");
      break;
    case 0b010:
      strcat(str, "lw    ");
      break;
    case 0b100:
      strcat(str, "lbu   ");
      break;
    case 0b101:
      strcat(str, "lhu   ");
      break;
      /*rv64I*/
    case 0b011:
      strcat(str, "ld    ");
      break;
    default:
      fprintf(stderr, "Invalid instruction %u:\n", instruction);
      abort();
    }
  }

  if (opcode == 0b0010011LL) {
    uint32_t instruction = funct3;
    switch (instruction) {
    case 0b000:
      strcat(str, "addi  ");
      break;
    case 0b010:
      strcat(str, "slti  ");
      break;
    case 0b011:
      strcat(str, "sltiu ");
      break;
    case 0b100:
      strcat(str, "xori  ");
      break;
    case 0b110:
      strcat(str, "ori   ");
      break;
    case 0b111:
      strcat(str, "andi  ");
      break;
    case 0b001:
    case 0b101: {
      shiftFlag = 1;
      if (instruction == 0b001LL && funct7 == 0b0000000LL)
        strcat(str, "slli ");
      else if (instruction == 0b101LL && funct7 == 0b0000000LL)
        strcat(str, "srli ");
      else if (instruction == 0b101LL && funct7 == 0b0100000LL)
        strcat(str, "srai ");
      else {
        fprintf(stderr, " Invalid instruction: funct3 = %u, funct7 = %u\n",
                funct3, funct7);
        abort();
      }
    } break;
    default:
      fprintf(stderr, "Invalid instruction %u:\n", instruction);
      abort();
    }
  }

  if (opcode == 0b0001111LL) {
    uint32_t instruction = funct3;

    switch (instruction) {
    case 0b000:
      fenceFlag1 = 1;
      strcat(str, "fence ");
      break;
    case 0b001:
      fenceFlag2 = 1;
      strcat(str, "fence.i ");
      break;
    default:
      fprintf(stderr, "Invalid instruction %u:\n", instruction);
      abort();
    }
  }

  if (opcode == 0b1110011LL) {
    if (funct3 == 0b000LL) {
      int32_t instruction = imm11_0;
      switch (instruction) {

      case 0b000000000000:
        strcat(str, "ecall ");
        break;
      case 0b000000100000:
        strcat(str, "ebreak ");
        break;
      default:
        fprintf(stderr, "Invalid instruction %u:\n", instruction);
        abort();
      }
    } else {
      uint32_t instruction = funct3;
      switch (instruction) {
      case 0b001:
      case 0b010:
      case 0b011: {
        if (instruction == 0b001LL)
          strcat(str, "csrrw ");
        else if (instruction == 0b001LL)
          strcat(str, "csrrs ");
        else if (instruction == 0b001LL)
          strcat(str, "csrrc ");
        else {
          fprintf(stderr, "Invalid instruction %u:\n", instruction);
          abort();
        }
        csrFlag1 = 1;
      } break;

      case 0b101:
      case 0b110:
      case 0b111: {
        csrFlag2 = 1;
        if (instruction == 0b101LL)
          strcat(str, "csrrwi ");
        else if (instruction == 0b101LL)
          strcat(str, "cssrrsi ");
        else if (instruction == 0b101LL)
          strcat(str, "csrrci ");
        else {
          fprintf(stderr, "Invalid instruction %u:\n", instruction);
          abort();
        }
      } break;
      default:
        fprintf(stderr, "Invalid instruction %u:\n", instruction);
        abort();
      }
    }
  }

  if (jalrFlag) {
    sprintf(str_imm11_0, "%d", imm11_0);

    strcat(str, registerAbiName[rd]);
    strcat(str, ",");
    strcat(str, registerAbiName[rs1]);
    strcat(str, ",");
    strcat(str, str_imm11_0);
  } else if (shiftFlag) {
    char *str_shamt;
    sprintf(str_shamt, "%u", shamt);

    strcat(str, registerAbiName[rd]);
    strcat(str, ",");
    strcat(str, registerAbiName[rs1]);
    strcat(str, ",");
    strcat(str, str_shamt);
  } else if (fenceFlag1) {
    char *str_pred;
    char *str_succ;
    sprintf(str_pred, "%u", pred);
    sprintf(str_succ, "%u", succ);

    strcat(str, str_pred);
    strcat(str, ",");
    strcat(str, str_succ);
  } else if (fenceFlag2) {
    ;
  } else if (csrFlag1) {

    strcat(str, registerAbiName[rd]);
    strcat(str, ",");
    strcat(str, "csr");
    strcat(str, ",");
    strcat(str, registerAbiName[rs1]);
  } else if (csrFlag2) {
    char *str_zimm4_0;
    sprintf(str_zimm4_0, "%u", zimm4_0);

    strcat(str, registerAbiName[rd]);
    strcat(str, ",");
    strcat(str, "csr");
    strcat(str, ",");
    strcat(str, str_zimm4_0);
  } else {
    if (imm11_0 < 0)
      sprintf(str_imm11_0, "%u", abs(imm11_0));
    else
      sprintf(str_imm11_0, "%u", imm11_0);
    strcat(str, registerAbiName[rd]);
    strcat(str, ",");
    if (imm11_0 < 0)
      strcat(str, "-");
    strcat(str, str_imm11_0);
    strcat(str, "(");
    strcat(str, registerAbiName[rs1]);
    strcat(str, ")");
  }
  return str;
}

static void sTypeInstruction(int32_t instructionEncoding, void **instStr) {

  int32_t imm4_0 = (instructionEncoding >> 7) & 0b11111;
  uint32_t funct3 = (instructionEncoding >> 12) & 0b111;
  uint32_t rs1 = (instructionEncoding >> 15) & 0b11111;
  uint32_t rs2 = (instructionEncoding >> 20) & 0b11111;
  int32_t imm11_5 = (instructionEncoding >> 25);

  int32_t imm = imm11_5 << 5 | imm4_0;

  uint32_t instruction = funct3;

  char *str = calloc(80, sizeof(uint8_t));

  char str_imm[6];
  sprintf(str_imm, "%d", imm);

  switch (instruction) {
  case 0b000:
    strcat(str, "sb    ");
    break;
  case 0b001L:
    strcat(str, "sh    ");
    break;
  case 0b010L:
    strcat(str, "sw    ");
    break;

    /*rv64*/
  case 0b011L:
    strcat(str, "sd    ");
    break;
  default:
    fprintf(stderr, "Invalid instruction %u:\n", instruction);
    abort();
  }
  strcat(str, registerAbiName[rs2]);
  strcat(str, ",");
  strcat(str, str_imm);
  strcat(str, "(");
  strcat(str, registerAbiName[rs1]);
  strcat(str, ")");
  *instStr = str;
}

static void bTypeInstruction(int32_t instructionEncoding, void **instStr) {

  int32_t imm12 = instructionEncoding & 0b1000000000000000000000000000000;
  int32_t imm10_5 = instructionEncoding & 0b111111000000000000000000000000;
  uint32_t rs2 = instructionEncoding & 0b111110000000000000000000;
  uint32_t rs1 = instructionEncoding & 0b1111100000000000000;
  uint32_t funct3 = instructionEncoding & 0b11100000000000;
  int32_t imm4_1 = instructionEncoding & 0b11110000000;
  int32_t imm11 = instructionEncoding & 0b1000000;

  int32_t imm = imm4_1 << 1 | imm10_5 << 5 | imm11 << 11 | imm12 << 12;

  uint32_t instruction = funct3;
  char str[80];

  char *str_imm;
  sprintf(str_imm, "%d", imm);

  char *registerAbiName[rs1];

  switch (instruction) {
  case 0b000:
    strcat(str, "beq   ");
    break;
  case 0b001:
    strcat(str, "bne   ");
    break;
  case 0b100:
    strcat(str, "blt   ");
    break;
  case 0b101:
    strcat(str, "bge   ");
    break;
  case 0b110:
    strcat(str, "bltu  ");
    break;
  case 0b111:
    strcat(str, "bgeu  ");
    break;
  default:
    fprintf(stderr, "Invalid instruction %u:\n", instruction);
    abort();
  }

  strcat(str, registerAbiName[rs1]);
  strcat(str, ",");
  strcat(str, registerAbiName[rs2]);
  strcat(str, ",");
  strcat(str, str_imm);

  *instStr = str;
}

static void uTypeInstruction(int32_t opcode, int32_t instructionEncoding,
                             void **instStr) {
  int32_t imm31_12 = instructionEncoding >> 12;
  uint32_t rd = (instructionEncoding >> 7) & 0b11111;

  static char str[80];

  if (opcode == 0b0110111LL)
    strcat(str, "lui   ");
  else if (opcode == 0b0010111LL)
    strcat(str, "auipc ");

  char str_imm31_12[12];

  sprintf(str_imm31_12, "%d", abs(imm31_12));
  sprintf(str_imm31_12, "%d", abs(imm31_12));

  strcat(str, registerAbiName[rd]);
  strcat(str, ",");
  if (imm31_12 < 0)
    strcat(str, "-");
  strcat(str, str_imm31_12);
  *instStr = str;
}

static void jTypeInstruction(int32_t instructionEncoding, void **instStr) {

  int32_t imm20 = instructionEncoding & 0b10000000000000000000000000000000;
  int32_t imm10_1 = instructionEncoding & 0b111111111100000000000000000000;
  int32_t imm11 = instructionEncoding & 0b10000000000000000000;
  int32_t imm19_12 = instructionEncoding & 0b1111111100000000000;
  uint32_t rd = instructionEncoding & 0b11111000000;

  int32_t imm = imm10_1 << 1 | imm11 << 11 | imm19_12 << 12 | imm20 << 20;
  static char str[80];

  char *str_imm;
  sprintf(str_imm, "%d", imm);

  strcat(str, registerAbiName[rd]);
  strcat(str, ",");
  strcat(str, str_imm);
  *instStr = str;
}

static void decode(int32_t instructionEncoding, unsigned char C) {
  const char *instStr;

  if (C == 4) {
    int32_t opcode = instructionEncoding & 0b1111111;
    switch (opcode) {
    case 0b0110011: // R
      rTypeInstruction(instructionEncoding, instStr);
      break;

    case 0b1100111: // I Type
    case 0b0000011:
    case 0b0010011:
    case 0b0001111:
    case 0b1110011:
      instStr = iTypeInstruction(opcode, instructionEncoding);
      break;

    case 0b0100011: // S Type
      sTypeInstruction(instructionEncoding, (void **)&instStr);
      break;

    case 0b1100011: // B Type
      bTypeInstruction(instructionEncoding, (void **)&instStr);
      break;

    case 0b0110111: // U Type
    case 0b0010111:
      uTypeInstruction(opcode, instructionEncoding, (void **)&instStr);
      break;

    case 0b1101111: // J Type
      jTypeInstruction(instructionEncoding, (void **)&instStr);
      break;

    default:
      fprintf(stderr, "Invalid instruction %u:\n", instructionEncoding);
      abort();
    }

  } else {
  }

  fprintf(stderr, "%04x:     %08x    %s\n", instructionIndex,
          instructionEncoding, instStr);

  if (C == 4)
    instructionIndex += uncompressionInstruction;
  else
    instructionIndex += compressionInstruction;
}

int disassembleText(const char *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Disassembly of section .text:\n");

  if (!shdrTextOff) {
    fprintf(stderr, "\n");
    return 0;
  }

  char *strBuffer = malloc(shdrTextSize);
  char *strBufferEnd = strBuffer + shdrTextSize;

  if (!fseek(fileHandle, shdrTextOff, SEEK_SET))
    fread(strBuffer, 1, shdrTextSize, fileHandle);

  while (strBuffer != strBufferEnd) {
    int32_t instructionEncoding =
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
