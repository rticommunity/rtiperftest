###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2024
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description:
# ------------
# CMakeLists.txt for building Perftest with Connext Cert
#
################################################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.8.4)

INCLUDE(../resource/cmake/common_utils.cmake)

function(remove_unwanted_flags)
    function(remove_warning_flags_from var_name)
        set(current_flags "${${var_name}}")

        string(REGEX MATCHALL "[^ ]+" flags_list "${current_flags}")

        set(new_flags "")
        foreach(flag ${flags_list})
            if(NOT flag MATCHES "^-W.*")
                list(APPEND new_flags "${flag}")
            endif()
        endforeach()

        string(REPLACE ";" " " new_flags_str "${new_flags}")
        set(${var_name} "${new_flags_str}" PARENT_SCOPE)
    endfunction()

    # TODO: Remove these calls and the function once PERFTEST-388 is addressed
    remove_warning_flags_from(CMAKE_CXX_FLAGS)
    remove_warning_flags_from(CMAKE_C_FLAGS)

    string(REPLACE "-O0" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    string(REPLACE "-O0" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}" PARENT_SCOPE)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}" PARENT_SCOPE)
endfunction()

function(get_micro_major_version dds_config_h)
    if (NOT EXISTS ${dds_config_h})
        set(return_value -1 PARENT_SCOPE)
    endif()
    file(STRINGS ${dds_config_h} config_strings)
    foreach(l in ${config_strings})
        string(REPLACE "\\" "" lm ${l})
        if(${lm} MATCHES "^[ ]*#define[ ]+RTIME_DDS_VERSION_MAJOR[ ]+([0-9]+).*")
            set(maj ${CMAKE_MATCH_1})
        endif()
    endforeach()
    set(ts "")
    set(micro_major_version "${maj}" PARENT_SCOPE)
endfunction(get_micro_major_version)

LINK_DIRECTORIES()

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

PROJECT(Perftest_Cert CXX C)
SET(SOURCE_EXTENSION cxx)

remove_unwanted_flags()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3")

SET(RTIDDSGEN $ENV{RTIMEHOME}/rtiddsgen/scripts/rtiddsgen)
SET_PROPERTY(GLOBAL PROPERTY RULE_MESSAGES OFF)

SET(IDL_SOURCE_C perftest.idl)
IF (RTIME_ZEROCOPY_AVAILABLE)
    SET(IDL_SOURCE_C ${IDL_SOURCE_C} perftest_cert_zc.idl)
    add_compile_definitions(RTI_ZEROCOPY_AVAILABLE)
    add_definitions(-DRTIME_HAVE_ZCOPYV2=1)
ENDIF()

SET(IDL_GEN_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/gen)

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})

IF (NOT EXISTS gen)
    EXECUTE_PROCESS(COMMAND ${CMAKE_COMMAND} -E make_directory gen)
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

get_core_lib_dir_name(${RTIME_TARGET_NAME})

# If the architecture has CERT in it, enable Cert
string(REGEX MATCH "CERT" cert_match ${RTIME_TARGET_NAME})
IF (cert_match)
    ADD_DEFINITIONS(-DRTI_CERT)
ENDIF()

# Recovering ADDITIONAL_DEFINES from a file that should have been previously
# created and populated by build.sh
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/additional_defines.txt ADDITIONAL_DEFINES)

IF (DEFINED RTI_CERT OR cert_match)
    IF (RTIME_ZEROCOPY_AVAILABLE)
        list(APPEND MICRO_C_LIBS rti_me_netiozcopy${RTI_LIB_SUFFIX})
    ENDIF()
    IF(arch_is_pi)
        list(APPEND MICRO_C_LIBS rti_me_psl${RTI_LIB_SUFFIX})
    ENDIF()
    list(APPEND MICRO_C_LIBS rti_me${RTI_LIB_SUFFIX})
    IF(arch_is_pi)
        list(APPEND MICRO_C_LIBS rti_me_psl${RTI_LIB_SUFFIX})
    ENDIF()
    add_definitions(${ADDITIONAL_DEFINES} -DRTI_CERT -DPERFTEST_CERT)
