find_package(LLVM)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

#include(AddLLVM.cmake)

set(LLVM_LINK_COMPONENTS
  AllTargetsDescs
  AllTargetsDisassemblers
  AllTargetsInfos
  BinaryFormat
  CodeGen
  DebugInfoDWARF
  DebugInfoPDB
  Demangle
  MC
  MCDisassembler
  Object
  Support
  Symbolize
#  RISCVInfo
  )

##
#set(LLVM_TARGET_DEFINITIONS RISCV.td)
#
#tablegen(LLVM RISCVGenAsmMatcher.inc -gen-asm-matcher)
#tablegen(LLVM RISCVGenAsmWriter.inc -gen-asm-writer)
#tablegen(LLVM RISCVGenCompressInstEmitter.inc -gen-compress-inst-emitter)
#tablegen(LLVM RISCVGenDAGISel.inc -gen-dag-isel)
#tablegen(LLVM RISCVGenDisassemblerTables.inc -gen-disassembler)
#tablegen(LLVM RISCVGenGlobalISel.inc -gen-global-isel)
#tablegen(LLVM RISCVGenInstrInfo.inc -gen-instr-info)
#tablegen(LLVM RISCVGenMCCodeEmitter.inc -gen-emitter)
#tablegen(LLVM RISCVGenMCPseudoLowering.inc -gen-pseudo-lowering)
#tablegen(LLVM RISCVGenRegisterBank.inc -gen-register-bank)
#tablegen(LLVM RISCVGenRegisterInfo.inc -gen-register-info)
#tablegen(LLVM RISCVGenSubtargetInfo.inc -gen-subtarget)
#tablegen(LLVM RISCVGenSystemOperands.inc -gen-searchable-tables)
#
#add_public_tablegen_target(RISCVCommonTableGen)
##



add_llvm_executable(alp
  alp.cpp
  COFFDump.cpp
  ELFDump.cpp
  MachODump.cpp
  WasmDump.cpp
  XCOFFDump.cpp

  DEPENDS
  intrinsics_gen
  )

#  ADD_TO_COMPONENT
#  RISCV
#)

