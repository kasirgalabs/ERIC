if(NOT LLVM_BUILD_LLVM_DYLIB AND NOT LLVM_BUILD_LLVM_C_DYLIB)
  set(LLVM_TOOL_LLVM_SHLIB_BUILD Off)
endif()

if(NOT LLVM_USE_INTEL_JITEVENTS )
  set(LLVM_TOOL_LLVM_JITLISTENER_BUILD Off)
endif()

if(CYGWIN OR NOT LLVM_ENABLE_PIC)
  set(LLVM_TOOL_LTO_BUILD Off)
endif()

add_subdirectory(elf2encryptedhex)
add_subdirectory(kasirga)

add_llvm_implicit_projects()

set(LLVM_COMMON_DEPENDS ${LLVM_COMMON_DEPENDS} PARENT_SCOPE)
