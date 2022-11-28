#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "elf.h"

#include "dataOperate.h"

extern uint64_t shdrTextOff;
extern uint64_t shdrTextSize;
extern uint8_t riscvLen;

static uint8_t compressionInstLen = 2;
static uint8_t uncompressionInstLen = 4;

/* types */

typedef uint64_t rv_inst;
typedef uint16_t rv_opcode;

/* enums */

typedef enum { rv32, rv64, rv128 } rv_isa;

typedef enum {
  rm_rne = 0,
  rm_rtz = 1,
  rm_rdn = 2,
  rm_rup = 3,
  rm_rmm = 4,
  rm_dyn = 7,
} rv_rm;

typedef enum {
  fence_i = 8,
  fence_o = 4,
  fence_r = 2,
  fence_w = 1,
} rv_fence;

typedef enum {
  reg_x00_f00,
  reg_x01_f01,
  reg_x02_f02,
  reg_x03_f03,
  reg_x04_f04,
  reg_x05_f05,
  reg_x06_f06,
  reg_x07_f07,
  reg_x08_f08,
  reg_x09_f09,
  reg_x10_f10,
  reg_x11_f11,
  reg_x12_f12,
  reg_x13_f13,
  reg_x14_f14,
  reg_x15_f15,
  reg_x16_f16,
  reg_x17_f17,
  reg_x18_f18,
  reg_x19_f19,
  reg_x20_f20,
  reg_x21_f21,
  reg_x22_f22,
  reg_x23_f23,
  reg_x24_f24,
  reg_x25_f25,
  reg_x26_f26,
  reg_x27_f27,
  reg_x28_f28,
  reg_x29_f29,
  reg_x30_f30,
  reg_x31_f31,
} rv_reg;

typedef enum {
  rvc_end,
  rvc_rd_eq_ra,
  rvc_rd_eq_x0,
  rvc_rs1_eq_x0,
  rvc_rs2_eq_x0,
  rvc_rs2_eq_rs1,
  rvc_rs1_eq_ra,
  rvc_imm_eq_zero,
  rvc_imm_eq_n1,
  rvc_imm_eq_p1,
  rvc_csr_eq_0x001,
  rvc_csr_eq_0x002,
  rvc_csr_eq_0x003,
  rvc_csr_eq_0xc00,
  rvc_csr_eq_0xc01,
  rvc_csr_eq_0xc02,
  rvc_csr_eq_0xc80,
  rvc_csr_eq_0xc81,
  rvc_csr_eq_0xc82,
} rvc_constraint;

typedef enum {
  type_illegal,
  type_none,
  type_u,
  type_uj,
  type_i,
  type_i_sh5,
  type_i_sh6,
  type_i_sh7,
  type_i_csr,
  type_s,
  type_sb,
  type_r,
  type_r_m,
  type_r4_m,
  type_r_a,
  type_r_l,
  type_r_f,
  type_cb,
  type_cb_imm,
  type_cb_sh5,
  type_cb_sh6,
  type_ci,
  type_ci_sh5,
  type_ci_sh6,
  type_ci_16sp,
  type_ci_lwsp,
  type_ci_ldsp,
  type_ci_lqsp,
  type_ci_li,
  type_ci_lui,
  type_ci_none,
  type_ciw_4spn,
  type_cj,
  type_cj_jal,
  type_cl_lw,
  type_cl_ld,
  type_cl_lq,
  type_cr,
  type_cr_mv,
  type_cr_jalr,
  type_cr_jr,
  type_cs,
  type_cs_sw,
  type_cs_sd,
  type_cs_sq,
  type_css_swsp,
  type_css_sdsp,
  type_css_sqsp,
} rv_type;

typedef enum {
  op_illegal,
  op_lui,
  op_auipc,
  op_jal,
  op_jalr,
  op_beq,
  op_bne,
  op_blt,
  op_bge,
  op_bltu,
  op_bgeu,
  op_lb,
  op_lh,
  op_lw,
  op_lbu,
  op_lhu,
  op_sb,
  op_sh,
  op_sw,
  op_addi,
  op_slti,
  op_sltiu,
  op_xori,
  op_ori,
  op_andi,
  op_slli,
  op_srli,
  op_srai,
  op_add,
  op_sub,
  op_sll,
  op_slt,
  op_sltu,
  op_xor,
  op_srl,
  op_sra,
  op_or,
  op_and,
  op_fence,
  op_fence_i,
  op_lwu,
  op_ld,
  op_sd,
  op_addiw,
  op_slliw,
  op_srliw,
  op_sraiw,
  op_addw,
  op_subw,
  op_sllw,
  op_srlw,
  op_sraw,
  op_ldu,
  op_lq,
  op_sq,
  op_addid,
  op_sllid,
  op_srlid,
  op_sraid,
  op_addd,
  op_subd,
  op_slld,
  op_srld,
  op_srad,
  op_mul,
  op_mulh,
  op_mulhsu,
  op_mulhu,
  op_div,
  op_divu,
  op_rem,
  op_remu,
  op_mulw,
  op_divw,
  op_divuw,
  op_remw,
  op_remuw,
  op_muld,
  op_divd,
  op_divud,
  op_remd,
  op_remud,
  op_lr_w,
  op_sc_w,
  op_amoswap_w,
  op_amoadd_w,
  op_amoxor_w,
  op_amoor_w,
  op_amoand_w,
  op_amomin_w,
  op_amomax_w,
  op_amominu_w,
  op_amomaxu_w,
  op_lr_d,
  op_sc_d,
  op_amoswap_d,
  op_amoadd_d,
  op_amoxor_d,
  op_amoor_d,
  op_amoand_d,
  op_amomin_d,
  op_amomax_d,
  op_amominu_d,
  op_amomaxu_d,
  op_lr_q,
  op_sc_q,
  op_amoswap_q,
  op_amoadd_q,
  op_amoxor_q,
  op_amoor_q,
  op_amoand_q,
  op_amomin_q,
  op_amomax_q,
  op_amominu_q,
  op_amomaxu_q,
  op_ecall,
  op_ebreak,
  op_uret,
  op_sret,
  op_hret,
  op_mret,
  op_dret,
  op_sfence_vm,
  op_sfence_vma,
  op_wfi,
  op_csrrw,
  op_csrrs,
  op_csrrc,
  op_csrrwi,
  op_csrrsi,
  op_csrrci,
  op_flw,
  op_fsw,
  op_fmadd_s,
  op_fmsub_s,
  op_fnmsub_s,
  op_fnmadd_s,
  op_fadd_s,
  op_fsub_s,
  op_fmul_s,
  op_fdiv_s,
  op_fsgnj_s,
  op_fsgnjn_s,
  op_fsgnjx_s,
  op_fmin_s,
  op_fmax_s,
  op_fsqrt_s,
  op_fle_s,
  op_flt_s,
  op_feq_s,
  op_fcvt_w_s,
  op_fcvt_wu_s,
  op_fcvt_s_w,
  op_fcvt_s_wu,
  op_fmv_x_s,
  op_fclass_s,
  op_fmv_s_x,
  op_fcvt_l_s,
  op_fcvt_lu_s,
  op_fcvt_s_l,
  op_fcvt_s_lu,
  op_fld,
  op_fsd,
  op_fmadd_d,
  op_fmsub_d,
  op_fnmsub_d,
  op_fnmadd_d,
  op_fadd_d,
  op_fsub_d,
  op_fmul_d,
  op_fdiv_d,
  op_fsgnj_d,
  op_fsgnjn_d,
  op_fsgnjx_d,
  op_fmin_d,
  op_fmax_d,
  op_fcvt_s_d,
  op_fcvt_d_s,
  op_fsqrt_d,
  op_fle_d,
  op_flt_d,
  op_feq_d,
  op_fcvt_w_d,
  op_fcvt_wu_d,
  op_fcvt_d_w,
  op_fcvt_d_wu,
  op_fclass_d,
  op_fcvt_l_d,
  op_fcvt_lu_d,
  op_fmv_x_d,
  op_fcvt_d_l,
  op_fcvt_d_lu,
  op_fmv_d_x,
  op_flq,
  op_fsq,
  op_fmadd_q,
  op_fmsub_q,
  op_fnmsub_q,
  op_fnmadd_q,
  op_fadd_q,
  op_fsub_q,
  op_fmul_q,
  op_fdiv_q,
  op_fsgnj_q,
  op_fsgnjn_q,
  op_fsgnjx_q,
  op_fmin_q,
  op_fmax_q,
  op_fcvt_s_q,
  op_fcvt_q_s,
  op_fcvt_d_q,
  op_fcvt_q_d,
  op_fsqrt_q,
  op_fle_q,
  op_flt_q,
  op_feq_q,
  op_fcvt_w_q,
  op_fcvt_wu_q,
  op_fcvt_q_w,
  op_fcvt_q_wu,
  op_fclass_q,
  op_fcvt_l_q,
  op_fcvt_lu_q,
  op_fcvt_q_l,
  op_fcvt_q_lu,
  op_fmv_x_q,
  op_fmv_q_x,
  op_c_addi4spn,
  op_c_fld,
  op_c_lw,
  op_c_flw,
  op_c_fsd,
  op_c_sw,
  op_c_fsw,
  op_c_nop,
  op_c_addi,
  op_c_jal,
  op_c_li,
  op_c_addi16sp,
  op_c_lui,
  op_c_srli,
  op_c_srai,
  op_c_andi,
  op_c_sub,
  op_c_xor,
  op_c_or,
  op_c_and,
  op_c_subw,
  op_c_addw,
  op_c_j,
  op_c_beqz,
  op_c_bnez,
  op_c_slli,
  op_c_fldsp,
  op_c_lwsp,
  op_c_flwsp,
  op_c_jr,
  op_c_mv,
  op_c_ebreak,
  op_c_jalr,
  op_c_add,
  op_c_fsdsp,
  op_c_swsp,
  op_c_fswsp,
  op_c_ld,
  op_c_sd,
  op_c_addiw,
  op_c_ldsp,
  op_c_sdsp,
  op_c_lq,
  op_c_sq,
  op_c_lqsp,
  op_c_sqsp,
  op_nop,
  op_mv,
  op_not,
  op_neg,
  op_negw,
  op_sext_w,
  op_seqz,
  op_snez,
  op_sltz,
  op_sgtz,
  op_fmv_s,
  op_fabs_s,
  op_fneg_s,
  op_fmv_d,
  op_fabs_d,
  op_fneg_d,
  op_fmv_q,
  op_fabs_q,
  op_fneg_q,
  op_beqz,
  op_bnez,
  op_blez,
  op_bgez,
  op_bltz,
  op_bgtz,
  op_ble,
  op_bleu,
  op_bgt,
  op_bgtu,
  op_j,
  op_ret,
  op_jr,
  op_rdcycle,
  op_rdtime,
  op_rdinstret,
  op_rdcycleh,
  op_rdtimeh,
  op_rdinstreth,
  op_frcsr,
  op_frrm,
  op_frflags,
  op_fscsr,
  op_fsrm,
  op_fsflags,
  op_fsrmi,
  op_fsflagsi,
} rv_op;

/* structures */

typedef struct {
  uint64_t pc;
  uint64_t inst;
  int32_t imm;
  uint16_t op;
  uint8_t encodingType;
  uint8_t rd;
  uint8_t rs1;
  uint8_t rs2;
  uint8_t rs3;
  uint8_t rm;
  uint8_t pred;
  uint8_t succ;
  uint8_t aq;
  uint8_t rl;
} rv_instInfo;

typedef struct {
  const int32_t op;
  const rvc_constraint *constraints;
} rv_compData;

enum { rvcd_imm_nz = 0x1, rvcd_imm_nz_hint = 0x2 };

typedef struct {
  const char *const name;
  const rv_type encodingType;
  const char *const format;
  const rv_compData *pseudo;
  const short decomp_rv32;
  const short decomp_rv64;
  const short decomp_rv128;
  const short decomp_data;
} rv_opcodeData;

/* register names */

static const char rv_i_reg_abi_name[32][5] = {
    "zero", "ra", "sp", "gp", "tp",  "t0",  "t1", "t2", "s0", "s1", "a0",
    "a1",   "a2", "a3", "a4", "a5",  "a6",  "a7", "s2", "s3", "s4", "s5",
    "s6",   "s7", "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6",
};

static const char rv_f_reg_abi_name[32][5] = {
    "ft0", "ft1", "ft2",  "ft3",  "ft4", "ft5", "ft6",  "ft7",
    "fs0", "fs1", "fa0",  "fa1",  "fa2", "fa3", "fa4",  "fa5",
    "fa6", "fa7", "fs2",  "fs3",  "fs4", "fs5", "fs6",  "fs7",
    "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11",
};

