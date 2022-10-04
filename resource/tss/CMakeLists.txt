###############################################################################
# (c) Copyright, Real-Time Innovations 2018-2022
#
# All rights reserved.
# No duplications, whole or partial, manual or electronic, may be made
# without express written permission.  Any such copies, or
# revisions thereof, must display this notice unaltered.
# This code contains trade secrets of Real-Time Innovations, Inc.
#
# Description:
# ------------
# CMakeLists.txt for building perftest with Connext DDS Pro or Micro
#
################################################################################

cmake_minimum_required(VERSION 3.8)
cmake_policy(SET CMP0048 NEW) # Support VERSION in project
if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW) # Support *_ROOT variables
endif()

# Build configuration variables. To overwrite the default value, run cmake with
# -Dvar=value. These two variables must be before the project call since
# toolchains need them.
# option(BUILD_SHARED_LIBS "Libraries type; TRUE for shared, FALSE for static" FALSE)
set(CMAKE_BUILD_TYPE "debug" CACHE STRING "Build type: 'Debug' or 'Release'")
option(RTI_REPLACE_GEN "Replace generated source? TRUE to replace, FALSE to keep" TRUE)

set (CMAKE_CXX_STANDARD 11)

set(RTI_CONNEXT_PRO_TYPE "pro")
set(RTI_CONNEXT_MICRO_TYPE "micro")

set(RTI_CONNEXT_TYPE ${RTI_CONNEXT_MICRO_TYPE} CACHE STRING "Connext DDS product type: 'pro' or 'micro'")
set_property(CACHE RTI_CONNEXT_TYPE PROPERTY STRINGS
                 ${RTI_CONNEXT_PRO_TYPE}
                 ${RTI_CONNEXT_MICRO_TYPE})

set(RTI_TSS_MICRO_2_4_11_1_SUPPORT FALSE CACHE BOOL "Set to TRUE if compiling against Micro 2.4.11.1")

# Prerequisite environment variables
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    if (NOT DEFINED ENV{NDDSHOME})
        message(FATAL_ERROR "[RTI] NDDSHOME is not set. Please set to Connext DDS Pro installation directory.")
    endif()

    file(TO_CMAKE_PATH "$ENV{NDDSHOME}" CONNEXTHOME)
    file(TO_CMAKE_PATH "$ENV{NDDSHOME}" NDDSHOME)
    set(NDDSHOME $ENV{NDDSHOME})
    set(RTIDDSGEN_OPTS "")
else()
    if (NOT DEFINED ENV{RTIMEHOME})
        message(FATAL_ERROR "[RTI] RTIMEHOME is not set. Please set to Connext DDS Micro installation directory.")
    endif()

    file(TO_CMAKE_PATH "$ENV{RTIMEHOME}" CONNEXTHOME)
    file(TO_CMAKE_PATH "$ENV{RTIMEHOME}" RTIMEHOME)
    set(RTIDDSGEN_OPTS "-micro")
    add_definitions(-DRTI_CONNEXT_MICRO)
endif()

if (NOT DEFINED RTITSSHOME)
    if (NOT DEFINED ENV{RTITSSHOME})
        message(FATAL_ERROR "[RTI] RTITSSHOME is not set. Please set to Connext TSS installation directory")
    endif()

    file(TO_CMAKE_PATH "$ENV{RTITSSHOME}" RTITSSHOME)
endif()

if (NOT DEFINED RTI_TSS_PLATFORM)
    if (NOT DEFINED ENV{RTITSSARCH})
        message(FATAL_ERROR "[RTI] RTITSSARCH is not set. Please set to the name of the platform architecture (e.g. x64Linux3gcc4.8.2).")
    endif()

    file(TO_CMAKE_PATH "$ENV{RTITSSARCH}" RTI_TSS_PLATFORM)
endif()

