CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

function(SET_LIB_ORDER INPUT_LIST_VAR)
    # The target library order
    SET(TARGET_LIB_ORDER
        rti_me_appgen${RTI_LIB_SUFFIX}
        rti_me_cpp${RTI_LIB_SUFFIX}
        rti_me_netiosdm${RTI_LIB_SUFFIX}
        rti_me_netioshmem${RTI_LIB_SUFFIX}
        rti_me_netiozcopy${RTI_LIB_SUFFIX}
        rti_me_rhsm${RTI_LIB_SUFFIX}
        rti_me_whsm${RTI_LIB_SUFFIX}
        rti_me_discdpse${RTI_LIB_SUFFIX}
        rti_me_discdpde${RTI_LIB_SUFFIX}
        rti_me_netiopsl_cpp${RTI_LIB_SUFFIX}
        rti_me_netiopsl${RTI_LIB_SUFFIX}
        rti_me_ddsfilter${RTI_LIB_SUFFIX}
        rti_me_ddsxtypes${RTI_LIB_SUFFIX}
        rti_me${RTI_LIB_SUFFIX}
        rti_me_ospsl${RTI_LIB_SUFFIX}
        rti_me_ddspsk${RTI_LIB_SUFFIX}
        rti_me_pskpsl${RTI_LIB_SUFFIX}
    )

    SET(INPUT_LIST ${${INPUT_LIST_VAR}})

    # Remove duplicates
    LIST(LENGTH INPUT_LIST LIST_LENGTH)
    IF(LIST_LENGTH GREATER 0)
        LIST(REMOVE_DUPLICATES INPUT_LIST)
    ENDIF()

    # Reorder the items based on the target order
    SET(REORDERED_LIBS)
    FOREACH(item IN LISTS TARGET_LIB_ORDER)
        IF(item IN_LIST INPUT_LIST)
            LIST(APPEND REORDERED_LIBS ${item})
        ENDIF()
    ENDFOREACH()

    SET(${INPUT_LIST_VAR} ${REORDERED_LIBS} PARENT_SCOPE)
endfunction()

IF (NOT DEFINED RTIMEHOME)
    IF (DEFINED ENV{RTIMEHOME})
        SET(RTIMEHOME $ENV{RTIMEHOME})
    ELSE()
        MESSAGE(FATAL_ERROR "RTIMEHOME must be defined or set as an environment variable.")
    ENDIF()
ENDIF()
MESSAGE(STATUS "[RTI] RTIMEHOME ..................: ${RTIMEHOME}")

IF (NOT DEFINED RTIME_TARGET_NAME)
    IF (DEFINED ENV{RTIME_TARGET_NAME})
        SET(RTIME_TARGET_NAME $ENV{RTIME_TARGET_NAME})
    ELSE()
        MESSAGE(FATAL_ERROR "RTIME_TARGET_NAME must be defined or set as an environment variable.")
    ENDIF()
ENDIF()
MESSAGE(STATUS "[RTI] RTIME_TARGET_NAME ..........: ${RTIME_TARGET_NAME}")

ADD_DEFINITIONS(${ADDITIONAL_DEFINES})

SET(FIND_RESULT -1)
STRING(FIND "${ADDITIONAL_DEFINES}" "PERFTEST_RTI_MICRO_24x_COMPATIBILITY" FIND_RESULT)
IF (NOT FIND_RESULT EQUAL -1)
    SET(24x_COMPATIBILITY_SPECIFIED TRUE)
ELSE()
    SET(24x_COMPATIBILITY_SPECIFIED FALSE)
ENDIF()
STRING(FIND "${ADDITIONAL_DEFINES}" "RTI_LW_SECURE_PERFTEST" FIND_RESULT)
IF (NOT FIND_RESULT EQUAL -1)
    SET(RTIME_LIGHTWEIGHT_SECURE TRUE)