/* instruction formats */

static const char rv_fmt_none[] = "O\t";
static const char rv_fmt_rs1[] = "O\t1";
static const char rv_fmt_offset[] = "O\to";
static const char rv_fmt_pred_succ[] = "O\tp,s";
static const char rv_fmt_rs1_rs2[] = "O\t1,2";
static const char rv_fmt_rd_imm[] = "O\t0,i";
static const char rv_fmt_rd_offset[] = "O\t0,o";
static const char rv_fmt_rd_rs1_rs2[] = "O\t0,1,2";
static const char rv_fmt_frd_rs1[] = "O\t3,1";
static const char rv_fmt_rd_frs1[] = "O\t0,4";
static const char rv_fmt_rd_frs1_frs2[] = "O\t0,4,5";
static const char rv_fmt_frd_frs1_frs2[] = "O\t3,4,5";
static const char rv_fmt_rm_frd_frs1[] = "O\tr,3,4";
static const char rv_fmt_rm_frd_rs1[] = "O\tr,3,1";
static const char rv_fmt_rm_rd_frs1[] = "O\tr,0,4";
static const char rv_fmt_rm_frd_frs1_frs2[] = "O\tr,3,4,5";
static const char rv_fmt_rm_frd_frs1_frs2_frs3[] = "O\tr,3,4,5,6";
static const char rv_fmt_rd_rs1_imm[] = "O\t0,1,i";
static const char rv_fmt_rd_rs1_offset[] = "O\t0,1,i";
static const char rv_fmt_rd_offset_rs1[] = "O\t0,i(1)";
static const char rv_fmt_frd_offset_rs1[] = "O\t3,i(1)";
static const char rv_fmt_rd_csr_rs1[] = "O\t0,c,1";
static const char rv_fmt_rd_csr_zimm[] = "O\t0,c,7";
static const char rv_fmt_rs2_offset_rs1[] = "O\t2,i(1)";
static const char rv_fmt_frs2_offset_rs1[] = "O\t5,i(1)";
static const char rv_fmt_rs1_rs2_offset[] = "O\t1,2,o";
static const char rv_fmt_rs2_rs1_offset[] = "O\t2,1,o";
static const char rv_fmt_aqrl_rd_rs2_rs1[] = "OAR\t0,2,(1)";
static const char rv_fmt_aqrl_rd_rs1[] = "OAR\t0,(1)";
static const char rv_fmt_rd[] = "O\t0";
static const char rv_fmt_rd_zimm[] = "O\t0,7";
static const char rv_fmt_rd_rs1[] = "O\t0,1";
static const char rv_fmt_rd_rs2[] = "O\t0,2";
static const char rv_fmt_rs1_offset[] = "O\t1,o";
static const char rv_fmt_rs2_offset[] = "O\t2,o";

/* pseudo-instruction constraints */

static const rvc_constraint rvcc_last[] = {rvc_end};
static const rvc_constraint rvcc_imm_eq_zero[] = {rvc_imm_eq_zero, rvc_end};
static const rvc_constraint rvcc_imm_eq_n1[] = {rvc_imm_eq_n1, rvc_end};
static const rvc_constraint rvcc_imm_eq_p1[] = {rvc_imm_eq_p1, rvc_end};
static const rvc_constraint rvcc_rs1_eq_x0[] = {rvc_rs1_eq_x0, rvc_end};
static const rvc_constraint rvcc_rs2_eq_x0[] = {rvc_rs2_eq_x0, rvc_end};
static const rvc_constraint rvcc_rs2_eq_rs1[] = {rvc_rs2_eq_rs1, rvc_end};
static const rvc_constraint rvcc_jal_j[] = {rvc_rd_eq_x0, rvc_end};
static const rvc_constraint rvcc_jal_jal[] = {rvc_rd_eq_ra, rvc_end};
static const rvc_constraint rvcc_jalr_jr[] = {rvc_rd_eq_x0, rvc_imm_eq_zero,
                                              rvc_end};
static const rvc_constraint rvcc_jalr_jalr[] = {rvc_rd_eq_ra, rvc_imm_eq_zero,
                                                rvc_end};
static const rvc_constraint rvcc_jalr_ret[] = {rvc_rd_eq_x0, rvc_rs1_eq_ra,
                                               rvc_end};
static const rvc_constraint rvcc_addi_nop[] = {rvc_rd_eq_x0, rvc_rs1_eq_x0,
                                               rvc_imm_eq_zero, rvc_end};
static const rvc_constraint rvcc_rdcycle[] = {rvc_rs1_eq_x0, rvc_csr_eq_0xc00,
                                              rvc_end};
static const rvc_constraint rvcc_rdtime[] = {rvc_rs1_eq_x0, rvc_csr_eq_0xc01,
                                             rvc_end};
static const rvc_constraint rvcc_rdinstret[] = {rvc_rs1_eq_x0, rvc_csr_eq_0xc02,
                                                rvc_end};
static const rvc_constraint rvcc_rdcycleh[] = {rvc_rs1_eq_x0, rvc_csr_eq_0xc80,
                                               rvc_end};
static const rvc_constraint rvcc_rdtimeh[] = {rvc_rs1_eq_x0, rvc_csr_eq_0xc81,
                                              rvc_end};
static const rvc_constraint rvcc_rdinstreth[] = {rvc_rs1_eq_x0,
                                                 rvc_csr_eq_0xc82, rvc_end};
static const rvc_constraint rvcc_frcsr[] = {rvc_rs1_eq_x0, rvc_csr_eq_0x003,
                                            rvc_end};
static const rvc_constraint rvcc_frrm[] = {rvc_rs1_eq_x0, rvc_csr_eq_0x002,
                                           rvc_end};
static const rvc_constraint rvcc_frflags[] = {rvc_rs1_eq_x0, rvc_csr_eq_0x001,
                                              rvc_end};
static const rvc_constraint rvcc_fscsr[] = {rvc_csr_eq_0x003, rvc_end};
static const rvc_constraint rvcc_fsrm[] = {rvc_csr_eq_0x002, rvc_end};
static const rvc_constraint rvcc_fsflags[] = {rvc_csr_eq_0x001, rvc_end};
static const rvc_constraint rvcc_fsrmi[] = {rvc_csr_eq_0x002, rvc_end};
static const rvc_constraint rvcc_fsflagsi[] = {rvc_csr_eq_0x001, rvc_end};

/* pseudo-instruction metadata */

static const rv_compData rvcp_jal[] = {
    {op_j, rvcc_jal_j}, {op_jal, rvcc_jal_jal}, {op_illegal, NULL}};

static const rv_compData rvcp_jalr[] = {{op_ret, rvcc_jalr_ret},
                                        {op_jr, rvcc_jalr_jr},
                                        {op_jalr, rvcc_jalr_jalr},
                                        {op_illegal, NULL}};

static const rv_compData rvcp_beq[] = {{op_beqz, rvcc_rs2_eq_x0},
                                       {op_illegal, NULL}};

static const rv_compData rvcp_bne[] = {{op_bnez, rvcc_rs2_eq_x0},
                                       {op_illegal, NULL}};

static const rv_compData rvcp_blt[] = {{op_bltz, rvcc_rs2_eq_x0},
                                       {op_bgtz, rvcc_rs1_eq_x0},
                                       {op_bgt, rvcc_last},
                                       {op_illegal, NULL}};

static const rv_compData rvcp_bge[] = {{op_blez, rvcc_rs1_eq_x0},
                                       {op_bgez, rvcc_rs2_eq_x0},
                                       {op_ble, rvcc_last},
                                       {op_illegal, NULL}};

static const rv_compData rvcp_bltu[] = {{op_bgtu, rvcc_last},
                                        {op_illegal, NULL}};

static const rv_compData rvcp_bgeu[] = {{op_bleu, rvcc_last},
                                        {op_illegal, NULL}};

static const rv_compData rvcp_addi[] = {
    {op_nop, rvcc_addi_nop}, {op_mv, rvcc_imm_eq_zero}, {op_illegal, NULL}};

static const rv_compData rvcp_sltiu[] = {{op_seqz, rvcc_imm_eq_p1},
                                         {op_illegal, NULL}};

static const rv_compData rvcp_xori[] = {{op_not, rvcc_imm_eq_n1},
                                        {op_illegal, NULL}};

static const rv_compData rvcp_sub[] = {{op_neg, rvcc_rs1_eq_x0},
                                       {op_illegal, NULL}};

static const rv_compData rvcp_slt[] = {
    {op_sltz, rvcc_rs2_eq_x0}, {op_sgtz, rvcc_rs1_eq_x0}, {op_illegal, NULL}};

static const rv_compData rvcp_sltu[] = {{op_snez, rvcc_rs1_eq_x0},
                                        {op_illegal, NULL}};

static const rv_compData rvcp_addiw[] = {{op_sext_w, rvcc_imm_eq_zero},
                                         {op_illegal, NULL}};

static const rv_compData rvcp_subw[] = {{op_negw, rvcc_rs1_eq_x0},
                                        {op_illegal, NULL}};

static const rv_compData rvcp_csrrw[] = {{op_fscsr, rvcc_fscsr},
                                         {op_fsrm, rvcc_fsrm},
                                         {op_fsflags, rvcc_fsflags},
                                         {op_illegal, NULL}};

static const rv_compData rvcp_csrrs[] = {
    {op_rdcycle, rvcc_rdcycle},     {op_rdtime, rvcc_rdtime},
    {op_rdinstret, rvcc_rdinstret}, {op_rdcycleh, rvcc_rdcycleh},
    {op_rdtimeh, rvcc_rdtimeh},     {op_rdinstreth, rvcc_rdinstreth},
    {op_frcsr, rvcc_frcsr},         {op_frrm, rvcc_frrm},
    {op_frflags, rvcc_frflags},     {op_illegal, NULL}};

static const rv_compData rvcp_csrrwi[] = {
    {op_fsrmi, rvcc_fsrmi}, {op_fsflagsi, rvcc_fsflagsi}, {op_illegal, NULL}};

static const rv_compData rvcp_fsgnj_s[] = {{op_fmv_s, rvcc_rs2_eq_rs1},
                                           {op_illegal, NULL}};

static const rv_compData rvcp_fsgnjn_s[] = {{op_fneg_s, rvcc_rs2_eq_rs1},
                                            {op_illegal, NULL}};

static const rv_compData rvcp_fsgnjx_s[] = {{op_fabs_s, rvcc_rs2_eq_rs1},
                                            {op_illegal, NULL}};

static const rv_compData rvcp_fsgnj_d[] = {{op_fmv_d, rvcc_rs2_eq_rs1},
                                           {op_illegal, NULL}};

static const rv_compData rvcp_fsgnjn_d[] = {{op_fneg_d, rvcc_rs2_eq_rs1},
                                            {op_illegal, NULL}};

static const rv_compData rvcp_fsgnjx_d[] = {{op_fabs_d, rvcc_rs2_eq_rs1},
                                            {op_illegal, NULL}};

static const rv_compData rvcp_fsgnj_q[] = {{op_fmv_q, rvcc_rs2_eq_rs1},
                                           {op_illegal, NULL}};

static const rv_compData rvcp_fsgnjn_q[] = {{op_fneg_q, rvcc_rs2_eq_rs1},
                                            {op_illegal, NULL}};

static const rv_compData rvcp_fsgnjx_q[] = {{op_fabs_q, rvcc_rs2_eq_rs1},
                                            {op_illegal, NULL}};

/* instruction metadata */

