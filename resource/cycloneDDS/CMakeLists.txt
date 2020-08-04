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

# This can be removed once we stop depending on Micro Osapi to build.
set(RTI_LIB_PREFIX rti_me)
set(RTI_LIB_SUFFIX)

IF(NOT (DEFINED RTI_SHARED_LIB))
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        set(RTI_LIB_SUFFIX "z")
        set(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        set(RTI_LIB_SUFFIX "zd")
        set(RTI_DEBUG_BUILD TRUE)
    ENDIF()
ELSE()
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        set(RTI_LIB_SUFFIX "")
        set(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        set(RTI_LIB_SUFFIX "d")
        set(RTI_DEBUG_BUILD TRUE)
    ENDIF()
ENDIF()

set(MICRO_C_LIBS
    rti_me${RTI_LIB_SUFFIX})

set(FASTDDS_LIBS fastcdr fastrtps)

# This will not be needed once we take out the Micro dependency
set(PLATFORM_LIBS dl;m;pthread;)
INCLUDE_DIRECTORIES($ENV{RTIMEHOME}/include $ENV{RTIMEHOME}/include/rti_me)
LINK_DIRECTORIES($ENV{RTIMEHOME}/lib/x64Darwin14clang6.0)


#idlc_generate(perftestType_lib "srcIdl/perftest.idl" "-d tmp -D PERFTEST_CYCLONEDDS")

ADD_EXECUTABLE(CycloneDDSperftest_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/cycloneDDS/CycloneDDSImpl.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/cycloneDDS/Infrastructure_cycloneDDS.cxx
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

TARGET_LINK_LIBRARIES(CycloneDDSperftest_cpp ${MICRO_C_LIBS} CycloneDDS::ddsc ${PLATFORM_LIBS})
