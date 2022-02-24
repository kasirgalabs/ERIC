//===---- elf2encryptedhex.cpp - elf2encryptedhex Hex Code Obfuscator -----===//
//===---------------------- Ported From LLVM Tools ------------------------===//
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

#include "elf2encryptedhex.h"

#include "MCTargetDesc/RISCVInstPrinter.h"

#include "ELFDump.h"

#include "llvm/ADT/IndexedMap.h"
#include "llvm/ADT/Optional.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/ADT/STLExtras.h"

#include "llvm/ADT/StringExtras.h"

#include "llvm/ADT/Triple.h"

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

#include "llvm/Object/Archive.h"

#include "llvm/Object/ELFObjectFile.h"

#include "llvm/Support/Casting.h"

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
#include <sstream>
#include <iomanip>
#include "picosha2.h"
#include "RSA32.h"
#include "custom-encryptor.h"

using namespace llvm;
using namespace llvm::object;
using namespace llvm::objdump;

// RSA keys are set internally for 32 bit rsa encryption
// Here, keys should be set big enough to encrypt 32 bit data correctly (each instruction has 32 or 16 bit)
const unsigned long long int pkey = 65537;
const unsigned long long int qkey = 65543;
const unsigned long long int ekey = 65537; //459226363;

static cl::OptionCategory ObjdumpCat("llvm-objdump Options");

cl::opt<bool> objdump::rsa("rsa",
    cl::desc("encrypt all instructions with rsa, usage: --rsa"),
    cl::cat(ObjdumpCat));

// encrypt with the given key option for all over hex

cl::opt<std::string> objdump::enckeyall("enckeyall",
    cl::desc("32 bit key option to encrypt all instructions, usage: --enckeyall=\"<your32bitkeyasbinary>(for 16 bit compressed instructions it uses most significant(left) 16 bit of this as key)\""),
    cl::cat(ObjdumpCat));

static cl::opt<std::string> instnum("instnum",
    cl::desc("Number of instructions to encrypt from beginning of the program"),
    cl::cat(ObjdumpCat));

// key options for 32 bit instructions(instruction length 32 bit) dependent on extension