ELSE()
    SET(RTIME_LIGHTWEIGHT_SECURE FALSE)
ENDIF()

IF (NOT (DEFINED RTI_SHARED_LIB))
    SET(RTI_SHARED_LIB FALSE)
ENDIF()

MESSAGE(STATUS "[RTI] RTI_SHARED_LIB .............: ${RTI_SHARED_LIB}")
MESSAGE(STATUS "[RTI] RTIME_LIGHTWEIGHT_SECURE ...: ${RTIME_LIGHTWEIGHT_SECURE}")
MESSAGE(STATUS "[RTI] EXCLUDE_SHMEM ..............: ${EXCLUDE_SHMEM}")
MESSAGE(STATUS "[RTI] EXCLUDE_ZCV2 ...............: ${EXCLUDE_ZCV2}")
message(STATUS "[RTI] ADDITIONAL_DEFINES .........: ${ADDITIONAL_DEFINES}")

SET(RTI_LIB_PREFIX librti_me_)
SET(RTI_LIB_SUFFIX)
SET(RTI_LIB_SUFFIX_SSL)

IF (NOT RTI_SHARED_LIB)
    IF (${CMAKE_BUILD_TYPE} MATCHES "[Rr]elease")
        SET(RTI_LIB_SUFFIX "z")
        SET(RTI_DEBUG_BUILD FALSE)
        ADD_DEFINITIONS(-DNDEBUG)
    ELSE()
        SET(RTI_LIB_SUFFIX "zd")
        SET(RTI_DEBUG_BUILD TRUE)
    ENDIF()
    SET(RTI_LIB_SUFFIX_SSL "z")
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

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(perftest CXX C)
SET(SOURCE_EXTENSION cxx)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    SET(PLATFORM_LIBS m pthread rt)
    ADD_DEFINITIONS(-DRTI_LINUX)
ELSEIF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    SET(PLATFORM_LIBS dl m pthread )
    ADD_DEFINITIONS(-DRTI_DARWIN)
ELSEIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    SET(RTI_LIB_PREFIX rti_me_)
    SET(PLATFORM_LIBS netapi32.lib advapi32.lib user32.lib winmm.lib WS2_32.lib)
    ADD_DEFINITIONS(-DRTI_WIN32)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)

    IF (RTI_USE_NMAKE OR (${CMAKE_GENERATOR} MATCHES "NMake"))
        string(APPEND CMAKE_EXE_LINKER_FLAGS " /defaultlib:libc.lib /defaultlib:msvcrt.lib /defaultlib:Winmm.lib /nodefaultlib:libcmt.lib /nodefaultlib:libcmtd.lib /nodefaultlib:msvcrtd.lib")
    ELSE()
        IF (RTI_DEBUG_BUILD)
            string(APPEND CMAKE_EXE_LINKER_FLAGS " /NODEFAULTLIB:MSVCRT.lib")
        ENDIF()
        add_compile_options(
            $<$<CONFIG:>:/MD>
            $<$<CONFIG:Debug>:/MDd>
            $<$<CONFIG:Release>:/MD>
        )
    ENDIF()
ENDIF()

SET(RTIDDSGEN ${RTIMEHOME}/rtiddsgen/scripts/rtiddsgen)
SET(RTIDDSMAG ${RTIMEHOME}/rtiddsmag/scripts/rtiddsmag)

SET_PROPERTY(GLOBAL PROPERTY RULE_MESSAGES OFF)

SET(IDL_SOURCE_C perftest.idl)

SET(IDL_PATH ${CMAKE_CURRENT_SOURCE_DIR})
SET(XML_PATH ${CMAKE_CURRENT_SOURCE_DIR})

SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/objs/${RTIME_TARGET_NAME})

IF (CMAKE_GENERATOR MATCHES "Visual Studio")
    #Visual studio needs to run commands using 'call'
    #if there is more than one dependency
    SET(COMMAND_PREFIX call)
