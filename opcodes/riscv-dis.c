/* RISC-V disassembler
   Copyright (C) 2011-2021 Free Software Foundation, Inc.

   Contributed by Andrew Waterman (andrew@sifive.com).
   Based on MIPS target.

   This file is part of the GNU opcodes library.

   This library is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   It is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING3. If not,
   see <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"
#include "disassemble.h"
#include "libiberty.h"
#define IN_DISASSEMBLER
#include "opcode/riscv.h"
#include "opintl.h"
#include "elf-bfd.h"
#include "elf/riscv.h"
#include "elfxx-riscv.h"

#include "bfd_stdint.h"
#include <ctype.h>

static enum riscv_priv_spec_class default_priv_spec = PRIV_SPEC_CLASS_NONE;

struct riscv_private_data
{
  bfd_vma gp;
  bfd_vma print_addr;
  bfd_vma hi_addr[OP_MASK_RD + 1];
};

static const char * const *riscv_gpr_names;
static const char * const *riscv_fpr_names;

/* Other options.  */
static int no_aliases;	/* If set disassemble as most general inst.  */

enum sfpu_mach_type {
  SFPU_MACH_GRAYSKULL = 1,
  SFPU_MACH_WORMHOLE = 2,
  SFPU_MACH_BLACKHOLE = 3
} sfpu_mach;

static void
set_default_riscv_dis_options (void)
{
  riscv_gpr_names = riscv_gpr_names_abi;
  riscv_fpr_names = riscv_fpr_names_abi;
  no_aliases = 0;
}

static bfd_boolean
parse_riscv_dis_option_without_args (const char *option)
{
  if (strcmp (option, "no-aliases") == 0)
    no_aliases = 1;
  else if (strcmp (option, "numeric") == 0)
    {
      riscv_gpr_names = riscv_gpr_names_numeric;
      riscv_fpr_names = riscv_fpr_names_numeric;
    }
  else
    return FALSE;
  return TRUE;
}

static void
parse_riscv_dis_option (const char *option)
{
  char *equal, *value;

  if (parse_riscv_dis_option_without_args (option))
    return;

  equal = strchr (option, '=');
  if (equal == NULL)
    {
      /* The option without '=' should be defined above.  */
      opcodes_error_handler (_("unrecognized disassembler option: %s"), option);
      return;
    }
  if (equal == option
      || *(equal + 1) == '\0')
    {
      /* Invalid options with '=', no option name before '=',
       and no value after '='.  */
      opcodes_error_handler (_("unrecognized disassembler option with '=': %s"),
                            option);
      return;
    }

  *equal = '\0';
  value = equal + 1;
  if (strcmp (option, "priv-spec") == 0)
    {
      enum riscv_priv_spec_class priv_spec = PRIV_SPEC_CLASS_NONE;
      if (!riscv_get_priv_spec_class (value, &priv_spec))
	opcodes_error_handler (_("unknown privilege spec set by %s=%s"),
			       option, value);
      else if (default_priv_spec == PRIV_SPEC_CLASS_NONE)
	default_priv_spec = priv_spec;
      else if (default_priv_spec != priv_spec)
	opcodes_error_handler (_("mis-matched privilege spec set by %s=%s, "
				 "the elf privilege attribute is %s"),
			       option, value,
			       riscv_get_priv_spec_name (default_priv_spec));
    }
  else
    {
      /* xgettext:c-format */
      opcodes_error_handler (_("unrecognized disassembler option: %s"), option);
    }
}

static void
parse_riscv_dis_options (const char *opts_in)
{
  char *opts = xstrdup (opts_in), *opt = opts, *opt_end = opts;

  set_default_riscv_dis_options ();

  for ( ; opt_end != NULL; opt = opt_end + 1)
    {
      if ((opt_end = strchr (opt, ',')) != NULL)
	*opt_end = 0;
      parse_riscv_dis_option (opt);
    }

  free (opts);
}

/* Print one argument from an array.  */

static void
arg_print (struct disassemble_info *info, unsigned long val,
	   const char* const* array, size_t size)
{
  const char *s = val >= size || array[val] == NULL ? "unknown" : array[val];
  (*info->fprintf_func) (info->stream, "%s", s);
}

static void
maybe_print_address (struct riscv_private_data *pd, int base_reg, int offset)
{
  if (pd->hi_addr[base_reg] != (bfd_vma)-1)
    {
      pd->print_addr = (base_reg != 0 ? pd->hi_addr[base_reg] : 0) + offset;
      pd->hi_addr[base_reg] = -1;
    }
  else if (base_reg == X_GP && pd->gp != (bfd_vma)-1)
    pd->print_addr = pd->gp + offset;
  else if (base_reg == X_TP || base_reg == 0)
    pd->print_addr = offset;
}

/* Print insn arguments for 32/64-bit code.  */

