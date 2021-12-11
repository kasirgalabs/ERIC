find_package(LLVM)
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

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
  )


add_llvm_executable(elf2encryptedhex
  elf2encryptedhex.cpp
  COFFDump.cpp
  ELFDump.cpp
  MachODump.cpp
  WasmDump.cpp
  XCOFFDump.cpp

  DEPENDS
  intrinsics_gen
  )