ELSE()
    SET(COMMAND_PREFIX)
ENDIF()

IF (RTIME_CERT)
    ADD_DEFINITIONS(-DRTI_CERT)
ENDIF()

SET(RTI_SECURE_INC)
SET(RTI_SECURE_LINK_DIRECTORES)

IF (NOT RTI_SHARED_LIB)
    SET(RTI_LIB_EXT ${CMAKE_STATIC_LIBRARY_SUFFIX})
ELSE()
    SET(RTI_LIB_EXT ${CMAKE_SHARED_LIBRARY_SUFFIX})
ENDIF()

IF ((${CMAKE_SYSTEM_NAME} STREQUAL "Windows") AND RTI_SHARED_LIB)
    ADD_DEFINITIONS(-DRTIME_DLL_VARIABLE)
ENDIF()

IF (${RTIME_LIGHTWEIGHT_SECURE})
   IF(NOT DEFINED OPENSSLHOME)
        IF(DEFINED ENV{OPENSSLHOME})
            SET(OPENSSLHOME $ENV{OPENSSLHOME})
        ELSE()
            IF(RTI_DEBUG_BUILD)
                SET(_cfg_type "debug")
            ELSE()
                SET(_cfg_type "release")
            ENDIF()

            IF(RTI_SHARED_LIB)
                SET(_cfg_link "shared")
            ELSE()
                SET(_cfg_link "static")
            ENDIF()

            SET(OPENSSLHOME
                "${RTIMEHOME}/third_party/openssl-3.5.0/${RTIME_TARGET_NAME}/${_cfg_type}-${_cfg_link}"
            )
        ENDIF()
    ENDIF()

    MESSAGE(STATUS "[RTI] OPENSSLHOME ................: ${OPENSSLHOME}")

    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        SET(SSL_LIBS libcrypto${RTI_LIB_SUFFIX_SSL})
        SET(PLATFORM_LIBS crypt32.lib ${PLATFORM_LIBS})
    ELSE()
        SET(SSL_LIBS crypto)

        IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
            LIST(INSERT SSL_LIBS 2 dl)
        ENDIF()

    ENDIF()

    SET(RTI_SECURE_LINK_DIRECTORES ${OPENSSLHOME}/lib)
    SET(RTI_SECURE_INC ${OPENSSLHOME}/include)
ENDIF()

# Find ZCV2, SHMEM, and xtypes libraries in the library folder and add them
# to our library list. This is for convenience. In the future this will
# be based on the example generated and not on the presence of the libraries.
# CPP requires Shared memory. ZCv2 requries shared memory.
STRING(REGEX MATCH "^[^-]+" ARCH_NAME ${RTIME_TARGET_NAME})
SET(LIB_PATH "${RTIMEHOME}/lib/${ARCH_NAME}")
SET(PSL_LIB_PATH "${RTIMEHOME}/lib/${RTIME_TARGET_NAME}")

# Add the xtypes library if supported in case the IDL was compiled with
# -interpreted 1 (default).
IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}ddsxtypes${RTI_LIB_SUFFIX}${RTI_LIB_EXT}")
    SET(MICRO_C_LIBS
        rti_me_ddsxtypes${RTI_LIB_SUFFIX}
        rti_me${RTI_LIB_SUFFIX}
    )
ELSE()
    SET(MICRO_C_LIBS
        rti_me${RTI_LIB_SUFFIX}
    )
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}ddsfilter${RTI_LIB_SUFFIX}${RTI_LIB_EXT}")
    LIST(APPEND MICRO_C_LIBS "rti_me_ddsfilter${RTI_LIB_SUFFIX}")
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}netioshmem${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
            AND NOT EXCLUDE_SHMEM)
    LIST(APPEND MICRO_C_LIBS "rti_me_netioshmem${RTI_LIB_SUFFIX}")
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}netiosdm${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
            AND NOT EXCLUDE_SHMEM)
    LIST(APPEND MICRO_C_LIBS "rti_me_netiosdm${RTI_LIB_SUFFIX}")