static void
print_insn_args (const char *d, insn_t l, bfd_vma pc, disassemble_info *info)
{
  struct riscv_private_data *pd = info->private_data;
  int rs1 = (l >> OP_SH_RS1) & OP_MASK_RS1;
  int rd = (l >> OP_SH_RD) & OP_MASK_RD;
  fprintf_ftype print = info->fprintf_func;

  if (*d != '\0')
    print (info->stream, "\t");

  for (; *d != '\0'; d++)
    {
      switch (*d)
	{
	case 'C': /* RVC */
	  switch (*++d)
	    {
	    case 's': /* RS1 x8-x15 */
	    case 'w': /* RS1 x8-x15 */
	      print (info->stream, "%s",
		     riscv_gpr_names[EXTRACT_OPERAND (CRS1S, l) + 8]);
	      break;
	    case 't': /* RS2 x8-x15 */
	    case 'x': /* RS2 x8-x15 */
	      print (info->stream, "%s",
		     riscv_gpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
	      break;
	    case 'U': /* RS1, constrained to equal RD */
	      print (info->stream, "%s", riscv_gpr_names[rd]);
	      break;
	    case 'c': /* RS1, constrained to equal sp */
	      print (info->stream, "%s", riscv_gpr_names[X_SP]);
	      break;
	    case 'V': /* RS2 */
	      print (info->stream, "%s",
		     riscv_gpr_names[EXTRACT_OPERAND (CRS2, l)]);
	      break;
	    case 'i':
	      print (info->stream, "%d", (int)EXTRACT_RVC_SIMM3 (l));
	      break;
	    case 'o':
	    case 'j':
	      print (info->stream, "%d", (int)EXTRACT_RVC_IMM (l));
	      break;
	    case 'k':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LW_IMM (l));
	      break;
	    case 'l':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LD_IMM (l));
	      break;
	    case 'm':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LWSP_IMM (l));
	      break;
	    case 'n':
	      print (info->stream, "%d", (int)EXTRACT_RVC_LDSP_IMM (l));
	      break;
	    case 'K':
	      print (info->stream, "%d", (int)EXTRACT_RVC_ADDI4SPN_IMM (l));
	      break;
	    case 'L':
	      print (info->stream, "%d", (int)EXTRACT_RVC_ADDI16SP_IMM (l));
	      break;
	    case 'M':
	      print (info->stream, "%d", (int)EXTRACT_RVC_SWSP_IMM (l));
	      break;
	    case 'N':
	      print (info->stream, "%d", (int)EXTRACT_RVC_SDSP_IMM (l));
	      break;
	    case 'p':
	      info->target = EXTRACT_RVC_B_IMM (l) + pc;
	      (*info->print_address_func) (info->target, info);
	      break;
	    case 'a':
	      info->target = EXTRACT_RVC_J_IMM (l) + pc;
	      (*info->print_address_func) (info->target, info);
	      break;
	    case 'u':
	      print (info->stream, "0x%x",
		     (int)(EXTRACT_RVC_IMM (l) & (RISCV_BIGIMM_REACH-1)));
	      break;
	    case '>':
	      print (info->stream, "0x%x", (int)EXTRACT_RVC_IMM (l) & 0x3f);
	      break;
	    case '<':
	      print (info->stream, "0x%x", (int)EXTRACT_RVC_IMM (l) & 0x1f);
	      break;
	    case 'T': /* floating-point RS2 */
	      print (info->stream, "%s",
		     riscv_fpr_names[EXTRACT_OPERAND (CRS2, l)]);
	      break;
	    case 'D': /* floating-point RS2 x8-x15 */
	      print (info->stream, "%s",
		     riscv_fpr_names[EXTRACT_OPERAND (CRS2S, l) + 8]);
	      break;
	    }
	  break;

	case ',':
	  print (info->stream, "%c ", *d);
	  break;

	case '(':
	case ')':
	case '[':
	case ']':
	  print (info->stream, "%c", *d);
	  break;

	case '0':
	  /* Only print constant 0 if it is the last argument */
	  if (!d[1])
	    print (info->stream, "0");
	  break;

	case 'b':
	case 's':
	  if ((l & MASK_JALR) == MATCH_JALR)
	    maybe_print_address (pd, rs1, 0);
	  print (info->stream, "%s", riscv_gpr_names[rs1]);
	  break;

	case 't':
	  print (info->stream, "%s",
		 riscv_gpr_names[EXTRACT_OPERAND (RS2, l)]);
	  break;

	case 'u':
	  print (info->stream, "0x%x",
		 (unsigned)EXTRACT_UTYPE_IMM (l) >> RISCV_IMM_BITS);
	  break;

	case 'm':
	  arg_print (info, EXTRACT_OPERAND (RM, l),
		     riscv_rm, ARRAY_SIZE (riscv_rm));
	  break;

	case 'P':
	  arg_print (info, EXTRACT_OPERAND (PRED, l),
		     riscv_pred_succ, ARRAY_SIZE (riscv_pred_succ));
	  break;

	case 'Q':
	  arg_print (info, EXTRACT_OPERAND (SUCC, l),
		     riscv_pred_succ, ARRAY_SIZE (riscv_pred_succ));
	  break;

	case 'o':
	  maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l));
	  /* Fall through.  */
	case 'j':
	  if (((l & MASK_ADDI) == MATCH_ADDI && rs1 != 0)
	      || (l & MASK_JALR) == MATCH_JALR)
	    maybe_print_address (pd, rs1, EXTRACT_ITYPE_IMM (l));
	  print (info->stream, "%d", (int)EXTRACT_ITYPE_IMM (l));
	  break;

	case 'q':
	  maybe_print_address (pd, rs1, EXTRACT_STYPE_IMM (l));
	  print (info->stream, "%d", (int)EXTRACT_STYPE_IMM (l));
	  break;

	case 'a':
	  info->target = EXTRACT_UJTYPE_IMM (l) + pc;
	  (*info->print_address_func) (info->target, info);
	  break;

	case 'p':
	  info->target = EXTRACT_SBTYPE_IMM (l) + pc;
	  (*info->print_address_func) (info->target, info);
	  break;

	case 'd':
	  if ((l & MASK_AUIPC) == MATCH_AUIPC)
	    pd->hi_addr[rd] = pc + EXTRACT_UTYPE_IMM (l);
	  else if ((l & MASK_LUI) == MATCH_LUI)
	    pd->hi_addr[rd] = EXTRACT_UTYPE_IMM (l);
	  else if ((l & MASK_C_LUI) == MATCH_C_LUI)
	    pd->hi_addr[rd] = EXTRACT_RVC_LUI_IMM (l);
	  print (info->stream, "%s", riscv_gpr_names[rd]);
	  break;

	case 'z':
	  print (info->stream, "%s", riscv_gpr_names[0]);
	  break;

	case '>':
	  print (info->stream, "0x%x", (int)EXTRACT_OPERAND (SHAMT, l));
	  break;

	case '<':
	  print (info->stream, "0x%x", (int)EXTRACT_OPERAND (SHAMTW, l));
	  break;

	case 'S':
	case 'U':
	  print (info->stream, "%s", riscv_fpr_names[rs1]);
	  break;

	case 'T':
	  print (info->stream, "%s", riscv_fpr_names[EXTRACT_OPERAND (RS2, l)]);
	  break;

	case 'D':
	  print (info->stream, "%s", riscv_fpr_names[rd]);
	  break;

	case 'R':
	  print (info->stream, "%s", riscv_fpr_names[EXTRACT_OPERAND (RS3, l)]);
	  break;

	case 'E':
	  {
	    static const char *riscv_csr_hash[4096];    /* Total 2^12 CSR.  */
	    static bfd_boolean init_csr = FALSE;
	    unsigned int csr = EXTRACT_OPERAND (CSR, l);

	    if (!init_csr)
	      {
		unsigned int i;
		for (i = 0; i < 4096; i++)
		  riscv_csr_hash[i] = NULL;

		/* Set to the newest privilege version.  */
		if (default_priv_spec == PRIV_SPEC_CLASS_NONE)
		  default_priv_spec = PRIV_SPEC_CLASS_DRAFT - 1;

#define DECLARE_CSR(name, num, class, define_version, abort_version)	\
		if (riscv_csr_hash[num] == NULL 			\
		    && ((define_version == PRIV_SPEC_CLASS_NONE 	\
			 && abort_version == PRIV_SPEC_CLASS_NONE)	\
			|| (default_priv_spec >= define_version 	\
			    && default_priv_spec < abort_version)))	\
		  riscv_csr_hash[num] = #name;
#define DECLARE_CSR_ALIAS(name, num, class, define_version, abort_version) \
		DECLARE_CSR (name, num, class, define_version, abort_version)
#include "opcode/riscv-opc.h"
#undef DECLARE_CSR
	      }

	    if (riscv_csr_hash[csr] != NULL)
	      print (info->stream, "%s", riscv_csr_hash[csr]);
	    else
	      print (info->stream, "0x%x", csr);
	    break;
	  }

	case 'w':
	case 'y':
	case 'l':  
	  {
	      switch (*++d)
		{
		case 'a': /* MUL/ADD SRCA L0-L15 */
	          print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YMULADD_SRCA, l)]);
		  break;
		case 'b': /* MUL/ADD SRCB L0-L15 */
	          print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YMULADD_SRCB, l)]);
		  break;
		case 'c': /* MUL/ADD SRCC L0-L15 */
	          print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YMULADD_SRCC, l)]);
		  break;
		case 'd': /* LOAD/STORE RD L0-L3 */
	          print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YLOADSTORE_RD, l)]);
		  ++d;
		  break;
		case 'e': /* MUL/ADD DEST L0-L3 */
	          print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YMULADD_DEST, l)]);
		  break;
		case 'f': /* imm12_math */
	          print (info->stream, "0x%03lX", EXTRACT_OPERAND (YCC_IMM12_MATH, l));
		  break;
		case 'g': /* CC Instructions LREG_C L0-L15 */
	          print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YCC_LREG_C, l)]);
		  break;
		case 'h': /* CC Instructions LREG_DEST L0-L3 */
		  if (info->mach == bfd_mach_riscv32_sfpu_wormhole  &&
		      EXTRACT_OPERAND (SFPU_OP, l) == 0x91 /* SFPCONFIG */)
		    print (info->stream, "%lu", EXTRACT_OPERAND (YCC_LREG_DEST, l));
		  else if (info->mach == bfd_mach_riscv32_sfpu_blackhole  &&
                       EXTRACT_OPERAND (SFPU_OP, l) == 0x91 /* SFPCONFIG */)
                     print (info->stream, "%lu", EXTRACT_OPERAND (YCC_LREG_DEST, l));
		  else
		    print (info->stream, "%s", riscv_sfpur_names_abi[EXTRACT_OPERAND (YCC_LREG_DEST, l)]);
		  break;
		case 'i': /* CC Instructions instr_mod1 */
		  {
		    char x = *++d;
		    if (x == '1')
		      print (info->stream, "%ld", EXTRACT_OPERAND (YCC_INSTR_MOD1, l));
		    else if (x == '2')
		      {
			switch (EXTRACT_OPERAND (SFPU_OP, l)) {
			  case 0x76:  /* SFPDIVP2 */
			  case 0x7A:  /* SFPSHFT */
			    print (info->stream, "%d", ((short)EXTRACT_OPERAND (YCC_INSTR_MOD1, l)));
			    break;
			  case 0x82:  /* SFPSETEXP */
			  case 0x83:  /* SFPSETMAN */
			    print (info->stream, "%u", ((unsigned short)EXTRACT_OPERAND (YCC_INSTR_MOD1, l)));
			    break;
			  default:
			    print (info->stream, "%u", ((unsigned short)EXTRACT_OPERAND (YCC_INSTR_MOD1, l)));
			    break;
			}
		      }
		    else if (x == '5')
		      {
			switch (EXTRACT_OPERAND (SFPU_OP, l)) {
			  case 0x79:
			    print (info->stream, "%d", ((short)EXTRACT_OPERAND (YCC_INSTR_MOD1, l)));
			    break;
			  default:
			    print (info->stream, "%u", ((unsigned short)EXTRACT_OPERAND (YCC_INSTR_MOD1, l)));
			    break;
			}
		      }
		    else
		      print (info->stream, "%u", ((unsigned short)EXTRACT_OPERAND (YCC_INSTR_MOD1, l)));
		  }
		  break;
		case 'j': /* imm16_math */
	          print (info->stream, "%d", ((short)EXTRACT_OPERAND (YMULI_IMM16_MATH, l)));
		  break;
		case 'k': /* Wormhole INCRWC operands */
		  {
		    char x = *++d;
		    if (x == '1')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WINCRWC_RWC_CR, l));
		    else if (x == '2')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WINCRWC_RWC_D, l));
		    else if (x == '3')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WINCRWC_RWC_B, l));
		    else if (x == '4')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WINCRWC_RWC_A, l));
		    else
		      print (info->stream, _("# internal error, undefined modifier (k%c)"), x);
		  }
		  break;
		case 'l': /* Wormhole REPLAY operands */
		  {
		    char x = *++d;
		    if (x == '1')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WREPLAY_START_IDX, l));
		    else if (x == '2')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WREPLAY_LEN, l));
		    else if (x == '3')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WREPLAY_EXEC_WHILE_LOAD, l));
		    else if (x == '4')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WREPLAY_LOAD_MODE, l));
		    else
		      print (info->stream, _("# internal error, undefined modifier (l%c)"), x);
		  }
		  break;
		case 'm': /* load/store instr_mod0 */
	          print (info->stream, "%ld", EXTRACT_OPERAND (YLOADSTORE_INSTR_MOD0, l));
		  ++d;
		  break;
		case 'n': /* dest_reg_addr */
	          print (info->stream, "%d", ((short)EXTRACT_OPERAND (YDEST_REG_ADDR, l)));
		  break;
		case 'o': /* mul/add instr_mod0 */
	          print (info->stream, "%ld", EXTRACT_OPERAND (YMULADD_INSTR_MOD0, l));
		  break;
		case 'p': /* wormhole load/store addr_mode */
	          print (info->stream, "%ld", EXTRACT_OPERAND (WLOADSTORE_ADDR_MODE, l));
		  break;
		case 'q': /* wormhole dest_reg_addr */
	          print (info->stream, "%d", ((short)EXTRACT_OPERAND (WLOADSTORE_DEST_REG_ADDR, l)));
		  break;
		case 'r': /* Wormhole STOCH_RND operands */
		  {
		    char x = *++d;
		    if (x == '1')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WSTOCH_RND_MODE, l));
		    else if (x == '2')
		      print (info->stream, "%ld", EXTRACT_OPERAND (WSTOCH_RND_IMM8_MATH, l));
		    else
		      print (info->stream, _("# internal error, undefined modifier (r%c)"), x);
		  }
		  break;
		case 's':
		  {
		    char x = *++d;
		    switch (x)
		      {
		      case '1':
			print (info->stream, "%ld", EXTRACT_OPERAND (DMA_REG_OP_A, l)); break;
		      case '2':
			print (info->stream, "%ld", EXTRACT_OPERAND (DMA_REG_OP_B, l)); break;
		      case '3':
			print (info->stream, "%ld", EXTRACT_OPERAND (DMA_REG_OP_RES, l)); break;
		      case '4':
			print (info->stream, "%ld", EXTRACT_OPERAND (DMA_REG_OP_B_ISCONST, l)); break;
		      case '5':
			print (info->stream, "%ld", EXTRACT_OPERAND (DMA_REG_OP_OPSEL, l)); break;
		      case '6':
			print (info->stream, "%ld", EXTRACT_OPERAND (PAYLOAD_SIGSEL_SIZE, l)); break;
		      case '7':
			print (info->stream, "%ld", EXTRACT_OPERAND (PAYLOAD_SIGSEL, l)); break;
		      case '8':
			print (info->stream, "%ld", EXTRACT_OPERAND (SET_SIG_MODE, l)); break;
		      case '9':
			print (info->stream, "%ld", EXTRACT_OPERAND (REG_INDEX16B, l)); break;
		      case 'a':
			print (info->stream, "%ld", EXTRACT_OPERAND (CNT_SET_MASK, l)); break;
		      case 'b':
			print (info->stream, "%ld", EXTRACT_OPERAND (CH1_Y, l)); break;
		      case 'c':
			print (info->stream, "%ld", EXTRACT_OPERAND (CH1_X, l)); break;
		      case 'd':
			print (info->stream, "%ld", EXTRACT_OPERAND (CH0_Y, l)); break;
		      case 'e':
			print (info->stream, "%ld", EXTRACT_OPERAND (CH0_X, l)); break;
		      case 'f':
			print (info->stream, "%ld", EXTRACT_OPERAND (BIT_MASK, l)); break;
		      case 'g':
			print (info->stream, "%ld", EXTRACT_OPERAND (WAIT_RES_14BIT, l)); break;
		      case 'h':
			print (info->stream, "%ld", EXTRACT_OPERAND (ADDR_MODE, l)); break;
		      case 'i':
			print (info->stream, "%ld", EXTRACT_OPERAND (INDEX_EN, l)); break;
		      case 'j':
			print (info->stream, "%ld", EXTRACT_OPERAND (DST, l)); break;
		      case 'k':
			print (info->stream, "%ld", EXTRACT_OPERAND (MEMHIER_SEL, l)); break;
		      case 'l':
			print (info->stream, "%ld", EXTRACT_OPERAND (SWAP_OR_INCR_VAL, l)); break;
		      case 'm':
			print (info->stream, "%ld", EXTRACT_OPERAND (WRAP_VAL_OR_SWAP_MASK, l)); break;
		      case 'n':
			print (info->stream, "%ld", EXTRACT_OPERAND (SEL32B, l)); break;
		      case 'o':
			print (info->stream, "%ld", EXTRACT_OPERAND (DATA_REG_IDX, l)); break;
		      case 'p':
			print (info->stream, "%ld", EXTRACT_OPERAND (ADDR_REG_IDX, l)); break;
		      case 'q':
                        print (info->stream, "%ld", EXTRACT_OPERAND (NO_INCR, l)); break;
                      case 'r':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MUTEX_IDX, l)); break;
		      case 's':
			print (info->stream, "%ld", EXTRACT_OPERAND (CLEAR_DVALID, l)); break;
		      case 't':
			print (info->stream, "%ld", EXTRACT_OPERAND (RESET, l)); break;
		      case 'u':
			print (info->stream, "%ld", EXTRACT_OPERAND (ROTATE_WEIGHTS, l)); break;
		      case 'v':
			print (info->stream, "%ld", EXTRACT_OPERAND (ADDRMODE, l)); break;
		      case 'w':
			print (info->stream, "%ld", EXTRACT_OPERAND (DST_15BIT, l)); break;
                      case 'x':
                        print (info->stream, "%ld", EXTRACT_OPERAND (DEST_ACCUM_EN, l)); break;
                      case 'y':
                        print (info->stream, "%ld", EXTRACT_OPERAND (INSTR_MOD19, l)); break;
                      case 'z':
                        print (info->stream, "%ld", EXTRACT_OPERAND (EL_ADDRMODE, l)); break;
		      default:
			print (info->stream, _("# internal error, undefined modifier (ys%c)"), x);
			break;
		      }
		      break;
		  }
		case 't':
		  {
		    char x = *++d;
		    switch (x)
		      {
		      case '0':
			print (info->stream, "%ld", EXTRACT_OPERAND (FLUSH_SPEC, l)); break;
		      case '1':
			print (info->stream, "%ld", EXTRACT_OPERAND (INSTRMOD19, l)); break;
		      case '2':
			print (info->stream, "%ld", EXTRACT_OPERAND (MAX_POOL_INDEX_EN, l)); break;
		      case '3':
			print (info->stream, "%ld", EXTRACT_OPERAND (RESET_SRCB_GATE_CONTROL, l));
			break;
		      case '4':
			print (info->stream, "%ld", EXTRACT_OPERAND (RESET_SRCA_GATE_CONTROL, l));
			break;
		      case '5':
			print (info->stream, "%ld", EXTRACT_OPERAND (RWC_CR, l)); break;
		      case '6':
			print (info->stream, "%ld", EXTRACT_OPERAND (RWC_D, l)); break;
		      case '7':
			print (info->stream, "%ld", EXTRACT_OPERAND (RWC_B, l)); break;
		      case '8':
			print (info->stream, "%ld", EXTRACT_OPERAND (RWC_A, l)); break;
		      case '9':
			print (info->stream, "%ld", EXTRACT_OPERAND (SIZE_SEL, l)); break;
		      case 'a':
			print (info->stream, "%ld", EXTRACT_OPERAND (OFFSET_INDEX, l)); break;
		      case 'b':
                        print (info->stream, "%ld", EXTRACT_OPERAND (AUTO_INC_SPEC, l)); break;
		      case 'c':
			print (info->stream, "%ld", EXTRACT_OPERAND (TDMA_DATA_REG_IDX, l)); break;
		      case 'd':
			print (info->stream, "%ld", EXTRACT_OPERAND (REGADDR, l)); break;
		      case 'e':
			print (info->stream, "%ld", EXTRACT_OPERAND (ROTATEWEIGHTS, l)); break;
		      case 'f':
			print (info->stream, "%ld", EXTRACT_OPERAND (MOP_TYPE, l)); break;
		      case 'g':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LOOP_COUNT, l)); break;
                      case 'h':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ZMASK_LO16, l)); break;
                      case 'i':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ZMASK_HI16, l)); break;
                      case 'j':
                        print (info->stream, "%ld", EXTRACT_OPERAND (DEST_32B_LO, l)); break;
                      case 'k':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SRC, l)); break;
                      case 'l':
                        print (info->stream, "%ld", EXTRACT_OPERAND (INSTRMODE, l)); break;
                      case 'm':
                        print (info->stream, "%ld", EXTRACT_OPERAND (DST_MOV, l)); break;
                      case 'n':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SRCA, l)); break;
                      case 'o':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SRCB, l)); break;
                      case 'p':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ADDRMODE_PACR, l)); break;
                      case 'q':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ZERO_WRITE, l)); break;
                      case 'r':
                        print (info->stream, "%ld", EXTRACT_OPERAND (OVRD_TREAD_ID, l)); break;
                      case 's':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CONCAT, l)); break;
                      case 't':
                        print (info->stream, "%ld", EXTRACT_OPERAND (FLUSH, l)); break;
                      case 'u':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LAST, l)); break;
                      case 'v':
                        print (info->stream, "%ld", EXTRACT_OPERAND (PUSH, l)); break;
                      case 'w':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ADDR_SEL, l)); break;
                      case 'x':
                        print (info->stream, "%ld", EXTRACT_OPERAND (WR_DATA, l)); break;
                      case 'y':
                        print (info->stream, "%ld", EXTRACT_OPERAND (PACK_SEL, l)); break;
                      case 'z':
                        print (info->stream, "%ld", EXTRACT_OPERAND (STREAM_ID, l)); break;
		      default:
			print (info->stream, _("# internal error, undefined modifier (yt%c)"), x); 
			break;
		      }
		      break;
		  }
                case 'u': /* */
                  {
                    char x = *++d;
                    switch (x)
                      {
                      case '0':
                        print (info->stream, "%ld", EXTRACT_OPERAND (FLUSH_SET, l)); break;
                      case '1':
                        print (info->stream, "%ld", EXTRACT_OPERAND (GPR_ADDRESS, l)); break;
                      case '2':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CFG_REG, l)); break;
                      case '3':
                        print (info->stream, "%ld", EXTRACT_OPERAND (TARGET_SEL, l)); break;
                      case '4':
                        print (info->stream, "%ld", EXTRACT_OPERAND (BYTE_OFFSET, l)); break;
                      case '5':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CONTEXTID_2, l)); break;
                      case '6':
                        print (info->stream, "%ld", EXTRACT_OPERAND (FLOP_INDEX, l)); break;
                      case '7':
                        print (info->stream, "%ld", EXTRACT_OPERAND (REG_INDEX, l)); break;
                      case '8':
                        print (info->stream, "%ld", EXTRACT_OPERAND (START_IDX, l)); break;
                      case '9':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LEN, l)); break;
                      case 'a':
                        print (info->stream, "%ld", EXTRACT_OPERAND (EXECUTE_WHILE_LOADING, l));
			break;
                      case 'b':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LOAD_MODE, l)); break;
                      case 'c':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MASK, l)); break;
                      case 'd':
                        print (info->stream, "%ld", EXTRACT_OPERAND (DATA, l)); break;
                      case 'e':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CFG_REG_ADDR, l)); break;
                      case 'f':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SEM_SEL, l)); break;
                      case 'g':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MAX_VALUE, l)); break;
                      case 'h':
                        print (info->stream, "%ld", EXTRACT_OPERAND (INIT_VALUE, l)); break;
                      case 'i':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SEMSEL_SEMINIT, l)); break;
                      case 'j':
                        print (info->stream, "%ld", EXTRACT_OPERAND (STALL_RES, l)); break;
                      case 'k':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SEMSEL_SEMWAIT, l)); break;
                      case 'l':
                        print (info->stream, "%ld", EXTRACT_OPERAND (WAIT_SEM_COND, l)); break;
                      case 'm':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CHANNEL_INDEX, l)); break;
                      case 'n':
                        print (info->stream, "%ld", EXTRACT_OPERAND (DIMENSIONINDEX, l)); break;
                      case 'o':
                        print (info->stream, "%ld", EXTRACT_OPERAND (VALUE, l)); break;
                      case 'p':
                        print (info->stream, "%ld", EXTRACT_OPERAND (X_END2, l)); break;
                      case 'q':
                        print (info->stream, "%ld", EXTRACT_OPERAND (X_START, l)); break;
                      case 'r':
                        print (info->stream, "%ld", EXTRACT_OPERAND (BITMASK, l)); break;
                      case 's':
                        print (info->stream, "%ld", EXTRACT_OPERAND (REGMASK, l)); break;
                      case 't':
                        print (info->stream, "%ld", EXTRACT_OPERAND (HALO_MASK, l)); break;
                      case 'u':
                        print (info->stream, "%ld", EXTRACT_OPERAND (REG_MASK_2, l)); break;
                      case 'v':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SETC16_REG, l)); break;
                      case 'w':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SETC16_VALUE, l)); break;
                      case 'x':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SETVALID, l)); break;
                      case 'y':
                        print (info->stream, "%ld", EXTRACT_OPERAND (RWC_BIAS, l)); break;
                      case 'z':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SET_INC_CTRL, l)); break;
                      default:
                        print (info->stream, _("# internal error, undefined modifier (l%c)"), x); 
			break;
                      }
		      break;
                  }
                case 'v':
                  {
                    char x = *++d;
                    switch (x)
                      {
                      case '0':
                        print (info->stream, "%ld", EXTRACT_OPERAND (Y_END, l)); break;
                      case '1':
                        print (info->stream, "%ld", EXTRACT_OPERAND (Y_START, l)); break;
                      case '2':
                        print (info->stream, "%ld", EXTRACT_OPERAND (X_END, l)); break;
                      case '3':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CLEAR_AB_VLD, l)); break;
		      case '4':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LREG_IND, l)); break;
                      case '5':
                        print (info->stream, "%ld", EXTRACT_OPERAND (INSTR_MOD0, l)); break;
                      case '6':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SFPU_ADDR_MODE, l)); break;
                      case '7':
                        print (info->stream, "%ld", EXTRACT_OPERAND (DEST_REG_ADDR, l)); break;

                      case '8':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LOG2_AMOUNT2, l)); break;
                      case '9':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SHIFT_MODE, l)); break;
                      case 'a':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ROT_SHIFT, l));
                        break;
                      case 'b':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SHIFT_ROW, l)); break;
                      case 'c':
                        print (info->stream, "%ld", EXTRACT_OPERAND (WAIT_RES, l)); break;
                      case 'd':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SIZESEL, l)); break;
                      case 'e':
                        print (info->stream, "%ld", EXTRACT_OPERAND (REGSIZESEL, l)); break;
                      case 'f':
                        print (info->stream, "%ld", EXTRACT_OPERAND (OFFSETINDEX, l)); break;
                      case 'g':
                        print (info->stream, "%ld", EXTRACT_OPERAND (UNPACK_BLOCK_SELECTION, l));
			break;
                      case 'h':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ADDRMODE_UNPACROP, l)); break;
                      case 'i':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CFGCONTEXTCNTINC, l)); break;
                      case 'j':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CFGCONTEXTID, l)); break;
                      case 'k':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ADDRCNTCONTEXTID, l)); break;
                      case 'l':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SETDATVALID, l)); break;
                      case 'm':
                        print (info->stream, "%ld", EXTRACT_OPERAND (RAREB_EN, l)); break;
                      case 'n':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ZEROWRITE2, l)); break;
                      case 'o':
                        print (info->stream, "%ld", EXTRACT_OPERAND (AUTOINCCONTEXTID, l)); break;
                      case 'p':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ROWSEARCH, l)); break;
                      case 'q':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SEARCHCASHFLOW, l)); break;
                      case 'r':
                        print (info->stream, "%ld", EXTRACT_OPERAND (NOOP, l)); break;
                      case 's':
                        print (info->stream, "%ld", EXTRACT_OPERAND (WR128B, l)); break;
                      case 't':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CFGREG, l)); break;
                      case 'u':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MOV_BLOCK_SELECTION, l)); break;
                      case 'v':
                        print (info->stream, "%ld", EXTRACT_OPERAND (LAST_XMOVOP, l)); break;
                      case 'w':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CLEARCODE, l)); break;
                      case 'x':
                        print (info->stream, "%ld", EXTRACT_OPERAND (ZEROVAL, l)); break;
                      case 'y':
                        print (info->stream, "%ld", EXTRACT_OPERAND (WRITEMODE, l)); break;
                      case 'z':
                        print (info->stream, "%ld", EXTRACT_OPERAND (BANKMASK, l)); break;
                      default:
                        print (info->stream, _("# internal error, undefined modifier (l%c)"), x);
                        break;
		      }
		    break;
		  }
                case 'w':
                  {
                    char x = *++d;
                    switch (x)
                      {
                      case '0':
                        print (info->stream, "%ld", EXTRACT_OPERAND (SRCMASK, l)); break;
		      case '1':
                        print (info->stream, "%ld", EXTRACT_OPERAND (CMP_VAL, l)); break;
		      case '2':
			print (info->stream, "%ld", EXTRACT_OPERAND (PKEDG_X_START, l)); break;
		      case '3':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MOV_INSTR_MOD, l)); break;
		      case '4':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MOV_SRC, l)); break;
		      case '5':
                        print (info->stream, "%ld", EXTRACT_OPERAND (MOV_DST, l)); break;
		      case '6':
                        print (info->stream, "%ld", EXTRACT_OPERAND (GRAY_STALL_RES, l)); break;
                      case '7':
                        print (info->stream, "%ld", EXTRACT_OPERAND (GRAY_SEM_SEL, l)); break;
		      case '8':
                        print (info->stream, "%ld", EXTRACT_OPERAND (POOL_ADDR_MODE, l)); break;
		      }
		  }
		  break;
		  case 'x': //Blackhole
                   {
                    char x = *++d;
                    switch (x)
                      {
			case 'a':x = *++d;
                                switch (x)
                                {
				case '0':
                                       	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_MODE_2, l)); break;
				case '2':
                                	print (info->stream, "%ld", EXTRACT_OPERAND (LSFPU_ADDR_MODE, l)); break;
				case '3':
                                	print (info->stream, "%ld", EXTRACT_OPERAND (LDEST_REG_ADDR, l)); break;
				case '4':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_MODE, l)); break;
				case '5':
                                      	print (info->stream, "%ld", EXTRACT_OPERAND (DEST_REG_ADDR, l)); break;
				case '6':
                                       	print (info->stream, "%ld", EXTRACT_OPERAND (INSTRMODE, l)); break;
				case '7':
                                      	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_MODE, l)); break;
				case '8':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (INSTR_MOD19, l)); break;
				case 'a':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_DEST_2, l)); break;
                              	case 'b':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_CFG_CONTEXT, l)); break;
                            	case 'c':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_ROW_PAD_ZERO, l)); break;
                            	case 'd':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_DEST_ACCESS_MODE, l)); break;
                             	case 'e':
                                      	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_MODE_3, l)); break;
                            	case 'f':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_CNT_CONTEXT, l)); break;
                            	case 'g':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_ZERO_WRITE, l)); break;
                             	case 'h':
                                       	print (info->stream, "%ld", EXTRACT_OPERAND (L_READ_INTF_SEL, l)); break;
                            	case 'i':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_OVERTHREAD_ID, l)); break;
                            	case 'j':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (CONCAT, l)); break;
                              	case 'k':
                                  	print (info->stream, "%ld", EXTRACT_OPERAND (L_CTXT_CTRL, l)); break;
                              	case 'l':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (FLUSH_SET, l)); break;
                            	case 'm':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (LAST, l)); break;
                            	case 'n':
                                      	print (info->stream, "%ld", EXTRACT_OPERAND (PUSH, l)); break;
                            	case 'o':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (ADDR_SEL, l)); break;
                            	case 'p':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_UNUSED, l)); break;
                            	case 'q':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (L_DISABLE_STALL, l)); break;
                             	case 'r':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_SEL, l)); break;
                           	case 's':
                             		print (info->stream, "%ld", EXTRACT_OPERAND (L_STREAM_ID, l)); break;
				case 't':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDRMODE_3, l)); break;
				case 'u':
                                       	print (info->stream, "%ld", EXTRACT_OPERAND (L_ROT_SHIFT, l)); break;
				case 'v':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_DISABLE_MASK_OLD_VALUE, l)); break;
                                case 'w':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_OPERATION, l)); break;
                                case 'x':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_MASK_WIDTH, l)); break;
                                case 'y':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_RIGHT_CSHIFT_AMT, l)); break;
                                case 'z':
					print (info->stream, "%ld", EXTRACT_OPERAND (L_SCRATCH_SEL, l)); break;
				}break;
			case 'b':x = *++d;
                                switch (x)
                                {
				case '0':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_TARGET_SEL, l)); break;
                                case '1':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_WAIT_STREAM_SEL, l)); break;
                                case '2':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_STREAM_ID_SEL, l)); break;
                                case '3':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_STREAM_REG_ADDR, l)); break;
                                case '4':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_CFG_REG_2, l)); break;
				case 'a':
                                       	print (info->stream, "%ld", EXTRACT_OPERAND (UNPACK_BLOCK_SELECTION, l)); break;
                             	case 'b':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (ADDRMODE_UNPACROP, l)); break;
                             	case 'c':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (L_CFG_CONTEXT_CNT_INC, l)); break;
                               	case 'd':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (CFGCONTEXTID, l)); break;
                            	case 'e':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_CNT_CONTEXT_ID, l)); break;
                           	case 'f':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (OVRD_TREAD_ID, l)); break;
                             	case 'g':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (SETDATVALID, l)); break;
                             	case 'h':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_SRC_BCAST, l)); break;
                            	case 'i':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_ZERO_WRITE_2, l)); break;
                             	case 'j':
                                  	print (info->stream, "%ld", EXTRACT_OPERAND (AUTOINCCONTEXTID, l)); break;
				case 'k':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (ROWSEARCH, l)); break;
                           	case 'l':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (SEARCHCASHFLOW, l)); break;
                              	case 'm':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (UNPACK_BLOCK_SELECTION, l)); break;
                            	case 'n':
                                 	print (info->stream, "%ld", EXTRACT_OPERAND (L_STREAM_ID_2, l)); break;
                              	case 'o':
                                  	print (info->stream, "%ld", EXTRACT_OPERAND (L_MSG_CLR_CNT, l)); break;
                         	case 'p':
                                     	print (info->stream, "%ld", EXTRACT_OPERAND (L_SETDVALID, l)); break;
                            	case 'q':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_CLR_TO_FMT_CNTRL, l)); break;
                               	case 'r':
                                 	print (info->stream, "%ld", EXTRACT_OPERAND (L_STALL_CLR_CNTRL, l)); break;
                            	case 's':
                                   	print (info->stream, "%ld", EXTRACT_OPERAND (L_BANK_CLR_CNTRL, l)); break;
                            	case 't':
                                    	print (info->stream, "%ld", EXTRACT_OPERAND (L_SRC_CLR_CNTRL, l)); break;
                            	case 'u':
                                  	print (info->stream, "%ld", EXTRACT_OPERAND (L_UNPACK_POP, l)); break;
				case 'v':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_CFG_REG, l)); break;
                                case 'w':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_LINGER_TIME, l)); break;
                                case 'x':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_RESOURCE, l)); break;
                                case 'y':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_OP_CLASS, l)); break;
                                case 'z':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_TARGET_VALUE, l)); break;
                                }break;
      case 'c':x = *++d;
                                switch (x)
                                {
        case '0':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_32BIT_MODE, l)); break;
                                case '1':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_CLR_ZERO_FLAGS, l)); break;
                                case '2':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_ADDR_MODE_4, l)); break;
                                case '3':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_WHERE, l)); break;
                                case '4':
                                        print (info->stream, "%ld", EXTRACT_OPERAND (L_INSTRMODE, l)); break;

                               }break;
		      }
		   }
                  break;
		}
	    }
	  break;

	case 'Z':
	  print (info->stream, "%d", rs1);
	  break;

	default:
	  /* xgettext:c-format */
	  print (info->stream, _("# internal error, undefined modifier (%c)"),
		 *d);
	  return;
	}
    }
}

