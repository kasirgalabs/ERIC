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
// xor, or ve and icin _ eklemek zorundaydım.
extern cl::opt<bool> beq               ;
extern cl::opt<bool> bne               ;
extern cl::opt<bool> blt               ;
extern cl::opt<bool> bge               ;
extern cl::opt<bool> bltu              ;
extern cl::opt<bool> bgeu              ;
extern cl::opt<bool> jalr              ;
extern cl::opt<bool> jal               ;
extern cl::opt<bool> lui               ;
extern cl::opt<bool> auipc             ;
extern cl::opt<bool> addi              ;
extern cl::opt<bool> slli              ;
extern cl::opt<bool> slti              ;
extern cl::opt<bool> sltiu             ;
extern cl::opt<bool> xori              ;
extern cl::opt<bool> srli              ;
extern cl::opt<bool> srai              ;
extern cl::opt<bool> ori               ;
extern cl::opt<bool> andi              ;
extern cl::opt<bool> add               ;
extern cl::opt<bool> sub               ;
extern cl::opt<bool> sll               ;
extern cl::opt<bool> slt               ;
extern cl::opt<bool> sltu              ;
extern cl::opt<bool> xor_               ;
extern cl::opt<bool> srl               ;
extern cl::opt<bool> sra               ;
extern cl::opt<bool> or_                ;
extern cl::opt<bool> and_               ;
extern cl::opt<bool> addiw             ;
extern cl::opt<bool> slliw             ;
extern cl::opt<bool> srliw             ;
extern cl::opt<bool> sraiw             ;
extern cl::opt<bool> addw              ;
extern cl::opt<bool> subw              ;
extern cl::opt<bool> sllw              ;
extern cl::opt<bool> srlw              ;
extern cl::opt<bool> sraw              ;
extern cl::opt<bool> lb                ;
extern cl::opt<bool> lh                ;
extern cl::opt<bool> lw                ;
extern cl::opt<bool> ld                ;
extern cl::opt<bool> lbu               ;
extern cl::opt<bool> lhu               ;
extern cl::opt<bool> lwu               ;
extern cl::opt<bool> sb                ;
extern cl::opt<bool> sh                ;
extern cl::opt<bool> sw                ;
extern cl::opt<bool> sd                ;
extern cl::opt<bool> fence             ;
extern cl::opt<bool> fence_i           ;
extern cl::opt<bool> mul               ;
extern cl::opt<bool> mulh              ;
extern cl::opt<bool> mulhsu            ;
extern cl::opt<bool> mulhu             ;
extern cl::opt<bool> div               ;
extern cl::opt<bool> divu              ;
extern cl::opt<bool> rem               ;
extern cl::opt<bool> remu              ;
extern cl::opt<bool> mulw              ;
extern cl::opt<bool> divw              ;
extern cl::opt<bool> divuw             ;
extern cl::opt<bool> remw              ;
extern cl::opt<bool> remuw             ;
extern cl::opt<bool> lr_w              ;
extern cl::opt<bool> sc_w              ;
extern cl::opt<bool> lr_d              ;
extern cl::opt<bool> sc_d              ;
extern cl::opt<bool> ecall             ;
extern cl::opt<bool> ebreak            ;
extern cl::opt<bool> uret              ;
extern cl::opt<bool> mret              ;
extern cl::opt<bool> dret              ;
extern cl::opt<bool> sfence_vma        ;
extern cl::opt<bool> wfi               ;
extern cl::opt<bool> csrrw             ;
extern cl::opt<bool> csrrs             ;
extern cl::opt<bool> csrrc             ;
extern cl::opt<bool> csrrwi            ;
extern cl::opt<bool> csrrsi            ;
extern cl::opt<bool> csrrci            ;
//extern cl::opt<bool> custom0           ;
//extern cl::opt<bool> custom0_rs1       ;
//extern cl::opt<bool> custom0_rs1_rs2   ;
//extern cl::opt<bool> custom0_rd        ;
//extern cl::opt<bool> custom0_rd_rs1    ;
//extern cl::opt<bool> custom0_rd_rs1_rs2;
//extern cl::opt<bool> custom1           ;
//extern cl::opt<bool> custom1_rs1       ;
//extern cl::opt<bool> custom1_rs1_rs2   ;
//extern cl::opt<bool> custom1_rd        ;
//extern cl::opt<bool> custom1_rd_rs1    ;
//extern cl::opt<bool> custom1_rd_rs1_rs2;
//extern cl::opt<bool> custom2           ;
//extern cl::opt<bool> custom2_rs1       ;
//extern cl::opt<bool> custom2_rs1_rs2   ;
//extern cl::opt<bool> custom2_rd        ;
//extern cl::opt<bool> custom2_rd_rs1    ;
//extern cl::opt<bool> custom2_rd_rs1_rs2;
//extern cl::opt<bool> custom3           ;
//extern cl::opt<bool> custom3_rs1       ;
//extern cl::opt<bool> custom3_rs1_rs2   ;
//extern cl::opt<bool> custom3_rd        ;
//extern cl::opt<bool> custom3_rd_rs1    ;
//extern cl::opt<bool> custom3_rd_rs1_rs2;
extern cl::opt<bool> slli_rv32         ;
extern cl::opt<bool> srli_rv32         ;
extern cl::opt<bool> srai_rv32         ;
extern cl::opt<bool> rdcycle           ;
extern cl::opt<bool> rdtime            ;
extern cl::opt<bool> rdinstret         ;
extern cl::opt<bool> rdcycleh          ;
extern cl::opt<bool> rdtimeh           ;
extern cl::opt<bool> rdinstreth        ;

