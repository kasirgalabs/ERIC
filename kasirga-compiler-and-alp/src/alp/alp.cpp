//===-- alp.cpp - Object file dumping utility for llvm -----------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This program is a utility that works like binutils "objdump", that is, it
// dumps out a plethora of information about an object file depending on the
// flags.
//
// The flags and output of this program should be near identical to those of
// binutils objdump.
//
//===----------------------------------------------------------------------===//

// her seyin basi     Name = unwrapOrError(Symbol.getName(), FileName, ArchiveName, ArchitectureName); 
//#include "MCTargetDesc/RISCVBaseInfo.h"
//#include "MCTargetDesc/RISCVFixupKinds.h"
//#include "MCTargetDesc/RISCVMCExpr.h"
//#include "MCTargetDesc/RISCVMCTargetDesc.h"
#include "MCTargetDesc/RISCVInstPrinter.h"
#include "RISCV.h"
//#include "RISCVTargetMachine.h"
#include "llvm/IR/Instruction.h"
#include <stdlib.h> // ekledim
#include "alp.h"
#include "COFFDump.h"
#include "ELFDump.h"
#include "MachODump.h"
#include "WasmDump.h"
#include "XCOFFDump.h"
#include "llvm/ADT/IndexedMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SetOperations.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSet.h"
#include "llvm/ADT/Triple.h"
#include "llvm/CodeGen/FaultMaps.h"
#include "llvm/DebugInfo/DWARF/DWARFContext.h"
#include "llvm/DebugInfo/Symbolize/Symbolize.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCDisassembler/MCRelocationInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/MC/MCInstrAnalysis.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCObjectFileInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCTargetOptions.h"
#include "llvm/Object/Archive.h"
#include "llvm/Object/COFF.h"
#include "llvm/Object/COFFImportFile.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/MachO.h"
#include "llvm/Object/MachOUniversal.h"
#include "llvm/Object/ObjectFile.h"
#include "llvm/Object/Wasm.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Errc.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/InitLLVM.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/StringSaver.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/WithColor.h"
#include "llvm/Support/raw_ostream.h"
#include <algorithm>
#include <cctype>
#include <cstring>
#include <system_error>
#include <unordered_map>
#include <utility>
#include <fstream>
#include <string>

#include <bitset>
#include <iomanip>
#include <sstream>
//////////////////////
llvm::StringRef opcodeList[90] = {
"beq"               
,"bne"               
,"blt"               
,"bge"               
,"bltu"              
,"bgeu"              
,"jalr"              
,"jal"               
,"lui"               
,"auipc"             
,"addi"              
,"slli"              
,"slti"              
,"sltiu"             
,"xori"              
,"srli"              
,"srai"              
,"ori"               
,"andi"              
,"add"               
,"sub"               
,"sll"               
,"slt"               
,"sltu"              
,"xor"               
,"srl"               
,"sra"               
,"or"                
,"and"               
,"addiw"             
,"slliw"             
,"srliw"             
,"sraiw"             
,"addw"              
,"subw"              
,"sllw"              
,"srlw"              
,"sraw"              
,"lb"                
,"lh"                
,"lw"                
,"ld"                
,"lbu"               
,"lhu"               
,"lwu"               
,"sb"                
,"sh"                
,"sw"                
,"sd"                
,"fence"             
,"fence_i"           
,"mul"               
,"mulh"              
,"mulhsu"            
,"mulhu"             
,"div"               
,"divu"              
,"rem"               
,"remu"              
,"mulw"              
,"divw"              
,"divuw"             
,"remw"              
,"remuw"             
,"lr_w"              
,"sc_w"              
,"lr_d"              
,"sc_d"              
,"ecall"             
,"ebreak"            
,"uret"              
,"mret"              
,"dret"              
,"sfence_vma"        
,"wfi"               
,"csrrw"             
,"csrrs"             
,"csrrc"             
,"csrrwi"            
,"csrrsi"            
,"csrrci"            
,"slli_rv32"         
,"srli_rv32"         
,"srai_rv32"         
,"rdcycle"           
,"rdtime"            
,"rdinstret"         
,"rdcycleh"          
,"rdtimeh"           
,"rdinstreth"
};

llvm::StringRef opcodes_rv32i[52] = {
"lui"
,"auipc"
,"addi"
,"slti"
,"sltiu"
,"xori"
,"ori"
,"andi"
,"slli"
,"srli"
,"srai"
,"add"
,"sub"
,"sll"
,"slt"
,"sltu"
,"xor"
,"srl"
,"sra"
,"or"
,"and"
,"fence"
,"fence.i"
,"csrrw"
,"csrrs"
,"csrrc"
,"csrrwi"
,"csrrsi"
,"csrrci"
,"ecall"
,"ebreak"
,"uret"
,"sret"
,"mret"
,"wfi"
,"sfence.vma"
,"lb"
,"lh"
,"lw"
,"lbu"
,"lhu"
,"sb"
,"sh"
,"sw"
,"jal"
,"jalr"
,"beq"
,"bne"
,"blt"
,"bge"
,"bltu"
,"bgeu"};

llvm::StringRef opcodes_rv32m[8] =  {
"mul"
,"mulh"
,"mulhsu"
,"mulhu"
,"div"
,"divu"
,"rem"
,"remu"};

llvm::StringRef opcodes_rv32a[11] = {
"lr.w"
,"sc.w"
,"amoswap.w"
,"amoadd.w"
,"amoxor.w"
,"amoand.w"
,"amoor.w"
,"amomin.w"
,"amomax.w"
,"amominu.w"
,"amomaxu.w"};

llvm::StringRef opcodes_rv32f[52] = {
"fmadd.s"
,"fmsub.s"
,"fnmsub.s"
,"fnmadd.s"
,"fadd.s"
,"fsub.s"
,"fmul.s"
,"fdiv.s"
,"fsqrt.s"
,"fsgnj.s"
,"fsgnjn.s"
,"fsgnjx.s"
,"fmin.s"
,"fmax.s"
,"fcvt.w.s"
,"fcvt.wu.s"
,"fmv.x.w"
,"feq.s"
,"flt.s"
,"fle.s"
,"fclass.s"
,"fcvt.s.w"
,"fcvt.s.wu"
,"fmv.w.x"
,"fmadd.d"
,"fmsub.d"
,"fnmsub.d"
,"fnmadd.d"
,"fadd.d"
,"fsub.d"
,"fmul.d"
,"fdiv.d"
,"fsqrt.d"
,"fsgnj.d"
,"fsgnjn.d"
,"fsgnjx.d"
,"fmin.d"
,"fmax.d"
,"fcvt.s.d"
,"fcvt.d.s"
,"feq.d"
,"flt.d"
,"fle.d"
,"fclass.d"
,"fcvt.w.d"
,"fcvt.wu.d"
,"fcvt.d.w"
,"fcvt.d.wu"
,"flw"
,"fsw"
,"fld"
,"fsd"};

llvm::StringRef opcodes_rv32c[42] = {
"c.addi4spn"
,"c.fld"
,"c.lw"
,"c.flw"
,"c.ld"
,"c.fsd"
,"c.sw"
,"c.fsw"
,"c.sd"
,"c.nop"
,"c.addi"
,"c.jal"
,"c.addiw"
,"c.li"
,"c.addi16sp"
,"c.lui"
,"c.srli"
,"c.srai"
,"c.andi"
,"c.sub"
,"c.xor"
,"c.or"
,"c.and"
,"c.subw"
,"c.addw"
,"c.j"
,"c.beqz"
,"c.bnez"
,"c.slli"
,"c.fldsp"
,"c.lwsp"
,"c.flwsp"
,"c.ldsp"
,"c.jr"
,"c.mv"
,"c.ebreak"
,"c.jalr"
,"c.add"
,"c.fsdsp"
,"c.swsp"
,"c.fswsp"
,"c.sdsp"};

//////////ekleme
int g_argc;
char **g_argv;

using namespace llvm;
using namespace llvm::object;
using namespace llvm::objdump;

#define DEBUG_TYPE "objdump"

static cl::OptionCategory ObjdumpCat("llvm-objdump Options");

