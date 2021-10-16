##
find_package(LLVM)
find_package(LLVM REQUIRED CONFIG)



include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

find_package(Clang)
find_package(Clang CONFIG)
include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINITIONS})
list(APPEND CMAKE_MODULE_PATH "${CLANG_CMAKE_DIR}")
include(AddClang)

##

set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  Analysis
  CodeGen
  Core
  IPO
  AggressiveInstCombine
  InstCombine
  Instrumentation
  MC
  MCParser
  ObjCARCOpts
  Option
  ScalarOpts
  Support
  TransformUtils
  Vectorize

  ##############################################ekledim
  #AllTargetsDescs
  #AllTargetsDisassemblers
  #AllTargetsInfos
  #BinaryFormat
  ##CodeGen
  #DebugInfoDWARF
  #DebugInfoPDB
  #Demangle
  ##MC
  #MCDisassembler
  #Object
  ##Support
  #Symbolize
  ##############################################
  )

  
option(CLANG_PLUGIN_SUPPORT "Build kasirga with plugin support" ON)

# Support plugins.
if(CLANG_PLUGIN_SUPPORT)
  set(support_plugins SUPPORT_PLUGINS)
endif()

if(NOT CLANG_BUILT_STANDALONE)
  set(tablegen_deps intrinsics_gen)
endif()

add_llvm_executable(kasirga
  driver.cpp
  cc1_main.cpp
  cc1as_main.cpp
  cc1gen_reproducer_main.cpp

  ##############################################ekledim
  #alp.cpp
  #COFFDump.cpp
  #ELFDump.cpp
  #MachODump.cpp
  #WasmDump.cpp
  #XCOFFDump.cpp

  #DEPENDS
  #intrinsics_gen
  ##############################################

  DEPENDS
  ##############################################ekledim
  #intrinsics_gen
  ##############################################
  ${tablegen_deps}
  ${support_plugins}
  )

clang_target_link_libraries(kasirga
  PRIVATE
  clangBasic
  clangCodeGen
  clangDriver
  clangFrontend
  clangFrontendTool
  clangSerialization
  )

if(WIN32 AND NOT CYGWIN)
  # Prevent versioning if the buildhost is targeting for Win32.
else()
  set_target_properties(kasirga PROPERTIES VERSION "${CLANG_EXECUTABLE_VERSION}")
endif()

# Support plugins.
if(CLANG_PLUGIN_SUPPORT)
  export_executable_symbols_for_plugins(kasirga)
endif()

#add_dependencies(kasirga clang-resource-headers)

if(NOT CLANG_LINKS_TO_CREATE)
  set(CLANG_LINKS_TO_CREATE kasirga++ kasirga-cl kasirga-cpp)
endif()

foreach(link ${CLANG_LINKS_TO_CREATE})
  add_clang_symlink(${link} kasirga)
endforeach()

# Configure plist creation for OS X.
set (TOOL_INFO_PLIST "Info.plist" CACHE STRING "Plist name")
if (APPLE)
  if (CLANG_VENDOR)
    set(TOOL_INFO_NAME "${CLANG_VENDOR} kasirga")
  else()
    set(TOOL_INFO_NAME "kasirga")
  endif()

  set(TOOL_INFO_UTI "${CLANG_VENDOR_UTI}")
  set(TOOL_INFO_VERSION "${CLANG_VERSION}")
  set(TOOL_INFO_BUILD_VERSION "${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR}")

  set(TOOL_INFO_PLIST_OUT "${CMAKE_CURRENT_BINARY_DIR}/${TOOL_INFO_PLIST}")
  target_link_libraries(kasirga ############
    PRIVATE
    "-Wl,-sectcreate,__TEXT,__info_plist,${TOOL_INFO_PLIST_OUT}")
  configure_file("${TOOL_INFO_PLIST}.in" "${TOOL_INFO_PLIST_OUT}" @ONLY)

  set(TOOL_INFO_UTI)
  set(TOOL_INFO_NAME)
  set(TOOL_INFO_VERSION)
  set(TOOL_INFO_BUILD_VERSION)
endif()

if(CLANG_ORDER_FILE AND
    (LLVM_LINKER_IS_LD64 OR LLVM_LINKER_IS_GOLD OR LLVM_LINKER_IS_LLD))
  include(CheckLinkerFlag)

  if (LLVM_LINKER_IS_LD64)
    set(LINKER_ORDER_FILE_OPTION "-Wl,-order_file,${CLANG_ORDER_FILE}")
  elseif (LLVM_LINKER_IS_GOLD)
    set(LINKER_ORDER_FILE_OPTION "-Wl,--section-ordering-file,${CLANG_ORDER_FILE}")
  elseif (LLVM_LINKER_IS_LLD)
    set(LINKER_ORDER_FILE_OPTION "-Wl,--symbol-ordering-file,${CLANG_ORDER_FILE}")
  endif()

  # This is a test to ensure the actual order file works with the linker.
  check_linker_flag(${LINKER_ORDER_FILE_OPTION} LINKER_ORDER_FILE_WORKS)

  # Passing an empty order file disables some linker layout optimizations.
  # To work around this and enable workflows for re-linking when the order file
  # changes we check during configuration if the file is empty, and make it a
  # configuration dependency.
  file(READ ${CLANG_ORDER_FILE} ORDER_FILE LIMIT 20)
  if("${ORDER_FILE}" STREQUAL "\n")
    set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CLANG_ORDER_FILE})
  elseif(LINKER_ORDER_FILE_WORKS)
    target_link_libraries(kasirga PRIVATE ${LINKER_ORDER_FILE_OPTION})
    set_target_properties(kasirga PROPERTIES LINK_DEPENDS ${CLANG_ORDER_FILE})
  endif()
endif()

