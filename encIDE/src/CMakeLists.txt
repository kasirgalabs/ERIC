find_package(wxWidgets COMPONENTS aui adv html xml net richtext stc core base REQUIRED)

include( "${wxWidgets_USE_FILE}" )

set(encIDE_SOURCE_FILES encIDEApp.cpp encIDEApp.h encIDEenc.cpp encIDEenc.h encIDEMain.cpp encIDEMain.h encIDEselBits.cpp encIDEselBits.h)

if(WIN32)
 set(encIDE_SOURCE_FILES ${encIDE_SOURCE_FILES} icon.rc)
endif(WIN32)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(${CMAKE_PROJECT_NAME} WIN32 ${encIDE_SOURCE_FILES})

target_link_libraries(${CMAKE_PROJECT_NAME} ${wxWidgets_LIBRARIES})