///////////////////////////////////////

extern cl::opt<bool> p_beq               ;
extern cl::opt<bool> p_bne               ;
extern cl::opt<bool> p_blt               ;
extern cl::opt<bool> p_bge               ;
extern cl::opt<bool> p_bltu              ;
extern cl::opt<bool> p_bgeu              ;
extern cl::opt<bool> p_jalr              ;
extern cl::opt<bool> p_jal               ;
extern cl::opt<bool> p_lui               ;
extern cl::opt<bool> p_auipc             ;
extern cl::opt<bool> p_addi              ;
extern cl::opt<bool> p_slli              ;
extern cl::opt<bool> p_slti              ;
extern cl::opt<bool> p_sltiu             ;
extern cl::opt<bool> p_xori              ;
extern cl::opt<bool> p_srli              ;
extern cl::opt<bool> p_srai              ;
extern cl::opt<bool> p_ori               ;
extern cl::opt<bool> p_andi              ;
extern cl::opt<bool> p_add               ;
extern cl::opt<bool> p_sub               ;
extern cl::opt<bool> p_sll               ;
extern cl::opt<bool> p_slt               ;
extern cl::opt<bool> p_sltu              ;
extern cl::opt<bool> p_xor_               ;
extern cl::opt<bool> p_srl               ;
extern cl::opt<bool> p_sra               ;
extern cl::opt<bool> p_or_                ;
extern cl::opt<bool> p_and_               ;
extern cl::opt<bool> p_addiw             ;
extern cl::opt<bool> p_slliw             ;
extern cl::opt<bool> p_srliw             ;
extern cl::opt<bool> p_sraiw             ;
extern cl::opt<bool> p_addw              ;
extern cl::opt<bool> p_subw              ;
extern cl::opt<bool> p_sllw              ;
extern cl::opt<bool> p_srlw              ;
extern cl::opt<bool> p_sraw              ;
extern cl::opt<bool> p_lb                ;
extern cl::opt<bool> p_lh                ;
extern cl::opt<bool> p_lw                ;
extern cl::opt<bool> p_ld                ;
extern cl::opt<bool> p_lbu               ;
extern cl::opt<bool> p_lhu               ;
extern cl::opt<bool> p_lwu               ;
extern cl::opt<bool> p_sb                ;
extern cl::opt<bool> p_sh                ;
extern cl::opt<bool> p_sw                ;
extern cl::opt<bool> p_sd                ;
extern cl::opt<bool> p_fence             ;
extern cl::opt<bool> p_fence_i           ;
extern cl::opt<bool> p_mul               ;
extern cl::opt<bool> p_mulh              ;
extern cl::opt<bool> p_mulhsu            ;
extern cl::opt<bool> p_mulhu             ;
extern cl::opt<bool> p_div               ;
extern cl::opt<bool> p_divu              ;
extern cl::opt<bool> p_rem               ;
extern cl::opt<bool> p_remu              ;
extern cl::opt<bool> p_mulw              ;
extern cl::opt<bool> p_divw              ;
extern cl::opt<bool> p_divuw             ;
extern cl::opt<bool> p_remw              ;
extern cl::opt<bool> p_remuw             ;
extern cl::opt<bool> p_lr_w              ;
extern cl::opt<bool> p_sc_w              ;
extern cl::opt<bool> p_lr_d              ;
extern cl::opt<bool> p_sc_d              ;
extern cl::opt<bool> p_ecall             ;
extern cl::opt<bool> p_ebreak            ;
extern cl::opt<bool> p_uret              ;
extern cl::opt<bool> p_mret              ;
extern cl::opt<bool> p_dret              ;
extern cl::opt<bool> p_sfence_vma        ;
extern cl::opt<bool> p_wfi               ;
extern cl::opt<bool> p_csrrw             ;
extern cl::opt<bool> p_csrrs             ;
extern cl::opt<bool> p_csrrc             ;
extern cl::opt<bool> p_csrrwi            ;
extern cl::opt<bool> p_csrrsi            ;
extern cl::opt<bool> p_csrrci            ;
extern cl::opt<bool> p_slli_rv32         ;
extern cl::opt<bool> p_srli_rv32         ;
extern cl::opt<bool> p_srai_rv32         ;
extern cl::opt<bool> p_rdcycle           ;
extern cl::opt<bool> p_rdtime            ;
extern cl::opt<bool> p_rdinstret         ;
extern cl::opt<bool> p_rdcycleh          ;
extern cl::opt<bool> p_rdtimeh           ;
extern cl::opt<bool> p_rdinstreth        ;

