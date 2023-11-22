cmake_minimum_required(VERSION 2.8.12)

if(NOT CMAKE_VERSION VERSION_LESS 3.0)
    cmake_policy(SET CMP0048 NEW)
endif()
# src/fastdds_exp/C++/DDS/DiffMsgMultiThreadMixedFlowControl
project(DiffMsgMultiThreadMixedFlowControl)
# DiffMsgMultiThreadMixedFlowControl
# DiffMsgMultiThreadMixedFlowControl

# Find requirements
if(NOT fastcdr_FOUND)
    find_package(fastcdr REQUIRED)
endif()

if(NOT foonathan_memory_FOUND)
    find_package(foonathan_memory REQUIRED)
endif()

if(NOT fastrtps_FOUND)
    find_package(fastrtps REQUIRED)
endif()

#Check C++11
include(CheckCXXCompilerFlag)
if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANG OR
        CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    check_cxx_compiler_flag(-std=c++11 SUPPORTS_CXX11)
    if(NOT SUPPORTS_CXX11)
        message(FATAL_ERROR "Compiler doesn't support C++11")
    endif()
endif()

message(STATUS "Configuring DDS Flow Control example...")
file(GLOB DDS_FLOWCONTROL_EXAMPLE_SOURCES_CXX "*.cxx" "msg/*.cxx")
file(GLOB DDS_FLOWCONTROL_EXAMPLE_SOURCES_CPP "*.cpp" "msg/*.cpp")

add_executable(DiffMsgMultiThreadMixedFlowControl ${DDS_FLOWCONTROL_EXAMPLE_SOURCES_CXX} ${DDS_FLOWCONTROL_EXAMPLE_SOURCES_CPP})
target_compile_definitions(DiffMsgMultiThreadMixedFlowControl PRIVATE
    $<$<AND:$<NOT:$<BOOL:${WIN32}>>,$<STREQUAL:"${CMAKE_BUILD_TYPE}","Debug">>:__DEBUG>
    $<$<BOOL:${INTERNAL_DEBUG}>:__INTERNALDEBUG> # Internal debug activated.
    )
target_link_libraries(DiffMsgMultiThreadMixedFlowControl fastrtps fastcdr foonathan_memory)
install(TARGETS DiffMsgMultiThreadMixedFlowControl
    RUNTIME DESTINATION examples/C++/DDS/DiffMsgMultiThreadMixedFlowControl/${BIN_INSTALL_DIR})
