#!/bin/bash

################################################################################
# Variable declaration

filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`
idl_location="${script_location}/srcIdl"
classic_cpp_folder="${script_location}/srcCpp"
modern_cpp_folder="${script_location}/srcCpp03"
java_folder="${script_location}/srcJava"
java_scripts_folder="${script_location}/resource/java_scripts"
bin_folder="${script_location}/bin"

# Default values:
BUILD_CPP=1
BUILD_CPP03=1
BUILD_JAVA=1
MAKE_EXE=make
JAVAC_EXE=javac
JAVA_EXE=java
JAR_EXE=jar

# Default values for libraries
RELEASE_DEBUG=release
STATIC_DYNAMIC=static
USE_SECURE_LIBS=0

# Needed when compiling statically using security
RTI_OPENSSLHOME=""

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
    echo "    --platform <your_arch>       Platform for which build.sh is going to compile"
    echo "                                 RTI Perftest.                                  "
    echo "    --nddshome <path>            Path to the *RTI Connext DDS installation*. If "
    echo "                                 this parameter is not present, the \$NDDSHOME  "
    echo "                                 variable should be set.                        "
    echo "    --skip-java-build            Avoid Java ByteCode generation creation.       "
    echo "    --skip-cpp-build             Avoid C++ code generation and compilation.     "
    echo "    --skip-cpp03-build           Avoid C++ New PSM code generation and          "
    echo "                                 compilation.                                   "
    echo "    --make <path>                Path to the GNU make executable. If this       "
    echo "                                 parameter is not present, GNU make variable    "
    echo "                                 should be available from your \$PATH variable. "
    echo "    --java-home <path>           Path to the Java JDK home folder. If this      "
    echo "                                 parameter is not present, javac, jar and java  "
    echo "                                 executables should be available from your      "
    echo "                                 \$PATH variable.                               "
    echo "    --clean                      If this option is present, the build.sh script "
    echo "                                 will clean all the generated code and binaries "
    echo "    --debug                      Compile against the RTI Connext Debug          "
    echo "                                 libraries. Default is against release ones.    "
    echo "    --dynamic                    Compile against the RTI Connext Dynamic        "
    echo "                                 libraries. Default is against static ones.     "
    echo "    --secure                     Enable the security options for compilation.   "
    echo "                                 Default is not enabled.                        "
    echo "    --openssl-home <path>        Path to the openssl home. This will be used    "
    echo "                                 when compiling statically and using security   "
    echo "                                 Default is an empty string (current folder).   "
    echo "    --help -h                    Display this message.                          "
    echo "                                                                                "
    echo "================================================================================"
    echo ""
}

function clean()
{
    echo ""
    echo -e "${INFO_TAG} Cleaning generated files."

    rm -f  "${script_location}"/srcC*/perftest.*
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

    echo ""
    echo "================================================================================"
    echo ""

}

function executable_checking()
{
    # Is NDDSHOME set?
    if [ -z "${NDDSHOME}" ]; then
        echo -e "${ERROR_TAG} The NDDSHOME variable is not set"
        usage
        exit -1
    fi

    # Is platform specified?
    if [ -z "${platform}" ]; then
        echo -e "${ERROR_TAG} The platform argument is missing"
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
    additional_defines="O3"
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
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalRtiLibraries nddssecurity -additionalLibraries \"sslz cryptoz\""
            rtiddsgen_extra_options="${rtiddsgen_extra_options} -additionalLibraryPaths \"${RTI_OPENSSLHOME}/${RELEASE_DEBUG}/lib\""
            echo -e "${INFO_TAG} Using security plugin. Linking Statically."
        fi
    fi
}

function build_cpp()
{
    additional_defines_calculation

    ##############################################################################
    # Generate files for srcCpp

    rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${classic_cpp_lang_string} -unboundedSupport -replace -create typefiles -create makefiles -platform ${platform} -additionalHeaderFiles \"MessagingIF.h RTIDDSImpl.h perftest_cpp.h qos_string.h CpuMonitor.h PerftestTransport.h\" -additionalSourceFiles  \"RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx\" -additionalDefines \"${additional_defines}\" ${rtiddsgen_extra_options} -d \"${classic_cpp_folder}\" \"${idl_location}/perftest.idl\" "

    echo ""
    echo -e "${INFO_TAG} Generating types and makefiles for ${classic_cpp_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${classic_cpp_lang_string}."
        exit -1
    fi
    cp "${classic_cpp_folder}/perftest_publisher.cxx" \
    "${classic_cpp_folder}/perftest_subscriber.cxx"

    ##############################################################################
    # Compile srcCpp code
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp"
    "${MAKE_EXE}" -C "${classic_cpp_folder}" -f makefile_perftest_${platform}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${classic_cpp_lang_string}."
        exit -1
    fi
    echo -e "${INFO_TAG} Compilation successful"

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
    elif [ -e "${perftest_cpp_name_beginning}.vxe" ]; then
        executable_extension=".vxe"
    fi
    cp -f "${perftest_cpp_name_beginning}${executable_extension}" \
    "${destination_folder}/perftest_cpp${executable_extension}"
}

function build_cpp03()
{
    additional_defines_calculation
    ##############################################################################
    # Generate files for srcCpp03

    rtiddsgen_command="\"${rtiddsgen_executable}\" -language ${modern_cpp_lang_string} -unboundedSupport -replace -create typefiles -create makefiles -platform ${platform} -additionalHeaderFiles \"MessagingIF.h RTIDDSImpl.h perftest_cpp.h qos_string.h CpuMonitor.h PerftestTransport.h\" -additionalSourceFiles \"RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx\" -additionalDefines \"${additional_defines}\" ${rtiddsgen_extra_options} -d \"${modern_cpp_folder}\" \"${idl_location}/perftest.idl\""

    echo ""
    echo -e "${INFO_TAG} Generating types and makefiles for ${modern_cpp_lang_string}."
    echo -e "${INFO_TAG} Command: $rtiddsgen_command"

    # Executing RTIDDSGEN command here.
    eval $rtiddsgen_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure generating code for ${modern_cpp_lang_string}."
        exit -1
    fi
    cp "${modern_cpp_folder}/perftest_publisher.cxx" \
    "${modern_cpp_folder}/perftest_subscriber.cxx"

    ##############################################################################
    # Compile srcCpp03 code
    echo ""
    echo -e "${INFO_TAG} Compiling perftest_cpp03."
    "${MAKE_EXE}" -C "${modern_cpp_folder}" -f makefile_perftest_${platform}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compiling code for ${modern_cpp_folder}."
        exit -1
    fi
    echo -e "${INFO_TAG} Compilation successful"

    echo ""
    echo -e "${INFO_TAG} Copying executable into: \"bin/${platform}/${RELEASE_DEBUG}\" folder"

    # Create bin folder if not exists and copy executables, since this command
    # has to work for several different architectures, we will try to find the
    # executable name with different line endings.
    perftest_cpp03_name_beginning="${modern_cpp_folder}/objs/${platform}/perftest_publisher"
    executable_extension=""
    destination_folder="${bin_folder}/${platform}/${RELEASE_DEBUG}"
    mkdir -p "${bin_folder}/${platform}/${RELEASE_DEBUG}"

    if [ -e "$perftest_cpp03_name_beginning" ]; then
        executable_extension=""
    elif [ -e "${perftest_cpp03_name_beginning}.lo" ]; then
        executable_extension=".lo"
    elif [ -e "${perftest_cpp03_name_beginning}.vxe" ]; then
        executable_extension=".vxe"
    fi
    cp -f "${perftest_cpp03_name_beginning}${executable_extension}" \
    "${destination_folder}/perftest_cpp03${executable_extension}"
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
        --skip-java-build)
            BUILD_JAVA=0
            ;;
        --skip-cpp-build)
            BUILD_CPP=0
            ;;
        --skip-cpp03-build)
            BUILD_CPP03=0
            ;;
        --make)
            MAKE_EXE=$2
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
        --debug)
            RELEASE_DEBUG=debug
            ;;
        --dynamic)
            STATIC_DYNAMIC=dynamic
            ;;
        --secure)
            USE_SECURE_LIBS=1
            ;;
        --openssl-home)
            RTI_OPENSSLHOME=$2
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

rtiddsgen_executable="$NDDSHOME/bin/rtiddsgen"

classic_cpp_lang_string=C++
modern_cpp_lang_string=C++03
java_lang_string=java

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

echo ""
echo "================================================================================"
echo ""