////////////////ekleme
cl::opt<std::string> objdump::dosya(
    "dosya",
    cl::desc(
        "if given file option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::bits(
    "bits",
    cl::desc(
        "if given bits option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::key(
    "key",
    cl::desc(
        "if given key option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::beq(
    "beq",
    cl::desc("if given beq option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bne(
    "bne",
    cl::desc("if given bne option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::blt(
    "blt",
    cl::desc("if given blt option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bge(
    "bge",
    cl::desc("if given bge option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bltu(
    "bltu",
    cl::desc("if given bltu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bgeu(
    "bgeu",
    cl::desc("if given bgeu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::jalr(
    "jalr",
    cl::desc("if given jalr option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::jal(
    "jal",
    cl::desc("if given jal option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lui(
    "lui",
    cl::desc("if given lui option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::auipc(
    "auipc",
    cl::desc("if given auipc option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::addi(
    "addi",
    cl::desc("if given addi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slli(
    "slli",
    cl::desc("if given slli option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slti(
    "slti",
    cl::desc("if given slti option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sltiu(
    "sltiu",
    cl::desc("if given sltiu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::xori(
    "xori",
    cl::desc("if given xori option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srli(
    "srli",
    cl::desc("if given srli option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srai(
    "srai",
    cl::desc("if given srai option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ori(
    "ori",
    cl::desc("if given ori option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::andi(
    "andi",
    cl::desc("if given andi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::add(
    "add",
    cl::desc("if given add option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sub(
    "sub",
    cl::desc("if given sub option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sll(
    "sll",
    cl::desc("if given sll option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slt(
    "slt",
    cl::desc("if given slt option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sltu(
    "sltu",
    cl::desc("if given sltu option"),
    cl::cat(ObjdumpCat));

// xor_
cl::opt<bool> objdump::xor_(
    "xor",
    cl::desc("if given xor_ option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srl(
    "srl",
    cl::desc("if given srl option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sra(
    "sra",
    cl::desc("if given sra option"),
    cl::cat(ObjdumpCat));

// or_
cl::opt<bool> objdump::or_(
    "or",
    cl::desc("if given or_ option"),
    cl::cat(ObjdumpCat));

// and_
cl::opt<bool> objdump::and_(
    "and",
    cl::desc("if given and_ option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::addiw(
    "addiw",
    cl::desc("if given addiw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slliw(
    "slliw",
    cl::desc("if given slliw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srliw(
    "srliw",
    cl::desc("if given srliw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sraiw(
    "sraiw",
    cl::desc("if given sraiw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::addw(
    "addw",
    cl::desc("if given addw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::subw(
    "subw",
    cl::desc("if given subw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sllw(
    "sllw",
    cl::desc("if given sllw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srlw(
    "srlw",
    cl::desc("if given srlw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sraw(
    "sraw",
    cl::desc("if given sraw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lb(
    "lb",
    cl::desc("if given lb option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lh(
    "lh",
    cl::desc("if given lh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lw(
    "lw",
    cl::desc("if given lw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ld(
    "ld",
    cl::desc("if given ld option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lbu(
    "lbu",
    cl::desc("if given lbu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lhu(
    "lhu",
    cl::desc("if given lhu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lwu(
    "lwu",
    cl::desc("if given lwu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sb(
    "sb",
    cl::desc("if given sb option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sh(
    "sh",
    cl::desc("if given sh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sw(
    "sw",
    cl::desc("if given sw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sd(
    "sd",
    cl::desc("if given sd option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fence(
    "fence",
    cl::desc("if given fence option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fence_i(
    "fence_i",
    cl::desc("if given fence_i option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mul(
    "mul",
    cl::desc("if given mul option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulh(
    "mulh",
    cl::desc("if given mulh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulhsu(
    "mulhsu",
    cl::desc("if given mulhsu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulhu(
    "mulhu",
    cl::desc("if given mulhu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::div(
    "div",
    cl::desc("if given div option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::divu(
    "divu",
    cl::desc("if given divu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rem(
    "rem",
    cl::desc("if given rem option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::remu(
    "remu",
    cl::desc("if given remu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulw(
    "mulw",
    cl::desc("if given mulw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::divw(
    "divw",
    cl::desc("if given divw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::divuw(
    "divuw",
    cl::desc("if given divuw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::remw(
    "remw",
    cl::desc("if given remw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::remuw(
    "remuw",
    cl::desc("if given remuw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lr_w(
    "lr_w",
    cl::desc("if given lr_w option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sc_w(
    "sc_w",
    cl::desc("if given sc_w option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lr_d(
    "lr_d",
    cl::desc("if given lr_d option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sc_d(
    "sc_d",
    cl::desc("if given sc_d option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ecall(
    "ecall",
    cl::desc("if given ecall option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ebreak(
    "ebreak",
    cl::desc("if given ebreak option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::uret(
    "uret",
    cl::desc("if given uret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mret(
    "mret",
    cl::desc("if given mret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::dret(
    "dret",
    cl::desc("if given dret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sfence_vma(
    "sfence_vma",
    cl::desc("if given sfence_vma option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::wfi(
    "wfi",
    cl::desc("if given wfi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrw(
    "csrrw",
    cl::desc("if given csrrw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrs(
    "csrrs",
    cl::desc("if given csrrs option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrc(
    "csrrc",
    cl::desc("if given csrrc option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrwi(
    "csrrwi",
    cl::desc("if given csrrwi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrsi(
    "csrrsi",
    cl::desc("if given csrrsi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrci(
    "csrrci",
    cl::desc("if given csrrci option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slli_rv32(
    "slli_rv32",
    cl::desc("if given slli_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srli_rv32(
    "srli_rv32",
    cl::desc("if given srli_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srai_rv32(
    "srai_rv32",
    cl::desc("if given srai_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rdcycle(
    "rdcycle",
    cl::desc("if given rdcycle option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rdtime(
    "rdtime",
    cl::desc("if given rdtime option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rdinstret(
    "rdinstret",
    cl::desc("if given rdinstret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rdcycleh(
    "rdcycleh",
    cl::desc("if given rdcycleh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rdtimeh(
    "rdtimeh",
    cl::desc("if given rdtimeh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rdinstreth(
    "rdinstreth",
    cl::desc("if given rdinstreth option"),
    cl::cat(ObjdumpCat));

///////////////////////////////// pler

cl::opt<bool> objdump::p_beq(
    "p_beq",
    cl::desc("if given beq option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_bne(
    "p_bne",
    cl::desc("if given bne option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_blt(
    "p_blt",
    cl::desc("if given blt option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_bge(
    "p_bge",
    cl::desc("if given bge option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_bltu(
    "p_bltu",
    cl::desc("if given bltu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_bgeu(
    "p_bgeu",
    cl::desc("if given bgeu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_jalr(
    "p_jalr",
    cl::desc("if given jalr option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_jal(
    "p_jal",
    cl::desc("if given jal option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lui(
    "p_lui",
    cl::desc("if given lui option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_auipc(
    "p_auipc",
    cl::desc("if given auipc option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_addi(
    "p_addi",
    cl::desc("if given addi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_slli(
    "p_slli",
    cl::desc("if given slli option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_slti(
    "p_slti",
    cl::desc("if given slti option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sltiu(
    "p_sltiu",
    cl::desc("if given sltiu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_xori(
    "p_xori",
    cl::desc("if given xori option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srli(
    "p_srli",
    cl::desc("if given srli option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srai(
    "p_srai",
    cl::desc("if given srai option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_ori(
    "p_ori",
    cl::desc("if given ori option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_andi(
    "p_andi",
    cl::desc("if given andi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_add(
    "p_add",
    cl::desc("if given add option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sub(
    "p_sub",
    cl::desc("if given sub option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sll(
    "p_sll",
    cl::desc("if given sll option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_slt(
    "p_slt",
    cl::desc("if given slt option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sltu(
    "p_sltu",
    cl::desc("if given sltu option"),
    cl::cat(ObjdumpCat));

// xor_
cl::opt<bool> objdump::p_xor_(
    "p_xor",
    cl::desc("if given xor_ option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srl(
    "p_srl",
    cl::desc("if given srl option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sra(
    "p_sra",
    cl::desc("if given sra option"),
    cl::cat(ObjdumpCat));

// or_
cl::opt<bool> objdump::p_or_(
    "p_or",
    cl::desc("if given or_ option"),
    cl::cat(ObjdumpCat));

// and_
cl::opt<bool> objdump::p_and_(
    "p_and",
    cl::desc("if given and_ option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_addiw(
    "p_addiw",
    cl::desc("if given addiw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_slliw(
    "p_slliw",
    cl::desc("if given slliw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srliw(
    "p_srliw",
    cl::desc("if given srliw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sraiw(
    "p_sraiw",
    cl::desc("if given sraiw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_addw(
    "p_addw",
    cl::desc("if given addw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_subw(
    "p_subw",
    cl::desc("if given subw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sllw(
    "p_sllw",
    cl::desc("if given sllw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srlw(
    "p_srlw",
    cl::desc("if given srlw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sraw(
    "p_sraw",
    cl::desc("if given sraw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lb(
    "p_lb",
    cl::desc("if given lb option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lh(
    "p_lh",
    cl::desc("if given lh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lw(
    "p_lw",
    cl::desc("if given lw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_ld(
    "p_ld",
    cl::desc("if given ld option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lbu(
    "p_lbu",
    cl::desc("if given lbu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lhu(
    "p_lhu",
    cl::desc("if given lhu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lwu(
    "p_lwu",
    cl::desc("if given lwu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sb(
    "p_sb",
    cl::desc("if given sb option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sh(
    "p_sh",
    cl::desc("if given sh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sw(
    "p_sw",
    cl::desc("if given sw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sd(
    "p_sd",
    cl::desc("if given sd option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_fence(
    "p_fence",
    cl::desc("if given fence option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_fence_i(
    "p_fence_i",
    cl::desc("if given fence_i option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_mul(
    "p_mul",
    cl::desc("if given mul option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_mulh(
    "p_mulh",
    cl::desc("if given mulh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_mulhsu(
    "p_mulhsu",
    cl::desc("if given mulhsu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_mulhu(
    "p_mulhu",
    cl::desc("if given mulhu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_div(
    "p_div",
    cl::desc("if given div option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_divu(
    "p_divu",
    cl::desc("if given divu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rem(
    "p_rem",
    cl::desc("if given rem option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_remu(
    "p_remu",
    cl::desc("if given remu option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_mulw(
    "p_mulw",
    cl::desc("if given mulw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_divw(
    "p_divw",
    cl::desc("if given divw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_divuw(
    "p_divuw",
    cl::desc("if given divuw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_remw(
    "p_remw",
    cl::desc("if given remw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_remuw(
    "p_remuw",
    cl::desc("if given remuw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lr_w(
    "p_lr_w",
    cl::desc("if given lr_w option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sc_w(
    "p_sc_w",
    cl::desc("if given sc_w option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_lr_d(
    "p_lr_d",
    cl::desc("if given lr_d option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sc_d(
    "p_sc_d",
    cl::desc("if given sc_d option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_ecall(
    "p_ecall",
    cl::desc("if given ecall option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_ebreak(
    "p_ebreak",
    cl::desc("if given ebreak option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_uret(
    "p_uret",
    cl::desc("if given uret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_mret(
    "p_mret",
    cl::desc("if given mret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_dret(
    "p_dret",
    cl::desc("if given dret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_sfence_vma(
    "p_sfence_vma",
    cl::desc("if given sfence_vma option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_wfi(
    "p_wfi",
    cl::desc("if given wfi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_csrrw(
    "p_csrrw",
    cl::desc("if given csrrw option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_csrrs(
    "p_csrrs",
    cl::desc("if given csrrs option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_csrrc(
    "p_csrrc",
    cl::desc("if given csrrc option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_csrrwi(
    "p_csrrwi",
    cl::desc("if given csrrwi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_csrrsi(
    "p_csrrsi",
    cl::desc("if given csrrsi option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_csrrci(
    "p_csrrci",
    cl::desc("if given csrrci option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_slli_rv32(
    "p_slli_rv32",
    cl::desc("if given slli_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srli_rv32(
    "p_srli_rv32",
    cl::desc("if given srli_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_srai_rv32(
    "p_srai_rv32",
    cl::desc("if given srai_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rdcycle(
    "p_rdcycle",
    cl::desc("if given rdcycle option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rdtime(
    "p_rdtime",
    cl::desc("if given rdtime option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rdinstret(
    "p_rdinstret",
    cl::desc("if given rdinstret option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rdcycleh(
    "p_rdcycleh",
    cl::desc("if given rdcycleh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rdtimeh(
    "p_rdtimeh",
    cl::desc("if given rdtimeh option"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::p_rdinstreth(
    "p_rdinstreth",
    cl::desc("if given rdinstreth option"),
    cl::cat(ObjdumpCat));

//////////////////////////////// bpler

cl::opt<std::string> objdump::b_p_beq(
    "b_p_beq",
    cl::desc("if given beq option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bne(
    "b_p_bne",
    cl::desc("if given bne option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_blt(
    "b_p_blt",
    cl::desc("if given blt option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bge(
    "b_p_bge",
    cl::desc("if given bge option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bltu(
    "b_p_bltu",
    cl::desc("if given bltu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bgeu(
    "b_p_bgeu",
    cl::desc("if given bgeu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_jalr(
    "b_p_jalr",
    cl::desc("if given jalr option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_jal(
    "b_p_jal",
    cl::desc("if given jal option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lui(
    "b_p_lui",
    cl::desc("if given lui option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_auipc(
    "b_p_auipc",
    cl::desc("if given auipc option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_addi(
    "b_p_addi",
    cl::desc("if given addi option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slli(
    "b_p_slli",
    cl::desc("if given slli option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slti(
    "b_p_slti",
    cl::desc("if given slti option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sltiu(
    "b_p_sltiu",
    cl::desc("if given sltiu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_xori(
    "b_p_xori",
    cl::desc("if given xori option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srli(
    "b_p_srli",
    cl::desc("if given srli option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srai(
    "b_p_srai",
    cl::desc("if given srai option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ori(
    "b_p_ori",
    cl::desc("if given ori option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_andi(
    "b_p_andi",
    cl::desc("if given andi option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_add(
    "b_p_add",
    cl::desc("if given add option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sub(
    "b_p_sub",
    cl::desc("if given sub option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sll(
    "b_p_sll",
    cl::desc("if given sll option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slt(
    "b_p_slt",
    cl::desc("if given slt option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sltu(
    "b_p_sltu",
    cl::desc("if given sltu option"),
    cl::cat(ObjdumpCat));

// xor_
cl::opt<std::string> objdump::b_p_xor_(
    "b_p_xor",
    cl::desc("if given xor_ option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srl(
    "b_p_srl",
    cl::desc("if given srl option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sra(
    "b_p_sra",
    cl::desc("if given sra option"),
    cl::cat(ObjdumpCat));

// or_
cl::opt<std::string> objdump::b_p_or_(
    "b_p_or",
    cl::desc("if given or_ option"),
    cl::cat(ObjdumpCat));

// and_
cl::opt<std::string> objdump::b_p_and_(
    "b_p_and",
    cl::desc("if given and_ option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_addiw(
    "b_p_addiw",
    cl::desc("if given addiw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slliw(
    "b_p_slliw",
    cl::desc("if given slliw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srliw(
    "b_p_srliw",
    cl::desc("if given srliw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sraiw(
    "b_p_sraiw",
    cl::desc("if given sraiw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_addw(
    "b_p_addw",
    cl::desc("if given addw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_subw(
    "b_p_subw",
    cl::desc("if given subw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sllw(
    "b_p_sllw",
    cl::desc("if given sllw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srlw(
    "b_p_srlw",
    cl::desc("if given srlw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sraw(
    "b_p_sraw",
    cl::desc("if given sraw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lb(
    "b_p_lb",
    cl::desc("if given lb option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lh(
    "b_p_lh",
    cl::desc("if given lh option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lw(
    "b_p_lw",
    cl::desc("if given lw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ld(
    "b_p_ld",
    cl::desc("if given ld option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lbu(
    "b_p_lbu",
    cl::desc("if given lbu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lhu(
    "b_p_lhu",
    cl::desc("if given lhu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lwu(
    "b_p_lwu",
    cl::desc("if given lwu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sb(
    "b_p_sb",
    cl::desc("if given sb option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sh(
    "b_p_sh",
    cl::desc("if given sh option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sw(
    "b_p_sw",
    cl::desc("if given sw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sd(
    "b_p_sd",
    cl::desc("if given sd option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fence(
    "b_p_fence",
    cl::desc("if given fence option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fence_i(
    "b_p_fence_i",
    cl::desc("if given fence_i option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mul(
    "b_p_mul",
    cl::desc("if given mul option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulh(
    "b_p_mulh",
    cl::desc("if given mulh option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulhsu(
    "b_p_mulhsu",
    cl::desc("if given mulhsu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulhu(
    "b_p_mulhu",
    cl::desc("if given mulhu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_div(
    "b_p_div",
    cl::desc("if given div option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_divu(
    "b_p_divu",
    cl::desc("if given divu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rem(
    "b_p_rem",
    cl::desc("if given rem option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_remu(
    "b_p_remu",
    cl::desc("if given remu option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulw(
    "b_p_mulw",
    cl::desc("if given mulw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_divw(
    "b_p_divw",
    cl::desc("if given divw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_divuw(
    "b_p_divuw",
    cl::desc("if given divuw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_remw(
    "b_p_remw",
    cl::desc("if given remw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_remuw(
    "b_p_remuw",
    cl::desc("if given remuw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lr_w(
    "b_p_lr_w",
    cl::desc("if given lr_w option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sc_w(
    "b_p_sc_w",
    cl::desc("if given sc_w option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lr_d(
    "b_p_lr_d",
    cl::desc("if given lr_d option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sc_d(
    "b_p_sc_d",
    cl::desc("if given sc_d option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ecall(
    "b_p_ecall",
    cl::desc("if given ecall option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ebreak(
    "b_p_ebreak",
    cl::desc("if given ebreak option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_uret(
    "b_p_uret",
    cl::desc("if given uret option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mret(
    "b_p_mret",
    cl::desc("if given mret option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_dret(
    "b_p_dret",
    cl::desc("if given dret option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sfence_vma(
    "b_p_sfence_vma",
    cl::desc("if given sfence_vma option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_wfi(
    "b_p_wfi",
    cl::desc("if given wfi option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrw(
    "b_p_csrrw",
    cl::desc("if given csrrw option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrs(
    "b_p_csrrs",
    cl::desc("if given csrrs option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrc(
    "b_p_csrrc",
    cl::desc("if given csrrc option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrwi(
    "b_p_csrrwi",
    cl::desc("if given csrrwi option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrsi(
    "b_p_csrrsi",
    cl::desc("if given csrrsi option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrci(
    "b_p_csrrci",
    cl::desc("if given csrrci option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slli_rv32(
    "b_p_slli_rv32",
    cl::desc("if given slli_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srli_rv32(
    "b_p_srli_rv32",
    cl::desc("if given srli_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srai_rv32(
    "b_p_srai_rv32",
    cl::desc("if given srai_rv32 option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rdcycle(
    "b_p_rdcycle",
    cl::desc("if given rdcycle option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rdtime(
    "b_p_rdtime",
    cl::desc("if given rdtime option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rdinstret(
    "b_p_rdinstret",
    cl::desc("if given rdinstret option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rdcycleh(
    "b_p_rdcycleh",
    cl::desc("if given rdcycleh option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rdtimeh(
    "b_p_rdtimeh",
    cl::desc("if given rdtimeh option"),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rdinstreth(
    "b_p_rdinstreth",
    cl::desc("if given rdinstreth option"),
    cl::cat(ObjdumpCat));

///////////////////////////////

//cl::opt<bool> objdump::add(
//    "add",
//    cl::desc("if given add option"),
//    cl::cat(ObjdumpCat));
//// --add olarak kullanılıyor
//
//cl::opt<bool> objdump::sub(
//    "sub",
//    cl::desc("if given sub option"),
//    cl::cat(ObjdumpCat));






static cl::opt<uint64_t> AdjustVMA(
    "adjust-vma",
    cl::desc("Increase the displayed address by the specified offset"),
    cl::value_desc("offset"), cl::init(0), cl::cat(ObjdumpCat));

static cl::opt<bool>
    AllHeaders("all-headers",
               cl::desc("Display all available header information"),
               cl::cat(ObjdumpCat));
static cl::alias AllHeadersShort("x", cl::desc("Alias for --all-headers"),
                                 cl::NotHidden, cl::Grouping,
                                 cl::aliasopt(AllHeaders));

static cl::opt<std::string>
    ArchName("arch-name",
             cl::desc("Target arch to disassemble for, "
                      "see -version for available targets"),
             cl::cat(ObjdumpCat));

cl::opt<bool>
    objdump::ArchiveHeaders("archive-headers",
                            cl::desc("Display archive header information"),
                            cl::cat(ObjdumpCat));
static cl::alias ArchiveHeadersShort("a",
                                     cl::desc("Alias for --archive-headers"),
                                     cl::NotHidden, cl::Grouping,
                                     cl::aliasopt(ArchiveHeaders));

cl::opt<bool> objdump::Demangle("demangle", cl::desc("Demangle symbols names"),
                                cl::init(false), cl::cat(ObjdumpCat));
static cl::alias DemangleShort("C", cl::desc("Alias for --demangle"),
                               cl::NotHidden, cl::Grouping,
                               cl::aliasopt(Demangle));

cl::opt<bool> objdump::Disassemble(
    "disassemble",
    cl::desc("Display assembler mnemonics for the machine instructions"),
    cl::cat(ObjdumpCat));
static cl::alias DisassembleShort("d", cl::desc("Alias for --disassemble"),
                                  cl::NotHidden, cl::Grouping,
                                  cl::aliasopt(Disassemble));

cl::opt<bool> objdump::DisassembleAll(
    "disassemble-all",
    cl::desc("Display assembler mnemonics for the machine instructions"),
    cl::cat(ObjdumpCat));
static cl::alias DisassembleAllShort("D",
                                     cl::desc("Alias for --disassemble-all"),
                                     cl::NotHidden, cl::Grouping,
                                     cl::aliasopt(DisassembleAll));

cl::opt<bool> objdump::SymbolDescription(
    "symbol-description",
    cl::desc("Add symbol description for disassembly. This "
             "option is for XCOFF files only"),
    cl::init(false), cl::cat(ObjdumpCat));

static cl::list<std::string>
    DisassembleSymbols("disassemble-symbols", cl::CommaSeparated,
                       cl::desc("List of symbols to disassemble. "
                                "Accept demangled names when --demangle is "
                                "specified, otherwise accept mangled names"),
                       cl::cat(ObjdumpCat));

static cl::opt<bool> DisassembleZeroes(
    "disassemble-zeroes",
    cl::desc("Do not skip blocks of zeroes when disassembling"),
    cl::cat(ObjdumpCat));
static cl::alias
    DisassembleZeroesShort("z", cl::desc("Alias for --disassemble-zeroes"),
                           cl::NotHidden, cl::Grouping,
                           cl::aliasopt(DisassembleZeroes));

static cl::list<std::string>
    DisassemblerOptions("disassembler-options",
                        cl::desc("Pass target specific disassembler options"),
                        cl::value_desc("options"), cl::CommaSeparated,
                        cl::cat(ObjdumpCat));
static cl::alias
    DisassemblerOptionsShort("M", cl::desc("Alias for --disassembler-options"),
                             cl::NotHidden, cl::Grouping, cl::Prefix,
                             cl::CommaSeparated,
                             cl::aliasopt(DisassemblerOptions));

cl::opt<DIDumpType> objdump::DwarfDumpType(
    "dwarf", cl::init(DIDT_Null), cl::desc("Dump of dwarf debug sections:"),
    cl::values(clEnumValN(DIDT_DebugFrame, "frames", ".debug_frame")),
    cl::cat(ObjdumpCat));

static cl::opt<bool> DynamicRelocations(
    "dynamic-reloc",
    cl::desc("Display the dynamic relocation entries in the file"),
    cl::cat(ObjdumpCat));
static cl::alias DynamicRelocationShort("R",
                                        cl::desc("Alias for --dynamic-reloc"),
                                        cl::NotHidden, cl::Grouping,
                                        cl::aliasopt(DynamicRelocations));

static cl::opt<bool>
    FaultMapSection("fault-map-section",
                    cl::desc("Display contents of faultmap section"),
                    cl::cat(ObjdumpCat));

static cl::opt<bool>
    FileHeaders("file-headers",
                cl::desc("Display the contents of the overall file header"),
                cl::cat(ObjdumpCat));
static cl::alias FileHeadersShort("f", cl::desc("Alias for --file-headers"),
                                  cl::NotHidden, cl::Grouping,
                                  cl::aliasopt(FileHeaders));

cl::opt<bool>
    objdump::SectionContents("full-contents",
                             cl::desc("Display the content of each section"),
                             cl::cat(ObjdumpCat));
static cl::alias SectionContentsShort("s",
                                      cl::desc("Alias for --full-contents"),
                                      cl::NotHidden, cl::Grouping,
                                      cl::aliasopt(SectionContents));

static cl::list<std::string> InputFilenames(cl::Positional,
                                            cl::desc("<input object files>"),
                                            cl::ZeroOrMore,
                                            cl::cat(ObjdumpCat));

static cl::opt<bool>
    PrintLines("line-numbers",
               cl::desc("Display source line numbers with "
                        "disassembly. Implies disassemble object"),
               cl::cat(ObjdumpCat));
static cl::alias PrintLinesShort("l", cl::desc("Alias for --line-numbers"),
                                 cl::NotHidden, cl::Grouping,
                                 cl::aliasopt(PrintLines));

static cl::opt<bool> MachOOpt("macho",
                              cl::desc("Use MachO specific object file parser"),
                              cl::cat(ObjdumpCat));
static cl::alias MachOm("m", cl::desc("Alias for --macho"), cl::NotHidden,
                        cl::Grouping, cl::aliasopt(MachOOpt));

cl::opt<std::string> objdump::MCPU(
    "mcpu", cl::desc("Target a specific cpu type (-mcpu=help for details)"),
    cl::value_desc("cpu-name"), cl::init(""), cl::cat(ObjdumpCat));

cl::list<std::string> objdump::MAttrs("mattr", cl::CommaSeparated,
                                      cl::desc("Target specific attributes"),
                                      cl::value_desc("a1,+a2,-a3,..."),
                                      cl::cat(ObjdumpCat));

cl::opt<bool> objdump::NoShowRawInsn(
    "no-show-raw-insn",
    cl::desc(
        "When disassembling instructions, do not print the instruction bytes."),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::NoLeadingAddr("no-leading-addr",
                                     cl::desc("Print no leading address"),
                                     cl::cat(ObjdumpCat));

static cl::opt<bool> RawClangAST(
    "raw-clang-ast",
    cl::desc("Dump the raw binary contents of the clang AST section"),
    cl::cat(ObjdumpCat));

cl::opt<bool>
    objdump::Relocations("reloc",
                         cl::desc("Display the relocation entries in the file"),
                         cl::cat(ObjdumpCat));
static cl::alias RelocationsShort("r", cl::desc("Alias for --reloc"),
                                  cl::NotHidden, cl::Grouping,
                                  cl::aliasopt(Relocations));

cl::opt<bool>
    objdump::PrintImmHex("print-imm-hex",
                         cl::desc("Use hex format for immediate values"),
                         cl::cat(ObjdumpCat));

cl::opt<bool>
    objdump::PrivateHeaders("private-headers",
                            cl::desc("Display format specific file headers"),
                            cl::cat(ObjdumpCat));
static cl::alias PrivateHeadersShort("p",
                                     cl::desc("Alias for --private-headers"),
                                     cl::NotHidden, cl::Grouping,
                                     cl::aliasopt(PrivateHeaders));

cl::list<std::string>
    objdump::FilterSections("section",
                            cl::desc("Operate on the specified sections only. "
                                     "With -macho dump segment,section"),
                            cl::cat(ObjdumpCat));
static cl::alias FilterSectionsj("j", cl::desc("Alias for --section"),
                                 cl::NotHidden, cl::Grouping, cl::Prefix,
                                 cl::aliasopt(FilterSections));

cl::opt<bool> objdump::SectionHeaders(
    "section-headers",
    cl::desc("Display summaries of the headers for each section."),
    cl::cat(ObjdumpCat));
static cl::alias SectionHeadersShort("headers",
                                     cl::desc("Alias for --section-headers"),
                                     cl::NotHidden,
                                     cl::aliasopt(SectionHeaders));
static cl::alias SectionHeadersShorter("h",
                                       cl::desc("Alias for --section-headers"),
                                       cl::NotHidden, cl::Grouping,
                                       cl::aliasopt(SectionHeaders));

static cl::opt<bool>
    ShowLMA("show-lma",
            cl::desc("Display LMA column when dumping ELF section headers"),
            cl::cat(ObjdumpCat));

static cl::opt<bool> PrintSource(
    "source",
    cl::desc(
        "Display source inlined with disassembly. Implies disassemble object"),
    cl::cat(ObjdumpCat));
static cl::alias PrintSourceShort("S", cl::desc("Alias for -source"),
                                  cl::NotHidden, cl::Grouping,
                                  cl::aliasopt(PrintSource));

static cl::opt<uint64_t>
    StartAddress("start-address", cl::desc("Disassemble beginning at address"),
                 cl::value_desc("address"), cl::init(0), cl::cat(ObjdumpCat));
static cl::opt<uint64_t> StopAddress("stop-address",
                                     cl::desc("Stop disassembly at address"),
                                     cl::value_desc("address"),
                                     cl::init(UINT64_MAX), cl::cat(ObjdumpCat));

cl::opt<bool> objdump::SymbolTable("syms", cl::desc("Display the symbol table"),
                                   cl::cat(ObjdumpCat));
static cl::alias SymbolTableShort("t", cl::desc("Alias for --syms"),
                                  cl::NotHidden, cl::Grouping,
                                  cl::aliasopt(SymbolTable));

static cl::opt<bool> DynamicSymbolTable(
    "dynamic-syms",
    cl::desc("Display the contents of the dynamic symbol table"),
    cl::cat(ObjdumpCat));
static cl::alias DynamicSymbolTableShort("T",
                                         cl::desc("Alias for --dynamic-syms"),
                                         cl::NotHidden, cl::Grouping,
                                         cl::aliasopt(DynamicSymbolTable));

cl::opt<std::string> objdump::TripleName(
    "triple",
    cl::desc(
        "Target triple to disassemble for, see -version for available targets"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::UnwindInfo("unwind-info",
                                  cl::desc("Display unwind information"),
                                  cl::cat(ObjdumpCat));
static cl::alias UnwindInfoShort("u", cl::desc("Alias for --unwind-info"),
                                 cl::NotHidden, cl::Grouping,
                                 cl::aliasopt(UnwindInfo));

static cl::opt<bool>
    Wide("wide", cl::desc("Ignored for compatibility with GNU objdump"),
         cl::cat(ObjdumpCat));
static cl::alias WideShort("w", cl::Grouping, cl::aliasopt(Wide));

enum DebugVarsFormat {
  DVDisabled,
  DVUnicode,
  DVASCII,
};

static cl::opt<DebugVarsFormat> DbgVariables(
    "debug-vars", cl::init(DVDisabled),
    cl::desc("Print the locations (in registers or memory) of "
             "source-level variables alongside disassembly"),
    cl::ValueOptional,
    cl::values(clEnumValN(DVUnicode, "", "unicode"),
               clEnumValN(DVUnicode, "unicode", "unicode"),
               clEnumValN(DVASCII, "ascii", "unicode")),
    cl::cat(ObjdumpCat));

static cl::opt<int>
    DbgIndent("debug-vars-indent", cl::init(40),
              cl::desc("Distance to indent the source-level variable display, "
                       "relative to the start of the disassembly"),
              cl::cat(ObjdumpCat));

static cl::extrahelp
    HelpResponse("\nPass @FILE as argument to read options from FILE.\n");

static StringSet<> DisasmSymbolSet;
StringSet<> objdump::FoundSectionSet;
static StringRef ToolName;

//const MCRegisterInfo &MRI;
/*
///////////////////////////////ekledim
namespace portedprint{

static bool RISCVInstPrinterValidateMCOperand(const MCOperand &MCOp,
                  const MCSubtargetInfo &STI,
                  unsigned PredicateIndex) {
  switch (PredicateIndex) {
  default:
    llvm_unreachable("Unknown MCOperandPredicate kind");
    break;
  case 1: {

    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<12, 1>(Imm);
    return MCOp.isBareSymbolRef();
  
    }
  case 2: {

    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isShiftedInt<20, 1>(Imm);
    return MCOp.isBareSymbolRef();
  
    }
  case 3: {

    int64_t Imm;
    if (MCOp.evaluateAsConstantImm(Imm))
      return isInt<12>(Imm);
    return MCOp.isBareSymbolRef();
  
    }
  }
}
}
*/


namespace {
struct FilterResult {
  // True if the section should not be skipped.
  bool Keep;

  // True if the index counter should be incremented, even if the section should
  // be skipped. For example, sections may be skipped if they are not included
  // in the --section flag, but we still want those to count toward the section
  // count.
  bool IncrementIndex;
};
} // namespace

static FilterResult checkSectionFilter(object::SectionRef S) {
  if (FilterSections.empty())
    return {/*Keep=*/true, /*IncrementIndex=*/true};

  Expected<StringRef> SecNameOrErr = S.getName();
  if (!SecNameOrErr) {
    consumeError(SecNameOrErr.takeError());
    return {/*Keep=*/false, /*IncrementIndex=*/false};
  }
  StringRef SecName = *SecNameOrErr;

  // StringSet does not allow empty key so avoid adding sections with
  // no name (such as the section with index 0) here.
  if (!SecName.empty())
    FoundSectionSet.insert(SecName);

  // Only show the section if it's in the FilterSections list, but always
  // increment so the indexing is stable.
  return {/*Keep=*/is_contained(FilterSections, SecName),
          /*IncrementIndex=*/true};
}

SectionFilter objdump::ToolSectionFilter(object::ObjectFile const &O,
                                         uint64_t *Idx) {
  // Start at UINT64_MAX so that the first index returned after an increment is
  // zero (after the unsigned wrap).
  if (Idx)
    *Idx = UINT64_MAX;
  return SectionFilter(
      [Idx](object::SectionRef S) {
        FilterResult Result = checkSectionFilter(S);
        if (Idx != nullptr && Result.IncrementIndex)
          *Idx += 1;
        return Result.Keep;
      },
      O);
}

std::string objdump::getFileNameForError(const object::Archive::Child &C,
                                         unsigned Index) {
  Expected<StringRef> NameOrErr = C.getName();
  if (NameOrErr)
    return std::string(NameOrErr.get());
  // If we have an error getting the name then we print the index of the archive
  // member. Since we are already in an error state, we just ignore this error.
  consumeError(NameOrErr.takeError());
  return "<file index: " + std::to_string(Index) + ">";
}

void objdump::reportWarning(Twine Message, StringRef File) {
  // Output order between errs() and outs() matters especially for archive
  // files where the output is per member object.
  outs().flush();
  WithColor::warning(errs(), ToolName)
      << "'" << File << "': " << Message << "\n";
}

LLVM_ATTRIBUTE_NORETURN void objdump::reportError(StringRef File,
                                                  Twine Message) {
  outs().flush();
  WithColor::error(errs(), ToolName) << "'" << File << "': " << Message << "\n";
  exit(1);
}

LLVM_ATTRIBUTE_NORETURN void objdump::reportError(Error E, StringRef FileName,
                                                  StringRef ArchiveName,
                                                  StringRef ArchitectureName) {
  assert(E);
  outs().flush();
  WithColor::error(errs(), ToolName);
  if (ArchiveName != "")
    errs() << ArchiveName << "(" << FileName << ")";
  else
    errs() << "'" << FileName << "'";
  if (!ArchitectureName.empty())
    errs() << " (for architecture " << ArchitectureName << ")";
  errs() << ": ";
  logAllUnhandledErrors(std::move(E), errs());
  exit(1);
}

static void reportCmdLineWarning(Twine Message) {
  WithColor::warning(errs(), ToolName) << Message << "\n";
}

LLVM_ATTRIBUTE_NORETURN static void reportCmdLineError(Twine Message) {
  WithColor::error(errs(), ToolName) << Message << "\n";
  exit(1);
}

static void warnOnNoMatchForSections() {
  SetVector<StringRef> MissingSections;
  for (StringRef S : FilterSections) {
    if (FoundSectionSet.count(S))
      return;
    // User may specify a unnamed section. Don't warn for it.
    if (!S.empty())
      MissingSections.insert(S);
  }

  // Warn only if no section in FilterSections is matched.
  for (StringRef S : MissingSections)
    reportCmdLineWarning("section '" + S +
                         "' mentioned in a -j/--section option, but not "
                         "found in any input file");
}

static const Target *getTarget(const ObjectFile *Obj) {
  // Figure out the target triple.
  Triple TheTriple("unknown-unknown-unknown");
  if (TripleName.empty()) {
    TheTriple = Obj->makeTriple();
  } else {
    TheTriple.setTriple(Triple::normalize(TripleName));
    auto Arch = Obj->getArch();
    if (Arch == Triple::arm || Arch == Triple::armeb)
      Obj->setARMSubArch(TheTriple);
  }

  // Get the target specific parser.
  std::string Error;
  const Target *TheTarget = TargetRegistry::lookupTarget(ArchName, TheTriple,
                                                         Error);
  if (!TheTarget)
    reportError(Obj->getFileName(), "can't find target: " + Error);

  // Update the triple name and return the found target.
  TripleName = TheTriple.getTriple();
  return TheTarget;
}

bool objdump::isRelocAddressLess(RelocationRef A, RelocationRef B) {
  return A.getOffset() < B.getOffset();
}

static Error getRelocationValueString(const RelocationRef &Rel,
                                      SmallVectorImpl<char> &Result) {
  const ObjectFile *Obj = Rel.getObject();
  if (auto *ELF = dyn_cast<ELFObjectFileBase>(Obj))
    return getELFRelocationValueString(ELF, Rel, Result);
  if (auto *COFF = dyn_cast<COFFObjectFile>(Obj))
    return getCOFFRelocationValueString(COFF, Rel, Result);
  if (auto *Wasm = dyn_cast<WasmObjectFile>(Obj))
    return getWasmRelocationValueString(Wasm, Rel, Result);
  if (auto *MachO = dyn_cast<MachOObjectFile>(Obj))
    return getMachORelocationValueString(MachO, Rel, Result);
  if (auto *XCOFF = dyn_cast<XCOFFObjectFile>(Obj))
    return getXCOFFRelocationValueString(XCOFF, Rel, Result);
  llvm_unreachable("unknown object file format");
}

/// Indicates whether this relocation should hidden when listing
/// relocations, usually because it is the trailing part of a multipart
/// relocation that will be printed as part of the leading relocation.
static bool getHidden(RelocationRef RelRef) {
  auto *MachO = dyn_cast<MachOObjectFile>(RelRef.getObject());
  if (!MachO)
    return false;

  unsigned Arch = MachO->getArch();
  DataRefImpl Rel = RelRef.getRawDataRefImpl();
  uint64_t Type = MachO->getRelocationType(Rel);

  // On arches that use the generic relocations, GENERIC_RELOC_PAIR
  // is always hidden.
  if (Arch == Triple::x86 || Arch == Triple::arm || Arch == Triple::ppc)
    return Type == MachO::GENERIC_RELOC_PAIR;

  if (Arch == Triple::x86_64) {
    // On x86_64, X86_64_RELOC_UNSIGNED is hidden only when it follows
    // an X86_64_RELOC_SUBTRACTOR.
    if (Type == MachO::X86_64_RELOC_UNSIGNED && Rel.d.a > 0) {
      DataRefImpl RelPrev = Rel;
      RelPrev.d.a--;
      uint64_t PrevType = MachO->getRelocationType(RelPrev);
      if (PrevType == MachO::X86_64_RELOC_SUBTRACTOR)
        return true;
    }
  }

  return false;
}

// anonymous namespace
// namespace olduğu için sadece bu dosyada kullanılabiliyor içindekiler
// ama örneğin llvm:: ya da object:: gibi bi olaya gerek olmuyo direk kullanılabiliyo 

namespace {

/// Get the column at which we want to start printing the instruction
/// disassembly, taking into account anything which appears to the left of it.
unsigned getInstStartColumn(const MCSubtargetInfo &STI) {
  return NoShowRawInsn ? 16 : STI.getTargetTriple().isX86() ? 40 : 24;
}

/// Stores a single expression representing the location of a source-level
/// variable, along with the PC range for which that expression is valid.
struct LiveVariable {
  DWARFLocationExpression LocExpr;
  const char *VarName;
  DWARFUnit *Unit;
  const DWARFDie FuncDie;

  LiveVariable(const DWARFLocationExpression &LocExpr, const char *VarName,
               DWARFUnit *Unit, const DWARFDie FuncDie)
      : LocExpr(LocExpr), VarName(VarName), Unit(Unit), FuncDie(FuncDie) {}

  bool liveAtAddress(object::SectionedAddress Addr) {
    if (LocExpr.Range == None)
      return false;
    return LocExpr.Range->SectionIndex == Addr.SectionIndex &&
           LocExpr.Range->LowPC <= Addr.Address &&
           LocExpr.Range->HighPC > Addr.Address;
  }

  void print(raw_ostream &OS, const MCRegisterInfo &MRI) const {
    DataExtractor Data({LocExpr.Expr.data(), LocExpr.Expr.size()},
                       Unit->getContext().isLittleEndian(), 0);
    DWARFExpression Expression(Data, Unit->getAddressByteSize());
    Expression.printCompact(OS, MRI);
  }
};

/// Helper class for printing source variable locations alongside disassembly.
class LiveVariablePrinter {
  // Information we want to track about one column in which we are printing a
  // variable live range.
  struct Column {
    unsigned VarIdx = NullVarIdx;
    bool LiveIn = false;
    bool LiveOut = false;
    bool MustDrawLabel  = false;

    bool isActive() const { return VarIdx != NullVarIdx; }

    static constexpr unsigned NullVarIdx = std::numeric_limits<unsigned>::max();
  };

  // All live variables we know about in the object/image file.
  std::vector<LiveVariable> LiveVariables;

  // The columns we are currently drawing.
  IndexedMap<Column> ActiveCols;

  const MCRegisterInfo &MRI;
  const MCSubtargetInfo &STI;

  void addVariable(DWARFDie FuncDie, DWARFDie VarDie) {
    uint64_t FuncLowPC, FuncHighPC, SectionIndex;
    FuncDie.getLowAndHighPC(FuncLowPC, FuncHighPC, SectionIndex);
    const char *VarName = VarDie.getName(DINameKind::ShortName);
    DWARFUnit *U = VarDie.getDwarfUnit();

    Expected<DWARFLocationExpressionsVector> Locs =
        VarDie.getLocations(dwarf::DW_AT_location);
    if (!Locs) {
      // If the variable doesn't have any locations, just ignore it. We don't
      // report an error or warning here as that could be noisy on optimised
      // code.
      consumeError(Locs.takeError());
      return;
    }

    for (const DWARFLocationExpression &LocExpr : *Locs) {
      if (LocExpr.Range) {
        LiveVariables.emplace_back(LocExpr, VarName, U, FuncDie);
      } else {
        // If the LocExpr does not have an associated range, it is valid for
        // the whole of the function.
        // TODO: technically it is not valid for any range covered by another
        // LocExpr, does that happen in reality?
        DWARFLocationExpression WholeFuncExpr{
            DWARFAddressRange(FuncLowPC, FuncHighPC, SectionIndex),
            LocExpr.Expr};
        LiveVariables.emplace_back(WholeFuncExpr, VarName, U, FuncDie);
      }
    }
  }

  void addFunction(DWARFDie D) {
    for (const DWARFDie &Child : D.children()) {
      if (Child.getTag() == dwarf::DW_TAG_variable ||
          Child.getTag() == dwarf::DW_TAG_formal_parameter)
        addVariable(D, Child);
      else
        addFunction(Child);
    }
  }

  // Get the column number (in characters) at which the first live variable
  // line should be printed.
  unsigned getIndentLevel() const {
    return DbgIndent + getInstStartColumn(STI);
  }

  // Indent to the first live-range column to the right of the currently
  // printed line, and return the index of that column.
  // TODO: formatted_raw_ostream uses "column" to mean a number of characters
  // since the last \n, and we use it to mean the number of slots in which we
  // put live variable lines. Pick a less overloaded word.
  unsigned moveToFirstVarColumn(formatted_raw_ostream &OS) {
    // Logical column number: column zero is the first column we print in, each
    // logical column is 2 physical columns wide.
    unsigned FirstUnprintedLogicalColumn =
        std::max((int)(OS.getColumn() - getIndentLevel() + 1) / 2, 0);
    // Physical column number: the actual column number in characters, with
    // zero being the left-most side of the screen.
    unsigned FirstUnprintedPhysicalColumn =
        getIndentLevel() + FirstUnprintedLogicalColumn * 2;

    if (FirstUnprintedPhysicalColumn > OS.getColumn())
      OS.PadToColumn(FirstUnprintedPhysicalColumn);

    return FirstUnprintedLogicalColumn;
  }

  unsigned findFreeColumn() {
    for (unsigned ColIdx = 0; ColIdx < ActiveCols.size(); ++ColIdx)
      if (!ActiveCols[ColIdx].isActive())
        return ColIdx;

    size_t OldSize = ActiveCols.size();
    ActiveCols.grow(std::max<size_t>(OldSize * 2, 1));
    return OldSize;
  }

public:
  LiveVariablePrinter(const MCRegisterInfo &MRI, const MCSubtargetInfo &STI)
      : LiveVariables(), ActiveCols(Column()), MRI(MRI), STI(STI) {}

  void dump() const {
    for (const LiveVariable &LV : LiveVariables) {
      dbgs() << LV.VarName << " @ " << LV.LocExpr.Range << ": ";
      LV.print(dbgs(), MRI);
      //degistirdim
      //dbgs() << "\n";
    }
  }

  void addCompileUnit(DWARFDie D) {
    if (D.getTag() == dwarf::DW_TAG_subprogram)
      addFunction(D);
    else
      for (const DWARFDie &Child : D.children())
        addFunction(Child);
  }

  /// Update to match the state of the instruction between ThisAddr and
  /// NextAddr. In the common case, any live range active at ThisAddr is
  /// live-in to the instruction, and any live range active at NextAddr is
  /// live-out of the instruction. If IncludeDefinedVars is false, then live
  /// ranges starting at NextAddr will be ignored.
  void update(object::SectionedAddress ThisAddr,
              object::SectionedAddress NextAddr, bool IncludeDefinedVars) {
    // First, check variables which have already been assigned a column, so
    // that we don't change their order.
    SmallSet<unsigned, 8> CheckedVarIdxs;
    for (unsigned ColIdx = 0, End = ActiveCols.size(); ColIdx < End; ++ColIdx) {
      if (!ActiveCols[ColIdx].isActive())
        continue;
      CheckedVarIdxs.insert(ActiveCols[ColIdx].VarIdx);
      LiveVariable &LV = LiveVariables[ActiveCols[ColIdx].VarIdx];
      ActiveCols[ColIdx].LiveIn = LV.liveAtAddress(ThisAddr);
      ActiveCols[ColIdx].LiveOut = LV.liveAtAddress(NextAddr);
      LLVM_DEBUG(dbgs() << "pass 1, " << ThisAddr.Address << "-"
                        << NextAddr.Address << ", " << LV.VarName << ", Col "
                        << ColIdx << ": LiveIn=" << ActiveCols[ColIdx].LiveIn
                        << ", LiveOut=" << ActiveCols[ColIdx].LiveOut << "\n");

      if (!ActiveCols[ColIdx].LiveIn && !ActiveCols[ColIdx].LiveOut)
        ActiveCols[ColIdx].VarIdx = Column::NullVarIdx;
    }

    // Next, look for variables which don't already have a column, but which
    // are now live.
    if (IncludeDefinedVars) {
      for (unsigned VarIdx = 0, End = LiveVariables.size(); VarIdx < End;
           ++VarIdx) {
        if (CheckedVarIdxs.count(VarIdx))
          continue;
        LiveVariable &LV = LiveVariables[VarIdx];
        bool LiveIn = LV.liveAtAddress(ThisAddr);
        bool LiveOut = LV.liveAtAddress(NextAddr);
        if (!LiveIn && !LiveOut)
          continue;

        unsigned ColIdx = findFreeColumn();
        LLVM_DEBUG(dbgs() << "pass 2, " << ThisAddr.Address << "-"
                          << NextAddr.Address << ", " << LV.VarName << ", Col "
                          << ColIdx << ": LiveIn=" << LiveIn
                          << ", LiveOut=" << LiveOut << "\n");
        ActiveCols[ColIdx].VarIdx = VarIdx;
        ActiveCols[ColIdx].LiveIn = LiveIn;
        ActiveCols[ColIdx].LiveOut = LiveOut;
        ActiveCols[ColIdx].MustDrawLabel = true;
      }
    }
  }

  enum class LineChar {
    RangeStart,
    RangeMid,
    RangeEnd,
    LabelVert,
    LabelCornerNew,
    LabelCornerActive,
    LabelHoriz,
  };
  const char *getLineChar(LineChar C) const {
    bool IsASCII = DbgVariables == DVASCII;
    switch (C) {
    case LineChar::RangeStart:
      return IsASCII ? "^" : u8"\u2548";
    case LineChar::RangeMid:
      return IsASCII ? "|" : u8"\u2503";
    case LineChar::RangeEnd:
      return IsASCII ? "v" : u8"\u253b";
    case LineChar::LabelVert:
      return IsASCII ? "|" : u8"\u2502";
    case LineChar::LabelCornerNew:
      return IsASCII ? "/" : u8"\u250c";
    case LineChar::LabelCornerActive:
      return IsASCII ? "|" : u8"\u2520";
    case LineChar::LabelHoriz:
      return IsASCII ? "-" : u8"\u2500";
    }
    llvm_unreachable("Unhandled LineChar enum");
  }

  /// Print live ranges to the right of an existing line. This assumes the
  /// line is not an instruction, so doesn't start or end any live ranges, so
  /// we only need to print active ranges or empty columns. If AfterInst is
  /// true, this is being printed after the last instruction fed to update(),
  /// otherwise this is being printed before it.
  void printAfterOtherLine(formatted_raw_ostream &OS, bool AfterInst) { // bu da lazim
    if (ActiveCols.size()) {
      unsigned FirstUnprintedColumn = moveToFirstVarColumn(OS);
      for (size_t ColIdx = FirstUnprintedColumn, End = ActiveCols.size();
           ColIdx < End; ++ColIdx) {
        if (ActiveCols[ColIdx].isActive()) {
          if ((AfterInst && ActiveCols[ColIdx].LiveOut) ||
              (!AfterInst && ActiveCols[ColIdx].LiveIn))
            OS << getLineChar(LineChar::RangeMid);
          else if (!AfterInst && ActiveCols[ColIdx].LiveOut)
            OS << getLineChar(LineChar::LabelVert);
          else
            OS << " ";
        }
        OS << " ";
      }
    }
    //OS << "\n"; //degistirdim
  }

  /// Print any live variable range info needed to the right of a
  /// non-instruction line of disassembly. This is where we print the variable
  /// names and expressions, with thin line-drawing characters connecting them
  /// to the live range which starts at the next instruction. If MustPrint is
  /// true, we have to print at least one line (with the continuation of any
  /// already-active live ranges) because something has already been printed
  /// earlier on this line.
  void printBetweenInsts(formatted_raw_ostream &OS, bool MustPrint) {
    bool PrintedSomething = false;
    for (unsigned ColIdx = 0, End = ActiveCols.size(); ColIdx < End; ++ColIdx) {
      if (ActiveCols[ColIdx].isActive() && ActiveCols[ColIdx].MustDrawLabel) {
        // First we need to print the live range markers for any active
        // columns to the left of this one.
        OS.PadToColumn(getIndentLevel());
        for (unsigned ColIdx2 = 0; ColIdx2 < ColIdx; ++ColIdx2) {
          if (ActiveCols[ColIdx2].isActive()) {
            if (ActiveCols[ColIdx2].MustDrawLabel &&
                           !ActiveCols[ColIdx2].LiveIn)
              OS << getLineChar(LineChar::LabelVert) << " ";
            else
              OS << getLineChar(LineChar::RangeMid) << " ";
          } else
            OS << "  ";
        }

        // Then print the variable name and location of the new live range,
        // with box drawing characters joining it to the live range line.
        OS << getLineChar(ActiveCols[ColIdx].LiveIn
                              ? LineChar::LabelCornerActive
                              : LineChar::LabelCornerNew)
           << getLineChar(LineChar::LabelHoriz) << " ";
        WithColor(OS, raw_ostream::GREEN)
            << LiveVariables[ActiveCols[ColIdx].VarIdx].VarName;
        OS << " = ";
        {
          WithColor ExprColor(OS, raw_ostream::CYAN);
          LiveVariables[ActiveCols[ColIdx].VarIdx].print(OS, MRI);
        }

        // If there are any columns to the right of the expression we just
        // printed, then continue their live range lines.
        unsigned FirstUnprintedColumn = moveToFirstVarColumn(OS);
        for (unsigned ColIdx2 = FirstUnprintedColumn, End = ActiveCols.size();
             ColIdx2 < End; ++ColIdx2) {
          if (ActiveCols[ColIdx2].isActive() && ActiveCols[ColIdx2].LiveIn)
            OS << getLineChar(LineChar::RangeMid) << " ";
          else
            OS << "  ";
        }

        //OS << "\n"; //degistirdim
        PrintedSomething = true;
      }
    }

    for (unsigned ColIdx = 0, End = ActiveCols.size(); ColIdx < End; ++ColIdx)
      if (ActiveCols[ColIdx].isActive())
        ActiveCols[ColIdx].MustDrawLabel = false;

    // If we must print something (because we printed a line/column number),
    // but don't have any new variables to print, then print a line which
    // just continues any existing live ranges.
    if (MustPrint && !PrintedSomething)
      printAfterOtherLine(OS, false);
  }

  /// Print the live variable ranges to the right of a disassembled instruction.
  void printAfterInst(formatted_raw_ostream &OS) {
    if (!ActiveCols.size())
      return;
    unsigned FirstUnprintedColumn = moveToFirstVarColumn(OS);
    for (unsigned ColIdx = FirstUnprintedColumn, End = ActiveCols.size();
         ColIdx < End; ++ColIdx) {
      if (!ActiveCols[ColIdx].isActive())
        OS << "  ";
      else if (ActiveCols[ColIdx].LiveIn && ActiveCols[ColIdx].LiveOut)
        OS << getLineChar(LineChar::RangeMid) << " ";
      else if (ActiveCols[ColIdx].LiveOut)
        OS << getLineChar(LineChar::RangeStart) << " ";
      else if (ActiveCols[ColIdx].LiveIn)
        OS << getLineChar(LineChar::RangeEnd) << " ";
      else
        llvm_unreachable("var must be live in or out!");
    //////ekleme
    //degistirdim
    //OS << "\b";
    }
  }
};

class SourcePrinter {
protected:
  DILineInfo OldLineInfo;
  const ObjectFile *Obj = nullptr;
  std::unique_ptr<symbolize::LLVMSymbolizer> Symbolizer;
  // File name to file contents of source.
  std::unordered_map<std::string, std::unique_ptr<MemoryBuffer>> SourceCache;
  // Mark the line endings of the cached source.
  std::unordered_map<std::string, std::vector<StringRef>> LineCache;
  // Keep track of missing sources.
  StringSet<> MissingSources;
  // Only emit 'no debug info' warning once.
  bool WarnedNoDebugInfo;

private:
  bool cacheSource(const DILineInfo& LineInfoFile);

  void printLines(formatted_raw_ostream &OS, const DILineInfo &LineInfo,
                  StringRef Delimiter, LiveVariablePrinter &LVP);

  void printSources(formatted_raw_ostream &OS, const DILineInfo &LineInfo,
                    StringRef ObjectFilename, StringRef Delimiter,
                    LiveVariablePrinter &LVP);

public:
  SourcePrinter() = default;
  SourcePrinter(const ObjectFile *Obj, StringRef DefaultArch)
      : Obj(Obj), WarnedNoDebugInfo(false) {
    symbolize::LLVMSymbolizer::Options SymbolizerOpts;
    SymbolizerOpts.PrintFunctions =
        DILineInfoSpecifier::FunctionNameKind::LinkageName;
    SymbolizerOpts.Demangle = Demangle;
    SymbolizerOpts.DefaultArch = std::string(DefaultArch);
    Symbolizer.reset(new symbolize::LLVMSymbolizer(SymbolizerOpts));
  }
  virtual ~SourcePrinter() = default;
  virtual void printSourceLine(formatted_raw_ostream &OS,
                               object::SectionedAddress Address,
                               StringRef ObjectFilename,
                               LiveVariablePrinter &LVP,
                               StringRef Delimiter = "; ");
};

bool SourcePrinter::cacheSource(const DILineInfo &LineInfo) {
  std::unique_ptr<MemoryBuffer> Buffer;
  if (LineInfo.Source) {
    Buffer = MemoryBuffer::getMemBuffer(*LineInfo.Source);
  } else {
    auto BufferOrError = MemoryBuffer::getFile(LineInfo.FileName);
    if (!BufferOrError) {
      if (MissingSources.insert(LineInfo.FileName).second)
        reportWarning("failed to find source " + LineInfo.FileName,
                      Obj->getFileName());
      return false;
    }
    Buffer = std::move(*BufferOrError);
  }
  // Chomp the file to get lines
  const char *BufferStart = Buffer->getBufferStart(),
             *BufferEnd = Buffer->getBufferEnd();
  std::vector<StringRef> &Lines = LineCache[LineInfo.FileName];
  const char *Start = BufferStart;
  for (const char *I = BufferStart; I != BufferEnd; ++I)
    if (*I == '\n') {
      Lines.emplace_back(Start, I - Start - (BufferStart < I && I[-1] == '\r'));
      Start = I + 1;
    }
  if (Start < BufferEnd)
    Lines.emplace_back(Start, BufferEnd - Start);
  SourceCache[LineInfo.FileName] = std::move(Buffer);
  return true;
}

void SourcePrinter::printSourceLine(formatted_raw_ostream &OS,
                                    object::SectionedAddress Address,
                                    StringRef ObjectFilename,
                                    LiveVariablePrinter &LVP,
                                    StringRef Delimiter) {
  if (!Symbolizer)
    return;

  DILineInfo LineInfo = DILineInfo();
  auto ExpectedLineInfo = Symbolizer->symbolizeCode(*Obj, Address);
  std::string ErrorMessage;
  if (!ExpectedLineInfo)
    ErrorMessage = toString(ExpectedLineInfo.takeError());
  else
    LineInfo = *ExpectedLineInfo;

  if (LineInfo.FileName == DILineInfo::BadString) {
    if (!WarnedNoDebugInfo) {
      std::string Warning =
          "failed to parse debug information for " + ObjectFilename.str();
      if (!ErrorMessage.empty())
        Warning += ": " + ErrorMessage;
      reportWarning(Warning, ObjectFilename);
      WarnedNoDebugInfo = true;
    }
  }

  if (PrintLines)
    printLines(OS, LineInfo, Delimiter, LVP);
  if (PrintSource)
    printSources(OS, LineInfo, ObjectFilename, Delimiter, LVP);
  OldLineInfo = LineInfo;
}

void SourcePrinter::printLines(formatted_raw_ostream &OS,
                               const DILineInfo &LineInfo, StringRef Delimiter,
                               LiveVariablePrinter &LVP) {
  bool PrintFunctionName = LineInfo.FunctionName != DILineInfo::BadString &&
                           LineInfo.FunctionName != OldLineInfo.FunctionName;
  if (PrintFunctionName) {
    OS << Delimiter << LineInfo.FunctionName;
    // If demangling is successful, FunctionName will end with "()". Print it
    // only if demangling did not run or was unsuccessful.
    if (!StringRef(LineInfo.FunctionName).endswith("()"))
      OS << "()";
    OS << ":\n";
  }
  if (LineInfo.FileName != DILineInfo::BadString && LineInfo.Line != 0 &&
      (OldLineInfo.Line != LineInfo.Line ||
       OldLineInfo.FileName != LineInfo.FileName || PrintFunctionName)) {
    OS << Delimiter << LineInfo.FileName << ":" << LineInfo.Line;
    LVP.printBetweenInsts(OS, true);
  }
}

void SourcePrinter::printSources(formatted_raw_ostream &OS,
                                 const DILineInfo &LineInfo,
                                 StringRef ObjectFilename, StringRef Delimiter,
                                 LiveVariablePrinter &LVP) {
  if (LineInfo.FileName == DILineInfo::BadString || LineInfo.Line == 0 ||
      (OldLineInfo.Line == LineInfo.Line &&
       OldLineInfo.FileName == LineInfo.FileName))
    return;

  if (SourceCache.find(LineInfo.FileName) == SourceCache.end())
    if (!cacheSource(LineInfo))
      return;
  auto LineBuffer = LineCache.find(LineInfo.FileName);
  if (LineBuffer != LineCache.end()) {
    if (LineInfo.Line > LineBuffer->second.size()) {
      reportWarning(
          formatv(
              "debug info line number {0} exceeds the number of lines in {1}",
              LineInfo.Line, LineInfo.FileName),
          ObjectFilename);
      return;
    }
    // Vector begins at 0, line numbers are non-zero
    OS << Delimiter << LineBuffer->second[LineInfo.Line - 1];
    LVP.printBetweenInsts(OS, true);
  }
}

static bool isAArch64Elf(const ObjectFile *Obj) {
  const auto *Elf = dyn_cast<ELFObjectFileBase>(Obj);
  return Elf && Elf->getEMachine() == ELF::EM_AARCH64;
}

static bool isArmElf(const ObjectFile *Obj) {
  const auto *Elf = dyn_cast<ELFObjectFileBase>(Obj);
  return Elf && Elf->getEMachine() == ELF::EM_ARM;
}

static bool hasMappingSymbols(const ObjectFile *Obj) {
  return isArmElf(Obj) || isAArch64Elf(Obj);
}

static void printRelocation(formatted_raw_ostream &OS, StringRef FileName,
                            const RelocationRef &Rel, uint64_t Address,
                            bool Is64Bits) {
  StringRef Fmt = Is64Bits ? "\t\t%016" PRIx64 ":  " : "\t\t\t%08" PRIx64 ":  ";
  SmallString<16> Name;
  SmallString<32> Val;
  Rel.getTypeName(Name);
  if (Error E = getRelocationValueString(Rel, Val))
    reportError(std::move(E), FileName);
  OS << format(Fmt.data(), Address) << Name << "\t" << Val;
}

// MCInstPrinter.cpp tan port
// neden namespace?
// cunku using namespace llvm; diyince dumpbytes mcinstprinter.h taki 
// dumpbytesa giriyor --> llvm::dumpbytes burada aynısı olamayacağı için hatali oluyor
// ya yeni namespace tanımlayacaktım ya da işe yaramaz fazladan parametre ekleyip override etmiş olacaktım.
// namespaceler override niyetine çok iyi bee...
/*
  namespace dosya{
    string filename = string(g_argv[2]);
    fstream OSS(filename.substr(filename.find('.')) + ".hex");
  }
*/

////////////////// ters basmak için reverse class templatei
template <typename C>
struct reverse_wrapper {

    C & c_;
    reverse_wrapper(C & c) :  c_(c) {}

    typename C::reverse_iterator begin() {return c_.rbegin();}
    typename C::reverse_iterator end() {return c_.rend(); }
};

template <typename C, size_t N>
struct reverse_wrapper< C[N] >{

    C (&c_)[N];
    reverse_wrapper( C(&c)[N] ) : c_(c) {}

    typename std::reverse_iterator<const C *> begin() { return std::rbegin(c_); }
    typename std::reverse_iterator<const C *> end() { return std::rend(c_); }
};


template <typename C>
reverse_wrapper<C> r_wrap(C & c) {
    return reverse_wrapper<C>(c);
}
///////////////////

using namespace std;




    //string filename = "out";//= string(g_argv[2]);
    //StringRef filename = g_argv[2];
    //fstream OSS(filename + ".hex", fstream::in | fstream::out | fstream::trunc);//"out" + ".hex"
    //fstream OSS(filename.substr(filename.find('.')) + ".hex", fstream::in | fstream::out | fstream::trunc);
    //OSS(filename.substr(filename.find('.')) + ".hex");
    
    //fstream OSS("out.hex", fstream::in | fstream::out | fstream::trunc);
    fstream OSS;
bool flag = false;

const char* hex_char_to_bin(char c)
{
    switch(toupper(c))
    {
        case '0': return "0000";
        case '1': return "0001";
        case '2': return "0010";
        case '3': return "0011";
        case '4': return "0100";
        case '5': return "0101";
        case '6': return "0110";
        case '7': return "0111";
        case '8': return "1000";
        case '9': return "1001";
        case 'A': return "1010";
        case 'B': return "1011";
        case 'C': return "1100";
        case 'D': return "1101";
        case 'E': return "1110";
        case 'F': return "1111";
    }
}

std::string strBitArrXor(std::string a, std::string b, int j){

string c = "";

//for ( int i = 0; i < 8*j; ++i )
//{
//    char ac = a[i];
//    char bc = b[i];
//
//    int ai = ac - '0';
//    int bi = bc - '0';
//    int ci = ai ^ bi;
//    char cc = ci + '0';
//    c += cc;
//}

for ( int i = 0; i < 8*j; ++i )
{
    c += ((a[i] - '0') ^ (b[i] - '0')) + '0';
}
return c;
}

namespace porteddump {
 void dumpBytes(ArrayRef<uint8_t> bytes, raw_ostream &OS, bool encrypt, bool encryptpar, int n) {
//add = false; sub = false;
  //if(!dosya.empty() && flag){ flag = false;
  //  ifstream dos(dosya);
  //  string option = "";
  //  char character;
  //  if (dos.is_open()){
  //    while(dos >> std::noskipws >> character){
  //      //if(character='a') add = true;
  //      if(character != ' ' || character != '\n')
  //        option += string(1,character);
  //      else {
  //        if( option == "add"                ){ add = true; option = ""; } //else sub=true;
  //        if( option == "sub"                ){ sub = true; option = ""; }
  //      }
  //    }
  //  }
  //}
//int bite = atoi(bits.c_str());
//int array[2];
//for (int i = 1; i >= 0; i--) {
//    array[i] = bite % 10;
//    bite /= 10;
//}

//if(array[0] == 1) add = true;
//if(array[1] == 1) sub = true;

   int j = 0;
   char hexArr[(bytes.size()) / 8]; // 4 bayt geliyor 8 lazim

   static const char hex_rep[] = "0123456789abcdef";
   bool First = true;
   for (char i: r_wrap(bytes)) { //OS<<hex_rep[(i & 0xF0) >> 4]<<"\n"; if(!dosya.empty()) OS<<dosya; //OS<<hex_rep[i & 0xF]<<"\n"; //ekledim
     //if (First)
     //  First = false;
     //else
     //  OSS << ' '; //i = i xor 255;

    
/////////////ek
//if(add && sub && flag) { i = i xor 125; flag = false;}
//     if(add && flag) { i = i xor 255; flag = false;} //ff 
//     if(sub && flag) { i = i xor 170; flag = false;} //aa
     

     //if(encrypt) i = i xor 170;
     //if(add) { i = i xor 255;} //ff 
     //if(sub) { i = i xor 170;} //aa
     hexArr[2*j] = hex_rep[(i & 0xF0) >> 4];
     hexArr[2*j + 1] = hex_rep[i & 0xF];
     j++;
   }
   std::string binArr = "";
    for(int i=0;i<strlen(hexArr);i++)
      binArr += hex_char_to_bin(hexArr[i]);
      //OSS << hex_char_to_bin(hexArr[i]);
    //for(int i=0;i<strlen(hexArr);i++)
    //  OSS << hexArr[i];

    if(encrypt){
      if(key != ""){
        OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(key), j),NULL,  2);
      }
    }
    else if(encryptpar){ // eger encrypt varsa partiali hic yapmiyor, once encrypt sonra partial yapmasi saglanabilir //bi dahakine keyi opcode kismindan ver ki bura kalabalik olmasin.
      if     (b_p_beq          != "" && n == 0 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_beq         ), j),NULL,  2);
      else if(b_p_bne          != "" && n == 1 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_bne         ), j),NULL,  2);
      else if(b_p_blt          != "" && n == 2 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_blt         ), j),NULL,  2);
      else if(b_p_bge          != "" && n == 3 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_bge         ), j),NULL,  2);
      else if(b_p_bltu         != "" && n == 4 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_bltu        ), j),NULL,  2);
      else if(b_p_bgeu         != "" && n == 5 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_bgeu        ), j),NULL,  2);
      else if(b_p_jalr         != "" && n == 6 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_jalr        ), j),NULL,  2);
      else if(b_p_jal          != "" && n == 7 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_jal         ), j),NULL,  2);
      else if(b_p_lui          != "" && n == 8 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lui         ), j),NULL,  2);
      else if(b_p_auipc        != "" && n == 9 ) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_auipc       ), j),NULL,  2);
      else if(b_p_addi         != "" && n == 10) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_addi        ), j),NULL,  2);
      else if(b_p_slli         != "" && n == 11) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_slli        ), j),NULL,  2);
      else if(b_p_slti         != "" && n == 12) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_slti        ), j),NULL,  2);
      else if(b_p_sltiu        != "" && n == 13) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sltiu       ), j),NULL,  2);
      else if(b_p_xori         != "" && n == 14) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_xori        ), j),NULL,  2);
      else if(b_p_srli         != "" && n == 15) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_srli        ), j),NULL,  2);
      else if(b_p_srai         != "" && n == 16) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_srai        ), j),NULL,  2);
      else if(b_p_ori          != "" && n == 17) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_ori         ), j),NULL,  2);
      else if(b_p_andi         != "" && n == 18) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_andi        ), j),NULL,  2);
      else if(b_p_add          != "" && n == 19) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_add         ), j),NULL,  2);
      else if(b_p_sub          != "" && n == 20) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sub         ), j),NULL,  2);
      else if(b_p_sll          != "" && n == 21) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sll         ), j),NULL,  2);
      else if(b_p_slt          != "" && n == 22) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_slt         ), j),NULL,  2);
      else if(b_p_sltu         != "" && n == 23) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sltu        ), j),NULL,  2);
      else if(b_p_xor_         != "" && n == 24) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_xor_        ), j),NULL,  2);
      else if(b_p_srl          != "" && n == 25) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_srl         ), j),NULL,  2);
      else if(b_p_sra          != "" && n == 26) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sra         ), j),NULL,  2);
      else if(b_p_or_          != "" && n == 27) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_or_         ), j),NULL,  2);
      else if(b_p_and_         != "" && n == 28) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_and_        ), j),NULL,  2);
      else if(b_p_addiw        != "" && n == 29) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_addiw       ), j),NULL,  2);
      else if(b_p_slliw        != "" && n == 30) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_slliw       ), j),NULL,  2);
      else if(b_p_srliw        != "" && n == 31) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_srliw       ), j),NULL,  2);
      else if(b_p_sraiw        != "" && n == 32) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sraiw       ), j),NULL,  2);
      else if(b_p_addw         != "" && n == 33) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_addw        ), j),NULL,  2);
      else if(b_p_subw         != "" && n == 34) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_subw        ), j),NULL,  2);
      else if(b_p_sllw         != "" && n == 35) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sllw        ), j),NULL,  2);
      else if(b_p_srlw         != "" && n == 36) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_srlw        ), j),NULL,  2);
      else if(b_p_sraw         != "" && n == 37) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sraw        ), j),NULL,  2);
      else if(b_p_lb           != "" && n == 38) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lb          ), j),NULL,  2);
      else if(b_p_lh           != "" && n == 39) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lh          ), j),NULL,  2);
      else if(b_p_lw           != "" && n == 40) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lw          ), j),NULL,  2);
      else if(b_p_ld           != "" && n == 41) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_ld          ), j),NULL,  2);
      else if(b_p_lbu          != "" && n == 42) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lbu         ), j),NULL,  2);
      else if(b_p_lhu          != "" && n == 43) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lhu         ), j),NULL,  2);
      else if(b_p_lwu          != "" && n == 44) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lwu         ), j),NULL,  2);
      else if(b_p_sb           != "" && n == 45) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sb          ), j),NULL,  2);
      else if(b_p_sh           != "" && n == 46) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sh          ), j),NULL,  2);
      else if(b_p_sw           != "" && n == 47) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sw          ), j),NULL,  2);
      else if(b_p_sd           != "" && n == 48) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sd          ), j),NULL,  2);
      else if(b_p_fence        != "" && n == 49) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_fence       ), j),NULL,  2);
      else if(b_p_fence_i      != "" && n == 50) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_fence_i     ), j),NULL,  2);
      else if(b_p_mul          != "" && n == 51) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_mul         ), j),NULL,  2);
      else if(b_p_mulh         != "" && n == 52) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_mulh        ), j),NULL,  2);
      else if(b_p_mulhsu       != "" && n == 53) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_mulhsu      ), j),NULL,  2);
      else if(b_p_mulhu        != "" && n == 54) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_mulhu       ), j),NULL,  2);
      else if(objdump::b_p_div != "" && n == 55) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(objdump::b_p_div), j),NULL,  2);
      else if(b_p_divu         != "" && n == 56) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_divu        ), j),NULL,  2);
      else if(b_p_rem          != "" && n == 57) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_rem         ), j),NULL,  2);
      else if(b_p_remu         != "" && n == 58) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_remu        ), j),NULL,  2);
      else if(b_p_mulw         != "" && n == 59) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_mulw        ), j),NULL,  2);
      else if(b_p_divw         != "" && n == 60) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_divw        ), j),NULL,  2);
      else if(b_p_divuw        != "" && n == 61) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_divuw       ), j),NULL,  2);
      else if(b_p_remw         != "" && n == 62) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_remw        ), j),NULL,  2);
      else if(b_p_remuw        != "" && n == 63) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_remuw       ), j),NULL,  2);
      else if(b_p_lr_w         != "" && n == 64) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lr_w        ), j),NULL,  2);
      else if(b_p_sc_w         != "" && n == 65) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sc_w        ), j),NULL,  2);
      else if(b_p_lr_d         != "" && n == 66) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_lr_d        ), j),NULL,  2);
      else if(b_p_sc_d         != "" && n == 67) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sc_d        ), j),NULL,  2);
      else if(b_p_ecall        != "" && n == 68) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_ecall       ), j),NULL,  2);
      else if(b_p_ebreak       != "" && n == 69) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_ebreak      ), j),NULL,  2);
      else if(b_p_uret         != "" && n == 70) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_uret        ), j),NULL,  2);
      else if(b_p_mret         != "" && n == 71) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_mret        ), j),NULL,  2);
      else if(b_p_dret         != "" && n == 72) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_dret        ), j),NULL,  2);
      else if(b_p_sfence_vma   != "" && n == 73) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_sfence_vma  ), j),NULL,  2);
      else if(b_p_wfi          != "" && n == 74) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_wfi         ), j),NULL,  2);
      else if(b_p_csrrw        != "" && n == 75) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_csrrw       ), j),NULL,  2);
      else if(b_p_csrrs        != "" && n == 76) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_csrrs       ), j),NULL,  2);
      else if(b_p_csrrc        != "" && n == 77) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_csrrc       ), j),NULL,  2);
      else if(b_p_csrrwi       != "" && n == 78) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_csrrwi      ), j),NULL,  2);
      else if(b_p_csrrsi       != "" && n == 79) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_csrrsi      ), j),NULL,  2);
      else if(b_p_csrrci       != "" && n == 80) OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(b_p_csrrci      ), j),NULL,  2);
    }
    else OSS << std::hex << setfill('0') << setw(2*j) << std::stoll(binArr,NULL,  2);

OS << "";
  /*
    if(encrypt){
      if(key != ""){
        if(j == 2){
          std::bitset<16> b_partial (std::string(key));
          std::bitset<16> b_binArr (std::string(binArr));
          //std::bitset<16> b_result = (b_binArr ^ b_partial);
          //b_partial ^= b_binArr;
          //OSS << b_partial;
        }
        else if(j == 4){
          std::bitset<32> b_partial (std::string(key));
          std::bitset<32> b_binArr (std::string(binArr));
          //std::bitset<32> b_result = (b_binArr ^ b_partial);
          //b_partial ^= b_binArr;
          //OSS << b_partial;
          //OSS << b_result;
        }
        //if(j == 2){
          //OSS << (b_binArr xor b_partial)
        //}

        //else if(j == 4){}
      }
      //else i = i xor 0;

    }*/

//for(int i=0;i<bits.length();i++){
//  bitArr[i] = bits[i] - '0';
//}

 }
}
class PrettyPrinter {
public:
  virtual ~PrettyPrinter() = default;
  virtual void // bu printinste giriyor.
  printInst(MCInstPrinter &IP, const MCInst *MI, ArrayRef<uint8_t> Bytes,
            object::SectionedAddress Address, formatted_raw_ostream &OS,
            StringRef Annot, MCSubtargetInfo const &STI, SourcePrinter *SP,
            StringRef ObjectFilename, std::vector<RelocationRef> *Rels,
            LiveVariablePrinter &LVP) { //, const MCRegisterInfo &MRI) { // buraya fazladan MRI parametresi verelim sonra printInste oradan da uncompressInste 
    
    if (SP && (PrintSource || PrintLines))
      SP->printSourceLine(OS, Address, ObjectFilename, LVP);
    LVP.printBetweenInsts(OS, false);

    size_t Start = OS.tell();
    //degistirdim
    //if (!NoLeadingAddr)
    //  OS << format("%8" PRIx64 ":", Address.Address);

int bitArr[90];
for(int i=89;i>=0;i--){
  bitArr[i] = 0;
}

for(int i=0;i<bits.length();i++){
  bitArr[i] = bits[i] - '0';
}

// hepsini baslangicta falsea esitlemeyi dene ya da else kullan.

if(bitArr[0 ] == 1) beq                = true;
if(bitArr[1 ] == 1) bne                = true;
if(bitArr[2 ] == 1) blt                = true;
if(bitArr[3 ] == 1) bge                = true;
if(bitArr[4 ] == 1) bltu               = true;
if(bitArr[5 ] == 1) bgeu               = true;
if(bitArr[6 ] == 1) jalr               = true;
if(bitArr[7 ] == 1) jal                = true;
if(bitArr[8 ] == 1) lui                = true;
if(bitArr[9 ] == 1) auipc              = true;
if(bitArr[10] == 1) addi               = true;
if(bitArr[11] == 1) slli               = true;
if(bitArr[12] == 1) slti               = true;
if(bitArr[13] == 1) sltiu              = true;
if(bitArr[14] == 1) xori               = true;
if(bitArr[15] == 1) srli               = true;
if(bitArr[16] == 1) srai               = true;
if(bitArr[17] == 1) ori                = true;
if(bitArr[18] == 1) andi               = true;
if(bitArr[19] == 1) add                = true;
if(bitArr[20] == 1) sub                = true;
if(bitArr[21] == 1) sll                = true;
if(bitArr[22] == 1) slt                = true;
if(bitArr[23] == 1) sltu               = true;
if(bitArr[24] == 1) xor_               = true; 
if(bitArr[25] == 1) srl                = true;
if(bitArr[26] == 1) sra                = true;
if(bitArr[27] == 1) or_                = true; 
if(bitArr[28] == 1) and_               = true; 
if(bitArr[29] == 1) addiw              = true;
if(bitArr[30] == 1) slliw              = true;
if(bitArr[31] == 1) srliw              = true;
if(bitArr[32] == 1) sraiw              = true;
if(bitArr[33] == 1) addw               = true;
if(bitArr[34] == 1) subw               = true;
if(bitArr[35] == 1) sllw               = true;
if(bitArr[36] == 1) srlw               = true;
if(bitArr[37] == 1) sraw               = true;
if(bitArr[38] == 1) lb                 = true;
if(bitArr[39] == 1) lh                 = true;
if(bitArr[40] == 1) lw                 = true;
if(bitArr[41] == 1) ld                 = true;
if(bitArr[42] == 1) lbu                = true;
if(bitArr[43] == 1) lhu                = true;
if(bitArr[44] == 1) lwu                = true;
if(bitArr[45] == 1) sb                 = true;
if(bitArr[46] == 1) sh                 = true;
if(bitArr[47] == 1) sw                 = true;
if(bitArr[48] == 1) sd                 = true;
if(bitArr[49] == 1) fence              = true;
if(bitArr[50] == 1) fence_i            = true;
if(bitArr[51] == 1) mul                = true;
if(bitArr[52] == 1) mulh               = true;
if(bitArr[53] == 1) mulhsu             = true;
if(bitArr[54] == 1) mulhu              = true;
if(bitArr[55] == 1) objdump::div       = true; // div is ambigious hatasindan dolayi
if(bitArr[56] == 1) divu               = true;
if(bitArr[57] == 1) rem                = true;
if(bitArr[58] == 1) remu               = true;
if(bitArr[59] == 1) mulw               = true;
if(bitArr[60] == 1) divw               = true;
if(bitArr[61] == 1) divuw              = true;
if(bitArr[62] == 1) remw               = true;
if(bitArr[63] == 1) remuw              = true;
if(bitArr[64] == 1) lr_w               = true;
if(bitArr[65] == 1) sc_w               = true;
if(bitArr[66] == 1) lr_d               = true;
if(bitArr[67] == 1) sc_d               = true;
if(bitArr[68] == 1) ecall              = true;
if(bitArr[69] == 1) ebreak             = true;
if(bitArr[70] == 1) uret               = true;
if(bitArr[71] == 1) mret               = true;
if(bitArr[72] == 1) dret               = true;
if(bitArr[73] == 1) sfence_vma         = true;
if(bitArr[74] == 1) wfi                = true;
if(bitArr[75] == 1) csrrw              = true;
if(bitArr[76] == 1) csrrs              = true;
if(bitArr[77] == 1) csrrc              = true;
if(bitArr[78] == 1) csrrwi             = true;
if(bitArr[79] == 1) csrrsi             = true;
if(bitArr[80] == 1) csrrci             = true;
if(bitArr[81] == 1) slli_rv32          = true;
if(bitArr[82] == 1) srli_rv32          = true;
if(bitArr[83] == 1) srai_rv32          = true;
if(bitArr[84] == 1) rdcycle            = true;
if(bitArr[85] == 1) rdtime             = true;
if(bitArr[86] == 1) rdinstret          = true;
if(bitArr[87] == 1) rdcycleh           = true;
if(bitArr[88] == 1) rdtimeh            = true;
if(bitArr[89] == 1) rdinstreth         = true;

//NoShowRawInsn = false;
    if (!NoShowRawInsn) {
      //OS << ' ';
      bool encrypt = false;
      bool encryptpar = false;
      int n = 0;

      //uint64_t Addr =
      //  Address.Address + (STI.getTargetTriple().isX86() ? Bytes.size() : 0);
//
      //llvm::StringRef opcode = portedprint::printInst(MI, Addr, "", STI, OS, MRI);

      if     (beq          && (MI->getOpcode() == RISCV::BEQ  || MI->getOpcode() == RISCV::C_BEQZ)) encrypt = true;
      else if(bne          && (MI->getOpcode() == RISCV::BNE  || MI->getOpcode() == RISCV::C_BNEZ)) encrypt = true;
      else if(blt          && (MI->getOpcode() == RISCV::BLT                                     )) encrypt = true;     
      else if(bge          && (MI->getOpcode() == RISCV::BGE                                     )) encrypt = true;     
      else if(bltu         && (MI->getOpcode() == RISCV::BLTU                                    )) encrypt = true;     
      else if(bgeu         && (MI->getOpcode() == RISCV::BGEU                                    )) encrypt = true;     
      else if(jalr         && (MI->getOpcode() == RISCV::JALR || MI->getOpcode() == RISCV::C_JALR)) encrypt = true;     
      else if(jal          && (MI->getOpcode() == RISCV::JAL  || MI->getOpcode() == RISCV::C_JAL )) encrypt = true;     
      else if(lui          && (MI->getOpcode() == RISCV::LUI  || MI->getOpcode() == RISCV::C_LUI || MI->getOpcode() == RISCV::C_LUI_HINT)) encrypt = true;     
      else if(auipc        && (MI->getOpcode() == RISCV::AUIPC                                   )) encrypt = true;     
      else if(addi         && (MI->getOpcode() == RISCV::ADDI  || MI->getOpcode() == RISCV::C_ADDI || MI->getOpcode() == RISCV::C_ADDI16SP || MI->getOpcode() == RISCV::C_ADDI4SPN || MI->getOpcode() == RISCV::C_ADDI_HINT_X0 || MI->getOpcode() == RISCV::C_ADDI_NOP || MI->getOpcode() == RISCV::C_ADDI_HINT_IMM_ZERO)) encrypt = true; 
      else if(slli         && (MI->getOpcode() == RISCV::SLLI  || MI->getOpcode() == RISCV::C_SLLI || MI->getOpcode() == RISCV::C_SLLI64_HINT || MI->getOpcode() == RISCV::C_SLLI_HINT)) encrypt = true;     
      else if(slti         && (MI->getOpcode() == RISCV::SLTI                                    ))  encrypt = true;     
      else if(sltiu        && (MI->getOpcode() == RISCV::SLTIU                                   )) encrypt = true;     
      else if(xori         && (MI->getOpcode() == RISCV::XORI                                    )) encrypt = true;     
      else if(srli         && (MI->getOpcode() == RISCV::SRLI  || MI->getOpcode() == RISCV::C_SRLI || MI->getOpcode() == RISCV::C_SRLI64_HINT)) encrypt = true;     
      else if(srai         && (MI->getOpcode() == RISCV::SRAI  || MI->getOpcode() == RISCV::C_SRAI || MI->getOpcode() == RISCV::C_SRAI64_HINT)) encrypt = true;     
      else if(ori          && (MI->getOpcode() == RISCV::ORI                                     )) encrypt = true;     
      else if(andi         && (MI->getOpcode() == RISCV::ANDI || MI->getOpcode() == RISCV::C_ANDI)) encrypt = true;     
      else if(add          && (MI->getOpcode() == RISCV::ADD  || MI->getOpcode() == RISCV::C_ADD || MI->getOpcode() == RISCV::C_ADD_HINT)) encrypt = true;     
      else if(sub          && (MI->getOpcode() == RISCV::SUB  || MI->getOpcode() == RISCV::C_SUB || MI->getOpcode() == RISCV::G_SUB)) encrypt = true;     
      else if(sll          && (MI->getOpcode() == RISCV::SLL                                     )) encrypt = true;     
      else if(slt          && (MI->getOpcode() == RISCV::SLT                                     )) encrypt = true;     
      else if(sltu         && (MI->getOpcode() == RISCV::SLTU                                    )) encrypt = true;     
      else if(xor_         && (MI->getOpcode() == RISCV::XOR || MI->getOpcode() == RISCV::C_XOR || MI->getOpcode() == RISCV::G_XOR || MI->getOpcode() == RISCV::G_ATOMICRMW_XOR)) encrypt = true;     
      else if(srl          && (MI->getOpcode() == RISCV::SRL                                     )) encrypt = true;     
      else if(sra          && (MI->getOpcode() == RISCV::SRA                                     )) encrypt = true;     
      else if(or_          && (MI->getOpcode() == RISCV::OR  || MI->getOpcode() == RISCV::ORN || MI->getOpcode() == RISCV::C_OR || MI->getOpcode() == RISCV::G_OR || MI->getOpcode() == RISCV::G_ATOMICRMW_OR)) encrypt = true;     
      else if(and_         && (MI->getOpcode() == RISCV::AND || MI->getOpcode() == RISCV::ANDN || MI->getOpcode() == RISCV::C_AND || MI->getOpcode() == RISCV::G_AND || MI->getOpcode() == RISCV::G_ATOMICRMW_AND)) encrypt = true; //amoandleri almadim
      else if(addiw        && (MI->getOpcode() == RISCV::ADDIW || MI->getOpcode() == RISCV::C_ADDIW )) encrypt = true;  //c_addiwu yu almadim   
      else if(slliw        && (MI->getOpcode() == RISCV::SLLIW                                   )) encrypt = true; // slliuw yi almadim   
      else if(srliw        && (MI->getOpcode() == RISCV::SRLIW                                   )) encrypt = true;     
      else if(sraiw        && (MI->getOpcode() == RISCV::SRAIW                                   )) encrypt = true;     
      else if(addw         && (MI->getOpcode() == RISCV::ADDW || MI->getOpcode() == RISCV::C_ADDW )) encrypt = true; // amolari ve adduw addwu yu almadim  
      else if(subw         && (MI->getOpcode() == RISCV::SUBW || MI->getOpcode() == RISCV::C_SUBW )) encrypt = true; // subwu ve subuw u almadim     
      else if(sllw         && (MI->getOpcode() == RISCV::SLLW                                    )) encrypt = true;     
      else if(srlw         && (MI->getOpcode() == RISCV::SRLW                                    )) encrypt = true;     
      else if(sraw         && (MI->getOpcode() == RISCV::SRAW                                    )) encrypt = true;     
      else if(lb           && (MI->getOpcode() == RISCV::LB   || MI->getOpcode() == RISCV::PseudoLB)) encrypt = true;     
      else if(lh           && (MI->getOpcode() == RISCV::LH   || MI->getOpcode() == RISCV::PseudoLH)) encrypt = true;     
      else if(lw           && (MI->getOpcode() == RISCV::LW   || MI->getOpcode() == RISCV::C_LW || MI->getOpcode() == RISCV::C_LWSP || MI->getOpcode() == RISCV::PseudoLW)) encrypt = true;     
      else if(ld           && (MI->getOpcode() == RISCV::LD   || MI->getOpcode() == RISCV::C_LD || MI->getOpcode() == RISCV::C_LDSP || MI->getOpcode() == RISCV::PseudoLD)) encrypt = true;     
      else if(lbu          && (MI->getOpcode() == RISCV::LBU  || MI->getOpcode() == RISCV::PseudoLBU)) encrypt = true;     
      else if(lhu          && (MI->getOpcode() == RISCV::LHU  || MI->getOpcode() == RISCV::PseudoLHU)) encrypt = true;     
      else if(lwu          && (MI->getOpcode() == RISCV::LWU  || MI->getOpcode() == RISCV::PseudoLWU)) encrypt = true;     
      else if(sb           && (MI->getOpcode() == RISCV::SB   || MI->getOpcode() == RISCV::PseudoSB)) encrypt = true; // sbde daha cok var ama almadim    
      else if(sh           && (MI->getOpcode() == RISCV::SH  || MI->getOpcode() == RISCV::PseudoSH)) encrypt = true;     
      else if(sw           && (MI->getOpcode() == RISCV::SW  || MI->getOpcode() == RISCV::C_SW || MI->getOpcode() == RISCV::C_SWSP || MI->getOpcode() == RISCV::PseudoSW)) encrypt = true;     
      else if(sd           && (MI->getOpcode() == RISCV::SD  || MI->getOpcode() == RISCV::C_SD || MI->getOpcode() == RISCV::C_SDSP || MI->getOpcode() == RISCV::G_SDIV || MI->getOpcode() == RISCV::PseudoSD)) encrypt = true;     
      else if(fence        && (MI->getOpcode() == RISCV::FENCE || MI->getOpcode() == RISCV::FENCE_TSO || MI->getOpcode() == RISCV::G_FENCE)) encrypt = true;     
      else if(fence_i      && (MI->getOpcode() == RISCV::FENCE_I                                 )) encrypt = true;     
      else if(mul          && (MI->getOpcode() == RISCV::MUL   || MI->getOpcode() == RISCV::G_MUL)) encrypt = true;     
      else if(mulh         && (MI->getOpcode() == RISCV::MULH                                    )) encrypt = true;     
      else if(mulhsu       && (MI->getOpcode() == RISCV::MULHSU                                  )) encrypt = true;     
      else if(mulhu        && (MI->getOpcode() == RISCV::MULHU                                   )) encrypt = true;     
      else if(objdump::div && (MI->getOpcode() == RISCV::DIV                                     )) encrypt = true;     
      else if(divu         && (MI->getOpcode() == RISCV::DIVU                                    )) encrypt = true;     
      else if(rem          && (MI->getOpcode() == RISCV::REM                                     )) encrypt = true;     
      else if(remu         && (MI->getOpcode() == RISCV::REMU                                    )) encrypt = true;     
      else if(mulw         && (MI->getOpcode() == RISCV::MULW                                    )) encrypt = true;     
      else if(divw         && (MI->getOpcode() == RISCV::DIVW                                    )) encrypt = true;     
      else if(divuw        && (MI->getOpcode() == RISCV::DIVUW                                   )) encrypt = true;     
      else if(remw         && (MI->getOpcode() == RISCV::REMW                                    )) encrypt = true;     
      else if(remuw        && (MI->getOpcode() == RISCV::REMUW                                   )) encrypt = true;     
      else if(lr_w         && (MI->getOpcode() == RISCV::LR_W  || MI->getOpcode() == RISCV::LR_W_AQ || MI->getOpcode() == RISCV::LR_W_AQ_RL || MI->getOpcode() == RISCV::LR_W_RL)) encrypt = true;     
      else if(sc_w         && (MI->getOpcode() == RISCV::SC_W  || MI->getOpcode() == RISCV::SC_W_AQ || MI->getOpcode() == RISCV::SC_W_AQ_RL || MI->getOpcode() == RISCV::SC_W_RL)) encrypt = true;     
      else if(lr_d         && (MI->getOpcode() == RISCV::LR_D  || MI->getOpcode() == RISCV::LR_D_AQ || MI->getOpcode() == RISCV::LR_D_AQ_RL || MI->getOpcode() == RISCV::LR_D_RL)) encrypt = true;     
      else if(sc_d         && (MI->getOpcode() == RISCV::SC_D  || MI->getOpcode() == RISCV::SC_D_AQ || MI->getOpcode() == RISCV::SC_D_AQ_RL || MI->getOpcode() == RISCV::SC_D_RL)) encrypt = true;     
      else if(ecall        && (MI->getOpcode() == RISCV::ECALL                                   )) encrypt = true;     
      else if(ebreak       && (MI->getOpcode() == RISCV::EBREAK || MI->getOpcode() == RISCV::C_EBREAK)) encrypt = true;     
      else if(uret         && (MI->getOpcode() == RISCV::URET                                    )) encrypt = true;     
      else if(mret         && (MI->getOpcode() == RISCV::MRET                                    )) encrypt = true;     
      else if(dret         && (MI->getOpcode() == RISCV::DRET                                    )) encrypt = true;     
      else if(sfence_vma   && (MI->getOpcode() == RISCV::SFENCE_VMA                              )) encrypt = true;     
      else if(wfi          && (MI->getOpcode() == RISCV::WFI                                     )) encrypt = true;     
      else if(csrrw        && (MI->getOpcode() == RISCV::CSRRW                                   )) encrypt = true;     
      else if(csrrs        && (MI->getOpcode() == RISCV::CSRRS                                   )) encrypt = true;     
      else if(csrrc        && (MI->getOpcode() == RISCV::CSRRC                                   )) encrypt = true;     
      else if(csrrwi       && (MI->getOpcode() == RISCV::CSRRWI                                  )) encrypt = true;     
      else if(csrrsi       && (MI->getOpcode() == RISCV::CSRRSI                                  )) encrypt = true;     
      else if(csrrci       && (MI->getOpcode() == RISCV::CSRRCI                                  )) encrypt = true;     
      //lse if(slli_rv32    && (MI->getOpcode() == RISCV::                                   )) encrypt = true;     
      //lse if(srli_rv32    && opcodeList[82] == opcode) encrypt = true;     
      //lse if(srai_rv32    && opcodeList[83] == opcode) encrypt = true;     
      //lse if(rdcycle      && opcodeList[84] == opcode) encrypt = true;     
      //lse if(rdtime       && opcodeList[85] == opcode) encrypt = true;     
      //lse if(rdinstret    && opcodeList[86] == opcode) encrypt = true;     
      //lse if(rdcycleh     && opcodeList[87] == opcode) encrypt = true;     
      //lse if(rdtimeh      && opcodeList[88] == opcode) encrypt = true;     
      //lse if(rdinstreth   && opcodeList[89] == opcode) encrypt = true; 


      //////////////////////////////////// partial
      if     (b_p_beq          != "" && (MI->getOpcode() == RISCV::BEQ  || MI->getOpcode() == RISCV::C_BEQZ)) encryptpar = true;
      else if(b_p_bne          != "" && (MI->getOpcode() == RISCV::BNE  || MI->getOpcode() == RISCV::C_BNEZ)) encryptpar = true;
      else if(b_p_blt          != "" && (MI->getOpcode() == RISCV::BLT                                     )) encryptpar = true;     
      else if(b_p_bge          != "" && (MI->getOpcode() == RISCV::BGE                                     )) encryptpar = true;     
      else if(b_p_bltu         != "" && (MI->getOpcode() == RISCV::BLTU                                    )) encryptpar = true;     
      else if(b_p_bgeu         != "" && (MI->getOpcode() == RISCV::BGEU                                    )) encryptpar = true;     
      else if(b_p_jalr         != "" && (MI->getOpcode() == RISCV::JALR || MI->getOpcode() == RISCV::C_JALR)) encryptpar = true;     
      else if(b_p_jal          != "" && (MI->getOpcode() == RISCV::JAL  || MI->getOpcode() == RISCV::C_JAL )) encryptpar = true;     
      else if(b_p_lui          != "" && (MI->getOpcode() == RISCV::LUI  || MI->getOpcode() == RISCV::C_LUI || MI->getOpcode() == RISCV::C_LUI_HINT)) encrypt = true;     
      else if(b_p_auipc        != "" && (MI->getOpcode() == RISCV::AUIPC                                   )) encryptpar = true;     
      else if(b_p_addi         != "" && (MI->getOpcode() == RISCV::ADDI  || MI->getOpcode() == RISCV::C_ADDI || MI->getOpcode() == RISCV::C_ADDI16SP || MI->getOpcode() == RISCV::C_ADDI4SPN || MI->getOpcode() == RISCV::C_ADDI_HINT_X0 || MI->getOpcode() == RISCV::C_ADDI_NOP || MI->getOpcode() == RISCV::C_ADDI_HINT_IMM_ZERO)) encryptpar = true; 
      else if(b_p_slli         != "" && (MI->getOpcode() == RISCV::SLLI  || MI->getOpcode() == RISCV::C_SLLI || MI->getOpcode() == RISCV::C_SLLI64_HINT || MI->getOpcode() == RISCV::C_SLLI_HINT)) encryptpar = true;     
      else if(b_p_slti         != "" && (MI->getOpcode() == RISCV::SLTI                                    )) encryptpar = true;     
      else if(b_p_sltiu        != "" && (MI->getOpcode() == RISCV::SLTIU                                   )) encryptpar = true;     
      else if(b_p_xori         != "" && (MI->getOpcode() == RISCV::XORI                                    )) encryptpar = true;     
      else if(b_p_srli         != "" && (MI->getOpcode() == RISCV::SRLI  || MI->getOpcode() == RISCV::C_SRLI || MI->getOpcode() == RISCV::C_SRLI64_HINT)) encryptpar = true;     
      else if(b_p_srai         != "" && (MI->getOpcode() == RISCV::SRAI  || MI->getOpcode() == RISCV::C_SRAI || MI->getOpcode() == RISCV::C_SRAI64_HINT)) encryptpar = true;     
      else if(b_p_ori          != "" && (MI->getOpcode() == RISCV::ORI                                     )) encryptpar = true;     
      else if(b_p_andi         != "" && (MI->getOpcode() == RISCV::ANDI || MI->getOpcode() == RISCV::C_ANDI)) encryptpar = true;     
      else if(b_p_add          != "" && (MI->getOpcode() == RISCV::ADD  || MI->getOpcode() == RISCV::C_ADD || MI->getOpcode() == RISCV::C_ADD_HINT)) encryptpar = true;     
      else if(b_p_sub          != "" && (MI->getOpcode() == RISCV::SUB  || MI->getOpcode() == RISCV::C_SUB || MI->getOpcode() == RISCV::G_SUB)) encryptpar = true;     
      else if(b_p_sll          != "" && (MI->getOpcode() == RISCV::SLL                                     )) encryptpar = true;     
      else if(b_p_slt          != "" && (MI->getOpcode() == RISCV::SLT                                     )) encryptpar = true;     
      else if(b_p_sltu         != "" && (MI->getOpcode() == RISCV::SLTU                                    )) encryptpar = true;     
      else if(b_p_xor_         != "" && (MI->getOpcode() == RISCV::XOR || MI->getOpcode() == RISCV::C_XOR || MI->getOpcode() == RISCV::G_XOR || MI->getOpcode() == RISCV::G_ATOMICRMW_XOR)) encryptpar = true;     
      else if(b_p_srl          != "" && (MI->getOpcode() == RISCV::SRL                                     )) encryptpar = true;     
      else if(b_p_sra          != "" && (MI->getOpcode() == RISCV::SRA                                     )) encryptpar = true;     
      else if(b_p_or_          != "" && (MI->getOpcode() == RISCV::OR  || MI->getOpcode() == RISCV::ORN || MI->getOpcode() == RISCV::C_OR || MI->getOpcode() == RISCV::G_OR || MI->getOpcode() == RISCV::G_ATOMICRMW_OR)) encryptpar = true;     
      else if(b_p_and_         != "" && (MI->getOpcode() == RISCV::AND || MI->getOpcode() == RISCV::ANDN || MI->getOpcode() == RISCV::C_AND || MI->getOpcode() == RISCV::G_AND || MI->getOpcode() == RISCV::G_ATOMICRMW_AND)) encryptpar = true; //amoandleri almadim
      else if(b_p_addiw        != "" && (MI->getOpcode() == RISCV::ADDIW || MI->getOpcode() == RISCV::C_ADDIW )) encryptpar = true;  //c_addiwu yu almadim   
      else if(b_p_slliw        != "" && (MI->getOpcode() == RISCV::SLLIW                                   )) encryptpar = true; // slliuw yi almadim   
      else if(b_p_srliw        != "" && (MI->getOpcode() == RISCV::SRLIW                                   )) encryptpar = true;     
      else if(b_p_sraiw        != "" && (MI->getOpcode() == RISCV::SRAIW                                   )) encryptpar = true;     
      else if(b_p_addw         != "" && (MI->getOpcode() == RISCV::ADDW || MI->getOpcode() == RISCV::C_ADDW )) encryptpar = true; // amolari ve adduw addwu yu almadim  
      else if(b_p_subw         != "" && (MI->getOpcode() == RISCV::SUBW || MI->getOpcode() == RISCV::C_SUBW )) encryptpar = true; // subwu ve subuw u almadim     
      else if(b_p_sllw         != "" && (MI->getOpcode() == RISCV::SLLW                                    )) encryptpar = true;     
      else if(b_p_srlw         != "" && (MI->getOpcode() == RISCV::SRLW                                    )) encryptpar = true;     
      else if(b_p_sraw         != "" && (MI->getOpcode() == RISCV::SRAW                                    )) encryptpar = true;     
      else if(b_p_lb           != "" && (MI->getOpcode() == RISCV::LB   || MI->getOpcode() == RISCV::PseudoLB)) encryptpar = true;     
      else if(b_p_lh           != "" && (MI->getOpcode() == RISCV::LH   || MI->getOpcode() == RISCV::PseudoLH)) encryptpar = true;     
      else if(b_p_lw           != "" && (MI->getOpcode() == RISCV::LW   || MI->getOpcode() == RISCV::C_LW || MI->getOpcode() == RISCV::C_LWSP || MI->getOpcode() == RISCV::PseudoLW)) encryptpar = true;     
      else if(b_p_ld           != "" && (MI->getOpcode() == RISCV::LD   || MI->getOpcode() == RISCV::C_LD || MI->getOpcode() == RISCV::C_LDSP || MI->getOpcode() == RISCV::PseudoLD)) encryptpar = true;     
      else if(b_p_lbu          != "" && (MI->getOpcode() == RISCV::LBU  || MI->getOpcode() == RISCV::PseudoLBU)) encryptpar = true;     
      else if(b_p_lhu          != "" && (MI->getOpcode() == RISCV::LHU  || MI->getOpcode() == RISCV::PseudoLHU)) encryptpar = true;     
      else if(b_p_lwu          != "" && (MI->getOpcode() == RISCV::LWU  || MI->getOpcode() == RISCV::PseudoLWU)) encryptpar = true;     
      else if(b_p_sb           != "" && (MI->getOpcode() == RISCV::SB   || MI->getOpcode() == RISCV::PseudoSB)) encryptpar = true; // sbde daha cok var ama almadim    
      else if(b_p_sh           != "" && (MI->getOpcode() == RISCV::SH  || MI->getOpcode() == RISCV::PseudoSH)) encryptpar = true;     
      else if(b_p_sw           != "" && (MI->getOpcode() == RISCV::SW  || MI->getOpcode() == RISCV::C_SW || MI->getOpcode() == RISCV::C_SWSP || MI->getOpcode() == RISCV::PseudoSW)) encryptpar = true;     
      else if(b_p_sd           != "" && (MI->getOpcode() == RISCV::SD  || MI->getOpcode() == RISCV::C_SD || MI->getOpcode() == RISCV::C_SDSP || MI->getOpcode() == RISCV::G_SDIV || MI->getOpcode() == RISCV::PseudoSD)) encryptpar = true;     
      else if(b_p_fence        != "" && (MI->getOpcode() == RISCV::FENCE || MI->getOpcode() == RISCV::FENCE_TSO || MI->getOpcode() == RISCV::G_FENCE)) encryptpar = true;     
      else if(b_p_fence_i      != "" && (MI->getOpcode() == RISCV::FENCE_I                                 )) encryptpar = true;     
      else if(b_p_mul          != "" && (MI->getOpcode() == RISCV::MUL   || MI->getOpcode() == RISCV::G_MUL)) encryptpar = true;     
      else if(b_p_mulh         != "" && (MI->getOpcode() == RISCV::MULH                                    )) encryptpar = true;     
      else if(b_p_mulhsu       != "" && (MI->getOpcode() == RISCV::MULHSU                                  )) encryptpar = true;     
      else if(b_p_mulhu        != "" && (MI->getOpcode() == RISCV::MULHU                                   )) encryptpar = true;     
      else if(objdump::b_p_div != "" && (MI->getOpcode() == RISCV::DIV                                     )) encryptpar = true;     
      else if(b_p_divu         != "" && (MI->getOpcode() == RISCV::DIVU                                    )) encryptpar = true;     
      else if(b_p_rem          != "" && (MI->getOpcode() == RISCV::REM                                     )) encryptpar = true;     
      else if(b_p_remu         != "" && (MI->getOpcode() == RISCV::REMU                                    )) encryptpar = true;     
      else if(b_p_mulw         != "" && (MI->getOpcode() == RISCV::MULW                                    )) encryptpar = true;     
      else if(b_p_divw         != "" && (MI->getOpcode() == RISCV::DIVW                                    )) encryptpar = true;     
      else if(b_p_divuw        != "" && (MI->getOpcode() == RISCV::DIVUW                                   )) encryptpar = true;     
      else if(b_p_remw         != "" && (MI->getOpcode() == RISCV::REMW                                    )) encryptpar = true;     
      else if(b_p_remuw        != "" && (MI->getOpcode() == RISCV::REMUW                                   )) encryptpar = true;     
      else if(b_p_lr_w         != "" && (MI->getOpcode() == RISCV::LR_W  || MI->getOpcode() == RISCV::LR_W_AQ || MI->getOpcode() == RISCV::LR_W_AQ_RL || MI->getOpcode() == RISCV::LR_W_RL)) encryptpar = true;     
      else if(b_p_sc_w         != "" && (MI->getOpcode() == RISCV::SC_W  || MI->getOpcode() == RISCV::SC_W_AQ || MI->getOpcode() == RISCV::SC_W_AQ_RL || MI->getOpcode() == RISCV::SC_W_RL)) encryptpar = true;     
      else if(b_p_lr_d         != "" && (MI->getOpcode() == RISCV::LR_D  || MI->getOpcode() == RISCV::LR_D_AQ || MI->getOpcode() == RISCV::LR_D_AQ_RL || MI->getOpcode() == RISCV::LR_D_RL)) encryptpar = true;     
      else if(b_p_sc_d         != "" && (MI->getOpcode() == RISCV::SC_D  || MI->getOpcode() == RISCV::SC_D_AQ || MI->getOpcode() == RISCV::SC_D_AQ_RL || MI->getOpcode() == RISCV::SC_D_RL)) encryptpar = true;     
      else if(b_p_ecall        != "" && (MI->getOpcode() == RISCV::ECALL                                   )) encryptpar = true;     
      else if(b_p_ebreak       != "" && (MI->getOpcode() == RISCV::EBREAK || MI->getOpcode() == RISCV::C_EBREAK)) encryptpar = true;     
      else if(b_p_uret         != "" && (MI->getOpcode() == RISCV::URET                                    )) encryptpar = true;     
      else if(b_p_mret         != "" && (MI->getOpcode() == RISCV::MRET                                    )) encryptpar = true;     
      else if(b_p_dret         != "" && (MI->getOpcode() == RISCV::DRET                                    )) encryptpar = true;     
      else if(b_p_sfence_vma   != "" && (MI->getOpcode() == RISCV::SFENCE_VMA                              )) encryptpar = true;     
      else if(b_p_wfi          != "" && (MI->getOpcode() == RISCV::WFI                                     )) encryptpar = true;     
      else if(b_p_csrrw        != "" && (MI->getOpcode() == RISCV::CSRRW                                   )) encryptpar = true;     
      else if(b_p_csrrs        != "" && (MI->getOpcode() == RISCV::CSRRS                                   )) encryptpar = true;     
      else if(b_p_csrrc        != "" && (MI->getOpcode() == RISCV::CSRRC                                   )) encryptpar = true;     
      else if(b_p_csrrwi       != "" && (MI->getOpcode() == RISCV::CSRRWI                                  )) encryptpar = true;     
      else if(b_p_csrrsi       != "" && (MI->getOpcode() == RISCV::CSRRSI                                  )) encryptpar = true;     
      else if(b_p_csrrci       != "" && (MI->getOpcode() == RISCV::CSRRCI                                  )) encryptpar = true;

//bu ve usttekini bool p_ ile yapabiliriz.
      if     (b_p_beq          != "" && (MI->getOpcode() == RISCV::BEQ  || MI->getOpcode() == RISCV::C_BEQZ)) n=0;
      else if(b_p_bne          != "" && (MI->getOpcode() == RISCV::BNE  || MI->getOpcode() == RISCV::C_BNEZ)) n=1;
      else if(b_p_blt          != "" && (MI->getOpcode() == RISCV::BLT                                     )) n=2;     
      else if(b_p_bge          != "" && (MI->getOpcode() == RISCV::BGE                                     )) n=3;     
      else if(b_p_bltu         != "" && (MI->getOpcode() == RISCV::BLTU                                    )) n=4;     
      else if(b_p_bgeu         != "" && (MI->getOpcode() == RISCV::BGEU                                    )) n=5;     
      else if(b_p_jalr         != "" && (MI->getOpcode() == RISCV::JALR || MI->getOpcode() == RISCV::C_JALR)) n=6;     
      else if(b_p_jal          != "" && (MI->getOpcode() == RISCV::JAL  || MI->getOpcode() == RISCV::C_JAL )) n=7;     
      else if(b_p_lui          != "" && (MI->getOpcode() == RISCV::LUI  || MI->getOpcode() == RISCV::C_LUI || MI->getOpcode() == RISCV::C_LUI_HINT)) n=8;     
      else if(b_p_auipc        != "" && (MI->getOpcode() == RISCV::AUIPC                                   )) n=9;     
      else if(b_p_addi         != "" && (MI->getOpcode() == RISCV::ADDI  || MI->getOpcode() == RISCV::C_ADDI || MI->getOpcode() == RISCV::C_ADDI16SP || MI->getOpcode() == RISCV::C_ADDI4SPN || MI->getOpcode() == RISCV::C_ADDI_HINT_X0 || MI->getOpcode() == RISCV::C_ADDI_NOP || MI->getOpcode() == RISCV::C_ADDI_HINT_IMM_ZERO)) n=10; 
      else if(b_p_slli         != "" && (MI->getOpcode() == RISCV::SLLI  || MI->getOpcode() == RISCV::C_SLLI || MI->getOpcode() == RISCV::C_SLLI64_HINT || MI->getOpcode() == RISCV::C_SLLI_HINT)) n=11;     
      else if(b_p_slti         != "" && (MI->getOpcode() == RISCV::SLTI                                    )) n=12;     
      else if(b_p_sltiu        != "" && (MI->getOpcode() == RISCV::SLTIU                                   )) n=13;     
      else if(b_p_xori         != "" && (MI->getOpcode() == RISCV::XORI                                    )) n=14;     
      else if(b_p_srli         != "" && (MI->getOpcode() == RISCV::SRLI  || MI->getOpcode() == RISCV::C_SRLI || MI->getOpcode() == RISCV::C_SRLI64_HINT)) n=15;     
      else if(b_p_srai         != "" && (MI->getOpcode() == RISCV::SRAI  || MI->getOpcode() == RISCV::C_SRAI || MI->getOpcode() == RISCV::C_SRAI64_HINT)) n=16;     
      else if(b_p_ori          != "" && (MI->getOpcode() == RISCV::ORI                                     )) n=17;     
      else if(b_p_andi         != "" && (MI->getOpcode() == RISCV::ANDI || MI->getOpcode() == RISCV::C_ANDI)) n=18;     
      else if(b_p_add          != "" && (MI->getOpcode() == RISCV::ADD  || MI->getOpcode() == RISCV::C_ADD || MI->getOpcode() == RISCV::C_ADD_HINT)) n=19;     
      else if(b_p_sub          != "" && (MI->getOpcode() == RISCV::SUB  || MI->getOpcode() == RISCV::C_SUB || MI->getOpcode() == RISCV::G_SUB)) n=20;     
      else if(b_p_sll          != "" && (MI->getOpcode() == RISCV::SLL                                     )) n=21;     
      else if(b_p_slt          != "" && (MI->getOpcode() == RISCV::SLT                                     )) n=22;     
      else if(b_p_sltu         != "" && (MI->getOpcode() == RISCV::SLTU                                    )) n=23;     
      else if(b_p_xor_         != "" && (MI->getOpcode() == RISCV::XOR || MI->getOpcode() == RISCV::C_XOR || MI->getOpcode() == RISCV::G_XOR || MI->getOpcode() == RISCV::G_ATOMICRMW_XOR)) n=24;     
      else if(b_p_srl          != "" && (MI->getOpcode() == RISCV::SRL                                     )) n=25;     
      else if(b_p_sra          != "" && (MI->getOpcode() == RISCV::SRA                                     )) n=26;     
      else if(b_p_or_          != "" && (MI->getOpcode() == RISCV::OR  || MI->getOpcode() == RISCV::ORN || MI->getOpcode() == RISCV::C_OR || MI->getOpcode() == RISCV::G_OR || MI->getOpcode() == RISCV::G_ATOMICRMW_OR)) n=27;     
      else if(b_p_and_         != "" && (MI->getOpcode() == RISCV::AND || MI->getOpcode() == RISCV::ANDN || MI->getOpcode() == RISCV::C_AND || MI->getOpcode() == RISCV::G_AND || MI->getOpcode() == RISCV::G_ATOMICRMW_AND)) n=28; //amoandleri almadim
      else if(b_p_addiw        != "" && (MI->getOpcode() == RISCV::ADDIW || MI->getOpcode() == RISCV::C_ADDIW )) n=29;  //c_addiwu yu almadim   
      else if(b_p_slliw        != "" && (MI->getOpcode() == RISCV::SLLIW                                   )) n=30; // slliuw yi almadim   
      else if(b_p_srliw        != "" && (MI->getOpcode() == RISCV::SRLIW                                   )) n=31;     
      else if(b_p_sraiw        != "" && (MI->getOpcode() == RISCV::SRAIW                                   )) n=32;     
      else if(b_p_addw         != "" && (MI->getOpcode() == RISCV::ADDW || MI->getOpcode() == RISCV::C_ADDW )) n=33; // amolari ve adduw addwu yu almadim  
      else if(b_p_subw         != "" && (MI->getOpcode() == RISCV::SUBW || MI->getOpcode() == RISCV::C_SUBW )) n=34; // subwu ve subuw u almadim     
      else if(b_p_sllw         != "" && (MI->getOpcode() == RISCV::SLLW                                    )) n=35;     
      else if(b_p_srlw         != "" && (MI->getOpcode() == RISCV::SRLW                                    )) n=36;     
      else if(b_p_sraw         != "" && (MI->getOpcode() == RISCV::SRAW                                    )) n=37;     
      else if(b_p_lb           != "" && (MI->getOpcode() == RISCV::LB   || MI->getOpcode() == RISCV::PseudoLB)) n=38;     
      else if(b_p_lh           != "" && (MI->getOpcode() == RISCV::LH   || MI->getOpcode() == RISCV::PseudoLH)) n=39;     
      else if(b_p_lw           != "" && (MI->getOpcode() == RISCV::LW   || MI->getOpcode() == RISCV::C_LW || MI->getOpcode() == RISCV::C_LWSP || MI->getOpcode() == RISCV::PseudoLW)) n=40;     
      else if(b_p_ld           != "" && (MI->getOpcode() == RISCV::LD   || MI->getOpcode() == RISCV::C_LD || MI->getOpcode() == RISCV::C_LDSP || MI->getOpcode() == RISCV::PseudoLD)) n=41;     
      else if(b_p_lbu          != "" && (MI->getOpcode() == RISCV::LBU  || MI->getOpcode() == RISCV::PseudoLBU)) n=42;     
      else if(b_p_lhu          != "" && (MI->getOpcode() == RISCV::LHU  || MI->getOpcode() == RISCV::PseudoLHU)) n=43;     
      else if(b_p_lwu          != "" && (MI->getOpcode() == RISCV::LWU  || MI->getOpcode() == RISCV::PseudoLWU)) n=44;     
      else if(b_p_sb           != "" && (MI->getOpcode() == RISCV::SB   || MI->getOpcode() == RISCV::PseudoSB)) n=45; // sbde daha cok var ama almadim    
      else if(b_p_sh           != "" && (MI->getOpcode() == RISCV::SH  || MI->getOpcode() == RISCV::PseudoSH)) n=46;     
      else if(b_p_sw           != "" && (MI->getOpcode() == RISCV::SW  || MI->getOpcode() == RISCV::C_SW || MI->getOpcode() == RISCV::C_SWSP || MI->getOpcode() == RISCV::PseudoSW)) n=47;     
      else if(b_p_sd           != "" && (MI->getOpcode() == RISCV::SD  || MI->getOpcode() == RISCV::C_SD || MI->getOpcode() == RISCV::C_SDSP || MI->getOpcode() == RISCV::G_SDIV || MI->getOpcode() == RISCV::PseudoSD)) n=48;     
      else if(b_p_fence        != "" && (MI->getOpcode() == RISCV::FENCE || MI->getOpcode() == RISCV::FENCE_TSO || MI->getOpcode() == RISCV::G_FENCE)) n=49;     
      else if(b_p_fence_i      != "" && (MI->getOpcode() == RISCV::FENCE_I                                 )) n=50;     
      else if(b_p_mul          != "" && (MI->getOpcode() == RISCV::MUL   || MI->getOpcode() == RISCV::G_MUL)) n=51;     
      else if(b_p_mulh         != "" && (MI->getOpcode() == RISCV::MULH                                    )) n=52;     
      else if(b_p_mulhsu       != "" && (MI->getOpcode() == RISCV::MULHSU                                  )) n=53;     
      else if(b_p_mulhu        != "" && (MI->getOpcode() == RISCV::MULHU                                   )) n=54;     
      else if(objdump::b_p_div != "" && (MI->getOpcode() == RISCV::DIV                                     )) n=55;     
      else if(b_p_divu         != "" && (MI->getOpcode() == RISCV::DIVU                                    )) n=56;     
      else if(b_p_rem          != "" && (MI->getOpcode() == RISCV::REM                                     )) n=57;     
      else if(b_p_remu         != "" && (MI->getOpcode() == RISCV::REMU                                    )) n=58;     
      else if(b_p_mulw         != "" && (MI->getOpcode() == RISCV::MULW                                    )) n=59;     
      else if(b_p_divw         != "" && (MI->getOpcode() == RISCV::DIVW                                    )) n=60;     
      else if(b_p_divuw        != "" && (MI->getOpcode() == RISCV::DIVUW                                   )) n=61;     
      else if(b_p_remw         != "" && (MI->getOpcode() == RISCV::REMW                                    )) n=62;     
      else if(b_p_remuw        != "" && (MI->getOpcode() == RISCV::REMUW                                   )) n=63;     
      else if(b_p_lr_w         != "" && (MI->getOpcode() == RISCV::LR_W  || MI->getOpcode() == RISCV::LR_W_AQ || MI->getOpcode() == RISCV::LR_W_AQ_RL || MI->getOpcode() == RISCV::LR_W_RL)) n=64;     
      else if(b_p_sc_w         != "" && (MI->getOpcode() == RISCV::SC_W  || MI->getOpcode() == RISCV::SC_W_AQ || MI->getOpcode() == RISCV::SC_W_AQ_RL || MI->getOpcode() == RISCV::SC_W_RL)) n=65;     
      else if(b_p_lr_d         != "" && (MI->getOpcode() == RISCV::LR_D  || MI->getOpcode() == RISCV::LR_D_AQ || MI->getOpcode() == RISCV::LR_D_AQ_RL || MI->getOpcode() == RISCV::LR_D_RL)) n=66;     
      else if(b_p_sc_d         != "" && (MI->getOpcode() == RISCV::SC_D  || MI->getOpcode() == RISCV::SC_D_AQ || MI->getOpcode() == RISCV::SC_D_AQ_RL || MI->getOpcode() == RISCV::SC_D_RL)) n=67;     
      else if(b_p_ecall        != "" && (MI->getOpcode() == RISCV::ECALL                                   )) n=68;     
      else if(b_p_ebreak       != "" && (MI->getOpcode() == RISCV::EBREAK || MI->getOpcode() == RISCV::C_EBREAK)) n=69;     
      else if(b_p_uret         != "" && (MI->getOpcode() == RISCV::URET                                    )) n=70;     
      else if(b_p_mret         != "" && (MI->getOpcode() == RISCV::MRET                                    )) n=71;     
      else if(b_p_dret         != "" && (MI->getOpcode() == RISCV::DRET                                    )) n=72;     
      else if(b_p_sfence_vma   != "" && (MI->getOpcode() == RISCV::SFENCE_VMA                              )) n=73;     
      else if(b_p_wfi          != "" && (MI->getOpcode() == RISCV::WFI                                     )) n=74;     
      else if(b_p_csrrw        != "" && (MI->getOpcode() == RISCV::CSRRW                                   )) n=75;     
      else if(b_p_csrrs        != "" && (MI->getOpcode() == RISCV::CSRRS                                   )) n=76;     
      else if(b_p_csrrc        != "" && (MI->getOpcode() == RISCV::CSRRC                                   )) n=77;     
      else if(b_p_csrrwi       != "" && (MI->getOpcode() == RISCV::CSRRWI                                  )) n=78;     
      else if(b_p_csrrsi       != "" && (MI->getOpcode() == RISCV::CSRRSI                                  )) n=79;     
      else if(b_p_csrrci       != "" && (MI->getOpcode() == RISCV::CSRRCI                                  )) n=80; 
      //////////////////////////////////// partial

//OS.flush();

      porteddump::dumpBytes(Bytes, OS, encrypt, encryptpar, n); // hexi burada yaziyor
    }

    // The output of printInst starts with a tab. Print some spaces so that
    // the tab has 1 column and advances to the target tab stop.
    unsigned TabStop = getInstStartColumn(STI);
    unsigned Column = OS.tell() - Start;
    OS.indent(Column < TabStop - 1 ? TabStop - 1 - Column : 7 - Column % 8);
//////////////////////////////////////////////////////////////////////////////////////////
//string a = std::to_string( MI->getOpcode() ) ; // unsigned int to string
//if(MI->getOpcode() == 371) {OS<<"addiiiii"; flag = true;}
//Instruction::getOpcodeName(MI->getOpcode());
//OS<<a;
//StringRef b = IP.getOpcodeName(MI->getOpcode());
//OS<<MCInstPrinter::getOpcodeName(MI->getOpcode()); // ekledim
//if(MI->getOpcode() == RISCV::ADDI)
OS<<"\n";
//OS<<b;
//if(MI->getOpcode() == RISCV::SW) OS<<b;//"basarili";//IP.getOpcodeName(MI->getOpcode())"";
//if(addi && MI->getOpcode() == RISCV::C_ADDI4SPN        /*&& opcodeList[10] == opcode*/) OS<<"GIRDIaddi";//encrypt = true;   
//////////////////////////////////////////////////7



    if (MI) { // degistirdim
      // See MCInstPrinter::printInst. On targets where a PC relative immediate
      // is relative to the next instruction and the length of a MCInst is
      // difficult to measure (x86), this is the address of the next
      // instruction.
      uint64_t Addr =
          Address.Address + (STI.getTargetTriple().isX86() ? Bytes.size() : 0);
      IP.printInst(MI, Addr, "", STI, OS);
      //portedprint::printInstruction(MI, Addr, STI, OS);//ekledim
      //portedprint::printInst(MI, Addr, "", STI, OS, MRI); //buraya fazladan MRI parametresi
    } //else
      //OS << "\t<unknown>";
  }
};
PrettyPrinter PrettyPrinterInst;

class HexagonPrettyPrinter : public PrettyPrinter {
public:
  void printLead(ArrayRef<uint8_t> Bytes, uint64_t Address,
                 formatted_raw_ostream &OS) { OS<<"deneme"; // buraya girmiyor demek ki
    uint32_t opcode =
      (Bytes[3] << 24) | (Bytes[2] << 16) | (Bytes[1] << 8) | Bytes[0];
    //if (!NoLeadingAddr)
    //  OS << format("%8" PRIx64 ":", Address);
    if (!NoShowRawInsn) {
      OS << "\t";
      //dumpBytes(Bytes.slice(0, 4), OS);
      //OS << format("\t%08" PRIx32, opcode);
      //degistirdim
      OS << opcode;
    }
  }
  //void printInst(MCInstPrinter &IP, const MCInst *MI, ArrayRef<uint8_t> Bytes,
  //               object::SectionedAddress Address, formatted_raw_ostream &OS,
  //               StringRef Annot, MCSubtargetInfo const &STI, SourcePrinter *SP,
  //               StringRef ObjectFilename, std::vector<RelocationRef> *Rels,
  //               LiveVariablePrinter &LVP) override {
  //  if (SP && (PrintSource || PrintLines))
  //    SP->printSourceLine(OS, Address, ObjectFilename, LVP, "");
  //  if (!MI) {
  //    printLead(Bytes, Address.Address, OS);
  //    //OS << " <unknown>";
  //    return;
  //  }
  //  std::string Buffer;
  //  {
  //    raw_string_ostream TempStream(Buffer);
  //    IP.printInst(MI, Address.Address, "", STI, TempStream);
  //  }
  //  StringRef Contents(Buffer);
  //  // Split off bundle attributes
  //  auto PacketBundle = Contents.rsplit('\n');
  //  // Split off first instruction from the rest
  //  auto HeadTail = PacketBundle.first.split('\n');
  //  auto Preamble = " { ";
  //  auto Separator = "";
//
  //  // Hexagon's packets require relocations to be inline rather than
  //  // clustered at the end of the packet.
  //  std::vector<RelocationRef>::const_iterator RelCur = Rels->begin();
  //  std::vector<RelocationRef>::const_iterator RelEnd = Rels->end();
  //  auto PrintReloc = [&]() -> void {
  //    while ((RelCur != RelEnd) && (RelCur->getOffset() <= Address.Address)) {
  //      if (RelCur->getOffset() == Address.Address) {
  //        printRelocation(OS, ObjectFilename, *RelCur, Address.Address, false);
  //        return;
  //      }
  //      ++RelCur;
  //    }
  //  };
//
  //  while (!HeadTail.first.empty()) {
  //    OS << Separator;
  //    Separator = "\n";
  //    if (SP && (PrintSource || PrintLines))
  //      SP->printSourceLine(OS, Address, ObjectFilename, LVP, "");
  //    printLead(Bytes, Address.Address, OS);
  //    OS << Preamble;
  //    Preamble = "   ";
  //    StringRef Inst;
  //    auto Duplex = HeadTail.first.split('\v');
  //    if (!Duplex.second.empty()) {
  //      OS << Duplex.first;
  //      OS << "; ";
  //      Inst = Duplex.second;
  //    }
  //    else
  //      Inst = HeadTail.first;
  //    OS << Inst;
  //    HeadTail = HeadTail.second.split('\n');
  //    if (HeadTail.first.empty())
  //      OS << " } " << PacketBundle.second;
  //    PrintReloc();
  //    Bytes = Bytes.slice(4);
  //    Address.Address += 4;
  //  }
  //}
};
HexagonPrettyPrinter HexagonPrettyPrinterInst;

class AMDGCNPrettyPrinter : public PrettyPrinter {
public:
  //void printInst(MCInstPrinter &IP, const MCInst *MI, ArrayRef<uint8_t> Bytes,
  //               object::SectionedAddress Address, formatted_raw_ostream &OS,
  //               StringRef Annot, MCSubtargetInfo const &STI, SourcePrinter *SP,
  //               StringRef ObjectFilename, std::vector<RelocationRef> *Rels,
  //               LiveVariablePrinter &LVP) override {
  //  if (SP && (PrintSource || PrintLines))
  //    SP->printSourceLine(OS, Address, ObjectFilename, LVP);
//
  //  if (MI) {
  //    SmallString<40> InstStr;
  //    raw_svector_ostream IS(InstStr);
//
  //    IP.printInst(MI, Address.Address, "", STI, IS);
//
  //    OS << left_justify(IS.str(), 60);
  //  } else {
  //    // an unrecognized encoding - this is probably data so represent it
  //    // using the .long directive, or .byte directive if fewer than 4 bytes
  //    // remaining
  //    if (Bytes.size() >= 4) {
  //      OS << format("\t.long 0x%08" PRIx32 " ",
  //                   support::endian::read32<support::little>(Bytes.data()));
  //      OS.indent(42);
  //    } else {
  //        OS << format("\t.byte 0x%02" PRIx8, Bytes[0]);
  //        for (unsigned int i = 1; i < Bytes.size(); i++)
  //          OS << format(", 0x%02" PRIx8, Bytes[i]);
  //        OS.indent(55 - (6 * Bytes.size()));
  //    }
  //  }
//
  //  OS << format("// %012" PRIX64 ":", Address.Address);
  //  if (Bytes.size() >= 4) {
  //    // D should be casted to uint32_t here as it is passed by format to
  //    // snprintf as vararg.
  //    for (uint32_t D : makeArrayRef(
  //             reinterpret_cast<const support::little32_t *>(Bytes.data()),
  //             Bytes.size() / 4))
  //      OS << format(" %08" PRIX32, D);
  //  } else {
  //    for (unsigned char B : Bytes)
  //      OS << format(" %02" PRIX8, B);
  //  }
//
  //  if (!Annot.empty())
  //    OS << " // " << Annot;
  //}
};
AMDGCNPrettyPrinter AMDGCNPrettyPrinterInst;

class BPFPrettyPrinter : public PrettyPrinter {
public:
  //void printInst(MCInstPrinter &IP, const MCInst *MI, ArrayRef<uint8_t> Bytes,
  //               object::SectionedAddress Address, formatted_raw_ostream &OS,
  //               StringRef Annot, MCSubtargetInfo const &STI, SourcePrinter *SP,
  //               StringRef ObjectFilename, std::vector<RelocationRef> *Rels,
  //               LiveVariablePrinter &LVP) override {
  //  if (SP && (PrintSource || PrintLines))
  //    SP->printSourceLine(OS, Address, ObjectFilename, LVP);//degistirdim
  //  //if (!NoLeadingAddr)
  //  //  OS << format("%8" PRId64 ":", Address.Address / 8);
  //  if (!NoShowRawInsn) {
  //    OS << "\t";
  //    //dumpBytes(Bytes, OS);
  //  }
  //  if (MI)
  //    IP.printInst(MI, Address.Address, "", STI, OS);
  //  //else
  //  //  OS << "\t<unknown>";
  //}
};
BPFPrettyPrinter BPFPrettyPrinterInst;

PrettyPrinter &selectPrettyPrinter(Triple const &Triple) {
  switch(Triple.getArch()) {
  default:
    return PrettyPrinterInst;
  case Triple::hexagon:
    return HexagonPrettyPrinterInst;
  case Triple::amdgcn:
    return AMDGCNPrettyPrinterInst;
  case Triple::bpfel:
  case Triple::bpfeb:
    return BPFPrettyPrinterInst;
  }
}
}

static uint8_t getElfSymbolType(const ObjectFile *Obj, const SymbolRef &Sym) {
  assert(Obj->isELF());
  if (auto *Elf32LEObj = dyn_cast<ELF32LEObjectFile>(Obj))
    return Elf32LEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  if (auto *Elf64LEObj = dyn_cast<ELF64LEObjectFile>(Obj))
    return Elf64LEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  if (auto *Elf32BEObj = dyn_cast<ELF32BEObjectFile>(Obj))
    return Elf32BEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  if (auto *Elf64BEObj = cast<ELF64BEObjectFile>(Obj))
    return Elf64BEObj->getSymbol(Sym.getRawDataRefImpl())->getType();
  llvm_unreachable("Unsupported binary format");
}

template <class ELFT> static void
addDynamicElfSymbols(const ELFObjectFile<ELFT> *Obj,
                     std::map<SectionRef, SectionSymbolsTy> &AllSymbols) {
  for (auto Symbol : Obj->getDynamicSymbolIterators()) {
    uint8_t SymbolType = Symbol.getELFType();
    if (SymbolType == ELF::STT_SECTION)
      continue;

    uint64_t Address = unwrapOrError(Symbol.getAddress(), Obj->getFileName());
    // ELFSymbolRef::getAddress() returns size instead of value for common
    // symbols which is not desirable for disassembly output. Overriding.
    if (SymbolType == ELF::STT_COMMON)
      Address = Obj->getSymbol(Symbol.getRawDataRefImpl())->st_value;

    StringRef Name = unwrapOrError(Symbol.getName(), Obj->getFileName());
    if (Name.empty())
      continue;

    section_iterator SecI =
        unwrapOrError(Symbol.getSection(), Obj->getFileName());
    if (SecI == Obj->section_end())
      continue;

    AllSymbols[*SecI].emplace_back(Address, Name, SymbolType);
  }
}

static void
addDynamicElfSymbols(const ObjectFile *Obj,
                     std::map<SectionRef, SectionSymbolsTy> &AllSymbols) {
  assert(Obj->isELF());
  if (auto *Elf32LEObj = dyn_cast<ELF32LEObjectFile>(Obj))
    addDynamicElfSymbols(Elf32LEObj, AllSymbols);
  else if (auto *Elf64LEObj = dyn_cast<ELF64LEObjectFile>(Obj))
    addDynamicElfSymbols(Elf64LEObj, AllSymbols);
  else if (auto *Elf32BEObj = dyn_cast<ELF32BEObjectFile>(Obj))
    addDynamicElfSymbols(Elf32BEObj, AllSymbols);
  else if (auto *Elf64BEObj = cast<ELF64BEObjectFile>(Obj))
    addDynamicElfSymbols(Elf64BEObj, AllSymbols);
  else
    llvm_unreachable("Unsupported binary format");
}

static void addPltEntries(const ObjectFile *Obj,
                          std::map<SectionRef, SectionSymbolsTy> &AllSymbols,
                          StringSaver &Saver) {
  Optional<SectionRef> Plt = None;
  for (const SectionRef &Section : Obj->sections()) {
    Expected<StringRef> SecNameOrErr = Section.getName();
    if (!SecNameOrErr) {
      consumeError(SecNameOrErr.takeError());
      continue;
    }
    if (*SecNameOrErr == ".plt")
      Plt = Section;
  }
  if (!Plt)
    return;
  if (auto *ElfObj = dyn_cast<ELFObjectFileBase>(Obj)) {
    for (auto PltEntry : ElfObj->getPltAddresses()) {
      SymbolRef Symbol(PltEntry.first, ElfObj);
      uint8_t SymbolType = getElfSymbolType(Obj, Symbol);

      StringRef Name = unwrapOrError(Symbol.getName(), Obj->getFileName());
      if (!Name.empty())
        AllSymbols[*Plt].emplace_back(
            PltEntry.second, Saver.save((Name + "@plt").str()), SymbolType);
    }
  }
}

// Normally the disassembly output will skip blocks of zeroes. This function
// returns the number of zero bytes that can be skipped when dumping the
// disassembly of the instructions in Buf.
static size_t countSkippableZeroBytes(ArrayRef<uint8_t> Buf) {
  // Find the number of leading zeroes.
  size_t N = 0;
  while (N < Buf.size() && !Buf[N])
    ++N;

  // We may want to skip blocks of zero bytes, but unless we see
  // at least 8 of them in a row.
  if (N < 8)
    return 0;

  // We skip zeroes in multiples of 4 because do not want to truncate an
  // instruction if it starts with a zero byte.
  return N & ~0x3;
}

// Returns a map from sections to their relocations.
static std::map<SectionRef, std::vector<RelocationRef>>
getRelocsMap(object::ObjectFile const &Obj) {
  std::map<SectionRef, std::vector<RelocationRef>> Ret;
  uint64_t I = (uint64_t)-1;
  for (SectionRef Sec : Obj.sections()) {
    ++I;
    Expected<section_iterator> RelocatedOrErr = Sec.getRelocatedSection();
    if (!RelocatedOrErr)
      reportError(Obj.getFileName(),
                  "section (" + Twine(I) +
                      "): failed to get a relocated section: " +
                      toString(RelocatedOrErr.takeError()));

    section_iterator Relocated = *RelocatedOrErr;
    if (Relocated == Obj.section_end() || !checkSectionFilter(*Relocated).Keep)
      continue;
    std::vector<RelocationRef> &V = Ret[*Relocated];
    for (const RelocationRef &R : Sec.relocations())
      V.push_back(R);
    // Sort relocations by address.
    llvm::stable_sort(V, isRelocAddressLess);
  }
  return Ret;
}

// Used for --adjust-vma to check if address should be adjusted by the
// specified value for a given section.
// For ELF we do not adjust non-allocatable sections like debug ones,
// because they are not loadable.
// TODO: implement for other file formats.
static bool shouldAdjustVA(const SectionRef &Section) {
  const ObjectFile *Obj = Section.getObject();
  if (Obj->isELF())
    return ELFSectionRef(Section).getFlags() & ELF::SHF_ALLOC;
  return false;
}


typedef std::pair<uint64_t, char> MappingSymbolPair;
static char getMappingSymbolKind(ArrayRef<MappingSymbolPair> MappingSymbols,
                                 uint64_t Address) {
  auto It =
      partition_point(MappingSymbols, [Address](const MappingSymbolPair &Val) {
        return Val.first <= Address;
      });
  // Return zero for any address before the first mapping symbol; this means
  // we should use the default disassembly mode, depending on the target.
  if (It == MappingSymbols.begin())
    return '\x00';
  return (It - 1)->second;
}

static uint64_t dumpARMELFData(uint64_t SectionAddr, uint64_t Index,
                               uint64_t End, const ObjectFile *Obj,
                               ArrayRef<uint8_t> Bytes,
                               ArrayRef<MappingSymbolPair> MappingSymbols,
                               raw_ostream &OS) {
  support::endianness Endian =
      Obj->isLittleEndian() ? support::little : support::big; 
      // endian kısmı 
  OS << format("%8" PRIx64 ":\t", SectionAddr + Index);
  if (Index + 4 <= End) {
    //dumpBytes(Bytes.slice(Index, 4), OS);
    OS << "\t.word\t"
           << format_hex(support::endian::read32(Bytes.data() + Index, Endian),
                         10);
    return 4;
  }
  if (Index + 2 <= End) {
    //dumpBytes(Bytes.slice(Index, 2), OS);
    OS << "\t\t.short\t"
           << format_hex(support::endian::read16(Bytes.data() + Index, Endian),
                         6);
    return 2;
  }
  //dumpBytes(Bytes.slice(Index, 1), OS);
  OS << "\t\t.byte\t" << format_hex(Bytes[0], 4);
  return 1;
}

static void dumpELFData(uint64_t SectionAddr, uint64_t Index, uint64_t End,
                        ArrayRef<uint8_t> Bytes) {
  // print out data up to 8 bytes at a time in hex and ascii
  uint8_t AsciiData[9] = {'\0'};
  uint8_t Byte;
  int NumBytes = 0;

  for (; Index < End; ++Index) {
    if (NumBytes == 0)
      outs() << format("%8" PRIx64 ":", SectionAddr + Index);
    Byte = Bytes.slice(Index)[0];
    outs() << format(" %02x", Byte);
    AsciiData[NumBytes] = isPrint(Byte) ? Byte : '.';

    uint8_t IndentOffset = 0;
    NumBytes++;
    if (Index == End - 1 || NumBytes > 8) {
      // Indent the space for less than 8 bytes data.
      // 2 spaces for byte and one for space between bytes
      IndentOffset = 3 * (8 - NumBytes);
      for (int Excess = NumBytes; Excess < 8; Excess++)
        AsciiData[Excess] = '\0';
      NumBytes = 8;
    }
    if (NumBytes == 8) {
      AsciiData[8] = '\0';
      outs() << std::string(IndentOffset, ' ') << "         ";
      outs() << reinterpret_cast<char *>(AsciiData);
      //outs() << '\n'; //degistirdim
      NumBytes = 0;
    }
  }
}

SymbolInfoTy objdump::createSymbolInfo(const ObjectFile *Obj,
                                       const SymbolRef &Symbol) {
  const StringRef FileName = Obj->getFileName();
  const uint64_t Addr = unwrapOrError(Symbol.getAddress(), FileName);
  const StringRef Name = unwrapOrError(Symbol.getName(), FileName);

  if (Obj->isXCOFF() && SymbolDescription) {
    const auto *XCOFFObj = cast<XCOFFObjectFile>(Obj);
    DataRefImpl SymbolDRI = Symbol.getRawDataRefImpl();

    const uint32_t SymbolIndex = XCOFFObj->getSymbolIndex(SymbolDRI.p);
    Optional<XCOFF::StorageMappingClass> Smc =
        getXCOFFSymbolCsectSMC(XCOFFObj, Symbol);
    return SymbolInfoTy(Addr, Name, Smc, SymbolIndex,
                        isLabel(XCOFFObj, Symbol));
  } else
    return SymbolInfoTy(Addr, Name,
                        Obj->isELF() ? getElfSymbolType(Obj, Symbol)
                                     : (uint8_t)ELF::STT_NOTYPE);
}

static SymbolInfoTy createDummySymbolInfo(const ObjectFile *Obj,
                                          const uint64_t Addr, StringRef &Name,
                                          uint8_t Type) {
  if (Obj->isXCOFF() && SymbolDescription)
    return SymbolInfoTy(Addr, Name, None, None, false);
  else
    return SymbolInfoTy(Addr, Name, Type);
}

static void disassembleObject(const Target *TheTarget, const ObjectFile *Obj,
                              MCContext &Ctx, MCDisassembler *PrimaryDisAsm,
                              MCDisassembler *SecondaryDisAsm,
                              const MCInstrAnalysis *MIA, MCInstPrinter *IP,
                              const MCSubtargetInfo *PrimarySTI,
                              const MCSubtargetInfo *SecondarySTI,
                              PrettyPrinter &PIP,
                              SourcePrinter &SP, bool InlineRelocs) {
  const MCSubtargetInfo *STI = PrimarySTI;
  MCDisassembler *DisAsm = PrimaryDisAsm;

  std::map<SectionRef, std::vector<RelocationRef>> RelocMap;
  if (InlineRelocs)
    RelocMap = getRelocsMap(*Obj); // sectionda
  bool Is64Bits = Obj->getBytesInAddress() > 4; // printrelocationda kullaniliyor is64bits

  // Create a mapping from virtual address to symbol name.  This is used to
  // pretty print the symbols while disassembling.
  std::map<SectionRef, SectionSymbolsTy> AllSymbols;
  SectionSymbolsTy AbsoluteSymbols;
  const StringRef FileName = Obj->getFileName();

  for (const SymbolRef &Symbol : Obj->symbols()) {
    StringRef Name = unwrapOrError(Symbol.getName(), FileName);

    if (Obj->isELF() && getElfSymbolType(Obj, Symbol) == ELF::STT_SECTION)
      continue;


    section_iterator SecI = unwrapOrError(Symbol.getSection(), FileName);
    if (SecI != Obj->section_end())
      AllSymbols[*SecI].push_back(createSymbolInfo(Obj, Symbol));
    else
      AbsoluteSymbols.push_back(createSymbolInfo(Obj, Symbol));
  }

  if (AllSymbols.empty() && Obj->isELF())
    addDynamicElfSymbols(Obj, AllSymbols);

  BumpPtrAllocator A;
  StringSaver Saver(A);
  addPltEntries(Obj, AllSymbols, Saver);

  // Create a mapping from virtual address to section. An empty section can
  // cause more than one section at the same address. Sort such sections to be
  // before same-addressed non-empty sections so that symbol lookups prefer the
  // non-empty section.
  std::vector<std::pair<uint64_t, SectionRef>> SectionAddresses;
  for (SectionRef Sec : Obj->sections())
    SectionAddresses.emplace_back(Sec.getAddress(), Sec);
  llvm::stable_sort(SectionAddresses, [](const auto &LHS, const auto &RHS) {
    if (LHS.first != RHS.first)
      return LHS.first < RHS.first;
    return LHS.second.getSize() < RHS.second.getSize();
  });

  // Sort all the symbols, this allows us to use a simple binary search to find
  // Multiple symbols can have the same address. Use a stable sort to stabilize
  // the output.
  StringSet<> FoundDisasmSymbolSet;
  for (std::pair<const SectionRef, SectionSymbolsTy> &SecSyms : AllSymbols)
    stable_sort(SecSyms.second);
  stable_sort(AbsoluteSymbols);

  std::unique_ptr<DWARFContext> DICtx;
  LiveVariablePrinter LVP(*Ctx.getRegisterInfo(), *STI);

  if (DbgVariables != DVDisabled) {
    DICtx = DWARFContext::create(*Obj);
    for (const std::unique_ptr<DWARFUnit> &CU : DICtx->compile_units())
      LVP.addCompileUnit(CU->getUnitDIE(false));
  }

  LLVM_DEBUG(LVP.dump());

  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
    if (FilterSections.empty() &&
        (!Section.isText() || Section.isVirtual()))
      continue;

    uint64_t SectionAddr = Section.getAddress();
    uint64_t SectSize = Section.getSize();
    if (!SectSize)
      continue;

    // Get the list of all the symbols in this section.
    SectionSymbolsTy &Symbols = AllSymbols[Section];
    std::vector<MappingSymbolPair> MappingSymbols;
    if (hasMappingSymbols(Obj)) {
      for (const auto &Symb : Symbols) {
        uint64_t Address = Symb.Addr;
        StringRef Name = Symb.Name;
        if (Name.startswith("$d"))
          MappingSymbols.emplace_back(Address - SectionAddr, 'd');
        if (Name.startswith("$x"))
          MappingSymbols.emplace_back(Address - SectionAddr, 'x');
        if (Name.startswith("$a"))
          MappingSymbols.emplace_back(Address - SectionAddr, 'a');
        if (Name.startswith("$t"))
          MappingSymbols.emplace_back(Address - SectionAddr, 't');
      }
    }

    llvm::sort(MappingSymbols);

    if (Obj->isELF() && Obj->getArch() == Triple::amdgcn) {
      // AMDGPU disassembler uses symbolizer for printing labels
      std::unique_ptr<MCRelocationInfo> RelInfo(
        TheTarget->createMCRelocationInfo(TripleName, Ctx));
      if (RelInfo) {
        std::unique_ptr<MCSymbolizer> Symbolizer(
          TheTarget->createMCSymbolizer(
            TripleName, nullptr, nullptr, &Symbols, &Ctx, std::move(RelInfo)));
        DisAsm->setSymbolizer(std::move(Symbolizer));
      }
    }

    StringRef SegmentName = "";
    /*
    if (MachO) {
      DataRefImpl DR = Section.getRawDataRefImpl();
      SegmentName = MachO->getSectionFinalSegmentName(DR);
    }
*/
    StringRef SectionName = unwrapOrError(Section.getName(), Obj->getFileName());
    // If the section has no symbol at the start, just insert a dummy one.
    if (Symbols.empty() || Symbols[0].Addr != 0) {
      Symbols.insert(Symbols.begin(),
                     createDummySymbolInfo(Obj, SectionAddr, SectionName,
                                           Section.isText() ? ELF::STT_FUNC
                                                            : ELF::STT_OBJECT));
    }

    SmallString<40> Comments;
    raw_svector_ostream CommentStream(Comments);

    ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(
        unwrapOrError(Section.getContents(), Obj->getFileName()));

    uint64_t VMAAdjustment = 0;
    if (shouldAdjustVA(Section))
      VMAAdjustment = AdjustVMA;

    uint64_t Size;
    uint64_t Index;

    std::vector<RelocationRef> Rels = RelocMap[Section];
    std::vector<RelocationRef>::const_iterator RelCur = Rels.begin();
    std::vector<RelocationRef>::const_iterator RelEnd = Rels.end();
    // Disassemble symbol by symbol.
    for (unsigned SI = 0, SE = Symbols.size(); SI != SE; ++SI) {
      std::string SymbolName = Symbols[SI].Name.str();

      // Skip if --disassemble-symbols is not empty and the symbol is not in
      // the list.
      if (!DisasmSymbolSet.empty() && !DisasmSymbolSet.count(SymbolName))
        continue;

      uint64_t Start = Symbols[SI].Addr;
      if (Start < SectionAddr || StopAddress <= Start)
        continue;
      else
        FoundDisasmSymbolSet.insert(SymbolName);

      // The end is the section end, the beginning of the next symbol, or
      // --stop-address.
      uint64_t End = std::min<uint64_t>(SectionAddr + SectSize, StopAddress);
      if (SI + 1 < SE)
        End = std::min(End, Symbols[SI + 1].Addr);
      if (Start >= End || End <= StartAddress)
        continue;
      Start -= SectionAddr;
      End -= SectionAddr;


      if (Obj->isELF() && Obj->getArch() == Triple::amdgcn) {
        if (Symbols[SI].Type == ELF::STT_AMDGPU_HSA_KERNEL) {
          // skip amd_kernel_code_t at the begining of kernel symbol (256 bytes)
          Start += 256;
        }
        if (SI == SE - 1 ||
            Symbols[SI + 1].Type == ELF::STT_AMDGPU_HSA_KERNEL) {
          // cut trailing zeroes at the end of kernel
          // cut up to 256 bytes
          const uint64_t EndAlign = 256;
          const auto Limit = End - (std::min)(EndAlign, End - Start);
          while (End > Limit &&
            *reinterpret_cast<const support::ulittle32_t*>(&Bytes[End - 4]) == 0)
            End -= 4;
        }
      }


      Start += Size;

      Index = Start;
      if (SectionAddr < StartAddress)
        Index = std::max<uint64_t>(Index, StartAddress - SectionAddr);

      // If there is a data/common symbol inside an ELF text section and we are
      // only disassembling text (applicable all architectures), we are in a
      // situation where we must print the data and not disassemble it.
      if (Obj->isELF() && Section.isText()) {
        uint8_t SymTy = Symbols[SI].Type;
        if (SymTy == ELF::STT_OBJECT || SymTy == ELF::STT_COMMON) {
          dumpELFData(SectionAddr, Index, End, Bytes);
          Index = End;
        }
      }

      bool CheckARMELFData = hasMappingSymbols(Obj) &&
                             Symbols[SI].Type != ELF::STT_OBJECT;

      formatted_raw_ostream FOS(outs());
      while (Index < End) {

            uint64_t MaxOffset = End - Index;

            if (RelCur != RelEnd)
              MaxOffset = RelCur->getOffset() - Index;

            if (size_t N =
                    countSkippableZeroBytes(Bytes.slice(Index, MaxOffset))) {
              //FOS << "\t\t..." << '\n'; // degistirdim yoruma aldim
              Index += N;
              continue;
            }

          MCInst Inst;
          bool Disassembled =
              DisAsm->getInstruction(Inst, Size, Bytes.slice(Index),
                                     SectionAddr + Index, CommentStream);
          if (Size == 0)
            Size = 1;

          LVP.update({Index, Section.getIndex()},
                     {Index + Size, Section.getIndex()}, Index + Size != End);
          
          //Disassembled = true; //
//degistirdim
//const Target *TheTarget = getTarget(Obj); //buraya geliyomus zaten
      std::unique_ptr<const MCRegisterInfo> MRI( //buraya fazladan MRI parametresi
      TheTarget->createMCRegInfo(TripleName));
          PIP.printInst(
              *IP, Disassembled ? &Inst : nullptr, Bytes.slice(Index, Size), // ekledim
              {SectionAddr + Index + VMAAdjustment, Section.getIndex()}, FOS,
              "", *STI, &SP, Obj->getFileName(), &Rels, LVP); //, *MRI); //buraya fazladan MRI parametresi ekleyelim.

          if (Disassembled && MIA) { // bura cok onemli disassembled olmazsa yazmıyor hexleri
            uint64_t Target;
            bool PrintTarget =
                MIA->evaluateBranch(Inst, SectionAddr + Index, Size, Target);
            if (!PrintTarget)
              if (Optional<uint64_t> MaybeTarget =
                      MIA->evaluateMemoryOperandAddress(
                          Inst, SectionAddr + Index, Size)) {
                Target = *MaybeTarget;
                PrintTarget = true;
                FOS << "  # " << Twine::utohexstr(Target); // bura da onemli
              }

          }
        

        LVP.printAfterInst(FOS); //bura printafterinstteki boşluklar olmasa, bu fonk çalışmasa hexi dosyaya basmıyor
        OSS << "\n"; // FOS degistirdim

        // Hexagon does this in pretty printer
        if (Obj->getArch() != Triple::hexagon) {
          // Print relocation for instruction and data.
          while (RelCur != RelEnd) {
            uint64_t Offset = RelCur->getOffset();
            // If this relocation is hidden, skip it.
            if (getHidden(*RelCur) || SectionAddr + Offset < StartAddress) {
              ++RelCur;
              continue;
            }

            // Stop when RelCur's offset is past the disassembled
            // instruction/data. Note that it's possible the disassembled data
            // is not the complete data: we might see the relocation printed in
            // the middle of the data, but this matches the binutils objdump
            // output.
            if (Offset >= Index + Size)
              break;

            // When --adjust-vma is used, update the address printed.
            if (RelCur->getSymbol() != Obj->symbol_end()) {
              Expected<section_iterator> SymSI =
                  RelCur->getSymbol()->getSection();
              if (SymSI && *SymSI != Obj->section_end() &&
                  shouldAdjustVA(**SymSI))
                Offset += AdjustVMA;
            }

            printRelocation(FOS, Obj->getFileName(), *RelCur,
                            SectionAddr + Offset, Is64Bits);
            LVP.printAfterOtherLine(FOS, true);
            ++RelCur;
          }
        }

        Index += Size;
      }
    }
  }
}

//static void disassembleObject(const Target *TheTarget, const ObjectFile *Obj,
//                              MCContext &Ctx, MCDisassembler *PrimaryDisAsm,
//                              MCDisassembler *SecondaryDisAsm,
//                              const MCInstrAnalysis *MIA, MCInstPrinter *IP,
//                              const MCSubtargetInfo *PrimarySTI,
//                              const MCSubtargetInfo *SecondarySTI,
//                              PrettyPrinter &PIP,
//                              SourcePrinter &SP, bool InlineRelocs) {
//  const MCSubtargetInfo *STI = PrimarySTI;
//  MCDisassembler *DisAsm = PrimaryDisAsm;
//  bool PrimaryIsThumb = false;
//  if (isArmElf(Obj))
//    PrimaryIsThumb = STI->checkFeatures("+thumb-mode");
//
//  std::map<SectionRef, std::vector<RelocationRef>> RelocMap;
//  if (InlineRelocs)
//    RelocMap = getRelocsMap(*Obj);
//  bool Is64Bits = Obj->getBytesInAddress() > 4;
//
//  // Create a mapping from virtual address to symbol name.  This is used to
//  // pretty print the symbols while disassembling.
//  std::map<SectionRef, SectionSymbolsTy> AllSymbols;
//  SectionSymbolsTy AbsoluteSymbols;
//  const StringRef FileName = Obj->getFileName();
//  const MachOObjectFile *MachO = dyn_cast<const MachOObjectFile>(Obj);
//  for (const SymbolRef &Symbol : Obj->symbols()) {
//    StringRef Name = unwrapOrError(Symbol.getName(), FileName);
//    if (Name.empty() && !(Obj->isXCOFF() && SymbolDescription))
//      continue;
//
//    if (Obj->isELF() && getElfSymbolType(Obj, Symbol) == ELF::STT_SECTION)
//      continue;
//
//    // Don't ask a Mach-O STAB symbol for its section unless you know that
//    // STAB symbol's section field refers to a valid section index. Otherwise
//    // the symbol may error trying to load a section that does not exist.
//    if (MachO) {
//      DataRefImpl SymDRI = Symbol.getRawDataRefImpl();
//      uint8_t NType = (MachO->is64Bit() ?
//                       MachO->getSymbol64TableEntry(SymDRI).n_type:
//                       MachO->getSymbolTableEntry(SymDRI).n_type);
//      if (NType & MachO::N_STAB)
//        continue;
//    }
//
//    section_iterator SecI = unwrapOrError(Symbol.getSection(), FileName);
//    if (SecI != Obj->section_end())
//      AllSymbols[*SecI].push_back(createSymbolInfo(Obj, Symbol));
//    else
//      AbsoluteSymbols.push_back(createSymbolInfo(Obj, Symbol));
//  }
//
//  if (AllSymbols.empty() && Obj->isELF())
//    addDynamicElfSymbols(Obj, AllSymbols);
//
//  BumpPtrAllocator A;
//  StringSaver Saver(A);
//  addPltEntries(Obj, AllSymbols, Saver);
//
//  // Create a mapping from virtual address to section. An empty section can
//  // cause more than one section at the same address. Sort such sections to be
//  // before same-addressed non-empty sections so that symbol lookups prefer the
//  // non-empty section.
//  std::vector<std::pair<uint64_t, SectionRef>> SectionAddresses;
//  for (SectionRef Sec : Obj->sections())
//    SectionAddresses.emplace_back(Sec.getAddress(), Sec);
//  llvm::stable_sort(SectionAddresses, [](const auto &LHS, const auto &RHS) {
//    if (LHS.first != RHS.first)
//      return LHS.first < RHS.first;
//    return LHS.second.getSize() < RHS.second.getSize();
//  });
//
//  // Linked executables (.exe and .dll files) typically don't include a real
//  // symbol table but they might contain an export table.
//  if (const auto *COFFObj = dyn_cast<COFFObjectFile>(Obj)) {
//    for (const auto &ExportEntry : COFFObj->export_directories()) {
//      StringRef Name;
//      if (Error E = ExportEntry.getSymbolName(Name))
//        reportError(std::move(E), Obj->getFileName());
//      if (Name.empty())
//        continue;
//
//      uint32_t RVA;
//      if (Error E = ExportEntry.getExportRVA(RVA))
//        reportError(std::move(E), Obj->getFileName());
//
//      uint64_t VA = COFFObj->getImageBase() + RVA;
//      auto Sec = partition_point(
//          SectionAddresses, [VA](const std::pair<uint64_t, SectionRef> &O) {
//            return O.first <= VA;
//          });
//      if (Sec != SectionAddresses.begin()) {
//        --Sec;
//        AllSymbols[Sec->second].emplace_back(VA, Name, ELF::STT_NOTYPE);
//      } else
//        AbsoluteSymbols.emplace_back(VA, Name, ELF::STT_NOTYPE);
//    }
//  }
//
//  // Sort all the symbols, this allows us to use a simple binary search to find
//  // Multiple symbols can have the same address. Use a stable sort to stabilize
//  // the output.
//  StringSet<> FoundDisasmSymbolSet;
//  for (std::pair<const SectionRef, SectionSymbolsTy> &SecSyms : AllSymbols)
//    stable_sort(SecSyms.second);
//  stable_sort(AbsoluteSymbols);
//
//  std::unique_ptr<DWARFContext> DICtx;
//  LiveVariablePrinter LVP(*Ctx.getRegisterInfo(), *STI);
//
//  if (DbgVariables != DVDisabled) {
//    DICtx = DWARFContext::create(*Obj);
//    for (const std::unique_ptr<DWARFUnit> &CU : DICtx->compile_units())
//      LVP.addCompileUnit(CU->getUnitDIE(false));
//  }
//
//  LLVM_DEBUG(LVP.dump());
//
//  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
//    if (FilterSections.empty() && !DisassembleAll &&
//        (!Section.isText() || Section.isVirtual()))
//      continue;
//
//    uint64_t SectionAddr = Section.getAddress();
//    uint64_t SectSize = Section.getSize();
//    if (!SectSize)
//      continue;
//
//    // Get the list of all the symbols in this section.
//    SectionSymbolsTy &Symbols = AllSymbols[Section];
//    std::vector<MappingSymbolPair> MappingSymbols;
//    if (hasMappingSymbols(Obj)) {
//      for (const auto &Symb : Symbols) {
//        uint64_t Address = Symb.Addr;
//        StringRef Name = Symb.Name;
//        if (Name.startswith("$d"))
//          MappingSymbols.emplace_back(Address - SectionAddr, 'd');
//        if (Name.startswith("$x"))
//          MappingSymbols.emplace_back(Address - SectionAddr, 'x');
//        if (Name.startswith("$a"))
//          MappingSymbols.emplace_back(Address - SectionAddr, 'a');
//        if (Name.startswith("$t"))
//          MappingSymbols.emplace_back(Address - SectionAddr, 't');
//      }
//    }
//
//    llvm::sort(MappingSymbols);
//
//    if (Obj->isELF() && Obj->getArch() == Triple::amdgcn) {
//      // AMDGPU disassembler uses symbolizer for printing labels
//      std::unique_ptr<MCRelocationInfo> RelInfo(
//        TheTarget->createMCRelocationInfo(TripleName, Ctx));
//      if (RelInfo) {
//        std::unique_ptr<MCSymbolizer> Symbolizer(
//          TheTarget->createMCSymbolizer(
//            TripleName, nullptr, nullptr, &Symbols, &Ctx, std::move(RelInfo)));
//        DisAsm->setSymbolizer(std::move(Symbolizer));
//      }
//    }
//
//    StringRef SegmentName = "";
//    if (MachO) {
//      DataRefImpl DR = Section.getRawDataRefImpl();
//      SegmentName = MachO->getSectionFinalSegmentName(DR);
//    }
//
//    StringRef SectionName = unwrapOrError(Section.getName(), Obj->getFileName());
//    // If the section has no symbol at the start, just insert a dummy one.
//    if (Symbols.empty() || Symbols[0].Addr != 0) {
//      Symbols.insert(Symbols.begin(),
//                     createDummySymbolInfo(Obj, SectionAddr, SectionName,
//                                           Section.isText() ? ELF::STT_FUNC
//                                                            : ELF::STT_OBJECT));
//    }
//
//    SmallString<40> Comments;
//    raw_svector_ostream CommentStream(Comments);
//
//    ArrayRef<uint8_t> Bytes = arrayRefFromStringRef(
//        unwrapOrError(Section.getContents(), Obj->getFileName())); // bytes i aldigi yer
//
//    uint64_t VMAAdjustment = 0;
//    if (shouldAdjustVA(Section))
//      VMAAdjustment = AdjustVMA;
//
//    uint64_t Size;
//    uint64_t Index;
//    bool PrintedSection = false;
//    std::vector<RelocationRef> Rels = RelocMap[Section];
//    std::vector<RelocationRef>::const_iterator RelCur = Rels.begin();
//    std::vector<RelocationRef>::const_iterator RelEnd = Rels.end();
//    // Disassemble symbol by symbol.
//    for (unsigned SI = 0, SE = Symbols.size(); SI != SE; ++SI) {
//      std::string SymbolName = Symbols[SI].Name.str();
//      if (Demangle)
//        SymbolName = demangle(SymbolName);
//
//      // Skip if --disassemble-symbols is not empty and the symbol is not in
//      // the list.
//      if (!DisasmSymbolSet.empty() && !DisasmSymbolSet.count(SymbolName))
//        continue;
//
//      uint64_t Start = Symbols[SI].Addr;
//      if (Start < SectionAddr || StopAddress <= Start)
//        continue;
//      else
//        FoundDisasmSymbolSet.insert(SymbolName);
//
//      // The end is the section end, the beginning of the next symbol, or
//      // --stop-address.
//      uint64_t End = std::min<uint64_t>(SectionAddr + SectSize, StopAddress);
//      if (SI + 1 < SE)
//        End = std::min(End, Symbols[SI + 1].Addr);
//      if (Start >= End || End <= StartAddress)
//        continue;
//      Start -= SectionAddr;
//      End -= SectionAddr;
//
//      if (!PrintedSection) {
//        PrintedSection = true;
//        //outs() << "\nDisassembly of section ";
//        //if (!SegmentName.empty())
//          //outs() << SegmentName << ",";
//        //outs() << SectionName << ":\n";
//      }
//
//      if (Obj->isELF() && Obj->getArch() == Triple::amdgcn) {
//        if (Symbols[SI].Type == ELF::STT_AMDGPU_HSA_KERNEL) {
//          // skip amd_kernel_code_t at the begining of kernel symbol (256 bytes)
//          Start += 256;
//        }
//        if (SI == SE - 1 ||
//            Symbols[SI + 1].Type == ELF::STT_AMDGPU_HSA_KERNEL) {
//          // cut trailing zeroes at the end of kernel
//          // cut up to 256 bytes
//          const uint64_t EndAlign = 256;
//          const auto Limit = End - (std::min)(EndAlign, End - Start);
//          while (End > Limit &&
//            *reinterpret_cast<const support::ulittle32_t*>(&Bytes[End - 4]) == 0)
//            End -= 4;
//        }
//      }
//
//      //outs() << '\n';
//      //if (!NoLeadingAddr)
//        //outs() << format(Is64Bits ? "%016" PRIx64 " " : "%08" PRIx64 " ",
//          //               SectionAddr + Start + VMAAdjustment);
//      /*if (Obj->isXCOFF() && SymbolDescription) {
//        outs() << getXCOFFSymbolDescription(Symbols[SI], SymbolName) << ":\n";
//      } else
//        outs() << '<' << SymbolName << ">:\n";
//
//      // Don't print raw contents of a virtual section. A virtual section
//      // doesn't have any contents in the file.
//      if (Section.isVirtual()) {
//        outs() << "...\n";
//        continue;
//      }*/
//
//      auto Status = DisAsm->onSymbolStart(Symbols[SI], Size,
//                                          Bytes.slice(Start, End - Start),
//                                          SectionAddr + Start, CommentStream);
//      // To have round trippable disassembly, we fall back to decoding the
//      // remaining bytes as instructions.
//      //
//      // If there is a failure, we disassemble the failed region as bytes before
//      // falling back. The target is expected to print nothing in this case.
//      //
//      // If there is Success or SoftFail i.e no 'real' failure, we go ahead by
//      // Size bytes before falling back.
//      // So if the entire symbol is 'eaten' by the target:
//      //   Start += Size  // Now Start = End and we will never decode as
//      //                  // instructions
//      //
//      // Right now, most targets return None i.e ignore to treat a symbol
//      // separately. But WebAssembly decodes preludes for some symbols.
//      //
//      if (Status.hasValue()) {
//        if (Status.getValue() == MCDisassembler::Fail) {
//          outs() << "// Error in decoding " << SymbolName
//                 << " : Decoding failed region as bytes.\n";
//          for (uint64_t I = 0; I < Size; ++I) {
//            outs() << "\t.byte\t " << format_hex(Bytes[I], 1, /*Upper=*/true)
//                   << "\n";
//          }
//        }
//      } else {
//        Size = 0;
//      }
//
//      Start += Size;
//
//      Index = Start;
//      if (SectionAddr < StartAddress)
//        Index = std::max<uint64_t>(Index, StartAddress - SectionAddr);
//bool DisassembleAll = false;
//      // If there is a data/common symbol inside an ELF text section and we are
//      // only disassembling text (applicable all architectures), we are in a
//      // situation where we must print the data and not disassemble it.
//      if (Obj->isELF() && !DisassembleAll && Section.isText()) {
//        uint8_t SymTy = Symbols[SI].Type;
//        if (SymTy == ELF::STT_OBJECT || SymTy == ELF::STT_COMMON) {
//          dumpELFData(SectionAddr, Index, End, Bytes);
//          Index = End;
//        }
//      }
//
//      bool CheckARMELFData = hasMappingSymbols(Obj) &&
//                             Symbols[SI].Type != ELF::STT_OBJECT &&
//                             !DisassembleAll;
//      bool DumpARMELFData = false;
//      formatted_raw_ostream FOS(outs());
//      while (Index < End) {
//        // ARM and AArch64 ELF binaries can interleave data and text in the
//        // same section. We rely on the markers introduced to understand what
//        // we need to dump. If the data marker is within a function, it is
//        // denoted as a word/short etc.
//        if (CheckARMELFData) {
//          char Kind = getMappingSymbolKind(MappingSymbols, Index);
//          DumpARMELFData = Kind == 'd';
//          if (SecondarySTI) {
//            if (Kind == 'a') {
//              STI = PrimaryIsThumb ? SecondarySTI : PrimarySTI;
//              DisAsm = PrimaryIsThumb ? SecondaryDisAsm : PrimaryDisAsm;
//            } else if (Kind == 't') {
//              STI = PrimaryIsThumb ? PrimarySTI : SecondarySTI;
//              DisAsm = PrimaryIsThumb ? PrimaryDisAsm : SecondaryDisAsm;
//            }
//          }
//        }
//bool DisassembleZeroes = false;
//        if (DumpARMELFData) {
//          Size = dumpARMELFData(SectionAddr, Index, End, Obj, Bytes,
//                                MappingSymbols, FOS);
//        } else {
//          // When -z or --disassemble-zeroes are given we always dissasemble
//          // them. Otherwise we might want to skip zero bytes we see.
//          if (!DisassembleZeroes) {
//            uint64_t MaxOffset = End - Index;
//            // For --reloc: print zero blocks patched by relocations, so that
//            // relocations can be shown in the dump.
//            if (RelCur != RelEnd)
//              MaxOffset = RelCur->getOffset() - Index;
//
//            if (size_t N =
//                    countSkippableZeroBytes(Bytes.slice(Index, MaxOffset))) {
//              FOS << "\t\t..." << '\n';
//              Index += N;
//              continue;
//            }
//          }
//
//          // Disassemble a real instruction or a data when disassemble all is
//          // provided
//          MCInst Inst;
//          bool Disassembled =
//              DisAsm->getInstruction(Inst, Size, Bytes.slice(Index),
//                                     SectionAddr + Index, CommentStream);
//          if (Size == 0)
//            Size = 1;
//
//          LVP.update({Index, Section.getIndex()},
//                     {Index + Size, Section.getIndex()}, Index + Size != End);
////degistirdim
//          PIP.printInst(
//              *IP, !Disassembled ? &Inst : nullptr, Bytes.slice(Index, Size),
//              {SectionAddr + Index + VMAAdjustment, Section.getIndex()}, FOS,
//              "", *STI, &SP, Obj->getFileName(), &Rels, LVP); LVP.printAfterInst(FOS); FOS << "\n";} Index += Size;} // while in bittigi yer
//              } } // buraya 4 tane } ekleyince kapandı blocklar
////          /*FOS << CommentStream.str();
////          Comments.clear();*/
////
////          // If disassembly has failed, avoid analysing invalid/incomplete
////          // instruction information. Otherwise, try to resolve the target
////          // address (jump target or memory operand address) and print it on the
////          // right of the instruction.
////          if (Disassembled && MIA) {
////            uint64_t Target;
////            bool PrintTarget =
////                MIA->evaluateBranch(Inst, SectionAddr + Index, Size, Target);
////            if (!PrintTarget)
////              if (Optional<uint64_t> MaybeTarget =
////                      MIA->evaluateMemoryOperandAddress(
////                          Inst, SectionAddr + Index, Size)) {
////                Target = *MaybeTarget;
////                PrintTarget = true;
////                FOS << "  # " << Twine::utohexstr(Target);
////              }
////            if (PrintTarget) {
////              // In a relocatable object, the target's section must reside in
////              // the same section as the call instruction or it is accessed
////              // through a relocation.
////              //
////              // In a non-relocatable object, the target may be in any section.
////              // In that case, locate the section(s) containing the target
////              // address and find the symbol in one of those, if possible.
////              //
////              // N.B. We don't walk the relocations in the relocatable case yet.
////              std::vector<const SectionSymbolsTy *> TargetSectionSymbols;
////              if (!Obj->isRelocatableObject()) {
////                auto It = llvm::partition_point(
////                    SectionAddresses,
////                    [=](const std::pair<uint64_t, SectionRef> &O) {
////                      return O.first <= Target;
////                    });
////                uint64_t TargetSecAddr = 0;
////                while (It != SectionAddresses.begin()) {
////                  --It;
////                  if (TargetSecAddr == 0)
////                    TargetSecAddr = It->first;
////                  if (It->first != TargetSecAddr)
////                    break;
////                  TargetSectionSymbols.push_back(&AllSymbols[It->second]);
////                }
////              } else {
////                TargetSectionSymbols.push_back(&Symbols);
////              }
////              TargetSectionSymbols.push_back(&AbsoluteSymbols);
////
////              // Find the last symbol in the first candidate section whose
////              // offset is less than or equal to the target. If there are no
////              // such symbols, try in the next section and so on, before finally
////              // using the nearest preceding absolute symbol (if any), if there
////              // are no other valid symbols.
////              const SymbolInfoTy *TargetSym = nullptr;
////              for (const SectionSymbolsTy *TargetSymbols :
////                   TargetSectionSymbols) {
////                auto It = llvm::partition_point(
////                    *TargetSymbols,
////                    [=](const SymbolInfoTy &O) { return O.Addr <= Target; });
////                if (It != TargetSymbols->begin()) {
////                  TargetSym = &*(It - 1);
////                  break;
////                }
////              }
////
////              if (TargetSym != nullptr) {
////                uint64_t TargetAddress = TargetSym->Addr;
////                std::string TargetName = TargetSym->Name.str();
////                if (Demangle)
////                  TargetName = demangle(TargetName);
////
////                FOS << " <" << TargetName;
////                uint64_t Disp = Target - TargetAddress;
////                if (Disp)
////                  FOS << "+0x" << Twine::utohexstr(Disp);
////                FOS << '>';
////              }
////            }
////          }
////        }
////
////        LVP.printAfterInst(FOS);
////        FOS << "\n";
////
////        // Hexagon does this in pretty printer
////        if (Obj->getArch() != Triple::hexagon) {
////          // Print relocation for instruction and data.
////          while (RelCur != RelEnd) {
////            uint64_t Offset = RelCur->getOffset();
////            // If this relocation is hidden, skip it.
////            if (getHidden(*RelCur) || SectionAddr + Offset < StartAddress) {
////              ++RelCur;
////              continue;
////            }
////
////            // Stop when RelCur's offset is past the disassembled
////            // instruction/data. Note that it's possible the disassembled data
////            // is not the complete data: we might see the relocation printed in
////            // the middle of the data, but this matches the binutils objdump
////            // output.
////            if (Offset >= Index + Size)
////              break;
////
////            // When --adjust-vma is used, update the address printed.
////            if (RelCur->getSymbol() != Obj->symbol_end()) {
////              Expected<section_iterator> SymSI =
////                  RelCur->getSymbol()->getSection();
////              if (SymSI && *SymSI != Obj->section_end() &&
////                  shouldAdjustVA(**SymSI))
////                Offset += AdjustVMA;
////            }
////
////            printRelocation(FOS, Obj->getFileName(), *RelCur,
////                            SectionAddr + Offset, Is64Bits);
////            LVP.printAfterOtherLine(FOS, true);
////            ++RelCur;
////          }
////        }
////
////        Index += Size;
////      }
////    }
////  }
////  StringSet<> MissingDisasmSymbolSet =
////      set_difference(DisasmSymbolSet, FoundDisasmSymbolSet);
////  for (StringRef Sym : MissingDisasmSymbolSet.keys())
////    reportWarning("failed to disassemble missing symbol " + Sym, FileName);
//}

static void disassembleObject(const ObjectFile *Obj, bool InlineRelocs) {
  const Target *TheTarget = getTarget(Obj);

  // Package up features to be passed to target/subtarget
  SubtargetFeatures Features = Obj->getFeatures();
  if (!MAttrs.empty())
    for (unsigned I = 0; I != MAttrs.size(); ++I)
      Features.AddFeature(MAttrs[I]);

  std::unique_ptr<const MCRegisterInfo> MRI(
      TheTarget->createMCRegInfo(TripleName));
  if (!MRI)
    reportError(Obj->getFileName(),
                "no register info for target " + TripleName);

  // Set up disassembler.
  MCTargetOptions MCOptions;
  std::unique_ptr<const MCAsmInfo> AsmInfo(
      TheTarget->createMCAsmInfo(*MRI, TripleName, MCOptions));
  if (!AsmInfo)
    reportError(Obj->getFileName(),
                "no assembly info for target " + TripleName);
  std::unique_ptr<const MCSubtargetInfo> STI(
      TheTarget->createMCSubtargetInfo(TripleName, MCPU, Features.getString()));
  if (!STI)
    reportError(Obj->getFileName(),
                "no subtarget info for target " + TripleName);
  std::unique_ptr<const MCInstrInfo> MII(TheTarget->createMCInstrInfo());
  if (!MII)
    reportError(Obj->getFileName(),
                "no instruction info for target " + TripleName);
  MCObjectFileInfo MOFI;
  MCContext Ctx(AsmInfo.get(), MRI.get(), &MOFI);
  // FIXME: for now initialize MCObjectFileInfo with default values
  MOFI.InitMCObjectFileInfo(Triple(TripleName), false, Ctx);

  std::unique_ptr<MCDisassembler> DisAsm(
      TheTarget->createMCDisassembler(*STI, Ctx));
  if (!DisAsm)
    reportError(Obj->getFileName(), "no disassembler for target " + TripleName);

  // If we have an ARM object file, we need a second disassembler, because
  // ARM CPUs have two different instruction sets: ARM mode, and Thumb mode.
  // We use mapping symbols to switch between the two assemblers, where
  // appropriate.
  std::unique_ptr<MCDisassembler> SecondaryDisAsm;
  std::unique_ptr<const MCSubtargetInfo> SecondarySTI;
  if (isArmElf(Obj) && !STI->checkFeatures("+mclass")) {
    if (STI->checkFeatures("+thumb-mode"))
      Features.AddFeature("-thumb-mode");
    else
      Features.AddFeature("+thumb-mode");
    SecondarySTI.reset(TheTarget->createMCSubtargetInfo(TripleName, MCPU,
                                                        Features.getString()));
    SecondaryDisAsm.reset(TheTarget->createMCDisassembler(*SecondarySTI, Ctx));
  }

  std::unique_ptr<const MCInstrAnalysis> MIA(
      TheTarget->createMCInstrAnalysis(MII.get()));

  int AsmPrinterVariant = AsmInfo->getAssemblerDialect();
  std::unique_ptr<MCInstPrinter> IP(TheTarget->createMCInstPrinter(
      Triple(TripleName), AsmPrinterVariant, *AsmInfo, *MII, *MRI));
  if (!IP)
    reportError(Obj->getFileName(),
                "no instruction printer for target " + TripleName);
  IP->setPrintImmHex(PrintImmHex);
  IP->setPrintBranchImmAsAddress(true);

  PrettyPrinter &PIP = selectPrettyPrinter(Triple(TripleName));
  SourcePrinter SP(Obj, TheTarget->getName());

  for (StringRef Opt : DisassemblerOptions)
    if (!IP->applyTargetSpecificCLOption(Opt))
      reportError(Obj->getFileName(),
                  "Unrecognized disassembler option: " + Opt);

  disassembleObject(TheTarget, Obj, Ctx, DisAsm.get(), SecondaryDisAsm.get(),
                    MIA.get(), IP.get(), STI.get(), SecondarySTI.get(), PIP,
                    SP, InlineRelocs);
}

void objdump::printRelocations(const ObjectFile *Obj) {
  StringRef Fmt = Obj->getBytesInAddress() > 4 ? "%016" PRIx64 :
                                                 "%08" PRIx64;
  // Regular objdump doesn't print relocations in non-relocatable object
  // files.
  if (!Obj->isRelocatableObject())
    return;

  // Build a mapping from relocation target to a vector of relocation
  // sections. Usually, there is an only one relocation section for
  // each relocated section.
  MapVector<SectionRef, std::vector<SectionRef>> SecToRelSec;
  uint64_t Ndx;
  for (const SectionRef &Section : ToolSectionFilter(*Obj, &Ndx)) {
    if (Section.relocation_begin() == Section.relocation_end())
      continue;
    Expected<section_iterator> SecOrErr = Section.getRelocatedSection();
    if (!SecOrErr)
      reportError(Obj->getFileName(),
                  "section (" + Twine(Ndx) +
                      "): unable to get a relocation target: " +
                      toString(SecOrErr.takeError()));
    SecToRelSec[**SecOrErr].push_back(Section);
  }

  for (std::pair<SectionRef, std::vector<SectionRef>> &P : SecToRelSec) {
    StringRef SecName = unwrapOrError(P.first.getName(), Obj->getFileName());
    outs() << "RELOCATION RECORDS FOR [" << SecName << "]:\n";
    uint32_t OffsetPadding = (Obj->getBytesInAddress() > 4 ? 16 : 8);
    uint32_t TypePadding = 24;
    outs() << left_justify("OFFSET", OffsetPadding) << " "
           << left_justify("TYPE", TypePadding) << " "
           << "VALUE\n";

    for (SectionRef Section : P.second) {
      for (const RelocationRef &Reloc : Section.relocations()) {
        uint64_t Address = Reloc.getOffset();
        SmallString<32> RelocName;
        SmallString<32> ValueStr;
        if (Address < StartAddress || Address > StopAddress || getHidden(Reloc))
          continue;
        Reloc.getTypeName(RelocName);
        if (Error E = getRelocationValueString(Reloc, ValueStr))
          reportError(std::move(E), Obj->getFileName());

        outs() << format(Fmt.data(), Address) << " "
               << left_justify(RelocName, TypePadding) << " " << ValueStr
               << "\n";
      }
    }
    outs() << "\n"; // buradaki \n olmayinca hex codeunu dosyaya basmiyor onemli
    //degistirdim
  }
}

void objdump::printDynamicRelocations(const ObjectFile *Obj) {
  // For the moment, this option is for ELF only
  if (!Obj->isELF())
    return;

  const auto *Elf = dyn_cast<ELFObjectFileBase>(Obj);
  if (!Elf || Elf->getEType() != ELF::ET_DYN) {
    reportError(Obj->getFileName(), "not a dynamic object");
    return;
  }

  std::vector<SectionRef> DynRelSec = Obj->dynamic_relocation_sections();
  if (DynRelSec.empty())
    return;

  outs() << "DYNAMIC RELOCATION RECORDS\n";
  StringRef Fmt = Obj->getBytesInAddress() > 4 ? "%016" PRIx64 : "%08" PRIx64;
  for (const SectionRef &Section : DynRelSec)
    for (const RelocationRef &Reloc : Section.relocations()) {
      uint64_t Address = Reloc.getOffset();
      SmallString<32> RelocName;
      SmallString<32> ValueStr;
      Reloc.getTypeName(RelocName);
      if (Error E = getRelocationValueString(Reloc, ValueStr))
        reportError(std::move(E), Obj->getFileName());
      outs() << format(Fmt.data(), Address) << " " << RelocName << " "
             << ValueStr << "\n";
    }
}

// Returns true if we need to show LMA column when dumping section headers. We
// show it only when the platform is ELF and either we have at least one section
// whose VMA and LMA are different and/or when --show-lma flag is used.
static bool shouldDisplayLMA(const ObjectFile *Obj) {
  if (!Obj->isELF())
    return false;
  for (const SectionRef &S : ToolSectionFilter(*Obj))
    if (S.getAddress() != getELFSectionLMA(S))
      return true;
  return ShowLMA;
}

static size_t getMaxSectionNameWidth(const ObjectFile *Obj) {
  // Default column width for names is 13 even if no names are that long.
  size_t MaxWidth = 13;
  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
    StringRef Name = unwrapOrError(Section.getName(), Obj->getFileName());
    MaxWidth = std::max(MaxWidth, Name.size());
  }
  return MaxWidth;
}

void objdump::printSectionHeaders(const ObjectFile *Obj) {
  size_t NameWidth = getMaxSectionNameWidth(Obj);
  size_t AddressWidth = 2 * Obj->getBytesInAddress();
  bool HasLMAColumn = shouldDisplayLMA(Obj);
  if (HasLMAColumn)
    outs() << "Sections:\n"
              "Idx "
           << left_justify("Name", NameWidth) << " Size     "
           << left_justify("VMA", AddressWidth) << " "
           << left_justify("LMA", AddressWidth) << " Type\n";
  else
    outs() << "Sections:\n"
              "Idx "
           << left_justify("Name", NameWidth) << " Size     "
           << left_justify("VMA", AddressWidth) << " Type\n";

  uint64_t Idx;
  for (const SectionRef &Section : ToolSectionFilter(*Obj, &Idx)) {
    StringRef Name = unwrapOrError(Section.getName(), Obj->getFileName());
    uint64_t VMA = Section.getAddress();
    if (shouldAdjustVA(Section))
      VMA += AdjustVMA;

    uint64_t Size = Section.getSize();

    std::string Type = Section.isText() ? "TEXT" : "";
    if (Section.isData())
      Type += Type.empty() ? "DATA" : " DATA";
    if (Section.isBSS())
      Type += Type.empty() ? "BSS" : " BSS";

    if (HasLMAColumn)
      outs() << format("%3" PRIu64 " %-*s %08" PRIx64 " ", Idx, NameWidth,
                       Name.str().c_str(), Size)
             << format_hex_no_prefix(VMA, AddressWidth) << " "
             << format_hex_no_prefix(getELFSectionLMA(Section), AddressWidth)
             << " " << Type << "\n";
    else
      outs() << format("%3" PRIu64 " %-*s %08" PRIx64 " ", Idx, NameWidth,
                       Name.str().c_str(), Size)
             << format_hex_no_prefix(VMA, AddressWidth) << " " << Type << "\n";
  }
  //outs() << "\n"; //degistirdim
}

void objdump::printSectionContents(const ObjectFile *Obj) {
  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
    StringRef Name = unwrapOrError(Section.getName(), Obj->getFileName());
    uint64_t BaseAddr = Section.getAddress();
    uint64_t Size = Section.getSize();
    if (!Size)
      continue;

    outs() << "Contents of section " << Name << ":\n";
    if (Section.isBSS()) {
      outs() << format("<skipping contents of bss section at [%04" PRIx64
                       ", %04" PRIx64 ")>\n",
                       BaseAddr, BaseAddr + Size);
      continue;
    }

    StringRef Contents = unwrapOrError(Section.getContents(), Obj->getFileName());

    // Dump out the content as hex and printable ascii characters.
    for (std::size_t Addr = 0, End = Contents.size(); Addr < End; Addr += 16) {
      outs() << format(" %04" PRIx64 " ", BaseAddr + Addr);
      // Dump line of hex.
      for (std::size_t I = 0; I < 16; ++I) {
        if (I != 0 && I % 4 == 0)
          outs() << ' ';
        if (Addr + I < End)
          outs() << hexdigit((Contents[Addr + I] >> 4) & 0xF, true)
                 << hexdigit(Contents[Addr + I] & 0xF, true);
        else
          outs() << "  ";
      }
      // Print ascii.
      outs() << "  ";
      for (std::size_t I = 0; I < 16 && Addr + I < End; ++I) {
        if (isPrint(static_cast<unsigned char>(Contents[Addr + I]) & 0xFF))
          outs() << Contents[Addr + I];
        else
          outs() << ".";
      }
      //outs() << "\n"; //degistirdim
    }
  }
}

void objdump::printSymbolTable(const ObjectFile *O, StringRef ArchiveName,
                               StringRef ArchitectureName, bool DumpDynamic) {
  if (O->isCOFF() && !DumpDynamic) {
    outs() << "SYMBOL TABLE:\n";
    printCOFFSymbolTable(cast<const COFFObjectFile>(O));
    return;
  }

  const StringRef FileName = O->getFileName();

  if (!DumpDynamic) {
    outs() << "SYMBOL TABLE:\n";
    for (auto I = O->symbol_begin(); I != O->symbol_end(); ++I)
      printSymbol(O, *I, FileName, ArchiveName, ArchitectureName, DumpDynamic);
    return;
  }

  outs() << "DYNAMIC SYMBOL TABLE:\n";
  if (!O->isELF()) {
    reportWarning(
        "this operation is not currently supported for this file format",
        FileName);
    return;
  }

  const ELFObjectFileBase *ELF = cast<const ELFObjectFileBase>(O);
  for (auto I = ELF->getDynamicSymbolIterators().begin();
       I != ELF->getDynamicSymbolIterators().end(); ++I)
    printSymbol(O, *I, FileName, ArchiveName, ArchitectureName, DumpDynamic);
}

void objdump::printSymbol(const ObjectFile *O, const SymbolRef &Symbol,
                          StringRef FileName, StringRef ArchiveName,
                          StringRef ArchitectureName, bool DumpDynamic) {
  const MachOObjectFile *MachO = dyn_cast<const MachOObjectFile>(O);
  uint64_t Address = unwrapOrError(Symbol.getAddress(), FileName, ArchiveName,
                                   ArchitectureName);
  if ((Address < StartAddress) || (Address > StopAddress))
    return;
  SymbolRef::Type Type =
      unwrapOrError(Symbol.getType(), FileName, ArchiveName, ArchitectureName);
  uint32_t Flags =
      unwrapOrError(Symbol.getFlags(), FileName, ArchiveName, ArchitectureName);

  // Don't ask a Mach-O STAB symbol for its section unless you know that
  // STAB symbol's section field refers to a valid section index. Otherwise
  // the symbol may error trying to load a section that does not exist.
  bool IsSTAB = false;
  if (MachO) {
    DataRefImpl SymDRI = Symbol.getRawDataRefImpl();
    uint8_t NType =
        (MachO->is64Bit() ? MachO->getSymbol64TableEntry(SymDRI).n_type
                          : MachO->getSymbolTableEntry(SymDRI).n_type);
    if (NType & MachO::N_STAB)
      IsSTAB = true;
  }
  section_iterator Section = IsSTAB
                                 ? O->section_end()
                                 : unwrapOrError(Symbol.getSection(), FileName,
                                                 ArchiveName, ArchitectureName);

  StringRef Name;
  if (Type == SymbolRef::ST_Debug && Section != O->section_end()) {
    if (Expected<StringRef> NameOrErr = Section->getName())
      Name = *NameOrErr;
    else
      consumeError(NameOrErr.takeError());

  } else {
    Name = unwrapOrError(Symbol.getName(), FileName, ArchiveName,
                         ArchitectureName);
  }

  bool Global = Flags & SymbolRef::SF_Global;
  bool Weak = Flags & SymbolRef::SF_Weak;
  bool Absolute = Flags & SymbolRef::SF_Absolute;
  bool Common = Flags & SymbolRef::SF_Common;
  bool Hidden = Flags & SymbolRef::SF_Hidden;

  char GlobLoc = ' ';
  if ((Section != O->section_end() || Absolute) && !Weak)
    GlobLoc = Global ? 'g' : 'l';
  char IFunc = ' ';
  if (O->isELF()) {
    if (ELFSymbolRef(Symbol).getELFType() == ELF::STT_GNU_IFUNC)
      IFunc = 'i';
    if (ELFSymbolRef(Symbol).getBinding() == ELF::STB_GNU_UNIQUE)
      GlobLoc = 'u';
  }

  char Debug = ' ';
  if (DumpDynamic)
    Debug = 'D';
  else if (Type == SymbolRef::ST_Debug || Type == SymbolRef::ST_File)
    Debug = 'd';

  char FileFunc = ' ';
  if (Type == SymbolRef::ST_File)
    FileFunc = 'f';
  else if (Type == SymbolRef::ST_Function)
    FileFunc = 'F';
  else if (Type == SymbolRef::ST_Data)
    FileFunc = 'O';

  const char *Fmt = O->getBytesInAddress() > 4 ? "%016" PRIx64 : "%08" PRIx64;

  outs() << format(Fmt, Address) << " "
         << GlobLoc            // Local -> 'l', Global -> 'g', Neither -> ' '
         << (Weak ? 'w' : ' ') // Weak?
         << ' '                // Constructor. Not supported yet.
         << ' '                // Warning. Not supported yet.
         << IFunc              // Indirect reference to another symbol.
         << Debug              // Debugging (d) or dynamic (D) symbol.
         << FileFunc           // Name of function (F), file (f) or object (O).
         << ' ';
  if (Absolute) {
    outs() << "*ABS*";
  } else if (Common) {
    outs() << "*COM*";
  } else if (Section == O->section_end()) {
    outs() << "*UND*";
  } else {
    if (MachO) {
      DataRefImpl DR = Section->getRawDataRefImpl();
      StringRef SegmentName = MachO->getSectionFinalSegmentName(DR);
      outs() << SegmentName << ",";
    }
    StringRef SectionName = unwrapOrError(Section->getName(), FileName);
    outs() << SectionName;
  }

  if (Common || O->isELF()) {
    uint64_t Val =
        Common ? Symbol.getAlignment() : ELFSymbolRef(Symbol).getSize();
    outs() << '\t' << format(Fmt, Val);
  }

  if (O->isELF()) {
    uint8_t Other = ELFSymbolRef(Symbol).getOther();
    switch (Other) {
    case ELF::STV_DEFAULT:
      break;
    case ELF::STV_INTERNAL:
      outs() << " .internal";
      break;
    case ELF::STV_HIDDEN:
      outs() << " .hidden";
      break;
    case ELF::STV_PROTECTED:
      outs() << " .protected";
      break;
    default:
      outs() << format(" 0x%02x", Other);
      break;
    }
  } else if (Hidden) {
    outs() << " .hidden";
  }

  if (Demangle)
    outs() << ' ' << demangle(std::string(Name)) << '\n';
  else
    outs() << ' ' << Name << '\n';
}

static void printUnwindInfo(const ObjectFile *O) {
  outs() << "Unwind info:\n\n";

  if (const COFFObjectFile *Coff = dyn_cast<COFFObjectFile>(O))
    printCOFFUnwindInfo(Coff);
  else if (const MachOObjectFile *MachO = dyn_cast<MachOObjectFile>(O))
    printMachOUnwindInfo(MachO);
  else
    // TODO: Extract DWARF dump tool to objdump.
    WithColor::error(errs(), ToolName)
        << "This operation is only currently supported "
           "for COFF and MachO object files.\n";
}

/// Dump the raw contents of the __clangast section so the output can be piped
/// into llvm-bcanalyzer.
static void printRawClangAST(const ObjectFile *Obj) {
  if (outs().is_displayed()) {
    WithColor::error(errs(), ToolName)
        << "The -raw-clang-ast option will dump the raw binary contents of "
           "the clang ast section.\n"
           "Please redirect the output to a file or another program such as "
           "llvm-bcanalyzer.\n";
    return;
  }

  StringRef ClangASTSectionName("__clangast");
  if (Obj->isCOFF()) {
    ClangASTSectionName = "clangast";
  }

  Optional<object::SectionRef> ClangASTSection;
  for (auto Sec : ToolSectionFilter(*Obj)) {
    StringRef Name;
    if (Expected<StringRef> NameOrErr = Sec.getName())
      Name = *NameOrErr;
    else
      consumeError(NameOrErr.takeError());

    if (Name == ClangASTSectionName) {
      ClangASTSection = Sec;
      break;
    }
  }
  if (!ClangASTSection)
    return;

  StringRef ClangASTContents = unwrapOrError(
      ClangASTSection.getValue().getContents(), Obj->getFileName());
  outs().write(ClangASTContents.data(), ClangASTContents.size());
}

static void printFaultMaps(const ObjectFile *Obj) {
  StringRef FaultMapSectionName;

  if (Obj->isELF()) {
    FaultMapSectionName = ".llvm_faultmaps";
  } else if (Obj->isMachO()) {
    FaultMapSectionName = "__llvm_faultmaps";
  } else {
    WithColor::error(errs(), ToolName)
        << "This operation is only currently supported "
           "for ELF and Mach-O executable files.\n";
    return;
  }

  Optional<object::SectionRef> FaultMapSection;

  for (auto Sec : ToolSectionFilter(*Obj)) {
    StringRef Name;
    if (Expected<StringRef> NameOrErr = Sec.getName())
      Name = *NameOrErr;
    else
      consumeError(NameOrErr.takeError());

    if (Name == FaultMapSectionName) {
      FaultMapSection = Sec;
      break;
    }
  }

  outs() << "FaultMap table:\n";

  if (!FaultMapSection.hasValue()) {
    outs() << "<not found>\n";
    return;
  }

  StringRef FaultMapContents =
      unwrapOrError(FaultMapSection.getValue().getContents(), Obj->getFileName());
  FaultMapParser FMP(FaultMapContents.bytes_begin(),
                     FaultMapContents.bytes_end());

  outs() << FMP;
}

static void printPrivateFileHeaders(const ObjectFile *O, bool OnlyFirst) {
  if (O->isELF()) {
    printELFFileHeader(O);
    printELFDynamicSection(O);
    printELFSymbolVersionInfo(O);
    return;
  }
  if (O->isCOFF())
    return printCOFFFileHeader(O);
  if (O->isWasm())
    return printWasmFileHeader(O);
  if (O->isMachO()) {
    printMachOFileHeader(O);
    if (!OnlyFirst)
      printMachOLoadCommands(O);
    return;
  }
  reportError(O->getFileName(), "Invalid/Unsupported object file format");
}

static void printFileHeaders(const ObjectFile *O) {
  if (!O->isELF() && !O->isCOFF())
    reportError(O->getFileName(), "Invalid/Unsupported object file format");

  Triple::ArchType AT = O->getArch();
  outs() << "architecture: " << Triple::getArchTypeName(AT) << "\n";
  uint64_t Address = unwrapOrError(O->getStartAddress(), O->getFileName());

  StringRef Fmt = O->getBytesInAddress() > 4 ? "%016" PRIx64 : "%08" PRIx64;
  outs() << "start address: "
         << "0x" << format(Fmt.data(), Address) << "\n\n";
}

static void printArchiveChild(StringRef Filename, const Archive::Child &C) {
  Expected<sys::fs::perms> ModeOrErr = C.getAccessMode();
  if (!ModeOrErr) {
    WithColor::error(errs(), ToolName) << "ill-formed archive entry.\n";
    consumeError(ModeOrErr.takeError());
    return;
  }
  sys::fs::perms Mode = ModeOrErr.get();
  outs() << ((Mode & sys::fs::owner_read) ? "r" : "-");
  outs() << ((Mode & sys::fs::owner_write) ? "w" : "-");
  outs() << ((Mode & sys::fs::owner_exe) ? "x" : "-");
  outs() << ((Mode & sys::fs::group_read) ? "r" : "-");
  outs() << ((Mode & sys::fs::group_write) ? "w" : "-");
  outs() << ((Mode & sys::fs::group_exe) ? "x" : "-");
  outs() << ((Mode & sys::fs::others_read) ? "r" : "-");
  outs() << ((Mode & sys::fs::others_write) ? "w" : "-");
  outs() << ((Mode & sys::fs::others_exe) ? "x" : "-");

  outs() << " "; //degistirdim

  outs() << format("%d/%d %6" PRId64 " ", unwrapOrError(C.getUID(), Filename),
                   unwrapOrError(C.getGID(), Filename),
                   unwrapOrError(C.getRawSize(), Filename));

  StringRef RawLastModified = C.getRawLastModified();
  unsigned Seconds;
  if (RawLastModified.getAsInteger(10, Seconds))
    outs() << "(date: \"" << RawLastModified
           << "\" contains non-decimal chars) ";
  else {
    // Since ctime(3) returns a 26 character string of the form:
    // "Sun Sep 16 01:03:52 1973\n\0"
    // just print 24 characters.
    time_t t = Seconds;
    outs() << format("%.24s ", ctime(&t));
  }

  StringRef Name = "";
  Expected<StringRef> NameOrErr = C.getName();
  if (!NameOrErr) {
    consumeError(NameOrErr.takeError());
    Name = unwrapOrError(C.getRawName(), Filename);
  } else {
    Name = NameOrErr.get();
  }
  outs() << Name << "\n";
}

// For ELF only now.
static bool shouldWarnForInvalidStartStopAddress(ObjectFile *Obj) {
  if (const auto *Elf = dyn_cast<ELFObjectFileBase>(Obj)) {
    if (Elf->getEType() != ELF::ET_REL)
      return true;
  }
  return false;
}

static void checkForInvalidStartStopAddress(ObjectFile *Obj,
                                            uint64_t Start, uint64_t Stop) {
  if (!shouldWarnForInvalidStartStopAddress(Obj))
    return;

  for (const SectionRef &Section : Obj->sections())
    if (ELFSectionRef(Section).getFlags() & ELF::SHF_ALLOC) {
      uint64_t BaseAddr = Section.getAddress();
      uint64_t Size = Section.getSize();
      if ((Start < BaseAddr + Size) && Stop > BaseAddr)
        return;
    }

  if (StartAddress.getNumOccurrences() == 0)
    reportWarning("no section has address less than 0x" +
                      Twine::utohexstr(Stop) + " specified by --stop-address",
                  Obj->getFileName());
  else if (StopAddress.getNumOccurrences() == 0)
    reportWarning("no section has address greater than or equal to 0x" +
                      Twine::utohexstr(Start) + " specified by --start-address",
                  Obj->getFileName());
  else
    reportWarning("no section overlaps the range [0x" +
                      Twine::utohexstr(Start) + ",0x" + Twine::utohexstr(Stop) +
                      ") specified by --start-address/--stop-address",
                  Obj->getFileName());
}

static void dumpObject(ObjectFile *O, const Archive *A = nullptr,
                       const Archive::Child *C = nullptr) {
  // Avoid other output when using a raw option.
  /*if (!RawClangAST) {
    outs() << '\n';
    if (A)
      outs() << A->getFileName() << "(" << O->getFileName() << ")";
    else
      outs() << O->getFileName();
    outs() << ":\tfile format " << O->getFileFormatName().lower() << "\n\n";
  }*/

  if (StartAddress.getNumOccurrences() || StopAddress.getNumOccurrences())
    checkForInvalidStartStopAddress(O, StartAddress, StopAddress);

  // Note: the order here matches GNU objdump for compatability.
  StringRef ArchiveName = A ? A->getFileName() : "";
  if (ArchiveHeaders && !MachOOpt && C)
    printArchiveChild(ArchiveName, *C);
  if (FileHeaders)
    printFileHeaders(O);
  if (PrivateHeaders || FirstPrivateHeader)
    printPrivateFileHeaders(O, FirstPrivateHeader);
  if (SectionHeaders)
    printSectionHeaders(O);
  if (SymbolTable)
    printSymbolTable(O, ArchiveName);
  if (DynamicSymbolTable)
    printSymbolTable(O, ArchiveName, /*ArchitectureName=*/"",
                     /*DumpDynamic=*/true);
  if (DwarfDumpType != DIDT_Null) {
    std::unique_ptr<DIContext> DICtx = DWARFContext::create(*O);
    // Dump the complete DWARF structure.
    DIDumpOptions DumpOpts;
    DumpOpts.DumpType = DwarfDumpType;
    DICtx->dump(outs(), DumpOpts);
  }
  if (Relocations && !Disassemble)
    printRelocations(O);
  if (DynamicRelocations)
    printDynamicRelocations(O);
  if (SectionContents)
    printSectionContents(O);
  if (Disassemble)
    disassembleObject(O, Relocations);
  if (UnwindInfo)
    printUnwindInfo(O);

  // Mach-O specific options:
  if (ExportsTrie)
    printExportsTrie(O);
  if (Rebase)
    printRebaseTable(O);
  if (Bind)
    printBindTable(O);
  if (LazyBind)
    printLazyBindTable(O);
  if (WeakBind)
    printWeakBindTable(O);

  // Other special sections:
  if (RawClangAST)
    printRawClangAST(O);
  if (FaultMapSection)
    printFaultMaps(O);
}

static void dumpObject(const COFFImportFile *I, const Archive *A,
                       const Archive::Child *C = nullptr) {
  StringRef ArchiveName = A ? A->getFileName() : "";

  // Avoid other output when using a raw option.
  /*if (!RawClangAST)
    outs() << '\n'
           << ArchiveName << "(" << I->getFileName() << ")"
           << ":\tfile format COFF-import-file"
           << "\n\n";*/

  if (ArchiveHeaders && !MachOOpt && C)
    printArchiveChild(ArchiveName, *C);
  if (SymbolTable)
    printCOFFSymbolTable(I);
}

/// Dump each object file in \a a;
static void dumpArchive(const Archive *A) {
  Error Err = Error::success();
  unsigned I = -1;
  for (auto &C : A->children(Err)) {
    ++I;
    Expected<std::unique_ptr<Binary>> ChildOrErr = C.getAsBinary();
    if (!ChildOrErr) {
      if (auto E = isNotObjectErrorInvalidFileType(ChildOrErr.takeError()))
        reportError(std::move(E), getFileNameForError(C, I), A->getFileName());
      continue;
    }
    if (ObjectFile *O = dyn_cast<ObjectFile>(&*ChildOrErr.get()))
      dumpObject(O, A, &C);
    else if (COFFImportFile *I = dyn_cast<COFFImportFile>(&*ChildOrErr.get()))
      dumpObject(I, A, &C);
    else
      reportError(errorCodeToError(object_error::invalid_file_type),
                  A->getFileName());
  }
  if (Err)
    reportError(std::move(Err), A->getFileName());
}

/// Open file and figure out how to dump it.
static void dumpInput(StringRef file) {
  // If we are using the Mach-O specific object file parser, then let it parse
  // the file and process the command line options.  So the -arch flags can
  // be used to select specific slices, etc.
  if (MachOOpt) {
    parseInputMachO(file);
    return;
  }

  // Attempt to open the binary.
  OwningBinary<Binary> OBinary = unwrapOrError(createBinary(file), file);
  Binary &Binary = *OBinary.getBinary();

  if (Archive *A = dyn_cast<Archive>(&Binary))
    dumpArchive(A);
  else if (ObjectFile *O = dyn_cast<ObjectFile>(&Binary))
    dumpObject(O);
  else if (MachOUniversalBinary *UB = dyn_cast<MachOUniversalBinary>(&Binary))
    parseInputMachO(UB);
  else
    reportError(errorCodeToError(object_error::invalid_file_type), file);
}
/*
void getdosyadan(){

  if(!dosya.empty()){
    ifstream dos(dosya, fstream::in | fstream::out | fstream::trunc);
    string option = "";
    char character;
    if (dos.is_open()){
      while(dos >> std::noskipws >> character){
        
        if(character != ' ' && character != '\n')
          option += string(1,character);
        else {
          if( option == "add"                ){ add = true; option = ""; }
          if( option == "sub"                ){ sub = true; option = ""; }
        }
      }
    }
  }

}
*/



int main(int argc, char **argv) {

g_argc = argc;
g_argv = argv;



  using namespace llvm;
  InitLLVM X(argc, argv);
  const cl::OptionCategory *OptionFilters[] = {&ObjdumpCat, &MachOCat};
  cl::HideUnrelatedOptions(OptionFilters);

  SmallVector<const char *, 256> argv2(argv, argv + argc);

  std::string fileName = "";
  for(int i=0; i<argv2.size(); i++){
    if(StringRef(argv[i]).endswith(".o"))
      fileName = std::string(StringRef(argv[i]).rtrim(".o"));
  }

  OSS.open(fileName + "out.hex", fstream::in | fstream::out | fstream::trunc); //std::fstream::app | std::fstream::out);//fstream::in | fstream::out | fstream::trunc);
  //OSS << 12345678 << "\n";
//ekleme
//  if(!dosya.empty()){
//    ifstream dos(dosya);
//    string option = "";
//    char character;
//    if (dos.is_open()){
//      while(dos >> character){
//        if(character='a') add = true;
//        if(character != ' ' || character != '\n')
//          option += string(1,character);
//        else {
//          if( option == "add"                ){ add = true; option = ""; } //else sub=true;
//          if( option == "sub"                ){ sub = true; option = ""; }
//        }
//      }
//    }
//  }
//getdosyadan();
//add = true;
  // Initialize targets and assembly printers/parsers.
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllDisassemblers();

  // Register the target printer for --version.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  cl::ParseCommandLineOptions(argc, argv, "llvm object file dumper\n", nullptr,
                              /*EnvVar=*/nullptr,
                              /*LongOptionsUseDoubleDash=*/true);

  if (StartAddress >= StopAddress)
    reportCmdLineError("start address should be less than stop address");

  ToolName = argv[0];

int bitArr[90];
for(int i=89;i>=0;i--){
  bitArr[i] = 0;
}

for(int i=0;i<bits.length();i++){
  bitArr[i] = bits[i] - '0';
}

// hepsini baslangicta falsea esitlemeyi dene ya da else kullan.
/*
if(bitArr[0 ] == 1) beq                = true;
if(bitArr[1 ] == 1) bne                = true;
if(bitArr[2 ] == 1) blt                = true;
if(bitArr[3 ] == 1) bge                = true;
if(bitArr[4 ] == 1) bltu               = true;
if(bitArr[5 ] == 1) bgeu               = true;
if(bitArr[6 ] == 1) jalr               = true;
if(bitArr[7 ] == 1) jal                = true;
if(bitArr[8 ] == 1) lui                = true;
if(bitArr[9 ] == 1) auipc              = true;
if(bitArr[10] == 1) addi               = true;
if(bitArr[11] == 1) slli               = true;
if(bitArr[12] == 1) slti               = true;
if(bitArr[13] == 1) sltiu              = true;
if(bitArr[14] == 1) xori               = true;
if(bitArr[15] == 1) srli               = true;
if(bitArr[16] == 1) srai               = true;
if(bitArr[17] == 1) ori                = true;
if(bitArr[18] == 1) andi               = true;
if(bitArr[19] == 1) add                = true;
if(bitArr[20] == 1) sub                = true;
if(bitArr[21] == 1) sll                = true;
if(bitArr[22] == 1) slt                = true;
if(bitArr[23] == 1) sltu               = true;
if(bitArr[24] == 1) xor_               = true; 
if(bitArr[25] == 1) srl                = true;
if(bitArr[26] == 1) sra                = true;
if(bitArr[27] == 1) or_                = true; 
if(bitArr[28] == 1) and_               = true; 
if(bitArr[29] == 1) addiw              = true;
if(bitArr[30] == 1) slliw              = true;
if(bitArr[31] == 1) srliw              = true;
if(bitArr[32] == 1) sraiw              = true;
if(bitArr[33] == 1) addw               = true;
if(bitArr[34] == 1) subw               = true;
if(bitArr[35] == 1) sllw               = true;
if(bitArr[36] == 1) srlw               = true;
if(bitArr[37] == 1) sraw               = true;
if(bitArr[38] == 1) lb                 = true;
if(bitArr[39] == 1) lh                 = true;
if(bitArr[40] == 1) lw                 = true;
if(bitArr[41] == 1) ld                 = true;
if(bitArr[42] == 1) lbu                = true;
if(bitArr[43] == 1) lhu                = true;
if(bitArr[44] == 1) lwu                = true;
if(bitArr[45] == 1) sb                 = true;
if(bitArr[46] == 1) sh                 = true;
if(bitArr[47] == 1) sw                 = true;
if(bitArr[48] == 1) sd                 = true;
if(bitArr[49] == 1) fence              = true;
if(bitArr[50] == 1) fence_i            = true;
if(bitArr[51] == 1) mul                = true;
if(bitArr[52] == 1) mulh               = true;
if(bitArr[53] == 1) mulhsu             = true;
if(bitArr[54] == 1) mulhu              = true;
if(bitArr[55] == 1) objdump::div       = true; // div is ambigious hatasindan dolayi
if(bitArr[56] == 1) divu               = true;
if(bitArr[57] == 1) rem                = true;
if(bitArr[58] == 1) remu               = true;
if(bitArr[59] == 1) mulw               = true;
if(bitArr[60] == 1) divw               = true;
if(bitArr[61] == 1) divuw              = true;
if(bitArr[62] == 1) remw               = true;
if(bitArr[63] == 1) remuw              = true;
if(bitArr[64] == 1) lr_w               = true;
if(bitArr[65] == 1) sc_w               = true;
if(bitArr[66] == 1) lr_d               = true;
if(bitArr[67] == 1) sc_d               = true;
if(bitArr[68] == 1) ecall              = true;
if(bitArr[69] == 1) ebreak             = true;
if(bitArr[70] == 1) uret               = true;
if(bitArr[71] == 1) mret               = true;
if(bitArr[72] == 1) dret               = true;
if(bitArr[73] == 1) sfence_vma         = true;
if(bitArr[74] == 1) wfi                = true;
if(bitArr[75] == 1) csrrw              = true;
if(bitArr[76] == 1) csrrs              = true;
if(bitArr[77] == 1) csrrc              = true;
if(bitArr[78] == 1) csrrwi             = true;
if(bitArr[79] == 1) csrrsi             = true;
if(bitArr[80] == 1) csrrci             = true;
if(bitArr[81] == 1) slli_rv32          = true;
if(bitArr[82] == 1) srli_rv32          = true;
if(bitArr[83] == 1) srai_rv32          = true;
if(bitArr[84] == 1) rdcycle            = true;
if(bitArr[85] == 1) rdtime             = true;
if(bitArr[86] == 1) rdinstret          = true;
if(bitArr[87] == 1) rdcycleh           = true;
if(bitArr[88] == 1) rdtimeh            = true;
if(bitArr[89] == 1) rdinstreth         = true;
*/

  // Defaults to a.out if no filenames specified.
  if (InputFilenames.empty())
    InputFilenames.push_back("a.out");

  if (AllHeaders)
    ArchiveHeaders = FileHeaders = PrivateHeaders = Relocations =
        SectionHeaders = SymbolTable = true;

  if (DisassembleAll || PrintSource || PrintLines ||
      !DisassembleSymbols.empty())
    Disassemble = true;

  if (!ArchiveHeaders && !Disassemble && DwarfDumpType == DIDT_Null &&
      !DynamicRelocations && !FileHeaders && !PrivateHeaders && !RawClangAST &&
      !Relocations && !SectionHeaders && !SectionContents && !SymbolTable &&
      !DynamicSymbolTable && !UnwindInfo && !FaultMapSection &&
      !(MachOOpt &&
        (Bind || DataInCode || DylibId || DylibsUsed || ExportsTrie ||
         FirstPrivateHeader || IndirectSymbols || InfoPlist || LazyBind ||
         LinkOptHints || ObjcMetaData || Rebase || UniversalHeaders ||
         WeakBind || !FilterSections.empty()))) {
    cl::PrintHelpMessage();
    return 2;
  }

  DisasmSymbolSet.insert(DisassembleSymbols.begin(), DisassembleSymbols.end());

  llvm::for_each(InputFilenames, dumpInput);

  warnOnNoMatchForSections();
OSS<<""; // mainde boş da olsa bişey yazmazsam yapmadı.
  //OSS<<"afasdasd"; OSS<<" ";
//OSS.flush();
OSS.close();

  return EXIT_SUCCESS;
}
// Inputfilenames e vector olarak davran ismi almaya calis
// digerlerini kaptmayip onlari da dosya yazdirmasına dahil etmek gerek

// printrelocations da %08 ve %016 7 ve 15 boşluk basıyor hexten önce