ENDIF()

IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}netiozcopy${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
            AND NOT EXCLUDE_ZCV2)
    LIST(APPEND MICRO_C_LIBS "rti_me_netiozcopy${RTI_LIB_SUFFIX}")
    ADD_DEFINITIONS(-DRTIME_HAVE_ZCOPYV2=1)
ENDIF()

IF (${RTIME_LIGHTWEIGHT_SECURE})
    IF (EXISTS "${LIB_PATH}/${RTI_LIB_PREFIX}ddspsk${RTI_LIB_SUFFIX}${RTI_LIB_EXT}"
        AND EXISTS "${PSL_LIB_PATH}/${RTI_LIB_PREFIX}pskpsl${RTI_LIB_SUFFIX}${RTI_LIB_EXT}")

        LIST(APPEND MICRO_C_LIBS "rti_me_ddspsk${RTI_LIB_SUFFIX}")
        LIST(APPEND MICRO_C_LIBS "rti_me_pskpsl${RTI_LIB_SUFFIX}")
    ELSE()
        MESSAGE(FATAL_ERROR "PSK libraries were not found.")
    ENDIF()
ENDIF()

IF (NOT RTIME_CERT)
    LIST(APPEND MICRO_C_LIBS
        rti_me_cpp${RTI_LIB_SUFFIX}
        rti_me_rhsm${RTI_LIB_SUFFIX}
        rti_me_whsm${RTI_LIB_SUFFIX}
        rti_me_discdpse${RTI_LIB_SUFFIX}
        rti_me_discdpde${RTI_LIB_SUFFIX}
    )
ENDIF()

INCLUDE_DIRECTORIES(
    ${RTIMEHOME}/include
    ${RTIMEHOME}/include/rti_me
    ${RTI_SECURE_INC}
)

LINK_DIRECTORIES(
    ${RTIMEHOME}/lib/${RTIME_TARGET_NAME}
    ${RTI_SECURE_LINK_DIRECTORES}
)

STRING(REGEX MATCH "(.*)-(.*)" pil_match ${RTIME_TARGET_NAME})
IF (pil_match)
    SET(RTIME_PIL_ARCH ${CMAKE_MATCH_1})
    LIST(APPEND MICRO_C_LIBS rti_me_netiopsl_cpp${RTI_LIB_SUFFIX} rti_me_netiopsl${RTI_LIB_SUFFIX})
    LIST(APPEND MICRO_C_LIBS rti_me_ospsl${RTI_LIB_SUFFIX})
    LINK_DIRECTORIES(${RTIMEHOME}/lib/${RTIME_PIL_ARCH})
    GET_PROPERTY(cppdef DIRECTORY PROPERTY COMPILE_DEFINITIONS)
ENDIF()

FOREACH(idl ${IDL_SOURCE_C})
    GET_FILENAME_COMPONENT(filename ${idl} NAME)
    STRING(REGEX REPLACE "\\.idl" "" basename ${filename})
    LIST(APPEND IDL_GEN_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.${SOURCE_EXTENSION})

    LIST(APPEND IDL_GEN_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.h)

    IF (RTIME_IDL_ADD_REGENERATE_TYPESUPPORT_RULE)
        IF (NOT EXISTS ${idl})
            MESSAGE(FATAL_ERROR "${idl} does not exist.")
        ELSEIF(IS_DIRECTORY ${idl})
            MESSAGE(FATAL_ERROR "${idl} is not a file.")
        ENDIF()

        SET(IDL_GEN_FILE_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.${SOURCE_EXTENSION}
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.${SOURCE_EXTENSION})

        SET(IDL_GEN_FILE_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Plugin.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Support.h)

        ADD_CUSTOM_COMMAND(OUTPUT ${IDL_GEN_FILE_C} ${IDL_GEN_FILE_H}
                           COMMAND ${COMMAND_PREFIX} ${RTIDDSGEN} -language C
                                   -micro -verbosity 1 -update typefiles
                                   -d ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${IDL_PATH}/${idl}
                           DEPENDS ${IDL_PATH}/${idl}
                           COMMENT "Regeneration type support plugin from ${idl}"
                          )
        SET_SOURCE_FILES_PROPERTIES(${IDLIDL_GEN_FILE_C_GEN_C} ${IDL_GEN_FILE_H} PROPERTIES GENERATED 1)
    ENDIF()
