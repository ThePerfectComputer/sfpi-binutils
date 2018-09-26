/* This file is automatically generated by aarch64-gen.  Do not edit!  */
/* Copyright (C) 2012-2018 Free Software Foundation, Inc.
   Contributed by ARM Ltd.

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
#include "aarch64-asm.h"


const aarch64_opcode *
aarch64_find_real_opcode (const aarch64_opcode *opcode)
{
  /* Use the index as the key to locate the real opcode.  */
  int key = opcode - aarch64_opcode_table;
  int value;
  switch (key)
    {
    case 3:	/* ngc */
    case 2:	/* sbc */
      value = 2;	/* --> sbc.  */
      break;
    case 5:	/* ngcs */
    case 4:	/* sbcs */
      value = 4;	/* --> sbcs.  */
      break;
    case 8:	/* cmn */
    case 7:	/* adds */
      value = 7;	/* --> adds.  */
      break;
    case 11:	/* cmp */
    case 10:	/* subs */
      value = 10;	/* --> subs.  */
      break;
    case 13:	/* mov */
    case 12:	/* add */
      value = 12;	/* --> add.  */
      break;
    case 15:	/* cmn */
    case 14:	/* adds */
      value = 14;	/* --> adds.  */
      break;
    case 18:	/* cmp */
    case 17:	/* subs */
      value = 17;	/* --> subs.  */
      break;
    case 21:	/* cmn */
    case 20:	/* adds */
      value = 20;	/* --> adds.  */
      break;
    case 23:	/* neg */
    case 22:	/* sub */
      value = 22;	/* --> sub.  */
      break;
    case 25:	/* cmp */
    case 26:	/* negs */
    case 24:	/* subs */
      value = 24;	/* --> subs.  */
      break;
    case 151:	/* mov */
    case 150:	/* umov */
      value = 150;	/* --> umov.  */
      break;
    case 153:	/* mov */
    case 152:	/* ins */
      value = 152;	/* --> ins.  */
      break;
    case 155:	/* mov */
    case 154:	/* ins */
      value = 154;	/* --> ins.  */
      break;
    case 241:	/* mvn */
    case 240:	/* not */
      value = 240;	/* --> not.  */
      break;
    case 316:	/* mov */
    case 315:	/* orr */
      value = 315;	/* --> orr.  */
      break;
    case 387:	/* sxtl */
    case 386:	/* sshll */
      value = 386;	/* --> sshll.  */
      break;
    case 389:	/* sxtl2 */
    case 388:	/* sshll2 */
      value = 388;	/* --> sshll2.  */
      break;
    case 411:	/* uxtl */
    case 410:	/* ushll */
      value = 410;	/* --> ushll.  */
      break;
    case 413:	/* uxtl2 */
    case 412:	/* ushll2 */
      value = 412;	/* --> ushll2.  */
      break;
    case 534:	/* mov */
    case 533:	/* dup */
      value = 533;	/* --> dup.  */
      break;
    case 621:	/* sxtw */
    case 620:	/* sxth */
    case 619:	/* sxtb */
    case 622:	/* asr */
    case 618:	/* sbfx */
    case 617:	/* sbfiz */
    case 616:	/* sbfm */
      value = 616;	/* --> sbfm.  */
      break;
    case 625:	/* bfc */
    case 626:	/* bfxil */
    case 624:	/* bfi */
    case 623:	/* bfm */
      value = 623;	/* --> bfm.  */
      break;
    case 631:	/* uxth */
    case 630:	/* uxtb */
    case 633:	/* lsr */
    case 632:	/* lsl */
    case 629:	/* ubfx */
    case 628:	/* ubfiz */
    case 627:	/* ubfm */
      value = 627;	/* --> ubfm.  */
      break;
    case 663:	/* cset */
    case 662:	/* cinc */
    case 661:	/* csinc */
      value = 661;	/* --> csinc.  */
      break;
    case 666:	/* csetm */
    case 665:	/* cinv */
    case 664:	/* csinv */
      value = 664;	/* --> csinv.  */
      break;
    case 668:	/* cneg */
    case 667:	/* csneg */
      value = 667;	/* --> csneg.  */
      break;
    case 686:	/* rev */
    case 687:	/* rev64 */
      value = 686;	/* --> rev.  */
      break;
    case 712:	/* lsl */
    case 711:	/* lslv */
      value = 711;	/* --> lslv.  */
      break;
    case 714:	/* lsr */
    case 713:	/* lsrv */
      value = 713;	/* --> lsrv.  */
      break;
    case 716:	/* asr */
    case 715:	/* asrv */
      value = 715;	/* --> asrv.  */
      break;
    case 718:	/* ror */
    case 717:	/* rorv */
      value = 717;	/* --> rorv.  */
      break;
    case 729:	/* mul */
    case 728:	/* madd */
      value = 728;	/* --> madd.  */
      break;
    case 731:	/* mneg */
    case 730:	/* msub */
      value = 730;	/* --> msub.  */
      break;
    case 733:	/* smull */
    case 732:	/* smaddl */
      value = 732;	/* --> smaddl.  */
      break;
    case 735:	/* smnegl */
    case 734:	/* smsubl */
      value = 734;	/* --> smsubl.  */
      break;
    case 738:	/* umull */
    case 737:	/* umaddl */
      value = 737;	/* --> umaddl.  */
      break;
    case 740:	/* umnegl */
    case 739:	/* umsubl */
      value = 739;	/* --> umsubl.  */
      break;
    case 751:	/* ror */
    case 750:	/* extr */
      value = 750;	/* --> extr.  */
      break;
    case 970:	/* bic */
    case 969:	/* and */
      value = 969;	/* --> and.  */
      break;
    case 972:	/* mov */
    case 971:	/* orr */
      value = 971;	/* --> orr.  */
      break;
    case 975:	/* tst */
    case 974:	/* ands */
      value = 974;	/* --> ands.  */
      break;
    case 980:	/* uxtw */
    case 979:	/* mov */
    case 978:	/* orr */
      value = 978;	/* --> orr.  */
      break;
    case 982:	/* mvn */
    case 981:	/* orn */
      value = 981;	/* --> orn.  */
      break;
    case 986:	/* tst */
    case 985:	/* ands */
      value = 985;	/* --> ands.  */
      break;
    case 1112:	/* staddb */
    case 1016:	/* ldaddb */
      value = 1016;	/* --> ldaddb.  */
      break;
    case 1113:	/* staddh */
    case 1017:	/* ldaddh */
      value = 1017;	/* --> ldaddh.  */
      break;
    case 1114:	/* stadd */
    case 1018:	/* ldadd */
      value = 1018;	/* --> ldadd.  */
      break;
    case 1115:	/* staddlb */
    case 1020:	/* ldaddlb */
      value = 1020;	/* --> ldaddlb.  */
      break;
    case 1116:	/* staddlh */
    case 1023:	/* ldaddlh */
      value = 1023;	/* --> ldaddlh.  */
      break;
    case 1117:	/* staddl */
    case 1026:	/* ldaddl */
      value = 1026;	/* --> ldaddl.  */
      break;
    case 1118:	/* stclrb */
    case 1028:	/* ldclrb */
      value = 1028;	/* --> ldclrb.  */
      break;
    case 1119:	/* stclrh */
    case 1029:	/* ldclrh */
      value = 1029;	/* --> ldclrh.  */
      break;
    case 1120:	/* stclr */
    case 1030:	/* ldclr */
      value = 1030;	/* --> ldclr.  */
      break;
    case 1121:	/* stclrlb */
    case 1032:	/* ldclrlb */
      value = 1032;	/* --> ldclrlb.  */
      break;
    case 1122:	/* stclrlh */
    case 1035:	/* ldclrlh */
      value = 1035;	/* --> ldclrlh.  */
      break;
    case 1123:	/* stclrl */
    case 1038:	/* ldclrl */
      value = 1038;	/* --> ldclrl.  */
      break;
    case 1124:	/* steorb */
    case 1040:	/* ldeorb */
      value = 1040;	/* --> ldeorb.  */
      break;
    case 1125:	/* steorh */
    case 1041:	/* ldeorh */
      value = 1041;	/* --> ldeorh.  */
      break;
    case 1126:	/* steor */
    case 1042:	/* ldeor */
      value = 1042;	/* --> ldeor.  */
      break;
    case 1127:	/* steorlb */
    case 1044:	/* ldeorlb */
      value = 1044;	/* --> ldeorlb.  */
      break;
    case 1128:	/* steorlh */
    case 1047:	/* ldeorlh */
      value = 1047;	/* --> ldeorlh.  */
      break;
    case 1129:	/* steorl */
    case 1050:	/* ldeorl */
      value = 1050;	/* --> ldeorl.  */
      break;
    case 1130:	/* stsetb */
    case 1052:	/* ldsetb */
      value = 1052;	/* --> ldsetb.  */
      break;
    case 1131:	/* stseth */
    case 1053:	/* ldseth */
      value = 1053;	/* --> ldseth.  */
      break;
    case 1132:	/* stset */
    case 1054:	/* ldset */
      value = 1054;	/* --> ldset.  */
      break;
    case 1133:	/* stsetlb */
    case 1056:	/* ldsetlb */
      value = 1056;	/* --> ldsetlb.  */
      break;
    case 1134:	/* stsetlh */
    case 1059:	/* ldsetlh */
      value = 1059;	/* --> ldsetlh.  */
      break;
    case 1135:	/* stsetl */
    case 1062:	/* ldsetl */
      value = 1062;	/* --> ldsetl.  */
      break;
    case 1136:	/* stsmaxb */
    case 1064:	/* ldsmaxb */
      value = 1064;	/* --> ldsmaxb.  */
      break;
    case 1137:	/* stsmaxh */
    case 1065:	/* ldsmaxh */
      value = 1065;	/* --> ldsmaxh.  */
      break;
    case 1138:	/* stsmax */
    case 1066:	/* ldsmax */
      value = 1066;	/* --> ldsmax.  */
      break;
    case 1139:	/* stsmaxlb */
    case 1068:	/* ldsmaxlb */
      value = 1068;	/* --> ldsmaxlb.  */
      break;
    case 1140:	/* stsmaxlh */
    case 1071:	/* ldsmaxlh */
      value = 1071;	/* --> ldsmaxlh.  */
      break;
    case 1141:	/* stsmaxl */
    case 1074:	/* ldsmaxl */
      value = 1074;	/* --> ldsmaxl.  */
      break;
    case 1142:	/* stsminb */
    case 1076:	/* ldsminb */
      value = 1076;	/* --> ldsminb.  */
      break;
    case 1143:	/* stsminh */
    case 1077:	/* ldsminh */
      value = 1077;	/* --> ldsminh.  */
      break;
    case 1144:	/* stsmin */
    case 1078:	/* ldsmin */
      value = 1078;	/* --> ldsmin.  */
      break;
    case 1145:	/* stsminlb */
    case 1080:	/* ldsminlb */
      value = 1080;	/* --> ldsminlb.  */
      break;
    case 1146:	/* stsminlh */
    case 1083:	/* ldsminlh */
      value = 1083;	/* --> ldsminlh.  */
      break;
    case 1147:	/* stsminl */
    case 1086:	/* ldsminl */
      value = 1086;	/* --> ldsminl.  */
      break;
    case 1148:	/* stumaxb */
    case 1088:	/* ldumaxb */
      value = 1088;	/* --> ldumaxb.  */
      break;
    case 1149:	/* stumaxh */
    case 1089:	/* ldumaxh */
      value = 1089;	/* --> ldumaxh.  */
      break;
    case 1150:	/* stumax */
    case 1090:	/* ldumax */
      value = 1090;	/* --> ldumax.  */
      break;
    case 1151:	/* stumaxlb */
    case 1092:	/* ldumaxlb */
      value = 1092;	/* --> ldumaxlb.  */
      break;
    case 1152:	/* stumaxlh */
    case 1095:	/* ldumaxlh */
      value = 1095;	/* --> ldumaxlh.  */
      break;
    case 1153:	/* stumaxl */
    case 1098:	/* ldumaxl */
      value = 1098;	/* --> ldumaxl.  */
      break;
    case 1154:	/* stuminb */
    case 1100:	/* lduminb */
      value = 1100;	/* --> lduminb.  */
      break;
    case 1155:	/* stuminh */
    case 1101:	/* lduminh */
      value = 1101;	/* --> lduminh.  */
      break;
    case 1156:	/* stumin */
    case 1102:	/* ldumin */
      value = 1102;	/* --> ldumin.  */
      break;
    case 1157:	/* stuminlb */
    case 1104:	/* lduminlb */
      value = 1104;	/* --> lduminlb.  */
      break;
    case 1158:	/* stuminlh */
    case 1107:	/* lduminlh */
      value = 1107;	/* --> lduminlh.  */
      break;
    case 1159:	/* stuminl */
    case 1110:	/* lduminl */
      value = 1110;	/* --> lduminl.  */
      break;
    case 1161:	/* mov */
    case 1160:	/* movn */
      value = 1160;	/* --> movn.  */
      break;
    case 1163:	/* mov */
    case 1162:	/* movz */
      value = 1162;	/* --> movz.  */
      break;
    case 1205:	/* autibsp */
    case 1204:	/* autibz */
    case 1203:	/* autiasp */
    case 1202:	/* autiaz */
    case 1201:	/* pacibsp */
    case 1200:	/* pacibz */
    case 1199:	/* paciasp */
    case 1198:	/* paciaz */
    case 1182:	/* psb */
    case 1181:	/* esb */
    case 1180:	/* autib1716 */
    case 1179:	/* autia1716 */
    case 1178:	/* pacib1716 */
    case 1177:	/* pacia1716 */
    case 1176:	/* xpaclri */
    case 1175:	/* sevl */
    case 1174:	/* sev */
    case 1173:	/* wfi */
    case 1172:	/* wfe */
    case 1171:	/* yield */
    case 1170:	/* csdb */
    case 1169:	/* nop */
    case 1168:	/* hint */
      value = 1168;	/* --> hint.  */
      break;
    case 1186:	/* pssbb */
    case 1185:	/* ssbb */
    case 1184:	/* dsb */
      value = 1184;	/* --> dsb.  */
      break;
    case 1194:	/* tlbi */
    case 1193:	/* ic */
    case 1192:	/* dc */
    case 1191:	/* at */
    case 1190:	/* sys */
      value = 1190;	/* --> sys.  */
      break;
    case 2003:	/* bic */
    case 1253:	/* and */
      value = 1253;	/* --> and.  */
      break;
    case 1236:	/* mov */
    case 1255:	/* and */
      value = 1255;	/* --> and.  */
      break;
    case 1240:	/* movs */
    case 1256:	/* ands */
      value = 1256;	/* --> ands.  */
      break;
    case 2004:	/* cmple */
    case 1291:	/* cmpge */
      value = 1291;	/* --> cmpge.  */
      break;
    case 2007:	/* cmplt */
    case 1294:	/* cmpgt */
      value = 1294;	/* --> cmpgt.  */
      break;
    case 2005:	/* cmplo */
    case 1296:	/* cmphi */
      value = 1296;	/* --> cmphi.  */
      break;
    case 2006:	/* cmpls */
    case 1299:	/* cmphs */
      value = 1299;	/* --> cmphs.  */
      break;
    case 1233:	/* mov */
    case 1321:	/* cpy */
      value = 1321;	/* --> cpy.  */
      break;
    case 1235:	/* mov */
    case 1322:	/* cpy */
      value = 1322;	/* --> cpy.  */
      break;
    case 2014:	/* fmov */
    case 1238:	/* mov */
    case 1323:	/* cpy */
      value = 1323;	/* --> cpy.  */
      break;
    case 1228:	/* mov */
    case 1335:	/* dup */
      value = 1335;	/* --> dup.  */
      break;
    case 1230:	/* mov */
    case 1227:	/* mov */
    case 1336:	/* dup */
      value = 1336;	/* --> dup.  */
      break;
    case 2013:	/* fmov */
    case 1232:	/* mov */
    case 1337:	/* dup */
      value = 1337;	/* --> dup.  */
      break;
    case 1231:	/* mov */
    case 1338:	/* dupm */
      value = 1338;	/* --> dupm.  */
      break;
    case 2008:	/* eon */
    case 1340:	/* eor */
      value = 1340;	/* --> eor.  */
      break;
    case 1241:	/* not */
    case 1342:	/* eor */
      value = 1342;	/* --> eor.  */
      break;
    case 1242:	/* nots */
    case 1343:	/* eors */
      value = 1343;	/* --> eors.  */
      break;
    case 2009:	/* facle */
    case 1348:	/* facge */
      value = 1348;	/* --> facge.  */
      break;
    case 2010:	/* faclt */
    case 1349:	/* facgt */
      value = 1349;	/* --> facgt.  */
      break;
    case 2011:	/* fcmle */
    case 1362:	/* fcmge */
      value = 1362;	/* --> fcmge.  */
      break;
    case 2012:	/* fcmlt */
    case 1364:	/* fcmgt */
      value = 1364;	/* --> fcmgt.  */
      break;
    case 1225:	/* fmov */
    case 1370:	/* fcpy */
      value = 1370;	/* --> fcpy.  */
      break;
    case 1224:	/* fmov */
    case 1393:	/* fdup */
      value = 1393;	/* --> fdup.  */
      break;
    case 1226:	/* mov */
    case 1724:	/* orr */
      value = 1724;	/* --> orr.  */
      break;
    case 2015:	/* orn */
    case 1725:	/* orr */
      value = 1725;	/* --> orr.  */
      break;
    case 1229:	/* mov */
    case 1727:	/* orr */
      value = 1727;	/* --> orr.  */
      break;
    case 1239:	/* movs */
    case 1728:	/* orrs */
      value = 1728;	/* --> orrs.  */
      break;
    case 1234:	/* mov */
    case 1790:	/* sel */
      value = 1790;	/* --> sel.  */
      break;
    case 1237:	/* mov */
    case 1791:	/* sel */
      value = 1791;	/* --> sel.  */
      break;
    default: return NULL;
    }

  return aarch64_opcode_table + value;
}

