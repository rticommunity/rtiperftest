
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

PROJECT(FastDDS_Perftest CXX C)

ADD_DEFINITIONS( -DRTI_LANGUAGE_CPP_TRADITIONAL -DO3 -DRTI_LINUX -DRTI_UNIX -DPERTEST_EPROSIMA_FASTDDS)
LINK_DIRECTORIES()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/srcCpp ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS)
set(CMAKE_CXX_STANDARD 11)
set(SOURCE_EXTENSION cxx)

set(FASDDSGEN fastrtpsgen)

# Find requirements
find_package(fastcdr REQUIRED)
find_package(fastrtps REQUIRED)

SET_PROPERTY(GLOBAL PROPERTY RULE_MESSAGES OFF)

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
    rti_me_cpp${RTI_LIB_SUFFIX}
    rti_me_netioshmem${RTI_LIB_SUFFIX}
    rti_me_netiosdm${RTI_LIB_SUFFIX}
    rti_me_rhsm${RTI_LIB_SUFFIX}
    rti_me_whsm${RTI_LIB_SUFFIX}
    rti_me_discdpse${RTI_LIB_SUFFIX}
    rti_me_discdpde${RTI_LIB_SUFFIX}
    rti_me${RTI_LIB_SUFFIX})

set(FASTDDS_LIBS fastcdr fastrtps)

# This will not be needed once we take out the Micro dependency
set(PLATFORM_LIBS dl;m;pthread;)
INCLUDE_DIRECTORIES($ENV{RTIMEHOME}/include $ENV{RTIMEHOME}/include/rti_me)
LINK_DIRECTORIES(/performance/rtimehome/rti_me.2.0/lib/x64Linux4gcc7.3.0)

ADD_EXECUTABLE(FastDDSperftest_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/perftest.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/perftestPubSubTypes.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/FastDDSImpl.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/Infrastructure_fastDDS.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/ThreadPriorities.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/Parameter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/ParameterManager.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/CpuMonitor.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/PerftestPrinter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/PerftestTransport.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/Infrastructure_common.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/FileDataLoader.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/perftest_publisher.cxx)

TARGET_LINK_LIBRARIES(FastDDSperftest_cpp  ${MICRO_C_LIBS} ${FASTDDS_LIBS} ${PLATFORM_LIBS})
