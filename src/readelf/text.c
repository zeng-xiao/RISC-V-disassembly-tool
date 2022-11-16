#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf.h"

#include "dataOperate.h"

#include "instructions.def"

extern uint64_t shdrTextOff;
extern uint64_t shdrTextSize;

static uint8_t compressionInstLen = 2;
static uint8_t uncompressionInstLen = 4;

static int32_t instructionIndex = 0;

static const uint8_t *registerAbiName[32] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

static void rTypeInst(uint32_t instNoOpcode, uint8_t opcode,
                      const uint8_t **instStr) {
  int32_t funct7 = instNoOpcode & 0b1111111000000000000000000000000;
  uint32_t rs2 = instNoOpcode & 0b111110000000000000000000;
  uint32_t rs1 = instNoOpcode & 0b1111100000000000000;
  uint32_t funct3 = instNoOpcode & 0b11100000000000;
  uint32_t rd = instNoOpcode & 0b11111000000;

  uint8_t *outputInstStr = calloc(1024, sizeof(uint8_t));

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

  *instStr = outputInstStr;
}

static const uint8_t *iTypeInst(uint32_t instNoOpcode, uint8_t opcode,
                                const uint8_t **instStr) {
  uint8_t str_imm11_0[3];
  uint8_t *outputInstStr = calloc(1024, sizeof(uint8_t));

  int32_t imm11_0 = instNoOpcode >> 20;
  uint32_t rs1 = (instNoOpcode >> 15) & 0b11111;
  uint32_t funct3 = (instNoOpcode >> 12) & 0b111;
  uint32_t rd = (instNoOpcode >> 7) & 0b11111;

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
    switch (funct3) {
    case 0b000:
      strcat(outputInstStr, "lb    ");
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
      fprintf(stderr, "Invalid instruction funct3: %u\n", funct3);
      abort();
    }
  }

  if (opcode == 0b0010011LL) {
    switch (funct3) {
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
      if (funct3 == 0b001LL && funct7 == 0b0000000LL)
        strcat(outputInstStr, "slli ");
      else if (funct3 == 0b101LL && funct7 == 0b0000000LL)
        strcat(outputInstStr, "srli ");
      else if (funct3 == 0b101LL && funct7 == 0b0100000LL)
        strcat(outputInstStr, "srai ");
      else {
        fprintf(stderr, " Invalid instruction funct3:%u, funct7:%u\n", funct3,
                funct7);
        abort();
      }
    } break;
    default:
      fprintf(stderr, "Invalid instruction %u:\n", instNoOpcode);
      abort();
    }
  }

  if (opcode == 0b0001111LL) {
    switch (funct3) {
    case 0b000:
      fenceFlag1 = 1;
      strcat(outputInstStr, "fence ");
      break;
    case 0b001:
      fenceFlag2 = 1;
      strcat(outputInstStr, "fence.i ");
      break;
    default:
      fprintf(stderr, "Invalid instruction funct3: %u\n", funct3);
      abort();
    }
  }

  if (opcode == 0b1110011LL) {
    if (funct3 == 0b000LL) {
      switch (imm11_0) {
      case 0b000000000000:
        strcat(outputInstStr, "ecall ");
        break;
      case 0b000000100000:
        strcat(outputInstStr, "ebreak ");
        break;
      default:
        fprintf(stderr, "Invalid instruction funct3: %u\n", funct3);
        abort();
      }
    } else {
      switch (funct3) {
      case 0b001:
      case 0b010:
      case 0b011: {
        if (funct3 == 0b001LL)
          strcat(outputInstStr, "csrrw ");
        else if (funct3 == 0b001LL)
          strcat(outputInstStr, "csrrs ");
        else if (funct3 == 0b001LL)
          strcat(outputInstStr, "csrrc ");
        else {
          fprintf(stderr, "Invalid instruction funct3: %u\n", funct3);
          abort();
        }
        csrFlag1 = 1;
      } break;

      case 0b101:
      case 0b110:
      case 0b111: {
        csrFlag2 = 1;
        if (funct3 == 0b101LL)
          strcat(outputInstStr, "csrrwi ");
        else if (funct3 == 0b101LL)
          strcat(outputInstStr, "cssrrsi ");
        else if (funct3 == 0b101LL)
          strcat(outputInstStr, "csrrci ");
        else {
          fprintf(stderr, "Invalid instruction funct3: %u\n", funct3);
          abort();
        }
      } break;
      default:
        fprintf(stderr, "Invalid instruction funct3: %u\n", funct3);
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
    uint8_t *str_shamt;
    sprintf(str_shamt, "%u", shamt);

    strcat(outputInstStr, registerAbiName[rd]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, registerAbiName[rs1]);
    strcat(outputInstStr, ",");
    strcat(outputInstStr, str_shamt);
  } else if (fenceFlag1) {
    uint8_t *str_pred;
    uint8_t *str_succ;
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
    uint8_t *str_zimm4_0;
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

static void sTypeInst(uint32_t instNoOpcode, uint8_t opcode,
                      const uint8_t **instStr) {
  uint8_t *outputInstStr = calloc(1024, sizeof(uint8_t));
  int32_t imm4_0 = (instNoOpcode >> 7) & 0b11111;
  uint32_t funct3 = (instNoOpcode >> 12) & 0b111;
  uint32_t rs1 = (instNoOpcode >> 15) & 0b11111;
  uint32_t rs2 = (instNoOpcode >> 20) & 0b11111;
  int32_t imm11_5 = (instNoOpcode >> 25);

  int32_t imm = imm11_5 << 5 | imm4_0;

  uint8_t str_imm[6];
  sprintf(str_imm, "%d", imm);

  switch (funct3) {
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
    fprintf(stderr, "Invalid instruction %u:\n", instNoOpcode);
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

static void bTypeInst(uint32_t instNoOpcode, uint8_t opcode,
                      const uint8_t **instStr) {
  uint8_t *outputInstStr = calloc(1024, sizeof(uint8_t));
  int32_t imm12 = instNoOpcode & 0b1000000000000000000000000000000;
  int32_t imm10_5 = instNoOpcode & 0b111111000000000000000000000000;
  uint32_t rs2 = instNoOpcode & 0b111110000000000000000000;
  uint32_t rs1 = instNoOpcode & 0b1111100000000000000;
  uint32_t funct3 = instNoOpcode & 0b11100000000000;
  int32_t imm4_1 = instNoOpcode & 0b11110000000;
  int32_t imm11 = instNoOpcode & 0b1000000;

  int32_t imm = imm4_1 << 1 | imm10_5 << 5 | imm11 << 11 | imm12 << 12;

  uint8_t *str_imm;
  sprintf(str_imm, "%d", imm);

  uint8_t *registerAbiName[rs1];

  switch (funct3) {
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
    fprintf(stderr, "Invalid instruction %u:\n", instNoOpcode);
    abort();
  }

  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, str_imm);

  *instStr = outputInstStr;
}

static void uTypeInst(uint32_t instNoOpcode, uint8_t opcode,
                      const uint8_t **instStr) {
  int32_t imm31_12 = instNoOpcode >> 12;
  uint32_t rd = (instNoOpcode >> 7) & 0b11111;
  uint8_t *outputInstStr = calloc(1024, sizeof(uint8_t));
  if (opcode == 0b0110111LL)
    strcat(outputInstStr, "lui   ");
  else if (opcode == 0b0010111LL)
    strcat(outputInstStr, "auipc ");

  uint8_t str_imm31_12[12];

  sprintf(str_imm31_12, "%d", abs(imm31_12));
  sprintf(str_imm31_12, "%d", abs(imm31_12));

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm31_12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, str_imm31_12);
  *instStr = outputInstStr;
}

static void jTypeInst(uint32_t instNoOpcode, uint8_t opcode,
                      const uint8_t **instStr) {
  int32_t imm20 = instNoOpcode & 0b10000000000000000000000000000000;
  int32_t imm10_1 = instNoOpcode & 0b111111111100000000000000000000;
  int32_t imm11 = instNoOpcode & 0b10000000000000000000;
  int32_t imm19_12 = instNoOpcode & 0b1111111100000000000;
  uint32_t rd = instNoOpcode & 0b11111000000;

  int32_t imm = imm10_1 << 1 | imm11 << 11 | imm19_12 << 12 | imm20 << 20;
  uint8_t *outputInstStr = calloc(1024, sizeof(uint8_t));
  uint8_t *str_imm;
  sprintf(str_imm, "%d", imm);

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, str_imm);
  *instStr = outputInstStr;
}

static void decode(uint32_t instruction, bool isUncompressionInst) {
  const uint8_t *instStr;

  if (isUncompressionInst) {
    uint8_t opcode = instruction & 0b1111111;
    uint32_t instNoOpcode = instruction >> 7;
    switch (opcode) {
    case 0b0110011: // R
      rTypeInst(instNoOpcode, opcode, &instStr);
      break;

    case 0b1100111: // I Type
    case 0b0000011:
    case 0b0010011:
    case 0b0001111:
    case 0b1110011:
      iTypeInst(instNoOpcode, opcode, &instStr);
      break;

    case 0b0100011: // S Type
      sTypeInst(instNoOpcode, opcode, &instStr);
      break;

    case 0b1100011: // B Type
      bTypeInst(instNoOpcode, opcode, &instStr);
      break;

    case 0b0110111: // U Type
    case 0b0010111:
      uTypeInst(instNoOpcode, opcode, &instStr);
      break;

    case 0b1101111: // J Type
      jTypeInst(instNoOpcode, opcode, &instStr);
      break;

    default:
      fprintf(stderr, "Invalid instruction %u:\n", instruction);
      abort();
    }

  } else {
  }

  fprintf(stderr, "%04x:     %08x    %s\n", instructionIndex, instruction,
          instStr);

  if (isUncompressionInst)
    instructionIndex += uncompressionInstLen;
  else
    instructionIndex += compressionInstLen;
}

int disassembleText(const uint8_t *inputFileName) {
  FILE *fileHandle = fopen(inputFileName, "rb");

  fprintf(stderr, "\n\n");
  fprintf(stderr, "Disassembly of section .text:\n");

  if (!shdrTextOff) {
    fprintf(stderr, "No .text section\n");
    return 0;
  }

  uint8_t *instBuffer = malloc(shdrTextSize);
  uint8_t *instBufferEnd = instBuffer + shdrTextSize;

  if (!fseek(fileHandle, shdrTextOff, SEEK_SET))
    fread(instBuffer, 1, shdrTextSize, fileHandle);

  while (instBuffer != instBufferEnd) {
    uint32_t instruction =
        byte_get_little_endian(instBuffer, uncompressionInstLen);
    if (!(~instruction & 3LL)) {
      decode(instruction, true);
      instBuffer += uncompressionInstLen;
    } else {
      decode(instruction, false);
      instBuffer += compressionInstLen;
    }
  }

  closeFile(fileHandle);

  fprintf(stderr, "\n\n");
  return 0;
}