bfd_boolean
aarch64_insert_operand (const aarch64_operand *self,
			   const aarch64_opnd_info *info,
			   aarch64_insn *code, const aarch64_inst *inst,
			   aarch64_operand_error *errors)
{
  /* Use the index as the key.  */
  int key = self - aarch64_operands;
  switch (key)
    {
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 15:
    case 16:
    case 17:
    case 18:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
    case 158:
    case 159:
    case 160:
    case 161:
    case 162:
    case 175:
    case 176:
    case 177:
    case 178:
    case 179:
    case 180:
    case 181:
    case 182:
    case 183:
    case 187:
    case 190:
      return aarch64_ins_regno (self, info, code, inst, errors);
    case 13:
      return aarch64_ins_reg_extended (self, info, code, inst, errors);
    case 14:
      return aarch64_ins_reg_shifted (self, info, code, inst, errors);
    case 19:
      return aarch64_ins_ft (self, info, code, inst, errors);
    case 30:
    case 31:
    case 32:
    case 33:
    case 192:
      return aarch64_ins_reglane (self, info, code, inst, errors);
    case 34:
      return aarch64_ins_reglist (self, info, code, inst, errors);
    case 35:
      return aarch64_ins_ldst_reglist (self, info, code, inst, errors);
    case 36:
      return aarch64_ins_ldst_reglist_r (self, info, code, inst, errors);
    case 37:
      return aarch64_ins_ldst_elemlist (self, info, code, inst, errors);
    case 38:
    case 39:
    case 40:
    case 41:
    case 51:
    case 52:
    case 53:
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
    case 61:
    case 62:
    case 63:
    case 64:
    case 76:
    case 77:
    case 78:
    case 79:
    case 150:
    case 152:
    case 167:
    case 168:
    case 169:
    case 170:
    case 171:
    case 172:
    case 173:
    case 174:
      return aarch64_ins_imm (self, info, code, inst, errors);
    case 42:
    case 43:
      return aarch64_ins_advsimd_imm_shift (self, info, code, inst, errors);
    case 44:
    case 45:
    case 46:
      return aarch64_ins_advsimd_imm_modified (self, info, code, inst, errors);
    case 50:
    case 141:
      return aarch64_ins_fpimm (self, info, code, inst, errors);
    case 65:
    case 148:
      return aarch64_ins_limm (self, info, code, inst, errors);
    case 66:
      return aarch64_ins_aimm (self, info, code, inst, errors);
    case 67:
      return aarch64_ins_imm_half (self, info, code, inst, errors);
    case 68:
      return aarch64_ins_fbits (self, info, code, inst, errors);
    case 70:
    case 71:
    case 146:
      return aarch64_ins_imm_rotate2 (self, info, code, inst, errors);
    case 72:
    case 145:
      return aarch64_ins_imm_rotate1 (self, info, code, inst, errors);
    case 73:
    case 74:
      return aarch64_ins_cond (self, info, code, inst, errors);
    case 80:
    case 87:
      return aarch64_ins_addr_simple (self, info, code, inst, errors);
    case 81:
      return aarch64_ins_addr_regoff (self, info, code, inst, errors);
    case 82:
    case 83:
    case 84:
      return aarch64_ins_addr_simm (self, info, code, inst, errors);
    case 85:
      return aarch64_ins_addr_simm10 (self, info, code, inst, errors);
    case 86:
      return aarch64_ins_addr_uimm12 (self, info, code, inst, errors);
    case 88:
      return aarch64_ins_addr_offset (self, info, code, inst, errors);
    case 89:
      return aarch64_ins_simd_addr_post (self, info, code, inst, errors);
    case 90:
      return aarch64_ins_sysreg (self, info, code, inst, errors);
    case 91:
      return aarch64_ins_pstatefield (self, info, code, inst, errors);
    case 92:
    case 93:
    case 94:
    case 95:
      return aarch64_ins_sysins_op (self, info, code, inst, errors);
    case 96:
    case 97:
      return aarch64_ins_barrier (self, info, code, inst, errors);
    case 98:
      return aarch64_ins_prfop (self, info, code, inst, errors);
    case 99:
      return aarch64_ins_hint (self, info, code, inst, errors);
    case 100:
      return aarch64_ins_sve_addr_ri_s4 (self, info, code, inst, errors);
    case 101:
    case 102:
    case 103:
    case 104:
      return aarch64_ins_sve_addr_ri_s4xvl (self, info, code, inst, errors);
    case 105:
      return aarch64_ins_sve_addr_ri_s6xvl (self, info, code, inst, errors);
    case 106:
      return aarch64_ins_sve_addr_ri_s9xvl (self, info, code, inst, errors);
    case 107:
    case 108:
    case 109:
    case 110:
      return aarch64_ins_sve_addr_ri_u6 (self, info, code, inst, errors);
    case 111:
    case 112:
    case 113:
    case 114:
    case 115:
    case 116:
    case 117:
    case 118:
    case 119:
    case 120:
    case 121:
    case 122:
    case 123:
      return aarch64_ins_sve_addr_rr_lsl (self, info, code, inst, errors);
    case 124:
    case 125:
    case 126:
    case 127:
    case 128:
    case 129:
    case 130:
    case 131:
      return aarch64_ins_sve_addr_rz_xtw (self, info, code, inst, errors);
    case 132:
    case 133:
    case 134:
    case 135:
      return aarch64_ins_sve_addr_zi_u5 (self, info, code, inst, errors);
    case 136:
      return aarch64_ins_sve_addr_zz_lsl (self, info, code, inst, errors);
    case 137:
      return aarch64_ins_sve_addr_zz_sxtw (self, info, code, inst, errors);
    case 138:
      return aarch64_ins_sve_addr_zz_uxtw (self, info, code, inst, errors);
    case 139:
      return aarch64_ins_sve_aimm (self, info, code, inst, errors);
    case 140:
      return aarch64_ins_sve_asimm (self, info, code, inst, errors);
    case 142:
      return aarch64_ins_sve_float_half_one (self, info, code, inst, errors);
    case 143:
      return aarch64_ins_sve_float_half_two (self, info, code, inst, errors);
    case 144:
      return aarch64_ins_sve_float_zero_one (self, info, code, inst, errors);
    case 147:
      return aarch64_ins_inv_limm (self, info, code, inst, errors);
    case 149:
      return aarch64_ins_sve_limm_mov (self, info, code, inst, errors);
    case 151:
      return aarch64_ins_sve_scale (self, info, code, inst, errors);
    case 163:
    case 164:
      return aarch64_ins_sve_shlimm (self, info, code, inst, errors);
    case 165:
    case 166:
      return aarch64_ins_sve_shrimm (self, info, code, inst, errors);
    case 184:
    case 185:
    case 186:
      return aarch64_ins_sve_quad_index (self, info, code, inst, errors);
    case 188:
      return aarch64_ins_sve_index (self, info, code, inst, errors);
    case 189:
    case 191:
      return aarch64_ins_sve_reglist (self, info, code, inst, errors);
    default: assert (0); abort ();
    }
}