/* Print the RISC-V instruction at address MEMADDR in debugged memory,
   on using INFO.  Returns length of the instruction, in bytes.
   BIGENDIAN must be 1 if this is big-endian code, 0 if
   this is little-endian code.  */

static int
riscv_disassemble_insn (bfd_vma memaddr, insn_t word, disassemble_info *info)
{
  const struct riscv_opcode *op, *op1;
  static bfd_boolean init = 0;
  static const struct riscv_opcode *riscv_hash[OP_MASK_SFPU_OP + 1];
  static const struct riscv_opcode *riscv_hash_grayskull[OP_MASK_SFPU_OP + 1];
  static const struct riscv_opcode *riscv_hash_wormhole[OP_MASK_SFPU_OP + 1];
  static const struct riscv_opcode *riscv_hash_blackhole[OP_MASK_SFPU_OP + 1];
  struct riscv_private_data *pd;
  int insnlen;
  int is_sfpu = 0;
#define OP_HASH_IDX(i) ((i) & (riscv_insn_length (i) == 2 ? 0x3 : OP_MASK_OP))
#define SFPU_OP_HASH_IDX(i) \
        (((i) & 0xffffff00) == (MATCH_SFPNOP & 0xffffff00) ? \
	  SFP_OPCODE_END - SFP_OPCODE_START + OP_MASK_OP + 1 : \
          (((i) >> OP_SH_SFPU_OP) & OP_MASK_SFPU_OP) - SFP_OPCODE_START + OP_MASK_OP + 1)

  /* Build a hash table to shorten the search time.  */
  if (! init)
    {
      // insert instructions into hash table if sfpu_mach is grayskull target.
      if (sfpu_mach == SFPU_MACH_GRAYSKULL) {
        for (op = riscv_opcodes; op->name; op++) {
          if (!strncasecmp(op->name, "sfp", 3) ||
                 !strncasecmp(op->name, "tt", 2))
            {
              if (op->insn_class == INSN_CLASS_I_Y) {
                if (!riscv_hash_grayskull[SFPU_OP_HASH_IDX (op->match)])
                  riscv_hash_grayskull[SFPU_OP_HASH_IDX (op->match)] = op;
              }
            }
          else 
  	  {
            if (!riscv_hash[OP_HASH_IDX (op->match)])
              riscv_hash[OP_HASH_IDX (op->match)] = op;
          }
        }
      }
      // insert instructions into hash table if sfpu_mach is wormhole target.
      if (sfpu_mach == SFPU_MACH_WORMHOLE) {
        for (op = riscv_opcodes; op->name; op++) {
          if (!strncasecmp(op->name, "sfp", 3) ||
               !strncasecmp(op->name, "tt", 2))
            { 
              if (op->insn_class == INSN_CLASS_I_W) {
                if (!riscv_hash_wormhole[SFPU_OP_HASH_IDX (op->match)])
                  riscv_hash_wormhole[SFPU_OP_HASH_IDX (op->match)] = op;
              }
            }
          else
            { 
              if (!riscv_hash[OP_HASH_IDX (op->match)])
                riscv_hash[OP_HASH_IDX (op->match)] = op;
            }
          }
        }
          // insert instructions into hash table if sfpu_mach is blackhole target.
      if (sfpu_mach == SFPU_MACH_BLACKHOLE) {
        for (op = riscv_opcodes; op->name; op++) {
          if (!strncasecmp(op->name, "sfp", 3) ||
               !strncasecmp(op->name, "tt", 2))
            {
              if (op->insn_class == INSN_CLASS_I_L) {
                if (!riscv_hash_blackhole[SFPU_OP_HASH_IDX (op->match)])
                  riscv_hash_blackhole[SFPU_OP_HASH_IDX (op->match)] = op;
              }
            }
          else
            {
              if (!riscv_hash[OP_HASH_IDX (op->match)])
                riscv_hash[OP_HASH_IDX (op->match)] = op;
            }
          }
        }

      init = 1;
    }
  /* Unswizzle the bottom 2 bits so that we get back the original instruction
     for SFPU */
  if ((word & 0x3) != 0x3) {
    word = SFPU_OP_UNSWIZZLE(word);
    is_sfpu = 1;
  }

  if (info->private_data == NULL)
    {
      int i;

      pd = info->private_data = xcalloc (1, sizeof (struct riscv_private_data));
      pd->gp = -1;
      pd->print_addr = -1;
      for (i = 0; i < (int)ARRAY_SIZE (pd->hi_addr); i++)
	pd->hi_addr[i] = -1;

      for (i = 0; i < info->symtab_size; i++)
	if (strcmp (bfd_asymbol_name (info->symtab[i]), RISCV_GP_SYMBOL) == 0)
	  pd->gp = bfd_asymbol_value (info->symtab[i]);
    }
  else
    pd = info->private_data;

  insnlen = riscv_insn_length (word);

  /* RISC-V instructions are always little-endian.  */
  info->endian_code = BFD_ENDIAN_LITTLE;

  info->bytes_per_chunk = insnlen % 4 == 0 ? 4 : 2;
  info->bytes_per_line = 8;
  /* We don't support constant pools, so this must be code.  */
  info->display_endian = info->endian_code;
  info->insn_info_valid = 1;
  info->branch_delay_insns = 0;
  info->data_size = 0;
  info->insn_type = dis_nonbranch;
  info->target = 0;
  info->target2 = 0;
  // op = is_sfpu ?  riscv_hash[SFPU_OP_HASH_IDX (word)] :
  //                 riscv_hash[OP_HASH_IDX (word)];
  if (is_sfpu) {
      if (sfpu_mach == SFPU_MACH_WORMHOLE)
        op = riscv_hash_wormhole[SFPU_OP_HASH_IDX (word)];
      else if (sfpu_mach == SFPU_MACH_GRAYSKULL)
        op = riscv_hash_grayskull[SFPU_OP_HASH_IDX (word)];
      else if (sfpu_mach == SFPU_MACH_BLACKHOLE)
        op = riscv_hash_blackhole[SFPU_OP_HASH_IDX (word)];
      else
        op = riscv_hash[SFPU_OP_HASH_IDX (word)];
  } else {
    op = riscv_hash[OP_HASH_IDX (word)];
  }

  if (op != NULL)
    {
      unsigned xlen = 0;

      /* If XLEN is not known, get its value from the ELF class.  */
      if (info->mach == bfd_mach_riscv64)
	xlen = 64;
      else if (info->mach == bfd_mach_riscv32)
	xlen = 32;
      else if (info->mach == bfd_mach_riscv32_sfpu_wormhole)
	xlen = 32;
      else if (info->mach == bfd_mach_riscv32_sfpu_blackhole)
        xlen = 32;
      else if (info->section != NULL)
	{
	  Elf_Internal_Ehdr *ehdr = elf_elfheader (info->section->owner);
	  xlen = ehdr->e_ident[EI_CLASS] == ELFCLASS64 ? 64 : 32;
	}

      for (; op->name; op++)
	{
	  /* Does the opcode match?  */
	  if (! (op->match_func) (op, word))
	    continue;
	  /* Is this a pseudo-instruction and may we print it as such?  */
	  if (no_aliases && (op->pinfo & INSN_ALIAS))
	    continue;
	  /* Is this instruction restricted to a certain value of XLEN?  */
	  if ((op->xlen_requirement != 0) && (op->xlen_requirement != xlen))
	    continue;

	  /* It's a match.  */
	  (*info->fprintf_func) (info->stream, "%s", op->name);
	  print_insn_args (op->args, word, memaddr, info);

	  /* Try to disassemble multi-instruction addressing sequences.  */
	  if (pd->print_addr != (bfd_vma)-1)
	    {
	      info->target = pd->print_addr;
	      (*info->fprintf_func) (info->stream, " # ");
	      (*info->print_address_func) (info->target, info);
	      pd->print_addr = -1;
	    }

	  /* Finish filling out insn_info fields.  */
	  switch (op->pinfo & INSN_TYPE)
	    {
	    case INSN_BRANCH:
	      info->insn_type = dis_branch;
	      break;
	    case INSN_CONDBRANCH:
	      info->insn_type = dis_condbranch;
	      break;
	    case INSN_JSR:
	      info->insn_type = dis_jsr;
	      break;
	    case INSN_DREF:
	      info->insn_type = dis_dref;
	      break;
	    default:
	      break;
	    }

	  if (op->pinfo & INSN_DATA_SIZE)
	    {
	      int size = ((op->pinfo & INSN_DATA_SIZE)
			  >> INSN_DATA_SIZE_SHIFT);
	      info->data_size = 1 << (size - 1);
	    }

	  return insnlen;
	}
    }

  /* We did not find a match, so just print the instruction bits.  */
  info->insn_type = dis_noninsn;
  (*info->fprintf_func) (info->stream, "0x%llx", (unsigned long long)word);
  return insnlen;
}