const rv_opcodeData opcode_data[] = {
    {"illegal", type_illegal, rv_fmt_none, NULL, 0, 0, 0},
    {"lui", type_u, rv_fmt_rd_imm, NULL, 0, 0, 0},
    {"auipc", type_u, rv_fmt_rd_offset, NULL, 0, 0, 0},
    {"jal", type_uj, rv_fmt_rd_offset, rvcp_jal, 0, 0, 0},
    {"jalr", type_i, rv_fmt_rd_rs1_offset, rvcp_jalr, 0, 0, 0},
    {"beq", type_sb, rv_fmt_rs1_rs2_offset, rvcp_beq, 0, 0, 0},
    {"bne", type_sb, rv_fmt_rs1_rs2_offset, rvcp_bne, 0, 0, 0},
    {"blt", type_sb, rv_fmt_rs1_rs2_offset, rvcp_blt, 0, 0, 0},
    {"bge", type_sb, rv_fmt_rs1_rs2_offset, rvcp_bge, 0, 0, 0},
    {"bltu", type_sb, rv_fmt_rs1_rs2_offset, rvcp_bltu, 0, 0, 0},
    {"bgeu", type_sb, rv_fmt_rs1_rs2_offset, rvcp_bgeu, 0, 0, 0},
    {"lb", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"lh", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"lw", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"lbu", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"lhu", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"sb", type_s, rv_fmt_rs2_offset_rs1, NULL, 0, 0, 0},
    {"sh", type_s, rv_fmt_rs2_offset_rs1, NULL, 0, 0, 0},
    {"sw", type_s, rv_fmt_rs2_offset_rs1, NULL, 0, 0, 0},
    {"addi", type_i, rv_fmt_rd_rs1_imm, rvcp_addi, 0, 0, 0},
    {"slti", type_i, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"sltiu", type_i, rv_fmt_rd_rs1_imm, rvcp_sltiu, 0, 0, 0},
    {"xori", type_i, rv_fmt_rd_rs1_imm, rvcp_xori, 0, 0, 0},
    {"ori", type_i, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"andi", type_i, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"slli", type_i_sh7, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"srli", type_i_sh7, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"srai", type_i_sh7, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"add", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"sub", type_r, rv_fmt_rd_rs1_rs2, rvcp_sub, 0, 0, 0},
    {"sll", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"slt", type_r, rv_fmt_rd_rs1_rs2, rvcp_slt, 0, 0, 0},
    {"sltu", type_r, rv_fmt_rd_rs1_rs2, rvcp_sltu, 0, 0, 0},
    {"xor", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"srl", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"sra", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"or", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"and", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"fence", type_r_f, rv_fmt_pred_succ, NULL, 0, 0, 0},
    {"fence.i", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"lwu", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"ld", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"sd", type_s, rv_fmt_rs2_offset_rs1, NULL, 0, 0, 0},
    {"addiw", type_i, rv_fmt_rd_rs1_imm, rvcp_addiw, 0, 0, 0},
    {"slliw", type_i_sh5, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"srliw", type_i_sh5, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"sraiw", type_i_sh5, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"addw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"subw", type_r, rv_fmt_rd_rs1_rs2, rvcp_subw, 0, 0, 0},
    {"sllw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"srlw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"sraw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"ldu", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"lq", type_i, rv_fmt_rd_offset_rs1, NULL, 0, 0, 0},
    {"sq", type_s, rv_fmt_rs2_offset_rs1, NULL, 0, 0, 0},
    {"addid", type_i, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"sllid", type_i_sh6, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"srlid", type_i_sh6, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"sraid", type_i_sh6, rv_fmt_rd_rs1_imm, NULL, 0, 0, 0},
    {"addd", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"subd", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"slld", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"srld", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"srad", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"mul", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"mulh", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"mulhsu", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"mulhu", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"div", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"divu", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"rem", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"remu", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"mulw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"divw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"divuw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"remw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"remuw", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"muld", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"divd", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"divud", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"remd", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"remud", type_r, rv_fmt_rd_rs1_rs2, NULL, 0, 0, 0},
    {"lr.w", type_r_l, rv_fmt_aqrl_rd_rs1, NULL, 0, 0, 0},
    {"sc.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoswap.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoadd.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoxor.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoor.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoand.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomin.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomax.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amominu.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomaxu.w", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"lr.d", type_r_l, rv_fmt_aqrl_rd_rs1, NULL, 0, 0, 0},
    {"sc.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoswap.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoadd.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoxor.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoor.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoand.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomin.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomax.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amominu.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomaxu.d", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"lr.q", type_r_l, rv_fmt_aqrl_rd_rs1, NULL, 0, 0, 0},
    {"sc.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoswap.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoadd.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoxor.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoor.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amoand.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomin.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomax.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amominu.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"amomaxu.q", type_r_a, rv_fmt_aqrl_rd_rs2_rs1, NULL, 0, 0, 0},
    {"ecall", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"ebreak", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"uret", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"sret", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"hret", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"mret", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"dret", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"sfence.vm", type_r, rv_fmt_rs1, NULL, 0, 0, 0},
    {"sfence.vma", type_r, rv_fmt_rs1_rs2, NULL, 0, 0, 0},
    {"wfi", type_none, rv_fmt_none, NULL, 0, 0, 0},
    {"csrrw", type_i_csr, rv_fmt_rd_csr_rs1, rvcp_csrrw, 0, 0, 0},
    {"csrrs", type_i_csr, rv_fmt_rd_csr_rs1, rvcp_csrrs, 0, 0, 0},
    {"csrrc", type_i_csr, rv_fmt_rd_csr_rs1, NULL, 0, 0, 0},
    {"csrrwi", type_i_csr, rv_fmt_rd_csr_zimm, rvcp_csrrwi, 0, 0, 0},
    {"csrrsi", type_i_csr, rv_fmt_rd_csr_zimm, NULL, 0, 0, 0},
    {"csrrci", type_i_csr, rv_fmt_rd_csr_zimm, NULL, 0, 0, 0},
    {"flw", type_i, rv_fmt_frd_offset_rs1, NULL, 0, 0, 0},
    {"fsw", type_s, rv_fmt_frs2_offset_rs1, NULL, 0, 0, 0},
    {"fmadd.s", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fmsub.s", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fnmsub.s", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fnmadd.s", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fadd.s", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsub.s", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fmul.s", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fdiv.s", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsgnj.s", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnj_s, 0, 0, 0},
    {"fsgnjn.s", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnjn_s, 0, 0, 0},
    {"fsgnjx.s", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnjx_s, 0, 0, 0},
    {"fmin.s", type_r, rv_fmt_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fmax.s", type_r, rv_fmt_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsqrt.s", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fle.s", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"flt.s", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"feq.s", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"fcvt.w.s", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.wu.s", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.s.w", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.s.wu", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fmv.x.s", type_r, rv_fmt_rd_frs1, NULL, 0, 0, 0},
    {"fclass.s", type_r, rv_fmt_rd_frs1, NULL, 0, 0, 0},
    {"fmv.s.x", type_r, rv_fmt_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.l.s", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.lu.s", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.s.l", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.s.lu", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fld", type_i, rv_fmt_frd_offset_rs1, NULL, 0, 0, 0},
    {"fsd", type_s, rv_fmt_frs2_offset_rs1, NULL, 0, 0, 0},
    {"fmadd.d", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fmsub.d", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fnmsub.d", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fnmadd.d", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fadd.d", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsub.d", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fmul.d", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fdiv.d", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsgnj.d", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnj_d, 0, 0, 0},
    {"fsgnjn.d", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnjn_d, 0, 0, 0},
    {"fsgnjx.d", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnjx_d, 0, 0, 0},
    {"fmin.d", type_r, rv_fmt_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fmax.d", type_r, rv_fmt_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fcvt.s.d", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fcvt.d.s", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fsqrt.d", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fle.d", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"flt.d", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"feq.d", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"fcvt.w.d", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.wu.d", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.d.w", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.d.wu", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fclass.d", type_r, rv_fmt_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.l.d", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.lu.d", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fmv.x.d", type_r, rv_fmt_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.d.l", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.d.lu", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fmv.d.x", type_r, rv_fmt_frd_rs1, NULL, 0, 0, 0},
    {"flq", type_i, rv_fmt_frd_offset_rs1, NULL, 0, 0, 0},
    {"fsq", type_s, rv_fmt_frs2_offset_rs1, NULL, 0, 0, 0},
    {"fmadd.q", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fmsub.q", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fnmsub.q", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fnmadd.q", type_r4_m, rv_fmt_rm_frd_frs1_frs2_frs3, NULL, 0, 0, 0},
    {"fadd.q", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsub.q", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fmul.q", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fdiv.q", type_r_m, rv_fmt_rm_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fsgnj.q", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnj_q, 0, 0, 0},
    {"fsgnjn.q", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnjn_q, 0, 0, 0},
    {"fsgnjx.q", type_r, rv_fmt_frd_frs1_frs2, rvcp_fsgnjx_q, 0, 0, 0},
    {"fmin.q", type_r, rv_fmt_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fmax.q", type_r, rv_fmt_frd_frs1_frs2, NULL, 0, 0, 0},
    {"fcvt.s.q", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fcvt.q.s", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fcvt.d.q", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fcvt.q.d", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fsqrt.q", type_r_m, rv_fmt_rm_frd_frs1, NULL, 0, 0, 0},
    {"fle.q", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"flt.q", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"feq.q", type_r, rv_fmt_rd_frs1_frs2, NULL, 0, 0, 0},
    {"fcvt.w.q", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.wu.q", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.q.w", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.q.wu", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fclass.q", type_r, rv_fmt_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.l.q", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.lu.q", type_r_m, rv_fmt_rm_rd_frs1, NULL, 0, 0, 0},
    {"fcvt.q.l", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fcvt.q.lu", type_r_m, rv_fmt_rm_frd_rs1, NULL, 0, 0, 0},
    {"fmv.x.q", type_r, rv_fmt_rd_frs1, NULL, 0, 0, 0},
    {"fmv.q.x", type_r, rv_fmt_frd_rs1, NULL, 0, 0, 0},
    {"c.addi4spn", type_ciw_4spn, rv_fmt_rd_rs1_imm, NULL, op_addi, op_addi,
     op_addi, rvcd_imm_nz},
    {"c.fld", type_cl_ld, rv_fmt_frd_offset_rs1, NULL, op_fld, op_fld, 0},
    {"c.lw", type_cl_lw, rv_fmt_rd_offset_rs1, NULL, op_lw, op_lw, op_lw},
    {"c.flw", type_cl_lw, rv_fmt_frd_offset_rs1, NULL, op_flw, 0, 0},
    {"c.fsd", type_cs_sd, rv_fmt_frs2_offset_rs1, NULL, op_fsd, op_fsd, 0},
    {"c.sw", type_cs_sw, rv_fmt_rs2_offset_rs1, NULL, op_sw, op_sw, op_sw},
    {"c.fsw", type_cs_sw, rv_fmt_frs2_offset_rs1, NULL, op_fsw, 0, 0},
    {"c.nop", type_ci_none, rv_fmt_none, NULL, op_addi, op_addi, op_addi},
    {"c.addi", type_ci, rv_fmt_rd_rs1_imm, NULL, op_addi, op_addi, op_addi,
     rvcd_imm_nz_hint},
    {"c.jal", type_cj_jal, rv_fmt_rd_offset, NULL, op_jal, 0, 0},
    {"c.li", type_ci_li, rv_fmt_rd_rs1_imm, NULL, op_addi, op_addi, op_addi},
    {"c.addi16sp", type_ci_16sp, rv_fmt_rd_rs1_imm, NULL, op_addi, op_addi,
     op_addi, rvcd_imm_nz},
    {"c.lui", type_ci_lui, rv_fmt_rd_imm, NULL, op_lui, op_lui, op_lui,
     rvcd_imm_nz},
    {"c.srli", type_cb_sh6, rv_fmt_rd_rs1_imm, NULL, op_srli, op_srli, op_srli,
     rvcd_imm_nz},
    {"c.srai", type_cb_sh6, rv_fmt_rd_rs1_imm, NULL, op_srai, op_srai, op_srai,
     rvcd_imm_nz},
    {"c.andi", type_cb_imm, rv_fmt_rd_rs1_imm, NULL, op_andi, op_andi, op_andi,
     rvcd_imm_nz},
    {"c.sub", type_cs, rv_fmt_rd_rs1_rs2, NULL, op_sub, op_sub, op_sub},
    {"c.xor", type_cs, rv_fmt_rd_rs1_rs2, NULL, op_xor, op_xor, op_xor},
    {"c.or", type_cs, rv_fmt_rd_rs1_rs2, NULL, op_or, op_or, op_or},
    {"c.and", type_cs, rv_fmt_rd_rs1_rs2, NULL, op_and, op_and, op_and},
    {"c.subw", type_cs, rv_fmt_rd_rs1_rs2, NULL, op_subw, op_subw, op_subw},
    {"c.addw", type_cs, rv_fmt_rd_rs1_rs2, NULL, op_addw, op_addw, op_addw},
    {"c.j", type_cj, rv_fmt_rd_offset, NULL, op_jal, op_jal, op_jal},
    {"c.beqz", type_cb, rv_fmt_rs1_rs2_offset, NULL, op_beq, op_beq, op_beq},
    {"c.bnez", type_cb, rv_fmt_rs1_rs2_offset, NULL, op_bne, op_bne, op_bne},
    {"c.slli", type_ci_sh6, rv_fmt_rd_rs1_imm, NULL, op_slli, op_slli, op_slli,
     rvcd_imm_nz},
    {"c.fldsp", type_ci_ldsp, rv_fmt_frd_offset_rs1, NULL, op_fld, op_fld,
     op_fld},
    {"c.lwsp", type_ci_lwsp, rv_fmt_rd_offset_rs1, NULL, op_lw, op_lw, op_lw},
    {"c.flwsp", type_ci_lwsp, rv_fmt_frd_offset_rs1, NULL, op_flw, 0, 0},
    {"c.jr", type_cr_jr, rv_fmt_rd_rs1_offset, NULL, op_jalr, op_jalr, op_jalr},
    {"c.mv", type_cr_mv, rv_fmt_rd_rs1_rs2, NULL, op_addi, op_addi, op_addi},
    {"c.ebreak", type_ci_none, rv_fmt_none, NULL, op_ebreak, op_ebreak,
     op_ebreak},
    {"c.jalr", type_cr_jalr, rv_fmt_rd_rs1_offset, NULL, op_jalr, op_jalr,
     op_jalr},
    {"c.add", type_cr, rv_fmt_rd_rs1_rs2, NULL, op_add, op_add, op_add},
    {"c.fsdsp", type_css_sdsp, rv_fmt_frs2_offset_rs1, NULL, op_fsd, op_fsd,
     op_fsd},
    {"c.swsp", type_css_swsp, rv_fmt_rs2_offset_rs1, NULL, op_sw, op_sw, op_sw},
    {"c.fswsp", type_css_swsp, rv_fmt_frs2_offset_rs1, NULL, op_fsw, 0, 0},
    {"c.ld", type_cl_ld, rv_fmt_rd_offset_rs1, NULL, 0, op_ld, op_ld},
    {"c.sd", type_cs_sd, rv_fmt_rs2_offset_rs1, NULL, 0, op_sd, op_sd},
    {"c.addiw", type_ci, rv_fmt_rd_rs1_imm, NULL, 0, op_addiw, op_addiw},
    {"c.ldsp", type_ci_ldsp, rv_fmt_rd_offset_rs1, NULL, 0, op_ld, op_ld},
    {"c.sdsp", type_css_sdsp, rv_fmt_rs2_offset_rs1, NULL, 0, op_sd, op_sd},
    {"c.lq", type_cl_lq, rv_fmt_rd_offset_rs1, NULL, 0, 0, op_lq},
    {"c.sq", type_cs_sq, rv_fmt_rs2_offset_rs1, NULL, 0, 0, op_sq},
    {"c.lqsp", type_ci_lqsp, rv_fmt_rd_offset_rs1, NULL, 0, 0, op_lq},
    {"c.sqsp", type_css_sqsp, rv_fmt_rs2_offset_rs1, NULL, 0, 0, op_sq},
    {"nop", type_i, rv_fmt_none, NULL, 0, 0, 0},
    {"mv", type_i, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"not", type_i, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"neg", type_r, rv_fmt_rd_rs2, NULL, 0, 0, 0},
    {"negw", type_r, rv_fmt_rd_rs2, NULL, 0, 0, 0},
    {"sext.w", type_i, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"seqz", type_i, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"snez", type_r, rv_fmt_rd_rs2, NULL, 0, 0, 0},
    {"sltz", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"sgtz", type_r, rv_fmt_rd_rs2, NULL, 0, 0, 0},
    {"fmv.s", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fabs.s", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fneg.s", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fmv.d", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fabs.d", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fneg.d", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fmv.q", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fabs.q", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fneg.q", type_r, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"beqz", type_sb, rv_fmt_rs1_offset, NULL, 0, 0, 0},
    {"bnez", type_sb, rv_fmt_rs1_offset, NULL, 0, 0, 0},
    {"blez", type_sb, rv_fmt_rs2_offset, NULL, 0, 0, 0},
    {"bgez", type_sb, rv_fmt_rs1_offset, NULL, 0, 0, 0},
    {"bltz", type_sb, rv_fmt_rs1_offset, NULL, 0, 0, 0},
    {"bgtz", type_sb, rv_fmt_rs2_offset, NULL, 0, 0, 0},
    {"ble", type_sb, rv_fmt_rs2_rs1_offset, NULL, 0, 0, 0},
    {"bleu", type_sb, rv_fmt_rs2_rs1_offset, NULL, 0, 0, 0},
    {"bgt", type_sb, rv_fmt_rs2_rs1_offset, NULL, 0, 0, 0},
    {"bgtu", type_sb, rv_fmt_rs2_rs1_offset, NULL, 0, 0, 0},
    {"j", type_uj, rv_fmt_offset, NULL, 0, 0, 0},
    {"ret", type_i, rv_fmt_none, NULL, 0, 0, 0},
    {"jr", type_i, rv_fmt_rs1, NULL, 0, 0, 0},
    {"rdcycle", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"rdtime", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"rdinstret", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"rdcycleh", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"rdtimeh", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"rdinstreth", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"frcsr", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"frrm", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"frflags", type_i_csr, rv_fmt_rd, NULL, 0, 0, 0},
    {"fscsr", type_i_csr, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fsrm", type_i_csr, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fsflags", type_i_csr, rv_fmt_rd_rs1, NULL, 0, 0, 0},
    {"fsrmi", type_i_csr, rv_fmt_rd_zimm, NULL, 0, 0, 0},
    {"fsflagsi", type_i_csr, rv_fmt_rd_zimm, NULL, 0, 0, 0},
};

/* instruction length */

static size_t inst_length(rv_inst inst) {
  /* NOTE: supports maximum instruction size of 64-bits */

  /* instruction length coding standard
   *
   *      aa - 16 bit aa != 11
   *   bbb11 - 32 bit bbb != 111
   *  011111 - 48 bit
   * 0111111 - 64 bit
   */

  return (inst & 0b11) != 0b11             ? 2
         : (inst & 0b11100) != 0b11100     ? 4
         : (inst & 0b111111) == 0b011111   ? 6
         : (inst & 0b1111111) == 0b0111111 ? 8
                                           : 0;
}

/* CSR names */

static const char *csr_name(int csrno) {
  switch (csrno) {
  case 0x0000:
    return "ustatus";
  case 0x0001:
    return "fflags";
  case 0x0002:
    return "frm";
  case 0x0003:
    return "fcsr";
  case 0x0004:
    return "uie";
  case 0x0005:
    return "utvec";
  case 0x0007:
    return "utvt";
  case 0x0008:
    return "vstart";
  case 0x0009:
    return "vxsat";
  case 0x000a:
    return "vxrm";
  case 0x000f:
    return "vcsr";
  case 0x0040:
    return "uscratch";
  case 0x0041:
    return "uepc";
  case 0x0042:
    return "ucause";
  case 0x0043:
    return "utval";
  case 0x0044:
    return "uip";
  case 0x0045:
    return "unxti";
  case 0x0046:
    return "uintstatus";
  case 0x0048:
    return "uscratchcsw";
  case 0x0049:
    return "uscratchcswl";
  case 0x0100:
    return "sstatus";
  case 0x0102:
    return "sedeleg";
  case 0x0103:
    return "sideleg";
  case 0x0104:
    return "sie";
  case 0x0105:
    return "stvec";
  case 0x0106:
    return "scounteren";
  case 0x0107:
    return "stvt";
  case 0x0140:
    return "sscratch";
  case 0x0141:
    return "sepc";
  case 0x0142:
    return "scause";
  case 0x0143:
    return "stval";
  case 0x0144:
    return "sip";
  case 0x0145:
    return "snxti";
  case 0x0146:
    return "sintstatus";
  case 0x0148:
    return "sscratchcsw";
  case 0x0149:
    return "sscratchcswl";
  case 0x0180:
    return "satp";
  case 0x0200:
    return "vsstatus";
  case 0x0204:
    return "vsie";
  case 0x0205:
    return "vstvec";
  case 0x0240:
    return "vsscratch";
  case 0x0241:
    return "vsepc";
  case 0x0242:
    return "vscause";
  case 0x0243:
    return "vstval";
  case 0x0244:
    return "vsip";
  case 0x0280:
    return "vsatp";
  case 0x0300:
    return "mstatus";
  case 0x0301:
    return "misa";
  case 0x0302:
    return "medeleg";
  case 0x0303:
    return "mideleg";
  case 0x0304:
    return "mie";
  case 0x0305:
    return "mtvec";
  case 0x0306:
    return "mcounteren";
  case 0x0307:
    return "mtvt";
  case 0x0310:
    return "mstatush";
  case 0x0320:
    return "mcountinhibit";
  case 0x0323:
    return "mhpmevent3";
  case 0x0324:
    return "mhpmevent4";
  case 0x0325:
    return "mhpmevent5";
  case 0x0326:
    return "mhpmevent6";
  case 0x0327:
    return "mhpmevent7";
  case 0x0328:
    return "mhpmevent8";
  case 0x0329:
    return "mhpmevent9";
  case 0x032a:
    return "mhpmevent10";
  case 0x032b:
    return "mhpmevent11";
  case 0x032c:
    return "mhpmevent12";
  case 0x032d:
    return "mhpmevent13";
  case 0x032e:
    return "mhpmevent14";
  case 0x032f:
    return "mhpmevent15";
  case 0x0330:
    return "mhpmevent16";
  case 0x0331:
    return "mhpmevent17";
  case 0x0332:
    return "mhpmevent18";
  case 0x0333:
    return "mhpmevent19";
  case 0x0334:
    return "mhpmevent20";
  case 0x0335:
    return "mhpmevent21";
  case 0x0336:
    return "mhpmevent22";
  case 0x0337:
    return "mhpmevent23";
  case 0x0338:
    return "mhpmevent24";
  case 0x0339:
    return "mhpmevent25";
  case 0x033a:
    return "mhpmevent26";
  case 0x033b:
    return "mhpmevent27";
  case 0x033c:
    return "mhpmevent28";
  case 0x033d:
    return "mhpmevent29";
  case 0x033e:
    return "mhpmevent30";
  case 0x033f:
    return "mhpmevent31";
  case 0x0340:
    return "mscratch";
  case 0x0341:
    return "mepc";
  case 0x0342:
    return "mcause";
  case 0x0343:
    return "mtval";
  case 0x0344:
    return "mip";
  case 0x0345:
    return "mnxti";
  case 0x0346:
    return "mintstatus";
  case 0x0348:
    return "mscratchcsw";
  case 0x0349:
    return "mscratchcswl";
  case 0x034a:
    return "mtinst";
  case 0x034b:
    return "mtval2";
  case 0x03a0:
    return "pmpcfg0";
  case 0x03a1:
    return "pmpcfg1";
  case 0x03a2:
    return "pmpcfg2";
  case 0x03a3:
    return "pmpcfg3";
  case 0x03b0:
    return "pmpaddr0";
  case 0x03b1:
    return "pmpaddr1";
  case 0x03b2:
    return "pmpaddr2";
  case 0x03b3:
    return "pmpaddr3";
  case 0x03b4:
    return "pmpaddr4";
  case 0x03b5:
    return "pmpaddr5";
  case 0x03b6:
    return "pmpaddr6";
  case 0x03b7:
    return "pmpaddr7";
  case 0x03b8:
    return "pmpaddr8";
  case 0x03b9:
    return "pmpaddr9";
  case 0x03ba:
    return "pmpaddr10";
  case 0x03bb:
    return "pmpaddr11";
  case 0x03bc:
    return "pmpaddr12";
  case 0x03bd:
    return "pmpaddr13";
  case 0x03be:
    return "pmpaddr14";
  case 0x03bf:
    return "pmpaddr15";
  case 0x0600:
    return "hstatus";
  case 0x0602:
    return "hedeleg";
  case 0x0603:
    return "hideleg";
  case 0x0604:
    return "hie";
  case 0x0605:
    return "htimedelta";
  case 0x0606:
    return "hcounteren";
  case 0x0607:
    return "hgeie";
  case 0x0615:
    return "htimedeltah";
  case 0x0643:
    return "htval";
  case 0x0644:
    return "hip";
  case 0x0645:
    return "hvip";
  case 0x064a:
    return "htinst";
  case 0x0680:
    return "hgatp";
  case 0x07a0:
    return "tselect";
  case 0x07a1:
    return "tdata1";
  case 0x07a2:
    return "tdata2";
  case 0x07a3:
    return "tdata3";
  case 0x07a4:
    return "tinfo";
  case 0x07a5:
    return "tcontrol";
  case 0x07a8:
    return "mcontext";
  case 0x07a9:
    return "mnoise";
  case 0x07aa:
    return "scontext";
  case 0x07b0:
    return "dcsr";
  case 0x07b1:
    return "dpc";
  case 0x07b2:
    return "dscratch0";
  case 0x07b3:
    return "dscratch1";
  case 0x0b00:
    return "mcycle";
  case 0x0b02:
    return "minstret";
  case 0x0b03:
    return "mhpmcounter3";
  case 0x0b04:
    return "mhpmcounter4";
  case 0x0b05:
    return "mhpmcounter5";
  case 0x0b06:
    return "mhpmcounter6";
  case 0x0b07:
    return "mhpmcounter7";
  case 0x0b08:
    return "mhpmcounter8";
  case 0x0b09:
    return "mhpmcounter9";
  case 0x0b0a:
    return "mhpmcounter10";
  case 0x0b0b:
    return "mhpmcounter11";
  case 0x0b0c:
    return "mhpmcounter12";
  case 0x0b0d:
    return "mhpmcounter13";
  case 0x0b0e:
    return "mhpmcounter14";
  case 0x0b0f:
    return "mhpmcounter15";
  case 0x0b10:
    return "mhpmcounter16";
  case 0x0b11:
    return "mhpmcounter17";
  case 0x0b12:
    return "mhpmcounter18";
  case 0x0b13:
    return "mhpmcounter19";
  case 0x0b14:
    return "mhpmcounter20";
  case 0x0b15:
    return "mhpmcounter21";
  case 0x0b16:
    return "mhpmcounter22";
  case 0x0b17:
    return "mhpmcounter23";
  case 0x0b18:
    return "mhpmcounter24";
  case 0x0b19:
    return "mhpmcounter25";
  case 0x0b1a:
    return "mhpmcounter26";
  case 0x0b1b:
    return "mhpmcounter27";
  case 0x0b1c:
    return "mhpmcounter28";
  case 0x0b1d:
    return "mhpmcounter29";
  case 0x0b1e:
    return "mhpmcounter30";
  case 0x0b1f:
    return "mhpmcounter31";
  case 0x0b80:
    return "mcycleh";
  case 0x0b82:
    return "minstreth";
  case 0x0b83:
    return "mhpmcounter3h";
  case 0x0b84:
    return "mhpmcounter4h";
  case 0x0b85:
    return "mhpmcounter5h";
  case 0x0b86:
    return "mhpmcounter6h";
  case 0x0b87:
    return "mhpmcounter7h";
  case 0x0b88:
    return "mhpmcounter8h";
  case 0x0b89:
    return "mhpmcounter9h";
  case 0x0b8a:
    return "mhpmcounter10h";
  case 0x0b8b:
    return "mhpmcounter11h";
  case 0x0b8c:
    return "mhpmcounter12h";
  case 0x0b8d:
    return "mhpmcounter13h";
  case 0x0b8e:
    return "mhpmcounter14h";
  case 0x0b8f:
    return "mhpmcounter15h";
  case 0x0b90:
    return "mhpmcounter16h";
  case 0x0b91:
    return "mhpmcounter17h";
  case 0x0b92:
    return "mhpmcounter18h";
  case 0x0b93:
    return "mhpmcounter19h";
  case 0x0b94:
    return "mhpmcounter20h";
  case 0x0b95:
    return "mhpmcounter21h";
  case 0x0b96:
    return "mhpmcounter22h";
  case 0x0b97:
    return "mhpmcounter23h";
  case 0x0b98:
    return "mhpmcounter24h";
  case 0x0b99:
    return "mhpmcounter25h";
  case 0x0b9a:
    return "mhpmcounter26h";
  case 0x0b9b:
    return "mhpmcounter27h";
  case 0x0b9c:
    return "mhpmcounter28h";
  case 0x0b9d:
    return "mhpmcounter29h";
  case 0x0b9e:
    return "mhpmcounter30h";
  case 0x0b9f:
    return "mhpmcounter31h";
  case 0x0c00:
    return "cycle";
  case 0x0c01:
    return "time";
  case 0x0c02:
    return "instret";
  case 0x0c03:
    return "hpmcounter3";
  case 0x0c04:
    return "hpmcounter4";
  case 0x0c05:
    return "hpmcounter5";
  case 0x0c06:
    return "hpmcounter6";
  case 0x0c07:
    return "hpmcounter7";
  case 0x0c08:
    return "hpmcounter8";
  case 0x0c09:
    return "hpmcounter9";
  case 0x0c0a:
    return "hpmcounter10";
  case 0x0c0b:
    return "hpmcounter11";
  case 0x0c0c:
    return "hpmcounter12";
  case 0x0c0d:
    return "hpmcounter13";
  case 0x0c0e:
    return "hpmcounter14";
  case 0x0c0f:
    return "hpmcounter15";
  case 0x0c10:
    return "hpmcounter16";
  case 0x0c11:
    return "hpmcounter17";
  case 0x0c12:
    return "hpmcounter18";
  case 0x0c13:
    return "hpmcounter19";
  case 0x0c14:
    return "hpmcounter20";
  case 0x0c15:
    return "hpmcounter21";
  case 0x0c16:
    return "hpmcounter22";
  case 0x0c17:
    return "hpmcounter23";
  case 0x0c18:
    return "hpmcounter24";
  case 0x0c19:
    return "hpmcounter25";
  case 0x0c1a:
    return "hpmcounter26";
  case 0x0c1b:
    return "hpmcounter27";
  case 0x0c1c:
    return "hpmcounter28";
  case 0x0c1d:
    return "hpmcounter29";
  case 0x0c1e:
    return "hpmcounter30";
  case 0x0c1f:
    return "hpmcounter31";
  case 0x0c20:
    return "vl";
  case 0x0c21:
    return "vtype";
  case 0x0c22:
    return "vlenb";
  case 0x0c80:
    return "cycleh";
  case 0x0c81:
    return "timeh";
  case 0x0c82:
    return "instreth";
  case 0x0c83:
    return "hpmcounter3h";
  case 0x0c84:
    return "hpmcounter4h";
  case 0x0c85:
    return "hpmcounter5h";
  case 0x0c86:
    return "hpmcounter6h";
  case 0x0c87:
    return "hpmcounter7h";
  case 0x0c88:
    return "hpmcounter8h";
  case 0x0c89:
    return "hpmcounter9h";
  case 0x0c8a:
    return "hpmcounter10h";
  case 0x0c8b:
    return "hpmcounter11h";
  case 0x0c8c:
    return "hpmcounter12h";
  case 0x0c8d:
    return "hpmcounter13h";
  case 0x0c8e:
    return "hpmcounter14h";
  case 0x0c8f:
    return "hpmcounter15h";
  case 0x0c90:
    return "hpmcounter16h";
  case 0x0c91:
    return "hpmcounter17h";
  case 0x0c92:
    return "hpmcounter18h";
  case 0x0c93:
    return "hpmcounter19h";
  case 0x0c94:
    return "hpmcounter20h";
  case 0x0c95:
    return "hpmcounter21h";
  case 0x0c96:
    return "hpmcounter22h";
  case 0x0c97:
    return "hpmcounter23h";
  case 0x0c98:
    return "hpmcounter24h";
  case 0x0c99:
    return "hpmcounter25h";
  case 0x0c9a:
    return "hpmcounter26h";
  case 0x0c9b:
    return "hpmcounter27h";
  case 0x0c9c:
    return "hpmcounter28h";
  case 0x0c9d:
    return "hpmcounter29h";
  case 0x0c9e:
    return "hpmcounter30h";
  case 0x0c9f:
    return "hpmcounter31h";
  case 0x0e12:
    return "hgeip";
  case 0x0f11:
    return "mvendorid";
  case 0x0f12:
    return "marchid";
  case 0x0f13:
    return "mimpid";
  case 0x0f14:
    return "mhartid";
  case 0x0f15:
    return "mentropy";
  default:
    return NULL;
  }
}

/* Parse opcode of all riscv instructions */

static void rv_inst_opcode(rv_instInfo *instInfo, rv_isa isa) {
  rv_inst inst = instInfo->inst;
  rv_opcode op = op_illegal;
  switch (((inst >> 0) & 0b11)) {
  case 0: // compression instruction instLen=16bits
    switch (((inst >> 13) & 0b111)) {
    case 0:
      op = op_c_addi4spn;
      break;
    case 1:
      op = (isa == rv128) ? op_c_lq : op_c_fld;
      break;
    case 2:
      op = op_c_lw;
      break;
    case 3:
      op = (isa == rv32) ? op_c_flw : op_c_ld;
      break;
    case 5:
      op = (isa == rv128) ? op_c_sq : op_c_fsd;
      break;
    case 6:
      op = op_c_sw;
      break;
    case 7:
      op = (isa == rv32) ? op_c_fsw : op_c_sd;
      break;
    }
    break;
  case 1: // compression instruction instLen=16bits
    switch (((inst >> 13) & 0b111)) {
    case 0:
      switch (((inst >> 2) & 0b11111111111)) {
      case 0:
        op = op_c_nop;
        break;
      default:
        op = op_c_addi;
        break;
      }
      break;
    case 1:
      op = (isa == rv32) ? op_c_jal : op_c_addiw;
      break;
    case 2:
      op = op_c_li;
      break;
    case 3:
      switch (((inst >> 7) & 0b11111)) {
      case 2:
        op = op_c_addi16sp;
        break;
      default:
        op = op_c_lui;
        break;
      }
      break;
    case 4:
      switch (((inst >> 10) & 0b11)) {
      case 0:
        op = op_c_srli;
        break;
      case 1:
        op = op_c_srai;
        break;
      case 2:
        op = op_c_andi;
        break;
      case 3:
        switch (((inst >> 10) & 0b100) | ((inst >> 5) & 0b011)) {
        case 0:
          op = op_c_sub;
          break;
        case 1:
          op = op_c_xor;
          break;
        case 2:
          op = op_c_or;
          break;
        case 3:
          op = op_c_and;
          break;
        case 4:
          op = op_c_subw;
          break;
        case 5:
          op = op_c_addw;
          break;
        }
        break;
      }
      break;
    case 5:
      op = op_c_j;
      break;
    case 6:
      op = op_c_beqz;
      break;
    case 7:
      op = op_c_bnez;
      break;
    }
    break;
  case 2: // compression instruction instLen=16bits
    switch (((inst >> 13) & 0b111)) {
    case 0:
      op = op_c_slli;
      break;
    case 1:
      op = (isa == rv128) ? op_c_lqsp : op_c_fldsp;
      break;
    case 2:
      op = op_c_lwsp;
      break;
    case 3:
      op = (isa == rv32) ? op_c_flwsp : op_c_ldsp;
      break;
    case 4:
      switch (((inst >> 12) & 0b1)) {
      case 0:
        switch (((inst >> 2) & 0b11111)) {
        case 0:
          op = op_c_jr;
          break;
        default:
          op = op_c_mv;
          break;
        }
        break;
      case 1:
        switch (((inst >> 2) & 0b11111)) {
        case 0:
          switch (((inst >> 7) & 0b11111)) {
          case 0:
            op = op_c_ebreak;
            break;
          default:
            op = op_c_jalr;
            break;
          }
          break;
        default:
          op = op_c_add;
          break;
        }
        break;
      }
      break;
    case 5:
      op = (isa == rv128) ? op_c_sqsp : op_c_fsdsp;
      break;
    case 6:
      op = op_c_swsp;
      break;
    case 7:
      op = (isa == rv32) ? op_c_fswsp : op_c_sdsp;
      break;
    }
    break;
  case 3: // uncompression instruction instLen=32bits
    switch (((inst >> 2) & 0b11111)) {
    case 0:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_lb;
        break;
      case 1:
        op = op_lh;
        break;
      case 2:
        op = op_lw;
        break;
      case 3:
        op = op_ld;
        break;
      case 4:
        op = op_lbu;
        break;
      case 5:
        op = op_lhu;
        break;
      case 6:
        op = op_lwu;
        break;
      case 7:
        op = op_ldu;
        break;
      }
      break;
    case 1:
      switch (((inst >> 12) & 0b111)) {
      case 2:
        op = op_flw;
        break;
      case 3:
        op = op_fld;
        break;
      case 4:
        op = op_flq;
        break;
      }
      break;
    case 3:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_fence;
        break;
      case 1:
        op = op_fence_i;
        break;
      case 2:
        op = op_lq;
        break;
      }
      break;
    case 4:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_addi;
        break;
      case 1:
        switch (((inst >> 27) & 0b11111)) {
        case 0:
          op = op_slli;
          break;
        }
        break;
      case 2:
        op = op_slti;
        break;
      case 3:
        op = op_sltiu;
        break;
      case 4:
        op = op_xori;
        break;
      case 5:
        switch (((inst >> 27) & 0b11111)) {
        case 0:
          op = op_srli;
          break;
        case 8:
          op = op_srai;
          break;
        }
        break;
      case 6:
        op = op_ori;
        break;
      case 7:
        op = op_andi;
        break;
      }
      break;
    case 5:
      op = op_auipc;
      break;
    case 6:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_addiw;
        break;
      case 1:
        switch (((inst >> 25) & 0b1111111)) {
        case 0:
          op = op_slliw;
          break;
        }
        break;
      case 5:
        switch (((inst >> 25) & 0b1111111)) {
        case 0:
          op = op_srliw;
          break;
        case 32:
          op = op_sraiw;
          break;
        }
        break;
      }
      break;
    case 8:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_sb;
        break;
      case 1:
        op = op_sh;
        break;
      case 2:
        op = op_sw;
        break;
      case 3:
        op = op_sd;
        break;
      case 4:
        op = op_sq;
        break;
      }
      break;
    case 9:
      switch (((inst >> 12) & 0b111)) {
      case 2:
        op = op_fsw;
        break;
      case 3:
        op = op_fsd;
        break;
      case 4:
        op = op_fsq;
        break;
      }
      break;
    case 11:
      switch (((inst >> 24) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
      case 2:
        op = op_amoadd_w;
        break;
      case 3:
        op = op_amoadd_d;
        break;
      case 4:
        op = op_amoadd_q;
        break;
      case 10:
        op = op_amoswap_w;
        break;
      case 11:
        op = op_amoswap_d;
        break;
      case 12:
        op = op_amoswap_q;
        break;
      case 18:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_lr_w;
          break;
        }
        break;
      case 19:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_lr_d;
          break;
        }
        break;
      case 20:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_lr_q;
          break;
        }
        break;
      case 26:
        op = op_sc_w;
        break;
      case 27:
        op = op_sc_d;
        break;
      case 28:
        op = op_sc_q;
        break;
      case 34:
        op = op_amoxor_w;
        break;
      case 35:
        op = op_amoxor_d;
        break;
      case 36:
        op = op_amoxor_q;
        break;
      case 66:
        op = op_amoor_w;
        break;
      case 67:
        op = op_amoor_d;
        break;
      case 68:
        op = op_amoor_q;
        break;
      case 98:
        op = op_amoand_w;
        break;
      case 99:
        op = op_amoand_d;
        break;
      case 100:
        op = op_amoand_q;
        break;
      case 130:
        op = op_amomin_w;
        break;
      case 131:
        op = op_amomin_d;
        break;
      case 132:
        op = op_amomin_q;
        break;
      case 162:
        op = op_amomax_w;
        break;
      case 163:
        op = op_amomax_d;
        break;
      case 164:
        op = op_amomax_q;
        break;
      case 194:
        op = op_amominu_w;
        break;
      case 195:
        op = op_amominu_d;
        break;
      case 196:
        op = op_amominu_q;
        break;
      case 226:
        op = op_amomaxu_w;
        break;
      case 227:
        op = op_amomaxu_d;
        break;
      case 228:
        op = op_amomaxu_q;
        break;
      }
      break;
    case 12:
      switch (((inst >> 22) & 0b1111111000) | ((inst >> 12) & 0b0000000111)) {
      case 0:
        op = op_add;
        break;
      case 1:
        op = op_sll;
        break;
      case 2:
        op = op_slt;
        break;
      case 3:
        op = op_sltu;
        break;
      case 4:
        op = op_xor;
        break;
      case 5:
        op = op_srl;
        break;
      case 6:
        op = op_or;
        break;
      case 7:
        op = op_and;
        break;
      case 8:
        op = op_mul;
        break;
      case 9:
        op = op_mulh;
        break;
      case 10:
        op = op_mulhsu;
        break;
      case 11:
        op = op_mulhu;
        break;
      case 12:
        op = op_div;
        break;
      case 13:
        op = op_divu;
        break;
      case 14:
        op = op_rem;
        break;
      case 15:
        op = op_remu;
        break;
      case 256:
        op = op_sub;
        break;
      case 261:
        op = op_sra;
        break;
      }
      break;
    case 13:
      op = op_lui;
      break;
    case 14:
      switch (((inst >> 22) & 0b1111111000) | ((inst >> 12) & 0b0000000111)) {
      case 0:
        op = op_addw;
        break;
      case 1:
        op = op_sllw;
        break;
      case 5:
        op = op_srlw;
        break;
      case 8:
        op = op_mulw;
        break;
      case 12:
        op = op_divw;
        break;
      case 13:
        op = op_divuw;
        break;
      case 14:
        op = op_remw;
        break;
      case 15:
        op = op_remuw;
        break;
      case 256:
        op = op_subw;
        break;
      case 261:
        op = op_sraw;
        break;
      }
      break;
    case 16:
      switch (((inst >> 25) & 0b11)) {
      case 0:
        op = op_fmadd_s;
        break;
      case 1:
        op = op_fmadd_d;
        break;
      case 3:
        op = op_fmadd_q;
        break;
      }
      break;
    case 17:
      switch (((inst >> 25) & 0b11)) {
      case 0:
        op = op_fmsub_s;
        break;
      case 1:
        op = op_fmsub_d;
        break;
      case 3:
        op = op_fmsub_q;
        break;
      }
      break;
    case 18:
      switch (((inst >> 25) & 0b11)) {
      case 0:
        op = op_fnmsub_s;
        break;
      case 1:
        op = op_fnmsub_d;
        break;
      case 3:
        op = op_fnmsub_q;
        break;
      }
      break;
    case 19:
      switch (((inst >> 25) & 0b11)) {
      case 0:
        op = op_fnmadd_s;
        break;
      case 1:
        op = op_fnmadd_d;
        break;
      case 3:
        op = op_fnmadd_q;
        break;
      }
      break;
    case 20:
      switch (((inst >> 25) & 0b1111111)) {
      case 0:
        op = op_fadd_s;
        break;
      case 1:
        op = op_fadd_d;
        break;
      case 3:
        op = op_fadd_q;
        break;
      case 4:
        op = op_fsub_s;
        break;
      case 5:
        op = op_fsub_d;
        break;
      case 7:
        op = op_fsub_q;
        break;
      case 8:
        op = op_fmul_s;
        break;
      case 9:
        op = op_fmul_d;
        break;
      case 11:
        op = op_fmul_q;
        break;
      case 12:
        op = op_fdiv_s;
        break;
      case 13:
        op = op_fdiv_d;
        break;
      case 15:
        op = op_fdiv_q;
        break;
      case 16:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fsgnj_s;
          break;
        case 1:
          op = op_fsgnjn_s;
          break;
        case 2:
          op = op_fsgnjx_s;
          break;
        }
        break;
      case 17:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fsgnj_d;
          break;
        case 1:
          op = op_fsgnjn_d;
          break;
        case 2:
          op = op_fsgnjx_d;
          break;
        }
        break;
      case 19:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fsgnj_q;
          break;
        case 1:
          op = op_fsgnjn_q;
          break;
        case 2:
          op = op_fsgnjx_q;
          break;
        }
        break;
      case 20:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fmin_s;
          break;
        case 1:
          op = op_fmax_s;
          break;
        }
        break;
      case 21:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fmin_d;
          break;
        case 1:
          op = op_fmax_d;
          break;
        }
        break;
      case 23:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fmin_q;
          break;
        case 1:
          op = op_fmax_q;
          break;
        }
        break;
      case 32:
        switch (((inst >> 20) & 0b11111)) {
        case 1:
          op = op_fcvt_s_d;
          break;
        case 3:
          op = op_fcvt_s_q;
          break;
        }
        break;
      case 33:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_d_s;
          break;
        case 3:
          op = op_fcvt_d_q;
          break;
        }
        break;
      case 35:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_q_s;
          break;
        case 1:
          op = op_fcvt_q_d;
          break;
        }
        break;
      case 44:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fsqrt_s;
          break;
        }
        break;
      case 45:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fsqrt_d;
          break;
        }
        break;
      case 47:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fsqrt_q;
          break;
        }
        break;
      case 80:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fle_s;
          break;
        case 1:
          op = op_flt_s;
          break;
        case 2:
          op = op_feq_s;
          break;
        }
        break;
      case 81:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fle_d;
          break;
        case 1:
          op = op_flt_d;
          break;
        case 2:
          op = op_feq_d;
          break;
        }
        break;
      case 83:
        switch (((inst >> 12) & 0b111)) {
        case 0:
          op = op_fle_q;
          break;
        case 1:
          op = op_flt_q;
          break;
        case 2:
          op = op_feq_q;
          break;
        }
        break;
      case 96:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_w_s;
          break;
        case 1:
          op = op_fcvt_wu_s;
          break;
        case 2:
          op = op_fcvt_l_s;
          break;
        case 3:
          op = op_fcvt_lu_s;
          break;
        }
        break;
      case 97:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_w_d;
          break;
        case 1:
          op = op_fcvt_wu_d;
          break;
        case 2:
          op = op_fcvt_l_d;
          break;
        case 3:
          op = op_fcvt_lu_d;
          break;
        }
        break;
      case 99:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_w_q;
          break;
        case 1:
          op = op_fcvt_wu_q;
          break;
        case 2:
          op = op_fcvt_l_q;
          break;
        case 3:
          op = op_fcvt_lu_q;
          break;
        }
        break;
      case 104:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_s_w;
          break;
        case 1:
          op = op_fcvt_s_wu;
          break;
        case 2:
          op = op_fcvt_s_l;
          break;
        case 3:
          op = op_fcvt_s_lu;
          break;
        }
        break;
      case 105:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_d_w;
          break;
        case 1:
          op = op_fcvt_d_wu;
          break;
        case 2:
          op = op_fcvt_d_l;
          break;
        case 3:
          op = op_fcvt_d_lu;
          break;
        }
        break;
      case 107:
        switch (((inst >> 20) & 0b11111)) {
        case 0:
          op = op_fcvt_q_w;
          break;
        case 1:
          op = op_fcvt_q_wu;
          break;
        case 2:
          op = op_fcvt_q_l;
          break;
        case 3:
          op = op_fcvt_q_lu;
          break;
        }
        break;
      case 112:
        switch (((inst >> 17) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
        case 0:
          op = op_fmv_x_s;
          break;
        case 1:
          op = op_fclass_s;
          break;
        }
        break;
      case 113:
        switch (((inst >> 17) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
        case 0:
          op = op_fmv_x_d;
          break;
        case 1:
          op = op_fclass_d;
          break;
        }
        break;
      case 115:
        switch (((inst >> 17) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
        case 0:
          op = op_fmv_x_q;
          break;
        case 1:
          op = op_fclass_q;
          break;
        }
        break;
      case 120:
        switch (((inst >> 17) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
        case 0:
          op = op_fmv_s_x;
          break;
        }
        break;
      case 121:
        switch (((inst >> 17) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
        case 0:
          op = op_fmv_d_x;
          break;
        }
        break;
      case 123:
        switch (((inst >> 17) & 0b11111000) | ((inst >> 12) & 0b00000111)) {
        case 0:
          op = op_fmv_q_x;
          break;
        }
        break;
      }
      break;
    case 22:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_addid;
        break;
      case 1:
        switch (((inst >> 26) & 0b111111)) {
        case 0:
          op = op_sllid;
          break;
        }
        break;
      case 5:
        switch (((inst >> 26) & 0b111111)) {
        case 0:
          op = op_srlid;
          break;
        case 16:
          op = op_sraid;
          break;
        }
        break;
      }
      break;
    case 24:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_beq;
        break;
      case 1:
        op = op_bne;
        break;
      case 4:
        op = op_blt;
        break;
      case 5:
        op = op_bge;
        break;
      case 6:
        op = op_bltu;
        break;
      case 7:
        op = op_bgeu;
        break;
      }
      break;
    case 25:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        op = op_jalr;
        break;
      }
      break;
    case 27:
      op = op_jal;
      break;
    case 28:
      switch (((inst >> 12) & 0b111)) {
      case 0:
        switch (((inst >> 20) & 0b111111100000) |
                ((inst >> 7) & 0b000000011111)) {
        case 0:
          switch (((inst >> 15) & 0b1111111111)) {
          case 0:
            op = op_ecall;
            break;
          case 32:
            op = op_ebreak;
            break;
          case 64:
            op = op_uret;
            break;
          }
          break;
        case 256:
          switch (((inst >> 20) & 0b11111)) {
          case 2:
            switch (((inst >> 15) & 0b11111)) {
            case 0:
              op = op_sret;
              break;
            }
            break;
          case 4:
            op = op_sfence_vm;
            break;
          case 5:
            switch (((inst >> 15) & 0b11111)) {
            case 0:
              op = op_wfi;
              break;
            }
            break;
          }
          break;
        case 288:
          op = op_sfence_vma;
          break;
        case 512:
          switch (((inst >> 15) & 0b1111111111)) {
          case 64:
            op = op_hret;
            break;
          }
          break;
        case 768:
          switch (((inst >> 15) & 0b1111111111)) {
          case 64:
            op = op_mret;
            break;
          }
          break;
        case 1952:
          switch (((inst >> 15) & 0b1111111111)) {
          case 576:
            op = op_dret;
            break;
          }
          break;
        }
        break;
      case 1:
        op = op_csrrw;
        break;
      case 2:
        op = op_csrrs;
        break;
      case 3:
        op = op_csrrc;
        break;
      case 5:
        op = op_csrrwi;
        break;
      case 6:
        op = op_csrrsi;
        break;
      case 7:
        op = op_csrrci;
        break;
      }
      break;
    case 30:
      switch (((inst >> 22) & 0b1111111000) | ((inst >> 12) & 0b0000000111)) {
      case 0:
        op = op_addd;
        break;
      case 1:
        op = op_slld;
        break;
      case 5:
        op = op_srld;
        break;
      case 8:
        op = op_muld;
        break;
      case 12:
        op = op_divd;
        break;
      case 13:
        op = op_divud;
        break;
      case 14:
        op = op_remd;
        break;
      case 15:
        op = op_remud;
        break;
      case 256:
        op = op_subd;
        break;
      case 261:
        op = op_srad;
        break;
      }
      break;
    }
    break;
  }
  instInfo->op = op;
}