# Pro Platform architecture must be set in NDDSARCH environment variable
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    if(NOT DEFINED RTI_NDDS_ARCH)
        if(NOT DEFINED ENV{NDDSARCH})
            message(Warning "[RTI] Pro Architecture not specified. Please define a CMake variable RTI_NDDS_ARCH or
                    set environment variable NDDSARCH to use a specific Pro Architecture. Setting Pro Architecture to RTITSSARCH.")
            set(RTIPROARCH ${RTI_TSS_PLATFORM})
        else()
            set(RTIPROARCH $ENV{NDDSARCH})
        endif()
    else()
        set(RTIPROARCH ${RTI_NDDS_ARCH})
    endif()
endif()

#Micro Platform architecture must be set in RTIMEARCH environment variable
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_MICRO_TYPE})
    if(NOT DEFINED RTI_ME_ARCH)
        if(NOT DEFINED ENV{RTIMEARCH})
            message(Warning "[RTI] Micro Architecture not specified. Please define a CMake variable RTI_ME_ARCH or
            set environment variable RTIMEARCH to use a specific Micro Architecture. Setting Micro Architecture to RTITSSARCH.")
            set(RTIMEARCH ${RTI_TSS_PLATFORM})
        else()
            set(RTIMEARCH $ENV{RTIMEARCH})
        endif()
    else()
        set(RTIMEARCH ${RTI_ME_ARCH})
    endif()
endif()

set(DDSGENHOME ${RTITSSHOME})

# We need to setup the module path before 'project()' loads any file.
list(APPEND CMAKE_MODULE_PATH "${RTITSSHOME}/resource/cmake/Modules")
list(APPEND CMAKE_MODULE_PATH "${RTITSSHOME}/resource/cmake/Toolchains")

# Toolchain (host or cross-compilation) is included if available
set(RTI_TSS_ARCH_TOOLCHAIN
     ${RTITSSHOME}/resource/cmake/Toolchains/Architecture/${RTI_TSS_PLATFORM}.cmake)


include(${RTI_TSS_ARCH_TOOLCHAIN} OPTIONAL RESULT_VARIABLE FOUND_TOOLCHAIN)
if (FOUND_TOOLCHAIN)
    message(STATUS "[RTI] Included toolchain ......: ${FOUND_TOOLCHAIN}")
endif()


# Support only C and C++
set(project_langs C CXX)


project(perftest
        LANGUAGES ${project_langs})


include(ConnextTss)

# Toolchain for this platform
include(Architecture/${RTI_TSS_PLATFORM} OPTIONAL)

message(STATUS "[RTI] RTI_CONNEXT_TYPE ................: ${RTI_CONNEXT_TYPE}")
message(STATUS "[RTI] RTITSSHOME ......................: ${RTITSSHOME}")
message(STATUS "[RTI] CONNEXTHOME .....................: ${CONNEXTHOME}")
message(STATUS "[RTI] CMAKE_BUILD_TYPE ................: ${CMAKE_BUILD_TYPE}")
message(STATUS "[RTI] PLATFORM_ARCH ...................: ${RTI_TSS_PLATFORM}")
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    message(STATUS "[RTI] PRO_PLATFORM_ARCH ...............: ${RTIPROARCH}")
endif()
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_MICRO_TYPE})
    message(STATUS "[RTI] MICRO_PLATFORM_ARCH .............: ${RTIMEARCH}")
endif()
message(STATUS "[RTI] RTI_TSS_ENABLE_FACE_COMPLIANCE...: ${RTI_TSS_ENABLE_FACE_COMPLIANCE}")
message(STATUS "[RTI] RTI_REPLACE_GEN..................: ${RTI_REPLACE_GEN}")

# External libraries

if(NOT WIN32)
set(LIB_PREFIX "lib")
endif()

# Connext Pro
set(nddscore_lib        ${LIB_PREFIX}nddscore)
set(nddsc_lib           ${LIB_PREFIX}nddsc)
set(nddscpp_lib         ${LIB_PREFIX}nddscpp)
# Connext Micro
set(rtime_lib        ${LIB_PREFIX}rti_me)
set(rtime_rh_lib     ${LIB_PREFIX}rti_me_rhsm)
set(rtime_wh_lib     ${LIB_PREFIX}rti_me_whsm)
set(rtime_dpde_lib   ${LIB_PREFIX}rti_me_discdpde)
set(rtime_dpse_lib   ${LIB_PREFIX}rti_me_discdpse)
set(rtime_cpp_lib    ${LIB_PREFIX}rti_me_cpp)
# Connext TSS
set(rtitss_lib          ${LIB_PREFIX}rti_tsscpp)

