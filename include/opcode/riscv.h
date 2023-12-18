/* riscv.h.  RISC-V opcode list for GDB, the GNU debugger.
   Copyright (C) 2011-2021 Free Software Foundation, Inc.
   Contributed by Andrew Waterman

   This file is part of GDB, GAS, and the GNU binutils.

   GDB, GAS, and the GNU binutils are free software; you can redistribute
   them and/or modify them under the terms of the GNU General Public
   License as published by the Free Software Foundation; either version
   3, or (at your option) any later version.

   GDB, GAS, and the GNU binutils are distributed in the hope that they
   will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#ifndef _RISCV_H_
#define _RISCV_H_

#include "riscv-opc.h"
#include <stdlib.h>
#include <stdint.h>

typedef uint64_t insn_t;

static inline unsigned int riscv_insn_length (insn_t insn)
{
#ifdef IN_ASSEMBLER
  if ((insn & 0x3) != 0x3) /* RVC.  */
    return 2;
  if ((insn & 0x1f) != 0x1f) /* Base ISA and extensions in 32-bit space.  */
    return 4;
  if ((insn & 0x3f) == 0x1f) /* 48-bit extensions.  */
    return 6;
  if ((insn & 0x7f) == 0x3f) /* 64-bit extensions.  */
    return 8;
  /* Longer instructions not supported at the moment.  */
  return 2;
#elif defined IN_DISASSEMBLER
  static int sfpu_mode = 0;

  /* TODO:
   * We are hard-coding this to 4 because we know for now that on the SFPU, we
   * will never have anything other than a 4-byte instruction.  In case, this
   * changes in future, then we need to fix it here.
   */
  return 4;

  if (insn > 0xffff  &&
      (   (((insn >> 24) & 0xff) >= SFP_OPCODE_START  &&
           ((insn >> 24) & 0xff) < SFP_OPCODE_END)
       || ((insn >> 24) & 0xff) == 0x2)) /* SFPU */
    {
      sfpu_mode = 1;
      return 4;
    }
  if ((insn & 0x3) == 0x1  ||  (insn & 0x3) == 0x2)
    {
      sfpu_mode = 1;
      return 4;
    }
  if (sfpu_mode && insn == 0x0)
    {
      return 4;
    }
  if ((insn & 0x3) != 0x3) /* RVC.  */
    {
      sfpu_mode = 0;
      return 2;
    }
  if ((insn & 0x1f) != 0x1f) /* Base ISA and extensions in 32-bit space.  */
    {
      sfpu_mode = 0;
      return 4;
    }
  if ((insn & 0x3f) == 0x1f) /* 48-bit extensions.  */
    {
      sfpu_mode = 0;
      return 6;
    }
  if ((insn & 0x7f) == 0x3f) /* 64-bit extensions.  */
    {
      sfpu_mode = 0;
      return 8;
    }
  /* Longer instructions not supported at the moment.  */
  sfpu_mode = 0;
  return 2;
#else
  #error "Must define one of IN_ASSEMBLER or IN_DISASSEMBLER"
#endif
}

static const char * const riscv_rm[8] =
{
  "rne", "rtz", "rdn", "rup", "rmm", 0, 0, "dyn"
};

static const char * const riscv_pred_succ[16] =
{
  0,   "w",  "r",  "rw",  "o",  "ow",  "or",  "orw",
  "i", "iw", "ir", "irw", "io", "iow", "ior", "iorw"
};

#define RVC_JUMP_BITS 11
#define RVC_JUMP_REACH ((1ULL << RVC_JUMP_BITS) * RISCV_JUMP_ALIGN)

#define RVC_BRANCH_BITS 8
#define RVC_BRANCH_REACH ((1ULL << RVC_BRANCH_BITS) * RISCV_BRANCH_ALIGN)

#define RV_X(x, s, n)  (((x) >> (s)) & ((1 << (n)) - 1))
#define RV_IMM_SIGN(x) (-(((x) >> 31) & 1))

#define EXTRACT_ITYPE_IMM(x) \
  (RV_X(x, 20, 12) | (RV_IMM_SIGN(x) << 12))
#define EXTRACT_STYPE_IMM(x) \
  (RV_X(x, 7, 5) | (RV_X(x, 25, 7) << 5) | (RV_IMM_SIGN(x) << 12))
#define EXTRACT_SBTYPE_IMM(x) \
  ((RV_X(x, 8, 4) << 1) | (RV_X(x, 25, 6) << 5) | (RV_X(x, 7, 1) << 11) | (RV_IMM_SIGN(x) << 12))
#define EXTRACT_UTYPE_IMM(x) \
  ((RV_X(x, 12, 20) << 12) | (RV_IMM_SIGN(x) << 32))
#define EXTRACT_UJTYPE_IMM(x) \
  ((RV_X(x, 21, 10) << 1) | (RV_X(x, 20, 1) << 11) | (RV_X(x, 12, 8) << 12) | (RV_IMM_SIGN(x) << 20))
#define EXTRACT_RVC_IMM(x) \
  (RV_X(x, 2, 5) | (-RV_X(x, 12, 1) << 5))
#define EXTRACT_RVC_LUI_IMM(x) \
  (EXTRACT_RVC_IMM (x) << RISCV_IMM_BITS)
#define EXTRACT_RVC_SIMM3(x) \
  (RV_X(x, 10, 2) | (-RV_X(x, 12, 1) << 2))
#define EXTRACT_RVC_UIMM8(x) \
  (RV_X(x, 5, 8))
#define EXTRACT_RVC_ADDI4SPN_IMM(x) \
  ((RV_X(x, 6, 1) << 2) | (RV_X(x, 5, 1) << 3) | (RV_X(x, 11, 2) << 4) | (RV_X(x, 7, 4) << 6))
#define EXTRACT_RVC_ADDI16SP_IMM(x) \
  ((RV_X(x, 6, 1) << 4) | (RV_X(x, 2, 1) << 5) | (RV_X(x, 5, 1) << 6) | (RV_X(x, 3, 2) << 7) | (-RV_X(x, 12, 1) << 9))
#define EXTRACT_RVC_LW_IMM(x) \
  ((RV_X(x, 6, 1) << 2) | (RV_X(x, 10, 3) << 3) | (RV_X(x, 5, 1) << 6))
#define EXTRACT_RVC_LD_IMM(x) \
  ((RV_X(x, 10, 3) << 3) | (RV_X(x, 5, 2) << 6))
#define EXTRACT_RVC_LWSP_IMM(x) \
  ((RV_X(x, 4, 3) << 2) | (RV_X(x, 12, 1) << 5) | (RV_X(x, 2, 2) << 6))
#define EXTRACT_RVC_LDSP_IMM(x) \
  ((RV_X(x, 5, 2) << 3) | (RV_X(x, 12, 1) << 5) | (RV_X(x, 2, 3) << 6))
#define EXTRACT_RVC_SWSP_IMM(x) \
  ((RV_X(x, 9, 4) << 2) | (RV_X(x, 7, 2) << 6))
