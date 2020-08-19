cmake_minimum_required(VERSION 3.5)

PROJECT(CycloneDDS_Perftest CXX C)
ADD_DEFINITIONS( -DRTI_LANGUAGE_CPP_TRADITIONAL -DO3 -DRTI_LINUX -DRTI_UNIX -DPERFTEST_CYCLONEDDS)
LINK_DIRECTORIES()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/srcCpp ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/cycloneDDS)
set(CMAKE_CXX_STANDARD 11)

# Find requirements
find_package(CycloneDDS REQUIRED COMPONENTS idlc PATHS $ENV{CYCLONEDDSHOME})

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/Linux)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/Linux)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin/Linux)

#idlc_generate(perftestType_lib "srcIdl/perftest.idl" "-d tmp -D PERFTEST_CYCLONEDDS")

add_executable(CycloneDDSperftest_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/cycloneDDS/CycloneDDSImpl.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/cycloneDDS/perftest.c
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/ThreadPriorities.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/Parameter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/ParameterManager.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/CpuMonitor.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/PerftestPrinter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/PerftestTransport.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/Infrastructure_common.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/FileDataLoader.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/perftest_publisher.cxx)

target_link_libraries(CycloneDDSperftest_cpp CycloneDDS::ddsc -lpthread)