/* operand extractors */

static uint32_t operand_rd(rv_inst inst) { return (inst << 52) >> 59; }

static uint32_t operand_rs1(rv_inst inst) { return (inst << 44) >> 59; }

static uint32_t operand_rs2(rv_inst inst) { return (inst << 39) >> 59; }

static uint32_t operand_rs3(rv_inst inst) { return (inst << 32) >> 59; }

static uint32_t operand_aq(rv_inst inst) { return (inst << 37) >> 63; }

static uint32_t operand_rl(rv_inst inst) { return (inst << 38) >> 63; }

static uint32_t operand_pred(rv_inst inst) { return (inst << 36) >> 60; }

static uint32_t operand_succ(rv_inst inst) { return (inst << 40) >> 60; }

static uint32_t operand_rm(rv_inst inst) { return (inst << 49) >> 61; }

static uint32_t operand_shamt5(rv_inst inst) { return (inst << 39) >> 59; }

static uint32_t operand_shamt6(rv_inst inst) { return (inst << 38) >> 58; }

static uint32_t operand_shamt7(rv_inst inst) { return (inst << 37) >> 57; }

static uint32_t operand_crdq(rv_inst inst) { return (inst << 59) >> 61; }

static uint32_t operand_crs1q(rv_inst inst) { return (inst << 54) >> 61; }

