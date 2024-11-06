
CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

INCLUDE(../resource/cmake/common_utils.cmake)

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

IF (NOT (DEFINED RTI_SHARED_LIB))
    SET(RTI_SHARED_LIB FALSE)
ENDIF()
MESSAGE(STATUS "[RTI] RTI_SHARED_LIB .............: ${RTI_SHARED_LIB}")

message(STATUS "ADDITIONAL_DEFINES = ${ADDITIONAL_DEFINES}")
ADD_DEFINITIONS(${ADDITIONAL_DEFINES})
LINK_DIRECTORIES()
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
PROJECT(perftest CXX C)
SET(SOURCE_EXTENSION cxx)

# Evaluating whether 24x compatibility was specified
SET(24x_COMPATIBILITY_SPECIFIED -1)
STRING(FIND "${ADDITIONAL_DEFINES}" "PERFTEST_RTI_MICRO_24x_COMPATIBILITY" 24x_COMPATIBILITY_SPECIFIED)

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    SET(PLATFORM_LIBS dl nsl m pthread rt)
    ADD_DEFINITIONS(-DRTI_LINUX)
ELSEIF (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    SET(PLATFORM_LIBS dl m pthread )
    ADD_DEFINITIONS(-DRTI_DARWIN)
ELSEIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    SET(PLATFORM_LIBS netapi32.lib advapi32.lib user32.lib winmm.lib WS2_32.lib)
    ADD_DEFINITIONS(-DRTI_WIN32)
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
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /NODEFAULTLIB:MSVCRT.lib" CACHE INTERNAL "CMAKE_EXE_LINKER_FLAGS" FORCE)
    ADD_DEFINITIONS(-D_CRT_SECURE_NO_WARNINGS)
ELSE()
    SET(COMMAND_PREFIX)
ENDIF()

IF (RTIME_CERT)
    ADD_DEFINITIONS(-DRTI_CERT)
ENDIF()

SET(RTI_LIB_PREFIX rti_me)
SET(RTI_LIB_SUFFIX)
SET(RTI_LIB_SUFFIX_SSL)

SET(RTI_SECURE_INC)
SET(RTI_SECURE_LINK_DIRECTORES)

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

    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        ADD_DEFINITIONS(-DRTIME_DLL_VARIABLE)
    ENDIF()
ENDIF()

get_core_lib_dir_name(${RTIME_TARGET_NAME})

IF (${RTIME_SECURE})
    IF (NOT DEFINE OPENSSLHOME)
        IF (DEFINED ENV{OPENSSLHOME})
            SET(OPENSSLHOME $ENV{OPENSSLHOME})
        ELSE()
            MESSAGE(FATAL_ERROR "OPENSSLHOME must be defined or set as an environment variable.")
        ENDIF()
    ENDIF()
    MESSAGE(STATUS "[RTI] OPENSSLHOME ................: ${OPENSSLHOME}")

    SET(RTI_SECURE_LIB rti_me_seccore${RTI_LIB_SUFFIX})
    SET(RTI_SECURE_INC ${OPENSSLHOME}/include ${XML_INC})
    SET(RTI_SECURE_LINK_DIRECTORES ${OPENSSLHOME}/lib)

    IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
        SET(SSL_LIBS libssl${RTI_LIB_SUFFIX_SSL} libcrypto${RTI_LIB_SUFFIX_SSL})
        SET(PLATFORM_LIBS crypt32.lib ${PLATFORM_LIBS})
    ELSE()
        SET(SSL_LIBS ssl${RTI_LIB_SUFFIX_SSL} crypto${RTI_LIB_SUFFIX_SSL})
    ENDIF()

    IF (SYSLIBXML2)
        IF (LIBXML2HOME)
            SET(XML_INC ${LIBXML2HOME}/lib)
        ENDIF()
    SET(XML_LIBS xml2)
    ENDIF()
ENDIF()

IF (NOT RTIME_CERT)
    IF (NOT 24x_COMPATIBILITY_SPECIFIED EQUAL -1)
        SET(MICRO_C_LIBS rti_me_cpp${RTI_LIB_SUFFIX}
            rti_me_rhsm${RTI_LIB_SUFFIX}
            rti_me_whsm${RTI_LIB_SUFFIX}
            rti_me_discdpse${RTI_LIB_SUFFIX}
            rti_me_discdpde${RTI_LIB_SUFFIX}
            rti_me${RTI_LIB_SUFFIX})
    ELSE()
        IF (DEFINED RTIME_MAG_FILES)
            SET(MICRO_APPGEN_LIB rti_me_appgen${RTI_LIB_SUFFIX})
        ENDIF()

        SET(MICRO_C_LIBS ${MICRO_APPGEN_LIB}
                    rti_me_cpp${RTI_LIB_SUFFIX}
                    rti_me_netioshmem${RTI_LIB_SUFFIX}
                    rti_me_netiosdm${RTI_LIB_SUFFIX}
        )

        LIST(APPEND MICRO_C_LIBS
            rti_me_netioshmem${RTI_LIB_SUFFIX}
            rti_me_netiosdm${RTI_LIB_SUFFIX}
            rti_me_rhsm${RTI_LIB_SUFFIX}
            rti_me_whsm${RTI_LIB_SUFFIX}
            rti_me_discdpse${RTI_LIB_SUFFIX}
            rti_me_discdpde${RTI_LIB_SUFFIX}
                )

        IF(arch_is_pi)
            LIST(APPEND MICRO_C_LIBS
                rti_me_netiopsl_cpp${RTI_LIB_SUFFIX}
                rti_me_netiopsl${RTI_LIB_SUFFIX}
                rti_me_ospsl${RTI_LIB_SUFFIX})
        ENDIF()

        LIST(APPEND MICRO_C_LIBS
            rti_me${RTI_LIB_SUFFIX}
            )
    ENDIF()
ELSE()
    SET(MICRO_C_LIBS rti_me${RTI_LIB_SUFFIX}
    )
ENDIF()

INCLUDE_DIRECTORIES(${RTIMEHOME}/include ${RTIMEHOME}/include/rti_me ${RTI_SECURE_INC})
IF(${arch_is_pi})
    INCLUDE_DIRECTORIES(${RTIMEHOME}/include/rti_me/rti_me_psl)
ENDIF()

LINK_DIRECTORIES(${RTIMEHOME}/lib/${RTIME_TARGET_NAME} ${RTI_SECURE_LINK_DIRECTORES})
IF (${arch_is_pi})
    LINK_DIRECTORIES(${RTIMEHOME}/lib/${core_lib_dir_name})
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
                           COMMAND ${COMMAND_PREFIX} ${RTIDDSGEN} -language C++98
                                   -micro -verbosity 1 -update typefiles
                                   -d ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${IDL_PATH}/${idl}
                           DEPENDS ${IDL_PATH}/${idl}
                           COMMENT "Regeneration type support plugin from ${idl}"
                          )
        SET_SOURCE_FILES_PROPERTIES(${IDLIDL_GEN_FILE_C_GEN_C} ${IDL_GEN_FILE_H} PROPERTIES GENERATED 1)
    ENDIF()