ELSE()
    get_micro_major_version("$ENV{RTIMEHOME}/include/rti_me/dds_c/dds_c_config.h")

    set(MICRO_C_LIBS "")

    IF (RTIME_ZEROCOPY_AVAILABLE)
        list(APPEND MICRO_C_LIBS rti_me_netiozcopy${RTI_LIB_SUFFIX})
    ENDIF()

    IF (micro_major_version STREQUAL "4")
        list(APPEND MICRO_C_LIBS
                rti_me_netiosdm${RTI_LIB_SUFFIX}
                rti_me_netioshmem${RTI_LIB_SUFFIX}
                rti_me_ddsxtypes${RTI_LIB_SUFFIX}
                rti_me_netiopsl${RTI_LIB_SUFFIX} 
                rti_me_ospsl${RTI_LIB_SUFFIX} 
                                    )
    ENDIF()

    list(APPEND MICRO_C_LIBS rti_me_rhsm${RTI_LIB_SUFFIX}
                    rti_me_whsm${RTI_LIB_SUFFIX}
                    rti_me_discdpse${RTI_LIB_SUFFIX}
                    rti_me_discdpde${RTI_LIB_SUFFIX})

    IF(arch_is_pi)
        LIST(APPEND MICRO_C_LIBS
            rti_me_netiopsl${RTI_LIB_SUFFIX}
            rti_me_ospsl${RTI_LIB_SUFFIX}
            )
    ENDIF()

    IF (NOT micro_major_version STREQUAL "4")
        list(APPEND MICRO_C_LIBS
            rti_me_psl${RTI_LIB_SUFFIX}
        )
    ENDIF()

    LIST(APPEND MICRO_C_LIBS
        rti_me${RTI_LIB_SUFFIX}
    )

    IF(arch_is_pi)
        LIST(APPEND MICRO_C_LIBS
            rti_me_ospsl${RTI_LIB_SUFFIX})
    ENDIF()

    add_definitions(${ADDITIONAL_DEFINES} -DPERFTEST_CERT)
ENDIF()

INCLUDE_DIRECTORIES(gen/ $ENV{RTIMEHOME}/include $ENV{RTIMEHOME}/include/rti_me)

IF (${arch_is_pi})
    IF (RTIME_CERT)
        INCLUDE_DIRECTORIES($ENV{RTIMEHOME}/../rti_me_psl.1.0/include
            $ENV{RTIMEHOME}/../rti_me_psl.1.0/include/rti_me_psl)
    ELSE()
        INCLUDE_DIRECTORIES($ENV{RTIMEHOME}/include/rti_me/rti_me_psl)
    ENDIF()
ENDIF()

LINK_DIRECTORIES($ENV{RTIMEHOME}/lib/${RTIME_TARGET_NAME})
# If ${core_lib_dir_name} is not empty, we are working with a
# Platform Independent Cert
LINK_DIRECTORIES($ENV{RTIMEHOME}/lib/${core_lib_dir_name})

FOREACH(idl ${IDL_SOURCE_C})
    GET_FILENAME_COMPONENT(filename ${idl} NAME)
    STRING(REGEX REPLACE "\\.idl" "" basename ${filename})
    LIST(APPEND IDL_GEN_C ${basename}.c
                          ${basename}Plugin.c
                          ${basename}Support.c)

    LIST(APPEND IDL_GEN_H ${basename}.h
                          ${basename}Plugin.h
                          ${basename}Support.h)

ENDFOREACH()

FILE(GLOB MyCSources ${CMAKE_CURRENT_SOURCE_DIR}/*)
LIST(REMOVE_ITEM MyCSources 
        ${CMAKE_CURRENT_SOURCE_DIR}/perftest_subscriber.${SOURCE_EXTENSION}
        ${CMAKE_CURRENT_SOURCE_DIR}/perftest_publisher.${SOURCE_EXTENSION}
        ${CMAKE_CURRENT_SOURCE_DIR}/perftest_cpp.cxx)

if(${arch_is_pi})
    add_compile_definitions(RTI_CERT_IS_PI)
ENDIF()

ADD_EXECUTABLE(perftest_cpp_cert
                ${CMAKE_CURRENT_SOURCE_DIR}/perftest_cpp.${SOURCE_EXTENSION}
                ${MyCSources} ${IDL_GEN_C} ${IDL_GEN_H})

set_target_properties(perftest_cpp_cert PROPERTIES OUTPUT_NAME ${PERFTEST_EXECUTABLE_NAME})

TARGET_LINK_LIBRARIES(perftest_cpp_cert ${MICRO_C_LIBS} ${PLATFORM_LIBS})