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

void jal_excute(word_t *dnpc, word_t pc, int rd, word_t imm)
{
  *dnpc = pc + imm;
  R(rd) = pc + 4;
}

void jalr_excute(word_t *dnpc, word_t pc, int rd, word_t src1, word_t imm)
{
  *dnpc = (src1 + imm) & ~1;
  R(rd) = pc + 4;
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
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi, I, R(rd) = src1 + imm);
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori, I, R(rd) = src1 | imm);
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb, I, R(rd) = SEXT(BITS(Mr(src1 + imm, 1), 7, 0), 64));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh, I, R(rd) = SEXT(BITS(Mr(src1 + imm, 2), 15, 0), 64));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw, I, R(rd) = SEXT(BITS(Mr(src1 + imm, 4), 31, 0), 64));
  INSTPAT("??????? ????? ????? 011 ????? 00000 11", ld, I, R(rd) = Mr(src1 + imm, 8));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu, I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu, I, R(rd) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 110 ????? 00000 11", lwu, I, R(rd) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti, I, slts(src1, imm, rd, 0));
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu, I, slts(src1, imm, rd, 1));
  INSTPAT("??????? ????? ????? 000 ????? 00110 11", addiw, I, R(rd) = SEXT(BITS(src1 + imm, 31, 0), 64));
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb, S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 011 ????? 01000 11", sd, S, Mw(src1 + imm, 8, src2));
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add, R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub, R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt, R, slts(src1, src2, rd, 0));
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu, R, slts(src1, src2, rd, 1));
  INSTPAT("0000000 ????? ????? 000 ????? 01110 11", addw, R, R(rd) = SEXT(BITS(src1 + src2, 31, 0), 64));
  INSTPAT("0100000 ????? ????? 000 ????? 01110 11", subw, R, R(rd) = SEXT(BITS(src1 + src2, 31, 0), 64));
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 0));
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 1));
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 2));
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 3));
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 4));
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu, B, Bins(&s->dnpc, s->pc, src1, src2, imm, 5));
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal, J, Jal(&s->dnpc, s->pc, rd, imm));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr, I, Jalr(&s->dnpc, s->pc, rd, src1, imm));
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak, N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
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