static uint32_t operand_crs1rdq(rv_inst inst) { return (inst << 54) >> 61; }

static uint32_t operand_crs2q(rv_inst inst) { return (inst << 59) >> 61; }

static uint32_t operand_crd(rv_inst inst) { return (inst << 52) >> 59; }

static uint32_t operand_crs1(rv_inst inst) { return (inst << 52) >> 59; }

static uint32_t operand_crs1rd(rv_inst inst) { return (inst << 52) >> 59; }

static uint32_t operand_crs2(rv_inst inst) { return (inst << 57) >> 59; }

static uint32_t operand_cimmsh5(rv_inst inst) { return (inst << 57) >> 59; }

static uint32_t operand_csr12(rv_inst inst) { return (inst << 32) >> 52; }

static int32_t operand_imm12(rv_inst inst) {
  return ((int64_t)inst << 32) >> 52;
}

static int32_t operand_imm20(rv_inst inst) {
  return (((int64_t)inst << 32) >> 44) << 12;
}

static int32_t operand_jimm20(rv_inst inst) {
  return (((int64_t)inst << 32) >> 63) << 20 | ((inst << 33) >> 54) << 1 |
         ((inst << 43) >> 63) << 11 | ((inst << 44) >> 56) << 12;
}

static int32_t operand_simm12(rv_inst inst) {
  return (((int64_t)inst << 32) >> 57) << 5 | (inst << 52) >> 59;
}