int
print_insn_riscv (bfd_vma memaddr, struct disassemble_info *info)
{
  bfd_byte packet[2];
  insn_t insn = 0;
  bfd_vma n;
  int status;

  if (info->disassembler_options != NULL)
    {
      parse_riscv_dis_options (info->disassembler_options);
      /* Avoid repeatedly parsing the options.  */
      info->disassembler_options = NULL;
    }
  else if (riscv_gpr_names == NULL)
    set_default_riscv_dis_options ();

  /* Instructions are a sequence of 2-byte packets in little-endian order.  */
  for (n = 0; n < sizeof (insn) && n < riscv_insn_length (insn); n += 2)
    {
      status = (*info->read_memory_func) (memaddr + n, packet, 2, info);
      if (status != 0)
	{
	  /* Don't fail just because we fell off the end.  */
	  if (n > 0)
	    break;
	  (*info->memory_error_func) (status, memaddr, info);
	  return status;
	}

      insn |= ((insn_t) bfd_getl16 (packet)) << (8 * n);
    }

  return riscv_disassemble_insn (memaddr, insn, info);
}

disassembler_ftype
riscv_get_disassembler (bfd *abfd)
{
  /* If -Mpriv-spec= isn't set, then try to set it by checking the elf
     privileged attributes.  */
  if (abfd)
    {
      const char *sec_name = get_elf_backend_data (abfd)->obj_attrs_section;
      if (bfd_get_section_by_name (abfd, sec_name) != NULL)
        {
	  obj_attribute *attr = elf_known_obj_attributes_proc (abfd);
	  unsigned int Tag_a = Tag_RISCV_priv_spec;
	  unsigned int Tag_b = Tag_RISCV_priv_spec_minor;
	  unsigned int Tag_c = Tag_RISCV_priv_spec_revision;
	  riscv_get_priv_spec_class_from_numbers (attr[Tag_a].i,
						  attr[Tag_b].i,
						  attr[Tag_c].i,
						  &default_priv_spec);
        }
      if (abfd->tdata.elf_obj_data->elf_header->e_machine == EM_RISCV_GRAYSKULL)
        sfpu_mach = SFPU_MACH_GRAYSKULL;
      else if (abfd->tdata.elf_obj_data->elf_header->e_machine == EM_RISCV_WORMHOLE)
        sfpu_mach = SFPU_MACH_WORMHOLE;
      else if (abfd->tdata.elf_obj_data->elf_header->e_machine == EM_RISCV_BLACKHOLE)
        sfpu_mach = SFPU_MACH_BLACKHOLE;
    }
   return print_insn_riscv;
}

/* Prevent use of the fake labels that are generated as part of the DWARF
   and for relaxable relocations in the assembler.  */

bfd_boolean
riscv_symbol_is_valid (asymbol * sym,
                       struct disassemble_info * info ATTRIBUTE_UNUSED)
{
  const char * name;

  if (sym == NULL)
    return FALSE;

  name = bfd_asymbol_name (sym);

  return (strcmp (name, RISCV_FAKE_LABEL_NAME) != 0);
}

void
print_riscv_disassembler_options (FILE *stream)
{
  fprintf (stream, _("\n\
The following RISC-V-specific disassembler options are supported for use\n\
with the -M switch (multiple options should be separated by commas):\n"));

  fprintf (stream, _("\n\
  numeric         Print numeric register names, rather than ABI names.\n"));

  fprintf (stream, _("\n\
  no-aliases      Disassemble only into canonical instructions, rather\n\
                  than into pseudoinstructions.\n"));

  fprintf (stream, _("\n\
  priv-spec=PRIV  Print the CSR according to the chosen privilege spec\n\
                  (1.9, 1.9.1, 1.10, 1.11).\n"));

  fprintf (stream, _("\n"));
}
