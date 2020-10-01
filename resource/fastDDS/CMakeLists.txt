
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

PROJECT(FastDDS_Perftest CXX C)

ADD_DEFINITIONS( -DRTI_LANGUAGE_CPP_TRADITIONAL -DO3 -DRTI_LINUX -DRTI_UNIX -DPERFTEST_EPROSIMA_FASTDDS)
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

set(FASTDDS_LIBS fastcdr fastrtps)

ADD_EXECUTABLE(FastDDSperftest_cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/perftest.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/perftestPubSubTypes.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/fastDDS/FastDDSImpl.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/ThreadPriorities.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/Parameter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/ParameterManager.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/CpuMonitor.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCppCommon/PerftestPrinter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/PerftestTransport.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/Infrastructure_common.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/FileDataLoader.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/srcCpp/perftest_cpp.cxx)

TARGET_LINK_LIBRARIES(FastDDSperftest_cpp ${FASTDDS_LIBS})
