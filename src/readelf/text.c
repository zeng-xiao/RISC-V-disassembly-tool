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

static uint32_t instIndex = 0;

uint8_t immStr[20];

uint8_t outputInstStr[1024] = {'\0'};

static const uint8_t *registerAbiName[32] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

static const uint8_t *rTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct7 = (instNoOpcode >> 25) & 0b1111111;
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rd = (instNoOpcode >> 0) & 0b11111;

  uint16_t funct = funct7 << 3 | funct3;

  switch (funct) {
  case 0b0000000 << 7 | 0b000:
    strcat(outputInstStr, "add   ");
    break;
  case 0b0100000 << 3 | 0b000:
    strcat(outputInstStr, "sub   ");
    break;
  case 0b0000000 << 3 | 0b001:
    strcat(outputInstStr, "sll   ");
    break;
  case 0b0000000 << 3 | 0b010:
    strcat(outputInstStr, "slt   ");
    break;
  case 0b0000000 << 3 | 0b011:
    strcat(outputInstStr, "sltu  ");
    break;
  case 0b0000000 << 3 | 0b100:
    strcat(outputInstStr, "xor   ");
    break;
  case 0b0000000 << 3 | 0b101:
    strcat(outputInstStr, "srl   ");
    break;
  case 0b0100000 << 3 | 0b101:
    strcat(outputInstStr, "sra   ");
    break;
  case 0b0000000 << 3 | 0b110:
    strcat(outputInstStr, "or    ");
    break;
  case 0b0000000 << 3 | 0b111:
    strcat(outputInstStr, "and   ");
    break;

  default:
    fprintf(stderr, "R type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs1]);

  return outputInstStr;
}

static const uint8_t *iTypeJalrInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  int32_t imm31_12 = instNoOpcode >> 13;

  strcat(outputInstStr, "jalr  ");
  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm31_12 < 0)
    strcat(outputInstStr, "-");
  sprintf(immStr, "%u", abs(imm31_12));
  strcat(outputInstStr, "(");
  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *iTypeLbInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  int32_t imm11_0 = instNoOpcode >> 13;

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
    fprintf(stderr, "I type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm11_0 < 0)
    strcat(outputInstStr, "-");
  sprintf(immStr, "%u", abs(imm11_0));
  strcat(outputInstStr, "(");
  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *iTypeAddiInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;
  uint8_t funct7 = instNoOpcode >> 18;

  int32_t imm11_0 = instNoOpcode >> 13;

  int32_t outputNum = imm11_0;

  uint8_t shamt;

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
  case 0b101:
    if (funct3 == 0b001 && funct7 == 0b0000000)
      strcat(outputInstStr, "slli ");
    else if (funct3 == 0b101 && funct7 == 0b0000000)
      strcat(outputInstStr, "srli ");
    else if (funct3 == 0b101 && funct7 == 0b0100000)
      strcat(outputInstStr, "srai ");
    else {
      fprintf(stderr, "I type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
    shamt = (instNoOpcode >> 13) & 0b111111;
    outputNum = shamt;
    break;
  default:
    fprintf(stderr, "I type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs1]);
  if (outputNum < 0)
    strcat(outputInstStr, "-");
  sprintf(immStr, "%u", abs(outputNum));
  strcat(outputInstStr, immStr);

  return outputInstStr;
}

static const uint8_t *iTypeFenceInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t pred = (instNoOpcode >> 17) & 0b1111;
  uint8_t succ = (instNoOpcode >> 13) & 0b1111;

  bool fence = false;

  switch (funct3) {
  case 0b000:
    strcat(outputInstStr, "fence ");
    fence = true;
    break;
  case 0b001:
    strcat(outputInstStr, "fence.i");
    break;
  default:
    fprintf(stderr, "I type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  if (fence) {
    sprintf(immStr, "%u", pred);
    strcat(outputInstStr, ",");
    sprintf(immStr, "%u", succ);
  }

  return outputInstStr;
}

static const uint8_t *iTypeEcallInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  if (funct3 == 0b000) {
    uint16_t imm11_0 = instNoOpcode >> 13;

    switch (imm11_0) {
    case 0b000000000000:
      strcat(outputInstStr, "ecall");
      break;
    case 0b000000100000:
      strcat(outputInstStr, "ebreak");
      break;
    default:
      fprintf(stderr, "I type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
  } else {
    uint16_t csr = instNoOpcode >> 13;
    uint8_t rd = instNoOpcode & 0b11111;
    uint8_t rs1, zimm;

    switch (funct3) {
    case 0b001:
    case 0b010:
    case 0b011:
      rs1 = (instNoOpcode >> 8) & 0b11111;
      if (funct3 == 0b001)
        strcat(outputInstStr, "csrrw ");
      else if (funct3 == 0b001)
        strcat(outputInstStr, "csrrs ");
      else if (funct3 == 0b001)
        strcat(outputInstStr, "csrrc ");
      else {
        fprintf(stderr, "I type invalid instruction: %x\n",
                instNoOpcode << 7 | opcode);
        abort();
      }
      strcat(outputInstStr, registerAbiName[rd]);
      strcat(outputInstStr, ",");
      sprintf(immStr, "%u", csr);
      strcat(outputInstStr, ",");
      strcat(outputInstStr, registerAbiName[rs1]);
      break;

    case 0b101:
    case 0b110:
    case 0b111:
      zimm = (instNoOpcode >> 8) & 0b11111;

      if (funct3 == 0b101)
        strcat(outputInstStr, "csrrwi  ");
      else if (funct3 == 0b101)
        strcat(outputInstStr, "cssrrsi ");
      else if (funct3 == 0b101)
        strcat(outputInstStr, "csrrci  ");
      else {
        fprintf(stderr, "I type invalid instruction: %x\n",
                instNoOpcode << 7 | opcode);
        abort();
      }
      strcat(outputInstStr, registerAbiName[rd]);
      strcat(outputInstStr, ",");
      sprintf(immStr, "%u", csr);
      strcat(outputInstStr, ",");
      if (zimm < 0)
        strcat(outputInstStr, "-");
      sprintf(immStr, "%u", abs(zimm));
      break;

    default:
      fprintf(stderr, "I type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
  }
  return outputInstStr;
}

static const uint8_t *sTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  int32_t imm11_5 = instNoOpcode >> 18;
  int32_t imm4_0 = instNoOpcode & 0b11111;

  int32_t imm12 = imm11_5 << 5 | imm4_0;

  sprintf(immStr, "%u", abs(imm12));

  switch (funct3) {
  case 0b000:
    strcat(outputInstStr, "sb    ");
    break;
  case 0b001:
    strcat(outputInstStr, "sh    ");
    break;
  case 0b010:
    strcat(outputInstStr, "sw    ");
    break;

    /*rv64*/
  case 0b011:
    strcat(outputInstStr, "sd    ");
    break;
  default:
    fprintf(stderr, "S type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  if (imm12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *bTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  int32_t imm12_12 = (instNoOpcode >> 31) & 0b1;
  int32_t imm11_11 = instNoOpcode & 0b1;
  int32_t imm10_5 = (instNoOpcode >> 25) & 0b111111;
  int32_t imm4_1 = (instNoOpcode >> 1) & 0b1111;

  int32_t imm12 = imm12_12 << 12 | imm11_11 << 11 | imm10_5 << 5 | imm4_1 << 1;

  sprintf(immStr, "%u", abs(imm12));

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
    fprintf(stderr, "B type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, registerAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, registerAbiName[rs2]);
  strcat(outputInstStr, ",");
  if (imm12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);

  return outputInstStr;
}

static const uint8_t *uTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;

  int32_t imm31_12 = instNoOpcode >> 5;
  sprintf(immStr, "%u", abs(imm31_12));

  if (opcode == 0b0110111)
    strcat(outputInstStr, "lui   ");
  else if (opcode == 0b0010111)
    strcat(outputInstStr, "auipc ");
  else {
    fprintf(stderr, "U type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm31_12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);

  return outputInstStr;
}

static const uint8_t *jTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;

  int32_t imm20_20 = (instNoOpcode >> 24) & 0b1;
  int32_t imm19_12 = (instNoOpcode >> 5) & 0b11111111;
  int32_t imm11_11 = (instNoOpcode >> 13) & 0b1;
  int32_t imm10_1 = (instNoOpcode >> 14) & 0b1111111111;

  int32_t imm20 =
      imm20_20 << 20 | imm19_12 << 12 | imm11_11 << 11 | imm10_1 << 1;
  sprintf(immStr, "%u", abs(imm20));

  strcat(outputInstStr, registerAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm20 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);

  return outputInstStr;
}

static void decode(int32_t instruction, bool isUncompressionInst) {
  const uint8_t *instStr;

  if (isUncompressionInst) {
    uint8_t opcode = instruction & 0b1111111;
    int32_t instNoOpcode = instruction >> 7;

    switch (opcode) {
    case 0b0110011: // R Type
      instStr = rTypeInst(instNoOpcode, opcode);
      break;

    case 0b1100111: // I Type 1
      instStr = iTypeJalrInst(instNoOpcode, opcode);
      break;
    case 0b0000011: // I Type 2
      instStr = iTypeLbInst(instNoOpcode, opcode);
      break;
    case 0b0010011: // I Type 3
      instStr = iTypeAddiInst(instNoOpcode, opcode);
      break;
    case 0b0001111: // I Type 4
      instStr = iTypeFenceInst(instNoOpcode, opcode);
      break;
    case 0b1110011: // I Type 5
      instStr = iTypeEcallInst(instNoOpcode, opcode);
      break;

    case 0b0100011: // S Type
      instStr = sTypeInst(instNoOpcode, opcode);
      break;

    case 0b1100011: // B Type
      instStr = bTypeInst(instNoOpcode, opcode);
      break;

    case 0b0110111: // U Type
    case 0b0010111:
      instStr = uTypeInst(instNoOpcode, opcode);
      break;

    case 0b1101111: // J Type
      instStr = jTypeInst(instNoOpcode, opcode);
      break;

    default:
      fprintf(stderr, "Invalid instruction: %x\n", instruction);
      abort();
    }

  } else {
  }

  fprintf(stderr, "%04x:     %08x    %s\n", instIndex, instruction, instStr);

  if (isUncompressionInst)
    instIndex += uncompressionInstLen;
  else
    instIndex += compressionInstLen;

  /* The strcat function appends a copy of the string pointed to by s2
  (including the terminating null character) to the end of the string pointed to
  by s1. The initial character of s2 overwrites the null character at the end of
  s1. */
  outputInstStr[0] = '\0';
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
    int32_t instruction =
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