#define EXTRACT_RVC_SDSP_IMM(x) \
  ((RV_X(x, 10, 3) << 3) | (RV_X(x, 7, 3) << 6))
#define EXTRACT_RVC_B_IMM(x) \
  ((RV_X(x, 3, 2) << 1) | (RV_X(x, 10, 2) << 3) | (RV_X(x, 2, 1) << 5) | (RV_X(x, 5, 2) << 6) | (-RV_X(x, 12, 1) << 8))
#define EXTRACT_RVC_J_IMM(x) \
  ((RV_X(x, 3, 3) << 1) | (RV_X(x, 11, 1) << 4) | (RV_X(x, 2, 1) << 5) | (RV_X(x, 7, 1) << 6) | (RV_X(x, 6, 1) << 7) | (RV_X(x, 9, 2) << 8) | (RV_X(x, 8, 1) << 10) | (-RV_X(x, 12, 1) << 11))

#define ENCODE_ITYPE_IMM(x) \
  (RV_X(x, 0, 12) << 20)
#define ENCODE_STYPE_IMM(x) \
  ((RV_X(x, 0, 5) << 7) | (RV_X(x, 5, 7) << 25))
#define ENCODE_SBTYPE_IMM(x) \
  ((RV_X(x, 1, 4) << 8) | (RV_X(x, 5, 6) << 25) | (RV_X(x, 11, 1) << 7) | (RV_X(x, 12, 1) << 31))
#define ENCODE_UTYPE_IMM(x) \
  (RV_X(x, 12, 20) << 12)
#define ENCODE_UJTYPE_IMM(x) \
  ((RV_X(x, 1, 10) << 21) | (RV_X(x, 11, 1) << 20) | (RV_X(x, 12, 8) << 12) | (RV_X(x, 20, 1) << 31))
#define ENCODE_RVC_IMM(x) \
  ((RV_X(x, 0, 5) << 2) | (RV_X(x, 5, 1) << 12))
#define ENCODE_RVC_LUI_IMM(x) \
  ENCODE_RVC_IMM ((x) >> RISCV_IMM_BITS)
#define ENCODE_RVC_SIMM3(x) \
  (RV_X(x, 0, 3) << 10)
#define ENCODE_RVC_UIMM8(x) \
  (RV_X(x, 0, 8) << 5)
#define ENCODE_RVC_ADDI4SPN_IMM(x) \
  ((RV_X(x, 2, 1) << 6) | (RV_X(x, 3, 1) << 5) | (RV_X(x, 4, 2) << 11) | (RV_X(x, 6, 4) << 7))
#define ENCODE_RVC_ADDI16SP_IMM(x) \
  ((RV_X(x, 4, 1) << 6) | (RV_X(x, 5, 1) << 2) | (RV_X(x, 6, 1) << 5) | (RV_X(x, 7, 2) << 3) | (RV_X(x, 9, 1) << 12))
#define ENCODE_RVC_LW_IMM(x) \
  ((RV_X(x, 2, 1) << 6) | (RV_X(x, 3, 3) << 10) | (RV_X(x, 6, 1) << 5))
#define ENCODE_RVC_LD_IMM(x) \
  ((RV_X(x, 3, 3) << 10) | (RV_X(x, 6, 2) << 5))
#define ENCODE_RVC_LWSP_IMM(x) \
  ((RV_X(x, 2, 3) << 4) | (RV_X(x, 5, 1) << 12) | (RV_X(x, 6, 2) << 2))
#define ENCODE_RVC_LDSP_IMM(x) \
  ((RV_X(x, 3, 2) << 5) | (RV_X(x, 5, 1) << 12) | (RV_X(x, 6, 3) << 2))
#define ENCODE_RVC_SWSP_IMM(x) \
  ((RV_X(x, 2, 4) << 9) | (RV_X(x, 6, 2) << 7))
#define ENCODE_RVC_SDSP_IMM(x) \
  ((RV_X(x, 3, 3) << 10) | (RV_X(x, 6, 3) << 7))
#define ENCODE_RVC_B_IMM(x) \
  ((RV_X(x, 1, 2) << 3) | (RV_X(x, 3, 2) << 10) | (RV_X(x, 5, 1) << 2) | (RV_X(x, 6, 2) << 5) | (RV_X(x, 8, 1) << 12))
#define ENCODE_RVC_J_IMM(x) \
  ((RV_X(x, 1, 3) << 3) | (RV_X(x, 4, 1) << 11) | (RV_X(x, 5, 1) << 2) | (RV_X(x, 6, 1) << 7) | (RV_X(x, 7, 1) << 6) | (RV_X(x, 8, 2) << 9) | (RV_X(x, 10, 1) << 8) | (RV_X(x, 11, 1) << 12))

#define VALID_ITYPE_IMM(x) (EXTRACT_ITYPE_IMM(ENCODE_ITYPE_IMM(x)) == (x))
#define VALID_STYPE_IMM(x) (EXTRACT_STYPE_IMM(ENCODE_STYPE_IMM(x)) == (x))
#define VALID_SBTYPE_IMM(x) (EXTRACT_SBTYPE_IMM(ENCODE_SBTYPE_IMM(x)) == (x))
#define VALID_UTYPE_IMM(x) (EXTRACT_UTYPE_IMM(ENCODE_UTYPE_IMM(x)) == (x))
#define VALID_UJTYPE_IMM(x) (EXTRACT_UJTYPE_IMM(ENCODE_UJTYPE_IMM(x)) == (x))
#define VALID_RVC_IMM(x) (EXTRACT_RVC_IMM(ENCODE_RVC_IMM(x)) == (x))
#define VALID_RVC_LUI_IMM(x) (ENCODE_RVC_LUI_IMM(x) != 0 && EXTRACT_RVC_LUI_IMM(ENCODE_RVC_LUI_IMM(x)) == (x))
#define VALID_RVC_SIMM3(x) (EXTRACT_RVC_SIMM3(ENCODE_RVC_SIMM3(x)) == (x))
#define VALID_RVC_UIMM8(x) (EXTRACT_RVC_UIMM8(ENCODE_RVC_UIMM8(x)) == (x))
#define VALID_RVC_ADDI4SPN_IMM(x) (EXTRACT_RVC_ADDI4SPN_IMM(ENCODE_RVC_ADDI4SPN_IMM(x)) == (x))
#define VALID_RVC_ADDI16SP_IMM(x) (EXTRACT_RVC_ADDI16SP_IMM(ENCODE_RVC_ADDI16SP_IMM(x)) == (x))
#define VALID_RVC_LW_IMM(x) (EXTRACT_RVC_LW_IMM(ENCODE_RVC_LW_IMM(x)) == (x))
#define VALID_RVC_LD_IMM(x) (EXTRACT_RVC_LD_IMM(ENCODE_RVC_LD_IMM(x)) == (x))
#define VALID_RVC_LWSP_IMM(x) (EXTRACT_RVC_LWSP_IMM(ENCODE_RVC_LWSP_IMM(x)) == (x))
#define VALID_RVC_LDSP_IMM(x) (EXTRACT_RVC_LDSP_IMM(ENCODE_RVC_LDSP_IMM(x)) == (x))
#define VALID_RVC_SWSP_IMM(x) (EXTRACT_RVC_SWSP_IMM(ENCODE_RVC_SWSP_IMM(x)) == (x))
#define VALID_RVC_SDSP_IMM(x) (EXTRACT_RVC_SDSP_IMM(ENCODE_RVC_SDSP_IMM(x)) == (x))
#define VALID_RVC_B_IMM(x) (EXTRACT_RVC_B_IMM(ENCODE_RVC_B_IMM(x)) == (x))
#define VALID_RVC_J_IMM(x) (EXTRACT_RVC_J_IMM(ENCODE_RVC_J_IMM(x)) == (x))