extern cl::opt<std::string> b_p_beq               ;
extern cl::opt<std::string> b_p_bne               ;
extern cl::opt<std::string> b_p_blt               ;
extern cl::opt<std::string> b_p_bge               ;
extern cl::opt<std::string> b_p_bltu              ;
extern cl::opt<std::string> b_p_bgeu              ;
extern cl::opt<std::string> b_p_jalr              ;
extern cl::opt<std::string> b_p_jal               ;
extern cl::opt<std::string> b_p_lui               ;
extern cl::opt<std::string> b_p_auipc             ;
extern cl::opt<std::string> b_p_addi              ;
extern cl::opt<std::string> b_p_slli              ;
extern cl::opt<std::string> b_p_slti              ;
extern cl::opt<std::string> b_p_sltiu             ;
extern cl::opt<std::string> b_p_xori              ;
extern cl::opt<std::string> b_p_srli              ;
extern cl::opt<std::string> b_p_srai              ;
extern cl::opt<std::string> b_p_ori               ;
extern cl::opt<std::string> b_p_andi              ;
extern cl::opt<std::string> b_p_add               ;
extern cl::opt<std::string> b_p_sub               ;
extern cl::opt<std::string> b_p_sll               ;
extern cl::opt<std::string> b_p_slt               ;
extern cl::opt<std::string> b_p_sltu              ;
extern cl::opt<std::string> b_p_xor_               ;
extern cl::opt<std::string> b_p_srl               ;
extern cl::opt<std::string> b_p_sra               ;
extern cl::opt<std::string> b_p_or_                ;
extern cl::opt<std::string> b_p_and_               ;
extern cl::opt<std::string> b_p_addiw             ;
extern cl::opt<std::string> b_p_slliw             ;
extern cl::opt<std::string> b_p_srliw             ;
extern cl::opt<std::string> b_p_sraiw             ;
extern cl::opt<std::string> b_p_addw              ;
extern cl::opt<std::string> b_p_subw              ;
extern cl::opt<std::string> b_p_sllw              ;
extern cl::opt<std::string> b_p_srlw              ;
extern cl::opt<std::string> b_p_sraw              ;
extern cl::opt<std::string> b_p_lb                ;
extern cl::opt<std::string> b_p_lh                ;
extern cl::opt<std::string> b_p_lw                ;
extern cl::opt<std::string> b_p_ld                ;
extern cl::opt<std::string> b_p_lbu               ;
extern cl::opt<std::string> b_p_lhu               ;
extern cl::opt<std::string> b_p_lwu               ;
extern cl::opt<std::string> b_p_sb                ;
extern cl::opt<std::string> b_p_sh                ;
extern cl::opt<std::string> b_p_sw                ;
extern cl::opt<std::string> b_p_sd                ;
extern cl::opt<std::string> b_p_fence             ;
extern cl::opt<std::string> b_p_fence_i           ;
extern cl::opt<std::string> b_p_mul               ;
extern cl::opt<std::string> b_p_mulh              ;
extern cl::opt<std::string> b_p_mulhsu            ;
extern cl::opt<std::string> b_p_mulhu             ;
extern cl::opt<std::string> b_p_div               ;
extern cl::opt<std::string> b_p_divu              ;
extern cl::opt<std::string> b_p_rem               ;
extern cl::opt<std::string> b_p_remu              ;
extern cl::opt<std::string> b_p_mulw              ;
extern cl::opt<std::string> b_p_divw              ;
extern cl::opt<std::string> b_p_divuw             ;
extern cl::opt<std::string> b_p_remw              ;
extern cl::opt<std::string> b_p_remuw             ;
extern cl::opt<std::string> b_p_lr_w              ;
extern cl::opt<std::string> b_p_sc_w              ;
extern cl::opt<std::string> b_p_lr_d              ;
extern cl::opt<std::string> b_p_sc_d              ;
extern cl::opt<std::string> b_p_ecall             ;
extern cl::opt<std::string> b_p_ebreak            ;
extern cl::opt<std::string> b_p_uret              ;
extern cl::opt<std::string> b_p_mret              ;
extern cl::opt<std::string> b_p_dret              ;
extern cl::opt<std::string> b_p_sfence_vma        ;
extern cl::opt<std::string> b_p_wfi               ;
extern cl::opt<std::string> b_p_csrrw             ;
extern cl::opt<std::string> b_p_csrrs             ;
extern cl::opt<std::string> b_p_csrrc             ;
extern cl::opt<std::string> b_p_csrrwi            ;
extern cl::opt<std::string> b_p_csrrsi            ;
extern cl::opt<std::string> b_p_csrrci            ;
extern cl::opt<std::string> b_p_slli_rv32         ;
extern cl::opt<std::string> b_p_srli_rv32         ;
extern cl::opt<std::string> b_p_srai_rv32         ;
extern cl::opt<std::string> b_p_rdcycle           ;
extern cl::opt<std::string> b_p_rdtime            ;
extern cl::opt<std::string> b_p_rdinstret         ;
extern cl::opt<std::string> b_p_rdcycleh          ;
extern cl::opt<std::string> b_p_rdtimeh           ;
extern cl::opt<std::string> b_p_rdinstreth        ;

//////////////////////////////////////

extern cl::opt<std::string> dosya;
extern cl::opt<std::string> key;
extern cl::opt<std::string> bits;

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