ENDFOREACH()

IF (DEFINED RTIME_MAG_FILES)
    FOREACH(xml ${RTIME_MAG_FILES})
        IF (NOT EXISTS ${xml})
            MESSAGE(FATAL_ERROR "${xml} does not exist.")
        ELSEIF(IS_DIRECTORY ${xml})
            MESSAGE(FATAL_ERROR "${xml} is not a file.")
        ENDIF()

        GET_FILENAME_COMPONENT(filename ${xml} NAME)
        STRING(REGEX REPLACE "\\.xml" "" basename ${filename})
        GET_FILENAME_COMPONENT(basename ${xml} NAME_WE)

        STRING(REGEX REPLACE "\\.xml" "Qos.xml" xml_qos ${xml})
        IF (EXISTS ${xml_qos})
            IF(NOT IS_DIRECTORY ${xml_qos})
                SET(REFERENCED_OPT "YES")
            ENDIF()
        ENDIF()

        LIST(APPEND APP_GEN_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Appgen.c)

        LIST(APPEND APP_GEN_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}Appgen.h)

        LIST(APPEND APP_GEN_C ${CMAKE_CURRENT_SOURCE_DIR}/${basename}AppgenPlugin.cxx)

        LIST(APPEND APP_GEN_H ${CMAKE_CURRENT_SOURCE_DIR}/${basename}AppgenPlugin.h)


        IF (DEFINED REFERENCED_OPT)
            ADD_CUSTOM_COMMAND(OUTPUT ${APP_GEN_C} ${APP_GEN_H}
                               COMMAND ${COMMAND_PREFIX} ${RTIDDSMAG} -verbosity 1
                                       -language C++98
                                       -replace
                                       -d ${CMAKE_CURRENT_SOURCE_DIR}
                                       -idlFile ${IDL_PATH}/${IDL_SOURCE_C}
                                       -referencedFile ${XML_PATH}/${basename}Qos.xml
                                       ${XML_PATH}/${xml}
                               DEPENDS ${XML_PATH}/${xml} ${XML_PATH}/${basename}Qos.xml
                               COMMENT "Regeneration of application support from ${xml}"
            )
        ELSE()
            ADD_CUSTOM_COMMAND(OUTPUT ${APP_GEN_C} ${APP_GEN_H}
                               COMMAND ${COMMAND_PREFIX} ${RTIDDSMAG} -verbosity 1
                                       -language C++98
                                       -replace
                                       -d ${CMAKE_CURRENT_SOURCE_DIR}
                                       -idlFile ${IDL_PATH}/${IDL_SOURCE_C}
                                       ${XML_PATH}/${xml}
                               DEPENDS ${XML_PATH}/${xml}
                               COMMENT "Regeneration of application support from ${xml}"
            )
        ENDIF()
    ENDFOREACH()
