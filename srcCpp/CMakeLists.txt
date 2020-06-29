
CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

ADD_DEFINITIONS( -DRTI_LANGUAGE_CPP_TRADITIONAL -DO3 -DRTI_LINUX -DRTI_UNIX -DFASTDDS -DEPROSIMA_PERF_FASTDDS)
LINK_DIRECTORIES()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(perftest_FastDDS CXX C)
SET(SOURCE_EXTENSION cxx)

# SET(RTIDDSGEN $ENV{RTIMEHOME}/rtiddsgen/scripts/rtiddsgen)
# SET(RTIDDSMAG $ENV{RTIMEHOME}/rtiddsmag/scripts/rtiddsmag)
SET(RTIDDSGEN ~/rti_me.2.0/rtiddsgen/scripts/rtiddsgen)
SET(RTIDDSMAG ~/rti_me.2.0/rtiddsmag/scripts/rtiddsmag)

SET_PROPERTY(GLOBAL PROPERTY RULE_MESSAGES OFF)

SET(IDL_SOURCE_C perftest.idl)

SET(IDL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(XML_PATH ${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/x64Linux4gcc7.3.0)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/x64Linux4gcc7.3.0)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/x64Linux4gcc7.3.0)
# SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
# SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
# SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})

IF (CMAKE_GENERATOR MATCHES "Visual Studio")
    #Visual studio needs to run commands using 'call'
    #if there is more than one dependency
    SET(COMMAND_PREFIX call)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT.lib" CACHE INTERNAL "CMAKE_EXE_LINKER_FLAGS" FORCE)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ELSE()
    SET(COMMAND_PREFIX)
ENDIF()

SET(RTI_LIB_PREFIX rti_me)
SET(RTI_LIB_SUFFIX)

IF (NOT (DEFINED RTI_SHARED_LIB))
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "z")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "zd")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()
ELSE()
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "d")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()
ENDIF()

IF (DEFINED RTIME_MAG_FILES)
    SET(MICRO_APPGEN_LIB rti_me_appgen${RTI_LIB_SUFFIX})
ENDIF()

SET(MICRO_C_LIBS ${MICRO_APPGEN_LIB}
                 rti_me_cpp${RTI_LIB_SUFFIX}
                 rti_me_netioshmem${RTI_LIB_SUFFIX}
                 rti_me_netiosdm${RTI_LIB_SUFFIX}
                 rti_me_rhsm${RTI_LIB_SUFFIX}
                 rti_me_whsm${RTI_LIB_SUFFIX}
                 rti_me_discdpse${RTI_LIB_SUFFIX}
                 rti_me_discdpde${RTI_LIB_SUFFIX}
                 rti_me${RTI_LIB_SUFFIX}
                                  )

MESSAGE("RTIME_TARGET_NAME = x64Linux4gcc7.3.0")
# INCLUDE_DIRECTORIES(gen/ $ENV{RTIMEHOME}/include $ENV{RTIMEHOME}/include/rti_me)
# LINK_DIRECTORIES($ENV{RTIMEHOME}/lib/${RTIME_TARGET_NAME})
INCLUDE_DIRECTORIES(gen/ ~/rti_me.2.0/include ~/rti_me.2.0/include/rti_me)
LINK_DIRECTORIES(~/rti_me.2.0/lib/x64Linux4gcc7.3.0)

ADD_EXECUTABLE(perftest_cpp_FastDDS
               ${CMAKE_CURRENT_SOURCE_DIR}/perftest_publisher.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/FastDDSImpl.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_fastDDS.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.cxx
               ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.cxx
               ${IDL_GEN_C}
               ${IDL_GEN_H}
               ${APP_GEN_C}
               ${APP_GEN_H})

TARGET_LINK_LIBRARIES(perftest_cpp_FastDDS  ${MICRO_C_LIBS} ${PLATFORM_LIBS})