#define RISCV_RTYPE(insn, rd, rs1, rs2) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ((rs1) << OP_SH_RS1) | ((rs2) << OP_SH_RS2))
#define RISCV_ITYPE(insn, rd, rs1, imm) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ((rs1) << OP_SH_RS1) | ENCODE_ITYPE_IMM(imm))
#define RISCV_STYPE(insn, rs1, rs2, imm) \
  ((MATCH_ ## insn) | ((rs1) << OP_SH_RS1) | ((rs2) << OP_SH_RS2) | ENCODE_STYPE_IMM(imm))
#define RISCV_SBTYPE(insn, rs1, rs2, target) \
  ((MATCH_ ## insn) | ((rs1) << OP_SH_RS1) | ((rs2) << OP_SH_RS2) | ENCODE_SBTYPE_IMM(target))
#define RISCV_UTYPE(insn, rd, bigimm) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ENCODE_UTYPE_IMM(bigimm))
#define RISCV_UJTYPE(insn, rd, target) \
  ((MATCH_ ## insn) | ((rd) << OP_SH_RD) | ENCODE_UJTYPE_IMM(target))

#define RISCV_NOP RISCV_ITYPE(ADDI, 0, 0, 0)
#define RVC_NOP MATCH_C_ADDI

#define RISCV_CONST_HIGH_PART(VALUE) \
  (((VALUE) + (RISCV_IMM_REACH/2)) & ~(RISCV_IMM_REACH-1))
#define RISCV_CONST_LOW_PART(VALUE) ((VALUE) - RISCV_CONST_HIGH_PART (VALUE))
#define RISCV_PCREL_HIGH_PART(VALUE, PC) RISCV_CONST_HIGH_PART((VALUE) - (PC))
#define RISCV_PCREL_LOW_PART(VALUE, PC) RISCV_CONST_LOW_PART((VALUE) - (PC))

#define RISCV_JUMP_BITS RISCV_BIGIMM_BITS
#define RISCV_JUMP_ALIGN_BITS 1
#define RISCV_JUMP_ALIGN (1 << RISCV_JUMP_ALIGN_BITS)
#define RISCV_JUMP_REACH ((1ULL << RISCV_JUMP_BITS) * RISCV_JUMP_ALIGN)

#define RISCV_IMM_BITS 12
#define RISCV_BIGIMM_BITS (32 - RISCV_IMM_BITS)
#define RISCV_IMM_REACH (1LL << RISCV_IMM_BITS)
#define RISCV_BIGIMM_REACH (1LL << RISCV_BIGIMM_BITS)
#define RISCV_RVC_IMM_REACH (1LL << 6)
#define RISCV_BRANCH_BITS RISCV_IMM_BITS
#define RISCV_BRANCH_ALIGN_BITS RISCV_JUMP_ALIGN_BITS
#define RISCV_BRANCH_ALIGN (1 << RISCV_BRANCH_ALIGN_BITS)
#define RISCV_BRANCH_REACH (RISCV_IMM_REACH * RISCV_BRANCH_ALIGN)

/* RV fields.  */

#define OP_MASK_OP		0x7f
#define OP_SH_OP		0
#define OP_MASK_RS2		0x1f
#define OP_SH_RS2		20
#define OP_MASK_RS1		0x1f
#define OP_SH_RS1		15
#define OP_MASK_RS3		0x1fU
#define OP_SH_RS3		27
#define OP_MASK_RD		0x1f
#define OP_SH_RD		7
#define OP_MASK_SHAMT		0x3f
#define OP_SH_SHAMT		20
#define OP_MASK_SHAMTW		0x1f
#define OP_SH_SHAMTW		20
#define OP_MASK_RM		0x7
#define OP_SH_RM		12
#define OP_MASK_PRED		0xf
#define OP_SH_PRED		24
#define OP_MASK_SUCC		0xf
#define OP_SH_SUCC		20
#define OP_MASK_AQ		0x1
#define OP_SH_AQ		26
#define OP_MASK_RL		0x1
#define OP_SH_RL		25

#define OP_MASK_CUSTOM_IMM	0x7fU
#define OP_SH_CUSTOM_IMM	25
#define OP_MASK_CSR		0xfffU
#define OP_SH_CSR		20

#define OP_MASK_FUNCT3         0x7
#define OP_SH_FUNCT3           12
#define OP_MASK_FUNCT7         0x7fU
#define OP_SH_FUNCT7           25
#define OP_MASK_FUNCT2         0x3
#define OP_SH_FUNCT2           25

/* RVC fields.  */

#define OP_MASK_OP2            0x3
#define OP_SH_OP2              0

#define OP_MASK_CRS2 0x1f
#define OP_SH_CRS2 2
#define OP_MASK_CRS1S 0x7
#define OP_SH_CRS1S 7
#define OP_MASK_CRS2S 0x7
#define OP_SH_CRS2S 2

#define OP_MASK_CFUNCT6                0x3f
#define OP_SH_CFUNCT6          10
#define OP_MASK_CFUNCT4                0xf
#define OP_SH_CFUNCT4          12
#define OP_MASK_CFUNCT3                0x7
#define OP_SH_CFUNCT3          13
#define OP_MASK_CFUNCT2                0x3
#define OP_SH_CFUNCT2          5

/* Field names for SFPU instructions */
#define OP_SH_SFPU_OP		        24
#define OP_MASK_SFPU_OP			0xff
#define OP_SH_YMULADD_SRCA 		16
#define OP_MASK_YMULADD_SRCA 		0xf
#define OP_SH_YMULADD_SRCB 		12
#define OP_MASK_YMULADD_SRCB 		0xf
#define OP_SH_YMULADD_SRCC 		8
#define OP_MASK_YMULADD_SRCC 		0xf
#define OP_SH_YMULADD_DEST 		4
#define OP_MASK_YMULADD_DEST 		0xf
#define OP_SH_YMULADD_INSTR_MOD0	0
#define OP_MASK_YMULADD_INSTR_MOD0	0xf

#define OP_SH_YLOADSTORE_RD		20
#define OP_MASK_YLOADSTORE_RD		0xf
#define OP_SH_YLOADSTORE_INSTR_MOD0	16
#define OP_MASK_YLOADSTORE_INSTR_MOD0	0xf
#define OP_SH_YDEST_REG_ADDR		0
#define OP_MASK_YDEST_REG_ADDR		0xffff

#define OP_SH_YMULI_IMM16_MATH		8
#define OP_MASK_YMULI_IMM16_MATH	0xffff
#define OP_SH_LMULI_IMM16_MATH          0
#define OP_MASK_LMULI_IMM16_MATH        0xffff
#define OP_SH_YCC_IMM12_MATH		12
#define OP_MASK_YCC_IMM12_MATH		0xfff
#define OP_SH_YCC_LREG_C		8
#define OP_MASK_YCC_LREG_C		0xf
#define OP_SH_YCC_LREG_DEST		4
#define OP_MASK_YCC_LREG_DEST		0xf
#define OP_SH_YCC_INSTR_MOD1		0
#define OP_MASK_YCC_INSTR_MOD1		0xf
#define OP_SH_LCC_LREG_A                16
#define OP_MASK_LCC_LREG_A              0xff
#define OP_SH_LCC_LREG_B                4
#define OP_MASK_LCC_LREG_B              0xfff
#define OP_SH_L_LREG_DEST               4
#define OP_MASK_L_LREG_DEST             0xfffff
#define OP_SH_L_ADDR_MODE      		14 
#define OP_MASK_L_ADDR_MODE    		0x7 
#define OP_SH_L_ADDR_MODE_2      	14  
#define OP_MASK_L_ADDR_MODE_2   	0x1f
#define OP_SH_L_DEST_2                  0   
#define OP_MASK_L_DEST_2                0x7ff
#define OP_SH_L_CFG_CONTEXT             21  
#define OP_MASK_L_CFG_CONTEXT           0x7
#define OP_SH_L_ROW_PAD_ZERO            18 
#define OP_MASK_L_ROW_PAD_ZERO          0x7
#define OP_SH_L_DEST_ACCESS_MODE        17 
#define OP_MASK_L_DEST_ACCESS_MODE      0x1
#define OP_SH_L_ADDR_MODE_3             15 
#define OP_MASK_L_ADDR_MODE_3           0x3
#define OP_SH_L_ADDR_CNT_CONTEXT        13 
#define OP_MASK_L_ADDR_CNT_CONTEXT      0x3
#define OP_SH_L_ZERO_WRITE              12 
#define OP_MASK_L_ZERO_WRITE            0x1
#define OP_SH_L_READ_INTF_SEL           8  
#define OP_MASK_L_READ_INTF_SEL         0xf
#define OP_SH_L_OVERTHREAD_ID           7   
#define OP_MASK_L_OVERTHREAD_ID         0x1
#define OP_SH_L_CTXT_CTRL               2   
#define OP_MASK_L_CTXT_CTRL             0x3
#define OP_SH_L_UNUSED                  12   
#define OP_MASK_L_UNUSED                0x3ff
#define OP_SH_L_DISABLE_STALL           10 
#define OP_MASK_L_DISABLE_STALL         0x3
#define OP_SH_L_ADDR_SEL                8   
#define OP_MASK_L_ADDR_SEL              0x3
#define OP_SH_L_STREAM_ID               2   
#define OP_MASK_L_STREAM_ID             0x3f
#define OP_SH_L_ADDRMODE_3              14
#define OP_MASK_L_ADDRMODE_3            0x3ff
#define OP_SH_L_ROT_SHIFT               10   
#define OP_MASK_L_ROT_SHIFT             0xf
#define OP_SH_L_CFG_CONTEXT_CNT_INC     13 
#define OP_MASK_L_CFG_CONTEXT_CNT_INC   0x3
#define OP_SH_L_ADDR_CNT_CONTEXT_ID     8
#define OP_MASK_L_ADDR_CNT_CONTEXT_ID   0x3
#define OP_SH_L_SRC_BCAST               5
#define OP_MASK_L_SRC_BCAST             0x1
#define OP_SH_L_ZERO_WRITE_2            4   
#define OP_MASK_L_ZERO_WRITE_2          0x1
#define OP_SH_L_STREAM_ID_2             16 
#define OP_MASK_L_STREAM_ID_2           0x7f
#define OP_SH_L_MSG_CLR_CNT             12
#define OP_MASK_L_MSG_CLR_CNT           0xf
#define OP_SH_L_SETDVALID               8 
#define OP_MASK_L_SETDVALID             0xf
#define OP_SH_L_CLR_TO_FMT_CNTRL        6  
#define OP_MASK_L_CLR_TO_FMT_CNTRL      0x3
#define OP_SH_L_STALL_CLR_CNTRL         5  
#define OP_MASK_L_STALL_CLR_CNTRL       0x1
#define OP_SH_L_BANK_CLR_CNTRL          4  
#define OP_MASK_L_BANK_CLR_CNTRL        0x1
#define OP_SH_L_SRC_CLR_CNTRL           2 
#define OP_MASK_L_SRC_CLR_CNTRL         0x3
#define OP_SH_L_UNPACK_POP              0  
#define OP_MASK_L_UNPACK_POP            0x3
#define OP_SH_L_DISABLE_MASK_OLD_VALUE     23
#define OP_MASK_L_DISABLE_MASK_OLD_VALUE   0x1
#define OP_SH_L_OPERATION           	20
#define OP_MASK_L_OPERATION   		0x7
#define OP_SH_L_MASK_WIDTH     		15
#define OP_MASK_L_MASK_WIDTH  		0x1f
#define OP_SH_L_RIGHT_CSHIFT_AMT     	10
#define OP_MASK_L_RIGHT_CSHIFT_AMT   	0x1f
#define OP_SH_L_SCRATCH_SEL     	8
#define OP_MASK_L_SCRATCH_SEL   	0x3
#define OP_SH_L_CFG_REG     		0
#define OP_MASK_L_CFG_REG   		0xff
#define OP_SH_L_LINGER_TIME     	13
#define OP_MASK_L_LINGER_TIME   	0x7ff
#define OP_SH_L_RESOURCE     		4
#define OP_MASK_L_RESOURCE   		0x1ff
#define OP_SH_L_OP_CLASS     		0
#define OP_MASK_L_OP_CLASS    		0xf
#define OP_SH_L_TARGET_VALUE     	4
#define OP_MASK_L_TARGET_VALUE  	0x7ff
#define OP_SH_L_TARGET_SEL       	3
#define OP_MASK_L_TARGET_SEL  		0x1
#define OP_SH_L_WAIT_STREAM_SEL    	0
#define OP_MASK_L_WAIT_STREAM_SEL  	0x7
#define OP_SH_L_STREAM_ID_SEL    	21
#define OP_MASK_L_STREAM_ID_SEL  	0x7
#define OP_SH_L_STREAM_REG_ADDR    	11
#define OP_MASK_L_STREAM_REG_ADDR  	0x3ff
#define OP_SH_L_CFG_REG_2     		0
#define OP_MASK_L_CFG_REG_2   		0x7ff
#define OP_SH_WLOADSTORE_DEST_REG_ADDR   0
#define OP_MASK_WLOADSTORE_DEST_REG_ADDR 0x3fff
#define OP_SH_WLOADSTORE_ADDR_MODE       14
#define OP_MASK_WLOADSTORE_ADDR_MODE     0x3
#define OP_SH_L_32BIT_MODE       18
#define OP_MASK_L_32BIT_MODE     0x1
#define OP_SH_L_CLR_ZERO_FLAGS   17
#define OP_MASK_L_CLR_ZERO_FLAGS 0x1
#define OP_SH_L_ADDR_MODE_4   14
#define OP_MASK_L_ADDR_MODE_4 0x7
#define OP_SH_L_WHERE   0
#define OP_MASK_L_WHERE 0x3fff
#define OP_SH_L_INSTRMODE     11
#define OP_MASK_L_INSTRMODE   0x7


#define OP_SH_WINCRWC_RWC_A       6
#define OP_MASK_WINCRWC_RWC_A     0xf
#define OP_SH_WINCRWC_RWC_B       10
#define OP_MASK_WINCRWC_RWC_B     0xf
#define OP_SH_WINCRWC_RWC_CR      18
#define OP_MASK_WINCRWC_RWC_CR    0x3f
#define OP_SH_WINCRWC_RWC_D       14
#define OP_MASK_WINCRWC_RWC_D     0xf

#define OP_SH_WREPLAY_LOAD_MODE       	0
#define OP_MASK_WREPLAY_LOAD_MODE     	0x1
#define OP_SH_WREPLAY_EXEC_WHILE_LOAD   1
#define OP_MASK_WREPLAY_EXEC_WHILE_LOAD 0x7
#define OP_SH_WREPLAY_LEN       	4
#define OP_MASK_WREPLAY_LEN     	0x3ff
#define OP_SH_WREPLAY_START_IDX       	14
#define OP_MASK_WREPLAY_START_IDX     	0x3ff

#define OP_SH_WSTOCH_RND_MODE		21
#define OP_MASK_WSTOCH_RND_MODE		0x1
#define OP_SH_WSTOCH_RND_IMM8_MATH	16
#define OP_MASK_WSTOCH_RND_IMM8_MATH	0x1f

/* DMA REG Operands */
#define OP_SH_DMA_REG_OP_A		0
#define OP_MASK_DMA_REG_OP_A		0x3f
#define OP_SH_DMA_REG_OP_B		6
#define OP_MASK_DMA_REG_OP_B		0x3f
#define OP_SH_DMA_REG_OP_RES		12
#define OP_MASK_DMA_REG_OP_RES		0x3f
#define OP_SH_DMA_REG_OP_OPSEL		18
#define OP_MASK_DMA_REG_OP_OPSEL	0x1f
#define OP_SH_DMA_REG_OP_B_ISCONST	23
#define OP_MASK_DMA_REG_OP_B_ISCONST	0x1
#define OP_SH_PAYLOAD_SIGSEL_SIZE	22
#define OP_MASK_PAYLOAD_SIGSEL_SIZE	0x3
#define OP_SH_PAYLOAD_SIGSEL		8
#define OP_MASK_PAYLOAD_SIGSEL		0x3fff
#define OP_SH_SET_SIG_MODE		7
#define OP_MASK_SET_SIG_MODE		0x1
#define OP_SH_REG_INDEX16B		0
#define OP_MASK_REG_INDEX16B		0x7f
#define OP_SH_CNT_SET_MASK		21
#define OP_MASK_CNT_SET_MASK		0x7
#define OP_SH_CH1_Y			15
#define OP_MASK_CH1_Y			0x3f
#define OP_SH_CH1_X			12
#define OP_MASK_CH1_X			0x7
#define OP_SH_CH0_Y			9
#define OP_MASK_CH0_Y			0x7
#define OP_SH_CH0_X			6
#define OP_MASK_CH0_X			0x7
#define OP_SH_BIT_MASK			0
#define OP_MASK_BIT_MASK		0x3f
#define OP_SH_CLEAR_DVALID		22
#define OP_MASK_CLEAR_DVALID		0x3
#define OP_SH_ADDR_MODE			15
#define OP_MASK_ADDR_MODE		0x7f
#define OP_SH_INDEX_EN			14
#define OP_MASK_INDEX_EN		0x1
#define OP_SH_DST			0
#define OP_MASK_DST			0x3fff
#define OP_SH_MEMHIER_SEL		23
#define OP_MASK_MEMHIER_SEL		0x1
#define OP_SH_SWAP_OR_INCR_VAL		18
#define OP_MASK_SWAP_OR_INCR_VAL	0xf
#define OP_SH_WRAP_VAL_OR_SWAP_MASK	14
#define OP_MASK_WRAP_VAL_OR_SWAP_MASK   0x1ff
#define OP_SH_SEL32B			12
#define OP_MASK_SEL32B			0x3
#define OP_SH_DATA_REG_IDX		6
#define OP_MASK_DATA_REG_IDX		0x3f
#define OP_SH_ADDR_REG_IDX		0
#define OP_MASK_ADDR_REG_IDX		0x3f
#define OP_SH_NO_INCR			22
#define OP_MASK_NO_INCR			0x1
#define OP_SH_MUTEX_IDX	                0
#define OP_MASK_MUTEX_IDX               0xffffff
#define OP_SH_RESET                     0
#define OP_MASK_RESET                   0x3fffff
#define OP_SH_ROTATE_WEIGHTS            17
#define OP_MASK_ROTATE_WEIGHTS          0xf
#define OP_SH_ADDRMODE                  15
#define OP_MASK_ADDRMODE                0X3
#define OP_SH_DST_15BIT                 0
#define OP_MASK_DST_15BIT               0x7fff
#define OP_SH_DEST_ACCUM_EN             21
#define OP_MASK_DEST_ACCUM_EN           0x1
#define OP_SH_INSTR_MOD19               19
#define OP_MASK_INSTR_MOD19             0x3
#define OP_SH_EL_ADDRMODE                 15
#define OP_MASK_EL_ADDRMODE		0xf
#define OP_SH_FLUSH_SPEC                0
#define OP_MASK_FLUSH_SPEC		0xffffff
#define OP_SH_INSTRMOD19                19
#define OP_MASK_INSTRMOD19              0x7
#define OP_SH_MAX_POOL_INDEX_EN		14
#define OP_MASK_MAX_POOL_INDEX_EN	0x1
#define OP_SH_RESET_SRCB_GATE_CONTROL   1
#define OP_MASK_RESET_SRCB_GATE_CONTROL 0x7fffff
#define OP_SH_RESET_SRCA_GATE_CONTROL   0
#define OP_MASK_RESET_SRCA_GATE_CONTROL 0x1
#define OP_SH_CNT_SET_MASK		21
#define OP_MASK_CNT_SET_MASK		0x7
#define OP_SH_RWC_CR			18
#define OP_MASK_RWC_CR		       	0xf
#define OP_SH_RWC_D                    	14
#define OP_MASK_RWC_D                  	0xf
#define OP_SH_RWC_B                    	14
#define OP_MASK_RWC_B                  	0xf
#define OP_SH_RWC_A                    	6
#define OP_MASK_RWC_A                  	0xf
#define OP_SH_SIZE_SEL			22
#define OP_MASK_SIZE_SEL		0x3
#define OP_SH_OFFSET_INDEX		14
#define OP_MASK_OFFSET_INDEX		0xff
#define OP_SH_AUTO_INC_SPEC		12
#define OP_MASK_AUTO_INC_SPEC		0x3
#define OP_SH_TDMA_DATA_REG_IDX		18
#define OP_MASK_TDMA_DATA_REG_IDX	0x3f
#define OP_SH_REGADDR			0
#define OP_MASK_REGADDR			0x3ffff
#define OP_SH_ROTATEWEIGHTS		17
#define OP_MASK_ROTATEWEIGHTS		0x1f
#define OP_SH_MOP_TYPE			23
#define OP_MASK_MOP_TYPE		0x1
#define OP_SH_LOOP_COUNT		16
#define OP_MASK_LOOP_COUNT		0x7f
#define OP_SH_ZMASK_LO16		0	
#define OP_MASK_ZMASK_LO16		0xffff
#define OP_SH_ZMASK_HI16		0
#define OP_MASK_ZMASK_HI16		0xffffff
#define OP_SH_DEST_32B_LO		23
#define OP_MASK_DEST_32B_LO		0x1
#define OP_SH_SRC			17
#define OP_MASK_SRC			0x3f
#define OP_SH_INSTRMODE			12
#define OP_MASK_INSTRMODE		0x3
#define OP_SH_DST_MOV			0
#define OP_MASK_DST_MOV			0xfff
#define OP_SH_SRCA			17
#define OP_MASK_SRCA			0x7f
#define OP_SH_SRCB			0
#define OP_MASK_SRCB		  	0xfff
#define OP_SH_ADDRMODE_PACR		15
#define OP_MASK_ADDRMODE_PACR		0x1ff
#define OP_SH_ZERO_WRITE		12
#define OP_MASK_ZERO_WRITE		0x7
#define OP_SH_OVRD_TREAD_ID		7
#define OP_MASK_OVRD_TREAD_ID		0x1
#define OP_SH_CONCAT			4
#define OP_MASK_CONCAT			0x7
#define OP_SH_FLUSH			1
#define OP_MASK_FLUSH			0x7
#define OP_SH_LAST			0
#define OP_MASK_LAST			0x1
#define OP_SH_PUSH			23
#define OP_MASK_PUSH			0x1
#define OP_SH_ADDR_SEL			22	
#define OP_MASK_ADDR_SEL		0x1
#define OP_SH_WR_DATA			12 
#define OP_MASK_WR_DATA			0x3ff
#define OP_SH_PACK_SEL			8
#define OP_MASK_PACK_SEL		0xf
#define OP_SH_STREAM_ID			2
#define OP_MASK_STREAM_ID		0x3f
#define OP_SH_FLUSH_SET			1 
#define OP_MASK_FLUSH_SET		0x1
#define OP_SH_GPR_ADDRESS		16
#define OP_MASK_GPR_ADDRESS		0xff
#define OP_SH_CFG_REG			0
#define OP_MASK_CFG_REG			0xffff
#define OP_SH_TARGET_SEL		20
#define OP_MASK_TARGET_SEL		0x3
#define OP_SH_BYTE_OFFSET		18
#define OP_MASK_BYTE_OFFSET		0x3
#define OP_SH_CONTEXTID_2		16
#define OP_MASK_CONTEXTID_2		0x3
#define OP_SH_FLOP_INDEX		6 
#define OP_MASK_FLOP_INDEX		0x3ff
#define OP_SH_REG_INDEX			0
#define OP_MASK_REG_INDEX		0x3f
#define OP_SH_START_IDX			14
#define OP_MASK_START_IDX		0x3ff
#define OP_SH_LEN			4
#define OP_MASK_LEN			0x3ff
#define OP_SH_EXECUTE_WHILE_LOADING	1
#define OP_MASK_EXECUTE_WHILE_LOADING	0x7
#define OP_SH_LOAD_MODE			0
#define OP_MASK_LOAD_MODE		0x1
#define OP_SH_MASK			16
#define OP_MASK_MASK			0xff
#define OP_SH_DATA			8
#define OP_MASK_DATA			0xff
#define OP_SH_CFG_REG_ADDR		0
#define OP_MASK_CFG_REG_ADDR		0xff
#define OP_SH_SEM_SEL			2
#define OP_MASK_SEM_SEL			0x3fffff
#define OP_SH_MAX_VALUE			20
#define OP_MASK_MAX_VALUE		0xf
#define OP_SH_INIT_VALUE		16
#define OP_MASK_INIT_VALUE		0xf
#define OP_SH_SEMSEL_SEMINIT		2
#define OP_MASK_SEMSEL_SEMINIT		0x3fff
#define OP_SH_STALL_RES			15
#define OP_MASK_STALL_RES		0x1ff
#define OP_SH_SEMSEL_SEMWAIT		2
#define OP_MASK_SEMSEL_SEMWAIT		0x1fff
#define OP_SH_WAIT_SEM_COND		0
#define OP_MASK_WAIT_SEM_COND		0x3
#define OP_SH_CHANNEL_INDEX		20
#define OP_MASK_CHANNEL_INDEX		0x1
#define OP_SH_DIMENSIONINDEX		18
#define OP_MASK_DIMENSIONINDEX		0x3
#define OP_SH_VALUE			0
#define OP_MASK_VALUE			0x3ffff
#define OP_SH_X_END2			10
#define OP_MASK_X_END2			0x7ff
#define OP_SH_X_START			0
#define OP_MASK_X_START			0x3ff
#define OP_SH_BITMASK			0
#define OP_MASK_BITMASK			0x3f
#define OP_SH_REGMASK			1
#define OP_MASK_REGMASK			0x7fffff
#define OP_SH_HALO_MASK			0
#define OP_MASK_HALO_MASK		0x1
#define OP_SH_REG_MASK_2		0
#define OP_MASK_REG_MASK_2		0xffffff
#define OP_SH_SETC16_REG		16
#define OP_MASK_SETC16_REG		0xff
#define OP_SH_SETC16_VALUE		0
#define OP_MASK_SETC16_VALUE		0xffff
#define OP_SH_SETVALID			0
#define OP_MASK_SETVALID		0xffffff
#define OP_SH_RWC_BIAS			6
#define OP_MASK_RWC_BIAS		0xfff
#define OP_SH_SET_INC_CTRL		0
#define OP_MASK_SET_INC_CTRL		0x3f
#define OP_SH_Y_END			12
#define OP_MASK_Y_END			0xfff
#define OP_SH_Y_START			8
#define OP_MASK_Y_START			0xf
#define OP_SH_X_END			4
#define OP_MASK_X_END			0xf
#define OP_SH_CLEAR_AB_VLD		22
#define OP_MASK_CLEAR_AB_VLD		0x3
#define OP_SH_LREG_IND			20
#define OP_MASK_LREG_IND		0xf
#define OP_SH_INSTR_MOD0		16
#define OP_MASK_INSTR_MOD0		0xf
#define OP_SH_SFPU_ADDR_MODE		14
#define OP_MASK_SFPU_ADDR_MODE		0x3
#define OP_SH_LSFPU_ADDR_MODE           13 
#define OP_MASK_LSFPU_ADDR_MODE         0x7
#define OP_SH_DEST_REG_ADDR		0
#define OP_MASK_DEST_REG_ADDR		0x3fff
#define OP_SH_LDEST_REG_ADDR            0
#define OP_MASK_LDEST_REG_ADDR          0x1fff
#define OP_SH_LOG2_AMOUNT2		2
#define OP_MASK_LOG2_AMOUNT2		0x3ffff
#define OP_SH_SHIFT_MODE		0
#define OP_MASK_SHIFT_MODE		0x3
#define OP_SH_ROT_SHIFT			10
#define OP_MASK_ROT_SHIFT		0x1f
#define OP_SH_SHIFT_ROW			0
#define OP_MASK_SHIFT_ROW		0x3ff
#define OP_SH_WAIT_RES			0
#define OP_MASK_WAIT_RES		0x7fff
#define OP_SH_SIZESEL			22
#define OP_MASK_SIZESEL			0x1
#define OP_SH_REGSIZESEL		21
#define OP_MASK_REGSIZESEL		0x1
#define OP_SH_OFFSETINDEX		14
#define OP_MASK_OFFSETINDEX		0x7f
#define OP_SH_UNPACK_BLOCK_SELECTION	23
#define OP_MASK_UNPACK_BLOCK_SELECTION	0x1
#define OP_SH_ADDRMODE_UNPACROP		15
#define OP_MASK_ADDRMODE_UNPACROP	0xff
#define OP_SH_CFGCONTEXTCNTINC		12
#define OP_MASK_CFGCONTEXTCNTINC	0x3
#define OP_SH_CFGCONTEXTID		10
#define OP_MASK_CFGCONTEXTID		0x7
#define OP_SH_ADDRCNTCONTEXTID		8
#define OP_MASK_ADDRCNTCONTEXTID	0x7
#define OP_SH_SETDATVALID		6
#define OP_MASK_SETDATVALID		0x1
#define OP_SH_RAREB_EN			5
#define OP_MASK_RAREB_EN		0x1	
#define OP_SH_ZEROWRITE2		4
#define OP_MASK_ZEROWRITE2		0x1
#define OP_SH_AUTOINCCONTEXTID		3
#define OP_MASK_AUTOINCCONTEXTID	0x1
#define OP_SH_ROWSEARCH			2
#define OP_MASK_ROWSEARCH		0x1
#define OP_SH_SEARCHCASHFLOW		1
#define OP_MASK_SEARCHCASHFLOW		0x1
#define OP_SH_NOOP			0
#define OP_MASK_NOOP			0x7fffff	
#define OP_SH_WR128B			15
#define OP_MASK_WR128B			0x1
#define OP_SH_CFGREG			0
#define OP_MASK_CFGREG			0x7fff
#define OP_SH_MOV_BLOCK_SELECTION	23
#define OP_MASK_MOV_BLOCK_SELECTION	0x1
#define OP_SH_LAST_XMOVOP		0
#define OP_MASK_LAST_XMOVOP		0x7fffff
#define OP_SH_CLEARCODE			19
#define OP_MASK_CLEARCODE		0x1f
#define OP_SH_ZEROVAL			4
#define OP_MASK_ZEROVAL			0xfffff
#define OP_SH_WRITEMODE			3
#define OP_MASK_WRITEMODE		0x1
#define OP_SH_BANKMASK			2
#define OP_MASK_BANKMASK		0x1
#define OP_SH_SRCMASK			0
#define OP_MASK_SRCMASK			0x3
#define OP_SH_CMP_VAL			14
#define OP_MASK_CMP_VAL			0xf
#define OP_SH_PKEDG_X_START		0
#define OP_MASK_PKEDG_X_START		0xf
#define OP_SH_MOV_INSTR_MOD		19
#define OP_MASK_MOV_INSTR_MOD		0x1f
#define OP_SH_MOV_SRC			10
#define OP_MASK_MOV_SRC			0x1f
#define OP_SH_MOV_DST			0
#define OP_MASK_MOV_DST			0x3ff
#define OP_SH_GRAY_STALL_RES		14
#define OP_MASK_GRAY_STALL_RES		0x3ff
#define OP_SH_GRAY_SEM_SEL		2
#define OP_MASK_GRAY_SEM_SEL		0xfff
#define OP_SH_WAIT_RES_14BIT		0
#define OP_MASK_WAIT_RES_14BIT		0x3fff
#define OP_SH_POOL_ADDR_MODE		15
#define OP_MASK_POOL_ADDR_MODE     	0x7
/* ABI names for selected x-registers.  */

#define X_RA 1
#define X_SP 2
#define X_GP 3
#define X_TP 4
#define X_T0 5
#define X_T1 6
#define X_T2 7
#define X_T3 28

#define NGPR 32
#define NFPR 32
#define NSFPUR 16

/* These fake label defines are use by both the assembler, and
   libopcodes.  The assembler uses this when it needs to generate a fake
   label, and libopcodes uses it to hide the fake labels in its output.  */
#define RISCV_FAKE_LABEL_NAME ".L0 "
#define RISCV_FAKE_LABEL_CHAR ' '

/* Replace bits MASK << SHIFT of STRUCT with the equivalent bits in
   VALUE << SHIFT.  VALUE is evaluated exactly once.  */
#define INSERT_BITS(STRUCT, VALUE, MASK, SHIFT) \
  (STRUCT) = (((STRUCT) & ~((insn_t)(MASK) << (SHIFT))) \
	      | ((insn_t)((VALUE) & (MASK)) << (SHIFT)))

/* Extract bits MASK << SHIFT from STRUCT and shift them right
   SHIFT places.  */
#define EXTRACT_BITS(STRUCT, MASK, SHIFT) \
  (((STRUCT) >> (SHIFT)) & (MASK))

/* Extract the operand given by FIELD from integer INSN.  */
#define EXTRACT_OPERAND(FIELD, INSN) \
  EXTRACT_BITS ((INSN), OP_MASK_##FIELD, OP_SH_##FIELD)

/* Put top 2 bits, which are currently never 'b11 to bottom, indicating to Risc
   that they are not risc instructions. */
#define SFPU_OP_SWIZZLE(x)   ( (((x) >> 30) & 0x3) | (((x) & 0x3FFFFFFF) << 2) ) 
#define SFPU_OP_UNSWIZZLE(x) ( (((x) & 0x3) << 30) | (((x) & 0xfffffffc) >> 2) )

/* The maximal number of subset can be required. */
#define MAX_SUBSET_NUM 4

/* All RISC-V instructions belong to at least one of these classes.  */

enum riscv_insn_class
  {
   INSN_CLASS_NONE,

   INSN_CLASS_I,
   INSN_CLASS_C,
   INSN_CLASS_A,
   INSN_CLASS_M,
   INSN_CLASS_F,
   INSN_CLASS_D,
   INSN_CLASS_Q,
   INSN_CLASS_F_AND_C,
   INSN_CLASS_D_AND_C,
   INSN_CLASS_ZICSR,
   INSN_CLASS_ZIFENCEI,
   INSN_CLASS_ZIHINTPAUSE,
   INSN_CLASS_I_Y,  /* Gray Skull */
   INSN_CLASS_I_W,  /* Worm Hole */
   INSN_CLASS_I_L,  /* Black Hole */
  };

/* This structure holds information for a particular instruction.  */

struct riscv_opcode
{
  /* The name of the instruction.  */
  const char *name;
  /* The requirement of xlen for the instruction, 0 if no requirement.  */
  unsigned xlen_requirement;
  /* Class to which this instruction belongs.  Used to decide whether or
     not this instruction is legal in the current -march context.  */
  enum riscv_insn_class insn_class;
  /* A string describing the arguments for this instruction.  */
  const char *args;
  /* The basic opcode for the instruction.  When assembling, this
     opcode is modified by the arguments to produce the actual opcode
     that is used.  If pinfo is INSN_MACRO, then this is 0.  */
  insn_t match;
  /* If pinfo is not INSN_MACRO, then this is a bit mask for the
     relevant portions of the opcode when disassembling.  If the
     actual opcode anded with the match field equals the opcode field,
     then we have found the correct instruction.  If pinfo is
     INSN_MACRO, then this field is the macro identifier.  */
  insn_t mask;
  /* A function to determine if a word corresponds to this instruction.
     Usually, this computes ((word & mask) == match).  */
  int (*match_func) (const struct riscv_opcode *op, insn_t word);
  /* For a macro, this is INSN_MACRO.  Otherwise, it is a collection
     of bits describing the instruction, notably any relevant hazard
     information.  */
  unsigned long pinfo;
};

/* The current supported ISA spec versions.  */

enum riscv_isa_spec_class
{
  ISA_SPEC_CLASS_NONE,

  ISA_SPEC_CLASS_2P2,
  ISA_SPEC_CLASS_20190608,
  ISA_SPEC_CLASS_20191213,
  ISA_SPEC_CLASS_DRAFT
};

#define RISCV_UNKNOWN_VERSION -1

/* This structure holds version information for specific ISA.  */

struct riscv_ext_version
{
  const char *name;
  enum riscv_isa_spec_class isa_spec_class;
  int major_version;
  int minor_version;
};

/* All RISC-V CSR belong to one of these classes.  */

enum riscv_csr_class
{
  CSR_CLASS_NONE,

  CSR_CLASS_I,
  CSR_CLASS_I_32,      /* rv32 only */
  CSR_CLASS_F,         /* f-ext only */
  CSR_CLASS_DEBUG      /* debug CSR */
};

/* The current supported privilege spec versions.  */

enum riscv_priv_spec_class
{
  PRIV_SPEC_CLASS_NONE,

  PRIV_SPEC_CLASS_1P9P1,
  PRIV_SPEC_CLASS_1P10,
  PRIV_SPEC_CLASS_1P11,
  PRIV_SPEC_CLASS_DRAFT
};

/* This structure holds all restricted conditions for a CSR.  */

struct riscv_csr_extra
{
  /* Class to which this CSR belongs.  Used to decide whether or
     not this CSR is legal in the current -march context.  */
  enum riscv_csr_class csr_class;

  /* CSR may have differnet numbers in the previous priv spec.  */
  unsigned address;

  /* Record the CSR is defined/valid in which versions.  */
  enum riscv_priv_spec_class define_version;

  /* Record the CSR is aborted/invalid from which versions.  If it isn't
     aborted in the current version, then it should be CSR_CLASS_VDRAFT.  */
  enum riscv_priv_spec_class abort_version;

  /* The CSR may have more than one setting.  */
  struct riscv_csr_extra *next;
};

/* Instruction is a simple alias (e.g. "mv" for "addi").  */
#define	INSN_ALIAS		0x00000001

/* These are for setting insn_info fields.

   Nonbranch is the default.  Noninsn is used only if there is no match.
   There are no condjsr or dref2 instructions.  So that leaves condbranch,
   branch, jsr, and dref that we need to handle here, encoded in 3 bits.  */
#define INSN_TYPE		0x0000000e

/* Instruction is an unconditional branch.  */
#define INSN_BRANCH		0x00000002
/* Instruction is a conditional branch.  */
#define INSN_CONDBRANCH		0x00000004
/* Instruction is a jump to subroutine.  */
#define INSN_JSR		0x00000006
/* Instruction is a data reference.  */
#define INSN_DREF		0x00000008

/* We have 5 data reference sizes, which we can encode in 3 bits.  */
#define INSN_DATA_SIZE		0x00000070
#define INSN_DATA_SIZE_SHIFT	4
#define INSN_1_BYTE		0x00000010
#define INSN_2_BYTE		0x00000020
#define INSN_4_BYTE		0x00000030
#define INSN_8_BYTE		0x00000040
#define INSN_16_BYTE		0x00000050

/* Instruction is an SFPU instruction */
#define INSN_SFPU               0x00000100

/* Instruction is actually a macro.  It should be ignored by the
   disassembler, and requires special treatment by the assembler.  */
#define INSN_MACRO		0xffffffff

/* This is a list of macro expanded instructions.

   _I appended means immediate
   _A appended means address
   _AB appended means address with base register
   _D appended means 64 bit floating point constant
   _S appended means 32 bit floating point constant.  */

enum
{
  M_LA,
  M_LLA,
  M_LA_TLS_GD,
  M_LA_TLS_IE,
  M_LB,
  M_LBU,
  M_LH,
  M_LHU,
  M_LW,
  M_LWU,
  M_LD,
  M_SB,
  M_SH,
  M_SW,
  M_SD,
  M_FLW,
  M_FLD,
  M_FLQ,
  M_FSW,
  M_FSD,
  M_FSQ,
  M_CALL,
  M_J,
  M_LI,
  M_ZEXTH,
  M_ZEXTW,
  M_SEXTB,
  M_SEXTH,
  M_NUM_MACROS
};


extern const char * const riscv_gpr_names_numeric[NGPR];
extern const char * const riscv_gpr_names_abi[NGPR];
extern const char * const riscv_fpr_names_numeric[NFPR];
extern const char * const riscv_fpr_names_abi[NFPR];
extern const char * const riscv_sfpur_names_numeric[NSFPUR];
extern const char * const riscv_sfpur_names_abi[NSFPUR];

extern const struct riscv_opcode riscv_opcodes[];
extern const struct riscv_opcode riscv_insn_types[];
extern const struct riscv_ext_version riscv_ext_version_table[];

extern int
riscv_get_isa_spec_class (const char *, enum riscv_isa_spec_class *);

#endif /* _RISCV_H_ */
