/***************************************************************************************
 * Copyright (c) 2014-2024 Zihao Yu, Nanjing University
 *
 * NEMU is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 *
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 *
 * See the Mulan PSL v2 for more details.
 ***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write
#define Jal jal_excute
#define Jalr jalr_excute
#define Bins bins_excute
#define slts slts_excute

void insert_ftrace(int type, vaddr_t insAddr, vaddr_t target);
void difftest_skip_ref();
void jal_excute(word_t *dnpc, word_t pc, int rd, word_t imm)
{
  *dnpc = pc + imm;
  R(rd) = pc + 4;

#ifdef CONFIG_FTRACE
  // call function as long as the rd is x1
  if (rd == 1)
    insert_ftrace(0, pc, *dnpc);
#endif
}

void jalr_excute(word_t *dnpc, word_t pc, int rd, word_t src1, word_t imm, int rs)
{
  *dnpc = (src1 + imm) & ~1;
  R(rd) = pc + 4;

#ifdef CONFIG_FTRACE
  if (rd == 1)
    insert_ftrace(0, pc, *dnpc);
  else if (rd == 0 && imm == 0 && rs == 1)
    insert_ftrace(1, pc, *dnpc); // ret is jalr x0,0(x1)
  else if (rd == 0 && imm == 0)
    insert_ftrace(2, pc, *dnpc); // tail call
#endif
}

void bins_excute(word_t *dnpc, word_t pc, word_t src1, word_t src2, word_t imm, int type)
{
  word_t branch = pc + imm;
  switch (type)
  {
  case 0:
    if (src1 == src2)
      *dnpc = branch;
    break;
  case 1:
    if (src1 != src2)
      *dnpc = branch;
    break;
  case 2:
    if ((int64_t)src1 < (int64_t)src2)
      *dnpc = branch;
    break;
  case 3:
    if ((int64_t)src1 >= (int64_t)src2)
      *dnpc = branch;
    break;
  case 4:
    if (src1 < src2)
      *dnpc = branch;
    break;
  case 5:
    if (src1 >= src2)
      *dnpc = branch;
    break;
  default:
    break;
  }
}

void slts_excute(word_t src1, word_t src2, int rd, int type)
{
  R(rd) = 0;
  switch (type)
  {
  case 0:
    if ((int64_t)src1 < (int64_t)src2)
      R(rd) = 1;
    break;
  case 1:
    if (src1 < src2)
      R(rd) = 1;
    break;
  default:
    break;
  }
}

void csrrw_excute(word_t src1, word_t imm, int rd)
{
  switch (imm)
  {
  case 0x300:
    R(rd) = cpu.csrs.mstatus;
    cpu.csrs.mstatus = src1;
    break;
  case 0x305:
    R(rd) = cpu.csrs.mtvec;
    cpu.csrs.mtvec = src1;
    break;
  case 0x340:
    R(rd) = cpu.csrs.mscratch;
    cpu.csrs.mscratch = src1;
    break;
  case 0x341:
    R(rd) = cpu.csrs.mepc;
    cpu.csrs.mepc = src1;
    break;
  case 0x342:
    R(rd) = cpu.csrs.mcause;
    cpu.csrs.mcause = src1;
    break;
  case 0x180:
    R(rd) = cpu.csrs.satp;
    cpu.csrs.satp = src1;
    break;
  default:
    panic("%lx The %lx csr not implemented", cpu.pc, imm);
    break;
  }
}
void csrrs_excute(word_t src1, word_t imm, int rd)
{
  switch (imm)
  {
  case 0x300:
    R(rd) = cpu.csrs.mstatus;
    cpu.csrs.mstatus |= src1;
    break;
  case 0x305:
    R(rd) = cpu.csrs.mtvec;
    cpu.csrs.mtvec |= src1;
    break;
  case 0x340:
    R(rd) = cpu.csrs.mscratch;
    cpu.csrs.mscratch |= src1;
    break;
  case 0x341:
    R(rd) = cpu.csrs.mepc;
    cpu.csrs.mepc |= src1;
    break;
  case 0x342:
    R(rd) = cpu.csrs.mcause;
    cpu.csrs.mcause |= src1;
    break;
  case 0x180:
    R(rd) = cpu.csrs.satp;
    cpu.csrs.satp |= src1;
    break;
  default:
    panic("The %lx csr not implemented", imm);
    break;
  }
}

void mret_excute(vaddr_t *dnpc)
{
  *dnpc = cpu.csrs.mepc;
  word_t pmie = cpu.csrs.mstatus & MSTATUS_MPIE_BITS;
  cpu.csrs.mstatus = (cpu.csrs.mstatus & ~MSTATUS_MIE_BITS)|(pmie >> 4);
  cpu.csrs.mstatus |= MSTATUS_MPIE_BITS;
  IFDEF(CONFIG_DIFFTEST, difftest_skip_ref());
}

// 用spike的乘法实现
word_t mulhuALU(word_t a, word_t b)
{
  uint64_t t;
  uint32_t y1, y2, y3;
  uint64_t a0 = (uint32_t)a, a1 = a >> 32;
  uint64_t b0 = (uint32_t)b, b1 = b >> 32;

  t = a1 * b0 + ((a0 * b0) >> 32);
  y1 = t;
  y2 = t >> 32;

  t = a0 * b1 + y1;

  t = a1 * b1 + y2 + (t >> 32);
  y2 = t;
  y3 = t >> 32;

  return ((word_t)y3 << 32) | y2;
}

int64_t mulhALU(int64_t a, int64_t b)
{
  int negate = (a < 0) != (b < 0);
  uint64_t res = mulhuALU(a < 0 ? -a : a, b < 0 ? -b : b);
  return negate ? ~res + (a * b == 0) : res;
}

int64_t mulhsuALU(int64_t a, uint64_t b)
{
  int negate = a < 0;
  uint64_t res = mulhuALU(a < 0 ? -a : a, b);
  return negate ? ~res + (a * b == 0) : res;
}
enum
{
  TYPE_R,
  TYPE_I,
  TYPE_U,
  TYPE_S,
  TYPE_J,
  TYPE_B,
  TYPE_N, // none
};

#define src1R()     \
  do                \
  {                 \
    *src1 = R(rs1); \
  } while (0)
#define src2R()     \
  do                \
  {                 \
    *src2 = R(rs2); \
  } while (0)
#define immI()                        \
  do                                  \
  {                                   \
    *imm = SEXT(BITS(i, 31, 20), 12); \
  } while (0)
#define immU()                              \
  do                                        \
  {                                         \
    *imm = SEXT(BITS(i, 31, 12), 20) << 12; \
  } while (0)
#define immS()                                               \
  do                                                         \
  {                                                          \
    *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); \
  } while (0)
#define immJ()                                \
  do                                          \
  {                                           \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 20) | \
           (BITS(i, 19, 12) << 12) |          \
           (BITS(i, 20, 20) << 11) |          \
           (BITS(i, 30, 21) << 1);            \
  } while (0)
#define immB()                                \
  do                                          \
  {                                           \
    *imm = (SEXT(BITS(i, 31, 31), 1) << 12) | \
           (BITS(i, 7, 7) << 11) |            \
           (BITS(i, 30, 25) << 5) |           \
           (BITS(i, 11, 8) << 1);             \
                                              \
  } while (0);

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type)
{
  uint32_t i = s->isa.inst;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd = BITS(i, 11, 7);
  switch (type)
  {
  case TYPE_R:
    src1R();
    src2R();
    break;
  case TYPE_I:
    src1R();
    immI();
    break;
  case TYPE_U:
    immU();
    break;
  case TYPE_S:
    src1R();
    src2R();
    immS();
    break;
  case TYPE_J:
    immJ();
    break;
  case TYPE_B:
    src1R();
    src2R();
    immB();
    break;
  case TYPE_N:
    break;
  default:
    panic("unsupported type = %d", type);
  }
}

static int decode_exec(Decode *s)
{
  s->dnpc = s->snpc;
  bool sign = false;

#define INSTPAT_INST(s) ((s)->isa.inst)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */)         \
  {                                                                  \
    int rd = 0;                                                      \
    word_t src1 = 0, src2 = 0, imm = 0;                              \
    decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
    __VA_ARGS__;                                                     \
  }

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc, U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui, U, R(rd) = imm);
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I, R(rd) = SEXT(BITS(Mr(src1 + imm, 1), 7, 0), 8));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I, R(rd) = SEXT(BITS(Mr(src1 + imm, 2), 15, 0), 16)); // SEXT's len is its len
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I, R(rd) = SEXT(BITS(Mr(src1 + imm, 4), 31, 0), 32));
  INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld, I, R(rd) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I, R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 110 ????? 00000 11", lwu, I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("???? ???? ???? ????? 000 ????? 0001111", fence, I, R(rd) = R(rd)); // 具体的操作先忽略
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I, R(rd) = src1 + imm);
  INSTPAT("000000 ?????? ????? 001 ????? 00100 11", slli, I, R(rd) = src1 << BITS(imm, 5, 0));
  INSTPAT("000000 ?????? ????? 101 ????? 00100 11", srli, I, R(rd) = src1 >> BITS(imm, 5, 0));
  INSTPAT("010000 ?????? ????? 101 ????? 00100 11", srai, I, R(rd) = (int64_t)src1 >> BITS(imm, 5, 0));
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I, slts(src1, imm, rd, 0));
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I, slts(src1, imm, rd, 1));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori, I, R(rd) = src1 ^ imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi, I, R(rd) = src1 & imm);
  INSTPAT("??????? ????? ????? 000 ????? 00110 11", addiw, I, R(rd) = SEXT(BITS(src1 + imm, 31, 0), 32));
  INSTPAT("0000000 ????? ????? 001 ????? 00110 11", slliw, I, R(rd) = SEXT(BITS(BITS(src1, 31, 0) << BITS(imm, 4, 0), 31, 0), 32));
  INSTPAT("0000000 ????? ????? 101 ????? 00110 11", srliw, I, R(rd) = SEXT(BITS(BITS(src1, 31, 0) >> BITS(imm, 4, 0), 31, 0), 32));
  INSTPAT("0100000 ????? ????? 101 ????? 00110 11", sraiw, I, R(rd) = SEXT(BITS((int32_t)BITS(src1, 31, 0) >> BITS(imm, 4, 0), 31, 0), 32));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh, S, Mw(src1 + imm, 2, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw, S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd, S, Mw(src1 + imm, 8, src2));
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R, slts(src1, src2, rd, 0));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R, slts(src1, src2, rd, 1));
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll, R, R(rd) = src1 << BITS(src2, 5, 0));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl;, R, R(rd) = src1 >> BITS(src2, 5, 0));
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra, R, R(rd) = (int64_t)src1 >> BITS(src2, 5, 0));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor, R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or, R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and, R, R(rd) = src1 & src2);
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul, R, R(rd) = (int64_t)src1 * (int64_t)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh, R, R(rd) = mulhALU((int64_t)src1, (int64_t)src2));
  INSTPAT("0000001 ????? ????? 010 ????? 01100 11", mulhsu, R, R(rd) = mulhsuALU((int64_t)src1, (int64_t)src2));
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu, R, R(rd) = mulhuALU(src1, src2));
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", _div, R, R(rd) = (int64_t)src2 == 0 ? UINT64_MAX : ((int64_t)src1) == INT64_MIN && (int64_t)src2 == -1 ? (int64_t)src1
                                                                                                                                                           : (int64_t)src1 / (int64_t)src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu, R, R(rd) = (uint64_t)src2 == 0 ? UINT64_MAX : (uint64_t)src1 / (uint64_t)src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem, R, R(rd) = (int64_t)src2 == 0 ? (int64_t)src1 : ((int64_t)src1) == INT64_MIN && (int64_t)src2 == -1 ? 0
                                                                                                                                                             : (int64_t)src1 % (int64_t)src2);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu, R, R(rd) = (uint64_t)src2 == 0 ? (uint64_t)src1 : (uint64_t)src1 % (uint64_t)src2);
  INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw, R, R(rd) = SEXT(BITS(src1 + src2, 31, 0), 32));
  INSTPAT("0100000 ????? ????? 000 ????? 01110 11", subw, R, R(rd) = SEXT(BITS(src1 - src2, 31, 0), 32));
  INSTPAT("0000000 ????? ????? 001 ????? 01110 11", sllw, R, R(rd) = SEXT(BITS(BITS(src1, 31, 0) << BITS(src2, 4, 0), 31, 0), 32));
  INSTPAT("0000000 ????? ????? 101 ????? 01110 11", srlw, R, R(rd) = SEXT(BITS(BITS(src1, 31, 0) >> BITS(src2, 4, 0), 31, 0), 32));
  INSTPAT("0100000 ????? ????? 101 ????? 01110 11", sraw, R, R(rd) = SEXT(BITS((int32_t)BITS(src1, 31, 0) >> BITS(src2, 4, 0), 31, 0), 32));
  INSTPAT("0000001 ????? ????? 000 ????? 01110 11", mulw, R, R(rd) = SEXT(BITS(BITS(src1, 31, 0) * BITS(src2, 31, 0), 31, 0), 32));
  INSTPAT("0000001 ????? ????? 100 ????? 01110 11", divw, R, R(rd) = (int32_t)BITS(src2, 31, 0) == 0 ? UINT64_MAX : ((int32_t)BITS(src1, 31, 0) == INT32_MIN && (int32_t)BITS(src2, 31, 0) == -1 ? SEXT((int32_t)BITS(src1, 31, 0), 32) : SEXT(BITS((int32_t)BITS(src1, 31, 0) / (int32_t)BITS(src2, 31, 0), 31, 0), 32)));
  INSTPAT("0000001 ????? ????? 101 ????? 01110 11", divuw, R, R(rd) = (uint32_t)BITS(src2, 31, 0) == 0 ? UINT64_MAX : SEXT(BITS((uint32_t)src1 / (uint32_t)src2, 31, 0), 32));
  INSTPAT("0000001 ????? ????? 110 ????? 01110 11", remw, R, R(rd) = (int32_t)BITS(src2, 31, 0) == 0 ? SEXT((int32_t)BITS(src1, 31, 0), 32) : ((int32_t)BITS(src1, 31, 0) == INT32_MIN && (int32_t)BITS(src2, 31, 0) == -1 ? 0 : SEXT(BITS((int32_t)BITS(src1, 31, 0) % (int32_t)BITS(src2, 31, 0), 31, 0), 32)));
  INSTPAT("0000001 ????? ????? 111 ????? 01110 11", remuw, R, R(rd) = (uint32_t)BITS(src2, 31, 0) == 0 ? SEXT((uint32_t)BITS(src1, 31, 0), 32) : SEXT(BITS((uint32_t)BITS(src1, 31, 0) % (uint32_t)BITS(src2, 31, 0), 31, 0), 32));
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 0));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 1));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 2));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 3));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 4));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 5));
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J, Jal(&s->dnpc, s->pc, rd, imm));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I, Jalr(&s->dnpc, s->pc, rd, src1, imm, BITS(s->isa.inst, 19, 15)));
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw, I, csrrw_excute(src1, imm, rd));
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs, I, csrrs_excute(src1, imm, rd));
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret, R, mret_excute(&s->dnpc));
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall, R, s->dnpc = isa_raise_intr(isa_reg_str2val("a7", &sign), s->dnpc));
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv, N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s)
{
  s->isa.inst = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}