static int32_t operand_sbimm12(rv_inst inst) {
  return (((int64_t)inst << 32) >> 63) << 12 | ((inst << 33) >> 58) << 5 |
         ((inst << 52) >> 60) << 1 | ((inst << 56) >> 63) << 11;
}

static uint32_t operand_cimmsh6(rv_inst inst) {
  return ((inst << 51) >> 63) << 5 | (inst << 57) >> 59;
}

static int32_t operand_cimmi(rv_inst inst) {
  return (((int64_t)inst << 51) >> 63) << 5 | (inst << 57) >> 59;
}

static int32_t operand_cimmui(rv_inst inst) {
  return (((int64_t)inst << 51) >> 63) << 17 | ((inst << 57) >> 59) << 12;
}

static uint32_t operand_cimmlwsp(rv_inst inst) {
  return ((inst << 51) >> 63) << 5 | ((inst << 57) >> 61) << 2 |
         ((inst << 60) >> 62) << 6;
}

static uint32_t operand_cimmldsp(rv_inst inst) {
  return ((inst << 51) >> 63) << 5 | ((inst << 57) >> 62) << 3 |
         ((inst << 59) >> 61) << 6;
}

static uint32_t operand_cimmlqsp(rv_inst inst) {
  return ((inst << 51) >> 63) << 5 | ((inst << 57) >> 63) << 4 |
         ((inst << 58) >> 60) << 6;
}

