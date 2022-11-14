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

  char *outputInstStr = calloc(1024, sizeof(uint8_t));

  int32_t instruction = funct7 << 3 | funct3;

  switch (instruction) {
  case 0b0000000000:
    strcat(outputInstStr, "add   ");
    break;
  case 0b0100000000:
    strcat(outputInstStr, "sub   ");
    break;
  case 0b0000000001:
    strcat(outputInstStr, "sll   ");
    break;
  case 0b0000000010:
    strcat(outputInstStr, "slt   ");
    break;
  case 0b0000000011:
    strcat(outputInstStr, "sltu  ");
    break;
  case 0b0000000100:
    strcat(outputInstStr, "xor   ");
    break;
  case 0b0000000101:
    strcat(outputInstStr, "srl   ");
    break;
  case 0b0100000101:
    strcat(outputInstStr, "sra   ");
    break;
  case 0b0000000110:
    strcat(outputInstStr, "or    ");
    break;
  case 0b0000000111:
    strcat(outputInstStr, "and   ");
    break;

  default:
    fprintf(stderr, "Invalid instruction %u:\n", instruction);
    abort();
  }
  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs1]);

  instStr = outputInstStr;
}

static const char *iTypeInstruction(int32_t opcode,
                                    int32_t instructionEncoding) {
  char str_imm11_0[3];
  char *outputInstStr = calloc(1024, sizeof(uint8_t));

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
    strcat(outputInstStr, "jalr  ");
  }

  if (opcode == 0b0000011LL) {
    uint32_t instruction = funct3;
    switch (instruction) {
    case 0b000:
      strcat(outputInstStr, "lb ");
      break;
    case 0b001:
      strcat(outputInstStr, "lh    ");
      break;
    case 0b010:
      strcat(outputInstStr, "lw    ");
      break;
    case 0b100:
      strcat(outputInstStr, "lbu   ");
      break;
    case 0b101:
      strcat(outputInstStr, "lhu   ");
      break;
      /*rv64I*/
    case 0b011:
      strcat(outputInstStr, "ld    ");
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
      strcat(outputInstStr, "addi  ");
      break;
    case 0b010:
      strcat(outputInstStr, "slti  ");
      break;
    case 0b011:
      strcat(outputInstStr, "sltiu ");
      break;
    case 0b100:
      strcat(outputInstStr, "xori  ");
      break;
    case 0b110:
      strcat(outputInstStr, "ori   ");
      break;
    case 0b111:
      strcat(outputInstStr, "andi  ");
      break;
    case 0b001:
    case 0b101: {
      shiftFlag = 1;
      if (instruction == 0b001LL && funct7 == 0b0000000LL)
        strcat(outputInstStr, "slli ");
      else if (instruction == 0b101LL && funct7 == 0b0000000LL)
        strcat(outputInstStr, "srli ");
      else if (instruction == 0b101LL && funct7 == 0b0100000LL)
        strcat(outputInstStr, "srai ");
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
      strcat(outputInstStr, "fence ");
      break;
    case 0b001:
      fenceFlag2 = 1;
      strcat(outputInstStr, "fence.i ");
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
        strcat(outputInstStr, "ecall ");
        break;
      case 0b000000100000:
        strcat(outputInstStr, "ebreak ");
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
          strcat(outputInstStr, "csrrw ");
        else if (instruction == 0b001LL)
          strcat(outputInstStr, "csrrs ");
        else if (instruction == 0b001LL)
          strcat(outputInstStr, "csrrc ");
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
          strcat(outputInstStr, "csrrwi ");
        else if (instruction == 0b101LL)
          strcat(outputInstStr, "cssrrsi ");
        else if (instruction == 0b101LL)
          strcat(outputInstStr, "csrrci ");
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

    strcat(outputInstStr, registerAbiName[rd]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, registerAbiName[rs1]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, str_imm11_0);
  } else if (shiftFlag) {
    char *str_shamt;
    sprintf(str_shamt, "%u", shamt);

    strcat(outputInstStr, registerAbiName[rd]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, registerAbiName[rs1]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, str_shamt);
  } else if (fenceFlag1) {
    char *str_pred;
    char *str_succ;
    sprintf(str_pred, "%u", pred);
    sprintf(str_succ, "%u", succ);

    strcat(outputInstStr, str_pred);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, str_succ);
  } else if (fenceFlag2) {
    ;
  } else if (csrFlag1) {

    strcat(outputInstStr, registerAbiName[rd]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, "csr");
    strcat(outputInstStr, ",");
    strcat(outputInstStr, registerAbiName[rs1]);
  } else if (csrFlag2) {
    char *str_zimm4_0;
    sprintf(str_zimm4_0, "%u", zimm4_0);

    strcat(outputInstStr, registerAbiName[rd]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, "csr");
    strcat(outputInstStr, ",");
    strcat(outputInstStr, str_zimm4_0);
  } else {
    if (imm11_0 < 0)
      sprintf(str_imm11_0, "%u", abs(imm11_0));
    else
      sprintf(str_imm11_0, "%u", imm11_0);
    strcat(outputInstStr, registerAbiName[rd]);
    strcat(outputInstStr, ",");
    if (imm11_0 < 0)
      strcat(outputInstStr, "-");
    strcat(outputInstStr, str_imm11_0);
    strcat(outputInstStr, "(");
    strcat(outputInstStr, registerAbiName[rs1]);
    strcat(outputInstStr, ")");
  }
  return outputInstStr;
}

