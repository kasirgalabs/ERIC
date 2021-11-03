//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TOOLS_LLVM_OBJDUMP_LLVM_OBJDUMP_H
#define LLVM_TOOLS_LLVM_OBJDUMP_LLVM_OBJDUMP_H

#include "llvm/ADT/StringSet.h"
#include "llvm/DebugInfo/DIContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/Object/Archive.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Compiler.h"
#include "llvm/Support/DataTypes.h"

namespace llvm {
class StringRef;

namespace object {
class ELFObjectFileBase;
class ELFSectionRef;
class MachOObjectFile;
class MachOUniversalBinary;
class RelocationRef;
} // namespace object

namespace objdump {

extern cl::opt<bool> ArchiveHeaders;
extern cl::opt<bool> Demangle;
extern cl::opt<bool> Disassemble;
extern cl::opt<bool> DisassembleAll;
extern cl::opt<DIDumpType> DwarfDumpType;
extern cl::list<std::string> FilterSections;
extern cl::list<std::string> MAttrs;
extern cl::opt<std::string> MCPU;
extern cl::opt<bool> NoShowRawInsn;
extern cl::opt<bool> NoLeadingAddr;
extern cl::opt<bool> PrintImmHex;
extern cl::opt<bool> PrivateHeaders;
extern cl::opt<bool> Relocations;
extern cl::opt<bool> SectionHeaders;
extern cl::opt<bool> SectionContents;
extern cl::opt<bool> SymbolDescription;
extern cl::opt<bool> SymbolTable;
extern cl::opt<std::string> TripleName;
extern cl::opt<bool> UnwindInfo;

/////////////ekleme
//degistirdim

// All dots(.) replaced with underscore(_)

// rv32i
extern cl::opt<bool> lui;
extern cl::opt<bool> auipc;
extern cl::opt<bool> jal;
extern cl::opt<bool> jalr;
extern cl::opt<bool> beq;
extern cl::opt<bool> bne;
extern cl::opt<bool> blt;
extern cl::opt<bool> bge;
extern cl::opt<bool> bltu;
extern cl::opt<bool> bgeu;
extern cl::opt<bool> lb;
extern cl::opt<bool> lh;
extern cl::opt<bool> lw;
extern cl::opt<bool> lbu;
extern cl::opt<bool> lhu;
extern cl::opt<bool> sb;
extern cl::opt<bool> sh;
extern cl::opt<bool> sw;
extern cl::opt<bool> addi;
extern cl::opt<bool> slti;
extern cl::opt<bool> sltiu;
extern cl::opt<bool> xori;
extern cl::opt<bool> ori;
extern cl::opt<bool> andi;
extern cl::opt<bool> slli;
extern cl::opt<bool> srli;
extern cl::opt<bool> srai;
extern cl::opt<bool> add;
extern cl::opt<bool> sub;
extern cl::opt<bool> sll;
extern cl::opt<bool> slt;
extern cl::opt<bool> sltu;
extern cl::opt<bool> xor_;
extern cl::opt<bool> srl;
extern cl::opt<bool> sra;
extern cl::opt<bool> or_;
extern cl::opt<bool> and_;
extern cl::opt<bool> fence;
extern cl::opt<bool> fence_i;
extern cl::opt<bool> ecall;
extern cl::opt<bool> ebreak;
extern cl::opt<bool> csrrw;
extern cl::opt<bool> csrrs;
extern cl::opt<bool> csrrc;
extern cl::opt<bool> csrrwi;
extern cl::opt<bool> csrrsi;
extern cl::opt<bool> csrrci;

// rv64i
extern cl::opt<bool> lwu;
extern cl::opt<bool> ld;
extern cl::opt<bool> sd;
//extern cl::opt<bool> slli;
//extern cl::opt<bool> srli;
//extern cl::opt<bool> srai;
extern cl::opt<bool> addiw;
extern cl::opt<bool> slliw;
extern cl::opt<bool> srliw;
extern cl::opt<bool> sraiw;
extern cl::opt<bool> addw;
extern cl::opt<bool> subw;
extern cl::opt<bool> sllw;
extern cl::opt<bool> srlw;
extern cl::opt<bool> sraw;

// rv32m
extern cl::opt<bool> mul;
extern cl::opt<bool> mulh; 
extern cl::opt<bool> mulhsu;   
extern cl::opt<bool> mulhu;  
extern cl::opt<bool> div_;
extern cl::opt<bool> divu; 
extern cl::opt<bool> rem;
extern cl::opt<bool> remu;


// rv64m
extern cl::opt<bool> mulw;
extern cl::opt<bool> divw;
extern cl::opt<bool> divuw; 
extern cl::opt<bool> remw;
extern cl::opt<bool> remuw;

// rv32a
extern cl::opt<bool> lr_w;
extern cl::opt<bool> sc_w;
extern cl::opt<bool> amoswap_w;   
extern cl::opt<bool> amoadd_w;  
extern cl::opt<bool> amoxor_w;  
extern cl::opt<bool> amoand_w;  
extern cl::opt<bool> amoor_w; 
extern cl::opt<bool> amomin_w;  
extern cl::opt<bool> amomax_w;  
extern cl::opt<bool> amominu_w;   
extern cl::opt<bool> amomaxu_w;

// rv64a
extern cl::opt<bool> lr_d;
extern cl::opt<bool> sc_d;
extern cl::opt<bool> amoswap_d;
extern cl::opt<bool> amoadd_d;
extern cl::opt<bool> amoxor_d;
extern cl::opt<bool> amoand_d;
extern cl::opt<bool> amoor_d;
extern cl::opt<bool> amomin_d;
extern cl::opt<bool> amomax_d;
extern cl::opt<bool> amominu_d;
extern cl::opt<bool> amomaxu_d;

// rv32f
extern cl::opt<bool> flw;
extern cl::opt<bool> fsw;
extern cl::opt<bool> fmadd_s;
extern cl::opt<bool> fmsub_s;
extern cl::opt<bool> fnmsub_s;
extern cl::opt<bool> fnmadd_s;
extern cl::opt<bool> fadd_s;
extern cl::opt<bool> fsub_s;
extern cl::opt<bool> fmul_s;
extern cl::opt<bool> fdiv_s;
extern cl::opt<bool> fsqrt_s;
extern cl::opt<bool> fsgnj_s;
extern cl::opt<bool> fsgnjn_s;
extern cl::opt<bool> fsgnjx_s;
extern cl::opt<bool> fmin_s;
extern cl::opt<bool> fmax_s;
extern cl::opt<bool> fcvt_w_s;
extern cl::opt<bool> fcvt_wu_s;
extern cl::opt<bool> fmv_x_w;
extern cl::opt<bool> feq_s;
extern cl::opt<bool> flt_s;
extern cl::opt<bool> fle_s;
extern cl::opt<bool> fclass_s;
extern cl::opt<bool> fcvt_s_w;
extern cl::opt<bool> fcvt_s_wu;
extern cl::opt<bool> fmv_w_x;

// rv64f
extern cl::opt<bool>  fcvt_l_s;
extern cl::opt<bool>  fcvt_lu_s;
extern cl::opt<bool>  fcvt_s_l;
extern cl::opt<bool>  fcvt_s_lu;

// rv32d
extern cl::opt<bool> fld;
extern cl::opt<bool> fsd;
extern cl::opt<bool> fmadd_d;
extern cl::opt<bool> fmsub_d;
extern cl::opt<bool> fnmsub_d;
extern cl::opt<bool> fnmadd_d;
extern cl::opt<bool> fadd_d;
extern cl::opt<bool> fsub_d;
extern cl::opt<bool> fmul_d;
extern cl::opt<bool> fdiv_d;
extern cl::opt<bool> fsqrt_d;
extern cl::opt<bool> fsgnj_d;
extern cl::opt<bool> fsgnjn_d;
extern cl::opt<bool> fsgnjx_d;
extern cl::opt<bool> fmin_d;
extern cl::opt<bool> fmax_d;
extern cl::opt<bool> fcvt_s_d;
extern cl::opt<bool> fcvt_d_s;
extern cl::opt<bool> feq_d;
extern cl::opt<bool> flt_d;
extern cl::opt<bool> fle_d;
extern cl::opt<bool> fclass_d;
extern cl::opt<bool> fcvt_w_d;
extern cl::opt<bool> fcvt_wu_d;
extern cl::opt<bool> fcvt_d_w;
extern cl::opt<bool> fcvt_d_wu;

// rv64d
extern cl::opt<bool> fcvt_l_d;
extern cl::opt<bool> fcvt_lu_d;
extern cl::opt<bool> fmv_x_d;
extern cl::opt<bool> fcvt_d_l;
extern cl::opt<bool> fcvt_d_lu;
extern cl::opt<bool> fmv_d_x;

// rv32q
extern cl::opt<bool> flq;
extern cl::opt<bool> fsq;
extern cl::opt<bool> fmadd_q;
extern cl::opt<bool> fmsub_q;
extern cl::opt<bool> fnmsub_q;
extern cl::opt<bool> fnmadd_q;
extern cl::opt<bool> fadd_q;
extern cl::opt<bool> fsub_q;
extern cl::opt<bool> fmul_q;
extern cl::opt<bool> fdiv_q;
extern cl::opt<bool> fsqrt_q;
extern cl::opt<bool> fsgnj_q;
extern cl::opt<bool> fsgnjn_q;
extern cl::opt<bool> fsgnjx_q;
extern cl::opt<bool> fmin_q;
extern cl::opt<bool> fmax_q;
extern cl::opt<bool> fcvt_s_q;
extern cl::opt<bool> fcvt_q_s;
extern cl::opt<bool> fcvt_d_q;
extern cl::opt<bool> fcvt_q_d;
extern cl::opt<bool> feq_q;
extern cl::opt<bool> flt_q;
extern cl::opt<bool> fle_q;
extern cl::opt<bool> fclass_q;
extern cl::opt<bool> fcvt_w_q;
extern cl::opt<bool> fcvt_wu_q;
extern cl::opt<bool> fcvt_q_w;
extern cl::opt<bool> fcvt_q_wu;

// rv64q
extern cl::opt<bool> fcvt_l_q;
extern cl::opt<bool> fcvt_lu_q;
extern cl::opt<bool> fcvt_q_l;
extern cl::opt<bool> fcvt_q_lu;

// rvc quadrant 0
extern cl::opt<bool> c_addi4spn;
extern cl::opt<bool> c_fld;
extern cl::opt<bool> c_lq;
extern cl::opt<bool> c_lw;
extern cl::opt<bool> c_flw;
extern cl::opt<bool> c_ld;
extern cl::opt<bool> c_fsd;
extern cl::opt<bool> c_sq;
extern cl::opt<bool> c_sw;
extern cl::opt<bool> c_fsw;
extern cl::opt<bool> c_sd;

// rvc quadrant 1
extern cl::opt<bool> c_nop;
extern cl::opt<bool> c_addi;
extern cl::opt<bool> c_jal;
extern cl::opt<bool> c_addiw;
extern cl::opt<bool> c_li;
extern cl::opt<bool> c_addi16sp;
extern cl::opt<bool> c_lui;
extern cl::opt<bool> c_srli;
extern cl::opt<bool> c_srli64;
extern cl::opt<bool> c_srai;
extern cl::opt<bool> c_srai64;
extern cl::opt<bool> c_andi;
extern cl::opt<bool> c_sub;
extern cl::opt<bool> c_xor;
extern cl::opt<bool> c_or;
extern cl::opt<bool> c_and;
extern cl::opt<bool> c_subw;
extern cl::opt<bool> c_addw;
extern cl::opt<bool> c_j;
extern cl::opt<bool> c_beqz;
extern cl::opt<bool> c_bnez;

// rvc quadrant 2
extern cl::opt<bool> c_slli;
extern cl::opt<bool> c_slli64;
extern cl::opt<bool> c_fldsp;
extern cl::opt<bool> c_lqsp;
extern cl::opt<bool> c_lwsp;
extern cl::opt<bool> c_flwsp;
extern cl::opt<bool> c_ldsp;
extern cl::opt<bool> c_jr;
extern cl::opt<bool> c_mv;
extern cl::opt<bool> c_ebreak;
extern cl::opt<bool> c_jalr;
extern cl::opt<bool> c_add;
extern cl::opt<bool> c_fsdsp;
extern cl::opt<bool> c_sqsp;
extern cl::opt<bool> c_swsp;
extern cl::opt<bool> c_fswsp;
extern cl::opt<bool> c_sdsp;

///////////////////////////////////////

// bps
// rv32i
extern cl::opt<std::string> b_p_lui;
extern cl::opt<std::string> b_p_auipc;
extern cl::opt<std::string> b_p_jal;
extern cl::opt<std::string> b_p_jalr;
extern cl::opt<std::string> b_p_beq;
extern cl::opt<std::string> b_p_bne;
extern cl::opt<std::string> b_p_blt;
extern cl::opt<std::string> b_p_bge;
extern cl::opt<std::string> b_p_bltu;
extern cl::opt<std::string> b_p_bgeu;
extern cl::opt<std::string> b_p_lb;
extern cl::opt<std::string> b_p_lh;
extern cl::opt<std::string> b_p_lw;
extern cl::opt<std::string> b_p_lbu;
extern cl::opt<std::string> b_p_lhu;
extern cl::opt<std::string> b_p_sb;
extern cl::opt<std::string> b_p_sh;
extern cl::opt<std::string> b_p_sw;
extern cl::opt<std::string> b_p_addi;
extern cl::opt<std::string> b_p_slti;
extern cl::opt<std::string> b_p_sltiu;
extern cl::opt<std::string> b_p_xori;
extern cl::opt<std::string> b_p_ori;
extern cl::opt<std::string> b_p_andi;
extern cl::opt<std::string> b_p_slli;
extern cl::opt<std::string> b_p_srli;
extern cl::opt<std::string> b_p_srai;
extern cl::opt<std::string> b_p_add;
extern cl::opt<std::string> b_p_sub;
extern cl::opt<std::string> b_p_sll;
extern cl::opt<std::string> b_p_slt;
extern cl::opt<std::string> b_p_sltu;
extern cl::opt<std::string> b_p_xor_;
extern cl::opt<std::string> b_p_srl;
extern cl::opt<std::string> b_p_sra;
extern cl::opt<std::string> b_p_or_;
extern cl::opt<std::string> b_p_and_;
extern cl::opt<std::string> b_p_fence;
extern cl::opt<std::string> b_p_fence_i;
extern cl::opt<std::string> b_p_ecall;
extern cl::opt<std::string> b_p_ebreak;
extern cl::opt<std::string> b_p_csrrw;
extern cl::opt<std::string> b_p_csrrs;
extern cl::opt<std::string> b_p_csrrc;
extern cl::opt<std::string> b_p_csrrwi;
extern cl::opt<std::string> b_p_csrrsi;
extern cl::opt<std::string> b_p_csrrci;

// rv64i
extern cl::opt<std::string> b_p_lwu;
extern cl::opt<std::string> b_p_ld;
extern cl::opt<std::string> b_p_sd;
//extern cl::opt<std::string> b_p_slli;
//extern cl::opt<std::string> b_p_srli;
//extern cl::opt<std::string> b_p_srai;
extern cl::opt<std::string> b_p_addiw;
extern cl::opt<std::string> b_p_slliw;
extern cl::opt<std::string> b_p_srliw;
extern cl::opt<std::string> b_p_sraiw;
extern cl::opt<std::string> b_p_addw;
extern cl::opt<std::string> b_p_subw;
extern cl::opt<std::string> b_p_sllw;
extern cl::opt<std::string> b_p_srlw;
extern cl::opt<std::string> b_p_sraw;

// rv32m
extern cl::opt<std::string> b_p_mul;
extern cl::opt<std::string> b_p_mulh; 
extern cl::opt<std::string> b_p_mulhsu;   
extern cl::opt<std::string> b_p_mulhu;  
extern cl::opt<std::string> b_p_div_;
extern cl::opt<std::string> b_p_divu; 
extern cl::opt<std::string> b_p_rem;
extern cl::opt<std::string> b_p_remu;


// rv64m
extern cl::opt<std::string> b_p_mulw;
extern cl::opt<std::string> b_p_divw;
extern cl::opt<std::string> b_p_divuw; 
extern cl::opt<std::string> b_p_remw;
extern cl::opt<std::string> b_p_remuw;

// rv32a
extern cl::opt<std::string> b_p_lr_w;
extern cl::opt<std::string> b_p_sc_w;
extern cl::opt<std::string> b_p_amoswap_w;   
extern cl::opt<std::string> b_p_amoadd_w;  
extern cl::opt<std::string> b_p_amoxor_w;  
extern cl::opt<std::string> b_p_amoand_w;  
extern cl::opt<std::string> b_p_amoor_w; 
extern cl::opt<std::string> b_p_amomin_w;  
extern cl::opt<std::string> b_p_amomax_w;  
extern cl::opt<std::string> b_p_amominu_w;   
extern cl::opt<std::string> b_p_amomaxu_w;

// rv64a
extern cl::opt<std::string> b_p_lr_d;
extern cl::opt<std::string> b_p_sc_d;
extern cl::opt<std::string> b_p_amoswap_d;
extern cl::opt<std::string> b_p_amoadd_d;
extern cl::opt<std::string> b_p_amoxor_d;
extern cl::opt<std::string> b_p_amoand_d;
extern cl::opt<std::string> b_p_amoor_d;
extern cl::opt<std::string> b_p_amomin_d;
extern cl::opt<std::string> b_p_amomax_d;
extern cl::opt<std::string> b_p_amominu_d;
extern cl::opt<std::string> b_p_amomaxu_d;

// rv32f
extern cl::opt<std::string> b_p_flw;
extern cl::opt<std::string> b_p_fsw;
extern cl::opt<std::string> b_p_fmadd_s;
extern cl::opt<std::string> b_p_fmsub_s;
extern cl::opt<std::string> b_p_fnmsub_s;
extern cl::opt<std::string> b_p_fnmadd_s;
extern cl::opt<std::string> b_p_fadd_s;
extern cl::opt<std::string> b_p_fsub_s;
extern cl::opt<std::string> b_p_fmul_s;
extern cl::opt<std::string> b_p_fdiv_s;
extern cl::opt<std::string> b_p_fsqrt_s;
extern cl::opt<std::string> b_p_fsgnj_s;
extern cl::opt<std::string> b_p_fsgnjn_s;
extern cl::opt<std::string> b_p_fsgnjx_s;
extern cl::opt<std::string> b_p_fmin_s;
extern cl::opt<std::string> b_p_fmax_s;
extern cl::opt<std::string> b_p_fcvt_w_s;
extern cl::opt<std::string> b_p_fcvt_wu_s;
extern cl::opt<std::string> b_p_fmv_x_w;
extern cl::opt<std::string> b_p_feq_s;
extern cl::opt<std::string> b_p_flt_s;
extern cl::opt<std::string> b_p_fle_s;
extern cl::opt<std::string> b_p_fclass_s;
extern cl::opt<std::string> b_p_fcvt_s_w;
extern cl::opt<std::string> b_p_fcvt_s_wu;
extern cl::opt<std::string> b_p_fmv_w_x;

// rv64f
extern cl::opt<std::string> b_p_fcvt_l_s;
extern cl::opt<std::string> b_p_fcvt_lu_s;
extern cl::opt<std::string> b_p_fcvt_s_l;
extern cl::opt<std::string> b_p_fcvt_s_lu;

// rv32d
extern cl::opt<std::string> b_p_fld;
extern cl::opt<std::string> b_p_fsd;
extern cl::opt<std::string> b_p_fmadd_d;
extern cl::opt<std::string> b_p_fmsub_d;
extern cl::opt<std::string> b_p_fnmsub_d;
extern cl::opt<std::string> b_p_fnmadd_d;
extern cl::opt<std::string> b_p_fadd_d;
extern cl::opt<std::string> b_p_fsub_d;
extern cl::opt<std::string> b_p_fmul_d;
extern cl::opt<std::string> b_p_fdiv_d;
extern cl::opt<std::string> b_p_fsqrt_d;
extern cl::opt<std::string> b_p_fsgnj_d;
extern cl::opt<std::string> b_p_fsgnjn_d;
extern cl::opt<std::string> b_p_fsgnjx_d;
extern cl::opt<std::string> b_p_fmin_d;
extern cl::opt<std::string> b_p_fmax_d;
extern cl::opt<std::string> b_p_fcvt_s_d;
extern cl::opt<std::string> b_p_fcvt_d_s;
extern cl::opt<std::string> b_p_feq_d;
extern cl::opt<std::string> b_p_flt_d;
extern cl::opt<std::string> b_p_fle_d;
extern cl::opt<std::string> b_p_fclass_d;
extern cl::opt<std::string> b_p_fcvt_w_d;
extern cl::opt<std::string> b_p_fcvt_wu_d;
extern cl::opt<std::string> b_p_fcvt_d_w;
extern cl::opt<std::string> b_p_fcvt_d_wu;

// rv64d
extern cl::opt<std::string> b_p_fcvt_l_d;
extern cl::opt<std::string> b_p_fcvt_lu_d;
extern cl::opt<std::string> b_p_fmv_x_d;
extern cl::opt<std::string> b_p_fcvt_d_l;
extern cl::opt<std::string> b_p_fcvt_d_lu;
extern cl::opt<std::string> b_p_fmv_d_x;

// rv32q
extern cl::opt<std::string> b_p_flq;
extern cl::opt<std::string> b_p_fsq;
extern cl::opt<std::string> b_p_fmadd_q;
extern cl::opt<std::string> b_p_fmsub_q;
extern cl::opt<std::string> b_p_fnmsub_q;
extern cl::opt<std::string> b_p_fnmadd_q;
extern cl::opt<std::string> b_p_fadd_q;
extern cl::opt<std::string> b_p_fsub_q;
extern cl::opt<std::string> b_p_fmul_q;
extern cl::opt<std::string> b_p_fdiv_q;
extern cl::opt<std::string> b_p_fsqrt_q;
extern cl::opt<std::string> b_p_fsgnj_q;
extern cl::opt<std::string> b_p_fsgnjn_q;
extern cl::opt<std::string> b_p_fsgnjx_q;
extern cl::opt<std::string> b_p_fmin_q;
extern cl::opt<std::string> b_p_fmax_q;
extern cl::opt<std::string> b_p_fcvt_s_q;
extern cl::opt<std::string> b_p_fcvt_q_s;
extern cl::opt<std::string> b_p_fcvt_d_q;
extern cl::opt<std::string> b_p_fcvt_q_d;
extern cl::opt<std::string> b_p_feq_q;
extern cl::opt<std::string> b_p_flt_q;
extern cl::opt<std::string> b_p_fle_q;
extern cl::opt<std::string> b_p_fclass_q;
extern cl::opt<std::string> b_p_fcvt_w_q;
extern cl::opt<std::string> b_p_fcvt_wu_q;
extern cl::opt<std::string> b_p_fcvt_q_w;
extern cl::opt<std::string> b_p_fcvt_q_wu;

// rv64q
extern cl::opt<std::string> b_p_fcvt_l_q;
extern cl::opt<std::string> b_p_fcvt_lu_q;
extern cl::opt<std::string> b_p_fcvt_q_l;
extern cl::opt<std::string> b_p_fcvt_q_lu;

// rvc quadrant 0
extern cl::opt<std::string> b_p_c_addi4spn;
extern cl::opt<std::string> b_p_c_fld;
extern cl::opt<std::string> b_p_c_lq;
extern cl::opt<std::string> b_p_c_lw;
extern cl::opt<std::string> b_p_c_flw;
extern cl::opt<std::string> b_p_c_ld;
extern cl::opt<std::string> b_p_c_fsd;
extern cl::opt<std::string> b_p_c_sq;
extern cl::opt<std::string> b_p_c_sw;
extern cl::opt<std::string> b_p_c_fsw;
extern cl::opt<std::string> b_p_c_sd;

// rvc quadrant 1
extern cl::opt<std::string> b_p_c_nop;
extern cl::opt<std::string> b_p_c_addi;
extern cl::opt<std::string> b_p_c_jal;
extern cl::opt<std::string> b_p_c_addiw;
extern cl::opt<std::string> b_p_c_li;
extern cl::opt<std::string> b_p_c_addi16sp;
extern cl::opt<std::string> b_p_c_lui;
extern cl::opt<std::string> b_p_c_srli;
extern cl::opt<std::string> b_p_c_srli64;
extern cl::opt<std::string> b_p_c_srai;
extern cl::opt<std::string> b_p_c_srai64;
extern cl::opt<std::string> b_p_c_andi;
extern cl::opt<std::string> b_p_c_sub;
extern cl::opt<std::string> b_p_c_xor;
extern cl::opt<std::string> b_p_c_or;
extern cl::opt<std::string> b_p_c_and;
extern cl::opt<std::string> b_p_c_subw;
extern cl::opt<std::string> b_p_c_addw;
extern cl::opt<std::string> b_p_c_j;
extern cl::opt<std::string> b_p_c_beqz;
extern cl::opt<std::string> b_p_c_bnez;

// rvc quadrant 2
extern cl::opt<std::string> b_p_c_slli;
extern cl::opt<std::string> b_p_c_slli64;
extern cl::opt<std::string> b_p_c_fldsp;
extern cl::opt<std::string> b_p_c_lqsp;
extern cl::opt<std::string> b_p_c_lwsp;
extern cl::opt<std::string> b_p_c_flwsp;
extern cl::opt<std::string> b_p_c_ldsp;
extern cl::opt<std::string> b_p_c_jr;
extern cl::opt<std::string> b_p_c_mv;
extern cl::opt<std::string> b_p_c_ebreak;
extern cl::opt<std::string> b_p_c_jalr;
extern cl::opt<std::string> b_p_c_add;
extern cl::opt<std::string> b_p_c_fsdsp;
extern cl::opt<std::string> b_p_c_sqsp;
extern cl::opt<std::string> b_p_c_swsp;
extern cl::opt<std::string> b_p_c_fswsp;
extern cl::opt<std::string> b_p_c_sdsp;

//////////////////////////////////////

extern cl::opt<std::string> enckeyall;

// if encrypt true for an instruction xor with key
extern cl::opt<std::string> ienc32key; // 32 bit key for 32 bit instructions (rv32i extension)
extern cl::opt<std::string> menc32key; // 32 bit key for 32 bit instructions (rv32m extension)
extern cl::opt<std::string> aenc32key; // 32 bit key for 32 bit instructions (rv32a extension)
extern cl::opt<std::string> fenc32key; // 32 bit key for 32 bit instructions (rv32f extension)
extern cl::opt<std::string> denc32key; // 32 bit key for 32 bit instructions (rv32d extension)
extern cl::opt<std::string> qenc32key; // 32 bit key for 32 bit instructions (rv32q extension)

extern cl::opt<std::string> ienc64key; // 32 bit key for 32 bit instructions (rv64i extension) (64 bit riscv has 32 bit instructions)
extern cl::opt<std::string> menc64key; // 32 bit key for 32 bit instructions (rv64m extension) (64 bit riscv has 32 bit instructions)
extern cl::opt<std::string> aenc64key; // 32 bit key for 32 bit instructions (rv64a extension) (64 bit riscv has 32 bit instructions)
extern cl::opt<std::string> fenc64key; // 32 bit key for 32 bit instructions (rv64f extension) (64 bit riscv has 32 bit instructions)
extern cl::opt<std::string> denc64key; // 32 bit key for 32 bit instructions (rv64d extension) (64 bit riscv has 32 bit instructions)
extern cl::opt<std::string> qenc64key; // 32 bit key for 32 bit instructions (rv64q extension) (64 bit riscv has 32 bit instructions)

extern cl::opt<std::string> cencq0key; // 16 bit key for 16 bit compressed instructions (rvc quadrant 0 extension)
extern cl::opt<std::string> cencq1key; // 16 bit key for 16 bit compressed instructions (rvc quadrant 1 extension)
extern cl::opt<std::string> cencq2key; // 16 bit key for 16 bit compressed instructions (rvc quadrant 2 extension)


// give instructions as list like 0000010000100 -> corresponding to 1 instructions will be encrypted with key
extern cl::opt<std::string> ienc32insts; // 47 bit
extern cl::opt<std::string> menc32insts; // 8 bit
extern cl::opt<std::string> aenc32insts; // 11 bit
extern cl::opt<std::string> fenc32insts; // 26 bit
extern cl::opt<std::string> denc32insts; // 26 bit
extern cl::opt<std::string> qenc32insts; // 28 bit

extern cl::opt<std::string> ienc64insts; // 12bit // // 15 bit
extern cl::opt<std::string> menc64insts; // 5 bit
extern cl::opt<std::string> aenc64insts; // 11 bit
extern cl::opt<std::string> fenc64insts; // 4 bit
extern cl::opt<std::string> denc64insts; // 6 bit
extern cl::opt<std::string> qenc64insts; // 4 bit

extern cl::opt<std::string> cencq0insts; // 11 bit
extern cl::opt<std::string> cencq1insts; // 21 bit
extern cl::opt<std::string> cencq2insts; // 17 bit

//////////////////////////////////////

extern StringSet<> FoundSectionSet;

typedef std::function<bool(llvm::object::SectionRef const &)> FilterPredicate;

/// A filtered iterator for SectionRefs that skips sections based on some given
/// predicate.
class SectionFilterIterator {
public:
  SectionFilterIterator(FilterPredicate P,
                        llvm::object::section_iterator const &I,
                        llvm::object::section_iterator const &E)
      : Predicate(std::move(P)), Iterator(I), End(E) {
    ScanPredicate();
  }
  const llvm::object::SectionRef &operator*() const { return *Iterator; }
  SectionFilterIterator &operator++() {
    ++Iterator;
    ScanPredicate();
    return *this;
  }
  bool operator!=(SectionFilterIterator const &Other) const {
    return Iterator != Other.Iterator;
  }

private:
  void ScanPredicate() {
    while (Iterator != End && !Predicate(*Iterator)) {
      ++Iterator;
    }
  }
  FilterPredicate Predicate;
  llvm::object::section_iterator Iterator;
  llvm::object::section_iterator End;
};

/// Creates an iterator range of SectionFilterIterators for a given Object and
/// predicate.
class SectionFilter {
public:
  SectionFilter(FilterPredicate P, llvm::object::ObjectFile const &O)
      : Predicate(std::move(P)), Object(O) {}
  SectionFilterIterator begin() {
    return SectionFilterIterator(Predicate, Object.section_begin(),
                                 Object.section_end());
  }
  SectionFilterIterator end() {
    return SectionFilterIterator(Predicate, Object.section_end(),
                                 Object.section_end());
  }

private:
  FilterPredicate Predicate;
  llvm::object::ObjectFile const &Object;
};

// Various helper functions.

/// Creates a SectionFilter with a standard predicate that conditionally skips
/// sections when the --section objdump flag is provided.
///
/// Idx is an optional output parameter that keeps track of which section index
/// this is. This may be different than the actual section number, as some
/// sections may be filtered (e.g. symbol tables).
SectionFilter ToolSectionFilter(llvm::object::ObjectFile const &O,
                                uint64_t *Idx = nullptr);

bool isRelocAddressLess(object::RelocationRef A, object::RelocationRef B);
void printRelocations(const object::ObjectFile *O);
void printDynamicRelocations(const object::ObjectFile *O);
void printSectionHeaders(const object::ObjectFile *O);
void printSectionContents(const object::ObjectFile *O);
void printSymbolTable(const object::ObjectFile *O, StringRef ArchiveName,
                      StringRef ArchitectureName = StringRef(),
                      bool DumpDynamic = false);
void printSymbol(const object::ObjectFile *O, const object::SymbolRef &Symbol,
                 StringRef FileName, StringRef ArchiveName,
                 StringRef ArchitectureName, bool DumpDynamic);
LLVM_ATTRIBUTE_NORETURN void reportError(StringRef File, Twine Message);
LLVM_ATTRIBUTE_NORETURN void reportError(Error E, StringRef FileName,
                                         StringRef ArchiveName = "",
                                         StringRef ArchitectureName = "");
void reportWarning(Twine Message, StringRef File);

template <typename T, typename... Ts>
T unwrapOrError(Expected<T> EO, Ts &&... Args) {
  if (EO)
    return std::move(*EO);
  reportError(EO.takeError(), std::forward<Ts>(Args)...);
}

std::string getFileNameForError(const object::Archive::Child &C,
                                unsigned Index);
SymbolInfoTy createSymbolInfo(const object::ObjectFile *Obj,
                              const object::SymbolRef &Symbol);

} // namespace objdump
} // end namespace llvm

#endif