static int32_t operand_cimm16sp(rv_inst inst) {
  return (((int64_t)inst << 51) >> 63) << 9 | ((inst << 57) >> 63) << 4 |
         ((inst << 58) >> 63) << 6 | ((inst << 59) >> 62) << 7 |
         ((inst << 61) >> 63) << 5;
}

static int32_t operand_cimmj(rv_inst inst) {
  return (((int64_t)inst << 51) >> 63) << 11 | ((inst << 52) >> 63) << 4 |
         ((inst << 53) >> 62) << 8 | ((inst << 55) >> 63) << 10 |
         ((inst << 56) >> 63) << 6 | ((inst << 57) >> 63) << 7 |
         ((inst << 58) >> 61) << 1 | ((inst << 61) >> 63) << 5;
}

static int32_t operand_cimmb(rv_inst inst) {
  return (((int64_t)inst << 51) >> 63) << 8 | ((inst << 52) >> 62) << 3 |
         ((inst << 57) >> 62) << 6 | ((inst << 59) >> 62) << 1 |
         ((inst << 61) >> 63) << 5;
}

static uint32_t operand_cimmswsp(rv_inst inst) {
  return ((inst << 51) >> 60) << 2 | ((inst << 55) >> 62) << 6;
}

static uint32_t operand_cimmsdsp(rv_inst inst) {
  return ((inst << 51) >> 61) << 3 | ((inst << 54) >> 61) << 6;
}

static uint32_t operand_cimmsqsp(rv_inst inst) {
  return ((inst << 51) >> 62) << 4 | ((inst << 53) >> 60) << 6;
}

static uint32_t operand_cimm4spn(rv_inst inst) {
  return ((inst << 51) >> 62) << 4 | ((inst << 53) >> 60) << 6 |
         ((inst << 57) >> 63) << 2 | ((inst << 58) >> 63) << 3;
}

static uint32_t operand_cimmw(rv_inst inst) {
  return ((inst << 51) >> 61) << 3 | ((inst << 57) >> 63) << 2 |
         ((inst << 58) >> 63) << 6;
}

static uint32_t operand_cimmd(rv_inst inst) {
  return ((inst << 51) >> 61) << 3 | ((inst << 57) >> 62) << 6;
}

static uint32_t operand_cimmq(rv_inst inst) {
  return ((inst << 51) >> 62) << 4 | ((inst << 53) >> 63) << 8 |
         ((inst << 57) >> 62) << 6;
}

/* decode operands */

