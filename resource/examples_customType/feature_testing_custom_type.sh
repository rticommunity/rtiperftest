#!/bin/bash

################################################################################
# Variable declaration
script_location=`cd "\`dirname "$0"\`"; pwd`
rtiperftest_folder="${script_location}/../../"
tmp_folder="${rtiperftest_folder}/tmp"
tmp_sub_file="${tmp_folder}/tmp_sub.txt"
tmp_pub_file="${tmp_folder}/tmp_pub.txt"
tmp_build_file="${tmp_folder}/tmp_build.txt"
custom_type_location="${rtiperftest_folder}/srcIdl/customType"
executable=""
timeout="20"
custom_type="Test" # TODO add from the command-line parameter
pub_pid=""
sub_pid=""
test=(key_large key_no_large no_key_large no_key_no_large RTIPerftest_type key_and_sequence)
pub_command=""
sub_command=""
domain="1"

# We will use some colors to improve visibility of errors and information
RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'
INFO_TAG="${GREEN}[INFO]:${NC}"
ERROR_TAG="${RED}[ERROR]:${NC}"

################################################################################
function usage()
{
    echo ""
    echo "================================================================================"
    echo "This scripts accepts the following parameters:                                  "
    echo "                                                                                "
    echo "    --platform <your_arch>       Platform for which                             "
    echo "                                 install_testing_custom_type.sh is going to     "
    echo "                                 compileRTI Perftest.                           "
    echo "    --domain <domain_id>         Domain used for the test.                      "
    echo "                                 Default: 1                                     "
    echo "    --nddshome <path>            Path to the *RTI Connext DDS installation*. If "
    echo "                                 this parameter is not present, the \$NDDSHOME  "
    echo "                                 variable should be set.                        "
    echo "    --make <path>                Path to the GNU make executable. If this       "
    echo "                                 parameter is not present, the GNU make variable    "
    echo "                                 should be available from your \$PATH variable. "
    echo "    --help -h                    Display help message.                          "
    echo "================================================================================"
    echo ""
}

function clean()
{
    build_perfest_clean
    # Remove tmp files
    rm -rf "${tmp_folder}"/*
    rm -rf "${custom_type_location}"/*
    # Remove execution files
}

function verify_precondition_variables()
{
    # Is NDDSHOME set?
    if [ -z "${NDDSHOME}" ]; then
        echo -e "${ERROR_TAG} NDDSHOME variable is not set"
        usage
        exit -1
    fi

    # Is platform specified?
    if [ -z "${platform}" ]; then
        echo -e "${ERROR_TAG} The platform argument is missing"
        usage
        exit -1
    fi
}

function build_perfest_clean()
{
    cleaning_command="${rtiperftest_folder}build.sh --clean >&- 2>&-"
    # Executing compailing command
    eval $cleaning_command
}

function build_perftest()
{
    compailing_command="${rtiperftest_folder}build.sh --platform ${platform} --skip-java-build --skip-cpp03-build --customType ${custom_type} &> ${tmp_build_file}"
    echo ""
    echo -e "${INFO_TAG} Compailing for test ${1}"
    echo -e "${INFO_TAG} Command: $compailing_command"

    # Executing compailing command
    eval $compailing_command
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure compailing for ${1}, you can find the log in '${tmp_build_file}'"
        exit -1
    fi
}

function kill_pub_sub_jobs()
{
    kill_command="kill -9 ${pub_pid} >&- 2>&-"
    eval $kill_command
    kill_command="kill -9 ${sub_pid} >&- 2>&-"
    eval $kill_command
}

# Verify if the publisher and the subscriber communicate properly.
function verify_execution()
{
    execution_success_pub=false
    execution_success_sub=false
    j="0"
    while [ ${j} -lt 4 ] && [ "$execution_success_pub" = false -o "$execution_success_sub" = false ]
    do
        sleep $timeout
        if grep -Fq  "Test ended." ${tmp_sub_file}
        then # found
            execution_success_sub=true
        fi
        if grep -Fq  "Test ended." ${tmp_pub_file}
        then # found
            execution_success_pub=true
        fi
        j=$[${j}+1]
    done

    if [ "$execution_success_pub" = true ] && [ "$execution_success_sub" = true ]; then
        echo -e "${INFO_TAG} The install testing for ${1} does work."
        kill_pub_sub_jobs
    else
        echo -e "${ERROR_TAG} The install testing for ${1} does not work."
        kill_pub_sub_jobs
        exit -1
    fi
}

function set_command_publisher()
{
    pub_command="${executable} -pub -domain ${domain} -noprint -executionTime ${timeout}"
    if [[ ${2} = "dynamic" ]]; then
        pub_command=${pub_command}" -dynamicData"
    fi
    if ! [[ ${1} = *"no_key"* ]]; then
        pub_command=${pub_command}" -keyed"
    fi
    if ! [[ ${1} = *"no_large"* ]]; then
        pub_command=${pub_command}" -asynchronous"
    fi
    pub_command=${pub_command}" &> ${tmp_pub_file} &"
}

function set_command_subscriber()
{
    sub_command="${executable} -sub -domain ${domain} -noprint"
    if [[ ${2} = "dynamic" ]]; then
        sub_command=${sub_command}" -dynamicData"
    fi
    if ! [[ ${1} = *"no_key"* ]]; then
        sub_command=${sub_command}" -keyed"
    fi
    if ! [[ ${1} = *"no_large"* ]]; then
        sub_command=${sub_command}" -asynchronous"
    fi
    sub_command=${sub_command}" &> ${tmp_sub_file} &"
}

function execute_RTI_Perftest()
{
    echo ""
    echo -e "${INFO_TAG} RTI Perftest command for ${1}"
    echo -e "${INFO_TAG} Pub command: ${pub_command}"
    echo -e "${INFO_TAG} Sub command: ${sub_command}"

    eval ${pub_command}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure running pub for ${1}"
        exit -1
    fi
    pub_pid=$!

    eval ${sub_command}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure running sub for ${1}"
        exit -1
    fi
    sub_pid=$!

    verify_execution ${1}
}

function copy_scenario()
{
    copy_comand="cp -rf ${script_location}/${1}/* ${rtiperftest_folder}/"
    echo -e "${INFO_TAG} copy_comand: ${copy_comand}"
    eval ${copy_comand}
    if [ "$?" != 0 ]; then
        echo -e "${ERROR_TAG} Failure copy scenario for ${1}"
        exit -1
    fi
}

################################################################################
echo ""
echo "=============== Feature testing customType RTI PERFTEST: ================"

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
        --make)
            MAKE_EXE=$2
            shift
            ;;
        --platform)
            platform=$2
            executable="${rtiperftest_folder}/bin/${platform}/release/perftest_cpp"
            shift
            ;;
        --domain)
            domain=$2
            shift
            ;;
        --nddshome)
            export NDDSHOME=$2
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

cd ${rtiperftest_folder}
mkdir -p ${tmp_folder}
verify_precondition_variables
clean
for i in ${test[@]};do
    echo -e "${INFO_TAG} Running test ${i}"
    copy_scenario ${i}
    build_perftest ${i}
    set_command_publisher ${i}
    set_command_subscriber ${i}
    execute_RTI_Perftest ${i}
    set_command_publisher ${i} "dynamic"
    set_command_subscriber ${i} "dynamic"
    execute_RTI_Perftest ${i}
    clean
    echo "======================================"
done

rm -rf ${tmp_folder}

echo ""
echo -e "${INFO_TAG} The full feature testing was success."
echo "================================================================================"
echo ""
