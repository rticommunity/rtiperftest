#!/bin/bash

################################################################################
# Variable declaration

filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`
idl_location="${script_location}/srcIdl"
classic_cpp_folder="${script_location}/srcCpp"
common_cpp_folder="${script_location}/srcCppCommon"
modern_cpp_folder="${script_location}/srcCpp03"
java_folder="${script_location}/srcJava"
java_scripts_folder="${script_location}/resource/scripts/java_execution_scripts"
bin_folder="${script_location}/bin"
cStringifyFile_script="${script_location}/resource/scripts/cStringifyFile.pl"
qos_file="${script_location}/perftest_qos_profiles.xml"
doc_folder="${script_location}/srcDoc"
generate_doc_folder="${script_location}/doc"

# By default we will build pro, not micro
BUILD_MICRO=0

# In case we build micro, which version.
BUILD_MICRO_24x_COMPATIBILITY=0
MICRO_UNBOUNDED_SEQUENCE_SIZE=1048576


# Default values:
BUILD_CPP=1
BUILD_CPP03=1
BUILD_JAVA=1
MAKE_EXE=make
CMAKE_EXE=cmake
COMPILER_EXE="" # let rtiddsgen choose the default
LINKER_EXE="" # let rtiddsgen choose the default
PERL_EXEC=perl
JAVAC_EXE=javac
JAVA_EXE=java
JAR_EXE=jar

# Default values for libraries
RELEASE_DEBUG=release
STATIC_DYNAMIC=static
USE_SECURE_LIBS=0
LEGACY_DD_IMPL=0

# Needed when compiling statically using security
RTI_OPENSSLHOME=""

# Variables for customType
custom_type_folder="${idl_location}/customType"
USE_CUSTOM_TYPE=0
USE_CUSTOM_TYPE_FLAT=0
custom_type="" # Type of the customer
custom_type_flat="" # Type of the customer
custom_type_file_name_support="" # Name of the file with the type. "TSupport.h"
# Intermediate file for including the custom type file #include "file.idl"
custom_idl_file="${custom_type_folder}/custom.idl"

# Variables for FlatData
flatdata_ddsgen_version=3 # We just need the Major value of the version.
FLATDATA_AVAILABLE=0
ZEROCOPY_AVAILABLE=0
darwin_shmem_size=419430400

# For C++ Classic, variable to control if using or not
# the native implementation
RTI_PERFTEST_NANO_CLOCK=0

# We will use some colors to improve visibility of errors and information
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'
INFO_TAG="${GREEN}[INFO]:${NC}"
ERROR_TAG="${RED}[ERROR]:${NC}"

################################################################################

function usage()
{
    echo ""
    echo "This scripts accepts the following parameters:                                  "
    echo "                                                                                "
    echo "    --micro                      Build RTI Perftest for RTI Connext Micro       "
    echo "                                 By default RTI Perftest will assume it will be "
    echo "                                 built against RTI Connext DDS Professional     "
    echo "    --micro-24x-compatibility    Similar to --micro but ensuring compatibility  "
    echo "                                 with RTI Connext Micro 2.4.11 and above.       "
    echo "    --platform <your_arch>       Platform for which build.sh is going to compile"
    echo "                                 RTI Perftest.                                  "
    echo "    --nddshome <path>            Path to the *RTI Connext DDS Professional      "
    echo "                                 installation*. If this parameter is not present"
    echo "                                 the \$NDDSHOME variable should be set.         "
    echo "                                 If provided when building micro, it will be    "
    echo "                                 used as \$RTIMEHOME                            "
    echo "    --rtimehome <path>           Path to the *RTI Connext DDS Micro*            "
    echo "                                 installation*. If this parameter is not present"
    echo "                                 the \$RTIMEHOME variable should be set.        "
    echo "    --skip-java-build            Avoid Java ByteCode generation creation.       "
    echo "                                 (No effect when building for Micro)            "
    echo "    --skip-cpp-build             Avoid C++ code generation and compilation.     "
    echo "    --skip-cpp03-build           Avoid C++ New PSM code generation and          "
    echo "                                 compilation.                                   "
    echo "                                 (No effect when building for Micro)            "
    echo "    --java-build                 Only Java ByteCode generation creation.        "
    echo "    --cpp-build                  Only C++ code generation and compilation.      "
    echo "    --cpp03-build                Only C++ New PSM code generation and           "
    echo "                                 compilation.                                   "
    echo "    --make <path>                Path to the GNU make executable. If this       "
    echo "                                 parameter is not present, the GNU make variable"
    echo "                                 should be available from your \$PATH variable. "
    echo "    --cmake <path>               Path to the cmake executable. If this          "
    echo "                                 parameter is not present, cmake executable     "
    echo "                                 should be available from your \$PATH variable. "
    echo "                                 will clean all the generated code and binaries "
    echo "    --compiler <path>            Path to (or name of) the compiler executable.  "
    echo "                                 If this parameter is not a full path, the named"
    echo "                                 executable should be available from your       "
    echo "                                 \$PATH variable. (NOTE: c++/c++03 builds only) "
    echo "    --linker <path>              Path to (or name of) the linker executable.    "
    echo "                                 If this parameter is not a full path, the named"
    echo "                                 executable should be available from your       "
    echo "                                 \$PATH variable. (NOTE: c++/c++03 builds only) "
    echo "    --perl <path>                Path to PERL executable. If this parameter is  "
    echo "                                 not present, the path to PERL should be        "
    echo "                                 available from your \$PATH variable.           "
    echo "    --java-home <path>           Path to the Java JDK home folder. If this      "
    echo "                                 parameter is not present, javac, jar and java  "
    echo "                                 executables should be available from your      "
    echo "                                 \$PATH variable.                               "
    echo "                                 (No effect when building for Micro)            "
    echo "    --clean                      If this option is present, the build.sh script "
    echo "                                 will clean all the generated code and binaries "
    echo "    --debug                      Compile against the RTI Connext Debug          "
    echo "                                 libraries. Default is against release ones.    "
    echo "    --build-doc                  Generate the HTML and PDF documentation.       "
    echo "    --dynamic                    Compile against the RTI Connext Dynamic        "
    echo "                                 libraries. Default is against static ones.     "
    echo "                                 (No effect when building for Micro)            "
    echo "    --secure                     Enable the security options for compilation.   "
    echo "                                 Default is not enabled.                        "
    echo "    --openssl-home <path>        Path to the openssl home. This will be used    "
    echo "                                 when compiling statically and using security   "
    echo "                                 Default is an empty string (current folder).   "
    echo "    --customType <type>          Use the Custom type feature with your type.    "
    echo "                                 See details and examples of use in the         "
    echo "                                 documentation.                                 "
    echo "    --customTypeFlatData <type>  Use the Custom type feature with your FlatData "
    echo "                                 Type. See details and examples of use in the   "
    echo "                                 documentation.                                 "
    echo "    --flatData-max-size <size>   Specify the maximum bounded size on bytes      "
    echo "                                 for sequences when using FlatData language     "
    echo "                                 binding. Default 10MB                          "
    echo "    --osx-shmem-shmmax <size>    Maximum segment size for shared memory in OSX  "
    echo "                                 in bytes. Default 400MB                        "
    echo "    --ns-resolution              Try to use the system real-time clock to get   "
    echo "                                 nano-second resolution.                        "
    echo "                                 For the Classic C++ Implementation only.       "
    echo "                                 Default is not enabled.                        "
    echo "    --help -h                    Display this message.                          "
    echo "                                                                                "
    echo "================================================================================"
    echo ""
}

function clean_custom_type_files()
{
    # Remove generated files of the customer type
    for file in ${custom_type_folder}/*.idl
    do
        if [ -f $file ]; then
            name_file=$(basename $file)
            rm -rf ${idl_location}/${name_file}
            name_file="${name_file%.*}"
            rm -f "${script_location}"/srcC*/"${name_file}Plugin."*
            rm -f "${script_location}"/srcC*/"${name_file}."*
            rm -f "${script_location}"/srcC*/"${name_file}Support."*
        fi
    done
    rm -rf ${custom_idl_file}
}

function clean()
{
    echo ""
    echo -e "${INFO_TAG} Cleaning generated files."

    rm -f  "${script_location}"/srcC*/perftest.*
    rm -f  "${script_location}"/srcC*/perftest_ZeroCopy*
    rm -f  "${script_location}"/srcC*/perftestPlugin.*
    rm -f  "${script_location}"/srcC*/perftestSupport.*
    rm -f  "${script_location}"/srcC*/perftest_subscriber.*
    rm -f  "${script_location}"/srcC*/makefile_*
    rm -rf "${script_location}"/srcC*/objs
    rm -f  "${script_location}"/srcC*/*.vcxproj*
    rm -f  "${script_location}"/srcC*/*.vcproj*
    rm -f  "${script_location}"/srcC*/*.csproj
    rm -f  "${script_location}"/srcC*/*.sln
    rm -f  "${script_location}"/srcC*/*.sdf
    rm -f  "${script_location}"/srcC*/perftestImplPlugin.*
    rm -f  "${script_location}"/srcC*/perftestImpl.*
    rm -rf "${script_location}"/srcJava/class
    rm -rf "${script_location}"/srcJava/jar
    rm -rf "${script_location}"/srcJava/com/rti/perftest/gen
    rm -rf "${script_location}"/bin
    rm -f  "${script_location}"/srcCpp/*.txt
    rm -rf "${script_location}"/srcCpp/gen
    rm -rf "${script_location}"/srcCpp/perftest_build
    rm -rf "${script_location}"/srcCpp/perftestApplication.*
    clean_custom_type_files
    clean_documentation
    clean_src_cpp_common

    echo ""
    echo "================================================================================"
    echo ""

}

function executable_checking()
{

    # Is platform specified?
    if [ -z "${platform}" ]; then
        echo -e "${ERROR_TAG} The platform argument is missing"
        usage
        exit -1
    fi

    # If we are Building MICRO
    if [ "${BUILD_MICRO}" -eq "1" ]; then

        # Is RTIMEHOME set?
        if [ -z "${RTIMEHOME}" ]; then
            # Is NDDSHOME set?
            if [ -z "${NDDSHOME}" ]; then
                echo -e "${ERROR_TAG} Nor RTIMEHOME nor NDDSHOME variables are set or the paths do not exist"
                usage
                exit -1
            else
                echo -e "${INFO_TAG} The RTIMEHOME variable is not set or the path does not exist, using NDDSHOME instead"
            fi
        else
            export NDDSHOME="${RTIMEHOME}"
        fi

        # Is CMAKE in the path?
        if [ -z `which "${CMAKE_EXE}"` ]
        then
            echo -e "${YELLOW}[WARNING]:${NC} ${CMAKE_EXE} executable not found, perftest_cpp will not be built."
        fi

        # Is MAKE in the path?
        if [ "${BUILD_CPP}" -eq "1" ]; then
            if [ -z `which "${MAKE_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${MAKE_EXE} executable not found, perftest_cpp will not be built."
            fi
        fi
    else # If building pro

        # Is NDDSHOME set?
        if [ -z "${NDDSHOME}" ]; then
            echo -e "${ERROR_TAG} The NDDSHOME variable is not set or the path does not exist"
            usage
            exit -1
        fi

        # Is MAKE in the path?
        if [ "${BUILD_CPP}" -eq "1" ]; then
            if [ -z `which "${MAKE_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${MAKE_EXE} executable not found, perftest_cpp will not be built."
            fi
        fi
        if [ "${BUILD_CPP03}" -eq "1" ]; then
            if [ -z `which "${MAKE_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${MAKE_EXE} executable not found, perftest_cpp03 will not be built."
            fi
        fi

        # Is COMPILER in the path?
        if [[ "${BUILD_CPP}" -eq "1" ]] && [[ ! -z ${COMPILER_EXE} ]]; then
            if [ -z `which "${COMPILER_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${COMPILER_EXE} executable not found, perftest_cpp will not be built."
                BUILD_CPP=0
            fi
        fi
        if [[ "${BUILD_CPP03}" -eq "1" ]] && [[ ! -z ${COMPILER_EXE} ]]; then
            if [ -z `which "${COMPILER_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${COMPILER_EXE} executable not found, perftest_cpp03 will not be built."
                BUILD_CPP03=0
            fi
        fi

        # Is LINKER in the path?
        if [[ "${BUILD_CPP}" -eq "1" ]] && [[ ! -z ${LINKER_EXE} ]]; then
            if [ -z `which "${LINKER_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${LINKER_EXE} executable not found, perftest_cpp will not be built."
                BUILD_CPP=0
            fi
        fi
        if [[ "${BUILD_CPP03}" -eq "1" ]] && [[ ! -z ${LINKER_EXE} ]]; then
            if [ -z `which "${LINKER_EXE}"` ]
            then
                echo -e "${YELLOW}[WARNING]:${NC} ${LINKER_EXE} executable not found, perftest_cpp03 will not be built."
                BUILD_CPP03=0
            fi
        fi

        # Is JAVA in the path?
        if [ "${BUILD_JAVA}" -eq "1" ]; then
            if [ -z `which "${JAVAC_EXE}"` ]; then
                echo -e "${YELLOW}[WARNING]:${NC} javac executable not found, perftest_java will not be built."
                BUILD_JAVA=0
            fi
            if [ -z `which "${JAVA_EXE}"` ]; then
                echo -e "${YELLOW}[WARNING]:${NC} java executable not found, perftest_java will not be built."
                BUILD_JAVA=0
            fi
            if [ -z `which "${JAR_EXE}"` ]; then
                echo -e "${YELLOW}[WARNING]:${NC} jar executable not found, perftest_java will not be built."
                BUILD_JAVA=0
            fi
        fi

        # If the platform is android, we will just build C++ Classic by default.
        if [[ ${platform} == *"Android"* ]]; then
            BUILD_CPP03=0
            BUILD_JAVA=0
        fi

    fi #Micro/Pro

}

function library_sufix_calculation()
{
    echo ""

    library_sufix=""
    if [ "${STATIC_DYNAMIC}" == "static" ]; then
        echo -e "${INFO_TAG} C++ libraries will be compiled against RTI static libraries."
        library_sufix="z"
    else
        echo -e "${INFO_TAG} C++ libraries will be compiled against RTI dynamic libraries."
    fi
    if [ "${RELEASE_DEBUG}" == "debug" ]; then
        echo -e "${INFO_TAG} C++ libraries will be compiled against RTI debug libraries."
        library_sufix=${library_sufix}"d"
    else
        echo -e "${INFO_TAG} C++ libraries will be compiled against RTI release libraries."
    fi

    if [ "${library_sufix}" == "" ]; then
        rtiddsgen_extra_options="-sharedLib"
    else
        rtiddsgen_extra_options="-libSuffix ${library_sufix}"
    fi
}

function additional_defines_calculation()
{
    additional_defines=""
    additional_rti_libs=""

    # Avoid optimized out variables when debugging
    if [ "${RELEASE_DEBUG}" == "release" ]; then
        echo -e "${INFO_TAG} C++ code will be optimized."
        additional_defines=${additional_defines}"O3"
    else
        additional_defines=${additional_defines}"O0"
    fi

    if [ "${LEGACY_DD_IMPL}" == "1" ]; then
        echo -e "${INFO_TAG} Allow the use of both legacy and new Dynamic Data Impl."
        additional_defines=${additional_defines}" DRTI_LEGACY_DD_IMPL"
    fi

    if [ "${USE_SECURE_LIBS}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_SECURE_PERFTEST"
        if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            additional_defines=${additional_defines}" DRTI_PERFTEST_DYNAMIC_LINKING"
            echo -e "${INFO_TAG} Using security plugin. Linking Dynamically."
        else
            if [ "${RTI_OPENSSLHOME}" == "" ]; then
                echo -e "${ERROR_TAG} In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option."
                exit -1
            fi
            additional_rti_libs="nddssecurity ${additional_rti_libs}"
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraries \"sslz cryptoz\""
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraryPaths \"${RTI_OPENSSLHOME}/${RELEASE_DEBUG}/lib\""
            echo -e "${INFO_TAG} Using security plugin. Linking Statically."
        fi
    fi

    if [ "${USE_CUSTOM_TYPE}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_CUSTOM_TYPE="${custom_type}" DRTI_CUSTOM_TYPE_FILE_NAME_SUPPORT="${custom_type_file_name_support}
    fi

    if [ "${USE_CUSTOM_TYPE_FLAT}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_CUSTOM_TYPE_FLATDATA="${custom_type_flat}" DRTI_CUSTOM_TYPE_FILE_NAME_SUPPORT="${custom_type_file_name_support}
    fi

    if [ "${1}" = "CPPtraditional" ]; then
        additional_defines=${additional_defines}" DRTI_LANGUAGE_CPP_TRADITIONAL"

        if [ "${RTI_PERFTEST_NANO_CLOCK}" == "1" ]; then
            additional_defines=${additional_defines}" DRTI_PERFTEST_NANO_CLOCK"
        fi
    fi

    if [ "${1}}" = "CPPmodern" ]; then
        additional_defines=${additional_defines}" DRTI_LANGUAGE_CPP_MODERN"
    fi

    if [ -x "$(command -v git)" ]; then
        commit_id="$(git rev-parse --short HEAD)"
        additional_defines=${additional_defines}" DPERFTEST_COMMIT_ID='\\\"${commit_id}\\\"'"
    fi

    if [[ $platform == *"Darwin"* ]]; then
        additional_defines=${additional_defines}" DMAX_DARWIN_SHMEM_SIZE=${darwin_shmem_size}"
    fi

    # Adding RTI_ZEROCOPY_AVAILABLE, RTI_FLATDATA_AVAILABLE and RTI_FLATDATA_MAX_SIZE as defines
    if [ "${FLATDATA_AVAILABLE}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_FLATDATA_AVAILABLE"
        additional_defines_flatdata=" -D RTI_FLATDATA_AVAILABLE"
        if [ "${RTI_FLATDATA_MAX_SIZE}" != "" ]; then
            additional_defines=${additional_defines}" DRTI_FLATDATA_MAX_SIZE=${RTI_FLATDATA_MAX_SIZE}"
            additional_defines_flatdata=${additional_defines}" -D RTI_FLATDATA_MAX_SIZE=${RTI_FLATDATA_MAX_SIZE}"
        fi


        if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
            additional_rti_libs="nddsmetp ${additional_rti_libs}"
            additional_defines=${additional_defines}" DRTI_ZEROCOPY_AVAILABLE"
            additional_defines_flatdata=$additional_defines_flatdata" -D RTI_ZEROCOPY_AVAILABLE"
        fi
    fi
}

function additional_defines_calculation_micro()
{
    if [[ $platform == *"Darwin"* ]]; then
        additional_defines=" RTI_DARWIN"
    else
        if [[ $platform == *"Linux"* ]]; then
            additional_defines=" RTI_LINUX"
            additional_included_libraries="nsl;rt;"
        fi
    fi
    additional_defines="RTI_LANGUAGE_CPP_TRADITIONAL RTI_MICRO O3"${additional_defines}

    if [ "${RTI_PERFTEST_NANO_CLOCK}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_PERFTEST_NANO_CLOCK"
    fi

    if [ "${USE_SECURE_LIBS}" == "1" ]; then
        additional_defines="${additional_defines} RTI_SECURE_PERFTEST"

        if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            echo -e "${INFO_TAG} Using security plugin. Linking Dynamically."
        else
            if [ "${RTI_OPENSSLHOME}" == "" ]; then
                echo -e "${ERROR_TAG} In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option."
                exit -1
            fi
            echo -e "${INFO_TAG} Using security plugin. Linking Statically."
        fi
    fi
}

# Generate code for the type of the customer.
# Fill additional source and header files for custom type.
function build_cpp_custom_type()
{
    # Search the file which contains "Struct ${custom_type} {" and include it to ${custom_idl_file}
    found_idl=false
    for file in ${custom_type_folder}/*.idl
    do
        if [ -f $file ]; then
            if grep -Fq  "struct "${custom_type}" {" ${file}
            then # found
                custom_type_file_name_support=$(basename $file)
                custom_type_file_name_support=${custom_type_file_name_support%.*}
                custom_type_file_name_support=${custom_type_file_name_support}"Support.h"
                echo "#include \"$(basename $file)\"" > ${custom_idl_file}
                found_idl=true
            fi
        fi
    done
    if [ "$found_idl" = false ]; then
        echo -e "${ERROR_TAG} Cannot find an idl file with the ${custom_type} structure for custom type test."
        exit -1
    fi
    cp -rf ${custom_type_folder}/* ${idl_location}/
    additional_header_files_custom_type="CustomType.h"
    additional_source_files_custom_type="CustomType.cxx"
    # Find all the files in the folder ${custom_type_folder}
    # Run codegen with all those files
    for file in ${custom_type_folder}/*.idl
    do
        if [ -f $file ]; then
            rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${classic_cpp_lang_string} -unboundedSupport -I ${idl_location} -unboundedSupport -replace -create typefiles -d \"${classic_cpp_folder}\" \"${file}\" "
            echo -e "${INFO_TAG} Command: $rtiddsgen_command"
            eval $rtiddsgen_command
            if [ "$?" != 0 ]; then
                echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string} with the file ${file}."
                exit -1
            fi
            # Adding the generated file as additional HeaderFiles and SourceFiles
            name_file=$(basename $file)
            name_file="${name_file%.*}"
            additional_header_files_custom_type="${name_file}Plugin.h ${name_file}.h ${name_file}Support.h "$additional_header_files_custom_type
            additional_source_files_custom_type="${name_file}Plugin.cxx ${name_file}.cxx ${name_file}Support.cxx "$additional_source_files_custom_type
        fi
    done

    # Adding RTI_USE_CUSTOM_TYPE as a macro
    additional_defines_custom_type=" -D RTI_CUSTOM_TYPE=${custom_type}"

    if [ "${USE_CUSTOM_TYPE_FLAT}" == "1" ]; then
        additional_defines_custom_type="${additional_defines_custom_type} -D RTI_CUSTOM_TYPE_FLATDATA=${custom_type_flat}"
    fi
}

function generate_qos_string()
{
    # If PERL_EXEC is in the path, generate the qos_string.h file.
    if [ "${BUILD_CPP}" -eq "1" ]; then
        if [ -z `which "${PERL_EXEC}"` ]; then
            echo -e "${YELLOW}[WARNING]:${NC} PERL not found, ${classic_cpp_folder}/qos_string.h will not be updated."
        else
            ${PERL_EXEC} ${cStringifyFile_script} ${qos_file} PERFTEST_QOS_STRING > ${classic_cpp_folder}/qos_string.h
            echo -e "${INFO_TAG} QoS String ${classic_cpp_folder}/qos_string.h updated successfully"
        fi
    fi
    if [ "${BUILD_CPP03}" -eq "1" ]; then
        if [ -z `which "${PERL_EXEC}"` ]; then
            echo -e "${YELLOW}[WARNING]:${NC} PERL not found, ${modern_cpp_folder}/qos_string.h will not be updated."
        else
            ${PERL_EXEC} ${cStringifyFile_script} ${qos_file} PERFTEST_QOS_STRING > ${modern_cpp_folder}/qos_string.h
            echo -e "${INFO_TAG} QoS String ${modern_cpp_folder}/qos_string.h updated successfully"
        fi
    fi
}

function copy_src_cpp_common()
{
    for file in ${common_cpp_folder}/*
    do
        if [ -f $file ]; then
            cp -rf "$file" "${classic_cpp_folder}"
            cp -rf "$file" "${modern_cpp_folder}"
        fi
    done
}

function clean_src_cpp_common()
{
    for file in ${common_cpp_folder}/*
    do
        if [ -f $file ]; then
            name_file=$(basename $file)
            rm -rf "${modern_cpp_folder}/${name_file}"
            rm -rf "${classic_cpp_folder}/${name_file}"
        fi
    done
}

function check_flatData_zeroCopy_available()
{
    version=$(awk -F"version" '/version/ { split($2, a, " "); print a[1] }' <<< $(${rtiddsgen_executable} -version)) # e.g. 3.0.0
    # We just need the Major value of the version.
    major=`echo $version | awk -F. '{print $1}'`

    if [[ $major -ge $flatdata_ddsgen_version ]]; then
        echo -e "${INFO_TAG} FlatData is available"
        FLATDATA_AVAILABLE="1"
    fi

    if [[ "${FLATDATA_AVAILABLE}" == "1" ]] && [[ $platform != *"Android"* ]]; then
        echo -e "${INFO_TAG} Zero-Copy is available"
        ZEROCOPY_AVAILABLE="1"
    fi
}

function build_cpp()
{
    copy_src_cpp_common

    ##############################################################################
    # Generate files for the custom type files
    additional_defines_custom_type=""
    additional_header_files_custom_type=""
    additional_source_files_custom_type=""

    if [ "${USE_CUSTOM_TYPE}" == "1" ]; then
        build_cpp_custom_type
    fi

    check_flatData_zeroCopy_available
    additional_defines_calculation "CPPtraditional"

    additional_header_files="${additional_header_files_custom_type} \
        RTIRawTransportImpl.h \
        ThreadPriorities.h \
        Parameter.h \
        ParameterManager.h \
        RTIDDSLoggerDevice.h \
        MessagingIF.h \
        RTIDDSImpl.h \
        perftest_cpp.h \
        qos_string.h \
        CpuMonitor.h \
        PerftestTransport.h \
        PerftestSecurity.h \
        Infrastructure_common.h \
        Infrastructure_pro.h"

    additional_source_files="${additional_source_files_custom_type} \
        RTIRawTransportImpl.cxx \
        ThreadPriorities.cxx \
        Parameter.cxx \
        ParameterManager.cxx \
        RTIDDSLoggerDevice.cxx \
        RTIDDSImpl.cxx \
        CpuMonitor.cxx \
        PerftestTransport.cxx \
        PerftestSecurity.cxx \
        Infrastructure_common.cxx \
        Infrastructure_pro.cxx"

    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        additional_header_files="${additional_header_files} \
        perftest_ZeroCopy.h \
        perftest_ZeroCopyPlugin.h \
        perftest_ZeroCopySupport.h"

        additional_source_files="${additional_source_files} \
        perftest_ZeroCopy.cxx \
        perftest_ZeroCopyPlugin.cxx \
        perftest_ZeroCopySupport.cxx"
    fi

    ##############################################################################
    # Generate files for srcCpp

    # rtiddsgen ignores any specified rti addional library if using ZeroCopy
    # Therefore, we need to generate a makefile that contains
    # nddsmetp and nddssecurity libraries without compiling ZeroCopy code
    rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${classic_cpp_lang_string} \
        ${additional_defines_flatdata} \
        -unboundedSupport -replace -create typefiles -create makefiles \
        -platform ${platform} \
        -additionalHeaderFiles \"${additional_header_files}\" \
        -additionalSourceFiles \"${additional_source_files} \" \
        -additionalDefines \"${additional_defines}\" \
        -additionalRtiLibraries \"${additional_rti_libs} \" \
        ${rtiddsgen_extra_options} ${additional_defines_custom_type} \
        -d \"${classic_cpp_folder}\" \"${idl_location}/perftest.idl\""

    echo ""
    echo -e "${INFO_TAG} Generating types and makefiles for ${classic_cpp_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string}."
        clean_src_cpp_common
        exit -1
    fi

    # Generate ZeroCopy types avoiding performance degradation issue
    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        echo -e "${INFO_TAG} Generating Zero Copy code"
        rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${classic_cpp_lang_string} \
        ${additional_defines_flatdata} \
        -replace -create typefiles \
        -platform ${platform} \
        ${rtiddsgen_extra_options} ${additional_defines_custom_type} \
        -d \"${classic_cpp_folder}\" \"${idl_location}/perftest_ZeroCopy.idl\""

        echo -e "${INFO_TAG} Command: $rtiddsgen_command"
        eval $rtiddsgen_command
        if [ "$?" != 0 ]; then
            echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string}."
            clean_src_cpp_common
            exit -1
        fi

        rm -rf ${classic_cpp_folder}/makefile_perftest_ZeroCopy_${platform}
    fi

    cp "${classic_cpp_folder}/perftest_publisher.cxx" \
    "${classic_cpp_folder}/perftest_subscriber.cxx"

    ##############################################################################
    # Compile srcCpp code
    #
    # If the user requested a specific compiler or linker, set up those variables
    # now and feed them to the command we run against the makefile.
    if [ ! -z "$COMPILER_EXE" ]; then
        export COMPILER=$COMPILER_EXE
    fi
    if [ ! -z "$LINKER_EXE" ]; then
        export LINKER=$LINKER_EXE
    fi
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp"
    "${MAKE_EXE}" -C "${classic_cpp_folder}" -f makefile_perftest_${platform}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${classic_cpp_lang_string}."
        clean_src_cpp_common
        exit -1
    fi
    echo -e "${INFO_TAG} Compilation successful"

    # If the platform is android, there are other compile and build steps to be done
    # in order to create the shared object (.so), create the projects and build the apk.
    if [[ ${platform} == *"Android"* ]]; then
        echo ""
        echo -e "${INFO_TAG} Additional compile steps for Android architectures"
        "${MAKE_EXE}" -C "${classic_cpp_folder}" -f makefile_perftest_${platform} perftest.so perftest.projects perftest.apks
        if [ "$?" != 0 ]; then
                echo -e "${ERROR_TAG} Failure building apk in ${classic_cpp_lang_string} of ${platform}."
                clean_src_cpp_common
                exit -1
        fi
        echo -e "${INFO_TAG} Building apk successful"
    fi

    echo ""
    echo -e "${INFO_TAG} Copying executable into: \"bin/${platform}/${RELEASE_DEBUG}\" folder"

    # Create bin folder if not exists and copy executables, since this command
    # has to work for several different architectures, we will try to find the
    # executable name with different line endings.
    perftest_cpp_name_beginning="${classic_cpp_folder}/objs/${platform}/perftest_publisher"
    executable_extension=""
    destination_folder="${bin_folder}/${platform}/${RELEASE_DEBUG}"
    mkdir -p "${bin_folder}/${platform}/${RELEASE_DEBUG}"

    # In Android the path of the built apk slightly differs from other built binaries.
    if [[ ${platform} == *"Android"* ]]; then
        perftest_cpp_name_beginning="${classic_cpp_folder}/objs/${platform}/publisher/bin/perftest_publisher-debug"
    fi

    if [ -e "$perftest_cpp_name_beginning" ]; then
        executable_extension=""
    elif [ -e "${perftest_cpp_name_beginning}.so" ]; then
        executable_extension=".so"
    elif [ -e "${perftest_cpp_name_beginning}.lo" ]; then
        executable_extension=".lo"
    elif [ -e "${perftest_cpp_name_beginning}.vxe" ]; then
        executable_extension=".vxe"
    elif [ -e "${perftest_cpp_name_beginning}.apk" ]; then
        executable_extension=".apk"
    fi
    cp -f "${perftest_cpp_name_beginning}${executable_extension}" \
    "${destination_folder}/perftest_cpp${executable_extension}"

    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copying code for ${classic_cpp_lang_string}."
        clean_src_cpp_common
        exit -1
    else
        echo -e "${INFO_TAG} Copy successful for ${classic_cpp_lang_string}."
        clean_src_cpp_common
    fi

    if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            echo -e "${INFO_TAG} Code compiled dynamically, Add \"${NDDSHOME}/lib/${platform}\""
            if [ "${USE_SECURE_LIBS}" == "1" ]; then
                echo -e "        and <OPENSSL_HOME>/${RELEASE_DEBUG}/lib"
            fi
            echo -e "        to your LD_LIBRARY_PATH or DYLD_LIBRARY_PATH"
    fi
}

function build_micro_cpp()
{
    copy_src_cpp_common
    additional_defines_calculation_micro

    ##############################################################################
    # Generate files for srcCpp
    if [ "${BUILD_MICRO_24x_COMPATIBILITY}" -eq "1" ]; then
        additional_defines=${additional_defines}" RTI_MICRO_24x_COMPATIBILITY"
    else
        rtiddsgen_extra_options="${rtiddsgen_extra_options} -sequenceSize ${MICRO_UNBOUNDED_SEQUENCE_SIZE}"

        if [ "${USE_SECURE_LIBS}" != "1" ]; then
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalRtiLibraries \"nddsmetp\""
        else
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalRtiLibraries \"rti_me_netioshmem rti_me_netioshmem rti_me_seccore\" -additionalLibraries \"sslz cryptoz\" -additionalLibraryPaths \"${RTI_OPENSSLHOME}/${RELEASE_DEBUG}/lib\""
        fi
    fi

    additional_header_files=" \
        ThreadPriorities.h \
        Parameter.h \
        ParameterManager.h \
        MessagingIF.h \
        RTIDDSImpl.h \
        perftest_cpp.h \
        CpuMonitor.h \
        PerftestTransport.h \
        PerftestSecurity.h \
        Infrastructure_common.h \
        Infrastructure_micro.h"

    additional_source_files=" \
        ThreadPriorities.cxx \
        Parameter.cxx \
        ParameterManager.cxx \
        RTIDDSImpl.cxx \
        CpuMonitor.cxx \
        PerftestTransport.cxx \
        PerftestSecurity.cxx \
        Infrastructure_common.cxx \
        Infrastructure_micro.cxx"

    rtiddsgen_command="\"${rtiddsgen_executable}\" -micro -language ${classic_cpp_lang_string} \
            -replace -create typefiles -create makefiles \
            -additionalHeaderFiles \"$additional_header_files\" \
            -additionalSourceFiles \"$additional_source_files\" \
            -additionalDefines \"${additional_defines}\" \
            ${rtiddsgen_extra_options} -d \"${classic_cpp_folder}\" \"${idl_location}/perftest.idl\" "

    echo ""
    echo -e "${INFO_TAG} Generating types and makefiles for ${classic_cpp_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string}."
        exit -1
    fi
    cp "${classic_cpp_folder}/perftest_publisher.cxx" "${classic_cpp_folder}/perftest_subscriber.cxx"
    cp "${idl_location}/perftest.idl" "${classic_cpp_folder}/perftest.idl"
    touch "${classic_cpp_folder}/perftestApplication.cxx"
    touch "${classic_cpp_folder}/perftestApplication.h"

    ##############################################################################
    # Compile srcCpp code
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp"
    cd "${classic_cpp_folder}"

    cmake_generate_command="${CMAKE_EXE} -DCMAKE_BUILD_TYPE=${RELEASE_DEBUG} -G \"Unix Makefiles\" -B./perftest_build -H. -DRTIME_TARGET_NAME=${platform} -DPLATFORM_LIBS=\"dl;m;pthread;${additional_included_libraries}\""

	echo -e "${INFO_TAG} Cmake Generate Command: $cmake_generate_command"
    eval $cmake_generate_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating unix makefiles with cmake for ${classic_cpp_lang_string}."
        cd ..
        exit -1
    fi

	cmake_build_command="${CMAKE_EXE} --build ./perftest_build --config ${RELEASE_DEBUG} --target perftest_publisher"
    	echo -e "${INFO_TAG} Cmake Build Command: $cmake_build_command"
    eval $cmake_build_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${classic_cpp_lang_string}."
        cd ..
        exit -1
    fi

    echo -e "${INFO_TAG} Compilation successful"
    cd ..

    echo ""
    echo -e "${INFO_TAG} Copying executable into: \"bin/${platform}/${RELEASE_DEBUG}\" folder"

    # Create bin folder if not exists and copy executables, since this command
    # has to work for several different architectures, we will try to find the
    # executable name with different line endings.
    perftest_cpp_name_beginning="${classic_cpp_folder}/objs/${platform}/perftest_publisher"
    executable_extension=""
    destination_folder="${bin_folder}/${platform}/${RELEASE_DEBUG}"
    mkdir -p "${bin_folder}/${platform}/${RELEASE_DEBUG}"

    if [ -e "$perftest_cpp_name_beginning" ]; then
        executable_extension=""
    elif [ -e "${perftest_cpp_name_beginning}.so" ]; then
        executable_extension=".so"
    elif [ -e "${perftest_cpp_name_beginning}.lo" ]; then
        executable_extension=".lo"
    elif [ -e "${perftest_cpp_name_beginning}.vxe" ]; then
        executable_extension=".vxe"
    fi
    cp -f "${perftest_cpp_name_beginning}${executable_extension}" \
    "${destination_folder}/perftest_cpp_micro${executable_extension}"
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copying code for ${classic_cpp_lang_string}."
        clean_src_cpp_common
        exit -1
    else
        echo -e "${INFO_TAG} Copy successful for ${classic_cpp_lang_string}."
        clean_src_cpp_common
    fi
}

function build_cpp03()
{
    copy_src_cpp_common
    check_flatData_zeroCopy_available
    additional_defines_calculation "CPPModern"

    additional_header_files=" \
        ThreadPriorities.h \
        Parameter.h \
        ParameterManager.h \
        MessagingIF.h \
        RTIDDSImpl.h \
        perftest_cpp.h \
        qos_string.h \
        CpuMonitor.h \
        PerftestTransport.h"

    additional_source_files=" \
        ThreadPriorities.cxx \
        Parameter.cxx \
        ParameterManager.cxx \
        RTIDDSImpl.cxx \
        CpuMonitor.cxx \
        PerftestTransport.cxx"

    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        additional_header_files="${additional_header_files} \
        perftest_ZeroCopy.hpp \
        perftest_ZeroCopyPlugin.hpp"

        additional_source_files="${additional_source_files} \
        perftest_ZeroCopy.cxx \
        perftest_ZeroCopyPlugin.cxx"
    fi


    ##############################################################################
    # Generate files for srcCpp03
    rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${modern_cpp_lang_string} \
    ${additional_defines_flatdata} \
    -unboundedSupport -replace -create typefiles -create makefiles \
    -platform ${platform} \
    -additionalHeaderFiles \"$additional_header_files\" \
    -additionalSourceFiles \"$additional_source_files\" \
    -additionalDefines \"${additional_defines}\" \
    -additionalRtiLibraries \"${additional_rti_libs} \" \
    ${rtiddsgen_extra_options} \
    -d \"${modern_cpp_folder}\" \"${idl_location}/perftest.idl\""

    echo ""
    echo -e "${INFO_TAG} Generating types and makefiles for ${modern_cpp_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${modern_cpp_lang_string}."
        clean_src_cpp_common
        exit -1
    fi

    # Generate Zero Copy types avoiding performance degradation issue
    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        echo -e "${INFO_TAG} Generating Zero Copy code"
        rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${modern_cpp_lang_string} \
        ${additional_defines_flatdata} \
        -replace -create typefiles -platform ${platform} \
        ${rtiddsgen_extra_options} \
        -d \"${modern_cpp_folder}\" \"${idl_location}/perftest_ZeroCopy.idl\""

        echo -e "${INFO_TAG} Command: $rtiddsgen_command"
        eval $rtiddsgen_command
        if [ "$?" != 0 ]; then
            echo -e "${ERROR_TAG} Failure generating code for ${modern_cpp_lang_string}."
            clean_src_cpp_common
            exit -1
        fi

        rm -rf ${modern_cpp_folder}/makefile_perftest_ZeroCopy_${platform}
    fi

    cp "${modern_cpp_folder}/perftest_publisher.cxx" \
    "${modern_cpp_folder}/perftest_subscriber.cxx"

    ##############################################################################
    # Compile srcCpp03 code
    #
    # If the user requested a specific compiler or linker, set up those variables
    # now and feed them to the command we run against the makefile.
    if [ ! -z "$COMPILER_EXE" ]; then
        export COMPILER=$COMPILER_EXE
    fi
    if [ ! -z "$LINKER_EXE" ]; then
        export LINKER=$LINKER_EXE
    fi
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp03."
    "${MAKE_EXE}" -C "${modern_cpp_folder}" -f makefile_perftest_${platform}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${modern_cpp_folder}."
        clean_src_cpp_common
        exit -1
    fi
    echo -e "${INFO_TAG} Compilation successful"

    # If the platform is android, there are other compile and build steps to be done
    # in order to create the shared object (.so), create the projects and build the apk.
    if [[ ${platform} == *"Android"* ]]; then
        echo ""
        echo -e "${INFO_TAG} Additional compile steps for Android architectures"
        "${MAKE_EXE}" -C "${modern_cpp_folder}" -f makefile_perftest_${platform} perftest.so perftest.projects perftest.apks
        if [ "$?" != 0 ]; then
                echo -e "${ERROR_TAG} Failure building apk in ${modern_cpp_folder} of ${platform}."
                clean_src_cpp_common
                exit -1
        fi
        echo -e "${INFO_TAG} Building apk successful"
    fi

    echo ""
    echo -e "${INFO_TAG} Copying executable into: \"bin/${platform}/${RELEASE_DEBUG}\" folder"

    # Create bin folder if not exists and copy executables, since this command
    # has to work for several different architectures, we will try to find the
    # executable name with different line endings.
    perftest_cpp03_name_beginning="${modern_cpp_folder}/objs/${platform}/perftest_publisher"
    executable_extension=""
    destination_folder="${bin_folder}/${platform}/${RELEASE_DEBUG}"
    mkdir -p "${bin_folder}/${platform}/${RELEASE_DEBUG}"

    # In Android the path of the built apk slightly differs from other built binaries.
    if [[ ${platform} == *"Android"* ]]; then
        perftest_cpp03_name_beginning="${modern_cpp_folder}/objs/${platform}/publisher/bin/perftest_publisher-debug"
    fi

    if [ -e "$perftest_cpp03_name_beginning" ]; then
        executable_extension=""
    elif [ -e "${perftest_cpp03_name_beginning}.so" ]; then
        executable_extension=".so"
    elif [ -e "${perftest_cpp03_name_beginning}.lo" ]; then
        executable_extension=".lo"
    elif [ -e "${perftest_cpp03_name_beginning}.vxe" ]; then
        executable_extension=".vxe"
    elif [ -e "${perftest_cpp03_name_beginning}.apk" ]; then
        executable_extension=".apk"
    fi
    cp -f "${perftest_cpp03_name_beginning}${executable_extension}" \
    "${destination_folder}/perftest_cpp03${executable_extension}"
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copying code for ${classic_cpp_lang_string}."
        clean_src_cpp_common
        exit -1
    else
        echo -e "${INFO_TAG} Copy successful for ${classic_cpp_lang_string}."
        clean_src_cpp_common
    fi

    if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            echo -e "${INFO_TAG} Code compiled dynamically, Add \"${NDDSHOME}/lib/${platform}\""
            if [ "${USE_SECURE_LIBS}" == "1" ]; then
                echo -e "        and <OPENSSL_HOME>/${RELEASE_DEBUG}/lib"
            fi
            echo -e "        to your LD_LIBRARY_PATH or DYLD_LIBRARY_PATH"
    fi
}

function build_java()
{
    # Check whether we need the release or debug jar file
    echo ""
    if [ "${RELEASE_DEBUG}" == debug ]; then
        echo -e "${INFO_TAG} Java libraries will be compiled against RTI debug libraries."
        rtidds_java_jar="${NDDSHOME}/lib/java/nddsjavad.jar"
    else
        echo -e "${INFO_TAG} Java libraries will be compiled against RTI release libraries."
        rtidds_java_jar="${NDDSHOME}/lib/java/nddsjava.jar"
    fi

    ##############################################################################
    # Generate files for srcJava

    rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${java_lang_string} -unboundedSupport -replace -package com.rti.perftest.gen -d \"${java_folder}\" \"${idl_location}/perftest.idl\""

    echo ""
    echo -e "${INFO_TAG} Generating types and makefiles for ${java_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command

    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${java_lang_string}."
        exit -1
    fi
    mkdir -p "${java_folder}/class"
    mkdir -p "${java_folder}/jar/${RELEASE_DEBUG}"

    echo ""
    echo -e "${INFO_TAG} Executing javac"
    ${JAVAC_EXE} -d "${java_folder}/class" -cp "${rtidds_java_jar}" \
    "${java_folder}/com/rti/perftest/"*.java \
    "${java_folder}/com/rti/perftest/ddsimpl/"*.java \
    "${java_folder}/com/rti/perftest/gen/"*.java \
    "${java_folder}/com/rti/perftest/harness/"*.java
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure Doing javac."
        exit -1
    fi

    echo -e "${INFO_TAG} Generating jar"
    ${JAR_EXE} cf "${java_folder}/jar/${RELEASE_DEBUG}/perftest_java.jar" \
    -C "${java_folder}/class" .


    echo -e "${INFO_TAG} Copying files"
    mkdir -p "${bin_folder}/${RELEASE_DEBUG}"
    cp -f "${java_folder}/jar/${RELEASE_DEBUG}/perftest_java.jar" \
    "${bin_folder}/${RELEASE_DEBUG}/perftest_java.jar"
    cp -f "${java_scripts_folder}/perftest_java.sh" \
    "${bin_folder}/${RELEASE_DEBUG}/perftest_java.sh"
    chmod +x "${bin_folder}/${RELEASE_DEBUG}/perftest_java.sh"

    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copying jar."
        exit -1
    fi

    echo ""
    echo -e "${INFO_TAG} You can run the java .jar file by using the following command:"
    echo ""
    echo "${JAVA_EXE} -cp \"${bin_folder}/${RELEASE_DEBUG}/perftest_java.jar:${rtidds_java_jar}\" com.rti.perftest.ddsimpl.PerfTestLauncher"
    echo ""
    echo "You will need to set the \$LD_LIBRARY_PATH or \$DYLD_LIBRARY_PATH for this script."
    echo ""
    echo -e "${INFO_TAG} Or by running the following script:"
    echo ""
    echo "\"${bin_folder}/${RELEASE_DEBUG}/perftest_java.sh\""
    echo ""
    echo "You will need to set the \$NDDSHOME and \$RTI_PERFTEST_ARCH for this script."
    echo ""
    echo -e "${INFO_TAG} Compilation successful"

}

################################################################################
function clean_documentation()
{
    # Remove the content of ${doc_folder}/_build
    rm -rf ${doc_folder}/_build
    rm -rf ${generate_doc_folder}
}

function build_documentation()
{

    # Generate HTML
    echo ""
    echo -e "${INFO_TAG} Generating HTML documentation"
    cd ${doc_folder}
    ${MAKE_EXE} -f Makefile html > /dev/null 2>&1
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating HTML documentation"
        echo -e "${ERROR_TAG} You will need to install:
            sudo pip install -U sphinx
            sudo pip install sphinx_rtd_theme"
        exit -1
    fi
    rm -rf ${generate_doc_folder}/html
    mkdir -p ${generate_doc_folder}/html
    cp -rf ${doc_folder}/_build/html ${generate_doc_folder}/
    echo -e "${INFO_TAG} HTML Generation successful. You will find it under:
        ${generate_doc_folder}/html/index.html"


    # Generate PDF
    echo ""
    echo -e "${INFO_TAG} Generating PDF documentation"
    cd ${doc_folder}
    ${MAKE_EXE} -f Makefile latexpdf > /dev/null 2>&1
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating PDF documentation"
        echo -e "${ERROR_TAG} On Linux systems you might need to install 'texlive-full'."
        exit -1
    fi
    rm -rf ${generate_doc_folder}/pdf
    mkdir -p ${generate_doc_folder}/pdf
    cp -rf ${doc_folder}/_build/latex/RTI_Perftest.pdf ${generate_doc_folder}/pdf/RTI_Perftest_UsersManual.pdf
    echo -e "${INFO_TAG} PDF Generation successful. You will find it under:
        ${generate_doc_folder}/pdf/RTI_Perftest.pdf"

}

################################################################################
# Initial message
echo ""
echo "================================ RTI PERFTEST: ================================="

if [ "$#" -eq "0" ]; then
  usage
  exit -1
fi

while [ "$1" != "" ]; do
    case $1 in
        -h | --help)
            usage
            exit
            ;;
        -clean | --clean)
            clean
            exit
            ;;
        --micro)
            BUILD_MICRO=1
            ;;
        --micro-24x-compatibility)
            BUILD_MICRO=1
            BUILD_MICRO_24x_COMPATIBILITY=1
            ;;
        --skip-java-build)
            BUILD_JAVA=0
            ;;
        --skip-cpp-build)
            BUILD_CPP=0
            ;;
        --skip-cpp03-build)
            BUILD_CPP03=0
            ;;
        --java-build)
            BUILD_JAVA=1
            BUILD_CPP=0
            BUILD_CPP03=0
            ;;
        --cpp-build)
            BUILD_JAVA=0
            BUILD_CPP=1
            BUILD_CPP03=0
            ;;
        --cpp03-build)
            BUILD_JAVA=0
            BUILD_CPP=0
            BUILD_CPP03=1
            ;;
        --make)
            MAKE_EXE=$2
            shift
            ;;
        --cmake)
            CMAKE_EXE=$2
            shift
            ;;
        --compiler)
            COMPILER_EXE=$2
            shift
            ;;
        --linker)
            LINKER_EXE=$2
            shift
            ;;
        --perl)
            PERL_EXEC=$2
            shift
            ;;
        --java-home)
            JAVA_HOME=$2
            JAVAC_EXE="${JAVA_HOME}/bin/javac"
            JAVA_EXE="${JAVA_HOME}/bin/java"
            JAR_EXE="${JAVA_HOME}/bin/jar"
            shift
            ;;
        --platform)
            platform=$2
            shift
            ;;
        --nddshome)
            export NDDSHOME=$2
            shift
            ;;
        --rtimehome)
            export RTIMEHOME=$2
            shift
            ;;
        --debug)
            RELEASE_DEBUG=debug
            ;;
        --ns-resolution)
            RTI_PERFTEST_NANO_CLOCK=1
            ;;
        --dynamic)
            STATIC_DYNAMIC=dynamic
            ;;
        --secure)
            USE_SECURE_LIBS=1
            ;;
        --legacy-DynamicData)
            LEGACY_DD_IMPL=1
            ;;
        --customType)
            USE_CUSTOM_TYPE=1
            custom_type=$2
            if [ -z "${custom_type}" ]; then
                echo -e "${ERROR_TAG} --customType should be followed by the name of the type."
                usage
                exit -1
            fi
            shift
            ;;
        --customTypeFlatData)
            USE_CUSTOM_TYPE_FLAT=1
            custom_type_flat=$2
            if [ -z "${custom_type_flat}" ]; then
                echo -e "${ERROR_TAG} --customTypeFlatData should be followed by the name of the type."
                usage
                exit -1
            fi
            shift
            ;;
        --build-doc)
            build_documentation
            exit 0
            ;;
        --openssl-home)
            RTI_OPENSSLHOME=$2
            shift
            ;;
        --flatData-max-size)
            RTI_FLATDATA_MAX_SIZE=$2
            sizeInt=$(($RTI_FLATDATA_MAX_SIZE + 0)) # For OSX
            if [[ $sizeInt -le 0 ]]; then
                echo -e "${ERROR_TAG} \"--flatData-max-size n\" requires n > 0."
                exit -1
            fi
            shift
            ;;
        --osx-shmem-shmmax)
            darwin_shmem_size=$2
            shift
            ;;
        *)
            echo -e "${ERROR_TAG} unknown parameter \"$1\""
            usage
            exit -1
            ;;
    esac
    shift
done

executable_checking

if [ "${BUILD_MICRO}" -eq "1" ]; then

    rtiddsgen_executable="$RTIMEHOME/rtiddsgen/scripts/rtiddsgen"

    classic_cpp_lang_string=C++
    if [ "${BUILD_CPP}" -eq "1" ]; then
        library_sufix_calculation
        build_micro_cpp
    fi

else

    rtiddsgen_executable="$NDDSHOME/bin/rtiddsgen"

    classic_cpp_lang_string=C++
    modern_cpp_lang_string=C++03
    java_lang_string=java

    # Generate qos_string.h
    generate_qos_string

    if [ "${BUILD_CPP}" -eq "1" ]; then
        library_sufix_calculation
        build_cpp
    fi

    if [ "${BUILD_CPP03}" -eq "1" ]; then
        library_sufix_calculation
        build_cpp03
    fi

    if [ "${BUILD_JAVA}" -eq "1" ]; then
        build_java
    fi

fi

echo ""
echo "================================================================================"
echo ""