static void rv_inst_oprands(rv_instInfo *instInfo) {
  rv_inst inst = instInfo->inst;
  instInfo->encodingType = opcode_data[instInfo->op].encodingType;
  switch (instInfo->encodingType) {
  case type_none:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = 0;
    break;
  case type_u:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_imm20(inst);
    break;
  case type_uj:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_jimm20(inst);
    break;
  case type_i:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_imm12(inst);
    break;
  case type_i_sh5:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_shamt5(inst);
    break;
  case type_i_sh6:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_shamt6(inst);
    break;
  case type_i_sh7:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_shamt7(inst);
    break;
  case type_i_csr:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_csr12(inst);
    break;
  case type_s:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = operand_rs2(inst);
    instInfo->imm = operand_simm12(inst);
    break;
  case type_sb:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = operand_rs2(inst);
    instInfo->imm = operand_sbimm12(inst);
    break;
  case type_r:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = operand_rs2(inst);
    instInfo->imm = 0;
    break;
  case type_r_m:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = operand_rs2(inst);
    instInfo->imm = 0;
    instInfo->rm = operand_rm(inst);
    break;
  case type_r4_m:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = operand_rs2(inst);
    instInfo->rs3 = operand_rs3(inst);
    instInfo->imm = 0;
    instInfo->rm = operand_rm(inst);
    break;
  case type_r_a:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = operand_rs2(inst);
    instInfo->imm = 0;
    instInfo->aq = operand_aq(inst);
    instInfo->rl = operand_rl(inst);
    break;
  case type_r_l:
    instInfo->rd = operand_rd(inst);
    instInfo->rs1 = operand_rs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = 0;
    instInfo->aq = operand_aq(inst);
    instInfo->rl = operand_rl(inst);
    break;
  case type_r_f:
    instInfo->rd = instInfo->rs1 = instInfo->rs2 = reg_x00_f00;
    instInfo->pred = operand_pred(inst);
    instInfo->succ = operand_succ(inst);
    instInfo->imm = 0;
    break;
  case type_cb:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmb(inst);
    break;
  case type_cb_imm:
    instInfo->rd = instInfo->rs1 = operand_crs1rdq(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmi(inst);
    break;
  case type_cb_sh5:
    instInfo->rd = instInfo->rs1 = operand_crs1rdq(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmsh5(inst);
    break;
  case type_cb_sh6:
    instInfo->rd = instInfo->rs1 = operand_crs1rdq(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmsh6(inst);
    break;
  case type_ci:
    instInfo->rd = instInfo->rs1 = operand_crs1rd(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmi(inst);
    break;
  case type_ci_sh5:
    instInfo->rd = instInfo->rs1 = operand_crs1rd(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmsh5(inst);
    break;
  case type_ci_sh6:
    instInfo->rd = instInfo->rs1 = operand_crs1rd(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmsh6(inst);
    break;
  case type_ci_16sp:
    instInfo->rd = reg_x02_f02;
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimm16sp(inst);
    break;
  case type_ci_lwsp:
    instInfo->rd = operand_crd(inst);
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmlwsp(inst);
    break;
  case type_ci_ldsp:
    instInfo->rd = operand_crd(inst);
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmldsp(inst);
    break;
  case type_ci_lqsp:
    instInfo->rd = operand_crd(inst);
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmlqsp(inst);
    break;
  case type_ci_li:
    instInfo->rd = operand_crd(inst);
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmi(inst);
    break;
  case type_ci_lui:
    instInfo->rd = operand_crd(inst);
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmui(inst);
    break;
  case type_ci_none:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = 0;
    break;
  case type_ciw_4spn:
    instInfo->rd = operand_crdq(inst) + 8;
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimm4spn(inst);
    break;
  case type_cj:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmj(inst);
    break;
  case type_cj_jal:
    instInfo->rd = reg_x01_f01;
    instInfo->rs1 = reg_x00_f00;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmj(inst);
    break;
  case type_cl_lw:
    instInfo->rd = operand_crdq(inst) + 8;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmw(inst);
    break;
  case type_cl_ld:
    instInfo->rd = operand_crdq(inst) + 8;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmd(inst);
    break;
  case type_cl_lq:
    instInfo->rd = operand_crdq(inst) + 8;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = operand_cimmq(inst);
    break;
  case type_cr:
    instInfo->rd = instInfo->rs1 = operand_crs1rd(inst);
    instInfo->rs2 = operand_crs2(inst);
    instInfo->imm = 0;
    break;
  case type_cr_mv:
    instInfo->rd = operand_crd(inst);
    instInfo->rs1 = operand_crs2(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = 0;
    break;
  case type_cr_jalr:
    instInfo->rd = reg_x01_f01;
    instInfo->rs1 = operand_crs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = 0;
    break;
  case type_cr_jr:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_crs1(inst);
    instInfo->rs2 = reg_x00_f00;
    instInfo->imm = 0;
    break;
  case type_cs:
    instInfo->rd = instInfo->rs1 = operand_crs1rdq(inst) + 8;
    instInfo->rs2 = operand_crs2q(inst) + 8;
    instInfo->imm = 0;
    break;
  case type_cs_sw:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = operand_crs2q(inst) + 8;
    instInfo->imm = operand_cimmw(inst);
    break;
  case type_cs_sd:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = operand_crs2q(inst) + 8;
    instInfo->imm = operand_cimmd(inst);
    break;
  case type_cs_sq:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = operand_crs1q(inst) + 8;
    instInfo->rs2 = operand_crs2q(inst) + 8;
    instInfo->imm = operand_cimmq(inst);
    break;
  case type_css_swsp:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = operand_crs2(inst);
    instInfo->imm = operand_cimmswsp(inst);
    break;
  case type_css_sdsp:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = operand_crs2(inst);
    instInfo->imm = operand_cimmsdsp(inst);
    break;
  case type_css_sqsp:
    instInfo->rd = reg_x00_f00;
    instInfo->rs1 = reg_x02_f02;
    instInfo->rs2 = operand_crs2(inst);
    instInfo->imm = operand_cimmsqsp(inst);
    break;
  };
}

/* decompress instruction */

static void rv_inst_decompress(rv_instInfo *instInfo, rv_isa isa) {
  int decomp_op;
  switch (isa) {
  case rv32:
    decomp_op = opcode_data[instInfo->op].decomp_rv32;
    break;
  case rv64:
    decomp_op = opcode_data[instInfo->op].decomp_rv64;
    break;
  case rv128:
    decomp_op = opcode_data[instInfo->op].decomp_rv128;
    break;
  }
  if (decomp_op != op_illegal) {
    if ((opcode_data[instInfo->op].decomp_data & rvcd_imm_nz) &&
        instInfo->imm == 0) {
      instInfo->op = op_illegal;
    } else {
      instInfo->op = decomp_op;
      instInfo->encodingType = opcode_data[decomp_op].encodingType;
    }
  }
}

/* check constraint */

static bool check_constraints(rv_instInfo *instInfo, const rvc_constraint *c) {
  int32_t imm = instInfo->imm;
  uint8_t rd = instInfo->rd;
  uint8_t rs1 = instInfo->rs1;
  uint8_t rs2 = instInfo->rs2;

  while (*c != rvc_end) {
    switch (*c) {
    case rvc_rd_eq_ra:
      if (!(rd == 1))
        return false;
      break;
    case rvc_rd_eq_x0:
      if (!(rd == 0))
        return false;
      break;
    case rvc_rs1_eq_x0:
      if (!(rs1 == 0))
        return false;
      break;
    case rvc_rs2_eq_x0:
      if (!(rs2 == 0))
        return false;
      break;
    case rvc_rs2_eq_rs1:
      if (!(rs2 == rs1))
        return false;
      break;
    case rvc_rs1_eq_ra:
      if (!(rs1 == 1))
        return false;
      break;
    case rvc_imm_eq_zero:
      if (!(imm == 0))
        return false;
      break;
    case rvc_imm_eq_n1:
      if (!(imm == -1))
        return false;
      break;
    case rvc_imm_eq_p1:
      if (!(imm == 1))
        return false;
      break;
    case rvc_csr_eq_0x001:
      if (!(imm == 0x001))
        return false;
      break;
    case rvc_csr_eq_0x002:
      if (!(imm == 0x002))
        return false;
      break;
    case rvc_csr_eq_0x003:
      if (!(imm == 0x003))
        return false;
      break;
    case rvc_csr_eq_0xc00:
      if (!(imm == 0xc00))
        return false;
      break;
    case rvc_csr_eq_0xc01:
      if (!(imm == 0xc01))
        return false;
      break;
    case rvc_csr_eq_0xc02:
      if (!(imm == 0xc02))
        return false;
      break;
    case rvc_csr_eq_0xc80:
      if (!(imm == 0xc80))
        return false;
      break;
    case rvc_csr_eq_0xc81:
      if (!(imm == 0xc81))
        return false;
      break;
    case rvc_csr_eq_0xc82:
      if (!(imm == 0xc82))
        return false;
      break;
    default:
      break;
    }
    c++;
  }
  return true;
}

/* lift instruction to pseudo-instruction */

static void rv_inst_pseudo(rv_instInfo *instInfo) {
  const rv_compData *comp_data = opcode_data[instInfo->op].pseudo;
  if (!comp_data) {
    return;
  }
  while (comp_data->constraints) {
    if (check_constraints(instInfo, comp_data->constraints)) {
      instInfo->op = comp_data->op;
      instInfo->encodingType = opcode_data[instInfo->op].encodingType;
      return;
    }
    comp_data++;
  }
}

/* format instruction */

static void append(char *s1, const char *s2, size_t n) {
  /* The strlen function returns the number of characters that precede the
   * terminating null character. */
  size_t s1Len = strlen(s1);
  size_t s2Len = strlen(s2);
  if (n - s1Len - s2Len - 1 >= 0) {
    strncat(s1, s2, n - s1Len);
  } else
    fprintf(stderr, "buffer overflow");
}

#define INST_LEN_2 ("%04h" PRIx16 "              ")
#define INST_LEN_4 ("%08" PRIx32 "          ")
#define INST_LEN_6 ("%012" PRIx64 "      ")
#define INST_LEN_8 ("%016" PRIx64 "  ")

static void rv_inst_format(char *buf, size_t buflen, size_t tab,
                           rv_instInfo *instInfo) {
  char tmp[64];
  const char *fmt;

  size_t len = inst_length(instInfo->inst);
  switch (len) {
  case 2:
    snprintf(buf, buflen, INST_LEN_2, (int16_t)instInfo->inst);
    break;
  case 4:
    snprintf(buf, buflen, INST_LEN_4, (int32_t)instInfo->inst);
    break;
  case 6:
    snprintf(buf, buflen, INST_LEN_6, instInfo->inst);
    break;
  default:
    snprintf(buf, buflen, INST_LEN_8, (int64_t)instInfo->inst);
    break;
  }

  fmt = opcode_data[instInfo->op].format;
  while (*fmt) {
    switch (*fmt) {
    case 'O':
      append(buf, opcode_data[instInfo->op].name, buflen);
      break;
    case '(':
      append(buf, "(", buflen);
      break;
    case ',':
      append(buf, ",", buflen);
      break;
    case ')':
      append(buf, ")", buflen);
      break;
    case '0':
      append(buf, rv_i_reg_abi_name[instInfo->rd], buflen);
      break;
    case '1':
      append(buf, rv_i_reg_abi_name[instInfo->rs1], buflen);
      break;
    case '2':
      append(buf, rv_i_reg_abi_name[instInfo->rs2], buflen);
      break;
    case '3':
      append(buf, rv_f_reg_abi_name[instInfo->rd], buflen);
      break;
    case '4':
      append(buf, rv_f_reg_abi_name[instInfo->rs1], buflen);
      break;
    case '5':
      append(buf, rv_f_reg_abi_name[instInfo->rs2], buflen);
      break;
    case '6':
      append(buf, rv_f_reg_abi_name[instInfo->rs3], buflen);
      break;
    case '7':
      snprintf(tmp, sizeof(tmp), "%d", instInfo->rs1);
      append(buf, tmp, buflen);
      break;
    case 'i':
      snprintf(tmp, sizeof(tmp), "%d", instInfo->imm);
      append(buf, tmp, buflen);
      break;
    case 'o':
      snprintf(tmp, sizeof(tmp), "%d", instInfo->imm);
      append(buf, tmp, buflen);
      while (strlen(buf) < tab * 2) {
        append(buf, " ", buflen);
      }
      snprintf(tmp, sizeof(tmp), "# 0x%" PRIx64, instInfo->pc + instInfo->imm);
      append(buf, tmp, buflen);
      break;
    case 'c': {
      const char *name = csr_name(instInfo->imm & 0xfff);
      if (name) {
        append(buf, name, buflen);
      } else {
        snprintf(tmp, sizeof(tmp), "0x%03x", instInfo->imm & 0xfff);
        append(buf, tmp, buflen);
      }
      break;
    }
    case 'r':
      switch (instInfo->rm) {
      case rm_rne:
        append(buf, "rne", buflen);
        break;
      case rm_rtz:
        append(buf, "rtz", buflen);
        break;
      case rm_rdn:
        append(buf, "rdn", buflen);
        break;
      case rm_rup:
        append(buf, "rup", buflen);
        break;
      case rm_rmm:
        append(buf, "rmm", buflen);
        break;
      case rm_dyn:
        append(buf, "dyn", buflen);
        break;
      default:
        append(buf, "inv", buflen);
        break;
      }
      break;
    case 'p':
      if (instInfo->pred & fence_i) {
        append(buf, "i", buflen);
      }
      if (instInfo->pred & fence_o) {
        append(buf, "o", buflen);
      }
      if (instInfo->pred & fence_r) {
        append(buf, "r", buflen);
      }
      if (instInfo->pred & fence_w) {
        append(buf, "w", buflen);
      }
      break;
    case 's':
      if (instInfo->succ & fence_i) {
        append(buf, "i", buflen);
      }
      if (instInfo->succ & fence_o) {
        append(buf, "o", buflen);
      }
      if (instInfo->succ & fence_r) {
        append(buf, "r", buflen);
      }
      if (instInfo->succ & fence_w) {
        append(buf, "w", buflen);
      }
      break;
    case '\t':
      while (strlen(buf) < tab) {
        append(buf, " ", buflen);
      }
      break;
    case 'A':
      if (instInfo->aq) {
        append(buf, ".aq", buflen);
      }
      break;
    case 'R':
      if (instInfo->rl) {
        append(buf, ".rl", buflen);
      }
      break;
    default:
      break;
    }
    fmt++;
  }
}

/* instruction fetch */

void inst_fetch(const uint8_t *data, rv_inst *instp, size_t *length) {
  rv_inst inst = ((rv_inst)data[1] << 8) | ((rv_inst)data[0]);
  size_t len = *length = inst_length(inst);
  if (len >= 8)
    inst |= ((rv_inst)data[7] << 56) | ((rv_inst)data[6] << 48);
  if (len >= 6)
    inst |= ((rv_inst)data[5] << 40) | ((rv_inst)data[4] << 32);
  if (len >= 4)
    inst |= ((rv_inst)data[3] << 24) | ((rv_inst)data[2] << 16);
  *instp = inst;
}

/* disassemble instruction */

static void disassemble_inst(uint8_t *buf, size_t buflen, rv_isa isa,
                             rv_inst inst, uint64_t pc) {
  rv_instInfo dec = {.pc = pc, .inst = inst};
  rv_inst_opcode(&dec, isa);
  rv_inst_oprands(&dec);
  // Whether to display compression instructions?
  // rv_inst_decompress(&dec, isa);
  rv_inst_pseudo(&dec);
  rv_inst_format(buf, buflen, 32, &dec);
  fprintf(stderr, "%04" PRIx64 ":  %s\n", pc, buf);
}

int disassemble_text_section(const uint8_t *inputFileName) {
  int32_t inst;

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
    uint8_t buf[80] = {0};

    inst = byte_get_little_endian(instBuffer, uncompressionInstLen);

    if (inst_length(inst) == 4) { // uncompression instruction length is 4
      disassemble_inst(buf, sizeof(buf), riscvLen == 64 ? rv64 : rv32, inst,
                       shdrTextOff);
      instBuffer += uncompressionInstLen;
      shdrTextOff += uncompressionInstLen;
    } else if (inst_length(inst) == 2) { // compression instruction length is 2
      inst = byte_get_little_endian(instBuffer, compressionInstLen);
      disassemble_inst(buf, sizeof(buf), riscvLen == 64 ? rv64 : rv32, inst,
                       shdrTextOff);
      instBuffer += compressionInstLen;
      shdrTextOff += compressionInstLen;
    } else { // Other instruction lengths are not supported temporarily
      fprintf(stderr, "instruction length is error!\n\n");
    }
  }

  close_file(fileHandle);

  fprintf(stderr, "\n\n");
  return 0;
}
