cmake_minimum_required(VERSION 3.13.4)
project(clangport)


set(LT_LLVM_INSTALL_DIR "" CACHE PATH "LLVM installation directory")

# A bit of a sanity checking
set(LT_LLVM_INCLUDE_DIR "${LT_LLVM_INSTALL_DIR}/include/llvm")
if(NOT EXISTS "${LT_LLVM_INCLUDE_DIR}")
message(FATAL_ERROR
  " LT_LLVM_INSTALL_DIR (${LT_LLVM_INCLUDE_DIR}) is invalid.")
endif()

set(LT_LLVM_CMAKE_FILE "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/LLVMConfig.cmake")

#
set(CT_LLVM_CMAKE_FILE "${LT_LLVM_INSTALL_DIR}/lib/cmake/clang/ClangConfig.cmake")
#

if(NOT EXISTS "${LT_LLVM_CMAKE_FILE}")

get_filename_component(UPPER_DIR ${LT_LLVM_INSTALL_DIR} DIRECTORY)
set(LT_LLVM_CMAKE_FILE "${UPPER_DIR}/build/lib/cmake/llvm/LLVMConfig.cmake")
#
set(CT_LLVM_CMAKE_FILE "${UPPER_DIR}/build/lib/cmake/clang/ClangConfig.cmake")
#
endif()
##

if(NOT EXISTS "${LT_LLVM_CMAKE_FILE}")
message(FATAL_ERROR
    " LT_LLVM_CMAKE_FILE (${LT_LLVM_CMAKE_FILE}) is invalid.")
endif()


list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/llvm/")
#
list(APPEND CMAKE_PREFIX_PATH "${LT_LLVM_INSTALL_DIR}/lib/cmake/clang/")
#

find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LT_LLVM_INSTALL_DIR}")

message("LLVM STATUS:
  Definitions ${LLVM_DEFINITIONS}
  Includes    ${LLVM_INCLUDE_DIRS}
  Libraries   ${LLVM_LIBRARY_DIRS}
  Targets     ${LLVM_TARGETS_TO_BUILD}"
)

# Set the LLVM header and library paths
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
##

get_filename_component(LT_LLVM_PROJECT_DIR ${LT_LLVM_INSTALL_DIR} DIRECTORY)

include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/build/include")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/build/tools/clang")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/build/tools/clang/include")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/build/tools/clang/lib")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/build/tools/clang/lib/Driver")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/llvm/include")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang/include")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang/lib")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang/lib/Tooling")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang/lib/Driver")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang/lib/Driver/ToolChains")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/clang/lib/Driver/ToolChains/Arch")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/llvm/lib/Target/RISCV")
include_directories(SYSTEM "${LT_LLVM_PROJECT_DIR}/build/lib/Target/RISCV")

link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_DEFINITIONS})


set(CMAKE_CXX_STANDARD 14 CACHE STRING "")


if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE
      STRING "Build type (default Debug):" FORCE)
endif()


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall\
    -fdiagnostics-color=always") #-nostartfiles


if(NOT LLVM_ENABLE_RTTI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")
endif()


include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-fvisibility-inlines-hidden" SUPPORTS_FVISIBILITY_INLINES_HIDDEN_FLAG)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")


add_subdirectory(src)
