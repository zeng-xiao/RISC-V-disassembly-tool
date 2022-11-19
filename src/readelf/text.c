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

static uint8_t immStr[20];

static uint8_t outputInstStr[1024];

static const uint8_t *intRegAbiName[32] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

static const uint8_t *floatRegAbiName[32] = {
    "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
    "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
    "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"};

static const uint8_t *rTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct7 = (instNoOpcode >> 18) & 0b1111111;
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rd = (instNoOpcode >> 0) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  uint16_t funct = funct7 << 3 | funct3;

  switch (funct) {
  case 0b0000000 << 7 | 0b000:
    strcat(outputInstStr, "add    ");
    break;
  case 0b0100000 << 3 | 0b000:
    strcat(outputInstStr, "sub    ");
    break;
  case 0b0000000 << 3 | 0b001:
    strcat(outputInstStr, "sll    ");
    break;
  case 0b0000000 << 3 | 0b010:
    strcat(outputInstStr, "slt    ");
    break;
  case 0b0000000 << 3 | 0b011:
    strcat(outputInstStr, "sltu   ");
    break;
  case 0b0000000 << 3 | 0b100:
    strcat(outputInstStr, "xor    ");
    break;
  case 0b0000000 << 3 | 0b101:
    strcat(outputInstStr, "srl    ");
    break;
  case 0b0100000 << 3 | 0b101:
    strcat(outputInstStr, "sra    ");
    break;
  case 0b0000000 << 3 | 0b110:
    strcat(outputInstStr, "or     ");
    break;
  case 0b0000000 << 3 | 0b111:
    strcat(outputInstStr, "and    ");
    break;
  /* rv32M */
  case 0b0000001 << 3 | 0b000:
    strcat(outputInstStr, "mul    ");
    break;
  case 0b0000001 << 3 | 0b001:
    strcat(outputInstStr, "mulh   ");
    break;
  case 0b0000001 << 3 | 0b010:
    strcat(outputInstStr, "mulhsu ");
    break;
  case 0b0000001 << 3 | 0b011:
    strcat(outputInstStr, "mulhu ");
    break;
  case 0b0000001 << 3 | 0b100:
    strcat(outputInstStr, "div    ");
    break;
  case 0b0000001 << 3 | 0b101:
    strcat(outputInstStr, "divu   ");
    break;
  case 0b0000001 << 3 | 0b110:
    strcat(outputInstStr, "rem    ");
    break;
  case 0b0000001 << 3 | 0b111:
    strcat(outputInstStr, "remu   ");
    break;
  default:
    fprintf(stderr, "[1] R type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs2]);

  return outputInstStr;
}

static const uint8_t *r4TypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rm = (instNoOpcode >> 5) & 0b111;

  uint8_t rd = (instNoOpcode >> 0) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;
  uint8_t rs3 = instNoOpcode >> 20;

  switch (opcode) {
  case 0b1000011:
    strcat(outputInstStr, "fmadd.s ");
    break;
  case 0b1000111:
    strcat(outputInstStr, "fmsub.s ");
    break;
  case 0b1001011:
    strcat(outputInstStr, "fnmsub.s ");
    break;
  case 0b1001111:
    strcat(outputInstStr, "fnmadd.s ");
    break;
  default:
    fprintf(stderr, "[2] R4 type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, floatRegAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, floatRegAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, floatRegAbiName[rs2]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, floatRegAbiName[rs3]);

  return outputInstStr;
}

static const uint8_t *rTypeFaddsInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct7 = (instNoOpcode >> 18) & 0b1111111;
  uint8_t rm = (instNoOpcode >> 5) & 0b111;

  uint8_t rd = (instNoOpcode >> 0) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  uint8_t funct5 = (instNoOpcode >> 13) & 0b11111;

  switch (funct7) {
  case 0b0000001:
    strcat(outputInstStr, "fadd.d ");
    break;
  case 0b0000101:
    strcat(outputInstStr, "fsub.d ");
    break;
  case 0b0001001:
    strcat(outputInstStr, "fmul.d ");
    break;
  case 0b0001101:
    strcat(outputInstStr, "fdiv.d ");
    break;
  case 0b0101101:
    if (rs2 == 0b00000)
      strcat(outputInstStr, "fsqrt.d ");
    break;
  case 0b0010001:
    if (rm == 0b000)
      strcat(outputInstStr, "fsgnj.d ");
    if (rm == 0b010)
      strcat(outputInstStr, "fsgnjn.d ");
    if (rm == 0b001)
      strcat(outputInstStr, "fsgnjx.d ");
    break;
  case 0b0010101:
    if (rm == 0b000)
      strcat(outputInstStr, "fmin.d ");
    if (rm == 0b001)
      strcat(outputInstStr, "fmax.d ");
    break;
  case 0b0100000:
    if (rs2 == 0b00001)
      strcat(outputInstStr, "fcvt.s.d ");
    break;
  case 0b0100001:
    if (rs2 == 0b00000)
      strcat(outputInstStr, "fcvt.d.s ");
    break;
  case 0b1010001:
    if (rm == 0b010)
      strcat(outputInstStr, "feq.d  ");
    if (rm == 0b001)
      strcat(outputInstStr, "flt.d  ");
    if (rm == 0b000)
      strcat(outputInstStr, "fle.d  ");
    break;
  case 0b1110001:
    if (rs2 << 3 | rm == 0b00000 << 3 | 0b010)
      strcat(outputInstStr, " fclass.d ");
    else if (funct5 == 0b00000)
      strcat(outputInstStr, "fmv.x.d "); /* rv64 R Type RV64F */
    else {
      fprintf(stderr, "[21] R type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
    break;
  case 0b1100001:
    if (rs2 == 0b00000)
      strcat(outputInstStr, "fcvt.w.d ");
    else if (rs2 == 0b00001)
      strcat(outputInstStr, "fcvt.wu.d ");
    else if (funct5 == 0b00010)
      strcat(outputInstStr, "fcvt.l.d "); /* rv64 R Type RV64F */
    else if (funct5 == 0b00011)
      strcat(outputInstStr, "fcvt.lu.d "); /* rv64 R Type RV64F */
    else {
      fprintf(stderr, "[20] R type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
    break;
  case 0b1101001:
    if (rs2 == 0b00000)
      strcat(outputInstStr, "fcvt.d.w ");
    else if (rs2 == 0b00001)
      strcat(outputInstStr, "fcvt.d.wu ");
    else if (funct5 == 0b00010)
      strcat(outputInstStr, "fcvt.d.l "); /* rv64 R Type RV64F */
    else if (funct5 == 0b00011)
      strcat(outputInstStr, "fcvt.d.lu "); /* rv64 R Type RV64F */
    else {
      fprintf(stderr, "[19] R type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
    break;
    /* rv64 R Type RV64F */
  case 0b1100000:
    if (funct5 == 0b00010)
      strcat(outputInstStr, "fcvt.l.s ");
    if (funct5 == 0b00011)
      strcat(outputInstStr, "fcvt.lu.s ");
    break;
  case 0b1101000:
    if (funct5 == 0b00010)
      strcat(outputInstStr, "fcvt.s.l ");
    if (funct5 == 0b00011)
      strcat(outputInstStr, "fcvt.s.lu ");
    break;
  case 0b1111001:
    if (funct5 == 0b00000)
      strcat(outputInstStr, "fmv.d.x ");
    break;
  default:
    fprintf(stderr, "[3] R type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs2]);

  return outputInstStr;
}

static const uint8_t *rv64rTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct7 = (instNoOpcode >> 18) & 0b1111111;
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rd = (instNoOpcode >> 0) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  uint16_t funct = funct7 << 3 | funct3;

  switch (funct) {
  case 0b0000000 << 3 | 0b000:
    strcat(outputInstStr, "addw   ");
    break;
  case 0b0100000 << 3 | 0b000:
    strcat(outputInstStr, "subw   ");
    break;
  case 0b0000000 << 3 | 0b001:
    strcat(outputInstStr, "sllw   ");
    break;
  case 0b0000000 << 3 | 0b101:
    strcat(outputInstStr, "srlw   ");
    break;
  case 0b0100000 << 3 | 0b101:
    strcat(outputInstStr, "sraw    ");
    break;
    /* rv64M */
  case 0b0000001 << 3 | 0b000:
    strcat(outputInstStr, "mulw   ");
    break;
  case 0b0000001 << 3 | 0b100:
    strcat(outputInstStr, "divw   ");
    break;
  case 0b0000001 << 3 | 0b101:
    strcat(outputInstStr, "divuw   ");
    break;
  case 0b0000001 << 3 | 0b110:
    strcat(outputInstStr, "remw    ");
    break;
  case 0b0000001 << 3 | 0b111:
    strcat(outputInstStr, "remuw   ");
    break;
  default:
    fprintf(stderr, "[4] rv64R type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs2]);

  return outputInstStr;
}

static const uint8_t *rv64rTypeAInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct5 = instNoOpcode >> 20;
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t aq = (instNoOpcode >> 19) & 0b1;
  uint8_t rl = (instNoOpcode >> 18) & 0b1;

  uint8_t rd = (instNoOpcode >> 0) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  uint16_t funct = funct5 << 3 | funct3;

  bool lrd = false;
  switch (funct) {
  case 0b00010 << 7 | 0b011:
    lrd = true;
    strcat(outputInstStr, "lr.d   ");
    break;
  case 0b00011 << 3 | 0b011:
    strcat(outputInstStr, "sc.d   ");
    break;
  case 0b00001 << 3 | 0b011:
    strcat(outputInstStr, "amoswap.d ");
    break;
  case 0b00000 << 3 | 0b011:
    strcat(outputInstStr, "amoadd.d ");
    break;
  case 0b00100 << 3 | 0b011:
    strcat(outputInstStr, "amoxor.d ");
    break;
  case 0b01100 << 3 | 0b011:
    strcat(outputInstStr, "amoand.d ");
    break;
  case 0b01000 << 3 | 0b011:
    strcat(outputInstStr, "amoor.d ");
    break;
  case 0b10000 << 3 | 0b011:
    strcat(outputInstStr, "amomin.d ");
    break;
  case 0b10100 << 3 | 0b011:
    strcat(outputInstStr, "amomax.d ");
    break;
  case 0b11000 << 3 | 0b011:
    strcat(outputInstStr, "amominu.d ");
    break;
  case 0b11100 << 3 | 0b011:
    strcat(outputInstStr, "amomaxu.d ");
    break;
  default:
    fprintf(stderr, "[5] rv64R atomic type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  if (!lrd) {
    strcat(outputInstStr, intRegAbiName[rs2]);
    strcat(outputInstStr, ",");
  }
  strcat(outputInstStr, "(");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *rv64iTypeFlwInst(int32_t instNoOpcode, uint8_t opcode) {

  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rd = (instNoOpcode >> 0) & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  uint16_t imm11_0 = instNoOpcode >> 13;
  sprintf(immStr, "%u", abs(imm11_0));

  strcat(outputInstStr, "flw   ");
  strcat(outputInstStr, floatRegAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm11_0 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, floatRegAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *iTypeJalrInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  int32_t imm31_12 = instNoOpcode >> 13;

  strcat(outputInstStr, "jalr   ");
  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm31_12 < 0)
    strcat(outputInstStr, "-");
  sprintf(immStr, "%u", abs(imm31_12));
  strcat(outputInstStr, immStr);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, intRegAbiName[rs1]);
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
    strcat(outputInstStr, "lb     ");
    break;
  case 0b001:
    strcat(outputInstStr, "lh     ");
    break;
  case 0b010:
    strcat(outputInstStr, "lw     ");
    break;
  case 0b100:
    strcat(outputInstStr, "lbu    ");
    break;
  case 0b101:
    strcat(outputInstStr, "lhu    ");
    break;
    /*rv64I*/
  case 0b011:
    strcat(outputInstStr, "ld     ");
    break;
  case 0b110:
    strcat(outputInstStr, "lwu    ");
    break;
  default:
    fprintf(stderr, "[6] I type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm11_0 < 0)
    strcat(outputInstStr, "-");
  sprintf(immStr, "%u", abs(imm11_0));
  strcat(outputInstStr, immStr);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *iTypeAddiInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;
  uint8_t funct7 = (instNoOpcode >> 19) & 0b1111111;

  int32_t imm11_0 = instNoOpcode >> 13;

  int32_t outputNum = imm11_0;

  uint8_t shamt;

  switch (funct3) {
  case 0b000:
    strcat(outputInstStr, "addi   ");
    break;
  case 0b010:
    strcat(outputInstStr, "slti   ");
    break;
  case 0b011:
    strcat(outputInstStr, "sltiu  ");
    break;
  case 0b100:
    strcat(outputInstStr, "xori   ");
    break;
  case 0b110:
    strcat(outputInstStr, "ori    ");
    break;
  case 0b111:
    strcat(outputInstStr, "andi   ");
    break;
  case 0b001:
  case 0b101:
    if (funct3 == 0b001 && funct7 == 0b000000)
      strcat(outputInstStr, "slli   ");
    else if (funct3 == 0b101 && funct7 == 0b000000)
      strcat(outputInstStr, "srli   ");
    else if (funct3 == 0b101 && funct7 == 0b010000)
      strcat(outputInstStr, "srai   ");
    else {
      fprintf(stderr, "[7] I type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
    shamt = (instNoOpcode >> 13) & 0b111111;
    outputNum = shamt;
    break;
  default:
    fprintf(stderr, "[8] I type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ",");
  if (outputNum < 0)
    strcat(outputInstStr, "-");
  sprintf(immStr, "%u", abs(outputNum));
  strcat(outputInstStr, immStr);

  return outputInstStr;
}

static const uint8_t *rv64iTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;
  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;

  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;
  uint8_t funct7 = (instNoOpcode >> 18) & 0b1111111;

  uint8_t shamt;
  int32_t imm11_0 = instNoOpcode >> 13;
  int32_t outputNum = imm11_0;

  sprintf(immStr, "%u", abs(imm11_0));

  switch (funct3) {
  case 0b000:
    strcat(outputInstStr, "addiw  ");
    break;
  case 0b001:
    strcat(outputInstStr, "slliw  ");
    break;
  case 0b101:
    if (funct7 == 0b0000000)
      strcat(outputInstStr, "srliw  ");
    else if (funct7 == 0b0100000)
      strcat(outputInstStr, "sraiw  ");
    else {
      fprintf(stderr, "[9] rv64 I type invalid instruction: %x\n",
              instNoOpcode << 7 | opcode);
      abort();
    }
    shamt = (instNoOpcode >> 13) & 0b11111;
    outputNum = shamt;
    break;
  default:
    fprintf(stderr, "[10] I type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ",");
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
    fprintf(stderr, "[11] I type invalid instruction: %x\n",
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
    case 0b000000000001:
      strcat(outputInstStr, "ebreak");
      break;
    default:
      fprintf(stderr, "[12] I type invalid instruction: %x\n",
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
      else if (funct3 == 0b010)
        strcat(outputInstStr, "csrrs ");
      else if (funct3 == 0b011)
        strcat(outputInstStr, "csrrc ");
      else {
        fprintf(stderr, "[13] I type invalid instruction: %x\n",
                instNoOpcode << 7 | opcode);
        abort();
      }
      strcat(outputInstStr, intRegAbiName[rd]);
      strcat(outputInstStr, ",");
      sprintf(immStr, "%u", csr);
      strcat(outputInstStr, ",");
      strcat(outputInstStr, intRegAbiName[rs1]);
      break;

    case 0b101:
    case 0b110:
    case 0b111:
      zimm = (instNoOpcode >> 8) & 0b11111;

      if (funct3 == 0b101)
        strcat(outputInstStr, "csrrwi ");
      else if (funct3 == 0b101)
        strcat(outputInstStr, "cssrrsi ");
      else if (funct3 == 0b101)
        strcat(outputInstStr, "csrrci ");
      else {
        fprintf(stderr, "[14] I type invalid instruction: %x\n",
                instNoOpcode << 7 | opcode);
        abort();
      }
      strcat(outputInstStr, intRegAbiName[rd]);
      strcat(outputInstStr, ",");
      sprintf(immStr, "%u", csr);
      strcat(outputInstStr, ",");
      if (zimm < 0)
        strcat(outputInstStr, "-");
      sprintf(immStr, "%u", abs(zimm));
      break;

    default:
      fprintf(stderr, "[15] I type invalid instruction: %x\n",
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
    strcat(outputInstStr, "sb     ");
    break;
  case 0b001:
    strcat(outputInstStr, "sh     ");
    break;
  case 0b010:
    strcat(outputInstStr, "sw     ");
    break;

    /*rv64*/
  case 0b011:
    strcat(outputInstStr, "sd     ");
    break;
  default:
    fprintf(stderr, "[15] S type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }
  strcat(outputInstStr, intRegAbiName[rs2]);
  strcat(outputInstStr, ",");
  if (imm12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *rv32sTypeFswInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  int16_t imm11_5 = instNoOpcode >> 18;
  int16_t imm4_0 = instNoOpcode & 0b11111;

  int16_t imm12 = imm11_5 << 5 | imm4_0;
  sprintf(immStr, "%u", abs(imm12));

  strcat(outputInstStr, floatRegAbiName[rs2]);
  strcat(outputInstStr, ",");
  if (imm12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);
  strcat(outputInstStr, "(");
  strcat(outputInstStr, floatRegAbiName[rs1]);
  strcat(outputInstStr, ")");

  return outputInstStr;
}

static const uint8_t *bTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t funct3 = (instNoOpcode >> 5) & 0b111;

  uint8_t rs1 = (instNoOpcode >> 8) & 0b11111;
  uint8_t rs2 = (instNoOpcode >> 13) & 0b11111;

  int32_t imm12_12 = instNoOpcode >> 24;
  int32_t imm11_11 = instNoOpcode & 0b1;
  int32_t imm10_5 = (instNoOpcode >> 18) & 0b111111;
  int32_t imm4_1 = (instNoOpcode >> 1) & 0b1111;

  int32_t imm12 = imm12_12 << 12 | imm11_11 << 11 | imm10_5 << 5 | imm4_1 << 1;

  sprintf(immStr, "%u", abs(imm12));

  switch (funct3) {
  case 0b000:
    strcat(outputInstStr, "beq    ");
    break;
  case 0b001:
    strcat(outputInstStr, "bne    ");
    break;
  case 0b100:
    strcat(outputInstStr, "blt    ");
    break;
  case 0b101:
    strcat(outputInstStr, "bge    ");
    break;
  case 0b110:
    strcat(outputInstStr, "bltu   ");
    break;
  case 0b111:
    strcat(outputInstStr, "bgeu   ");
    break;
  default:
    fprintf(stderr, "[16] B type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, intRegAbiName[rs1]);
  strcat(outputInstStr, ",");
  strcat(outputInstStr, intRegAbiName[rs2]);
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
    strcat(outputInstStr, "lui    ");
  else if (opcode == 0b0010111)
    strcat(outputInstStr, "auipc  ");
  else {
    fprintf(stderr, "[17] U type invalid instruction: %x\n",
            instNoOpcode << 7 | opcode);
    abort();
  }

  strcat(outputInstStr, intRegAbiName[rd]);
  strcat(outputInstStr, ",");
  if (imm31_12 < 0)
    strcat(outputInstStr, "-");
  strcat(outputInstStr, immStr);

  return outputInstStr;
}

static const uint8_t *jTypeInst(int32_t instNoOpcode, uint8_t opcode) {
  uint8_t rd = instNoOpcode & 0b11111;

  int32_t imm20_20 = instNoOpcode >> 24;
  int32_t imm19_12 = (instNoOpcode >> 5) & 0b11111111;
  int32_t imm11_11 = (instNoOpcode >> 13) & 0b1;
  int32_t imm10_1 = (instNoOpcode >> 14) & 0b1111111111;

  int32_t imm20 =
      imm20_20 << 20 | imm19_12 << 12 | imm11_11 << 11 | imm10_1 << 1;
  sprintf(immStr, "%u", abs(imm20));

  strcat(outputInstStr, "jal    ");
  strcat(outputInstStr, intRegAbiName[rd]);
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
    case 0b0110011: // rv32/64 R Type
      instStr = rTypeInst(instNoOpcode, opcode);
      break;
    case 0b1000011: // rv32/64 R4 Type
    case 0b1000111: // rv32/64 R4 Type
    case 0b1001011: // rv32/64 R4 Type
    case 0b1001111: // rv32/64 R4 Type
      instStr = r4TypeInst(instNoOpcode, opcode);
      break;
    case 0b1010011: // rv32/64 R4 Type
      instStr = rTypeFaddsInst(instNoOpcode, opcode);
      break;
    case 0b0111011: // rv64 R Type addw subw sllw srlw sraw + RV64M
      instStr = rv64rTypeInst(instNoOpcode, opcode);
      break;
    case 0b0101111: // rv64 R Type RV64A
      instStr = rv64rTypeAInst(instNoOpcode, opcode);
      break;

    case 0b1100111: // rv32/64 I Type 1
      instStr = iTypeJalrInst(instNoOpcode, opcode);
      break;
    case 0b0000011: // rv32/64 I Type 2 + rv64I Type: lwu ld
      instStr = iTypeLbInst(instNoOpcode, opcode);
      break;
    case 0b0010011: // rv32/64 I Type 3
      instStr = iTypeAddiInst(instNoOpcode, opcode);
      break;
    case 0b0001111: // rv32/64 I Type 4
      instStr = iTypeFenceInst(instNoOpcode, opcode);
      break;
    case 0b1110011: // rv32/64 I Type 5
      instStr = iTypeEcallInst(instNoOpcode, opcode);
      break;
    case 0b0000111: // rv32 I Type flw
      instStr = rv64iTypeFlwInst(instNoOpcode, opcode);
      break;
    case 0b0011011: // rv64I Type: addiw slliw srliw sraiw
      instStr = rv64iTypeInst(instNoOpcode, opcode);
      break;

    case 0b0100011: // rv32/64 S Type
      instStr = sTypeInst(instNoOpcode, opcode);
      break;
    case 0b0100111: // rv32 S Type fsw
      instStr = rv32sTypeFswInst(instNoOpcode, opcode);
      break;

    case 0b1100011: // rv32/64 B Type
      instStr = bTypeInst(instNoOpcode, opcode);
      break;

    case 0b0110111: // rv32/64 U Type
    case 0b0010111:
      instStr = uTypeInst(instNoOpcode, opcode);
      break;

    case 0b1101111: // rv32/64 J Type
      instStr = jTypeInst(instNoOpcode, opcode);
      break;

    default:
      fprintf(stderr, "[18] Invalid instruction: %x\n", instruction);
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
  (including the terminating null character) to the end of the string pointed
  to by s1. The initial character of s2 overwrites the null character at the
  end of s1. */
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