ENDIF()

IF (${RTIME_SECURE})
    ADD_CUSTOM_TARGET(security_material ALL
                      DEPENDS ${CMAKE_SOURCE_DIR}/security/ca/ca.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher_key.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber.pem
                              ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber_key.pem
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.p7s
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.p7s
                              ${CMAKE_SOURCE_DIR}/security/xml/governance.p7s
                     )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf
                              ${CMAKE_SOURCE_DIR}/security/openssl_publisher.cnf
                              ${CMAKE_SOURCE_DIR}/security/openssl_subscriber.cnf
                              ${CMAKE_SOURCE_DIR}/security/xml/governance.xml
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.xml
                              ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.xml
                       COMMAND "${CMAKE_COMMAND}" -E copy_directory "${RTIMEHOME}/rtiddsgen/resource/rtiddsgen/security" "security"
                       COMMENT "Copy secure configuration files"
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem ${CMAKE_SOURCE_DIR}/security/ca/ca.pem
                       COMMAND "${CMAKE_COMMAND}" -E make_directory ca
                       COMMAND "${CMAKE_COMMAND}" -E touch ca/index.txt
                       COMMAND echo 01 > ca/serial
                       COMMAND openssl genrsa -out ca/ca_key.pem 2048
                       COMMAND openssl req -new -key ca/ca_key.pem -out ca/ca.csr -config openssl_ca.cnf
                       COMMAND openssl x509 -req -days 365 -in ca/ca.csr -signkey ca/ca_key.pem -out ca/ca.pem
                       COMMENT "Regenerating Certificate Authority"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher.pem ${CMAKE_SOURCE_DIR}/security/ca/certs/publisher_key.pem
                       COMMAND "${CMAKE_COMMAND}" -E make_directory ca/certs
                       COMMAND openssl genrsa -out ca/certs/publisher_key.pem 2048
                       COMMAND openssl req -new -key ca/certs/publisher_key.pem -out ca/certs/publisher.csr -config openssl_publisher.cnf
                       COMMAND openssl ca -batch -config openssl_ca.cnf -in ca/certs/publisher.csr -out ca/certs/publisher.pem
                       COMMENT "Regenerating Publisher identity"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf ${CMAKE_SOURCE_DIR}/security/openssl_publisher.cnf
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber.pem ${CMAKE_SOURCE_DIR}/security/ca/certs/subscriber_key.pem
                       COMMAND "${CMAKE_COMMAND}" -E make_directory ca/certs
                       COMMAND openssl genrsa -out ca/certs/subscriber_key.pem 2048
                       COMMAND openssl req -new -key ca/certs/subscriber_key.pem -out ca/certs/subscriber.csr -config openssl_subscriber.cnf
                       COMMAND openssl ca -batch -config openssl_ca.cnf -in ca/certs/subscriber.csr -out ca/certs/subscriber.pem
                       COMMENT "Regenerating Subscriber identity"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/openssl_ca.cnf ${CMAKE_SOURCE_DIR}/security/openssl_subscriber.cnf
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.p7s
                       COMMAND openssl cms -in xml/permissions_publisher.xml -out xml/permissions_publisher.p7s -signer ca/ca.pem -inkey ca/ca_key.pem -sign -outform smime
                       COMMENT "Regenerating Publisher Permissions file"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/xml/permissions_publisher.xml ${CMAKE_SOURCE_DIR}/security/ca/ca.pem ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.p7s
                       COMMAND openssl cms -in xml/permissions_subscriber.xml -out xml/permissions_subscriber.p7s -signer ca/ca.pem -inkey ca/ca_key.pem -sign -outform smime
                       COMMENT "Regenerating Subscriber Permissions file"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/xml/permissions_subscriber.xml ${CMAKE_SOURCE_DIR}/security/ca/ca.pem ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )

    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_SOURCE_DIR}/security/xml/governance.p7s
                       COMMAND openssl cms -in xml/governance.xml -out xml/governance.p7s -signer ca/ca.pem -inkey ca/ca_key.pem -sign -outform smime
                       COMMENT "Regenerating Governance file"
                       DEPENDS ${CMAKE_SOURCE_DIR}/security/xml/governance.xml ${CMAKE_SOURCE_DIR}/security/ca/ca.pem ${CMAKE_SOURCE_DIR}/security/ca/ca_key.pem
                       WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/security
                      )
ENDIF()

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

TARGET_LINK_LIBRARIES(perftest_publisher  ${MICRO_C_LIBS} ${XML_LIBS} ${SSL_LIBS} ${PLATFORM_LIBS})
TARGET_LINK_LIBRARIES(perftest_subscriber ${MICRO_C_LIBS} ${XML_LIBS} ${SSL_LIBS} ${PLATFORM_LIBS})