# Link libs
set(ndds_libs_dir    ${NDDSHOME}/lib/${RTIPROARCH})
set(rtime_libs_dir    ${RTIMEHOME}/lib/${RTIMEARCH})
set(rtitss_libs_dir   ${RTITSSHOME}/lib/${RTI_TSS_PLATFORM}/${RTI_CONNEXT_TYPE})

# static libs only
if(CMAKE_BUILD_TYPE MATCHES "release")
if(WIN32)
    set(LIB_SUFFIX "z.lib")
else()
    set(LIB_SUFFIX "z.a")
endif()
else()
if(WIN32)
    set(LIB_SUFFIX "zd.lib")
else()
    set(LIB_SUFFIX "zd.a")
endif()
endif()


# Includes
if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    list(APPEND RTITSS_INC_DIRS  ${RTITSSHOME}/include
                                 ${RTITSSHOME}/include/pro)
    list(APPEND CONNEXT_INC_DIRS ${NDDSHOME}/include
                                 ${NDDSHOME}/include/ndds)
else()
    list(APPEND RTITSS_INC_DIRS  ${RTITSSHOME}/include
                                 ${RTITSSHOME}/include/micro)
    list(APPEND CONNEXT_INC_DIRS ${RTIMEHOME}/include
                                 ${RTIMEHOME}/include/rti_me)
endif()

SET(GEN_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../srcCpp/gen)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS
                    ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS/pro
                    ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS/micro
                    ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS/TSS
                    ${GEN_OUTPUT_DIR}
                    ${CONNEXT_INC_DIRS}
                    ${RTITSS_INC_DIRS})

# Generated source for user data type
set(rtiddsgen_cmd   ${RTITSSHOME}/bin/rtiddsgen)

MESSAGE(STATUS "[RTI] rtiddsgen_cmd: ${rtiddsgen_cmd}")

if (RTI_REPLACE_GEN)
  set(REPLACE_GEN "-replace")
else()
  set(REPLACE_GEN "")
endif()

set(PERF_GEN_IDL     ${CMAKE_CURRENT_SOURCE_DIR}/../srcIdl/perftest.idl)

set(PERF_GEN_SRC    ${GEN_OUTPUT_DIR}/perftest.cxx
                    ${GEN_OUTPUT_DIR}/perftestSupport.cxx
                    ${GEN_OUTPUT_DIR}/perftestPlugin.cxx)

set(PERF_GEN_HEADERS ${GEN_OUTPUT_DIR}/perftest.h
                     ${GEN_OUTPUT_DIR}/perftest.hpp
                     ${GEN_OUTPUT_DIR}/perftestSupport.h
                     ${GEN_OUTPUT_DIR}/perftestPlugin.h
                     ${GEN_OUTPUT_DIR}/FACE/DM/TestData_t/TypedTS_Impl.hpp
                     ${GEN_OUTPUT_DIR}/FACE/DM/TestDataLarge_t/TypedTS_Impl.hpp
                     ${GEN_OUTPUT_DIR}/FACE/DM/TestDataKeyed_t/TypedTS_Impl.hpp
                     ${GEN_OUTPUT_DIR}/FACE/DM/TestDataKeyedLarge_t/TypedTS_Impl.hpp
                     ${GEN_OUTPUT_DIR}/perftest_TSSConfigInterface.hpp)

list(APPEND PERF_CONFIG_SRC  ${GEN_OUTPUT_DIR}/perftest_TSSQosSupport.cpp)

list(APPEND PERF_GEN_TYPE_SRC
        ${GEN_OUTPUT_DIR}/FACE/DM/TestData_t/TypedTS_Impl.cpp
        ${GEN_OUTPUT_DIR}/FACE/DM/TestDataLarge_t/TypedTS_Impl.cpp
        ${GEN_OUTPUT_DIR}/FACE/DM/TestDataKeyed_t/TypedTS_Impl.cpp
        ${GEN_OUTPUT_DIR}/FACE/DM/TestDataKeyedLarge_t/TypedTS_Impl.cpp)

add_definitions(-DFACE_SEQUENCE_AND_STRING_IMPLEMENTED=TRUE)
add_definitions(-DFACE_SEQUENCE_AND_STRING_SERIALIZABLE=TRUE)

