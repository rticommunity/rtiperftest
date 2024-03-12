#!/bin/bash

################################################################################
# Variable declaration

filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`
idl_location="${script_location}/srcIdl"
classic_cpp_folder="${script_location}/srcCpp"
common_cpp_folder="${script_location}/srcCppCommon"
modern_cpp_folder="${script_location}/srcCpp11"
java_folder="${script_location}/srcJava"
java_scripts_folder="${script_location}/resource/scripts/java_execution_scripts"
cs_folder="${script_location}/srcCs"
cs_scripts_folder="${script_location}/resource/scripts/cs_execution_scripts"
bin_folder="${script_location}/bin"
cStringifyFile_script="${script_location}/resource/scripts/cStringifyFile.pl"

qos_file="${script_location}/perftest_qos_profiles.xml"
doc_folder="${script_location}/srcDoc"
generate_doc_folder="${script_location}/doc"
resource_folder="${script_location}/resource"

# By default we will not build TSS with Pro libs
BUILD_TSS=0
BUILD_TSS_PRO=0
FACE_COMPLIANCE="None"

# By default we will build pro, not micro
BUILD_MICRO=0

# In case we build micro, which version.
BUILD_MICRO_24x_COMPATIBILITY=0
MICRO_UNBOUNDED_SEQUENCE_SIZE=1048576


# Default values for building the different APIS:
BUILD_CPP=1
BUILD_CPP11=1
BUILD_JAVA=1
BUILD_CS=0

# If this value is != 0, then it means the user specified specific APIS to be
# built, and not everything.
BUILD_SPECIFIC_APIS=0

MAKE_EXE=make
CMAKE_EXE=cmake
ADDITIONAL_CMAKE_ARGS=""
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
USE_LW_SECURE_LIBS=0
LEGACY_DD_IMPL=0

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

# For C++ Classic, variable to control if we want to force the use
# of the C++11 infrastructure
RTI_USE_CPP_11_INFRASTRUCTURE=0

# We will use some colors to improve visibility of errors and information
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'
INFO_TAG="${GREEN}[INFO]:${NC}"
WARNING_TAG="${YELLOW}[WARNING]:${NC}"
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
    echo "    --skip-cpp11-build           Avoid C++ New PSM code generation and          "
    echo "                                 compilation.                                   "
    echo "                                 (No effect when building for Micro)            "
    echo "    --java-build                 Only Java ByteCode generation creation.        "
    echo "    --cpp-build                  Only C++ code generation and compilation.      "
    echo "    --cpp11-build                Only C++ New PSM code generation and           "
    echo "                                 compilation.                                   "
    echo "    --make <path>                Path to the GNU make executable. If this       "
    echo "                                 parameter is not present, the GNU make variable"
    echo "                                 should be available from your \$PATH variable. "
    echo "    --cmake <path>               Path to the cmake executable. If this          "
    echo "                                 parameter is not present, cmake executable     "
    echo "                                 should be available from your \$PATH variable. "
    echo "                                 will clean all the generated code and binaries "
    echo "    --add-cmake-args <s>         Additional defines and arguments that will     "
    echo "                                 be passed to the cmake executable when building"
    echo "                                 Micro. Default: Not set.                       "
    echo "    --compiler <path>            Path to (or name of) the compiler executable.  "
    echo "                                 If this parameter is not a full path, the named"
    echo "                                 executable should be available from your       "
    echo "                                 \$PATH variable. (NOTE: c++/c++11 builds only) "
    echo "    --linker <path>              Path to (or name of) the linker executable.    "
    echo "                                 If this parameter is not a full path, the named"
    echo "                                 executable should be available from your       "
    echo "                                 \$PATH variable. (NOTE: c++/c++11 builds only) "
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
    echo "    --secure / --security        Enable the security options for compilation.   "
    echo "                                 Default is not enabled.                        "
    echo "    --lightWeightSecurity        Enable the security options for compilation.   "
    echo "                                 using the lightweight security library.        "
    echo "                                 Default is not enabled.                        "
    echo "    --openssl-home <path>        Path to the openssl home. This will be used    "
    echo "                                 when compiling statically and using security   "
    echo "                                 Default is an empty string (current folder).   "
    echo "                                 <path>/release/lib or <path>/debug/lib         "
    echo "                                 (depending on the --debug flag) should exist.  "
    echo "    --wolfssl-home <path>        Path to the wolfssl home. This will be used    "
    echo "                                 when compiling statically and using security   "
    echo "                                 Default is an empty string (current folder).   "
    echo "                                 <path>/release/lib or <path>/debug/lib         "
    echo "                                 (depending on the --debug flag) should exist.  "
    echo "    --openssl-version <version>  Force the use of a specific openssl version.   "
    echo "                                 By default use openssl-3, else 1, else wolfssl."
    echo "    --wolfssl-version <version>  Force the use of a specific WolfSSL version.   "
    echo "                                 By default use openssl.                        "
    echo "    --ssl-version <version>      Force the use of a certain ssl version.        "
    echo "                                 By default use openssl-3, else 1, else wolfssl."
    echo "    --customType <type>          Use the Custom type feature with your type.    "
    echo "                                 See details and examples of use in the         "
    echo "                                 documentation.                                 "
    echo "    --customTypeFlatData <type>  Use the Custom type feature with your FlatData "
    echo "                                 Type. See details and examples of use in the   "
    echo "                                 documentation.                                 "
    echo "    --flatData-max-size <size>   Specify the maximum bounded size on bytes      "
    echo "                                 for sequences when using FlatData language     "
    echo "                                 binding. Default 10MB                          "
    echo "    --no-zeroCopy                Do not try to compile against Zero-Copy libs   "
    echo "    --osx-shmem-shmmax <size>    Maximum segment size for shared memory in OSX  "
    echo "                                 in bytes. Default 400MB                        "
    echo "    --ns-resolution              Try to use the system real-time clock to get   "
    echo "                                 nano-second resolution.                        "
    echo "                                 For the Classic C++ Implementation only.       "
    echo "                                 Default is not enabled.                        "
    echo "    --tss                        Build against Connext TSS (pro and micro)      "
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

    rm -f  "${script_location}"/srcC*/README_*.txt
    rm -f  "${script_location}"/srcC*/perftest.*
    rm -f  "${script_location}"/srcC*/perftest_ZeroCopy*
    rm -f  "${script_location}"/srcC*/perftestPlugin.*
    rm -f  "${script_location}"/srcC*/perftestSupport.*
    rm -f  "${script_location}"/srcC*/perftest_publisher.*
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
    clean_copied_files

    echo ""
    echo "================================================================================"
    echo ""

}

function executable_checking()
{
    echo -e "\n${INFO_TAG} Pre-compilation checks."
    # Is platform specified?
    if [ -z "${platform}" ]; then
        echo -e "${ERROR_TAG} The platform argument is missing"
        usage
        exit -1
    fi

    # If we are Building MICRO
    if [ "${BUILD_TSS}" -eq "1" ]; then
        if [ -z "${RTITSSHOME}" ]; then
            echo -e "${ERROR_TAG} The RTITSSHOME variable is not set or the path does not exist"
            usage
            exit -1
        elif [ "${BUILD_TSS_PRO}" -eq "1" ] && [ -z "${NDDSHOME}" ]; then
            echo -e "${ERROR_TAG} The NDDSHOME variable is not set or the path does not exist"
            usage
            exit -1
        elif [ "${BUILD_MICRO}" -eq "1" ] && [ -z "${RTIMEHOME}" ]; then
            echo -e "${ERROR_TAG} The RTIMEHOME variable is not set or the path does not exist"
            usage
            exit -1
        fi

    elif [ "${BUILD_MICRO}" -eq "1" ]; then

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
            echo -e "${WARNING_TAG} ${CMAKE_EXE} executable not found, perftest_cpp will not be built."
        fi

        # Is MAKE in the path?
        if [ "${BUILD_CPP}" -eq "1" ]; then
            if [ -z `which "${MAKE_EXE}"` ]
            then
                echo -e "${WARNING_TAG} ${MAKE_EXE} executable not found, perftest_cpp will not be built."
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
                echo -e "${WARNING_TAG} ${MAKE_EXE} executable not found, perftest_cpp will not be built."
            fi
        fi
        if [ "${BUILD_CPP11}" -eq "1" ]; then
            if [ -z `which "${MAKE_EXE}"` ]
            then
                echo -e "${WARNING_TAG} ${MAKE_EXE} executable not found, perftest_cpp11 will not be built."
            fi
        fi

        # Is COMPILER in the path?
        if [[ "${BUILD_CPP}" -eq "1" ]] && [[ ! -z ${COMPILER_EXE} ]]; then
            if [ -z `which "${COMPILER_EXE}"` ]
            then
                echo -e "${WARNING_TAG} ${COMPILER_EXE} executable not found, perftest_cpp will not be built."
                BUILD_CPP=0
            fi
        fi
        if [[ "${BUILD_CPP11}" -eq "1" ]] && [[ ! -z ${COMPILER_EXE} ]]; then
            if [ -z `which "${COMPILER_EXE}"` ]
            then
                echo -e "${WARNING_TAG} ${COMPILER_EXE} executable not found, perftest_cpp11 will not be built."
                BUILD_CPP11=0
            fi
        fi

        # Is LINKER in the path?
        if [[ "${BUILD_CPP}" -eq "1" ]] && [[ ! -z ${LINKER_EXE} ]]; then
            if [ -z `which "${LINKER_EXE}"` ]
            then
                echo -e "${WARNING_TAG} ${LINKER_EXE} executable not found, perftest_cpp will not be built."
                BUILD_CPP=0
            fi
        fi
        if [[ "${BUILD_CPP11}" -eq "1" ]] && [[ ! -z ${LINKER_EXE} ]]; then
            if [ -z `which "${LINKER_EXE}"` ]
            then
                echo -e "${WARNING_TAG} ${LINKER_EXE} executable not found, perftest_cpp11 will not be built."
                BUILD_CPP11=0
            fi
        fi


        if [ "${BUILD_CS}" -eq "1" ]; then
            if [ -z `which dotnet` ]; then
                echo -e "${YELLOW}[WARNING]:${NC} dotnet executable not found, perftest_java will not be built."
                BUILD_CS=0
            fi
        fi

        # Is JAVA in the path?
        if [ "${BUILD_JAVA}" -eq "1" ]; then
            if [ -z `which "${JAVAC_EXE}"` ]; then
                echo -e "${WARNING_TAG} javac executable not found, perftest_java will not be built."
                BUILD_JAVA=0
            fi
            if [ -z `which "${JAVA_EXE}"` ]; then
                echo -e "${WARNING_TAG} java executable not found, perftest_java will not be built."
                BUILD_JAVA=0
            fi
            if [ -z `which "${JAR_EXE}"` ]; then
                echo -e "${WARNING_TAG} jar executable not found, perftest_java will not be built."
                BUILD_JAVA=0
            fi
        fi

        # If the platform is android, we will just build C++ Classic by default.
        if [[ ${platform} == *"Android"* ]]; then
            BUILD_CPP11=0
            BUILD_JAVA=0
            BUILD_CS=0
        fi

    fi #TSS/Micro/Pro

    echo -e ""
}

# Check if a path that includes a wildcard exists and return the actual path
function get_absolute_folder_path()
{
    local input="$1"
    local matching_folders=("${input}"*)  # Store matching folders in an array

    if [ -d "${matching_folders[0]}" ]; then
        local current_dir=$PWD

        cd ${matching_folders[0]} # Move to the first matching folder

        if [[ "$?" == "0" ]]; then
            result=$PWD
        else
            result=""
        fi

        cd $current_dir
    fi
}

# This function receives the ssl folder pattern and it tries to find it in the
# third_party directory.
function find_ssl_libraries()
{
    local find_pattern=$1

    # We will try to recreate the RTI_CRYPTOHOME, if it exists
    # We will not modify it.

    if [ "${RTI_CRYPTOHOME}" == "" ]; then

        export result=""
        get_absolute_folder_path "$NDDSHOME/third_party/${find_pattern}"

        if [[ "$result" == "" ]]; then
            # The path does not exist.
            return
        else
            export RTI_CRYPTOHOME="${result}/${platform}"
            if [ -d "$RTI_CRYPTOHOME" ]; then
                echo -e "${INFO_TAG} Using the CRYPTO LIBS from: \"${RTI_CRYPTOHOME}\""
            else
                export RTI_CRYPTOHOME=""
                echo -e "${INFO_TAG} \"${RTI_CRYPTOHOME}\" does not exist."
                return
            fi
        fi
    fi
}

# If the NDDSHOME is staged, we might find that the nddssecurity library
# is not under $NDDSHOME/lib/$platform/ but under $NDDSHOME/lib/$platform/$ndds_security_cryto_lib_folder
# lets try to find this path. This is a best effort approach if we cannof find
# we will not fail.
function rti_security_lib_path_calculation()
{
    local find_pattern=$1

    export result=""
    get_absolute_folder_path "${NDDSHOME}/lib/${platform}/${find_pattern}"
    if [[ "$result" != "" ]]; then
        export ndds_security_cryto_lib_folder=$result
        echo -e "${INFO_TAG} Using the Connext Security libs from: \"${ndds_security_cryto_lib_folder}\""
        return
    fi
}

# If the NDDSHOME is staged, we might find that the nddssecurity library
# is not under $NDDSHOME/lib/$platform/ but under $NDDSHOME/lib/$platform/$ndds_security_cryto_lib_folder
# lets try to find this path. This is a best effort approach if we cannof find
# we will not fail.
function get_rti_security_lib_path_for_cryto_path()
{
    local ssl_version="openssl-3"
    if [[ "${RTI_CRYPTOHOME}" == *"$ssl_version"* ]]; then
        rti_security_lib_path_calculation $ssl_version
        return
    fi

    ssl_version="openssl-1"
    if [[ "${RTI_CRYPTOHOME}" == *"$ssl_version"* ]]; then
        rti_security_lib_path_calculation $ssl_version
        return
    fi

    ssl_version="wolfssl-"
    if [[ "${RTI_CRYPTOHOME}" == *"$ssl_version"* ]]; then
        rti_security_lib_path_calculation $ssl_version
        return
    fi
}

# Function to try and get the RTI_CRYPTOHOME and ndds_security_cryto_lib_folder when no
# SSL version is provided
function crypto_path_calculation()
{
    # We are going to try the following options:
    # - Find first openssl 3 and use that
    # - Find openssl 1 and use that
    # - Find WolfSSL and use that
    # - Fail if nothing was found.

    echo -e "\n${INFO_TAG} Finding crypto libraries to use:"

    local ssl_version="openssl-3"
    find_ssl_libraries $ssl_version
    if [[ "${RTI_CRYPTOHOME}" != "" ]]; then
        rti_security_lib_path_calculation $ssl_version
        USE_OPENSSL=1
        return
    fi

    ssl_version="openssl-1"
    find_ssl_libraries $ssl_version
    if [[ "${RTI_CRYPTOHOME}" != "" ]]; then
        rti_security_lib_path_calculation $ssl_version
        USE_OPENSSL=1
        return
    fi

    ssl_version="wolfssl-"
    find_ssl_libraries $ssl_version
    if [[ "${RTI_CRYPTOHOME}" != "" ]]; then
        rti_security_lib_path_calculation $ssl_version
        USE_OPENSSL=0
        return
    fi

    # Well, we tried...
    echo -e "${ERROR_TAG} We couldn't find Any SSL libraries in your Connext " \
            "installation folder. You need to provide us with a path to a" \
            "crypto library. Set either the OpenSSL home path by" \
            "using the --openssl-home option or the WolfSSL home path" \
            "by using the --wolfssl-home option"
    exit -1
}

function library_sufix_calculation()
{
    echo -e "\n${INFO_TAG} Library calculations"

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
    additional_rtiddsgen_defines="-D PERFTEST_RTI_PRO"
    additional_defines="DPERFTEST_RTI_PRO "
    additional_rti_libs=""
    additional_lib_paths=""

    # Avoid optimized out variables when debugging
    if [ "${RELEASE_DEBUG}" == "release" ]; then
        echo -e "${INFO_TAG} C++ code will be optimized."
        additional_defines=${additional_defines}" O3"
    else
        additional_defines=${additional_defines}" O0"
    fi

    if [ "${LEGACY_DD_IMPL}" == "1" ]; then
        echo -e "${INFO_TAG} Allow the use of both legacy and new Dynamic Data Impl."
        additional_defines=${additional_defines}" DRTI_LEGACY_DD_IMPL"
    fi

    if [ "${RTI_MONITORING_2}" == "1" ]; then
        echo -e "${INFO_TAG} Adding RTI Monitoring Libraries."
        additional_rti_libs="rtimonitoring2 ${additional_rti_libs}"
        additional_defines=${additional_defines}" DRTI_MONITORING_2"
    fi

    if [ "${USE_SECURE_LIBS}" == "1" ]; then

        if [ "${USE_LW_SECURE_LIBS}" == "1" ]; then
            LWS_TAG="Lightweight "
        fi

        echo -e "\n${INFO_TAG} Using RTI ${LWS_TAG}Security Libraries"

        additional_defines="${additional_defines} DRTI_SECURE_PERFTEST"

        if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            additional_defines=${additional_defines}" DRTI_PERFTEST_DYNAMIC_LINKING"
            echo -e "${INFO_TAG} Linking Dynamically."

        else # Linking Statically.

            if [ "${USE_LW_SECURE_LIBS}" == "1" ]; then
                additional_defines="${additional_defines} DRTI_LW_SECURE_PERFTEST"
            fi

            # If we have provided the SSL Version (Hence the RTI_CRYPTOHOME will
            # be empty, no need to check)
            if [[ "${SSL_VERSION}" != "" ]]; then
                find_ssl_libraries $SSL_VERSION
                if [[ "${RTI_CRYPTOHOME}" == "" ]]; then
                    echo -e "${ERROR_TAG} ${SSL_VERSION} Not found."
                    exit -1
                fi
            fi

            # If the SSL_VERSION is empty and the $RTI_CRYPTOHOME is empty too
            # we need to be creative. If we set the $SSL_VERSION before, we will
            # not enter here.
            if [ "${RTI_CRYPTOHOME}" == "" ]; then
                crypto_path_calculation
            else
                # Check that the RTI_CRYPTOHOME exists
                if [ ! -d ${RTI_CRYPTOHOME} ]; then
                    echo -e "${ERROR_TAG} ${RTI_CRYPTOHOME} Is not a folder."
                    exit -1
                fi

                # Lets try to load the ndds_security_cryto_lib_folder if we can.
                get_rti_security_lib_path_for_cryto_path
            fi

            if [ "${USE_LW_SECURE_LIBS}" == "1" ]; then 
                additional_rti_libs="nddslightweightsecurity ${additional_rti_libs}"
            else
                additional_rti_libs="nddssecurity ${additional_rti_libs}"
            fi

            # If the $NDDSHOME points to a staging directory, then the security
            # libraries will be in a folder specific to the crypto library, we should
            # have calculated this in advance.

            # At this point RTI_CRYPTOHOME must be set. ndds_security_cryto_lib_folder
            # will be set only if the $NDDSHOME points to a staging directory.
            # In a staging directory, ndds_security_cryto_lib_folder is where
            # the security libraries are (a folder specific to the crypto library). In a normal
            # installation, the security libraries are in
            # "${NDDSHOME}/lib/${platform}/", which is already in the path.
            if [ -d "${ndds_security_cryto_lib_folder}" ]; then
                additional_lib_paths="${ndds_security_cryto_lib_folder} ${additional_lib_paths}"
            fi
            # Add the path to the crypto libraries.
            additional_lib_paths="${RTI_CRYPTOHOME}/${RELEASE_DEBUG}/lib ${additional_lib_paths}"

            if [ "${USE_OPENSSL}" = "1" ]; then
                rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraries \"ssl crypto\""
            else
                rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraries \"wolfssl\""
            fi

            # This option would cause issues on certain platforms (like macOS)
            # export ADDITIONAL_LINKER_FLAGS="$ADDITIONAL_LINKER_FLAGS -static"
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraryPaths \"${additional_lib_paths}\""
            echo -e "${INFO_TAG} Linking Statically."
        fi
    fi

    if [ "${USE_CUSTOM_TYPE}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_CUSTOM_TYPE="${custom_type}" DRTI_CUSTOM_TYPE_FILE_NAME_SUPPORT="${custom_type_file_name_support}
    fi

    if [ "${USE_CUSTOM_TYPE_FLAT}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_CUSTOM_TYPE_FLATDATA="${custom_type_flat}" DRTI_CUSTOM_TYPE_FILE_NAME_SUPPORT="${custom_type_file_name_support}
    fi

    if [ "${1}" = "CppTraditional" ]; then
        additional_defines=${additional_defines}" DRTI_LANGUAGE_CPP_TRADITIONAL"

        if [ "${RTI_USE_CPP_11_INFRASTRUCTURE}" == "1" ]; then
            echo -e "${INFO_TAG} Force using C++11 and C++11 Infrastructure."
            additional_defines=${additional_defines}" DRTI_USE_CPP_11_INFRASTRUCTURE"
        fi

        if [ "${RTI_PERFTEST_NANO_CLOCK}" == "1" ]; then
            additional_defines=${additional_defines}" DRTI_PERFTEST_NANO_CLOCK"
        fi
    fi

    if [ "${1}" = "CppModern" ]; then
        additional_defines=${additional_defines}" DRTI_LANGUAGE_CPP_MODERN"
    fi

    if [[ ! -z `which git` ]] ; then
        local folder_before=$PWD
        cd $script_location
        commit_id=`git rev-parse --short HEAD`
        branch_name=`git rev-parse --abbrev-ref HEAD`
        cd $folder_before
        additional_defines=${additional_defines}" DPERFTEST_COMMIT_ID='\\\"${commit_id}\\\"'"
        additional_defines=${additional_defines}" DPERFTEST_BRANCH_NAME='\\\"${branch_name}\\\"'"
    fi

    if [[ $platform == *"Darwin"* ]]; then
        additional_defines=${additional_defines}" DMAX_DARWIN_SHMEM_SIZE=${darwin_shmem_size}"
    fi

    # Adding RTI_ZEROCOPY_AVAILABLE, RTI_FLATDATA_AVAILABLE and RTI_FLATDATA_MAX_SIZE as defines
    if [ "${FLATDATA_AVAILABLE}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_FLATDATA_AVAILABLE"
        additional_rtiddsgen_defines_flatdata=" -D RTI_FLATDATA_AVAILABLE"
        if [ "${RTI_FLATDATA_MAX_SIZE}" != "" ]; then
            additional_defines=${additional_defines}" DRTI_FLATDATA_MAX_SIZE=${RTI_FLATDATA_MAX_SIZE}"
            additional_rtiddsgen_defines_flatdata=${additional_rtiddsgen_defines_flatdata}" -D RTI_FLATDATA_MAX_SIZE=${RTI_FLATDATA_MAX_SIZE}"
        fi

        if [ "${SKIP_ZEROCOPY}" ==  "1" ]; then
            export ZEROCOPY_AVAILABLE="0"
        fi

        if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
            additional_rti_libs="nddsmetp ${additional_rti_libs}"
            additional_defines=${additional_defines}" DRTI_ZEROCOPY_AVAILABLE"
            additional_rtiddsgen_defines_flatdata=$additional_rtiddsgen_defines_flatdata" -D RTI_ZEROCOPY_AVAILABLE"
        fi
    fi

    additional_rtiddsgen_defines="$additional_rtiddsgen_defines $additional_rtiddsgen_defines_flatdata"
}

function additional_defines_calculation_micro()
{
    additional_rtiddsgen_defines="-D PERFTEST_RTI_MICRO"

    if [[ $platform == *"Darwin"* ]]; then
        additional_defines=" RTI_DARWIN"
        additional_included_libraries="dl;m;pthread;"
    elif [[ $platform == *"Linux"* ]]; then
        additional_defines=" RTI_LINUX"
        additional_included_libraries="dl;m;pthread;nsl;rt;"
    elif [[ $platform == *"QNX"* ]]; then
        additional_defines=" RTI_QNX"
        additional_included_libraries="m;socket;"
    fi
    additional_defines="RTI_LANGUAGE_CPP_TRADITIONAL PERFTEST_RTI_MICRO O3"${additional_defines}

    if [ "${RTI_PERFTEST_NANO_CLOCK}" == "1" ]; then
        additional_defines=${additional_defines}" DRTI_PERFTEST_NANO_CLOCK"
    fi

    if [ "${RTI_USE_CPP_11_INFRASTRUCTURE}" == "1" ]; then
        echo -e "${INFO_TAG} Force using C++11 and C++11 Infrastructure."
        additional_defines=${additional_defines}" DRTI_USE_CPP_11_INFRASTRUCTURE"
    fi

    if [ "${USE_SECURE_LIBS}" == "1" ]; then
        additional_defines="${additional_defines} RTI_SECURE_PERFTEST"

        if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            echo -e "${INFO_TAG} Using Security Plugins. Linking Dynamically."
        else
            if [ "${RTI_CRYPTOHOME}" == "" ]; then
                # In this case, we are going to try to use the one that should be
                # under $NDDSHOME/third_party/<crypto_lib_dir>/<arch>, we will check if
                # it exists.
                export RTI_CRYPTOHOME="$NDDSHOME/third_party/${RTI_CRYPTO_LIB_DIRECTORY}${RTI_CRYPTO_LIB_DIRECTORY_VERSION}/${platform}"
                if [ ! -f "${RTI_CRYPTOHOME}" ]; then
                    # Well, we tried...
                    echo -e "${ERROR_TAG} In order to link statically using the "\
                        "Security Plugins you need to also provide a path to a"\
                        "crypto library. Set either the OpenSSL home path by"\
                        "using the --openssl-home option or the WolfSSL home path"\
                        "by using the --wolfssl-home option"
                    exit -1
                fi
                echo -e "${INFO_TAG} Using the CRYPTO LIBS FROM openSSL from: \"${RTI_CRYPTOHOME}\""
            fi
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
            echo -e "${INFO_TAG} Command (rtiddsgen custom type): $rtiddsgen_command"
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
    if [ "${BUILD_CPP}" -eq "1" ] || [ "${BUILD_CPP11}" -eq "1" ]; then
        if [ -z `which "${PERL_EXEC}"` ]; then
            echo -e "${WARNING_TAG} PERL not found, ${common_cpp_folder}/qos_string.h will not be updated."
        else
            ${PERL_EXEC} ${cStringifyFile_script} ${qos_file} PERFTEST_QOS_STRING > ${common_cpp_folder}/qos_string.h
            echo -e "${INFO_TAG} QoS String ${common_cpp_folder}/qos_string.h updated successfully"
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

function copy_src_cpp_connextDDS()
{
    # Copy files in the common folder for pro and micro
    for file in ${classic_cpp_folder}/connextDDS/*
    do
        if [ -f $file ]; then
            cp -rf "$file" "${classic_cpp_folder}"
        fi
    done

    # Copy now files specific for pro/micro
    src_specific_folder="pro"
    if [ ${BUILD_MICRO} != 0 ]; then
        src_specific_folder="micro"
    fi

    for file in ${classic_cpp_folder}/connextDDS/${src_specific_folder}/*
    do
        if [ -f $file ]; then
            cp -rf "$file" "${classic_cpp_folder}"
        fi
    done
}

function clean_copied_files()
{
    for file in ${common_cpp_folder}/*
    do
        if [ -f $file ]; then
            name_file=$(basename $file)
            rm -rf "${modern_cpp_folder}/${name_file}"
            rm -rf "${classic_cpp_folder}/${name_file}"
        fi
    done

    rm -rf "${modern_cpp_folder}/perftest_publisher.cxx"
    rm -rf "${modern_cpp_folder}/perftest_subscriber.cxx"
    rm -rf "${classic_cpp_folder}/perftest_publisher.cxx"
    rm -rf "${classic_cpp_folder}/perftest_subscriber.cxx"

    # Delete now files copied from srcCpp/connextDDS/
    for file in ${classic_cpp_folder}/connextDDS/*
    do
        if [ -f $file ]; then
            name_file=$(basename $file)
            rm -rf "${classic_cpp_folder}/${name_file}"
        fi
    done

    src_specific_folder="pro"
    if [ ${BUILD_MICRO} != 0 ]; then
        src_specific_folder="micro"
    fi

    for file in ${classic_cpp_folder}/connextDDS/${src_specific_folder}/*
    do
        if [ -f $file ]; then
            name_file=$(basename $file)
            rm -rf "${classic_cpp_folder}/${name_file}"
        fi
    done
}

function check_flatData_zeroCopy_available()
{
    version=$(awk -F"version" '/version/ { split($2, a, " "); print a[1] }' <<< $(${rtiddsgen_executable} -version)) # e.g. 3.0.0
    # We just need the Major value of the version.
    major=`echo $version | awk -F. '{print $1}'`

    if [[ $major -ge $flatdata_ddsgen_version && "${FAST_QUEUE}" != "1" ]]; then
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
    echo -e "${INFO_TAG} Copying files from srcCppCommon."
    copy_src_cpp_common

    echo -e "${INFO_TAG} Copying files from srcCpp/connextDDS."
    copy_src_cpp_connextDDS

    ##############################################################################
    # Generate files for the custom type files
    additional_defines_custom_type=""
    additional_header_files_custom_type=""
    additional_source_files_custom_type=""

    if [ "${USE_CUSTOM_TYPE}" == "1" ]; then
        build_cpp_custom_type
    fi

    check_flatData_zeroCopy_available
    additional_defines_calculation "CppTraditional"

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
        Infrastructure_pro.h \
        PerftestPrinter.h \
        FileDataLoader.h"

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
        Infrastructure_pro.cxx \
        PerftestPrinter.cxx \
        FileDataLoader.cxx"

    if [ "${FAST_QUEUE}" == "1" ]; then
        additional_header_files="${additional_header_files} \
        FastMemory.h"
        additional_source_files="${additional_source_files} \
        FastMemory.cxx"
        additional_defines=${additional_defines}" DPERFTEST_FAST_QUEUE"
    fi

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
        ${additional_rtiddsgen_defines} \
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
    if [[ "${SKIP_GENERATE}" == "" ]]; then
        eval $rtiddsgen_command
        if [ "$?" != 0 ]; then
            echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string}."
            clean_copied_files
            exit -1
        fi
    fi

    # Generate ZeroCopy types avoiding performance degradation issue
    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        echo -e "${INFO_TAG} Generating Zero Copy code"
        rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${classic_cpp_lang_string} \
        ${additional_rtiddsgen_defines} \
        -replace -create typefiles \
        -platform ${platform} \
        ${rtiddsgen_extra_options} ${additional_defines_custom_type} \
        -d \"${classic_cpp_folder}\" \"${idl_location}/perftest_ZeroCopy.idl\""

        echo -e "${INFO_TAG} Command (Generating Zero Copy types): $rtiddsgen_command"
        if [[ "${SKIP_GENERATE}" == "" ]]; then
            eval $rtiddsgen_command
            if [ "$?" != 0 ]; then
                echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string}."
                clean_copied_files
                exit -1
            fi
        fi

        rm -rf ${classic_cpp_folder}/makefile_perftest_ZeroCopy_${platform}
    fi

    cp "${classic_cpp_folder}/perftest_cpp.cxx" \
    "${classic_cpp_folder}/perftest_publisher.cxx"
    cp "${classic_cpp_folder}/perftest_cpp.cxx" \
    "${classic_cpp_folder}/perftest_subscriber.cxx"

    if [ "${JUST_GENERATE}" == "1" ]; then
        echo -e "${INFO_TAG} Code generation done. Skipping build (JUST_GENERATE=1)."
        exit 0
    fi

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
        clean_copied_files
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
                clean_copied_files
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

    if [ "${USE_LW_SECURE_LIBS}" == "1" ]; then
        executable_suffix="_lws"
    fi

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
    "${destination_folder}/perftest_cpp${executable_suffix}${executable_extension}"

    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copying code for ${classic_cpp_lang_string}."
        clean_copied_files
        exit -1
    else
        echo -e "${INFO_TAG} Copy successful for ${classic_cpp_lang_string}."
        clean_copied_files
    fi

    if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            echo -e "${INFO_TAG} Code compiled dynamically." \
                "Add \"${NDDSHOME}/lib/${platform}\""
            if [ "${USE_SECURE_LIBS}" == "1" ]; then
                echo -e "        and <CRYPTO_HOME>/${RELEASE_DEBUG}/lib"
            fi
            echo -e "        to your LD_LIBRARY_PATH or DYLD_LIBRARY_PATH"
    fi

    # Removing README files if those are created by rtiddsgen
    rm -f "${classic_cpp_folder}/README_${platform}.txt"

}

function build_micro_cpp()
{
    copy_src_cpp_common
    copy_src_cpp_connextDDS
    additional_defines_calculation_micro

    ##############################################################################
    # Generate files for srcCpp
    if [ "${BUILD_MICRO_24x_COMPATIBILITY}" -eq "1" ]; then
        additional_defines=${additional_defines}" PERFTEST_RTI_MICRO_24x_COMPATIBILITY"
    else
        rtiddsgen_extra_options="${rtiddsgen_extra_options} -sequenceSize ${MICRO_UNBOUNDED_SEQUENCE_SIZE}"

        if [ "${USE_SECURE_LIBS}" != "1" ]; then
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalRtiLibraries \"nddsmetp\""
        else
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalRtiLibraries \"rti_me_netioshmem rti_me_netioshmem rti_me_seccore\""
            if [ "${USE_OPENSSL}" = "1" ]; then
                rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraries \"ssl crypto\""
            else
                rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraries \"wolfssl\""
            fi
            # This option would cause issues on certain platforms (like macOS)
            # export ADDITIONAL_LINKER_FLAGS="$ADDITIONAL_LINKER_FLAGS -static"
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraryPaths \"${RTI_CRYPTOHOME}/${RELEASE_DEBUG}/lib\""
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
        Infrastructure_micro.h \
        FileDataLoader.h \
        PerftestPrinter.h"

    additional_source_files=" \
        ThreadPriorities.cxx \
        Parameter.cxx \
        ParameterManager.cxx \
        RTIDDSImpl.cxx \
        CpuMonitor.cxx \
        PerftestTransport.cxx \
        PerftestSecurity.cxx \
        Infrastructure_common.cxx \
        Infrastructure_micro.cxx \
        FileDataLoader.cxx \
        PerftestPrinter.cxx"

    rtiddsgen_command="\"${rtiddsgen_executable}\" ${additional_rtiddsgen_defines} \
            -micro -language ${classic_cpp_lang_string} \
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
    cp "${classic_cpp_folder}/perftest_cpp.cxx" "${classic_cpp_folder}/perftest_publisher.cxx"
    cp "${classic_cpp_folder}/perftest_cpp.cxx" "${classic_cpp_folder}/perftest_subscriber.cxx"
    cp "${idl_location}/perftest.idl" "${classic_cpp_folder}/perftest.idl"
    touch "${classic_cpp_folder}/perftestApplication.cxx"
    touch "${classic_cpp_folder}/perftestApplication.h"

    ##############################################################################
    # Compile srcCpp code
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp"
    cd "${classic_cpp_folder}"

    if [ "${COMPILER_EXE}" != "" ]; then
        echo -e "${INFO_TAG} Compiler: ${COMPILER_EXE}."
        cmake_c_compiler_string="-DCMAKE_C_COMPILER=${COMPILER_EXE} -DCMAKE_CXX_COMPILER=${COMPILER_EXE}"
    fi
    cmake_generate_command="${CMAKE_EXE} -DCMAKE_BUILD_TYPE=${RELEASE_DEBUG} ${cmake_c_compiler_string} -G \"Unix Makefiles\" -B./perftest_build -H. -DRTIME_TARGET_NAME=${platform} -DPLATFORM_LIBS=\"${additional_included_libraries}\" ${ADDITIONAL_CMAKE_ARGS}"
    echo -e "${INFO_TAG} Cmake Generate Command: $cmake_generate_command"
    eval $cmake_generate_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating makefiles with cmake for ${classic_cpp_lang_string}."
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
        clean_copied_files
        exit -1
    else
        echo -e "${INFO_TAG} Copy successful for ${classic_cpp_lang_string}."
        clean_copied_files
    fi
}

function build_tss_cpp()
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

    if [ "${BUILD_MICRO}" == "1" ]; then
        TSS_IMPL="micro"
    else
        additional_defines_calculation "CppTraditional"
        TSS_IMPL="pro"
    fi

    additional_defines=${additional_defines}" DRTI_PERF_TSS"

    cp "${resource_folder}/tss/CMakeLists.txt" \
    "${classic_cpp_folder}/CMakeLists.txt"

    ##############################################################################
    # Compile srcCpp code
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp"
    cd "${classic_cpp_folder}"

    cmake_generate_command="RTITSSARCH=${platform} ${CMAKE_EXE} \
                            -DRTI_CONNEXT_TYPE=${TSS_IMPL} \
                            -DRTI_TSS_ENABLE_FACE_COMPLIANCE=${FACE_COMPLIANCE} \
                            -DCMAKE_BUILD_TYPE=${RELEASE_DEBUG} \
                            -G \"Unix Makefiles\" \
                            -B./perftest_build -H."

	echo -e "${INFO_TAG} Cmake Generate Command: $cmake_generate_command"
    eval $cmake_generate_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating makefiles with cmake for ${classic_cpp_lang_string}."
        cd ..
        exit -1
    fi

	cmake_build_command="${CMAKE_EXE} --build ./perftest_build --config ${RELEASE_DEBUG}"
    	echo -e "${INFO_TAG} Cmake Build Command: $cmake_build_command"
    eval $cmake_build_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${classic_cpp_lang_string}."
        cd ..
        exit -1
    fi

    echo -e "${INFO_TAG} Compilation successful"
    cd ..

    clean_copied_files
}

function build_cpp11()
{
    copy_src_cpp_common
    check_flatData_zeroCopy_available
    additional_defines_calculation "CppModern"

    additional_header_files=" \
        ThreadPriorities.h \
        Parameter.h \
        ParameterManager.h \
        MessagingIF.h \
        RTIDDSImpl.h \
        perftest_cpp.h \
        qos_string.h \
        CpuMonitor.h \
        PerftestTransport.h \
        PerftestPrinter.h"

    additional_source_files=" \
        ThreadPriorities.cxx \
        Parameter.cxx \
        ParameterManager.cxx \
        RTIDDSImpl.cxx \
        CpuMonitor.cxx \
        PerftestTransport.cxx \
        PerftestPrinter.cxx"

    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        additional_header_files="${additional_header_files} \
        perftest_ZeroCopy.hpp \
        perftest_ZeroCopyPlugin.hpp"

        additional_source_files="${additional_source_files} \
        perftest_ZeroCopy.cxx \
        perftest_ZeroCopyPlugin.cxx"
    fi


    ##############################################################################
    # Generate files for srcCpp11
    rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${modern_cpp_lang_string} \
    ${additional_rtiddsgen_defines} \
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
    export output_string=$(eval $rtiddsgen_command 2>&1)
    echo -e "${output_string}"
    echo ""
    if [[ $output_string == *"ERROR"* ]]; then
        if [[ $output_string == *"Options language C++11 and architecture"* ]]; then
            echo -e "${WARNING_TAG} Skipping ${modern_cpp_lang_string} since the architecture does not support it."
            clean_copied_files
            return
        fi
        echo -e "${ERROR_TAG} Failure generating code for ${modern_cpp_lang_string}."
        clean_copied_files
        exit -1
    fi

    # Generate Zero Copy types avoiding performance degradation issue
    if [ "${ZEROCOPY_AVAILABLE}" == "1" ]; then
        echo -e "${INFO_TAG} Generating Zero Copy code"
        rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${modern_cpp_lang_string} \
        ${additional_rtiddsgen_defines} \
        -replace -create typefiles -platform ${platform} \
        ${rtiddsgen_extra_options} \
        -d \"${modern_cpp_folder}\" \"${idl_location}/perftest_ZeroCopy.idl\""

        echo -e "${INFO_TAG} Command (Generating Zero Copy types): $rtiddsgen_command"
        eval $rtiddsgen_command
        if [ "$?" != 0 ]; then
            echo -e "${ERROR_TAG} Failure generating code for ${modern_cpp_lang_string}."
            clean_copied_files
            exit -1
        fi

        rm -rf ${modern_cpp_folder}/makefile_perftest_ZeroCopy_${platform}
    fi

    cp "${modern_cpp_folder}/perftest_cpp.cxx" \
    "${modern_cpp_folder}/perftest_publisher.cxx"
    cp "${modern_cpp_folder}/perftest_cpp.cxx" \
    "${modern_cpp_folder}/perftest_subscriber.cxx"

    ##############################################################################
    # Compile srcCpp11 code
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
    echo -e "${INFO_TAG} Compiling perftest_cpp11."
    "${MAKE_EXE}" -C "${modern_cpp_folder}" -f makefile_perftest_${platform}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${modern_cpp_folder}."
        clean_copied_files
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
                clean_copied_files
                exit -1
        fi
        echo -e "${INFO_TAG} Building apk successful"
    fi

    echo ""
    echo -e "${INFO_TAG} Copying executable into: \"bin/${platform}/${RELEASE_DEBUG}\" folder"

    # Create bin folder if not exists and copy executables, since this command
    # has to work for several different architectures, we will try to find the
    # executable name with different line endings.
    perftest_cpp11_name_beginning="${modern_cpp_folder}/objs/${platform}/perftest_publisher"
    executable_extension=""
    destination_folder="${bin_folder}/${platform}/${RELEASE_DEBUG}"
    mkdir -p "${bin_folder}/${platform}/${RELEASE_DEBUG}"

    # In Android the path of the built apk slightly differs from other built binaries.
    if [[ ${platform} == *"Android"* ]]; then
        perftest_cpp11_name_beginning="${modern_cpp_folder}/objs/${platform}/publisher/bin/perftest_publisher-debug"
    fi

    if [ -e "$perftest_cpp11_name_beginning" ]; then
        executable_extension=""
    elif [ -e "${perftest_cpp11_name_beginning}.so" ]; then
        executable_extension=".so"
    elif [ -e "${perftest_cpp11_name_beginning}.lo" ]; then
        executable_extension=".lo"
    elif [ -e "${perftest_cpp11_name_beginning}.vxe" ]; then
        executable_extension=".vxe"
    elif [ -e "${perftest_cpp11_name_beginning}.apk" ]; then
        executable_extension=".apk"
    fi
    cp -f "${perftest_cpp11_name_beginning}${executable_extension}" \
    "${destination_folder}/perftest_cpp11${executable_extension}"
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copying code for ${classic_cpp_lang_string}."
        clean_copied_files
        exit -1
    else
        echo -e "${INFO_TAG} Copy successful for ${classic_cpp_lang_string}."
        clean_copied_files
    fi

    if [ "${STATIC_DYNAMIC}" == "dynamic" ]; then
            echo -e "${INFO_TAG} Code compiled dynamically." \
                "Add \"${NDDSHOME}/lib/${platform}\""
            if [ "${USE_SECURE_LIBS}" == "1" ]; then
                echo -e "        and <CRYPTO_HOME>/${RELEASE_DEBUG}/lib"
            fi
            echo -e "        to your LD_LIBRARY_PATH or DYLD_LIBRARY_PATH"
    fi

    # Removing README files if those are created by rtiddsgen
    rm -rf "${modern_cpp_folder}/README_${platform}.txt"
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

    rtiddsgen_command="\"${rtiddsgen_executable}\" -D PERFTEST_RTI_PRO -language ${java_lang_string} -unboundedSupport -replace -package com.rti.perftest.gen -d \"${java_folder}\" \"${idl_location}/perftest.idl\""

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

function build_cs()
{
    echo ""
    if [ "${RELEASE_DEBUG}" == debug ]; then
        echo -e "${INFO_TAG} Compiling in debug mode."
    else
        echo -e "${INFO_TAG} Compiling in release mode."
    fi

    ##############################################################################
    # Generate files for srcJava
    mkdir -p "${cs_folder}/ConnextDDS/GeneratedCode"
    rtiddsgen_command="\"${rtiddsgen_executable}\" -D PERFTEST_RTI_PRO -replace -language ${cs_lang_string} -unboundedSupport -d \"${cs_folder}/ConnextDDS/GeneratedCode\" \"${idl_location}/perftest.idl\""

    echo ""
    echo -e "${INFO_TAG} Generating types for ${cs_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${cs_lang_string}."
        exit -1
    fi

    ##############################################################################
    # Generate Files to compile

    rtiddsgen_command="\"${rtiddsgen_executable}\" -D PERFTEST_RTI_PRO -replace -language ${cs_lang_string} -platform ${platform_cs} -update makefiles -unboundedSupport -d \"${cs_folder}\" \"${idl_location}/perftest.idl\""

    echo ""
    echo -e "${INFO_TAG} Generating .net solution for ${cs_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating .net solution for ${cs_lang_string}."
        exit -1
    fi
    rm ${cs_folder}/README_${platform}.txt

    export dotnet_command="dotnet build --configuration ${RELEASE_DEBUG} \"${cs_folder}\""

    echo ""
    echo -e "${INFO_TAG} Compiling for dotnet"
    echo -e "${INFO_TAG} Command: $dotnet_command"

    eval ${dotnet_command}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating .net solution for ${cs_lang_string}."
        exit -1
    fi

    echo ""
    echo -e "${INFO_TAG} Creating script"
    mkdir -p "${bin_folder}/${RELEASE_DEBUG}"
    echo "dotnet run --project ${cs_folder} --configuration ${RELEASE_DEBUG} -- \$@" > "${bin_folder}/${RELEASE_DEBUG}/perftest_cs"
    chmod +x "${bin_folder}/${RELEASE_DEBUG}/perftest_cs"

    echo ""
    echo -e "${INFO_TAG} You can run the dotnet project by executing the following command:"
    echo ""
    echo "\"dotnet run --project ${cs_folder} --configuration ${RELEASE_DEBUG} -- <arguments>\""
    echo ""
    echo -e "${INFO_TAG} Alternatively, the following script can be executed:"
    echo ""
    echo "\"${bin_folder}/${RELEASE_DEBUG}/perftest_cs\""
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
    rm -rf ${generate_doc_folder}/html
    mkdir -p ${generate_doc_folder}/html
    sphinx-build ${doc_folder} ${generate_doc_folder}/html
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating HTML documentation"
        echo -e "${ERROR_TAG} You will need to install:
            sudo pip install -U sphinx
            sudo pip install sphinx_rtd_theme"
        exit -1
    fi

    echo -e "${INFO_TAG} HTML Generation successful. You will find it under:
        ${generate_doc_folder}/html/index.html"


    # Generate PDF
    echo ""
    echo -e "${INFO_TAG} Generating PDF documentation"
    rm -rf ${generate_doc_folder}/pdf
    mkdir -p ${generate_doc_folder}/pdf
    sphinx-build -b pdf ${doc_folder} ${generate_doc_folder}/pdf
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating PDF documentation"
        exit -1
    fi

    echo -e "${INFO_TAG} PDF Generation successful. You will find it under:
        ${generate_doc_folder}/pdf"

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
        --tss)
            BUILD_TSS=1
            ;;
        --face-profile)
            FACE_COMPLIANCE=$2
            shift
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
        --skip-cpp11-build)
            BUILD_CPP11=0
            ;;
        --java-build)
            BUILD_JAVA=1
            if [ $BUILD_SPECIFIC_APIS -eq 0 ]; then
                BUILD_SPECIFIC_APIS=1
                BUILD_CPP=0
                BUILD_CPP11=0
            fi
            ;;
        --cpp-build)
            BUILD_CPP=1
            if [ $BUILD_SPECIFIC_APIS -eq 0 ]; then
                BUILD_SPECIFIC_APIS=1
                BUILD_JAVA=0
                BUILD_CPP11=0
            fi
            ;;
        --cpp11-build)
            BUILD_CPP11=1
            if [ $BUILD_SPECIFIC_APIS -eq 0 ]; then
                BUILD_SPECIFIC_APIS=1
                BUILD_JAVA=0
                BUILD_CPP=0
            fi
            ;;
        --cs-build)
            BUILD_CS=1
            platform_cs="net5"
            if [ $BUILD_SPECIFIC_APIS -eq 0 ]; then
                BUILD_SPECIFIC_APIS=1
                BUILD_JAVA=0
                BUILD_CPP=0
                BUILD_CPP11=0
            fi
            ;;
        --make)
            MAKE_EXE=$2
            shift
            ;;
        --cmake)
            CMAKE_EXE=$2
            shift
            ;;
        --add-cmake-args)
            ADDITIONAL_CMAKE_ARGS=$2
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
        --force-c++11-infrastructure)
            RTI_USE_CPP_11_INFRASTRUCTURE=1
            ;;
        --dynamic)
            STATIC_DYNAMIC=dynamic
            ;;
        --secure)
            USE_SECURE_LIBS=1
            ;;
        --lightWeightSecure)
            USE_SECURE_LIBS=1
            USE_LW_SECURE_LIBS=1
            ;;
        --security)
            USE_SECURE_LIBS=1
            ;;
        --lightWeightSecurity)
            USE_SECURE_LIBS=1
            USE_LW_SECURE_LIBS=1
            ;;
        --legacy-DynamicData)
            LEGACY_DD_IMPL=1
            ;;
        --monitoring2)
            RTI_MONITORING_2=1
            ;;
        --enableMonitoring2)
            RTI_MONITORING_2=1
            ;;
        --observability)
            RTI_MONITORING_2=1
            ;;
        --enableObservability)
            RTI_MONITORING_2=1
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
            if [ ! -z "${RTI_CRYPTOHOME}" ]; then
                echo -e "${ERROR_TAG} More than one --openssl-home or" \
                    "--wolfssl-home were passed as arguments. Choose whether" \
                    "to compile perftest with OpenSSL or with WolfSSL."
                usage
                exit -1
            fi
            RTI_CRYPTOHOME=$2
            USE_OPENSSL=1
            shift
            ;;
        --wolfssl-home)
            if [ ! -z "${RTI_CRYPTOHOME}" ]; then
                echo -e "${ERROR_TAG} More than one --openssl-home or" \
                    "--wolfssl-home were passed as arguments. Choose whether" \
                    "to compile perftest with OpenSSL or with WolfSSL."
                usage
                exit -1
            fi
            RTI_CRYPTOHOME=$2
            USE_OPENSSL=0
            shift
            ;;
        --openssl-version)
            if [ ! -z "${RTI_CRYPTOHOME}" ]; then
                echo -e "${WARNING_TAG} --openssl-version will be ignored, as either " \
                        "--openssl-home or --wolfssl-home were provided already."
            else
                SSL_VERSION=$2
                USE_OPENSSL=1
            fi
            shift
            ;;
        --wolfssl-version)
            if [ ! -z "${RTI_CRYPTOHOME}" ]; then
                echo -e "${WARNING_TAG} --wolfssl-version will be ignored, as either " \
                        "--openssl-home or --wolfssl-home were provided already."
            else
                SSL_VERSION=$2
                USE_OPENSSL=0
            fi
            shift
            ;;
        --ssl-version)
            if [ ! -z "${RTI_CRYPTOHOME}" ]; then
                echo -e "${WARNING_TAG} --ssl-version will be ignored, as either " \
                        "--openssl-home or --wolfssl-home were provided already."
            else
                SSL_VERSION=$2
                if [[ "${SSL_VERSION}" == "wolf"* ]]; then
                    USE_OPENSSL=0
                fi
            fi
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
        --fastQueue)
            FAST_QUEUE=1
            ;;
        --no-zeroCopy)
            SKIP_ZEROCOPY="1"
            ;;
        # These options are not exposed to the user (yet)
        --just-generate)
            JUST_GENERATE=1
            ;;
        --skip-generate | --just-build)
            SKIP_GENERATE=1
            ;;
        *)
            echo -e "${ERROR_TAG} unknown parameter \"$1\""
            usage
            exit -1
            ;;
    esac
    shift
done

if [ "${BUILD_TSS}" -eq "1" ] && [ "${BUILD_MICRO}" -eq "0" ]; then
    BUILD_TSS_PRO=1
fi

executable_checking

if [ "${BUILD_TSS}" -eq "1" ]; then
    rtiddsgen_executable="$RTITSSHOME/bin/rtiddsgen"
    classic_cpp_lang_string=FACEC++

    if [ "${BUILD_CPP}" -eq "1" ]; then
        library_sufix_calculation
        build_tss_cpp
    fi

elif [ "${BUILD_MICRO}" -eq "1" ]; then

    rtiddsgen_executable="$RTIMEHOME/rtiddsgen/scripts/rtiddsgen"

    classic_cpp_lang_string=C++
    if [ "${BUILD_CPP}" -eq "1" ]; then
        library_sufix_calculation
        build_micro_cpp
    fi

else # Build for ConnextDDS Pro
    rtiddsgen_executable="$NDDSHOME/bin/rtiddsgen"

    classic_cpp_lang_string=C++
    modern_cpp_lang_string=C++11
    java_lang_string=java
    cs_lang_string=C#

    # Generate qos_string.h
    generate_qos_string

    if [ "${BUILD_CPP}" -eq "1" ]; then
        library_sufix_calculation
        build_cpp
    fi

    if [ "${BUILD_CPP11}" -eq "1" ]; then
        library_sufix_calculation
        build_cpp11
    fi

    if [ "${BUILD_JAVA}" -eq "1" ]; then
        build_java
    fi

    if [ "${BUILD_CS}" -eq "1" ]; then
        build_cs
    fi

fi

echo ""
echo "================================================================================"
echo ""