cl::opt<std::string> objdump::ienc32key("ienc32key",
    cl::desc("32 bit key option to encrypt rv32i instructions, usage: --ienc32key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::menc32key("menc32key",
    cl::desc("32 bit key option to encrypt rv32m instructions, usage: --menc32key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::aenc32key("aenc32key",
    cl::desc("32 bit key option to encrypt rv32a instructions, usage: --aenc32key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::fenc32key("fenc32key",
    cl::desc("32 bit key option to encrypt rv32f instructions, usage: --fenc32key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::denc32key("denc32key",
    cl::desc("32 bit key option to encrypt rv32d instructions, usage: --denc32key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::qenc32key("qenc32key",
    cl::desc("32 bit key option to encrypt rv32q instructions, usage: --qenc32key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// key options for 64 bit instructions(instruction length 32 bit) dependent on extension

cl::opt<std::string> objdump::ienc64key("ienc64key",
    cl::desc("32 bit key option to encrypt rv64i instructions, usage: --ienc64key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::menc64key("menc64key",
    cl::desc("32 bit key option to encrypt rv64m instructions, usage: --menc64key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::aenc64key("aenc64key",
    cl::desc("32 bit key option to encrypt rv64a instructions, usage: --aenc64key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::fenc64key("fenc64key",
    cl::desc("32 bit key option to encrypt rv64f instructions, usage: --fenc64key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::denc64key("denc64key",
    cl::desc("32 bit key option to encrypt rv64d instructions, usage: --denc64key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::qenc64key("qenc64key",
    cl::desc("32 bit key option to encrypt rv64q instructions, usage: --qenc64key=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// key options for compressed instructions(instruction length 16 bit) dependent on quadrant

cl::opt<std::string> objdump::cencq0key("cencq0key",
    cl::desc("16 bit key option to encrypt rvc quadrant 0 instructions, usage: --cencq0key=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::cencq1key("cencq1key",
    cl::desc("16 bit key option to encrypt rvc quadrant 1 instructions, usage: --cencq1key=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::cencq2key("cencq2key",
    cl::desc("16 bit key option to encrypt rvc quadrant 2 instructions, usage: --cencq2key=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// instruction list options for 32 bit instructions dependent on extension

cl::opt<std::string> objdump::ienc32insts("ienc32insts",
    cl::desc("47 bit rv32i instruction list to encrypt with ienc32key, usage: --ienc32insts=\"<your47bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::menc32insts("menc32insts",
    cl::desc("8 bit rv32m instruction list to encrypt with menc32key, usage: --menc32insts=\"<your8bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::aenc32insts("aenc32insts",
    cl::desc("11 bit rv32a instruction list to encrypt with aenc32key, usage: --aenc32insts=\"<your11bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::fenc32insts("fenc32insts",
    cl::desc("26 bit rv32f instruction list to encrypt with fenc32key, usage: --fenc32insts=\"<your26bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::denc32insts("denc32insts",
    cl::desc("26 bit rv32d instruction list to encrypt with denc32key, usage: --denc32insts=\"<your26bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::qenc32insts("qenc32insts",
    cl::desc("28 bit rv32q instruction list to encrypt with qenc32key, usage: --qenc32insts=\"<your28bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));


// instruction list options for 64 bit instructions dependent on extension

cl::opt<std::string> objdump::ienc64insts("ienc64insts",
    cl::desc("15 bit rv64i instruction list to encrypt with ienc64key, usage: --ienc64insts=\"<your15bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::menc64insts("menc64insts",
    cl::desc("5 bit rv64m instruction list to encrypt with menc64key, usage: --menc64insts=\"<your5bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::aenc64insts("aenc64insts",
    cl::desc("11 bit rv64a instruction list to encrypt with aenc64key, usage: --aenc64insts=\"<your11bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::fenc64insts("fenc64insts",
    cl::desc("4 bit rv64f instruction list to encrypt with fenc64key, usage: --fenc64insts=\"<your4bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::denc64insts("denc64insts",
    cl::desc("6 bit rv64d instruction list to encrypt with denc64key, usage: --denc64insts=\"<your6bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::qenc64insts("qenc64insts",
    cl::desc("4 bit rv64q instruction list to encrypt with qenc64key, usage: --qenc64insts=\"<your4bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));


// instruction list options for compressed instructions dependent on quadrant

cl::opt<std::string> objdump::cencq0insts("cencq0insts",
    cl::desc("11 bit rvc quadrant 0 instruction list to encrypt with cencq0key, usage: --cencq0insts=\"<your11bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::cencq1insts("cencq1insts",
    cl::desc("21 bit rvc quadrant 1 instruction list to encrypt with cencq1key, usage: --cencq1insts=\"<your21bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::cencq2insts("cencq2insts",
    cl::desc("17 bit rvc quadrant 2 instruction list to encrypt with cencq2key, usage: --cencq2insts=\"<your17bitinstlistasbinary(use 1 to encrypt corresponding instruction)>\""),
    cl::cat(ObjdumpCat));


// rv32i instruction options to encrypt with ienc32key

cl::opt<bool> objdump::lui("lui",
    cl::desc("lui instruction option to encrypt with ienc32key, usage: --lui"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::auipc("auipc",
    cl::desc("auipc instruction option to encrypt with ienc32key, usage: --auipc"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::jal("jal",
    cl::desc("jal instruction option to encrypt with ienc32key, usage: --jal"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::jalr("jalr",
    cl::desc("jalr instruction option to encrypt with ienc32key, usage: --jalr"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::beq("beq",
    cl::desc("beq instruction option to encrypt with ienc32key, usage: --beq"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bne("bne",
    cl::desc("bne instruction option to encrypt with ienc32key, usage: --bne"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::blt("blt",
    cl::desc("blt instruction option to encrypt with ienc32key, usage: --blt"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bge("bge",
    cl::desc("bge instruction option to encrypt with ienc32key, usage: --bge"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bltu("bltu",
    cl::desc("bltu instruction option to encrypt with ienc32key, usage: --bltu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::bgeu("bgeu",
    cl::desc("bgeu instruction option to encrypt with ienc32key, usage: --bgeu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lb("lb",
    cl::desc("lb instruction option to encrypt with ienc32key, usage: --lb"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lh("lh",
    cl::desc("lh instruction option to encrypt with ienc32key, usage: --lh"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lw("lw",
    cl::desc("lw instruction option to encrypt with ienc32key, usage: --lw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lbu("lbu",
    cl::desc("lbu instruction option to encrypt with ienc32key, usage: --lbu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::lhu("lhu",
    cl::desc("lhu instruction option to encrypt with ienc32key, usage: --lhu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sb("sb",
    cl::desc("sb instruction option to encrypt with ienc32key, usage: --sb"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sh("sh",
    cl::desc("sh instruction option to encrypt with ienc32key, usage: --sh"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sw("sw",
    cl::desc("sw instruction option to encrypt with ienc32key, usage: --sw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::addi("addi",
    cl::desc("addi instruction option to encrypt with ienc32key, usage: --addi"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slti("slti",
    cl::desc("slti instruction option to encrypt with ienc32key, usage: --slti"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sltiu("sltiu",
    cl::desc("sltiu instruction option to encrypt with ienc32key, usage: --sltiu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::xori("xori",
    cl::desc("xori instruction option to encrypt with ienc32key, usage: --xori"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ori("ori",
    cl::desc("ori instruction option to encrypt with ienc32key, usage: --ori"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::andi("andi",
    cl::desc("andi instruction option to encrypt with ienc32key, usage: --andi"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slli("slli",
    cl::desc("slli instruction option to encrypt with ienc32key, usage: --slli"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srli("srli",
    cl::desc("srli instruction option to encrypt with ienc32key, usage: --srli"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srai("srai",
    cl::desc("srai instruction option to encrypt with ienc32key, usage: --srai"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::add("add",
    cl::desc("add instruction option to encrypt with ienc32key, usage: --add"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sub("sub",
    cl::desc("sub instruction option to encrypt with ienc32key, usage: --sub"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sll("sll",
    cl::desc("sll instruction option to encrypt with ienc32key, usage: --sll"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slt("slt",
    cl::desc("slt instruction option to encrypt with ienc32key, usage: --slt"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sltu("sltu",
    cl::desc("sltu instruction option to encrypt with ienc32key, usage: --sltu"),
    cl::cat(ObjdumpCat));

// added underscore to avoid confusion with 'xor' operation in c++
cl::opt<bool> objdump::xor_("xor_",
    cl::desc("xor instruction option to encrypt with ienc32key, usage: --xor_"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srl("srl",
    cl::desc("srl instruction option to encrypt with ienc32key, usage: --srl"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sra("sra",
    cl::desc("sra instruction option to encrypt with ienc32key, usage: --sra"),
    cl::cat(ObjdumpCat));

// added underscore to avoid confusion with 'or' operation in c++
cl::opt<bool> objdump::or_("or_",
    cl::desc("or instruction option to encrypt with ienc32key, usage: --or_"),
    cl::cat(ObjdumpCat));

// added underscore to avoid confusion with 'and' operation in c++
cl::opt<bool> objdump::and_("and_",
    cl::desc("and instruction option to encrypt with ienc32key, usage: --and_"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fence("fence",
    cl::desc("fence instruction option to encrypt with ienc32key, usage: --fence"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fence_i("fence_i",
    cl::desc("fence_i instruction option to encrypt with ienc32key, usage: --fence_i"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ecall("ecall",
    cl::desc("ecall instruction option to encrypt with ienc32key, usage: --ecall"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ebreak("ebreak",
    cl::desc("ebreak instruction option to encrypt with ienc32key, usage: --ebreak"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrw("csrrw",
    cl::desc("csrrw instruction option to encrypt with ienc32key, usage: --csrrw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrs("csrrs",
    cl::desc("csrrs instruction option to encrypt with ienc32key, usage: --csrrs"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrc("csrrc",
    cl::desc("csrrc instruction option to encrypt with ienc32key, usage: --csrrc"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrwi("csrrwi",
    cl::desc("csrrwi instruction option to encrypt with ienc32key, usage: --csrrwi"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrsi("csrrsi",
    cl::desc("csrrsi instruction option to encrypt with ienc32key, usage: --csrrsi"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::csrrci("csrrci",
    cl::desc("csrrci instruction option to encrypt with ienc32key, usage: --csrrci"),
    cl::cat(ObjdumpCat));


// rv32m instruction options to encrypt with menc32key

cl::opt<bool> objdump::mul("mul",
    cl::desc("mul instruction option to encrypt with menc32key, usage: --mul"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulh("mulh",
    cl::desc("mulh instruction option to encrypt with menc32key, usage: --mulh"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulhsu("mulhsu",
    cl::desc("mulhsu instruction option to encrypt with menc32key, usage: --mulhsu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::mulhu("mulhu",
    cl::desc("mulhu instruction option to encrypt with menc32key, usage: --mulhu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::div_("div_",
    cl::desc("div instruction option to encrypt with menc32key, usage: --div_"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::divu("divu",
    cl::desc("divu instruction option to encrypt with menc32key, usage: --divu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::rem("rem",
    cl::desc("rem instruction option to encrypt with menc32key, usage: --rem"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::remu("remu",
    cl::desc("remu instruction option to encrypt with menc32key, usage: --remu"),
    cl::cat(ObjdumpCat));


// rv32a instruction options to encrypt with aenc32key

cl::opt<bool> objdump::lr_w("lr_w",
    cl::desc("lr_w instruction option to encrypt with aenc32key, usage: --lr_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sc_w("sc_w",
    cl::desc("sc_w instruction option to encrypt with aenc32key, usage: --sc_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoswap_w("amoswap_w",
    cl::desc("amoswap_w instruction option to encrypt with aenc32key, usage: --amoswap_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoadd_w("amoadd_w",
    cl::desc("amoadd_w instruction option to encrypt with aenc32key, usage: --amoadd_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoxor_w("amoxor_w",
    cl::desc("amoxor_w instruction option to encrypt with aenc32key, usage: --amoxor_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoand_w("amoand_w",
    cl::desc("amoand_w instruction option to encrypt with aenc32key, usage: --amoand_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoor_w("amoor_w",
    cl::desc("amoor_w instruction option to encrypt with aenc32key, usage: --amoor_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amomin_w("amomin_w",
    cl::desc("amomin_w instruction option to encrypt with aenc32key, usage: --amomin_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amomax_w("amomax_w",
    cl::desc("amomax_w instruction option to encrypt with aenc32key, usage: --amomax_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amominu_w("amominu_w",
    cl::desc("amominu_w instruction option to encrypt with aenc32key, usage: --amominu_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amomaxu_w("amomaxu_w",
    cl::desc("amomaxu_w instruction option to encrypt with aenc32key, usage: --amomaxu_w"),
    cl::cat(ObjdumpCat));


// rv32f instruction options to encrypt with fenc32key

cl::opt<bool> objdump::flw("flw",
    cl::desc("flw instruction option to encrypt with fenc32key, usage: --flw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsw("fsw",
    cl::desc("fsw instruction option to encrypt with fenc32key, usage: --fsw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmadd_s("fmadd_s",
    cl::desc("fmadd_s instruction option to encrypt with fenc32key, usage: --fmadd_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmsub_s("fmsub_s",
    cl::desc("fmsub_s instruction option to encrypt with fenc32key, usage: --fmsub_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fnmsub_s("fnmsub_s",
    cl::desc("fnmsub_s instruction option to encrypt with fenc32key, usage: --fnmsub_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fnmadd_s("fnmadd_s",
    cl::desc("fnmadd_s instruction option to encrypt with fenc32key, usage: --fnmadd_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fadd_s("fadd_s",
    cl::desc("fadd_s instruction option to encrypt with fenc32key, usage: --fadd_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsub_s("fsub_s",
    cl::desc("fsub_s instruction option to encrypt with fenc32key, usage: --fsub_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmul_s("fmul_s",
    cl::desc("fmul_s instruction option to encrypt with fenc32key, usage: --fmul_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fdiv_s("fdiv_s",
    cl::desc("fdiv_s instruction option to encrypt with fenc32key, usage: --fdiv_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsqrt_s("fsqrt_s",
    cl::desc("fsqrt_s instruction option to encrypt with fenc32key, usage: --fsqrt_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnj_s("fsgnj_s",
    cl::desc("fsgnj_s instruction option to encrypt with fenc32key, usage: --fsgnj_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnjn_s("fsgnjn_s",
    cl::desc("fsgnjn_s instruction option to encrypt with fenc32key, usage: --fsgnjn_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnjx_s("fsgnjx_s",
    cl::desc("fsgnjx_s instruction option to encrypt with fenc32key, usage: --fsgnjx_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmin_s("fmin_s",
    cl::desc("fmin_s instruction option to encrypt with fenc32key, usage: --fmin_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmax_s("fmax_s",
    cl::desc("fmax_s instruction option to encrypt with fenc32key, usage: --fmax_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_w_s("fcvt_w_s",
    cl::desc("fcvt_w_s instruction option to encrypt with fenc32key, usage: --fcvt_w_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_wu_s("fcvt_wu_s",
    cl::desc("fcvt_wu_s instruction option to encrypt with fenc32key, usage: --fcvt_wu_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmv_x_w("fmv_x_w",
    cl::desc("fmv_x_w instruction option to encrypt with fenc32key, usage: --fmv_x_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::feq_s("feq_s",
    cl::desc("feq_s instruction option to encrypt with fenc32key, usage: --feq_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::flt_s("flt_s",
    cl::desc("flt_s instruction option to encrypt with fenc32key, usage: --flt_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fle_s("fle_s",
    cl::desc("fle_s instruction option to encrypt with fenc32key, usage: --fle_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fclass_s("fclass_s",
    cl::desc("fclass_s instruction option to encrypt with fenc32key, usage: --fclass_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_s_w("fcvt_s_w",
    cl::desc("fcvt_s_w instruction option to encrypt with fenc32key, usage: --fcvt_s_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_s_wu("fcvt_s_wu",
    cl::desc("fcvt_s_wu instruction option to encrypt with fenc32key, usage: --fcvt_s_wu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmv_w_x("fmv_w_x",
    cl::desc("fmv_w_x instruction option to encrypt with fenc32key, usage: --fmv_w_x"),
    cl::cat(ObjdumpCat));


// rv32d instruction options to encrypt with denc32key

cl::opt<bool> objdump::fld("fld",
    cl::desc("fld instruction option to encrypt with denc32key, usage: --fld"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsd("fsd",
    cl::desc("fsd instruction option to encrypt with denc32key, usage: --fsd"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmadd_d("fmadd_d",
    cl::desc("fmadd_d instruction option to encrypt with denc32key, usage: --fmadd_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmsub_d("fmsub_d",
    cl::desc("fmsub_d instruction option to encrypt with denc32key, usage: --fmsub_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fnmsub_d("fnmsub_d",
    cl::desc("fnmsub_d instruction option to encrypt with denc32key, usage: --fnmsub_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fnmadd_d("fnmadd_d",
    cl::desc("fnmadd_d instruction option to encrypt with denc32key, usage: --fnmadd_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fadd_d("fadd_d",
    cl::desc("fadd_d instruction option to encrypt with denc32key, usage: --fadd_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsub_d("fsub_d",
    cl::desc("fsub_d instruction option to encrypt with denc32key, usage: --fsub_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmul_d("fmul_d",
    cl::desc("fmul_d instruction option to encrypt with denc32key, usage: --fmul_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fdiv_d("fdiv_d",
    cl::desc("fdiv_d instruction option to encrypt with denc32key, usage: --fdiv_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsqrt_d("fsqrt_d",
    cl::desc("fsqrt_d instruction option to encrypt with denc32key, usage: --fsqrt_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnj_d("fsgnj_d",
    cl::desc("fsgnj_d instruction option to encrypt with denc32key, usage: --fsgnj_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnjn_d("fsgnjn_d",
    cl::desc("fsgnjn_d instruction option to encrypt with denc32key, usage: --fsgnjn_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnjx_d("fsgnjx_d",
    cl::desc("fsgnjx_d instruction option to encrypt with denc32key, usage: --fsgnjx_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmin_d("fmin_d",
    cl::desc("fmin_d instruction option to encrypt with denc32key, usage: --fmin_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmax_d("fmax_d",
    cl::desc("fmax_d instruction option to encrypt with denc32key, usage: --fmax_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_s_d("fcvt_s_d",
    cl::desc("fcvt_s_d instruction option to encrypt with denc32key, usage: --fcvt_s_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_d_s("fcvt_d_s",
    cl::desc("fcvt_d_s instruction option to encrypt with denc32key, usage: --fcvt_d_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::feq_d("feq_d",
    cl::desc("feq_d instruction option to encrypt with denc32key, usage: --feq_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::flt_d("flt_d",
    cl::desc("flt_d instruction option to encrypt with denc32key, usage: --flt_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fle_d("fle_d",
    cl::desc("fle_d instruction option to encrypt with denc32key, usage: --fle_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fclass_d("fclass_d",
    cl::desc("fclass_d instruction option to encrypt with denc32key, usage: --fclass_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_w_d("fcvt_w_d",
    cl::desc("fcvt_w_d instruction option to encrypt with denc32key, usage: --fcvt_w_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_wu_d("fcvt_wu_d",
    cl::desc("fcvt_wu_d instruction option to encrypt with denc32key, usage: --fcvt_wu_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_d_w("fcvt_d_w",
    cl::desc("fcvt_d_w instruction option to encrypt with denc32key, usage: --fcvt_d_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_d_wu("fcvt_d_wu",
    cl::desc("fcvt_d_wu instruction option to encrypt with denc32key, usage: --fcvt_d_wu"),
    cl::cat(ObjdumpCat));


// rv32q instruction options to encrypt with qenc32key

cl::opt<bool> objdump::flq("flq",
    cl::desc("flq instruction option to encrypt with qenc32key, usage: --flq"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsq("fsq",
    cl::desc("fsq instruction option to encrypt with qenc32key, usage: --fsq"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmadd_q("fmadd_q",
    cl::desc("fmadd_q instruction option to encrypt with qenc32key, usage: --fmadd_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmsub_q("fmsub_q",
    cl::desc("fmsub_q instruction option to encrypt with qenc32key, usage: --fmsub_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fnmsub_q("fnmsub_q",
    cl::desc("fnmsub_q instruction option to encrypt with qenc32key, usage: --fnmsub_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fnmadd_q("fnmadd_q",
    cl::desc("fnmadd_q instruction option to encrypt with qenc32key, usage: --fnmadd_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fadd_q("fadd_q",
    cl::desc("fadd_q instruction option to encrypt with qenc32key, usage: --fadd_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsub_q("fsub_q",
    cl::desc("fsub_q instruction option to encrypt with qenc32key, usage: --fsub_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmul_q("fmul_q",
    cl::desc("fmul_q instruction option to encrypt with qenc32key, usage: --fmul_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fdiv_q("fdiv_q",
    cl::desc("fdiv_q instruction option to encrypt with qenc32key, usage: --fdiv_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsqrt_q("fsqrt_q",
    cl::desc("fsqrt_q instruction option to encrypt with qenc32key, usage: --fsqrt_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnj_q("fsgnj_q",
    cl::desc("fsgnj_q instruction option to encrypt with qenc32key, usage: --fsgnj_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnjn_q("fsgnjn_q",
    cl::desc("fsgnjn_q instruction option to encrypt with qenc32key, usage: --fsgnjn_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fsgnjx_q("fsgnjx_q",
    cl::desc("fsgnjx_q instruction option to encrypt with qenc32key, usage: --fsgnjx_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmin_q("fmin_q",
    cl::desc("fmin_q instruction option to encrypt with qenc32key, usage: --fmin_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmax_q("fmax_q",
    cl::desc("fmax_q instruction option to encrypt with qenc32key, usage: --fmax_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_s_q("fcvt_s_q",
    cl::desc("fcvt_s_q instruction option to encrypt with qenc32key, usage: --fcvt_s_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_q_s("fcvt_q_s",
    cl::desc("fcvt_q_s instruction option to encrypt with qenc32key, usage: --fcvt_q_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_d_q("fcvt_d_q",
    cl::desc("fcvt_d_q instruction option to encrypt with qenc32key, usage: --fcvt_d_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_q_d("fcvt_q_d",
    cl::desc("fcvt_q_d instruction option to encrypt with qenc32key, usage: --fcvt_q_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::feq_q("feq_q",
    cl::desc("feq_q instruction option to encrypt with qenc32key, usage: --feq_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::flt_q("flt_q",
    cl::desc("flt_q instruction option to encrypt with qenc32key, usage: --flt_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fle_q("fle_q",
    cl::desc("fle_q instruction option to encrypt with qenc32key, usage: --fle_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fclass_q("fclass_q",
    cl::desc("fclass_q instruction option to encrypt with qenc32key, usage: --fclass_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_w_q("fcvt_w_q",
    cl::desc("fcvt_w_q instruction option to encrypt with qenc32key, usage: --fcvt_w_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_wu_q("fcvt_wu_q",
    cl::desc("fcvt_wu_q instruction option to encrypt with qenc32key, usage: --fcvt_wu_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_q_w("fcvt_q_w",
    cl::desc("fcvt_q_w instruction option to encrypt with qenc32key, usage: --fcvt_q_w"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_q_wu("fcvt_q_wu",
    cl::desc("fcvt_q_wu instruction option to encrypt with qenc32key, usage: --fcvt_q_wu"),
    cl::cat(ObjdumpCat));


// rv64i instruction options to encrypt with ienc64key

cl::opt<bool> objdump::lwu("lwu",
    cl::desc("lwu instruction option to encrypt with ienc64key, usage: --lwu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::ld("ld",
    cl::desc("ld instruction option to encrypt with ienc64key, usage: --ld"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sd("sd",
    cl::desc("sd instruction option to encrypt with ienc64key, usage: --sd"),
    cl::cat(ObjdumpCat));

/*
cl::opt<bool> objdump::slli("slli",
    cl::desc("slli instruction option to encrypt with ienc64key, usage: --slli"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srli("srli",
    cl::desc("srli instruction option to encrypt with ienc64key, usage: --srli"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srai("srai",
    cl::desc("srai instruction option to encrypt with ienc64key, usage: --srai"),
    cl::cat(ObjdumpCat));
*/

cl::opt<bool> objdump::addiw("addiw",
    cl::desc("addiw instruction option to encrypt with ienc64key, usage: --addiw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::slliw("slliw",
    cl::desc("slliw instruction option to encrypt with ienc64key, usage: --slliw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srliw("srliw",
    cl::desc("srliw instruction option to encrypt with ienc64key, usage: --srliw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sraiw("sraiw",
    cl::desc("sraiw instruction option to encrypt with ienc64key, usage: --sraiw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::addw("addw",
    cl::desc("addw instruction option to encrypt with ienc64key, usage: --addw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::subw("subw",
    cl::desc("subw instruction option to encrypt with ienc64key, usage: --subw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sllw("sllw",
    cl::desc("sllw instruction option to encrypt with ienc64key, usage: --sllw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::srlw("srlw",
    cl::desc("srlw instruction option to encrypt with ienc64key, usage: --srlw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sraw("sraw",
    cl::desc("sraw instruction option to encrypt with ienc64key, usage: --sraw"),
    cl::cat(ObjdumpCat));


// rv64m instruction options to encrypt with menc64key

cl::opt<bool> objdump::mulw("mulw",
    cl::desc("mulw instruction option to encrypt with menc64key, usage: --mulw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::divw("divw",
    cl::desc("divw instruction option to encrypt with menc64key, usage: --divw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::divuw("divuw",
    cl::desc("divuw instruction option to encrypt with menc64key, usage: --divuw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::remw("remw",
    cl::desc("remw instruction option to encrypt with menc64key, usage: --remw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::remuw("remuw",
    cl::desc("remuw instruction option to encrypt with menc64key, usage: --remuw"),
    cl::cat(ObjdumpCat));


// rv64a instruction options to encrypt with aenc64key

cl::opt<bool> objdump::lr_d("lr_d",
    cl::desc("lr_d instruction option to encrypt with aenc64key, usage: --lr_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::sc_d("sc_d",
    cl::desc("sc_d instruction option to encrypt with aenc64key, usage: --sc_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoswap_d("amoswap_d",
    cl::desc("amoswap_d instruction option to encrypt with aenc64key, usage: --amoswap_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoadd_d("amoadd_d",
    cl::desc("amoadd_d instruction option to encrypt with aenc64key, usage: --amoadd_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoxor_d("amoxor_d",
    cl::desc("amoxor_d instruction option to encrypt with aenc64key, usage: --amoxor_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoand_d("amoand_d",
    cl::desc("amoand_d instruction option to encrypt with aenc64key, usage: --amoand_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amoor_d("amoor_d",
    cl::desc("amoor_d instruction option to encrypt with aenc64key, usage: --amoor_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amomin_d("amomin_d",
    cl::desc("amomin_d instruction option to encrypt with aenc64key, usage: --amomin_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amomax_d("amomax_d",
    cl::desc("amomax_d instruction option to encrypt with aenc64key, usage: --amomax_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amominu_d("amominu_d",
    cl::desc("amominu_d instruction option to encrypt with aenc64key, usage: --amominu_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::amomaxu_d("amomaxu_d",
    cl::desc("amomaxu_d instruction option to encrypt with aenc64key, usage: --amomaxu_d"),
    cl::cat(ObjdumpCat));


// rv64f instruction options to encrypt with fenc64key

cl::opt<bool> objdump::fcvt_l_s("fcvt_l_s",
    cl::desc("fcvt_l_s instruction option to encrypt with fenc64key, usage: --fcvt_l_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_lu_s("fcvt_lu_s",
    cl::desc("fcvt_lu_s instruction option to encrypt with fenc64key, usage: --fcvt_lu_s"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_s_l("fcvt_s_l",
    cl::desc("fcvt_s_l instruction option to encrypt with fenc64key, usage: --fcvt_s_l"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_s_lu("fcvt_s_lu",
    cl::desc("fcvt_s_lu instruction option to encrypt with fenc64key, usage: --fcvt_s_lu"),
    cl::cat(ObjdumpCat));


// rv64d instruction options to encrypt with denc64key

cl::opt<bool> objdump::fcvt_l_d("fcvt_l_d",
    cl::desc("fcvt_l_d instruction option to encrypt with denc64key, usage: --fcvt_l_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_lu_d("fcvt_lu_d",
    cl::desc("fcvt_lu_d instruction option to encrypt with denc64key, usage: --fcvt_lu_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmv_x_d("fmv_x_d",
    cl::desc("fmv_x_d instruction option to encrypt with denc64key, usage: --fmv_x_d"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_d_l("fcvt_d_l",
    cl::desc("fcvt_d_l instruction option to encrypt with denc64key, usage: --fcvt_d_l"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_d_lu("fcvt_d_lu",
    cl::desc("fcvt_d_lu instruction option to encrypt with denc64key, usage: --fcvt_d_lu"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fmv_d_x("fmv_d_x",
    cl::desc("fmv_d_x instruction option to encrypt with denc64key, usage: --fmv_d_x"),
    cl::cat(ObjdumpCat));


// rv64q instruction options to encrypt with qenc64key

cl::opt<bool> objdump::fcvt_l_q("fcvt_l_q",
    cl::desc("fcvt_l_q instruction option to encrypt with qenc64key, usage: --fcvt_l_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_lu_q("fcvt_lu_q",
    cl::desc("fcvt_lu_q instruction option to encrypt with qenc64key, usage: --fcvt_lu_q"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_q_l("fcvt_q_l",
    cl::desc("fcvt_q_l instruction option to encrypt with qenc64key, usage: --fcvt_q_l"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::fcvt_q_lu("fcvt_q_lu",
    cl::desc("fcvt_q_lu instruction option to encrypt with qenc64key, usage: --fcvt_q_lu"),
    cl::cat(ObjdumpCat));


// rvc quadrant 0 instruction options to encrypt with cencq0key

cl::opt<bool> objdump::c_addi4spn("c_addi4spn",
    cl::desc("c_addi4spn instruction option to encrypt with cencq0key, usage: --c_addi4spn"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_fld("c_fld",
    cl::desc("c_fld instruction option to encrypt with cencq0key, usage: --c_fld"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_lq("c_lq",
    cl::desc("c_lq instruction option to encrypt with cencq0key, usage: --c_lq"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_lw("c_lw",
    cl::desc("c_lw instruction option to encrypt with cencq0key, usage: --c_lw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_flw("c_flw",
    cl::desc("c_flw instruction option to encrypt with cencq0key, usage: --c_flw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_ld("c_ld",
    cl::desc("c_ld instruction option to encrypt with cencq0key, usage: --c_ld"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_fsd("c_fsd",
    cl::desc("c_fsd instruction option to encrypt with cencq0key, usage: --c_fsd"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_sq("c_sq",
    cl::desc("c_sq instruction option to encrypt with cencq0key, usage: --c_sq"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_sw("c_sw",
    cl::desc("c_sw instruction option to encrypt with cencq0key, usage: --c_sw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_fsw("c_fsw",
    cl::desc("c_fsw instruction option to encrypt with cencq0key, usage: --c_fsw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_sd("c_sd",
    cl::desc("c_sd instruction option to encrypt with cencq0key, usage: --c_sd"),
    cl::cat(ObjdumpCat));


// rvc quadrant 1 instruction options to encrypt with cencq1key

cl::opt<bool> objdump::c_nop("c_nop",
    cl::desc("c_nop instruction option to encrypt with cencq1key, usage: --c_nop"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_addi("c_addi",
    cl::desc("c_addi instruction option to encrypt with cencq1key, usage: --c_addi"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_jal("c_jal",
    cl::desc("c_jal instruction option to encrypt with cencq1key, usage: --c_jal"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_addiw("c_addiw",
    cl::desc("c_addiw instruction option to encrypt with cencq1key, usage: --c_addiw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_li("c_li",
    cl::desc("c_li instruction option to encrypt with cencq1key, usage: --c_li"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_addi16sp("c_addi16sp",
    cl::desc("c_addi16sp instruction option to encrypt with cencq1key, usage: --c_addi16sp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_lui("c_lui",
    cl::desc("c_lui instruction option to encrypt with cencq1key, usage: --c_lui"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_srli("c_srli",
    cl::desc("c_srli instruction option to encrypt with cencq1key, usage: --c_srli"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_srli64("c_srli64",
    cl::desc("c_srli64 instruction option to encrypt with cencq1key, usage: --c_srli64"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_srai("c_srai",
    cl::desc("c_srai instruction option to encrypt with cencq1key, usage: --c_srai"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_srai64("c_srai64",
    cl::desc("c_srai64 instruction option to encrypt with cencq1key, usage: --c_srai64"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_andi("c_andi",
    cl::desc("c_andi instruction option to encrypt with cencq1key, usage: --c_andi"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_sub("c_sub",
    cl::desc("c_sub instruction option to encrypt with cencq1key, usage: --c_sub"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_xor("c_xor",
    cl::desc("c_xor instruction option to encrypt with cencq1key, usage: --c_xor"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_or("c_or",
    cl::desc("c_or instruction option to encrypt with cencq1key, usage: --c_or"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_and("c_and",
    cl::desc("c_and instruction option to encrypt with cencq1key, usage: --c_and"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_subw("c_subw",
    cl::desc("c_subw instruction option to encrypt with cencq1key, usage: --c_subw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_addw("c_addw",
    cl::desc("c_addw instruction option to encrypt with cencq1key, usage: --c_addw"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_j("c_j",
    cl::desc("c_j instruction option to encrypt with cencq1key, usage: --c_j"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_beqz("c_beqz",
    cl::desc("c_beqz instruction option to encrypt with cencq1key, usage: --c_beqz"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_bnez("c_bnez",
    cl::desc("c_bnez instruction option to encrypt with cencq1key, usage: --c_bnez"),
    cl::cat(ObjdumpCat));


// rvc quadrant 2 instruction options to encrypt with cencq2key

cl::opt<bool> objdump::c_slli("c_slli",
    cl::desc("c_slli instruction option to encrypt with cencq2key, usage: --c_slli"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_slli64("c_slli64",
    cl::desc("c_slli64 instruction option to encrypt with cencq2key, usage: --c_slli64"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_fldsp("c_fldsp",
    cl::desc("c_fldsp instruction option to encrypt with cencq2key, usage: --c_fldsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_lqsp("c_lqsp",
    cl::desc("c_lqsp instruction option to encrypt with cencq2key, usage: --c_lqsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_lwsp("c_lwsp",
    cl::desc("c_lwsp instruction option to encrypt with cencq2key, usage: --c_lwsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_flwsp("c_flwsp",
    cl::desc("c_flwsp instruction option to encrypt with cencq2key, usage: --c_flwsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_ldsp("c_ldsp",
    cl::desc("c_ldsp instruction option to encrypt with cencq2key, usage: --c_ldsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_jr("c_jr",
    cl::desc("c_jr instruction option to encrypt with cencq2key, usage: --c_jr"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_mv("c_mv",
    cl::desc("c_mv instruction option to encrypt with cencq2key, usage: --c_mv"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_ebreak("c_ebreak",
    cl::desc("c_ebreak instruction option to encrypt with cencq2key, usage: --c_ebreak"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_jalr("c_jalr",
    cl::desc("c_jalr instruction option to encrypt with cencq2key, usage: --c_jalr"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_add("c_add",
    cl::desc("c_add instruction option to encrypt with cencq2key, usage: --c_add"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_fsdsp("c_fsdsp",
    cl::desc("c_fsdsp instruction option to encrypt with cencq2key, usage: --c_fsdsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_sqsp("c_sqsp",
    cl::desc("c_sqsp instruction option to encrypt with cencq2key, usage: --c_sqsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_swsp("c_swsp",
    cl::desc("c_swsp instruction option to encrypt with cencq2key, usage: --c_swsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_fswsp("c_fswsp",
    cl::desc("c_fswsp instruction option to encrypt with cencq2key, usage: --c_fswsp"),
    cl::cat(ObjdumpCat));

cl::opt<bool> objdump::c_sdsp("c_sdsp",
    cl::desc("c_sdsp instruction option to encrypt with cencq2key, usage: --c_sdsp"),
    cl::cat(ObjdumpCat));


// partial encryption rv32i instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_lui("b_p_lui",
    cl::desc("partial encryption option to encrypt lui instruction with the given key, usage: --b_p_lui=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_auipc("b_p_auipc",
    cl::desc("partial encryption option to encrypt auipc instruction with the given key, usage: --b_p_auipc=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_jal("b_p_jal",
    cl::desc("partial encryption option to encrypt jal instruction with the given key, usage: --b_p_jal=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_jalr("b_p_jalr",
    cl::desc("partial encryption option to encrypt jalr instruction with the given key, usage: --b_p_jalr=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_beq("b_p_beq",
    cl::desc("partial encryption option to encrypt beq instruction with the given key, usage: --b_p_beq=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bne("b_p_bne",
    cl::desc("partial encryption option to encrypt bne instruction with the given key, usage: --b_p_bne=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_blt("b_p_blt",
    cl::desc("partial encryption option to encrypt blt instruction with the given key, usage: --b_p_blt=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bge("b_p_bge",
    cl::desc("partial encryption option to encrypt bge instruction with the given key, usage: --b_p_bge=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bltu("b_p_bltu",
    cl::desc("partial encryption option to encrypt bltu instruction with the given key, usage: --b_p_bltu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_bgeu("b_p_bgeu",
    cl::desc("partial encryption option to encrypt bgeu instruction with the given key, usage: --b_p_bgeu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lb("b_p_lb",
    cl::desc("partial encryption option to encrypt lb instruction with the given key, usage: --b_p_lb=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lh("b_p_lh",
    cl::desc("partial encryption option to encrypt lh instruction with the given key, usage: --b_p_lh=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lw("b_p_lw",
    cl::desc("partial encryption option to encrypt lw instruction with the given key, usage: --b_p_lw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lbu("b_p_lbu",
    cl::desc("partial encryption option to encrypt lbu instruction with the given key, usage: --b_p_lbu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_lhu("b_p_lhu",
    cl::desc("partial encryption option to encrypt lhu instruction with the given key, usage: --b_p_lhu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sb("b_p_sb",
    cl::desc("partial encryption option to encrypt sb instruction with the given key, usage: --b_p_sb=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sh("b_p_sh",
    cl::desc("partial encryption option to encrypt sh instruction with the given key, usage: --b_p_sh=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sw("b_p_sw",
    cl::desc("partial encryption option to encrypt sw instruction with the given key, usage: --b_p_sw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_addi("b_p_addi",
    cl::desc("partial encryption option to encrypt addi instruction with the given key, usage: --b_p_addi=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slti("b_p_slti",
    cl::desc("partial encryption option to encrypt slti instruction with the given key, usage: --b_p_slti=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sltiu("b_p_sltiu",
    cl::desc("partial encryption option to encrypt sltiu instruction with the given key, usage: --b_p_sltiu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_xori("b_p_xori",
    cl::desc("partial encryption option to encrypt xori instruction with the given key, usage: --b_p_xori=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ori("b_p_ori",
    cl::desc("partial encryption option to encrypt ori instruction with the given key, usage: --b_p_ori=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_andi("b_p_andi",
    cl::desc("partial encryption option to encrypt andi instruction with the given key, usage: --b_p_andi=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slli("b_p_slli",
    cl::desc("partial encryption option to encrypt slli instruction with the given key, usage: --b_p_slli=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srli("b_p_srli",
    cl::desc("partial encryption option to encrypt srli instruction with the given key, usage: --b_p_srli=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srai("b_p_srai",
    cl::desc("partial encryption option to encrypt srai instruction with the given key, usage: --b_p_srai=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_add("b_p_add",
    cl::desc("partial encryption option to encrypt add instruction with the given key, usage: --b_p_add=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sub("b_p_sub",
    cl::desc("partial encryption option to encrypt sub instruction with the given key, usage: --b_p_sub=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sll("b_p_sll",
    cl::desc("partial encryption option to encrypt sll instruction with the given key, usage: --b_p_sll=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slt("b_p_slt",
    cl::desc("partial encryption option to encrypt slt instruction with the given key, usage: --b_p_slt=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sltu("b_p_sltu",
    cl::desc("partial encryption option to encrypt sltu instruction with the given key, usage: --b_p_sltu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

// added underscore to b_p_xor as the underscore has been added to xor
cl::opt<std::string> objdump::b_p_xor_("b_p_xor_",
    cl::desc("partial encryption option to encrypt xor instruction with the given key, usage: --b_p_xor_=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srl("b_p_srl",
    cl::desc("partial encryption option to encrypt srl instruction with the given key, usage: --b_p_srl=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sra("b_p_sra",
    cl::desc("partial encryption option to encrypt sra instruction with the given key, usage: --b_p_sra=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

// added underscore to b_p_or as the underscore has been added to or
cl::opt<std::string> objdump::b_p_or_("b_p_or_",
    cl::desc("partial encryption option to encrypt or instruction with the given key, usage: --b_p_or_=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

// added underscore to b_p_and as the underscore has been added to and
cl::opt<std::string> objdump::b_p_and_("b_p_and_",
    cl::desc("partial encryption option to encrypt and instruction with the given key, usage: --b_p_and_=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fence("b_p_fence",
    cl::desc("partial encryption option to encrypt fence instruction with the given key, usage: --b_p_fence=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fence_i("b_p_fence_i",
    cl::desc("partial encryption option to encrypt fence_i instruction with the given key, usage: --b_p_fence_i=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ecall("b_p_ecall",
    cl::desc("partial encryption option to encrypt ecall instruction with the given key, usage: --b_p_ecall=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ebreak("b_p_ebreak",
    cl::desc("partial encryption option to encrypt ebreak instruction with the given key, usage: --b_p_ebreak=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrw("b_p_csrrw",
    cl::desc("partial encryption option to encrypt csrrw instruction with the given key, usage: --b_p_csrrw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrs("b_p_csrrs",
    cl::desc("partial encryption option to encrypt csrrs instruction with the given key, usage: --b_p_csrrs=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrc("b_p_csrrc",
    cl::desc("partial encryption option to encrypt csrrc instruction with the given key, usage: --b_p_csrrc=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrwi("b_p_csrrwi",
    cl::desc("partial encryption option to encrypt csrrwi instruction with the given key, usage: --b_p_csrrwi=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrsi("b_p_csrrsi",
    cl::desc("partial encryption option to encrypt csrrsi instruction with the given key, usage: --b_p_csrrsi=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_csrrci("b_p_csrrci",
    cl::desc("partial encryption option to encrypt csrrci instruction with the given key, usage: --b_p_csrrci=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv32m instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_mul("b_p_mul",
    cl::desc("partial encryption option to encrypt mul instruction with the given key, usage: --b_p_mul=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulh("b_p_mulh",
    cl::desc("partial encryption option to encrypt mulh instruction with the given key, usage: --b_p_mulh=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulhsu("b_p_mulhsu",
    cl::desc("partial encryption option to encrypt mulhsu instruction with the given key, usage: --b_p_mulhsu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_mulhu("b_p_mulhu",
    cl::desc("partial encryption option to encrypt mulhu instruction with the given key, usage: --b_p_mulhu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_div_("b_p_div_",
    cl::desc("partial encryption option to encrypt div instruction with the given key, usage: --b_p_div_=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_divu("b_p_divu",
    cl::desc("partial encryption option to encrypt divu instruction with the given key, usage: --b_p_divu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_rem("b_p_rem",
    cl::desc("partial encryption option to encrypt rem instruction with the given key, usage: --b_p_rem=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_remu("b_p_remu",
    cl::desc("partial encryption option to encrypt remu instruction with the given key, usage: --b_p_remu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv32a instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_lr_w("b_p_lr_w",
    cl::desc("partial encryption option to encrypt lr_w instruction with the given key, usage: --b_p_lr_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sc_w("b_p_sc_w",
    cl::desc("partial encryption option to encrypt sc_w instruction with the given key, usage: --b_p_sc_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoswap_w("b_p_amoswap_w",
    cl::desc("partial encryption option to encrypt amoswap_w instruction with the given key, usage: --b_p_amoswap_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoadd_w("b_p_amoadd_w",
    cl::desc("partial encryption option to encrypt amoadd_w instruction with the given key, usage: --b_p_amoadd_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoxor_w("b_p_amoxor_w",
    cl::desc("partial encryption option to encrypt amoxor_w instruction with the given key, usage: --b_p_amoxor_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoand_w("b_p_amoand_w",
    cl::desc("partial encryption option to encrypt amoand_w instruction with the given key, usage: --b_p_amoand_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoor_w("b_p_amoor_w",
    cl::desc("partial encryption option to encrypt amoor_w instruction with the given key, usage: --b_p_amoor_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amomin_w("b_p_amomin_w",
    cl::desc("partial encryption option to encrypt amomin_w instruction with the given key, usage: --b_p_amomin_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amomax_w("b_p_amomax_w",
    cl::desc("partial encryption option to encrypt amomax_w instruction with the given key, usage: --b_p_amomax_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amominu_w("b_p_amominu_w",
    cl::desc("partial encryption option to encrypt amominu_w instruction with the given key, usage: --b_p_amominu_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amomaxu_w("b_p_amomaxu_w",
    cl::desc("partial encryption option to encrypt amomaxu_w instruction with the given key, usage: --b_p_amomaxu_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv32f instruction options to encrypt with the given keys


cl::opt<std::string> objdump::b_p_flw("b_p_flw",
    cl::desc("partial encryption option to encrypt flw instruction with the given key, usage: --b_p_flw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsw("b_p_fsw",
    cl::desc("partial encryption option to encrypt fsw instruction with the given key, usage: --b_p_fsw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmadd_s("b_p_fmadd_s",
    cl::desc("partial encryption option to encrypt fmadd_s instruction with the given key, usage: --b_p_fmadd_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmsub_s("b_p_fmsub_s",
    cl::desc("partial encryption option to encrypt fmsub_s instruction with the given key, usage: --b_p_fmsub_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fnmsub_s("b_p_fnmsub_s",
    cl::desc("partial encryption option to encrypt fnmsub_s instruction with the given key, usage: --b_p_fnmsub_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fnmadd_s("b_p_fnmadd_s",
    cl::desc("partial encryption option to encrypt fnmadd_s instruction with the given key, usage: --b_p_fnmadd_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fadd_s("b_p_fadd_s",
    cl::desc("partial encryption option to encrypt fadd_s instruction with the given key, usage: --b_p_fadd_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsub_s("b_p_fsub_s",
    cl::desc("partial encryption option to encrypt fsub_s instruction with the given key, usage: --b_p_fsub_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmul_s("b_p_fmul_s",
    cl::desc("partial encryption option to encrypt fmul_s instruction with the given key, usage: --b_p_fmul_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fdiv_s("b_p_fdiv_s",
    cl::desc("partial encryption option to encrypt fdiv_s instruction with the given key, usage: --b_p_fdiv_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsqrt_s("b_p_fsqrt_s",
    cl::desc("partial encryption option to encrypt fsqrt_s instruction with the given key, usage: --b_p_fsqrt_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnj_s("b_p_fsgnj_s",
    cl::desc("partial encryption option to encrypt fsgnj_s instruction with the given key, usage: --b_p_fsgnj_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnjn_s("b_p_fsgnjn_s",
    cl::desc("partial encryption option to encrypt fsgnjn_s instruction with the given key, usage: --b_p_fsgnjn_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnjx_s("b_p_fsgnjx_s",
    cl::desc("partial encryption option to encrypt fsgnjx_s instruction with the given key, usage: --b_p_fsgnjx_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmin_s("b_p_fmin_s",
    cl::desc("partial encryption option to encrypt fmin_s instruction with the given key, usage: --b_p_fmin_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmax_s("b_p_fmax_s",
    cl::desc("partial encryption option to encrypt fmax_s instruction with the given key, usage: --b_p_fmax_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_w_s("b_p_fcvt_w_s",
    cl::desc("partial encryption option to encrypt fcvt_w_s instruction with the given key, usage: --b_p_fcvt_w_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_wu_s("b_p_fcvt_wu_s",
    cl::desc("partial encryption option to encrypt fcvt_wu_s instruction with the given key, usage: --b_p_fcvt_wu_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmv_x_w("b_p_fmv_x_w",
    cl::desc("partial encryption option to encrypt fmv_x_w instruction with the given key, usage: --b_p_fmv_x_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_feq_s("b_p_feq_s",
    cl::desc("partial encryption option to encrypt feq_s instruction with the given key, usage: --b_p_feq_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_flt_s("b_p_flt_s",
    cl::desc("partial encryption option to encrypt flt_s instruction with the given key, usage: --b_p_flt_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fle_s("b_p_fle_s",
    cl::desc("partial encryption option to encrypt fle_s instruction with the given key, usage: --b_p_fle_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fclass_s("b_p_fclass_s",
    cl::desc("partial encryption option to encrypt fclass_s instruction with the given key, usage: --b_p_fclass_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_s_w("b_p_fcvt_s_w",
    cl::desc("partial encryption option to encrypt fcvt_s_w instruction with the given key, usage: --b_p_fcvt_s_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_s_wu("b_p_fcvt_s_wu",
    cl::desc("partial encryption option to encrypt fcvt_s_wu instruction with the given key, usage: --b_p_fcvt_s_wu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmv_w_x("b_p_fmv_w_x",
    cl::desc("partial encryption option to encrypt fmv_w_x instruction with the given key, usage: --b_p_fmv_w_x=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv32d instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_fld("b_p_fld",
    cl::desc("partial encryption option to encrypt fld instruction with the given key, usage: --b_p_fld=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsd("b_p_fsd",
    cl::desc("partial encryption option to encrypt fsd instruction with the given key, usage: --b_p_fsd=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmadd_d("b_p_fmadd_d",
    cl::desc("partial encryption option to encrypt fmadd_d instruction with the given key, usage: --b_p_fmadd_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmsub_d("b_p_fmsub_d",
    cl::desc("partial encryption option to encrypt fmsub_d instruction with the given key, usage: --b_p_fmsub_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fnmsub_d("b_p_fnmsub_d",
    cl::desc("partial encryption option to encrypt fnmsub_d instruction with the given key, usage: --b_p_fnmsub_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fnmadd_d("b_p_fnmadd_d",
    cl::desc("partial encryption option to encrypt fnmadd_d instruction with the given key, usage: --b_p_fnmadd_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fadd_d("b_p_fadd_d",
    cl::desc("partial encryption option to encrypt fadd_d instruction with the given key, usage: --b_p_fadd_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsub_d("b_p_fsub_d",
    cl::desc("partial encryption option to encrypt fsub_d instruction with the given key, usage: --b_p_fsub_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmul_d("b_p_fmul_d",
    cl::desc("partial encryption option to encrypt fmul_d instruction with the given key, usage: --b_p_fmul_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fdiv_d("b_p_fdiv_d",
    cl::desc("partial encryption option to encrypt fdiv_d instruction with the given key, usage: --b_p_fdiv_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsqrt_d("b_p_fsqrt_d",
    cl::desc("partial encryption option to encrypt fsqrt_d instruction with the given key, usage: --b_p_fsqrt_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnj_d("b_p_fsgnj_d",
    cl::desc("partial encryption option to encrypt fsgnj_d instruction with the given key, usage: --b_p_fsgnj_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnjn_d("b_p_fsgnjn_d",
    cl::desc("partial encryption option to encrypt fsgnjn_d instruction with the given key, usage: --b_p_fsgnjn_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnjx_d("b_p_fsgnjx_d",
    cl::desc("partial encryption option to encrypt fsgnjx_d instruction with the given key, usage: --b_p_fsgnjx_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmin_d("b_p_fmin_d",
    cl::desc("partial encryption option to encrypt fmin_d instruction with the given key, usage: --b_p_fmin_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmax_d("b_p_fmax_d",
    cl::desc("partial encryption option to encrypt fmax_d instruction with the given key, usage: --b_p_fmax_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_s_d("b_p_fcvt_s_d",
    cl::desc("partial encryption option to encrypt fcvt_s_d instruction with the given key, usage: --b_p_fcvt_s_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_d_s("b_p_fcvt_d_s",
    cl::desc("partial encryption option to encrypt fcvt_d_s instruction with the given key, usage: --b_p_fcvt_d_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_feq_d("b_p_feq_d",
    cl::desc("partial encryption option to encrypt feq_d instruction with the given key, usage: --b_p_feq_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_flt_d("b_p_flt_d",
    cl::desc("partial encryption option to encrypt flt_d instruction with the given key, usage: --b_p_flt_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fle_d("b_p_fle_d",
    cl::desc("partial encryption option to encrypt fle_d instruction with the given key, usage: --b_p_fle_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fclass_d("b_p_fclass_d",
    cl::desc("partial encryption option to encrypt fclass_d instruction with the given key, usage: --b_p_fclass_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_w_d("b_p_fcvt_w_d",
    cl::desc("partial encryption option to encrypt fcvt_w_d instruction with the given key, usage: --b_p_fcvt_w_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_wu_d("b_p_fcvt_wu_d",
    cl::desc("partial encryption option to encrypt fcvt_wu_d instruction with the given key, usage: --b_p_fcvt_wu_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_d_w("b_p_fcvt_d_w",
    cl::desc("partial encryption option to encrypt fcvt_d_w instruction with the given key, usage: --b_p_fcvt_d_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_d_wu("b_p_fcvt_d_wu",
    cl::desc("partial encryption option to encrypt fcvt_d_wu instruction with the given key, usage: --b_p_fcvt_d_wu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv32q instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_flq("b_p_flq",
    cl::desc("partial encryption option to encrypt flq instruction with the given key, usage: --b_p_flq=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsq("b_p_fsq",
    cl::desc("partial encryption option to encrypt fsq instruction with the given key, usage: --b_p_fsq=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmadd_q("b_p_fmadd_q",
    cl::desc("partial encryption option to encrypt fmadd_q instruction with the given key, usage: --b_p_fmadd_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmsub_q("b_p_fmsub_q",
    cl::desc("partial encryption option to encrypt fmsub_q instruction with the given key, usage: --b_p_fmsub_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fnmsub_q("b_p_fnmsub_q",
    cl::desc("partial encryption option to encrypt fnmsub_q instruction with the given key, usage: --b_p_fnmsub_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fnmadd_q("b_p_fnmadd_q",
    cl::desc("partial encryption option to encrypt fnmadd_q instruction with the given key, usage: --b_p_fnmadd_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fadd_q("b_p_fadd_q",
    cl::desc("partial encryption option to encrypt fadd_q instruction with the given key, usage: --b_p_fadd_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsub_q("b_p_fsub_q",
    cl::desc("partial encryption option to encrypt fsub_q instruction with the given key, usage: --b_p_fsub_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmul_q("b_p_fmul_q",
    cl::desc("partial encryption option to encrypt fmul_q instruction with the given key, usage: --b_p_fmul_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fdiv_q("b_p_fdiv_q",
    cl::desc("partial encryption option to encrypt fdiv_q instruction with the given key, usage: --b_p_fdiv_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsqrt_q("b_p_fsqrt_q",
    cl::desc("partial encryption option to encrypt fsqrt_q instruction with the given key, usage: --b_p_fsqrt_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnj_q("b_p_fsgnj_q",
    cl::desc("partial encryption option to encrypt fsgnj_q instruction with the given key, usage: --b_p_fsgnj_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnjn_q("b_p_fsgnjn_q",
    cl::desc("partial encryption option to encrypt fsgnjn_q instruction with the given key, usage: --b_p_fsgnjn_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fsgnjx_q("b_p_fsgnjx_q",
    cl::desc("partial encryption option to encrypt fsgnjx_q instruction with the given key, usage: --b_p_fsgnjx_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmin_q("b_p_fmin_q",
    cl::desc("partial encryption option to encrypt fmin_q instruction with the given key, usage: --b_p_fmin_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmax_q("b_p_fmax_q",
    cl::desc("partial encryption option to encrypt fmax_q instruction with the given key, usage: --b_p_fmax_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_s_q("b_p_fcvt_s_q",
    cl::desc("partial encryption option to encrypt fcvt_s_q instruction with the given key, usage: --b_p_fcvt_s_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_q_s("b_p_fcvt_q_s",
    cl::desc("partial encryption option to encrypt fcvt_q_s instruction with the given key, usage: --b_p_fcvt_q_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_d_q("b_p_fcvt_d_q",
    cl::desc("partial encryption option to encrypt fcvt_d_q instruction with the given key, usage: --b_p_fcvt_d_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_q_d("b_p_fcvt_q_d",
    cl::desc("partial encryption option to encrypt fcvt_q_d instruction with the given key, usage: --b_p_fcvt_q_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_feq_q("b_p_feq_q",
    cl::desc("partial encryption option to encrypt feq_q instruction with the given key, usage: --b_p_feq_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_flt_q("b_p_flt_q",
    cl::desc("partial encryption option to encrypt flt_q instruction with the given key, usage: --b_p_flt_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fle_q("b_p_fle_q",
    cl::desc("partial encryption option to encrypt fle_q instruction with the given key, usage: --b_p_fle_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fclass_q("b_p_fclass_q",
    cl::desc("partial encryption option to encrypt fclass_q instruction with the given key, usage: --b_p_fclass_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_w_q("b_p_fcvt_w_q",
    cl::desc("partial encryption option to encrypt fcvt_w_q instruction with the given key, usage: --b_p_fcvt_w_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_wu_q("b_p_fcvt_wu_q",
    cl::desc("partial encryption option to encrypt fcvt_wu_q instruction with the given key, usage: --b_p_fcvt_wu_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_q_w("b_p_fcvt_q_w",
    cl::desc("partial encryption option to encrypt fcvt_q_w instruction with the given key, usage: --b_p_fcvt_q_w=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_q_wu("b_p_fcvt_q_wu",
    cl::desc("partial encryption option to encrypt fcvt_q_wu instruction with the given key, usage: --b_p_fcvt_q_wu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv64i instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_lwu("b_p_lwu",
    cl::desc("partial encryption option to encrypt lwu instruction with the given key, usage: --b_p_lwu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_ld("b_p_ld",
    cl::desc("partial encryption option to encrypt ld instruction with the given key, usage: --b_p_ld=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sd("b_p_sd",
    cl::desc("partial encryption option to encrypt sd instruction with the given key, usage: --b_p_sd=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

/*
cl::opt<std::string> objdump::b_p_slli("b_p_slli",
    cl::desc("partial encryption option to encrypt slli instruction with the given key, usage: --b_p_slli=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srli("b_p_srli",
    cl::desc("partial encryption option to encrypt srli instruction with the given key, usage: --b_p_srli=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srai("b_p_srai",
    cl::desc("partial encryption option to encrypt srai instruction with the given key, usage: --b_p_srai=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));
*/

cl::opt<std::string> objdump::b_p_addiw("b_p_addiw",
    cl::desc("partial encryption option to encrypt addiw instruction with the given key, usage: --b_p_addiw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_slliw("b_p_slliw",
    cl::desc("partial encryption option to encrypt slliw instruction with the given key, usage: --b_p_slliw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srliw("b_p_srliw",
    cl::desc("partial encryption option to encrypt srliw instruction with the given key, usage: --b_p_srliw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sraiw("b_p_sraiw",
    cl::desc("partial encryption option to encrypt sraiw instruction with the given key, usage: --b_p_sraiw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_addw("b_p_addw",
    cl::desc("partial encryption option to encrypt addw instruction with the given key, usage: --b_p_addw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_subw("b_p_subw",
    cl::desc("partial encryption option to encrypt subw instruction with the given key, usage: --b_p_subw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sllw("b_p_sllw",
    cl::desc("partial encryption option to encrypt sllw instruction with the given key, usage: --b_p_sllw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_srlw("b_p_srlw",
    cl::desc("partial encryption option to encrypt srlw instruction with the given key, usage: --b_p_srlw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sraw("b_p_sraw",
    cl::desc("partial encryption option to encrypt sraw instruction with the given key, usage: --b_p_sraw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv64m instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_mulw("b_p_mulw",
    cl::desc("partial encryption option to encrypt mulw instruction with the given key, usage: --b_p_mulw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_divw("b_p_divw",
    cl::desc("partial encryption option to encrypt divw instruction with the given key, usage: --b_p_divw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_divuw("b_p_divuw",
    cl::desc("partial encryption option to encrypt divuw instruction with the given key, usage: --b_p_divuw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_remw("b_p_remw",
    cl::desc("partial encryption option to encrypt remw instruction with the given key, usage: --b_p_remw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_remuw("b_p_remuw",
    cl::desc("partial encryption option to encrypt remuw instruction with the given key, usage: --b_p_remuw=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv64a instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_lr_d("b_p_lr_d",
    cl::desc("partial encryption option to encrypt lr_d instruction with the given key, usage: --b_p_lr_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_sc_d("b_p_sc_d",
    cl::desc("partial encryption option to encrypt sc_d instruction with the given key, usage: --b_p_sc_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoswap_d("b_p_amoswap_d",
    cl::desc("partial encryption option to encrypt amoswap_d instruction with the given key, usage: --b_p_amoswap_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoadd_d("b_p_amoadd_d",
    cl::desc("partial encryption option to encrypt amoadd_d instruction with the given key, usage: --b_p_amoadd_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoxor_d("b_p_amoxor_d",
    cl::desc("partial encryption option to encrypt amoxor_d instruction with the given key, usage: --b_p_amoxor_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoand_d("b_p_amoand_d",
    cl::desc("partial encryption option to encrypt amoand_d instruction with the given key, usage: --b_p_amoand_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amoor_d("b_p_amoor_d",
    cl::desc("partial encryption option to encrypt amoor_d instruction with the given key, usage: --b_p_amoor_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amomin_d("b_p_amomin_d",
    cl::desc("partial encryption option to encrypt amomin_d instruction with the given key, usage: --b_p_amomin_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amomax_d("b_p_amomax_d",
    cl::desc("partial encryption option to encrypt amomax_d instruction with the given key, usage: --b_p_amomax_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amominu_d("b_p_amominu_d",
    cl::desc("partial encryption option to encrypt amominu_d instruction with the given key, usage: --b_p_amominu_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_amomaxu_d("b_p_amomaxu_d",
    cl::desc("partial encryption option to encrypt amomaxu_d instruction with the given key, usage: --b_p_amomaxu_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv64f instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_fcvt_l_s("b_p_fcvt_l_s",
    cl::desc("partial encryption option to encrypt fcvt_l_s instruction with the given key, usage: --b_p_fcvt_l_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_lu_s("b_p_fcvt_lu_s",
    cl::desc("partial encryption option to encrypt fcvt_lu_s instruction with the given key, usage: --b_p_fcvt_lu_s=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_s_l("b_p_fcvt_s_l",
    cl::desc("partial encryption option to encrypt fcvt_s_l instruction with the given key, usage: --b_p_fcvt_s_l=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_s_lu("b_p_fcvt_s_lu",
    cl::desc("partial encryption option to encrypt fcvt_s_lu instruction with the given key, usage: --b_p_fcvt_s_lu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv64d instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_fcvt_l_d("b_p_fcvt_l_d",
    cl::desc("partial encryption option to encrypt fcvt_l_d instruction with the given key, usage: --b_p_fcvt_l_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_lu_d("b_p_fcvt_lu_d",
    cl::desc("partial encryption option to encrypt fcvt_lu_d instruction with the given key, usage: --b_p_fcvt_lu_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmv_x_d("b_p_fmv_x_d",
    cl::desc("partial encryption option to encrypt fmv_x_d instruction with the given key, usage: --b_p_fmv_x_d=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_d_l("b_p_fcvt_d_l",
    cl::desc("partial encryption option to encrypt fcvt_d_l instruction with the given key, usage: --b_p_fcvt_d_l=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_d_lu("b_p_fcvt_d_lu",
    cl::desc("partial encryption option to encrypt fcvt_d_lu instruction with the given key, usage: --b_p_fcvt_d_lu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fmv_d_x("b_p_fmv_d_x",
    cl::desc("partial encryption option to encrypt fmv_d_x instruction with the given key, usage: --b_p_fmv_d_x=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rv64q instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_fcvt_l_q("b_p_fcvt_l_q",
    cl::desc("partial encryption option to encrypt fcvt_l_q instruction with the given key, usage: --b_p_fcvt_l_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_lu_q("b_p_fcvt_lu_q",
    cl::desc("partial encryption option to encrypt fcvt_lu_q instruction with the given key, usage: --b_p_fcvt_lu_q=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_q_l("b_p_fcvt_q_l",
    cl::desc("partial encryption option to encrypt fcvt_q_l instruction with the given key, usage: --b_p_fcvt_q_l=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_fcvt_q_lu("b_p_fcvt_q_lu",
    cl::desc("partial encryption option to encrypt fcvt_q_lu instruction with the given key, usage: --b_p_fcvt_q_lu=\"<your32bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rvc quadrant 0 instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_c_addi4spn("b_p_c_addi4spn",
    cl::desc("partial encryption option to encrypt c_addi4spn instruction with the given key, usage: --b_p_c_addi4spn=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_fld("b_p_c_fld",
    cl::desc("partial encryption option to encrypt c_fld instruction with the given key, usage: --b_p_c_fld=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_lq("b_p_c_lq",
    cl::desc("partial encryption option to encrypt c_lq instruction with the given key, usage: --b_p_c_lq=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_lw("b_p_c_lw",
    cl::desc("partial encryption option to encrypt c_lw instruction with the given key, usage: --b_p_c_lw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_flw("b_p_c_flw",
    cl::desc("partial encryption option to encrypt c_flw instruction with the given key, usage: --b_p_c_flw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_ld("b_p_c_ld",
    cl::desc("partial encryption option to encrypt c_ld instruction with the given key, usage: --b_p_c_ld=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_fsd("b_p_c_fsd",
    cl::desc("partial encryption option to encrypt c_fsd instruction with the given key, usage: --b_p_c_fsd=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_sq("b_p_c_sq",
    cl::desc("partial encryption option to encrypt c_sq instruction with the given key, usage: --b_p_c_sq=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_sw("b_p_c_sw",
    cl::desc("partial encryption option to encrypt c_sw instruction with the given key, usage: --b_p_c_sw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_fsw("b_p_c_fsw",
    cl::desc("partial encryption option to encrypt c_fsw instruction with the given key, usage: --b_p_c_fsw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_sd("b_p_c_sd",
    cl::desc("partial encryption option to encrypt c_sd instruction with the given key, usage: --b_p_c_sd=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rvc quadrant 1 instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_c_nop("b_p_c_nop",
    cl::desc("partial encryption option to encrypt c_nop instruction with the given key, usage: --b_p_c_nop=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_addi("b_p_c_addi",
    cl::desc("partial encryption option to encrypt c_addi instruction with the given key, usage: --b_p_c_addi=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_jal("b_p_c_jal",
    cl::desc("partial encryption option to encrypt c_jal instruction with the given key, usage: --b_p_c_jal=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_addiw("b_p_c_addiw",
    cl::desc("partial encryption option to encrypt c_addiw instruction with the given key, usage: --b_p_c_addiw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_li("b_p_c_li",
    cl::desc("partial encryption option to encrypt c_li instruction with the given key, usage: --b_p_c_li=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_addi16sp("b_p_c_addi16sp",
    cl::desc("partial encryption option to encrypt c_addi16sp instruction with the given key, usage: --b_p_c_addi16sp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_lui("b_p_c_lui",
    cl::desc("partial encryption option to encrypt c_lui instruction with the given key, usage: --b_p_c_lui=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_srli("b_p_c_srli",
    cl::desc("partial encryption option to encrypt c_srli instruction with the given key, usage: --b_p_c_srli=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_srli64("b_p_c_srli64",
    cl::desc("partial encryption option to encrypt c_srli64 instruction with the given key, usage: --b_p_c_srli64=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_srai("b_p_c_srai",
    cl::desc("partial encryption option to encrypt c_srai instruction with the given key, usage: --b_p_c_srai=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_srai64("b_p_c_srai64",
    cl::desc("partial encryption option to encrypt c_srai64 instruction with the given key, usage: --b_p_c_srai64=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_andi("b_p_c_andi",
    cl::desc("partial encryption option to encrypt c_andi instruction with the given key, usage: --b_p_c_andi=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_sub("b_p_c_sub",
    cl::desc("partial encryption option to encrypt c_sub instruction with the given key, usage: --b_p_c_sub=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_xor("b_p_c_xor",
    cl::desc("partial encryption option to encrypt c_xor instruction with the given key, usage: --b_p_c_xor=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_or("b_p_c_or",
    cl::desc("partial encryption option to encrypt c_or instruction with the given key, usage: --b_p_c_or=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_and("b_p_c_and",
    cl::desc("partial encryption option to encrypt c_and instruction with the given key, usage: --b_p_c_and=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_subw("b_p_c_subw",
    cl::desc("partial encryption option to encrypt c_subw instruction with the given key, usage: --b_p_c_subw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_addw("b_p_c_addw",
    cl::desc("partial encryption option to encrypt c_addw instruction with the given key, usage: --b_p_c_addw=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_j("b_p_c_j",
    cl::desc("partial encryption option to encrypt c_j instruction with the given key, usage: --b_p_c_j=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_beqz("b_p_c_beqz",
    cl::desc("partial encryption option to encrypt c_beqz instruction with the given key, usage: --b_p_c_beqz=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_bnez("b_p_c_bnez",
    cl::desc("partial encryption option to encrypt c_bnez instruction with the given key, usage: --b_p_c_bnez=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


// partial encryption rvc quadrant 2 instruction options to encrypt with the given keys

cl::opt<std::string> objdump::b_p_c_slli("b_p_c_slli",
    cl::desc("partial encryption option to encrypt c_slli instruction with the given key, usage: --b_p_c_slli=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_slli64("b_p_c_slli64",
    cl::desc("partial encryption option to encrypt c_slli64 instruction with the given key, usage: --b_p_c_slli64=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_fldsp("b_p_c_fldsp",
    cl::desc("partial encryption option to encrypt c_fldsp instruction with the given key, usage: --b_p_c_fldsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_lqsp("b_p_c_lqsp",
    cl::desc("partial encryption option to encrypt c_lqsp instruction with the given key, usage: --b_p_c_lqsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_lwsp("b_p_c_lwsp",
    cl::desc("partial encryption option to encrypt c_lwsp instruction with the given key, usage: --b_p_c_lwsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_flwsp("b_p_c_flwsp",
    cl::desc("partial encryption option to encrypt c_flwsp instruction with the given key, usage: --b_p_c_flwsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_ldsp("b_p_c_ldsp",
    cl::desc("partial encryption option to encrypt c_ldsp instruction with the given key, usage: --b_p_c_ldsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_jr("b_p_c_jr",
    cl::desc("partial encryption option to encrypt c_jr instruction with the given key, usage: --b_p_c_jr=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_mv("b_p_c_mv",
    cl::desc("partial encryption option to encrypt c_mv instruction with the given key, usage: --b_p_c_mv=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_ebreak("b_p_c_ebreak",
    cl::desc("partial encryption option to encrypt c_ebreak instruction with the given key, usage: --b_p_c_ebreak=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_jalr("b_p_c_jalr",
    cl::desc("partial encryption option to encrypt c_jalr instruction with the given key, usage: --b_p_c_jalr=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_add("b_p_c_add",
    cl::desc("partial encryption option to encrypt c_add instruction with the given key, usage: --b_p_c_add=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_fsdsp("b_p_c_fsdsp",
    cl::desc("partial encryption option to encrypt c_fsdsp instruction with the given key, usage: --b_p_c_fsdsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_sqsp("b_p_c_sqsp",
    cl::desc("partial encryption option to encrypt c_sqsp instruction with the given key, usage: --b_p_c_sqsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_swsp("b_p_c_swsp",
    cl::desc("partial encryption option to encrypt c_swsp instruction with the given key, usage: --b_p_c_swsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_fswsp("b_p_c_fswsp",
    cl::desc("partial encryption option to encrypt c_fswsp instruction with the given key, usage: --b_p_c_fswsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::b_p_c_sdsp("b_p_c_sdsp",
    cl::desc("partial encryption option to encrypt c_sdsp instruction with the given key, usage: --b_p_c_sdsp=\"<your16bitkeyasbinary>\""),
    cl::cat(ObjdumpCat));


static cl::opt<std::string>
    ArchName("arch-name",
             cl::desc("Target arch to disassemble for, "
                      "see -version for available targets"),
             cl::cat(ObjdumpCat));

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

static cl::opt<bool> DisassembleZeroes(
    "disassemble-zeroes",
    cl::desc("Do not skip blocks of zeroes when disassembling"),
    cl::cat(ObjdumpCat));
static cl::alias
    DisassembleZeroesShort("z", cl::desc("Alias for --disassemble-zeroes"),
                           cl::NotHidden, cl::Grouping,
                           cl::aliasopt(DisassembleZeroes));

static cl::list<std::string> InputFilenames(cl::Positional,
                                            cl::desc("<input object files>"),
                                            cl::ZeroOrMore,
                                            cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::MCPU(
    "mcpu", cl::desc("Target a specific cpu type (-mcpu=help for details)"),
    cl::value_desc("cpu-name"), cl::init(""), cl::cat(ObjdumpCat));

cl::list<std::string> objdump::MAttrs("mattr", cl::CommaSeparated,
                                      cl::desc("Target specific attributes"),
                                      cl::value_desc("a1,+a2,-a3,..."),
                                      cl::cat(ObjdumpCat));

static cl::opt<uint64_t>
    StartAddress("start-address", cl::desc("Disassemble beginning at address"),
                 cl::value_desc("address"), cl::init(0), cl::cat(ObjdumpCat));
static cl::opt<uint64_t> 
    StopAddress("stop-address",
                                     cl::desc("Stop disassembly at address"),
                                     cl::value_desc("address"),
                                     cl::init(UINT64_MAX), cl::cat(ObjdumpCat));

cl::opt<std::string> objdump::TripleName(
    "triple",
    cl::desc(
        "Target triple to disassemble for, see -version for available targets"),
    cl::cat(ObjdumpCat));

static cl::extrahelp
    HelpResponse("\nPass @FILE as argument to read options from FILE.\n");



static StringRef ToolName;

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

template< typename T >
std::string int_to_hex( T i )
{
  std::stringstream stream;
  stream << std::hex << i;
  return stream.str();
}

const char* hex_char_to_bin(char c)
{
    switch(std::tolower(c))
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
        case 'a': return "1010";
        case 'b': return "1011";
        case 'c': return "1100";
        case 'd': return "1101";
        case 'e': return "1110";
        case 'f': return "1111";
    }
    return "0000";
}

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

std::string strBitArrXor(std::string a, std::string b, int j){

std::string c = "";

for ( int i = 0; i < 8*j; ++i )
{
    c += ((a[i] - '0') ^ (b[i] - '0')) + '0';
}
return c;
}

int counter = 0;
int num = 0;

namespace portedOuts{
  std::error_code EC = std::error_code();
  
  raw_fd_ostream *outs; //("a.txt", EC); //, sys::fs::CD_CreateAlways, sys::fs::FA_Write | sys::fs::FA_Read, sys::fs::OF_None);
}

namespace portedDump {
 void dumpBytes(ArrayRef<uint8_t> bytes, raw_ostream &OS, std::string encryptInstParKey,
                bool encryptInstrv32i,
                bool encryptInstrv32m,
                bool encryptInstrv32a,
                bool encryptInstrv32f,
                bool encryptInstrv32d,
                bool encryptInstrv32q,

                bool encryptInstrv64i,
                bool encryptInstrv64m,
                bool encryptInstrv64a,
                bool encryptInstrv64f,
                bool encryptInstrv64d,
                bool encryptInstrv64q,

                bool encryptInstrvcq0,
                bool encryptInstrvcq1,
                bool encryptInstrvcq2
 ) {
  std::stringstream OSS;

  int j = 0;
  char hexArr[(bytes.size()) / 8];
  static const char hex_rep[] = "0123456789abcdef";

  for (char i: r_wrap(bytes)){
     hexArr[2*j] = hex_rep[(i & 0xF0) >> 4];
     hexArr[2*j + 1] = hex_rep[i & 0xF];
     j++;
  }

  std::string binArr = "";
  for(int i=0;i<strlen(hexArr);i++)
    binArr += hex_char_to_bin(hexArr[i]);

  counter++;

  // Here we can add desired encryption method

  if(custom == true){
    OSS << custom_encrypt(binArr);
  }
  else if(rsa){
      RSA32 rsa;
      
      OSS << std::hex << std::setfill('0') << std::setw(8) << int_to_hex(rsa.Encrypta(std::stoll(binArr, NULL, 2), pkey, qkey, ekey));
  }
  else{
    if(instnum != ""){
        if(enckeyall != "" && enckeyall != "00000000000000000000000000000000"  && (counter <= num)){
          std::stringstream enckeyallaligned;
          enckeyallaligned << std::setfill('0') << std::setw(32) << enckeyall; // std::setw should 32 or 8*j this is the choice but if this we should select least significant 16 bit of enckeyall
    
          OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(enckeyallaligned.str()), j),NULL,  2);
        }
    
        else if(encryptInstParKey != ""){
          OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(encryptInstParKey), j),NULL,  2);
        }
    
        else if(encryptInstrv32i){
          if(ienc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(ienc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32m){
          if(menc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(menc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32a){
          if(aenc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(aenc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32f){
          if(fenc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(fenc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32d){
          if(denc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(denc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32q){
          if(qenc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(qenc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
    
        else if(encryptInstrv64i){
          if(ienc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(ienc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64m){
          if(menc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(menc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64a){
          if(aenc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(aenc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64f){
          if(fenc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(fenc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64d){
          if(denc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(denc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64q){
          if(qenc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(qenc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
    
        else if(encryptInstrvcq0){
          if(cencq0key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(cencq0key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrvcq1){
          if(cencq1key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(cencq1key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrvcq2){
          if(cencq2key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(cencq2key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }


        else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
    }
    else {
       if(enckeyall != "" && enckeyall != "00000000000000000000000000000000"){
          std::stringstream enckeyallaligned;
          enckeyallaligned << std::setfill('0') << std::setw(32) << enckeyall; // std::setw should 32 or 8*j this is the choice but if this we should select least significant 16 bit of enckeyall
    
          OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(enckeyallaligned.str()), j),NULL,  2);
        }
    
        else if(encryptInstParKey != ""){
          OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(encryptInstParKey), j),NULL,  2);
        }
    
        else if(encryptInstrv32i){
          if(ienc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(ienc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32m){
          if(menc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(menc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32a){
          if(aenc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(aenc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32f){
          if(fenc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(fenc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32d){
          if(denc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(denc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv32q){
          if(qenc32key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(qenc32key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
    
        else if(encryptInstrv64i){
          if(ienc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(ienc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64m){
          if(menc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(menc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64a){
          if(aenc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(aenc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64f){
          if(fenc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(fenc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64d){
          if(denc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(denc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrv64q){
          if(qenc64key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(qenc64key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
    
        else if(encryptInstrvcq0){
          if(cencq0key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(cencq0key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrvcq1){
          if(cencq1key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(cencq1key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }
        else if(encryptInstrvcq2){
          if(cencq2key != ""){
            OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, std::string(cencq2key), j),NULL,  2);
          }
          else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
        }


        else{
            if(j==2) OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "0000000000000000", j),NULL,  2);
            else OSS << std::hex << std::setfill('0') << std::setw(2*j) << std::stoll(strBitArrXor(binArr, "00000000000000000000000000000000", j),NULL,  2);
          } // do not encrypt, just print to hex file
    
    }
  }
  OS << OSS.str();
  OSS.str("");
  //OS<<"dene";
  //OSS.clear();
  //OS << std::stoll(binArr,NULL,  2);
 } // end of dumpBytes function
} // end of namespace portedDump


static FilterResult checkSectionFilter(object::SectionRef S) {
  return {/*Keep=*/true, /*IncrementIndex=*/true}; // if (FilterSections.empty())
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


/// Indicates whether this relocation should hidden when listing
/// relocations, usually because it is the trailing part of a multipart
/// relocation that will be printed as part of the leading relocation.
static bool getHidden(RelocationRef RelRef) {
  return false; // if(!Macho)
}

namespace {

class PrettyPrinter {
public:
  virtual ~PrettyPrinter() = default;
  virtual void
  printInst(MCInstPrinter &IP, const MCInst *MI, ArrayRef<uint8_t> Bytes,
            object::SectionedAddress Address, formatted_raw_ostream &OS,
            StringRef Annot, MCSubtargetInfo const &STI,
            StringRef ObjectFilename) {

    //dumpBytes(Bytes, OS);

    bool encryptInstrv32i = false;
    bool encryptInstrv32m = false;
    bool encryptInstrv32a = false;
    bool encryptInstrv32f = false;
    bool encryptInstrv32d = false;
    bool encryptInstrv32q = false;

    bool encryptInstrv64i = false;
    bool encryptInstrv64m = false;
    bool encryptInstrv64a = false;
    bool encryptInstrv64f = false;
    bool encryptInstrv64d = false;
    bool encryptInstrv64q = false;

    bool encryptInstrvcq0 = false;
    bool encryptInstrvcq1 = false;
    bool encryptInstrvcq2 = false;

    std::string encryptInstParKey = "";

    if     ( lui        && ( MI->getOpcode() == RISCV::LUI        ) ) encryptInstrv32i = true;         
    else if( auipc      && ( MI->getOpcode() == RISCV::AUIPC      ) ) encryptInstrv32i = true; 
    else if( jal        && ( MI->getOpcode() == RISCV::JAL        ) ) encryptInstrv32i = true;
    else if( jalr       && ( MI->getOpcode() == RISCV::JALR       ) ) encryptInstrv32i = true;
    else if( beq        && ( MI->getOpcode() == RISCV::BEQ        ) ) encryptInstrv32i = true;
    else if( bne        && ( MI->getOpcode() == RISCV::BNE        ) ) encryptInstrv32i = true;
    else if( blt        && ( MI->getOpcode() == RISCV::BLT        ) ) encryptInstrv32i = true;
    else if( bge        && ( MI->getOpcode() == RISCV::BGE        ) ) encryptInstrv32i = true;
    else if( bltu       && ( MI->getOpcode() == RISCV::BLTU       ) ) encryptInstrv32i = true;
    else if( bgeu       && ( MI->getOpcode() == RISCV::BGEU       ) ) encryptInstrv32i = true;
    else if( lb         && ( MI->getOpcode() == RISCV::LB         ) ) encryptInstrv32i = true;
    else if( lh         && ( MI->getOpcode() == RISCV::LH         ) ) encryptInstrv32i = true;
    else if( lw         && ( MI->getOpcode() == RISCV::LW         ) ) encryptInstrv32i = true;
    else if( lbu        && ( MI->getOpcode() == RISCV::LBU        ) ) encryptInstrv32i = true;
    else if( lhu        && ( MI->getOpcode() == RISCV::LHU        ) ) encryptInstrv32i = true;
    else if( sb         && ( MI->getOpcode() == RISCV::SB         ) ) encryptInstrv32i = true;
    else if( sh         && ( MI->getOpcode() == RISCV::SH         ) ) encryptInstrv32i = true;
    else if( sw         && ( MI->getOpcode() == RISCV::SW         ) ) encryptInstrv32i = true;
    else if( addi       && ( MI->getOpcode() == RISCV::ADDI       ) ) encryptInstrv32i = true;
    else if( slti       && ( MI->getOpcode() == RISCV::SLTI       ) ) encryptInstrv32i = true;
    else if( sltiu      && ( MI->getOpcode() == RISCV::SLTIU      ) ) encryptInstrv32i = true; 
    else if( xori       && ( MI->getOpcode() == RISCV::XORI       ) ) encryptInstrv32i = true;
    else if( ori        && ( MI->getOpcode() == RISCV::ORI        ) ) encryptInstrv32i = true;
    else if( andi       && ( MI->getOpcode() == RISCV::ANDI       ) ) encryptInstrv32i = true;
    else if( slli       && ( MI->getOpcode() == RISCV::SLLI       ) ) encryptInstrv32i = true;
    else if( srli       && ( MI->getOpcode() == RISCV::SRLI       ) ) encryptInstrv32i = true;
    else if( srai       && ( MI->getOpcode() == RISCV::SRAI       ) ) encryptInstrv32i = true;
    else if( add        && ( MI->getOpcode() == RISCV::ADD        ) ) encryptInstrv32i = true;
    else if( sub        && ( MI->getOpcode() == RISCV::SUB        ) ) encryptInstrv32i = true;
    else if( sll        && ( MI->getOpcode() == RISCV::SLL        ) ) encryptInstrv32i = true;
    else if( slt        && ( MI->getOpcode() == RISCV::SLT        ) ) encryptInstrv32i = true;
    else if( sltu       && ( MI->getOpcode() == RISCV::SLTU       ) ) encryptInstrv32i = true;
    else if( xor_       && ( MI->getOpcode() == RISCV::XOR        ) ) encryptInstrv32i = true;
    else if( srl        && ( MI->getOpcode() == RISCV::SRL        ) ) encryptInstrv32i = true;
    else if( sra        && ( MI->getOpcode() == RISCV::SRA        ) ) encryptInstrv32i = true;
    else if( or_        && ( MI->getOpcode() == RISCV::OR         ) ) encryptInstrv32i = true;
    else if( and_       && ( MI->getOpcode() == RISCV::AND        ) ) encryptInstrv32i = true;
    else if( fence      && ( MI->getOpcode() == RISCV::FENCE      ) ) encryptInstrv32i = true; 
    else if( fence_i    && ( MI->getOpcode() == RISCV::FENCE_I    ) ) encryptInstrv32i = true;     
    else if( ecall      && ( MI->getOpcode() == RISCV::ECALL      ) ) encryptInstrv32i = true; 
    else if( ebreak     && ( MI->getOpcode() == RISCV::EBREAK     ) ) encryptInstrv32i = true;   
    else if( csrrw      && ( MI->getOpcode() == RISCV::CSRRW      ) ) encryptInstrv32i = true; 
    else if( csrrs      && ( MI->getOpcode() == RISCV::CSRRS      ) ) encryptInstrv32i = true; 
    else if( csrrc      && ( MI->getOpcode() == RISCV::CSRRC      ) ) encryptInstrv32i = true; 
    else if( csrrwi     && ( MI->getOpcode() == RISCV::CSRRWI     ) ) encryptInstrv32i = true;   
    else if( csrrsi     && ( MI->getOpcode() == RISCV::CSRRSI     ) ) encryptInstrv32i = true;   
    else if( csrrci     && ( MI->getOpcode() == RISCV::CSRRCI     ) ) encryptInstrv32i = true;   

    else if( lwu        && ( MI->getOpcode() == RISCV::LWU        ) ) encryptInstrv64i = true;
    else if( ld         && ( MI->getOpcode() == RISCV::LD         ) ) encryptInstrv64i = true;
    else if( sd         && ( MI->getOpcode() == RISCV::SD         ) ) encryptInstrv64i = true;
    //else if( slli       && ( MI->getOpcode() == RISCV::SLLI       ) ) encryptInstrv64i = true; 
    //else if( srli       && ( MI->getOpcode() == RISCV::SRLI       ) ) encryptInstrv64i = true; 
    //else if( srai       && ( MI->getOpcode() == RISCV::SRAI       ) ) encryptInstrv64i = true; 
    else if( addiw      && ( MI->getOpcode() == RISCV::ADDIW      ) ) encryptInstrv64i = true;   
    else if( slliw      && ( MI->getOpcode() == RISCV::SLLIW      ) ) encryptInstrv64i = true;   
    else if( srliw      && ( MI->getOpcode() == RISCV::SRLIW      ) ) encryptInstrv64i = true;   
    else if( sraiw      && ( MI->getOpcode() == RISCV::SRAIW      ) ) encryptInstrv64i = true;   
    else if( addw       && ( MI->getOpcode() == RISCV::ADDW       ) ) encryptInstrv64i = true;   
    else if( subw       && ( MI->getOpcode() == RISCV::SUBW       ) ) encryptInstrv64i = true;   
    else if( sllw       && ( MI->getOpcode() == RISCV::SLLW       ) ) encryptInstrv64i = true;   
    else if( srlw       && ( MI->getOpcode() == RISCV::SRLW       ) ) encryptInstrv64i = true;   
    else if( sraw       && ( MI->getOpcode() == RISCV::SRAW       ) ) encryptInstrv64i = true;

    else if( mul        && ( MI->getOpcode() == RISCV::MUL        ) ) encryptInstrv32m = true;   
    else if( mulh       && ( MI->getOpcode() == RISCV::MULH       ) ) encryptInstrv32m = true;     
    else if( mulhsu     && ( MI->getOpcode() == RISCV::MULHSU     ) ) encryptInstrv32m = true;         
    else if( mulhu      && ( MI->getOpcode() == RISCV::MULHU      ) ) encryptInstrv32m = true;       
    else if( div_       && ( MI->getOpcode() == RISCV::DIV        ) ) encryptInstrv32m = true;   
    else if( divu       && ( MI->getOpcode() == RISCV::DIVU       ) ) encryptInstrv32m = true;     
    else if( rem        && ( MI->getOpcode() == RISCV::REM        ) ) encryptInstrv32m = true;   
    else if( remu       && ( MI->getOpcode() == RISCV::REMU       ) ) encryptInstrv32m = true;

    else if( mulw       && ( MI->getOpcode() == RISCV::MULW       ) ) encryptInstrv64m = true;   
    else if( divw       && ( MI->getOpcode() == RISCV::DIVW       ) ) encryptInstrv64m = true;   
    else if( divuw      && ( MI->getOpcode() == RISCV::DIVUW      ) ) encryptInstrv64m = true;     
    else if( remw       && ( MI->getOpcode() == RISCV::REMW       ) ) encryptInstrv64m = true;   
    else if( remuw      && ( MI->getOpcode() == RISCV::REMUW      ) ) encryptInstrv64m = true; 

    else if( lr_w       && ( MI->getOpcode() == RISCV::LR_W       ) ) encryptInstrv32a = true;
    else if( sc_w       && ( MI->getOpcode() == RISCV::SC_W       ) ) encryptInstrv32a = true;
    else if( amoswap_w  && ( MI->getOpcode() == RISCV::AMOSWAP_W  ) ) encryptInstrv32a = true;               
    else if( amoadd_w   && ( MI->getOpcode() == RISCV::AMOADD_W   ) ) encryptInstrv32a = true;             
    else if( amoxor_w   && ( MI->getOpcode() == RISCV::AMOXOR_W   ) ) encryptInstrv32a = true;             
    else if( amoand_w   && ( MI->getOpcode() == RISCV::AMOAND_W   ) ) encryptInstrv32a = true;             
    else if( amoor_w    && ( MI->getOpcode() == RISCV::AMOOR_W    ) ) encryptInstrv32a = true;           
    else if( amomin_w   && ( MI->getOpcode() == RISCV::AMOMIN_W   ) ) encryptInstrv32a = true;             
    else if( amomax_w   && ( MI->getOpcode() == RISCV::AMOMAX_W   ) ) encryptInstrv32a = true;             
    else if( amominu_w  && ( MI->getOpcode() == RISCV::AMOMINU_W  ) ) encryptInstrv32a = true;               
    else if( amomaxu_w  && ( MI->getOpcode() == RISCV::AMOMAXU_W  ) ) encryptInstrv32a = true;         

    else if( lr_d       && ( MI->getOpcode() == RISCV::LR_D       ) ) encryptInstrv64a = true;
    else if( sc_d       && ( MI->getOpcode() == RISCV::SC_D       ) ) encryptInstrv64a = true;
    else if( amoswap_d  && ( MI->getOpcode() == RISCV::AMOSWAP_D  ) ) encryptInstrv64a = true;         
    else if( amoadd_d   && ( MI->getOpcode() == RISCV::AMOADD_D   ) ) encryptInstrv64a = true;         
    else if( amoxor_d   && ( MI->getOpcode() == RISCV::AMOXOR_D   ) ) encryptInstrv64a = true;         
    else if( amoand_d   && ( MI->getOpcode() == RISCV::AMOAND_D   ) ) encryptInstrv64a = true;         
    else if( amoor_d    && ( MI->getOpcode() == RISCV::AMOOR_D    ) ) encryptInstrv64a = true;     
    else if( amomin_d   && ( MI->getOpcode() == RISCV::AMOMIN_D   ) ) encryptInstrv64a = true;         
    else if( amomax_d   && ( MI->getOpcode() == RISCV::AMOMAX_D   ) ) encryptInstrv64a = true;         
    else if( amominu_d  && ( MI->getOpcode() == RISCV::AMOMINU_D  ) ) encryptInstrv64a = true;         
    else if( amomaxu_d  && ( MI->getOpcode() == RISCV::AMOMAXU_D  ) ) encryptInstrv64a = true;         

    else if( flw        && ( MI->getOpcode() == RISCV::FLW        ) ) encryptInstrv32f = true;
    else if( fsw        && ( MI->getOpcode() == RISCV::FSW        ) ) encryptInstrv32f = true;
    else if( fmadd_s    && ( MI->getOpcode() == RISCV::FMADD_S    ) ) encryptInstrv32f = true;     
    else if( fmsub_s    && ( MI->getOpcode() == RISCV::FMSUB_S    ) ) encryptInstrv32f = true;     
    else if( fnmsub_s   && ( MI->getOpcode() == RISCV::FNMSUB_S   ) ) encryptInstrv32f = true;         
    else if( fnmadd_s   && ( MI->getOpcode() == RISCV::FNMADD_S   ) ) encryptInstrv32f = true;         
    else if( fadd_s     && ( MI->getOpcode() == RISCV::FADD_S     ) ) encryptInstrv32f = true;   
    else if( fsub_s     && ( MI->getOpcode() == RISCV::FSUB_S     ) ) encryptInstrv32f = true;   
    else if( fmul_s     && ( MI->getOpcode() == RISCV::FMUL_S     ) ) encryptInstrv32f = true;   
    else if( fdiv_s     && ( MI->getOpcode() == RISCV::FDIV_S     ) ) encryptInstrv32f = true;   
    else if( fsqrt_s    && ( MI->getOpcode() == RISCV::FSQRT_S    ) ) encryptInstrv32f = true;       
    else if( fsgnj_s    && ( MI->getOpcode() == RISCV::FSGNJ_S    ) ) encryptInstrv32f = true;       
    else if( fsgnjn_s   && ( MI->getOpcode() == RISCV::FSGNJN_S   ) ) encryptInstrv32f = true;         
    else if( fsgnjx_s   && ( MI->getOpcode() == RISCV::FSGNJX_S   ) ) encryptInstrv32f = true;         
    else if( fmin_s     && ( MI->getOpcode() == RISCV::FMIN_S     ) ) encryptInstrv32f = true;     
    else if( fmax_s     && ( MI->getOpcode() == RISCV::FMAX_S     ) ) encryptInstrv32f = true;     
    else if( fcvt_w_s   && ( MI->getOpcode() == RISCV::FCVT_W_S   ) ) encryptInstrv32f = true;         
    else if( fcvt_wu_s  && ( MI->getOpcode() == RISCV::FCVT_WU_S  ) ) encryptInstrv32f = true;         
    else if( fmv_x_w    && ( MI->getOpcode() == RISCV::FMV_X_W    ) ) encryptInstrv32f = true;       
    else if( feq_s      && ( MI->getOpcode() == RISCV::FEQ_S      ) ) encryptInstrv32f = true;   
    else if( flt_s      && ( MI->getOpcode() == RISCV::FLT_S      ) ) encryptInstrv32f = true;   
    else if( fle_s      && ( MI->getOpcode() == RISCV::FLE_S      ) ) encryptInstrv32f = true;   
    else if( fclass_s   && ( MI->getOpcode() == RISCV::FCLASS_S   ) ) encryptInstrv32f = true;         
    else if( fcvt_s_w   && ( MI->getOpcode() == RISCV::FCVT_S_W   ) ) encryptInstrv32f = true;         
    else if( fcvt_s_wu  && ( MI->getOpcode() == RISCV::FCVT_S_WU  ) ) encryptInstrv32f = true;         
    else if( fmv_w_x    && ( MI->getOpcode() == RISCV::FMV_W_X    ) ) encryptInstrv32f = true;     

    else if( fcvt_l_s   && ( MI->getOpcode() == RISCV::FCVT_L_S   ) ) encryptInstrv64f = true;       
    else if( fcvt_lu_s  && ( MI->getOpcode() == RISCV::FCVT_LU_S  ) ) encryptInstrv64f = true;         
    else if( fcvt_s_l   && ( MI->getOpcode() == RISCV::FCVT_S_L   ) ) encryptInstrv64f = true;       
    else if( fcvt_s_lu  && ( MI->getOpcode() == RISCV::FCVT_S_LU  ) ) encryptInstrv64f = true;         

    else if( fld        && ( MI->getOpcode() == RISCV::FLD        ) ) encryptInstrv32d = true;
    else if( fsd        && ( MI->getOpcode() == RISCV::FSD        ) ) encryptInstrv32d = true;
    else if( fmadd_d    && ( MI->getOpcode() == RISCV::FMADD_D    ) ) encryptInstrv32d = true;     
    else if( fmsub_d    && ( MI->getOpcode() == RISCV::FMSUB_D    ) ) encryptInstrv32d = true;     
    else if( fnmsub_d   && ( MI->getOpcode() == RISCV::FNMSUB_D   ) ) encryptInstrv32d = true;       
    else if( fnmadd_d   && ( MI->getOpcode() == RISCV::FNMADD_D   ) ) encryptInstrv32d = true;       
    else if( fadd_d     && ( MI->getOpcode() == RISCV::FADD_D     ) ) encryptInstrv32d = true;   
    else if( fsub_d     && ( MI->getOpcode() == RISCV::FSUB_D     ) ) encryptInstrv32d = true;   
    else if( fmul_d     && ( MI->getOpcode() == RISCV::FMUL_D     ) ) encryptInstrv32d = true;   
    else if( fdiv_d     && ( MI->getOpcode() == RISCV::FDIV_D     ) ) encryptInstrv32d = true;   
    else if( fsqrt_d    && ( MI->getOpcode() == RISCV::FSQRT_D    ) ) encryptInstrv32d = true;     
    else if( fsgnj_d    && ( MI->getOpcode() == RISCV::FSGNJ_D    ) ) encryptInstrv32d = true;     
    else if( fsgnjn_d   && ( MI->getOpcode() == RISCV::FSGNJN_D   ) ) encryptInstrv32d = true;         
    else if( fsgnjx_d   && ( MI->getOpcode() == RISCV::FSGNJX_D   ) ) encryptInstrv32d = true;         
    else if( fmin_d     && ( MI->getOpcode() == RISCV::FMIN_D     ) ) encryptInstrv32d = true;   
    else if( fmax_d     && ( MI->getOpcode() == RISCV::FMAX_D     ) ) encryptInstrv32d = true;   
    else if( fcvt_s_d   && ( MI->getOpcode() == RISCV::FCVT_S_D   ) ) encryptInstrv32d = true;         
    else if( fcvt_d_s   && ( MI->getOpcode() == RISCV::FCVT_D_S   ) ) encryptInstrv32d = true;         
    else if( feq_d      && ( MI->getOpcode() == RISCV::FEQ_D      ) ) encryptInstrv32d = true; 
    else if( flt_d      && ( MI->getOpcode() == RISCV::FLT_D      ) ) encryptInstrv32d = true; 
    else if( fle_d      && ( MI->getOpcode() == RISCV::FLE_D      ) ) encryptInstrv32d = true; 
    else if( fclass_d   && ( MI->getOpcode() == RISCV::FCLASS_D   ) ) encryptInstrv32d = true;         
    else if( fcvt_w_d   && ( MI->getOpcode() == RISCV::FCVT_W_D   ) ) encryptInstrv32d = true;         
    else if( fcvt_wu_d  && ( MI->getOpcode() == RISCV::FCVT_WU_D  ) ) encryptInstrv32d = true;         
    else if( fcvt_d_w   && ( MI->getOpcode() == RISCV::FCVT_D_W   ) ) encryptInstrv32d = true;       
    else if( fcvt_d_wu  && ( MI->getOpcode() == RISCV::FCVT_D_WU  ) ) encryptInstrv32d = true;         

    else if( fcvt_l_d   && ( MI->getOpcode() == RISCV::FCVT_L_D   ) ) encryptInstrv64d = true;       
    else if( fcvt_lu_d  && ( MI->getOpcode() == RISCV::FCVT_LU_D  ) ) encryptInstrv64d = true;         
    else if( fmv_x_d    && ( MI->getOpcode() == RISCV::FMV_X_D    ) ) encryptInstrv64d = true;     
    else if( fcvt_d_l   && ( MI->getOpcode() == RISCV::FCVT_D_L   ) ) encryptInstrv64d = true;       
    else if( fcvt_d_lu  && ( MI->getOpcode() == RISCV::FCVT_D_LU  ) ) encryptInstrv64d = true;         
    else if( fmv_d_x    && ( MI->getOpcode() == RISCV::FMV_D_X    ) ) encryptInstrv64d = true;     


    else if( c_addi4spn && ( MI->getOpcode() == RISCV::C_ADDI4SPN ) ) encryptInstrvcq0 = true;           
    else if( c_fld      && ( MI->getOpcode() == RISCV::C_FLD      ) ) encryptInstrvcq0 = true; 
    //else if( c_lq       && ( MI->getOpcode() == RISCV::C_LQ       ) ) encryptInstrvcq0 = true; 
    else if( c_lw       && ( MI->getOpcode() == RISCV::C_LW       ) ) encryptInstrvcq0 = true;
    else if( c_flw      && ( MI->getOpcode() == RISCV::C_FLW      ) ) encryptInstrvcq0 = true; 
    else if( c_ld       && ( MI->getOpcode() == RISCV::C_LD       ) ) encryptInstrvcq0 = true;
    else if( c_fsd      && ( MI->getOpcode() == RISCV::C_FSD      ) ) encryptInstrvcq0 = true;   
    //else if( c_sq       && ( MI->getOpcode() == RISCV::C_SQ       ) ) encryptInstrvcq0 = true;
    else if( c_sw       && ( MI->getOpcode() == RISCV::C_SW       ) ) encryptInstrvcq0 = true;
    else if( c_fsw      && ( MI->getOpcode() == RISCV::C_FSW      ) ) encryptInstrvcq0 = true;   
    else if( c_sd       && ( MI->getOpcode() == RISCV::C_SD       ) ) encryptInstrvcq0 = true;

    else if( c_nop      && ( MI->getOpcode() == RISCV::C_NOP      ) ) encryptInstrvcq1 = true;   
    else if( c_addi     && ( MI->getOpcode() == RISCV::C_ADDI     ) ) encryptInstrvcq1 = true;   
    else if( c_jal      && ( MI->getOpcode() == RISCV::C_JAL      ) ) encryptInstrvcq1 = true;   
    else if( c_addiw    && ( MI->getOpcode() == RISCV::C_ADDIW    ) ) encryptInstrvcq1 = true;       
    else if( c_li       && ( MI->getOpcode() == RISCV::C_LI       ) ) encryptInstrvcq1 = true;
    else if( c_addi16sp && ( MI->getOpcode() == RISCV::C_ADDI16SP ) ) encryptInstrvcq1 = true;           
    else if( c_lui      && ( MI->getOpcode() == RISCV::C_LUI      ) ) encryptInstrvcq1 = true; 
    else if( c_srli     && ( MI->getOpcode() == RISCV::C_SRLI     ) ) encryptInstrvcq1 = true;   
    //else if( c_srli64   && ( MI->getOpcode() == RISCV::C_SRLI64   ) ) encryptInstrvcq1 = true;       
    else if( c_srai     && ( MI->getOpcode() == RISCV::C_SRAI     ) ) encryptInstrvcq1 = true;   
    //else if( c_srai64   && ( MI->getOpcode() == RISCV::C_SRAI64   ) ) encryptInstrvcq1 = true;       
    else if( c_andi     && ( MI->getOpcode() == RISCV::C_ANDI     ) ) encryptInstrvcq1 = true;   
    else if( c_sub      && ( MI->getOpcode() == RISCV::C_SUB      ) ) encryptInstrvcq1 = true; 
    else if( c_xor      && ( MI->getOpcode() == RISCV::C_XOR      ) ) encryptInstrvcq1 = true; 
    else if( c_or       && ( MI->getOpcode() == RISCV::C_OR       ) ) encryptInstrvcq1 = true;
    else if( c_and      && ( MI->getOpcode() == RISCV::C_AND      ) ) encryptInstrvcq1 = true; 
    else if( c_subw     && ( MI->getOpcode() == RISCV::C_SUBW     ) ) encryptInstrvcq1 = true;   
    else if( c_addw     && ( MI->getOpcode() == RISCV::C_ADDW     ) ) encryptInstrvcq1 = true;   
    else if( c_j        && ( MI->getOpcode() == RISCV::C_J        ) ) encryptInstrvcq1 = true;
    else if( c_beqz     && ( MI->getOpcode() == RISCV::C_BEQZ     ) ) encryptInstrvcq1 = true;   
    else if( c_bnez     && ( MI->getOpcode() == RISCV::C_BNEZ     ) ) encryptInstrvcq1 = true;   

    else if( c_slli     && ( MI->getOpcode() == RISCV::C_SLLI     ) ) encryptInstrvcq2 = true;   
    //else if( c_slli64   && ( MI->getOpcode() == RISCV::C_SLLI64   ) ) encryptInstrvcq2 = true;       
    else if( c_fldsp    && ( MI->getOpcode() == RISCV::C_FLDSP    ) ) encryptInstrvcq2 = true;     
    //else if( c_lqsp     && ( MI->getOpcode() == RISCV::C_LQSP     ) ) encryptInstrvcq2 = true;   
    else if( c_lwsp     && ( MI->getOpcode() == RISCV::C_LWSP     ) ) encryptInstrvcq2 = true;   
    else if( c_flwsp    && ( MI->getOpcode() == RISCV::C_FLWSP    ) ) encryptInstrvcq2 = true;     
    else if( c_ldsp     && ( MI->getOpcode() == RISCV::C_LDSP     ) ) encryptInstrvcq2 = true;   
    else if( c_jr       && ( MI->getOpcode() == RISCV::C_JR       ) ) encryptInstrvcq2 = true;
    else if( c_mv       && ( MI->getOpcode() == RISCV::C_MV       ) ) encryptInstrvcq2 = true;
    else if( c_ebreak   && ( MI->getOpcode() == RISCV::C_EBREAK   ) ) encryptInstrvcq2 = true;       
    else if( c_jalr     && ( MI->getOpcode() == RISCV::C_JALR     ) ) encryptInstrvcq2 = true;   
    else if( c_add      && ( MI->getOpcode() == RISCV::C_ADD      ) ) encryptInstrvcq2 = true; 
    else if( c_fsdsp    && ( MI->getOpcode() == RISCV::C_FSDSP    ) ) encryptInstrvcq2 = true;     
    //else if( c_sqsp     && ( MI->getOpcode() == RISCV::C_SQSP     ) ) encryptInstrvcq2 = true;   
    else if( c_swsp     && ( MI->getOpcode() == RISCV::C_SWSP     ) ) encryptInstrvcq2 = true;   
    else if( c_fswsp    && ( MI->getOpcode() == RISCV::C_FSWSP    ) ) encryptInstrvcq2 = true;     
    else if( c_sdsp     && ( MI->getOpcode() == RISCV::C_SDSP     ) ) encryptInstrvcq2 = true;   


    ///////////////////////////////// partial
    if     ( b_p_lui        != "" && ( MI->getOpcode() == RISCV::LUI        ) ) encryptInstParKey = b_p_lui        ;
    else if( b_p_auipc      != "" && ( MI->getOpcode() == RISCV::AUIPC      ) ) encryptInstParKey = b_p_auipc      ;
    else if( b_p_jal        != "" && ( MI->getOpcode() == RISCV::JAL        ) ) encryptInstParKey = b_p_jal        ;
    else if( b_p_jalr       != "" && ( MI->getOpcode() == RISCV::JALR       ) ) encryptInstParKey = b_p_jalr       ;
    else if( b_p_beq        != "" && ( MI->getOpcode() == RISCV::BEQ        ) ) encryptInstParKey = b_p_beq        ;
    else if( b_p_bne        != "" && ( MI->getOpcode() == RISCV::BNE        ) ) encryptInstParKey = b_p_bne        ;
    else if( b_p_blt        != "" && ( MI->getOpcode() == RISCV::BLT        ) ) encryptInstParKey = b_p_blt        ;
    else if( b_p_bge        != "" && ( MI->getOpcode() == RISCV::BGE        ) ) encryptInstParKey = b_p_bge        ;
    else if( b_p_bltu       != "" && ( MI->getOpcode() == RISCV::BLTU       ) ) encryptInstParKey = b_p_bltu       ;
    else if( b_p_bgeu       != "" && ( MI->getOpcode() == RISCV::BGEU       ) ) encryptInstParKey = b_p_bgeu       ;
    else if( b_p_lb         != "" && ( MI->getOpcode() == RISCV::LB         ) ) encryptInstParKey = b_p_lb         ;
    else if( b_p_lh         != "" && ( MI->getOpcode() == RISCV::LH         ) ) encryptInstParKey = b_p_lh         ;
    else if( b_p_lw         != "" && ( MI->getOpcode() == RISCV::LW         ) ) encryptInstParKey = b_p_lw         ;
    else if( b_p_lbu        != "" && ( MI->getOpcode() == RISCV::LBU        ) ) encryptInstParKey = b_p_lbu        ;
    else if( b_p_lhu        != "" && ( MI->getOpcode() == RISCV::LHU        ) ) encryptInstParKey = b_p_lhu        ;
    else if( b_p_sb         != "" && ( MI->getOpcode() == RISCV::SB         ) ) encryptInstParKey = b_p_sb         ;
    else if( b_p_sh         != "" && ( MI->getOpcode() == RISCV::SH         ) ) encryptInstParKey = b_p_sh         ;
    else if( b_p_sw         != "" && ( MI->getOpcode() == RISCV::SW         ) ) encryptInstParKey = b_p_sw         ;
    else if( b_p_addi       != "" && ( MI->getOpcode() == RISCV::ADDI       ) ) encryptInstParKey = b_p_addi       ;
    else if( b_p_slti       != "" && ( MI->getOpcode() == RISCV::SLTI       ) ) encryptInstParKey = b_p_slti       ;
    else if( b_p_sltiu      != "" && ( MI->getOpcode() == RISCV::SLTIU      ) ) encryptInstParKey = b_p_sltiu      ;
    else if( b_p_xori       != "" && ( MI->getOpcode() == RISCV::XORI       ) ) encryptInstParKey = b_p_xori       ;
    else if( b_p_ori        != "" && ( MI->getOpcode() == RISCV::ORI        ) ) encryptInstParKey = b_p_ori        ;
    else if( b_p_andi       != "" && ( MI->getOpcode() == RISCV::ANDI       ) ) encryptInstParKey = b_p_andi       ;
    else if( b_p_slli       != "" && ( MI->getOpcode() == RISCV::SLLI       ) ) encryptInstParKey = b_p_slli       ;
    else if( b_p_srli       != "" && ( MI->getOpcode() == RISCV::SRLI       ) ) encryptInstParKey = b_p_srli       ;
    else if( b_p_srai       != "" && ( MI->getOpcode() == RISCV::SRAI       ) ) encryptInstParKey = b_p_srai       ;
    else if( b_p_add        != "" && ( MI->getOpcode() == RISCV::ADD        ) ) encryptInstParKey = b_p_add        ;
    else if( b_p_sub        != "" && ( MI->getOpcode() == RISCV::SUB        ) ) encryptInstParKey = b_p_sub        ;
    else if( b_p_sll        != "" && ( MI->getOpcode() == RISCV::SLL        ) ) encryptInstParKey = b_p_sll        ;
    else if( b_p_slt        != "" && ( MI->getOpcode() == RISCV::SLT        ) ) encryptInstParKey = b_p_slt        ;
    else if( b_p_sltu       != "" && ( MI->getOpcode() == RISCV::SLTU       ) ) encryptInstParKey = b_p_sltu       ;
    else if( b_p_xor_       != "" && ( MI->getOpcode() == RISCV::XOR        ) ) encryptInstParKey = b_p_xor_       ;
    else if( b_p_srl        != "" && ( MI->getOpcode() == RISCV::SRL        ) ) encryptInstParKey = b_p_srl        ;
    else if( b_p_sra        != "" && ( MI->getOpcode() == RISCV::SRA        ) ) encryptInstParKey = b_p_sra        ;
    else if( b_p_or_        != "" && ( MI->getOpcode() == RISCV::OR         ) ) encryptInstParKey = b_p_or_        ;
    else if( b_p_and_       != "" && ( MI->getOpcode() == RISCV::AND        ) ) encryptInstParKey = b_p_and_       ;
    else if( b_p_fence      != "" && ( MI->getOpcode() == RISCV::FENCE      ) ) encryptInstParKey = b_p_fence      ;
    else if( b_p_fence_i    != "" && ( MI->getOpcode() == RISCV::FENCE_I    ) ) encryptInstParKey = b_p_fence_i    ;
    else if( b_p_ecall      != "" && ( MI->getOpcode() == RISCV::ECALL      ) ) encryptInstParKey = b_p_ecall      ;
    else if( b_p_ebreak     != "" && ( MI->getOpcode() == RISCV::EBREAK     ) ) encryptInstParKey = b_p_ebreak     ;
    else if( b_p_csrrw      != "" && ( MI->getOpcode() == RISCV::CSRRW      ) ) encryptInstParKey = b_p_csrrw      ;
    else if( b_p_csrrs      != "" && ( MI->getOpcode() == RISCV::CSRRS      ) ) encryptInstParKey = b_p_csrrs      ;
    else if( b_p_csrrc      != "" && ( MI->getOpcode() == RISCV::CSRRC      ) ) encryptInstParKey = b_p_csrrc      ;
    else if( b_p_csrrwi     != "" && ( MI->getOpcode() == RISCV::CSRRWI     ) ) encryptInstParKey = b_p_csrrwi     ;
    else if( b_p_csrrsi     != "" && ( MI->getOpcode() == RISCV::CSRRSI     ) ) encryptInstParKey = b_p_csrrsi     ;
    else if( b_p_csrrci     != "" && ( MI->getOpcode() == RISCV::CSRRCI     ) ) encryptInstParKey = b_p_csrrci     ;

    else if( b_p_lwu        != "" && ( MI->getOpcode() == RISCV::LWU        ) ) encryptInstParKey = b_p_lwu        ;
    else if( b_p_ld         != "" && ( MI->getOpcode() == RISCV::LD         ) ) encryptInstParKey = b_p_ld         ;
    else if( b_p_sd         != "" && ( MI->getOpcode() == RISCV::SD         ) ) encryptInstParKey = b_p_sd         ;
    //else if( b_p_slli       != "" && ( MI->getOpcode() == RISCV::SLLI       ) ) encryptInstParKey = b_p_slli       ;
    //else if( b_p_srli       != "" && ( MI->getOpcode() == RISCV::SRLI       ) ) encryptInstParKey = b_p_srli       ;
    //else if( b_p_srai       != "" && ( MI->getOpcode() == RISCV::SRAI       ) ) encryptInstParKey = b_p_srai       ;
    else if( b_p_addiw      != "" && ( MI->getOpcode() == RISCV::ADDIW      ) ) encryptInstParKey = b_p_addiw      ;
    else if( b_p_slliw      != "" && ( MI->getOpcode() == RISCV::SLLIW      ) ) encryptInstParKey = b_p_slliw      ;
    else if( b_p_srliw      != "" && ( MI->getOpcode() == RISCV::SRLIW      ) ) encryptInstParKey = b_p_srliw      ;
    else if( b_p_sraiw      != "" && ( MI->getOpcode() == RISCV::SRAIW      ) ) encryptInstParKey = b_p_sraiw      ;
    else if( b_p_addw       != "" && ( MI->getOpcode() == RISCV::ADDW       ) ) encryptInstParKey = b_p_addw       ;
    else if( b_p_subw       != "" && ( MI->getOpcode() == RISCV::SUBW       ) ) encryptInstParKey = b_p_subw       ;
    else if( b_p_sllw       != "" && ( MI->getOpcode() == RISCV::SLLW       ) ) encryptInstParKey = b_p_sllw       ;
    else if( b_p_srlw       != "" && ( MI->getOpcode() == RISCV::SRLW       ) ) encryptInstParKey = b_p_srlw       ;
    else if( b_p_sraw       != "" && ( MI->getOpcode() == RISCV::SRAW       ) ) encryptInstParKey = b_p_sraw       ;

    else if( b_p_mul        != "" && ( MI->getOpcode() == RISCV::MUL        ) ) encryptInstParKey = b_p_mul        ;
    else if( b_p_mulh       != "" && ( MI->getOpcode() == RISCV::MULH       ) ) encryptInstParKey = b_p_mulh       ;
    else if( b_p_mulhsu     != "" && ( MI->getOpcode() == RISCV::MULHSU     ) ) encryptInstParKey = b_p_mulhsu     ;
    else if( b_p_mulhu      != "" && ( MI->getOpcode() == RISCV::MULHU      ) ) encryptInstParKey = b_p_mulhu      ;
    else if( b_p_div_       != "" && ( MI->getOpcode() == RISCV::DIV        ) ) encryptInstParKey = b_p_div_       ;
    else if( b_p_divu       != "" && ( MI->getOpcode() == RISCV::DIVU       ) ) encryptInstParKey = b_p_divu       ;
    else if( b_p_rem        != "" && ( MI->getOpcode() == RISCV::REM        ) ) encryptInstParKey = b_p_rem        ;
    else if( b_p_remu       != "" && ( MI->getOpcode() == RISCV::REMU       ) ) encryptInstParKey = b_p_remu       ;

    else if( b_p_mulw       != "" && ( MI->getOpcode() == RISCV::MULW       ) ) encryptInstParKey = b_p_mulw       ;
    else if( b_p_divw       != "" && ( MI->getOpcode() == RISCV::DIVW       ) ) encryptInstParKey = b_p_divw       ;
    else if( b_p_divuw      != "" && ( MI->getOpcode() == RISCV::DIVUW      ) ) encryptInstParKey = b_p_divuw      ;
    else if( b_p_remw       != "" && ( MI->getOpcode() == RISCV::REMW       ) ) encryptInstParKey = b_p_remw       ;
    else if( b_p_remuw      != "" && ( MI->getOpcode() == RISCV::REMUW      ) ) encryptInstParKey = b_p_remuw      ;

    else if( b_p_lr_w       != "" && ( MI->getOpcode() == RISCV::LR_W       ) ) encryptInstParKey = b_p_lr_w       ;
    else if( b_p_sc_w       != "" && ( MI->getOpcode() == RISCV::SC_W       ) ) encryptInstParKey = b_p_sc_w       ;
    else if( b_p_amoswap_w  != "" && ( MI->getOpcode() == RISCV::AMOSWAP_W  ) ) encryptInstParKey = b_p_amoswap_w  ;
    else if( b_p_amoadd_w   != "" && ( MI->getOpcode() == RISCV::AMOADD_W   ) ) encryptInstParKey = b_p_amoadd_w   ;
    else if( b_p_amoxor_w   != "" && ( MI->getOpcode() == RISCV::AMOXOR_W   ) ) encryptInstParKey = b_p_amoxor_w   ;
    else if( b_p_amoand_w   != "" && ( MI->getOpcode() == RISCV::AMOAND_W   ) ) encryptInstParKey = b_p_amoand_w   ;
    else if( b_p_amoor_w    != "" && ( MI->getOpcode() == RISCV::AMOOR_W    ) ) encryptInstParKey = b_p_amoor_w    ;
    else if( b_p_amomin_w   != "" && ( MI->getOpcode() == RISCV::AMOMIN_W   ) ) encryptInstParKey = b_p_amomin_w   ;
    else if( b_p_amomax_w   != "" && ( MI->getOpcode() == RISCV::AMOMAX_W   ) ) encryptInstParKey = b_p_amomax_w   ;
    else if( b_p_amominu_w  != "" && ( MI->getOpcode() == RISCV::AMOMINU_W  ) ) encryptInstParKey = b_p_amominu_w  ;
    else if( b_p_amomaxu_w  != "" && ( MI->getOpcode() == RISCV::AMOMAXU_W  ) ) encryptInstParKey = b_p_amomaxu_w  ;

    else if( b_p_lr_d       != "" && ( MI->getOpcode() == RISCV::LR_D       ) ) encryptInstParKey = b_p_lr_d       ;
    else if( b_p_sc_d       != "" && ( MI->getOpcode() == RISCV::SC_D       ) ) encryptInstParKey = b_p_sc_d       ;
    else if( b_p_amoswap_d  != "" && ( MI->getOpcode() == RISCV::AMOSWAP_D  ) ) encryptInstParKey = b_p_amoswap_d  ;
    else if( b_p_amoadd_d   != "" && ( MI->getOpcode() == RISCV::AMOADD_D   ) ) encryptInstParKey = b_p_amoadd_d   ;
    else if( b_p_amoxor_d   != "" && ( MI->getOpcode() == RISCV::AMOXOR_D   ) ) encryptInstParKey = b_p_amoxor_d   ;
    else if( b_p_amoand_d   != "" && ( MI->getOpcode() == RISCV::AMOAND_D   ) ) encryptInstParKey = b_p_amoand_d   ;
    else if( b_p_amoor_d    != "" && ( MI->getOpcode() == RISCV::AMOOR_D    ) ) encryptInstParKey = b_p_amoor_d    ;
    else if( b_p_amomin_d   != "" && ( MI->getOpcode() == RISCV::AMOMIN_D   ) ) encryptInstParKey = b_p_amomin_d   ;
    else if( b_p_amomax_d   != "" && ( MI->getOpcode() == RISCV::AMOMAX_D   ) ) encryptInstParKey = b_p_amomax_d   ;
    else if( b_p_amominu_d  != "" && ( MI->getOpcode() == RISCV::AMOMINU_D  ) ) encryptInstParKey = b_p_amominu_d  ;
    else if( b_p_amomaxu_d  != "" && ( MI->getOpcode() == RISCV::AMOMAXU_D  ) ) encryptInstParKey = b_p_amomaxu_d  ;

    else if( b_p_flw        != "" && ( MI->getOpcode() == RISCV::FLW        ) ) encryptInstParKey = b_p_flw        ;
    else if( b_p_fsw        != "" && ( MI->getOpcode() == RISCV::FSW        ) ) encryptInstParKey = b_p_fsw        ;
    else if( b_p_fmadd_s    != "" && ( MI->getOpcode() == RISCV::FMADD_S    ) ) encryptInstParKey = b_p_fmadd_s    ;
    else if( b_p_fmsub_s    != "" && ( MI->getOpcode() == RISCV::FMSUB_S    ) ) encryptInstParKey = b_p_fmsub_s    ;
    else if( b_p_fnmsub_s   != "" && ( MI->getOpcode() == RISCV::FNMSUB_S   ) ) encryptInstParKey = b_p_fnmsub_s   ;
    else if( b_p_fnmadd_s   != "" && ( MI->getOpcode() == RISCV::FNMADD_S   ) ) encryptInstParKey = b_p_fnmadd_s   ;
    else if( b_p_fadd_s     != "" && ( MI->getOpcode() == RISCV::FADD_S     ) ) encryptInstParKey = b_p_fadd_s     ;
    else if( b_p_fsub_s     != "" && ( MI->getOpcode() == RISCV::FSUB_S     ) ) encryptInstParKey = b_p_fsub_s     ;
    else if( b_p_fmul_s     != "" && ( MI->getOpcode() == RISCV::FMUL_S     ) ) encryptInstParKey = b_p_fmul_s     ;
    else if( b_p_fdiv_s     != "" && ( MI->getOpcode() == RISCV::FDIV_S     ) ) encryptInstParKey = b_p_fdiv_s     ;
    else if( b_p_fsqrt_s    != "" && ( MI->getOpcode() == RISCV::FSQRT_S    ) ) encryptInstParKey = b_p_fsqrt_s    ;
    else if( b_p_fsgnj_s    != "" && ( MI->getOpcode() == RISCV::FSGNJ_S    ) ) encryptInstParKey = b_p_fsgnj_s    ;
    else if( b_p_fsgnjn_s   != "" && ( MI->getOpcode() == RISCV::FSGNJN_S   ) ) encryptInstParKey = b_p_fsgnjn_s   ;
    else if( b_p_fsgnjx_s   != "" && ( MI->getOpcode() == RISCV::FSGNJX_S   ) ) encryptInstParKey = b_p_fsgnjx_s   ;
    else if( b_p_fmin_s     != "" && ( MI->getOpcode() == RISCV::FMIN_S     ) ) encryptInstParKey = b_p_fmin_s     ;
    else if( b_p_fmax_s     != "" && ( MI->getOpcode() == RISCV::FMAX_S     ) ) encryptInstParKey = b_p_fmax_s     ;
    else if( b_p_fcvt_w_s   != "" && ( MI->getOpcode() == RISCV::FCVT_W_S   ) ) encryptInstParKey = b_p_fcvt_w_s   ;
    else if( b_p_fcvt_wu_s  != "" && ( MI->getOpcode() == RISCV::FCVT_WU_S  ) ) encryptInstParKey = b_p_fcvt_wu_s  ;
    else if( b_p_fmv_x_w    != "" && ( MI->getOpcode() == RISCV::FMV_X_W    ) ) encryptInstParKey = b_p_fmv_x_w    ;
    else if( b_p_feq_s      != "" && ( MI->getOpcode() == RISCV::FEQ_S      ) ) encryptInstParKey = b_p_feq_s      ;
    else if( b_p_flt_s      != "" && ( MI->getOpcode() == RISCV::FLT_S      ) ) encryptInstParKey = b_p_flt_s      ;
    else if( b_p_fle_s      != "" && ( MI->getOpcode() == RISCV::FLE_S      ) ) encryptInstParKey = b_p_fle_s      ;
    else if( b_p_fclass_s   != "" && ( MI->getOpcode() == RISCV::FCLASS_S   ) ) encryptInstParKey = b_p_fclass_s   ;
    else if( b_p_fcvt_s_w   != "" && ( MI->getOpcode() == RISCV::FCVT_S_W   ) ) encryptInstParKey = b_p_fcvt_s_w   ;
    else if( b_p_fcvt_s_wu  != "" && ( MI->getOpcode() == RISCV::FCVT_S_WU  ) ) encryptInstParKey = b_p_fcvt_s_wu  ;
    else if( b_p_fmv_w_x    != "" && ( MI->getOpcode() == RISCV::FMV_W_X    ) ) encryptInstParKey = b_p_fmv_w_x    ;

    else if( b_p_fcvt_l_s   != "" && ( MI->getOpcode() == RISCV::FCVT_L_S   ) ) encryptInstParKey = b_p_fcvt_l_s   ;
    else if( b_p_fcvt_lu_s  != "" && ( MI->getOpcode() == RISCV::FCVT_LU_S  ) ) encryptInstParKey = b_p_fcvt_lu_s  ;
    else if( b_p_fcvt_s_l   != "" && ( MI->getOpcode() == RISCV::FCVT_S_L   ) ) encryptInstParKey = b_p_fcvt_s_l   ;
    else if( b_p_fcvt_s_lu  != "" && ( MI->getOpcode() == RISCV::FCVT_S_LU  ) ) encryptInstParKey = b_p_fcvt_s_lu  ;

    else if( b_p_fld        != "" && ( MI->getOpcode() == RISCV::FLD        ) ) encryptInstParKey = b_p_fld        ;
    else if( b_p_fsd        != "" && ( MI->getOpcode() == RISCV::FSD        ) ) encryptInstParKey = b_p_fsd        ;
    else if( b_p_fmadd_d    != "" && ( MI->getOpcode() == RISCV::FMADD_D    ) ) encryptInstParKey = b_p_fmadd_d    ;
    else if( b_p_fmsub_d    != "" && ( MI->getOpcode() == RISCV::FMSUB_D    ) ) encryptInstParKey = b_p_fmsub_d    ;
    else if( b_p_fnmsub_d   != "" && ( MI->getOpcode() == RISCV::FNMSUB_D   ) ) encryptInstParKey = b_p_fnmsub_d   ;
    else if( b_p_fnmadd_d   != "" && ( MI->getOpcode() == RISCV::FNMADD_D   ) ) encryptInstParKey = b_p_fnmadd_d   ;
    else if( b_p_fadd_d     != "" && ( MI->getOpcode() == RISCV::FADD_D     ) ) encryptInstParKey = b_p_fadd_d     ;
    else if( b_p_fsub_d     != "" && ( MI->getOpcode() == RISCV::FSUB_D     ) ) encryptInstParKey = b_p_fsub_d     ;
    else if( b_p_fmul_d     != "" && ( MI->getOpcode() == RISCV::FMUL_D     ) ) encryptInstParKey = b_p_fmul_d     ;
    else if( b_p_fdiv_d     != "" && ( MI->getOpcode() == RISCV::FDIV_D     ) ) encryptInstParKey = b_p_fdiv_d     ;
    else if( b_p_fsqrt_d    != "" && ( MI->getOpcode() == RISCV::FSQRT_D    ) ) encryptInstParKey = b_p_fsqrt_d    ;
    else if( b_p_fsgnj_d    != "" && ( MI->getOpcode() == RISCV::FSGNJ_D    ) ) encryptInstParKey = b_p_fsgnj_d    ;
    else if( b_p_fsgnjn_d   != "" && ( MI->getOpcode() == RISCV::FSGNJN_D   ) ) encryptInstParKey = b_p_fsgnjn_d   ;
    else if( b_p_fsgnjx_d   != "" && ( MI->getOpcode() == RISCV::FSGNJX_D   ) ) encryptInstParKey = b_p_fsgnjx_d   ;
    else if( b_p_fmin_d     != "" && ( MI->getOpcode() == RISCV::FMIN_D     ) ) encryptInstParKey = b_p_fmin_d     ;
    else if( b_p_fmax_d     != "" && ( MI->getOpcode() == RISCV::FMAX_D     ) ) encryptInstParKey = b_p_fmax_d     ;
    else if( b_p_fcvt_s_d   != "" && ( MI->getOpcode() == RISCV::FCVT_S_D   ) ) encryptInstParKey = b_p_fcvt_s_d   ;
    else if( b_p_fcvt_d_s   != "" && ( MI->getOpcode() == RISCV::FCVT_D_S   ) ) encryptInstParKey = b_p_fcvt_d_s   ;
    else if( b_p_feq_d      != "" && ( MI->getOpcode() == RISCV::FEQ_D      ) ) encryptInstParKey = b_p_feq_d      ;
    else if( b_p_flt_d      != "" && ( MI->getOpcode() == RISCV::FLT_D      ) ) encryptInstParKey = b_p_flt_d      ;
    else if( b_p_fle_d      != "" && ( MI->getOpcode() == RISCV::FLE_D      ) ) encryptInstParKey = b_p_fle_d      ;
    else if( b_p_fclass_d   != "" && ( MI->getOpcode() == RISCV::FCLASS_D   ) ) encryptInstParKey = b_p_fclass_d   ;
    else if( b_p_fcvt_w_d   != "" && ( MI->getOpcode() == RISCV::FCVT_W_D   ) ) encryptInstParKey = b_p_fcvt_w_d   ;
    else if( b_p_fcvt_wu_d  != "" && ( MI->getOpcode() == RISCV::FCVT_WU_D  ) ) encryptInstParKey = b_p_fcvt_wu_d  ;
    else if( b_p_fcvt_d_w   != "" && ( MI->getOpcode() == RISCV::FCVT_D_W   ) ) encryptInstParKey = b_p_fcvt_d_w   ;
    else if( b_p_fcvt_d_wu  != "" && ( MI->getOpcode() == RISCV::FCVT_D_WU  ) ) encryptInstParKey = b_p_fcvt_d_wu  ;

    else if( b_p_fcvt_l_d   != "" && ( MI->getOpcode() == RISCV::FCVT_L_D   ) ) encryptInstParKey = b_p_fcvt_l_d   ;
    else if( b_p_fcvt_lu_d  != "" && ( MI->getOpcode() == RISCV::FCVT_LU_D  ) ) encryptInstParKey = b_p_fcvt_lu_d  ;
    else if( b_p_fmv_x_d    != "" && ( MI->getOpcode() == RISCV::FMV_X_D    ) ) encryptInstParKey = b_p_fmv_x_d    ;
    else if( b_p_fcvt_d_l   != "" && ( MI->getOpcode() == RISCV::FCVT_D_L   ) ) encryptInstParKey = b_p_fcvt_d_l   ;
    else if( b_p_fcvt_d_lu  != "" && ( MI->getOpcode() == RISCV::FCVT_D_LU  ) ) encryptInstParKey = b_p_fcvt_d_lu  ;
    else if( b_p_fmv_d_x    != "" && ( MI->getOpcode() == RISCV::FMV_D_X    ) ) encryptInstParKey = b_p_fmv_d_x    ;

    else if( b_p_c_addi4spn != "" && ( MI->getOpcode() == RISCV::C_ADDI4SPN ) ) encryptInstParKey = b_p_c_addi4spn ;
    else if( b_p_c_fld      != "" && ( MI->getOpcode() == RISCV::C_FLD      ) ) encryptInstParKey = b_p_c_fld      ;
    //else if( b_p_c_lq       != "" && ( MI->getOpcode() == RISCV::C_LQ       ) ) encryptInstParKey = b_p_c_lq       ; //not supported
    else if( b_p_c_lw       != "" && ( MI->getOpcode() == RISCV::C_LW       ) ) encryptInstParKey = b_p_c_lw       ;
    else if( b_p_c_flw      != "" && ( MI->getOpcode() == RISCV::C_FLW      ) ) encryptInstParKey = b_p_c_flw      ;
    else if( b_p_c_ld       != "" && ( MI->getOpcode() == RISCV::C_LD       ) ) encryptInstParKey = b_p_c_ld       ;
    else if( b_p_c_fsd      != "" && ( MI->getOpcode() == RISCV::C_FSD      ) ) encryptInstParKey = b_p_c_fsd      ;
    //else if( b_p_c_sq       != "" && ( MI->getOpcode() == RISCV::C_SQ       ) ) encryptInstParKey = b_p_c_sq       ; //not supported
    else if( b_p_c_sw       != "" && ( MI->getOpcode() == RISCV::C_SW       ) ) encryptInstParKey = b_p_c_sw       ;
    else if( b_p_c_fsw      != "" && ( MI->getOpcode() == RISCV::C_FSW      ) ) encryptInstParKey = b_p_c_fsw      ;
    else if( b_p_c_sd       != "" && ( MI->getOpcode() == RISCV::C_SD       ) ) encryptInstParKey = b_p_c_sd       ;

    else if( b_p_c_nop      != "" && ( MI->getOpcode() == RISCV::C_NOP      ) ) encryptInstParKey = b_p_c_nop      ;
    else if( b_p_c_addi     != "" && ( MI->getOpcode() == RISCV::C_ADDI     ) ) encryptInstParKey = b_p_c_addi     ;
    else if( b_p_c_jal      != "" && ( MI->getOpcode() == RISCV::C_JAL      ) ) encryptInstParKey = b_p_c_jal      ;
    else if( b_p_c_addiw    != "" && ( MI->getOpcode() == RISCV::C_ADDIW    ) ) encryptInstParKey = b_p_c_addiw    ;
    else if( b_p_c_li       != "" && ( MI->getOpcode() == RISCV::C_LI       ) ) encryptInstParKey = b_p_c_li       ;
    else if( b_p_c_addi16sp != "" && ( MI->getOpcode() == RISCV::C_ADDI16SP ) ) encryptInstParKey = b_p_c_addi16sp ;
    else if( b_p_c_lui      != "" && ( MI->getOpcode() == RISCV::C_LUI      ) ) encryptInstParKey = b_p_c_lui      ;
    else if( b_p_c_srli     != "" && ( MI->getOpcode() == RISCV::C_SRLI     ) ) encryptInstParKey = b_p_c_srli     ;
    //else if( b_p_c_srli64   != "" && ( MI->getOpcode() == RISCV::C_SRLI64   ) ) encryptInstParKey = b_p_c_srli64   ; //not supported
    else if( b_p_c_srai     != "" && ( MI->getOpcode() == RISCV::C_SRAI     ) ) encryptInstParKey = b_p_c_srai     ;
    //else if( b_p_c_srai64   != "" && ( MI->getOpcode() == RISCV::C_SRAI64   ) ) encryptInstParKey = b_p_c_srai64   ; //not supported
    else if( b_p_c_andi     != "" && ( MI->getOpcode() == RISCV::C_ANDI     ) ) encryptInstParKey = b_p_c_andi     ;
    else if( b_p_c_sub      != "" && ( MI->getOpcode() == RISCV::C_SUB      ) ) encryptInstParKey = b_p_c_sub      ;
    else if( b_p_c_xor      != "" && ( MI->getOpcode() == RISCV::C_XOR      ) ) encryptInstParKey = b_p_c_xor      ;
    else if( b_p_c_or       != "" && ( MI->getOpcode() == RISCV::C_OR       ) ) encryptInstParKey = b_p_c_or       ;
    else if( b_p_c_and      != "" && ( MI->getOpcode() == RISCV::C_AND      ) ) encryptInstParKey = b_p_c_and      ;
    else if( b_p_c_subw     != "" && ( MI->getOpcode() == RISCV::C_SUBW     ) ) encryptInstParKey = b_p_c_subw     ;
    else if( b_p_c_addw     != "" && ( MI->getOpcode() == RISCV::C_ADDW     ) ) encryptInstParKey = b_p_c_addw     ;
    else if( b_p_c_j        != "" && ( MI->getOpcode() == RISCV::C_J        ) ) encryptInstParKey = b_p_c_j        ;
    else if( b_p_c_beqz     != "" && ( MI->getOpcode() == RISCV::C_BEQZ     ) ) encryptInstParKey = b_p_c_beqz     ;
    else if( b_p_c_bnez     != "" && ( MI->getOpcode() == RISCV::C_BNEZ     ) ) encryptInstParKey = b_p_c_bnez     ;

    else if( b_p_c_slli     != "" && ( MI->getOpcode() == RISCV::C_SLLI     ) ) encryptInstParKey = b_p_c_slli     ;
    //else if( b_p_c_slli64   != "" && ( MI->getOpcode() == RISCV::C_SLLI64   ) ) encryptInstParKey = b_p_c_slli64   ;
    else if( b_p_c_fldsp    != "" && ( MI->getOpcode() == RISCV::C_FLDSP    ) ) encryptInstParKey = b_p_c_fldsp    ;
    //else if( b_p_c_lqsp     != "" && ( MI->getOpcode() == RISCV::C_LQSP     ) ) encryptInstParKey = b_p_c_lqsp     ;
    else if( b_p_c_lwsp     != "" && ( MI->getOpcode() == RISCV::C_LWSP     ) ) encryptInstParKey = b_p_c_lwsp     ;
    else if( b_p_c_flwsp    != "" && ( MI->getOpcode() == RISCV::C_FLWSP    ) ) encryptInstParKey = b_p_c_flwsp    ;
    else if( b_p_c_ldsp     != "" && ( MI->getOpcode() == RISCV::C_LDSP     ) ) encryptInstParKey = b_p_c_ldsp     ;
    else if( b_p_c_jr       != "" && ( MI->getOpcode() == RISCV::C_JR       ) ) encryptInstParKey = b_p_c_jr       ;
    else if( b_p_c_mv       != "" && ( MI->getOpcode() == RISCV::C_MV       ) ) encryptInstParKey = b_p_c_mv       ;
    else if( b_p_c_ebreak   != "" && ( MI->getOpcode() == RISCV::C_EBREAK   ) ) encryptInstParKey = b_p_c_ebreak   ;
    else if( b_p_c_jalr     != "" && ( MI->getOpcode() == RISCV::C_JALR     ) ) encryptInstParKey = b_p_c_jalr     ;
    else if( b_p_c_add      != "" && ( MI->getOpcode() == RISCV::C_ADD      ) ) encryptInstParKey = b_p_c_add      ;
    else if( b_p_c_fsdsp    != "" && ( MI->getOpcode() == RISCV::C_FSDSP    ) ) encryptInstParKey = b_p_c_fsdsp    ;
    //else if( b_p_c_sqsp     != "" && ( MI->getOpcode() == RISCV::C_SQSP     ) ) encryptInstParKey = b_p_c_sqsp     ;
    else if( b_p_c_swsp     != "" && ( MI->getOpcode() == RISCV::C_SWSP     ) ) encryptInstParKey = b_p_c_swsp     ;
    else if( b_p_c_fswsp    != "" && ( MI->getOpcode() == RISCV::C_FSWSP    ) ) encryptInstParKey = b_p_c_fswsp    ;
    else if( b_p_c_sdsp     != "" && ( MI->getOpcode() == RISCV::C_SDSP     ) ) encryptInstParKey = b_p_c_sdsp     ;

    //////////////////////////////////// partial

    portedDump::dumpBytes(Bytes, OS, encryptInstParKey, 
      encryptInstrv32i,
      encryptInstrv32m,
      encryptInstrv32a,
      encryptInstrv32f,
      encryptInstrv32d,
      encryptInstrv32q,

      encryptInstrv64i,
      encryptInstrv64m,
      encryptInstrv64a,
      encryptInstrv64f,
      encryptInstrv64d,
      encryptInstrv64q,

      encryptInstrvcq0,
      encryptInstrvcq1,
      encryptInstrvcq2
      );
  }
};
PrettyPrinter PrettyPrinterInst;

PrettyPrinter &selectPrettyPrinter(Triple const &Triple) {
  return PrettyPrinterInst;
}

} // end of anonymous namespace

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
      outs() << '\n';
      NumBytes = 0;
    }
  }
}

SymbolInfoTy objdump::createSymbolInfo(const ObjectFile *Obj,
                                       const SymbolRef &Symbol) {
  const StringRef FileName = Obj->getFileName();
  const uint64_t Addr = unwrapOrError(Symbol.getAddress(), FileName);
  const StringRef Name = unwrapOrError(Symbol.getName(), FileName);

  return SymbolInfoTy(Addr, Name,
                      Obj->isELF() ? getElfSymbolType(Obj, Symbol)
                                    : (uint8_t)ELF::STT_NOTYPE);
}

static SymbolInfoTy createDummySymbolInfo(const ObjectFile *Obj,
                                          const uint64_t Addr, StringRef &Name,
                                          uint8_t Type) {
  return SymbolInfoTy(Addr, Name, Type);
}

static void disassembleObject(const Target *TheTarget, const ObjectFile *Obj,
                              MCContext &Ctx, MCDisassembler *PrimaryDisAsm,
                              const MCInstrAnalysis *MIA, MCInstPrinter *IP,
                              const MCSubtargetInfo *PrimarySTI,
                              PrettyPrinter &PIP) {
  const MCSubtargetInfo *STI = PrimarySTI;
  MCDisassembler *DisAsm = PrimaryDisAsm;

  std::map<SectionRef, std::vector<RelocationRef>> RelocMap;

  // Create a mapping from virtual address to symbol name.  This is used to
  // pretty print the symbols while disassembling.
  std::map<SectionRef, SectionSymbolsTy> AllSymbols;
  SectionSymbolsTy AbsoluteSymbols;
  const StringRef FileName = Obj->getFileName();

  for (const SymbolRef &Symbol : Obj->symbols()) {
    StringRef Name = unwrapOrError(Symbol.getName(), FileName);
    if (Name.empty())
      continue;

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
  
  for (std::pair<const SectionRef, SectionSymbolsTy> &SecSyms : AllSymbols)
    stable_sort(SecSyms.second);
  stable_sort(AbsoluteSymbols);

  for (const SectionRef &Section : ToolSectionFilter(*Obj)) {
    if (!DisassembleAll &&
        (!Section.isText() || Section.isVirtual()))
      continue;

    uint64_t SectionAddr = Section.getAddress();
    uint64_t SectSize = Section.getSize();
    if (!SectSize)
      continue;

    // Get the list of all the symbols in this section.
    SectionSymbolsTy &Symbols = AllSymbols[Section];


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

    uint64_t Size;
    uint64_t Index;

    std::vector<RelocationRef> Rels = RelocMap[Section];
    std::vector<RelocationRef>::const_iterator RelCur = Rels.begin();
    std::vector<RelocationRef>::const_iterator RelEnd = Rels.end();
    // Disassemble symbol by symbol.
    for (unsigned SI = 0, SE = Symbols.size(); SI != SE; ++SI) {
      std::string SymbolName = Symbols[SI].Name.str();
      if (Demangle)
        SymbolName = demangle(SymbolName);

      uint64_t Start = Symbols[SI].Addr;
      if (Start < SectionAddr || StopAddress <= Start)
        continue;

      // The end is the section end, the beginning of the next symbol, or
      // --stop-address.
      uint64_t End = std::min<uint64_t>(SectionAddr + SectSize, StopAddress);
      if (SI + 1 < SE)
        End = std::min(End, Symbols[SI + 1].Addr);
      if (Start >= End || End <= StartAddress)
        continue;
      Start -= SectionAddr;
      End -= SectionAddr;


      // Don't print raw contents of a virtual section. A virtual section
      // doesn't have any contents in the file.
      if (Section.isVirtual()) {
        continue;
      }

      auto Status = DisAsm->onSymbolStart(Symbols[SI], Size,
                                          Bytes.slice(Start, End - Start),
                                          SectionAddr + Start, CommentStream);
      // To have round trippable disassembly, we fall back to decoding the
      // remaining bytes as instructions.
      //
      // If there is a failure, we disassemble the failed region as bytes before
      // falling back. The target is expected to print nothing in this case.
      //
      // If there is Success or SoftFail i.e no 'real' failure, we go ahead by
      // Size bytes before falling back.
      // So if the entire symbol is 'eaten' by the target:
      //   Start += Size  // Now Start = End and we will never decode as
      //                  // instructions
      //
      // Right now, most targets return None i.e ignore to treat a symbol
      // separately. But WebAssembly decodes preludes for some symbols.
      //
      if (Status.hasValue()) {
        if (Status.getValue() == MCDisassembler::Fail) {
          outs() << "// Error in decoding " << SymbolName
                 << " : Decoding failed region as bytes.\n";
          for (uint64_t I = 0; I < Size; ++I) {
            outs() << "\t.byte\t " << format_hex(Bytes[I], 1, /*Upper=*/true)
                   << "\n";
          }
        }
      } else {
        Size = 0;
      }

      Start += Size;

      Index = Start;
      if (SectionAddr < StartAddress)
        Index = std::max<uint64_t>(Index, StartAddress - SectionAddr);

      // If there is a data/common symbol inside an ELF text section and we are
      // only disassembling text (applicable all architectures), we are in a
      // situation where we must print the data and not disassemble it.
      if (Obj->isELF() && !DisassembleAll && Section.isText()) {
        uint8_t SymTy = Symbols[SI].Type;
        if (SymTy == ELF::STT_OBJECT || SymTy == ELF::STT_COMMON) {
          dumpELFData(SectionAddr, Index, End, Bytes);
          Index = End;
        }
      }

      formatted_raw_ostream FOS(*portedOuts::outs);
      
      while (Index < End) {
        
        // When -z or --disassemble-zeroes are given we always dissasemble
        // them. Otherwise we might want to skip zero bytes we see.
        if (!DisassembleZeroes) {
          uint64_t MaxOffset = End - Index;
          // For --reloc: print zero blocks patched by relocations, so that
          // relocations can be shown in the dump.
          if (RelCur != RelEnd)
            MaxOffset = RelCur->getOffset() - Index;

          if (size_t N =
                  countSkippableZeroBytes(Bytes.slice(Index, MaxOffset))) {
            Index += N;
            continue;
          }
        }

        // Disassemble a real instruction or a data when disassemble all is
        // provided
        MCInst Inst;
        bool Disassembled =
            DisAsm->getInstruction(Inst, Size, Bytes.slice(Index),
                                   SectionAddr + Index, CommentStream);
        if (Size == 0)
          Size = 1;


        //dumpBytes(Bytes.slice(Index, Size), FOS);
        PIP.printInst(
            *IP, Disassembled ? &Inst : nullptr, Bytes.slice(Index, Size),
            {SectionAddr + Index + VMAAdjustment, Section.getIndex()}, FOS,
            "", *STI, Obj->getFileName());

        FOS << "\n";

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

            ++RelCur;
          }
        }

        Index += Size;
      }
    }
  }
}

static void disassembleObject(const ObjectFile *Obj) {
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


  std::unique_ptr<const MCInstrAnalysis> MIA(
      TheTarget->createMCInstrAnalysis(MII.get()));

  int AsmPrinterVariant = AsmInfo->getAssemblerDialect();
  std::unique_ptr<MCInstPrinter> IP(TheTarget->createMCInstPrinter(
      Triple(TripleName), AsmPrinterVariant, *AsmInfo, *MII, *MRI));
  if (!IP)
    reportError(Obj->getFileName(),
                "no instruction printer for target " + TripleName);
  IP->setPrintImmHex(/*PrintImmHex*/false);
  IP->setPrintBranchImmAsAddress(true);

  PrettyPrinter &PIP = selectPrettyPrinter(Triple(TripleName));

  disassembleObject(TheTarget, Obj, Ctx, DisAsm.get(),
                    MIA.get(), IP.get(), STI.get(), PIP);
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
  if (StartAddress.getNumOccurrences() || StopAddress.getNumOccurrences())
    checkForInvalidStartStopAddress(O, StartAddress, StopAddress);


  if (Disassemble)
    disassembleObject(O);
}

/// Open file and figure out how to dump it.
static void dumpInput(StringRef file) {
  // Attempt to open the binary.
  OwningBinary<Binary> OBinary = unwrapOrError(createBinary(file), file);
  Binary &Binary = *OBinary.getBinary();

  if (ObjectFile *O = dyn_cast<ObjectFile>(&Binary))
    dumpObject(O);
  else
    reportError(errorCodeToError(object_error::invalid_file_type), file);
}

int main(int argc, char **argv) {
  using namespace llvm;
  InitLLVM X(argc, argv);
  const cl::OptionCategory *OptionFilters[] = {&ObjdumpCat};
  cl::HideUnrelatedOptions(OptionFilters);

  // Initialize targets and assembly printers/parsers.
  InitializeAllTargetInfos();
  InitializeAllTargetMCs();
  InitializeAllDisassemblers();

  // Register the target printer for --version.
  cl::AddExtraVersionPrinter(TargetRegistry::printRegisteredTargetsForVersion);

  cl::ParseCommandLineOptions(argc, argv, "llvm object file dumper\n", nullptr,
                              /*EnvVar=*/nullptr,
                              /*LongOptionsUseDoubleDash=*/true);

  ToolName = argv[0];

  SmallVector<const char *, 256> argv2(argv, argv + argc);
  std::string fileName = "";
  for(int i=0; i<argv2.size(); i++){
    if(StringRef(argv[i]).endswith(".o"))
      fileName = std::string(StringRef(argv[i]).rtrim(".o"));
  }

  portedOuts::outs = new llvm::raw_fd_ostream(fileName + "out.hex", portedOuts::EC);

  // Defaults to a.out if no filenames specified.
  if (InputFilenames.empty())
    InputFilenames.push_back("a.out");

  if (DisassembleAll)
    Disassemble = true;

  if (!Disassemble) {
    cl::PrintHelpMessage();
    return 2;
  }

  if(instnum != "")
    num = std::stoi(instnum);

  // before hex dump
  
  const int ienc32instsSize = 47;
  const int menc32instsSize = 8;
  const int aenc32instsSize = 11;
  const int fenc32instsSize = 26;
  const int denc32instsSize = 26;
  const int qenc32instsSize = 28;
  
  const int ienc64instsSize = 12; //15;
  const int menc64instsSize = 5;
  const int aenc64instsSize = 11;
  const int fenc64instsSize = 4;
  const int denc64instsSize = 6;
  const int qenc64instsSize = 4;
  
  const int cencq0instsSize = 11;
  const int cencq1instsSize = 21;
  const int cencq2instsSize = 17;
  
  int ienc32instsArr[ienc32instsSize];
  for(int i=ienc32instsSize-1; i>=0; i--){
    ienc32instsArr[i] = 0;
  }
  for(int i=0; i<ienc32insts.length(); i++){
    ienc32instsArr[i] = ienc32insts[i] - '0';
  }
  
  int menc32instsArr[menc32instsSize];
  for(int i=menc32instsSize-1; i>=0; i--){
    menc32instsArr[i] = 0;
  }
  for(int i=0; i<menc32insts.length(); i++){
    menc32instsArr[i] = menc32insts[i] - '0';
  }
  
  int aenc32instsArr[aenc32instsSize];
  for(int i=aenc32instsSize-1; i>=0; i--){
    aenc32instsArr[i] = 0;
  }
  for(int i=0; i<aenc32insts.length(); i++){
    aenc32instsArr[i] = aenc32insts[i] - '0';
  }
  
  int fenc32instsArr[fenc32instsSize];
  for(int i=fenc32instsSize-1; i>=0; i--){
    fenc32instsArr[i] = 0;
  }
  for(int i=0; i<fenc32insts.length(); i++){
    fenc32instsArr[i] = fenc32insts[i] - '0';
  }
  
  int denc32instsArr[denc32instsSize];
  for(int i=denc32instsSize-1; i>=0; i--){
    denc32instsArr[i] = 0;
  }
  for(int i=0; i<denc32insts.length(); i++){
    denc32instsArr[i] = denc32insts[i] - '0';
  }
  
  int qenc32instsArr[qenc32instsSize];
  for(int i=qenc32instsSize-1; i>=0; i--){
    qenc32instsArr[i] = 0;
  }
  for(int i=0; i<qenc32insts.length(); i++){
    qenc32instsArr[i] = qenc32insts[i] - '0';
  }
  
  int ienc64instsArr[ienc64instsSize];
  for(int i=ienc64instsSize-1; i>=0; i--){
    ienc64instsArr[i] = 0;
  }
  for(int i=0; i<ienc64insts.length(); i++){
    ienc64instsArr[i] = ienc64insts[i] - '0';
  }
  
  int menc64instsArr[menc64instsSize];
  for(int i=menc64instsSize-1; i>=0; i--){
    menc64instsArr[i] = 0;
  }
  for(int i=0; i<menc64insts.length(); i++){
    menc64instsArr[i] = menc64insts[i] - '0';
  }
  
  int aenc64instsArr[aenc64instsSize];
  for(int i=aenc64instsSize-1; i>=0; i--){
    aenc64instsArr[i] = 0;
  }
  for(int i=0; i<aenc64insts.length(); i++){
    aenc64instsArr[i] = aenc64insts[i] - '0';
  }
  
  int fenc64instsArr[fenc64instsSize];
  for(int i=fenc64instsSize-1; i>=0; i--){
    fenc64instsArr[i] = 0;
  }
  for(int i=0; i<fenc64insts.length(); i++){
    fenc64instsArr[i] = fenc64insts[i] - '0';
  }
  
  int denc64instsArr[denc64instsSize];
  for(int i=denc64instsSize-1; i>=0; i--){
    denc64instsArr[i] = 0;
  }
  for(int i=0; i<denc64insts.length(); i++){
    denc64instsArr[i] = denc64insts[i] - '0';
  }
  
  int qenc64instsArr[qenc64instsSize];
  for(int i=qenc64instsSize-1; i>=0; i--){
    qenc64instsArr[i] = 0;
  }
  for(int i=0; i<qenc64insts.length(); i++){
    qenc64instsArr[i] = qenc64insts[i] - '0';
  }
  
  int cencq0instsArr[cencq0instsSize];
  for(int i=cencq0instsSize-1; i>=0; i--){
    cencq0instsArr[i] = 0;
  }
  for(int i=0; i<cencq0insts.length(); i++){
    cencq0instsArr[i] = cencq0insts[i] - '0';
  }
  
  int cencq1instsArr[cencq1instsSize];
  for(int i=cencq1instsSize-1; i>=0; i--){
    cencq1instsArr[i] = 0;
  }
  for(int i=0; i<cencq1insts.length(); i++){
    cencq1instsArr[i] = cencq1insts[i] - '0';
  }
  
  int cencq2instsArr[cencq2instsSize];
  for(int i=cencq2instsSize-1; i>=0; i--){
    cencq2instsArr[i] = 0;
  }
  for(int i=0; i<cencq2insts.length(); i++){
    cencq2instsArr[i] = cencq2insts[i] - '0';
  }
  
  // rv32i
  if(ienc32instsArr[0 ] == 1) lui     = true;  
  if(ienc32instsArr[1 ] == 1) auipc   = true;
  if(ienc32instsArr[2 ] == 1) jal     = true;
  if(ienc32instsArr[3 ] == 1) jalr    = true;
  if(ienc32instsArr[4 ] == 1) beq     = true;
  if(ienc32instsArr[5 ] == 1) bne     = true;
  if(ienc32instsArr[6 ] == 1) blt     = true;
  if(ienc32instsArr[7 ] == 1) bge     = true;
  if(ienc32instsArr[8 ] == 1) bltu    = true;
  if(ienc32instsArr[9 ] == 1) bgeu    = true;
  if(ienc32instsArr[10] == 1) lb      = true;
  if(ienc32instsArr[11] == 1) lh      = true;
  if(ienc32instsArr[12] == 1) lw      = true;
  if(ienc32instsArr[13] == 1) lbu     = true;
  if(ienc32instsArr[14] == 1) lhu     = true;
  if(ienc32instsArr[15] == 1) sb      = true;
  if(ienc32instsArr[16] == 1) sh      = true;
  if(ienc32instsArr[17] == 1) sw      = true;
  if(ienc32instsArr[18] == 1) addi    = true;
  if(ienc32instsArr[19] == 1) slti    = true;
  if(ienc32instsArr[20] == 1) sltiu   = true;
  if(ienc32instsArr[21] == 1) xori    = true;
  if(ienc32instsArr[22] == 1) ori     = true;
  if(ienc32instsArr[23] == 1) andi    = true;
  if(ienc32instsArr[24] == 1) slli    = true;
  if(ienc32instsArr[25] == 1) srli    = true;
  if(ienc32instsArr[26] == 1) srai    = true;
  if(ienc32instsArr[27] == 1) add     = true;
  if(ienc32instsArr[28] == 1) sub     = true;
  if(ienc32instsArr[29] == 1) sll     = true;
  if(ienc32instsArr[30] == 1) slt     = true;
  if(ienc32instsArr[31] == 1) sltu    = true;
  if(ienc32instsArr[32] == 1) xor_    = true;
  if(ienc32instsArr[33] == 1) srl     = true;
  if(ienc32instsArr[34] == 1) sra     = true;
  if(ienc32instsArr[35] == 1) or_     = true;
  if(ienc32instsArr[36] == 1) and_    = true;
  if(ienc32instsArr[37] == 1) fence   = true;
  if(ienc32instsArr[38] == 1) fence_i = true;
  if(ienc32instsArr[39] == 1) ecall   = true;
  if(ienc32instsArr[40] == 1) ebreak  = true;
  if(ienc32instsArr[41] == 1) csrrw   = true;
  if(ienc32instsArr[42] == 1) csrrs   = true;
  if(ienc32instsArr[43] == 1) csrrc   = true;
  if(ienc32instsArr[44] == 1) csrrwi  = true;
  if(ienc32instsArr[45] == 1) csrrsi  = true;
  if(ienc32instsArr[46] == 1) csrrci  = true;
  
  // rv32m
  if(menc32instsArr[0 ] == 1) mul    = true;
  if(menc32instsArr[1 ] == 1) mulh   = true;
  if(menc32instsArr[2 ] == 1) mulhsu = true;
  if(menc32instsArr[3 ] == 1) mulhu  = true;
  if(menc32instsArr[4 ] == 1) div_   = true;
  if(menc32instsArr[5 ] == 1) divu   = true;
  if(menc32instsArr[6 ] == 1) rem    = true;
  if(menc32instsArr[7 ] == 1) remu   = true;
  
  // rv32a
  if(aenc32instsArr[0 ] == 1) lr_w      = true;
  if(aenc32instsArr[1 ] == 1) sc_w      = true;
  if(aenc32instsArr[2 ] == 1) amoswap_w = true;
  if(aenc32instsArr[3 ] == 1) amoadd_w  = true;
  if(aenc32instsArr[4 ] == 1) amoxor_w  = true;
  if(aenc32instsArr[5 ] == 1) amoand_w  = true;
  if(aenc32instsArr[6 ] == 1) amoor_w   = true;
  if(aenc32instsArr[7 ] == 1) amomin_w  = true;
  if(aenc32instsArr[8 ] == 1) amomax_w  = true;
  if(aenc32instsArr[9 ] == 1) amominu_w = true;
  if(aenc32instsArr[10] == 1) amomaxu_w = true;
  
  // rv32f
  if(fenc32instsArr[0 ] == 1) flw       = true;
  if(fenc32instsArr[1 ] == 1) fsw       = true;
  if(fenc32instsArr[2 ] == 1) fmadd_s   = true;
  if(fenc32instsArr[3 ] == 1) fmsub_s   = true;
  if(fenc32instsArr[4 ] == 1) fnmsub_s  = true;
  if(fenc32instsArr[5 ] == 1) fnmadd_s  = true;
  if(fenc32instsArr[6 ] == 1) fadd_s    = true;
  if(fenc32instsArr[7 ] == 1) fsub_s    = true;
  if(fenc32instsArr[8 ] == 1) fmul_s    = true;
  if(fenc32instsArr[9 ] == 1) fdiv_s    = true;
  if(fenc32instsArr[10] == 1) fsqrt_s   = true;
  if(fenc32instsArr[11] == 1) fsgnj_s   = true;
  if(fenc32instsArr[12] == 1) fsgnjn_s  = true;
  if(fenc32instsArr[13] == 1) fsgnjx_s  = true;
  if(fenc32instsArr[14] == 1) fmin_s    = true;
  if(fenc32instsArr[15] == 1) fmax_s    = true;
  if(fenc32instsArr[16] == 1) fcvt_w_s  = true;
  if(fenc32instsArr[17] == 1) fcvt_wu_s = true;
  if(fenc32instsArr[18] == 1) fmv_x_w   = true;
  if(fenc32instsArr[19] == 1) feq_s     = true;
  if(fenc32instsArr[20] == 1) flt_s     = true;
  if(fenc32instsArr[21] == 1) fle_s     = true;
  if(fenc32instsArr[22] == 1) fclass_s  = true;
  if(fenc32instsArr[23] == 1) fcvt_s_w  = true;
  if(fenc32instsArr[24] == 1) fcvt_s_wu = true;
  if(fenc32instsArr[25] == 1) fmv_w_x   = true;
  
  // rv32d
  if(denc32instsArr[0 ] == 1) fld       = true;
  if(denc32instsArr[1 ] == 1) fsd       = true;
  if(denc32instsArr[2 ] == 1) fmadd_d   = true;
  if(denc32instsArr[3 ] == 1) fmsub_d   = true;
  if(denc32instsArr[4 ] == 1) fnmsub_d  = true;
  if(denc32instsArr[5 ] == 1) fnmadd_d  = true;
  if(denc32instsArr[6 ] == 1) fadd_d    = true;
  if(denc32instsArr[7 ] == 1) fsub_d    = true;
  if(denc32instsArr[8 ] == 1) fmul_d    = true;
  if(denc32instsArr[9 ] == 1) fdiv_d    = true;
  if(denc32instsArr[10] == 1) fsqrt_d   = true;
  if(denc32instsArr[11] == 1) fsgnj_d   = true;
  if(denc32instsArr[12] == 1) fsgnjn_d  = true;
  if(denc32instsArr[13] == 1) fsgnjx_d  = true;
  if(denc32instsArr[14] == 1) fmin_d    = true;
  if(denc32instsArr[15] == 1) fmax_d    = true;
  if(denc32instsArr[16] == 1) fcvt_s_d  = true;
  if(denc32instsArr[17] == 1) fcvt_d_s  = true;
  if(denc32instsArr[18] == 1) feq_d     = true;
  if(denc32instsArr[19] == 1) flt_d     = true;
  if(denc32instsArr[20] == 1) fle_d     = true;
  if(denc32instsArr[21] == 1) fclass_d  = true;
  if(denc32instsArr[22] == 1) fcvt_w_d  = true;
  if(denc32instsArr[23] == 1) fcvt_wu_d = true;
  if(denc32instsArr[24] == 1) fcvt_d_w  = true;
  if(denc32instsArr[25] == 1) fcvt_d_wu = true;
  
  // rv32q
  if(qenc32instsArr[0 ] == 1) flq       = true;
  if(qenc32instsArr[1 ] == 1) fsq       = true;
  if(qenc32instsArr[2 ] == 1) fmadd_q   = true;
  if(qenc32instsArr[3 ] == 1) fmsub_q   = true;
  if(qenc32instsArr[4 ] == 1) fnmsub_q  = true;
  if(qenc32instsArr[5 ] == 1) fnmadd_q  = true;
  if(qenc32instsArr[6 ] == 1) fadd_q    = true;
  if(qenc32instsArr[7 ] == 1) fsub_q    = true;
  if(qenc32instsArr[8 ] == 1) fmul_q    = true;
  if(qenc32instsArr[9 ] == 1) fdiv_q    = true;
  if(qenc32instsArr[10] == 1) fsqrt_q   = true;
  if(qenc32instsArr[11] == 1) fsgnj_q   = true;
  if(qenc32instsArr[12] == 1) fsgnjn_q  = true;
  if(qenc32instsArr[13] == 1) fsgnjx_q  = true;
  if(qenc32instsArr[14] == 1) fmin_q    = true;
  if(qenc32instsArr[15] == 1) fmax_q    = true;
  if(qenc32instsArr[16] == 1) fcvt_s_q  = true;
  if(qenc32instsArr[17] == 1) fcvt_q_s  = true;
  if(qenc32instsArr[18] == 1) fcvt_d_q  = true;
  if(qenc32instsArr[19] == 1) fcvt_q_d  = true;
  if(qenc32instsArr[20] == 1) feq_q     = true;
  if(qenc32instsArr[21] == 1) flt_q     = true;
  if(qenc32instsArr[22] == 1) fle_q     = true;
  if(qenc32instsArr[23] == 1) fclass_q  = true;
  if(qenc32instsArr[24] == 1) fcvt_w_q  = true;
  if(qenc32instsArr[25] == 1) fcvt_wu_q = true;
  if(qenc32instsArr[26] == 1) fcvt_q_w  = true;
  if(qenc32instsArr[27] == 1) fcvt_q_wu  = true;
  
  // rv64i
  if(ienc64instsArr[0 ] == 1) lwu   = true;
  if(ienc64instsArr[1 ] == 1) ld    = true;
  if(ienc64instsArr[2 ] == 1) sd    = true;
  //if(ienc64instsArr[3 ] == 1) slli  = true;
  //if(ienc64instsArr[4 ] == 1) srli  = true;
  //if(ienc64instsArr[5 ] == 1) srai  = true;
  if(ienc64instsArr[3 ] == 1) addiw = true;
  if(ienc64instsArr[4 ] == 1) slliw = true;
  if(ienc64instsArr[5 ] == 1) srliw = true;
  if(ienc64instsArr[6 ] == 1) sraiw = true;
  if(ienc64instsArr[7 ] == 1) addw  = true;
  if(ienc64instsArr[8 ] == 1) subw  = true;
  if(ienc64instsArr[9 ] == 1) sllw  = true;
  if(ienc64instsArr[10] == 1) srlw  = true;
  if(ienc64instsArr[11] == 1) sraw  = true;
  
  // rv64m
  if(menc64instsArr[0 ] == 1) mulw  = true;
  if(menc64instsArr[1 ] == 1) divw  = true;
  if(menc64instsArr[2 ] == 1) divuw = true;
  if(menc64instsArr[3 ] == 1) remw  = true;
  if(menc64instsArr[4 ] == 1) remuw = true;
  
  // rv64a
  if(aenc64instsArr[0 ] == 1) lr_d      = true;
  if(aenc64instsArr[1 ] == 1) sc_d      = true;
  if(aenc64instsArr[2 ] == 1) amoswap_d = true;
  if(aenc64instsArr[3 ] == 1) amoadd_d  = true;
  if(aenc64instsArr[4 ] == 1) amoxor_d  = true;
  if(aenc64instsArr[5 ] == 1) amoand_d  = true;
  if(aenc64instsArr[6 ] == 1) amoor_d   = true;
  if(aenc64instsArr[7 ] == 1) amomin_d  = true;
  if(aenc64instsArr[8 ] == 1) amomax_d  = true;
  if(aenc64instsArr[9 ] == 1) amominu_d = true;
  if(aenc64instsArr[10] == 1) amomaxu_d = true;
  
  // rv64f
  if(fenc64instsArr[0 ] == 1) fcvt_l_s  = true;
  if(fenc64instsArr[1 ] == 1) fcvt_lu_s = true;
  if(fenc64instsArr[2 ] == 1) fcvt_s_l  = true;
  if(fenc64instsArr[3 ] == 1) fcvt_s_lu = true;
  
  // rv64d
  if(denc64instsArr[0 ] == 1) fcvt_l_d  = true;
  if(denc64instsArr[1 ] == 1) fcvt_lu_d = true;
  if(denc64instsArr[2 ] == 1) fmv_x_d   = true;
  if(denc64instsArr[3 ] == 1) fcvt_d_l  = true;
  if(denc64instsArr[4 ] == 1) fcvt_d_lu = true;
  if(denc64instsArr[5 ] == 1) fmv_d_x   = true;
  
  // rv64q
  if(qenc64instsArr[0 ] == 1) fcvt_l_q  = true;
  if(qenc64instsArr[1 ] == 1) fcvt_lu_q = true;
  if(qenc64instsArr[2 ] == 1) fcvt_q_l  = true;
  if(qenc64instsArr[3 ] == 1) fcvt_q_lu = true;
  
  // rvc quadrant 0
  if(cencq0instsArr[0 ] == 1) c_addi4spn = true;
  if(cencq0instsArr[1 ] == 1) c_fld      = true;
  if(cencq0instsArr[2 ] == 1) c_lq       = true;
  if(cencq0instsArr[3 ] == 1) c_lw       = true;
  if(cencq0instsArr[4 ] == 1) c_flw      = true;
  if(cencq0instsArr[5 ] == 1) c_ld       = true;
  if(cencq0instsArr[6 ] == 1) c_fsd      = true;
  if(cencq0instsArr[7 ] == 1) c_sq       = true;
  if(cencq0instsArr[8 ] == 1) c_sw       = true;
  if(cencq0instsArr[9 ] == 1) c_fsw      = true;
  if(cencq0instsArr[10] == 1) c_sd       = true;
  
  // rvc quadrant 1
  if(cencq1instsArr[0 ] == 1) c_nop      = true;
  if(cencq1instsArr[1 ] == 1) c_addi     = true;
  if(cencq1instsArr[2 ] == 1) c_jal      = true;
  if(cencq1instsArr[3 ] == 1) c_addiw    = true;
  if(cencq1instsArr[4 ] == 1) c_li       = true;
  if(cencq1instsArr[5 ] == 1) c_addi16sp = true;
  if(cencq1instsArr[6 ] == 1) c_lui      = true;
  if(cencq1instsArr[7 ] == 1) c_srli     = true;
  if(cencq1instsArr[8 ] == 1) c_srli64   = true;
  if(cencq1instsArr[9 ] == 1) c_srai     = true;
  if(cencq1instsArr[10] == 1) c_srai64   = true;
  if(cencq1instsArr[11] == 1) c_andi     = true;
  if(cencq1instsArr[12] == 1) c_sub      = true;
  if(cencq1instsArr[13] == 1) c_xor      = true;
  if(cencq1instsArr[14] == 1) c_or       = true;
  if(cencq1instsArr[15] == 1) c_and      = true;
  if(cencq1instsArr[16] == 1) c_subw     = true;
  if(cencq1instsArr[17] == 1) c_addw     = true;
  if(cencq1instsArr[18] == 1) c_j        = true;
  if(cencq1instsArr[19] == 1) c_beqz     = true;
  if(cencq1instsArr[20] == 1) c_bnez     = true;
  
  // rvc quadrant 2
  if(cencq2instsArr[0 ] == 1) c_slli   = true;
  if(cencq2instsArr[1 ] == 1) c_slli64 = true;
  if(cencq2instsArr[2 ] == 1) c_fldsp  = true;
  if(cencq2instsArr[3 ] == 1) c_lqsp   = true;
  if(cencq2instsArr[4 ] == 1) c_lwsp   = true;
  if(cencq2instsArr[5 ] == 1) c_flwsp  = true;
  if(cencq2instsArr[6 ] == 1) c_ldsp   = true;
  if(cencq2instsArr[7 ] == 1) c_jr     = true;
  if(cencq2instsArr[8 ] == 1) c_mv     = true;
  if(cencq2instsArr[9 ] == 1) c_ebreak = true;
  if(cencq2instsArr[10] == 1) c_jalr   = true;
  if(cencq2instsArr[11] == 1) c_add    = true;
  if(cencq2instsArr[12] == 1) c_fsdsp  = true;
  if(cencq2instsArr[13] == 1) c_sqsp   = true;
  if(cencq2instsArr[14] == 1) c_swsp   = true;
  if(cencq2instsArr[15] == 1) c_fswsp  = true;
  if(cencq2instsArr[16] == 1) c_sdsp   = true;

  llvm::for_each(InputFilenames, dumpInput);

  formatted_raw_ostream OOS(*portedOuts::outs);

  std::fstream OSS;
  OSS.open(fileName + "out.hex", std::fstream::in | std::fstream::out);
  
  std::string hashstr = "00000000000000000000000000000000";
  std::string binArr = "";
  
  OSS.seekg(std::ios_base::beg);

  std::string line;
  while (std::getline(OSS, line)){
      binArr = "";
      int_to_hex(line);
  if(line.length()<=8){
      for(int i=0; i<line.length(); i++)
        binArr += hex_char_to_bin((char)line.at(i));
  
      if(binArr.length() < 32){
        binArr = "0000000000000000" + binArr;
      }
  
      hashstr = strBitArrXor(binArr, hashstr, 4);
      }
  }

  OSS.seekg(std::ios_base::beg);

  std::vector<unsigned char> hash(picosha2::k_digest_size);
  picosha2::hash256(hashstr.begin(), hashstr.end(), hash.begin(), hash.end());
  std::string hex_str = picosha2::bytes_to_hex_string(hash.begin(), hash.end());
  OOS << hex_str << "\n";

  std::string instkeys =
  std::to_string(rsa)+
  std::to_string(lui)+
  std::to_string(auipc)+
  std::to_string(jal)+
  std::to_string(jalr)+
  std::to_string(beq)+
  std::to_string(bne)+
  std::to_string(blt)+
  std::to_string(bge)+
  std::to_string(bltu)+
  std::to_string(bgeu)+
  std::to_string(lb)+
  std::to_string(lh)+
  std::to_string(lw)+
  std::to_string(lbu)+
  std::to_string(lhu)+
  std::to_string(sb)+
  std::to_string(sh)+
  std::to_string(sw)+
  std::to_string(addi)+
  std::to_string(slti)+
  std::to_string(sltiu)+
  std::to_string(xori)+
  std::to_string(ori)+
  std::to_string(andi)+
  std::to_string(slli)+
  std::to_string(srli)+
  std::to_string(srai)+
  std::to_string(add)+
  std::to_string(sub)+
  std::to_string(sll)+
  std::to_string(slt)+
  std::to_string(sltu)+
  std::to_string(xor_)+
  std::to_string(srl)+
  std::to_string(sra)+
  std::to_string(or_)+
  std::to_string(and_)+
  std::to_string(fence)+
  std::to_string(fence_i)+
  std::to_string(ecall)+
  std::to_string(ebreak)+
  std::to_string(csrrw)+
  std::to_string(csrrs)+
  std::to_string(csrrc)+
  std::to_string(csrrwi)+
  std::to_string(csrrsi)+
  std::to_string(csrrci)+
  
  // rv64i
  std::to_string(lwu)+
  std::to_string(ld)+
  std::to_string(sd)+
  //std::to_string(slli)+
  //std::to_string(srli)+
  //std::to_string(srai)+
  std::to_string(addiw)+
  std::to_string(slliw)+
  std::to_string(srliw)+
  std::to_string(sraiw)+
  std::to_string(addw)+
  std::to_string(subw)+
  std::to_string(sllw)+
  std::to_string(srlw)+
  std::to_string(sraw)+
  
  // rv32m
  std::to_string(mul)+
  std::to_string(mulh)+ 
  std::to_string(mulhsu)+   
  std::to_string(mulhu)+  
  std::to_string(div_)+
  std::to_string(divu)+ 
  std::to_string(rem)+
  std::to_string(remu)+
  
  
  // rv64m
  std::to_string(mulw)+
  std::to_string(divw)+
  std::to_string(divuw)+ 
  std::to_string(remw)+
  std::to_string(remuw)+
  
  // rv32a
  std::to_string(lr_w)+
  std::to_string(sc_w)+
  std::to_string(amoswap_w)+   
  std::to_string(amoadd_w)+  
  std::to_string(amoxor_w)+  
  std::to_string(amoand_w)+  
  std::to_string(amoor_w)+ 
  std::to_string(amomin_w)+  
  std::to_string(amomax_w)+  
  std::to_string(amominu_w)+   
  std::to_string(amomaxu_w)+
  
  // rv64a
  std::to_string(lr_d)+
  std::to_string(sc_d)+
  std::to_string(amoswap_d)+
  std::to_string(amoadd_d)+
  std::to_string(amoxor_d)+
  std::to_string(amoand_d)+
  std::to_string(amoor_d)+
  std::to_string(amomin_d)+
  std::to_string(amomax_d)+
  std::to_string(amominu_d)+
  std::to_string(amomaxu_d)+
  
  // rv32f
  std::to_string(flw)+
  std::to_string(fsw)+
  std::to_string(fmadd_s)+
  std::to_string(fmsub_s)+
  std::to_string(fnmsub_s)+
  std::to_string(fnmadd_s)+
  std::to_string(fadd_s)+
  std::to_string(fsub_s)+
  std::to_string(fmul_s)+
  std::to_string(fdiv_s)+
  std::to_string(fsqrt_s)+
  std::to_string(fsgnj_s)+
  std::to_string(fsgnjn_s)+
  std::to_string(fsgnjx_s)+
  std::to_string(fmin_s)+
  std::to_string(fmax_s)+
  std::to_string(fcvt_w_s)+
  std::to_string(fcvt_wu_s)+
  std::to_string(fmv_x_w)+
  std::to_string(feq_s)+
  std::to_string(flt_s)+
  std::to_string(fle_s)+
  std::to_string(fclass_s)+
  std::to_string(fcvt_s_w)+
  std::to_string(fcvt_s_wu)+
  std::to_string(fmv_w_x)+
  
  // rv64f
  std::to_string( fcvt_l_s)+
  std::to_string( fcvt_lu_s)+
  std::to_string( fcvt_s_l)+
  std::to_string( fcvt_s_lu)+
  
  // rv32d
  std::to_string(fld)+
  std::to_string(fsd)+
  std::to_string(fmadd_d)+
  std::to_string(fmsub_d)+
  std::to_string(fnmsub_d)+
  std::to_string(fnmadd_d)+
  std::to_string(fadd_d)+
  std::to_string(fsub_d)+
  std::to_string(fmul_d)+
  std::to_string(fdiv_d)+
  std::to_string(fsqrt_d)+
  std::to_string(fsgnj_d)+
  std::to_string(fsgnjn_d)+
  std::to_string(fsgnjx_d)+
  std::to_string(fmin_d)+
  std::to_string(fmax_d)+
  std::to_string(fcvt_s_d)+
  std::to_string(fcvt_d_s)+
  std::to_string(feq_d)+
  std::to_string(flt_d)+
  std::to_string(fle_d)+
  std::to_string(fclass_d)+
  std::to_string(fcvt_w_d)+
  std::to_string(fcvt_wu_d)+
  std::to_string(fcvt_d_w)+
  std::to_string(fcvt_d_wu)+
  
  // rv64d
  std::to_string(fcvt_l_d)+
  std::to_string(fcvt_lu_d)+
  std::to_string(fmv_x_d)+
  std::to_string(fcvt_d_l)+
  std::to_string(fcvt_d_lu)+
  std::to_string(fmv_d_x)+
  
  // rv32q
  std::to_string(flq)+
  std::to_string(fsq)+
  std::to_string(fmadd_q)+
  std::to_string(fmsub_q)+
  std::to_string(fnmsub_q)+
  std::to_string(fnmadd_q)+
  std::to_string(fadd_q)+
  std::to_string(fsub_q)+
  std::to_string(fmul_q)+
  std::to_string(fdiv_q)+
  std::to_string(fsqrt_q)+
  std::to_string(fsgnj_q)+
  std::to_string(fsgnjn_q)+
  std::to_string(fsgnjx_q)+
  std::to_string(fmin_q)+
  std::to_string(fmax_q)+
  std::to_string(fcvt_s_q)+
  std::to_string(fcvt_q_s)+
  std::to_string(fcvt_d_q)+
  std::to_string(fcvt_q_d)+
  std::to_string(feq_q)+
  std::to_string(flt_q)+
  std::to_string(fle_q)+
  std::to_string(fclass_q)+
  std::to_string(fcvt_w_q)+
  std::to_string(fcvt_wu_q)+
  std::to_string(fcvt_q_w)+
  std::to_string(fcvt_q_wu)+
  
  // rv64q
  std::to_string(fcvt_l_q)+
  std::to_string(fcvt_lu_q)+
  std::to_string(fcvt_q_l)+
  std::to_string(fcvt_q_lu)+
  
  // rvc quadrant 0
  std::to_string(c_addi4spn)+
  std::to_string(c_fld)+
  std::to_string(c_lq)+
  std::to_string(c_lw)+
  std::to_string(c_flw)+
  std::to_string(c_ld)+
  std::to_string(c_fsd)+
  std::to_string(c_sq)+
  std::to_string(c_sw)+
  std::to_string(c_fsw)+
  std::to_string(c_sd)+
  
  // rvc quadrant 1
  std::to_string(c_nop)+
  std::to_string(c_addi)+
  std::to_string(c_jal)+
  std::to_string(c_addiw)+
  std::to_string(c_li)+
  std::to_string(c_addi16sp)+
  std::to_string(c_lui)+
  std::to_string(c_srli)+
  std::to_string(c_srli64)+
  std::to_string(c_srai)+
  std::to_string(c_srai64)+
  std::to_string(c_andi)+
  std::to_string(c_sub)+
  std::to_string(c_xor)+
  std::to_string(c_or)+
  std::to_string(c_and)+
  std::to_string(c_subw)+
  std::to_string(c_addw)+
  std::to_string(c_j)+
  std::to_string(c_beqz)+
  std::to_string(c_bnez)+
  
  // rvc quadrant 2
  std::to_string(c_slli)+
  std::to_string(c_slli64)+
  std::to_string(c_fldsp)+
  std::to_string(c_lqsp)+
  std::to_string(c_lwsp)+
  std::to_string(c_flwsp)+
  std::to_string(c_ldsp)+
  std::to_string(c_jr)+
  std::to_string(c_mv)+
  std::to_string(c_ebreak)+
  std::to_string(c_jalr)+
  std::to_string(c_add)+
  std::to_string(c_fsdsp)+
  std::to_string(c_sqsp)+
  std::to_string(c_swsp)+
  std::to_string(c_fswsp)+
  std::to_string(c_sdsp);

  if(!instkeys.empty())
    OOS<<instkeys<<"\n";

  std::string keys =
  enckeyall + " " +

  ienc32key + " " +
  menc32key + " " +
  aenc32key + " " +
  fenc32key + " " +
  denc32key + " " +
  qenc32key + " " +
 
  ienc64key + " " +
  menc64key + " " +
  aenc64key + " " +
  fenc64key + " " +
  denc64key + " " +
  qenc64key + " " +
 
  cencq0key + " " +
  cencq1key + " " +
  cencq2key + " " +
 
  ienc32insts + " " +
  menc32insts + " " +
  aenc32insts + " " +
  fenc32insts + " " +
  denc32insts + " " +
  qenc32insts + " " +
 
  ienc64insts + " " +
  menc64insts + " " +
  aenc64insts + " " + 
  fenc64insts + " " + 
  denc64insts + " " + 
  qenc64insts + " " + 
 
  cencq0insts + " " +
  cencq1insts + " " +
  cencq2insts;

  if(!keys.empty())
    OOS<<keys<<"\n";

  std::string b_p_keys = 
  // rv32i
  b_p_lui+ " " +
  b_p_auipc+ " " +
  b_p_jal+ " " +
  b_p_jalr+ " " +
  b_p_beq+ " " +
  b_p_bne+ " " +
  b_p_blt+ " " +
  b_p_bge+ " " +
  b_p_bltu+ " " +
  b_p_bgeu+ " " +
  b_p_lb+ " " +
  b_p_lh+ " " +
  b_p_lw+ " " +
  b_p_lbu+ " " +
  b_p_lhu+ " " +
  b_p_sb+ " " +
  b_p_sh+ " " +
  b_p_sw+ " " +
  b_p_addi+ " " +
  b_p_slti+ " " +
  b_p_sltiu+ " " +
  b_p_xori+ " " +
  b_p_ori+ " " +
  b_p_andi+ " " +
  b_p_slli+ " " +
  b_p_srli+ " " +
  b_p_srai+ " " +
  b_p_add+ " " +
  b_p_sub+ " " +
  b_p_sll+ " " +
  b_p_slt+ " " +
  b_p_sltu+ " " +
  b_p_xor_+ " " +
  b_p_srl+ " " +
  b_p_sra+ " " +
  b_p_or_+ " " +
  b_p_and_+ " " +
  b_p_fence+ " " +
  b_p_fence_i+ " " +
  b_p_ecall+ " " +
  b_p_ebreak+ " " +
  b_p_csrrw+ " " +
  b_p_csrrs+ " " +
  b_p_csrrc+ " " +
  b_p_csrrwi+ " " +
  b_p_csrrsi+ " " +
  b_p_csrrci+ " " +
  
  // rv64i
  b_p_lwu+ " " +
  b_p_ld+ " " +
  b_p_sd+ " " +
  //b_p_slli+ " " +
  //b_p_srli+ " " +
  //b_p_srai+ " " +
  b_p_addiw+ " " +
  b_p_slliw+ " " +
  b_p_srliw+ " " +
  b_p_sraiw+ " " +
  b_p_addw+ " " +
  b_p_subw+ " " +
  b_p_sllw+ " " +
  b_p_srlw+ " " +
  b_p_sraw+ " " +
  
  // rv32m
  b_p_mul+ " " +
  b_p_mulh+ " " + 
  b_p_mulhsu+ " " +   
  b_p_mulhu+ " " +  
  b_p_div_+ " " +
  b_p_divu+ " " + 
  b_p_rem+ " " +
  b_p_remu+ " " +
  
  
  // rv64m
  b_p_mulw+ " " +
  b_p_divw+ " " +
  b_p_divuw+ " " + 
  b_p_remw+ " " +
  b_p_remuw+ " " +
  
  // rv32a
  b_p_lr_w+ " " +
  b_p_sc_w+ " " +
  b_p_amoswap_w+ " " +   
  b_p_amoadd_w+ " " +  
  b_p_amoxor_w+ " " +  
  b_p_amoand_w+ " " +  
  b_p_amoor_w+ " " + 
  b_p_amomin_w+ " " +  
  b_p_amomax_w+ " " +  
  b_p_amominu_w+ " " +   
  b_p_amomaxu_w+ " " +
  
  // rv64a
  b_p_lr_d+ " " +
  b_p_sc_d+ " " +
  b_p_amoswap_d+ " " +
  b_p_amoadd_d+ " " +
  b_p_amoxor_d+ " " +
  b_p_amoand_d+ " " +
  b_p_amoor_d+ " " +
  b_p_amomin_d+ " " +
  b_p_amomax_d+ " " +
  b_p_amominu_d+ " " +
  b_p_amomaxu_d+ " " +
  
  // rv32f
  b_p_flw+ " " +
  b_p_fsw+ " " +
  b_p_fmadd_s+ " " +
  b_p_fmsub_s+ " " +
  b_p_fnmsub_s+ " " +
  b_p_fnmadd_s+ " " +
  b_p_fadd_s+ " " +
  b_p_fsub_s+ " " +
  b_p_fmul_s+ " " +
  b_p_fdiv_s+ " " +
  b_p_fsqrt_s+ " " +
  b_p_fsgnj_s+ " " +
  b_p_fsgnjn_s+ " " +
  b_p_fsgnjx_s+ " " +
  b_p_fmin_s+ " " +
  b_p_fmax_s+ " " +
  b_p_fcvt_w_s+ " " +
  b_p_fcvt_wu_s+ " " +
  b_p_fmv_x_w+ " " +
  b_p_feq_s+ " " +
  b_p_flt_s+ " " +
  b_p_fle_s+ " " +
  b_p_fclass_s+ " " +
  b_p_fcvt_s_w+ " " +
  b_p_fcvt_s_wu+ " " +
  b_p_fmv_w_x+ " " +
  
  // rv64f
  b_p_fcvt_l_s+ " " +
  b_p_fcvt_lu_s+ " " +
  b_p_fcvt_s_l+ " " +
  b_p_fcvt_s_lu+ " " +
  
  // rv32d
  b_p_fld+ " " +
  b_p_fsd+ " " +
  b_p_fmadd_d+ " " +
  b_p_fmsub_d+ " " +
  b_p_fnmsub_d+ " " +
  b_p_fnmadd_d+ " " +
  b_p_fadd_d+ " " +
  b_p_fsub_d+ " " +
  b_p_fmul_d+ " " +
  b_p_fdiv_d+ " " +
  b_p_fsqrt_d+ " " +
  b_p_fsgnj_d+ " " +
  b_p_fsgnjn_d+ " " +
  b_p_fsgnjx_d+ " " +
  b_p_fmin_d+ " " +
  b_p_fmax_d+ " " +
  b_p_fcvt_s_d+ " " +
  b_p_fcvt_d_s+ " " +
  b_p_feq_d+ " " +
  b_p_flt_d+ " " +
  b_p_fle_d+ " " +
  b_p_fclass_d+ " " +
  b_p_fcvt_w_d+ " " +
  b_p_fcvt_wu_d+ " " +
  b_p_fcvt_d_w+ " " +
  b_p_fcvt_d_wu+ " " +
  
  // rv64d
  b_p_fcvt_l_d+ " " +
  b_p_fcvt_lu_d+ " " +
  b_p_fmv_x_d+ " " +
  b_p_fcvt_d_l+ " " +
  b_p_fcvt_d_lu+ " " +
  b_p_fmv_d_x+ " " +
  
  // rv32q
  b_p_flq+ " " +
  b_p_fsq+ " " +
  b_p_fmadd_q+ " " +
  b_p_fmsub_q+ " " +
  b_p_fnmsub_q+ " " +
  b_p_fnmadd_q+ " " +
  b_p_fadd_q+ " " +
  b_p_fsub_q+ " " +
  b_p_fmul_q+ " " +
  b_p_fdiv_q+ " " +
  b_p_fsqrt_q+ " " +
  b_p_fsgnj_q+ " " +
  b_p_fsgnjn_q+ " " +
  b_p_fsgnjx_q+ " " +
  b_p_fmin_q+ " " +
  b_p_fmax_q+ " " +
  b_p_fcvt_s_q+ " " +
  b_p_fcvt_q_s+ " " +
  b_p_fcvt_d_q+ " " +
  b_p_fcvt_q_d+ " " +
  b_p_feq_q+ " " +
  b_p_flt_q+ " " +
  b_p_fle_q+ " " +
  b_p_fclass_q+ " " +
  b_p_fcvt_w_q+ " " +
  b_p_fcvt_wu_q+ " " +
  b_p_fcvt_q_w+ " " +
  b_p_fcvt_q_wu+ " " +
  
  // rv64q
  b_p_fcvt_l_q+ " " +
  b_p_fcvt_lu_q+ " " +
  b_p_fcvt_q_l+ " " +
  b_p_fcvt_q_lu+ " " +
  
  // rvc quadrant 0
  b_p_c_addi4spn+ " " +
  b_p_c_fld+ " " +
  b_p_c_lq+ " " +
  b_p_c_lw+ " " +
  b_p_c_flw+ " " +
  b_p_c_ld+ " " +
  b_p_c_fsd+ " " +
  b_p_c_sq+ " " +
  b_p_c_sw+ " " +
  b_p_c_fsw+ " " +
  b_p_c_sd+ " " +
  
  // rvc quadrant 1
  b_p_c_nop+ " " +
  b_p_c_addi+ " " +
  b_p_c_jal+ " " +
  b_p_c_addiw+ " " +
  b_p_c_li+ " " +
  b_p_c_addi16sp+ " " +
  b_p_c_lui+ " " +
  b_p_c_srli+ " " +
  b_p_c_srli64+ " " +
  b_p_c_srai+ " " +
  b_p_c_srai64+ " " +
  b_p_c_andi+ " " +
  b_p_c_sub+ " " +
  b_p_c_xor+ " " +
  b_p_c_or+ " " +
  b_p_c_and+ " " +
  b_p_c_subw+ " " +
  b_p_c_addw+ " " +
  b_p_c_j+ " " +
  b_p_c_beqz+ " " +
  b_p_c_bnez+ " " +
  
  // rvc quadrant 2
  b_p_c_slli+ " " +
  b_p_c_slli64+ " " +
  b_p_c_fldsp+ " " +
  b_p_c_lqsp+ " " +
  b_p_c_lwsp+ " " +
  b_p_c_flwsp+ " " +
  b_p_c_ldsp+ " " +
  b_p_c_jr+ " " +
  b_p_c_mv+ " " +
  b_p_c_ebreak+ " " +
  b_p_c_jalr+ " " +
  b_p_c_add+ " " +
  b_p_c_fsdsp+ " " +
  b_p_c_sqsp+ " " +
  b_p_c_swsp+ " " +
  b_p_c_fswsp+ " " +
  b_p_c_sdsp;

  if(!b_p_keys.empty())
    OOS << b_p_keys << "\n";

  OSS.close();
  return EXIT_SUCCESS;
}