ENDFOREACH()

SET_LIB_ORDER(MICRO_C_LIBS)

ADD_EXECUTABLE(perftest_publisher
               ${CMAKE_CURRENT_SOURCE_DIR}/perftest_publisher.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/perftestApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/perftestApplication.h
                ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.cxx ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.cxx ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.cxx ${CMAKE_CURRENT_SOURCE_DIR}/RTIDDSImpl.cxx ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.cxx ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.cxx ${CMAKE_CURRENT_SOURCE_DIR}/PerftestSecurity.cxx ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.cxx ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_micro.cxx ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.cxx ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.cxx                ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.h ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.h ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.h ${CMAKE_CURRENT_SOURCE_DIR}/MessagingIF.h ${CMAKE_CURRENT_SOURCE_DIR}/RTIDDSImpl.h ${CMAKE_CURRENT_SOURCE_DIR}/perftest_cpp.h ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.h ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.h ${CMAKE_CURRENT_SOURCE_DIR}/PerftestSecurity.h ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.h ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_micro.h ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.h ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.h               ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

ADD_EXECUTABLE(perftest_subscriber
               ${CMAKE_CURRENT_SOURCE_DIR}/perftest_subscriber.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/perftestApplication.${SOURCE_EXTENSION}
               ${CMAKE_CURRENT_SOURCE_DIR}/perftestApplication.h
                ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.cxx ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.cxx ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.cxx ${CMAKE_CURRENT_SOURCE_DIR}/RTIDDSImpl.cxx ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.cxx ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.cxx ${CMAKE_CURRENT_SOURCE_DIR}/PerftestSecurity.cxx ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.cxx ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_micro.cxx ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.cxx ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.cxx                ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/ThreadPriorities.h ${CMAKE_CURRENT_SOURCE_DIR}/Parameter.h ${CMAKE_CURRENT_SOURCE_DIR}/ParameterManager.h ${CMAKE_CURRENT_SOURCE_DIR}/MessagingIF.h ${CMAKE_CURRENT_SOURCE_DIR}/RTIDDSImpl.h ${CMAKE_CURRENT_SOURCE_DIR}/perftest_cpp.h ${CMAKE_CURRENT_SOURCE_DIR}/CpuMonitor.h ${CMAKE_CURRENT_SOURCE_DIR}/PerftestTransport.h ${CMAKE_CURRENT_SOURCE_DIR}/PerftestSecurity.h ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_common.h ${CMAKE_CURRENT_SOURCE_DIR}/Infrastructure_micro.h ${CMAKE_CURRENT_SOURCE_DIR}/FileDataLoader.h ${CMAKE_CURRENT_SOURCE_DIR}/PerftestPrinter.h               ${IDL_GEN_C} ${IDL_GEN_H} ${APP_GEN_C} ${APP_GEN_H})

TARGET_LINK_LIBRARIES(perftest_publisher
        ${MICRO_C_LIBS}
        ${XML_LIBS}
        ${SSL_LIBS}
        ${PLATFORM_LIBS}
)

TARGET_LINK_LIBRARIES(perftest_subscriber
        ${MICRO_C_LIBS}
        ${XML_LIBS}
        ${SSL_LIBS}
        ${PLATFORM_LIBS}
)
