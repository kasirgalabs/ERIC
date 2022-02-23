find_package(wxWidgets COMPONENTS stc core base REQUIRED)

include( "${wxWidgets_USE_FILE}" )

set(encIDEApp_SOURCE_FILES encIDEApp.h encIDEApp.cpp encIDEMain.h encIDEMain.cpp encIDEEnc.h encIDEEnc.cpp encIDESelBits.h encIDESelBits.cpp)

if(WIN32)
 set(encIDEApp_SOURCE_FILES ${encIDE_SOURCE_FILES} icon.rc)
endif(WIN32)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${encIDEApp_SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} ${wxWidgets_LIBRARIES})


if(NOT CONFIG)
    set(CONFIG "")
endif()

set(CONFIGSTR ${CONFIG})

unset(CONFIG CACHE)

# TODO Fix corresponding strings
if("${CONFIGSTR}" STREQUAL "")
    if(WIN32)
        file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/encide.conf
            "compilerpath=C:\\LLVM\\bin\\clang.exe\n"
            "riscvrootpath=C:\\riscv\\_install\\riscv64-unknown-elf\n"
            "riscvtargetoption=--riscv32\n"
            "extracompilerflags=-fnoaddrsig\n"
            "lastopenedfile=\n"
            "screenratio=0.8\n"
            "windowwidth=\n"
            "windowheight=\n"
            "windowpositionx=\n"
            "windowpositiony=\n"
            "zoomlevel=\n"
        )
    elseif(APPLE)
        file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/encide.conf
            "compilerpath=/usr/bin/clang\n"
            "riscvrootpath=/Users/shc/wx/encIDE/CMakeLists.txt\n"
            "riscvtargetoption=--riscv32\n"
            "extracompilerflags=-fnoaddrsig\n"
            "lastopenedfile=\n"
            "screenratio=0.8\n"
            "windowwidth=\n"
            "windowheight=\n"
            "windowpositionx=\n"
            "windowpositiony=\n"
            "zoomlevel=\n"
        )
    else()
        file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/encide.conf
            "compilerpath=/usr/bin/clang\n"
            "riscvrootpath=/home/shc/wx/encIDE/CMakeLists.txt\n"
            "riscvtargetoption=--riscv32\n"
            "extracompilerflags=-fnoaddrsig\n"
            "lastopenedfile=\n"
            "screenratio=0.8\n"
            "windowwidth=\n"
            "windowheight=\n"
            "windowpositionx=\n"
            "windowpositiony=\n"
            "zoomlevel=\n"
        )
    endif()
else()
    file(WRITE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/encide.conf ${CONFIGSTR})
endif()