# Connext TSS and DDS plugin generation
ADD_CUSTOM_COMMAND(OUTPUT ${PERF_GEN_SRC} ${PERF_CONFIG_SRC} ${PERF_GEN_TYPE_SRC} ${PERF_GEN_HEADERS} ${PERF_DISC_SRC}
                          nonexistentFileToForceThisCmd
                   COMMAND ${CMAKE_COMMAND} -E make_directory ${GEN_OUTPUT_DIR}
                   COMMAND ${rtiddsgen_cmd} -d ${GEN_OUTPUT_DIR}
                       -I ${RTITSSHOME}/include -D RTI_PERF_TSS
                       ${REPLACE_GEN} ${RTIDDSGEN_OPTS} -language FACEC++  ${PERF_GEN_IDL}
                    COMMENT "Running rtiddsgen (FACEC++) for ${PERF_GEN_IDL}"
                    DEPENDS ${PERF_GEN_IDL})

#Additional files to be copied
list(APPEND TSS_INFRASTRUCTURE_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS/TSS/CustomQosSupport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS/TSS/RTITSSImpl.cxx)

# perftest application
list(APPEND PERF_APP_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/perftest_cpp.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/connextDDS/${RTI_CONNEXT_TYPE}/Infrastructure_${RTI_CONNEXT_TYPE}.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.cxx
        ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.cxx
        ${TSS_INFRASTRUCTURE_FILES}
        ${PERF_CONFIG_SRC}
        ${PERF_GEN_SRC}
        ${PERF_GEN_TYPE_SRC}
        ${PERF_DISC_SRC})

set(APP_NAME perftest_cpp_tss_${RTI_CONNEXT_TYPE})
add_executable(${APP_NAME} ${PERF_APP_SRC})

if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    # add_definitions( -DPERFTEST_RTI_PRO )
    add_definitions( -DRTI_PERF_TSS_PRO )
else()
    # add_definitions(-DPERFTEST_RTI_MICRO )
    add_definitions( -DRTI_PERF_TSS_MICRO )
endif()

add_definitions( -DRTI_LANGUAGE_CPP_TRADITIONAL -O0 -DRTI_PERF_TSS -DRTI_USE_CPP_11_INFRASTRUCTURE)

if(${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_MICRO_TYPE})
    add_definitions( -DPERFTEST_RTI_MICRO_24x_COMPATIBILITY )
endif()

if (${RTI_CONNEXT_TYPE} MATCHES ${RTI_CONNEXT_PRO_TYPE})
    target_link_libraries(${APP_NAME}
                        PRIVATE ${rtitss_libs_dir}/${rtitss_lib}${LIB_SUFFIX}
                                ${ndds_libs_dir}/${nddscpp_lib}${LIB_SUFFIX}
                                ${ndds_libs_dir}/${nddsc_lib}${LIB_SUFFIX}
                                ${ndds_libs_dir}/${nddscore_lib}${LIB_SUFFIX}
                                ${SYSLIBS_ARCH})
elseif (((${RTI_TSS_ENABLE_FACE_COMPLIANCE} MATCHES "SafetyBase") OR
            (${RTI_TSS_ENABLE_FACE_COMPLIANCE} MATCHES "Security")) AND
        NOT ${RTI_TSS_MICRO_2_4_11_1_SUPPORT})
    target_link_libraries(${APP_NAME}
                        PRIVATE ${rtitss_libs_dir}/${rtitss_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_lib}${LIB_SUFFIX}
                                ${SYSLIBS_ARCH})
else()
    target_link_libraries(${APP_NAME}
                        PRIVATE ${rtitss_libs_dir}/${rtitss_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_rh_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_wh_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_dpde_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_dpse_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_lib}${LIB_SUFFIX}
                                ${rtime_libs_dir}/${rtime_cpp_lib}${LIB_SUFFIX}
                                ${SYSLIBS_ARCH})
endif()

target_compile_options(${APP_NAME} PRIVATE -fexceptions)

set(APP_OUTPUT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin/TSS/${RTI_TSS_PLATFORM}/${RTI_CONNEXT_TYPE}/${CMAKE_BUILD_TYPE})
add_custom_command(TARGET ${APP_NAME}
                   POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${APP_OUTPUT_DIR}
                    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${APP_NAME}> ${APP_OUTPUT_DIR}
                    COMMENT "Copying application to ${APP_OUTPUT_DIR} ...")