static void sTypeInstruction(int32_t instructionEncoding, void **instStr) {
  char *outputInstStr = calloc(1024, sizeof(uint8_t));
  int32_t imm4_0 = (instructionEncoding >> 7) & 0b11111;
  uint32_t funct3 = (instructionEncoding >> 12) & 0b111;
  uint32_t rs1 = (instructionEncoding >> 15) & 0b11111;
  uint32_t rs2 = (instructionEncoding >> 20) & 0b11111;
  int32_t imm11_5 = (instructionEncoding >> 25);

  int32_t imm = imm11_5 << 5 | imm4_0;

  uint32_t instruction = funct3;

  char str_imm[6];
  sprintf(str_imm, "%d", imm);

  switch (instruction) {
  case 0b000:
    strcat(outputInstStr, "sb    ");
    break;
  case 0b001L:
    strcat(outputInstStr, "sh    ");
    break;
  case 0b010L:
    strcat(outputInstStr, "sw    ");
    break;

    /*rv64*/
  case 0b011L:
    strcat(outputInstStr, "sd    ");
    break;
  default:
    fprintf(stderr, "Invalid instruction %u:\n", instruction);
    abort();
  }
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, str_imm);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ")");
  *instStr = outputInstStr;
}

static void bTypeInstruction(int32_t instructionEncoding, void **instStr) {
  char *outputInstStr = calloc(1024, sizeof(uint8_t));
  int32_t imm12 = instructionEncoding & 0b1000000000000000000000000000000;
  int32_t imm10_5 = instructionEncoding & 0b111111000000000000000000000000;
  uint32_t rs2 = instructionEncoding & 0b111110000000000000000000;
  uint32_t rs1 = instructionEncoding & 0b1111100000000000000;
  uint32_t funct3 = instructionEncoding & 0b11100000000000;
  int32_t imm4_1 = instructionEncoding & 0b11110000000;
  int32_t imm11 = instructionEncoding & 0b1000000;

  int32_t imm = imm4_1 << 1 | imm10_5 << 5 | imm11 << 11 | imm12 << 12;

  uint32_t instruction = funct3;

  char *str_imm;
  sprintf(str_imm, "%d", imm);

  char *registerAbiName[rs1];

  switch (instruction) {
  case 0b000:
    strcat(outputInstStr, "beq   ");
    break;
  case 0b001:
    strcat(outputInstStr, "bne   ");
    break;
  case 0b100:
    strcat(outputInstStr, "blt   ");
    break;
  case 0b101:
    strcat(outputInstStr, "bge   ");
    break;
  case 0b110:
    strcat(outputInstStr, "bltu  ");
    break;
  case 0b111:
    strcat(outputInstStr, "bgeu  ");
    break;
  default:
    fprintf(stderr, "Invalid instruction %u:\n", instruction);
    abort();
  }

  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, str_imm);

  *instStr = outputInstStr;
}

static void uTypeInstruction(int32_t opcode, int32_t instructionEncoding,
                             void **instStr) {
  int32_t imm31_12 = instructionEncoding >> 12;
  uint32_t rd = (instructionEncoding >> 7) & 0b11111;
  char *outputInstStr = calloc(1024, sizeof(uint8_t));
  if (opcode == 0b0110111LL)
    strcat(outputInstStr, "lui   ");
  else if (opcode == 0b0010111LL)
    strcat(outputInstStr, "auipc ");

  char str_imm31_12[12];

  sprintf(str_imm31_12, "%d", abs(imm31_12));
  sprintf(str_imm31_12, "%d", abs(imm31_12));

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm31_12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, str_imm31_12);
  *instStr = outputInstStr;
}

static void jTypeInstruction(int32_t instructionEncoding, void **instStr) {
  int32_t imm20 = instructionEncoding & 0b10000000000000000000000000000000;
  int32_t imm10_1 = instructionEncoding & 0b111111111100000000000000000000;
  int32_t imm11 = instructionEncoding & 0b10000000000000000000;
  int32_t imm19_12 = instructionEncoding & 0b1111111100000000000;
  uint32_t rd = instructionEncoding & 0b11111000000;

  int32_t imm = imm10_1 << 1 | imm11 << 11 | imm19_12 << 12 | imm20 << 20;
  char *outputInstStr = calloc(1024, sizeof(uint8_t));
  char *str_imm;
  sprintf(str_imm, "%d", imm);

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, str_imm);
  *instStr = outputInstStr;
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
