#!/bin/bash

################################################################################
filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`

# We export this path instead to do a cd command to the location of the XML
#   configuration file in case the service needs to be run on a different
#   location. If this happens the service would read the enviroment variable in
#   any case but not the file if the location has change.
#   For example this happens with rti recorder service.
export NDDS_QOS_PROFILES=$script_location/NDDS_QOS_PROFILES.xml
ERROR_TAG="${RED}[ERROR]:${NC}"

################################################################################
############################### PARSE PARAMETERS ###############################
################################################################################
while :
do
    case "$1" in
      -domain)
          read_domain=$2
          shift 2
          ;;
      -sendQueueSize)
          read_sendQueueSize=$2
          shift 2
          ;;
      -bestEffort)
          read_bestEffort=1
          shift
          ;;
      -keyed)
          read_keyed=1
          shift
          ;;
      -unbounded)
          read_unbounded=1
          shift
          ;;
      -asynchronous)
          read_asynchronous=1
          shift
          ;;
      -batchSize)
          read_batchSize=$2
          shift 2
          ;;
      -executionTime)
          read_exec_time=$2
          shift 2
          ;;
      -verbosity)
          read_verbosity=$2
          shift 2
          ;;
      -nddshome)
          read_NDDSHOME=$2
          shift 2
          ;;
      -db-destination)
          read_destination=$2
          shift 2
          ;;
      -routing_service)
          export SERVICE_NAME="routing"
          shift
          ;;
      -recording_service)
          export SERVICE_NAME="recording"
          shift
          ;;
      -*)
          echo "Error: Unknown option: $1" >&2
          exit 1
          ;;
      *)  # No more options
          break
          ;;
    esac
done

################################################################################
############################## CHECK THE SERVICE ###############################
################################################################################

# This common file is supposed to be called via recordingservice_wrapper.sh or
#   routingservice_wrapper.sh where the SERVICE_NAME would be set, so this
#   error should not be reachable.
if [ "$SERVICE_NAME" = "" ]; then
    echo -e "${ERROR_TAG} Any service was selected, use -recording_service or -routing_service command line options"
    exit -1
fi

################################################################################
############################## SETTING PARAMETERS ##############################
################################################################################

export TRANSPORT_BUILTIN_MASK=UDPv4

if [ "$read_domain" = "" ]; then
    export DOMAIN_BASE=0
else
    export DOMAIN_BASE=$read_domain
fi

if [ "$read_keyed" = "1" ]; then
    if [ "$read_unbounded" = "1" ]; then
        export TYPE_NAME=TestDataKeyedLarge_t
    else
        export TYPE_NAME=TestDataKeyed_t
    fi
    export BASE_PROFILE_QOS_MAX_INSTANCES=100000
    export BASE_PROFILE_QOS_INITIAL_INSTANCES=100000
    export BASE_PROFILE_QOS_INSTANCE_HAS_BUCKETS=100000
else
    if [ "$read_unbounded" = "1" ]; then
        export TYPE_NAME=TestDataLarge_t
    else
        export TYPE_NAME=TestData_t
    fi
    export BASE_PROFILE_QOS_MAX_INSTANCES=1
    export BASE_PROFILE_QOS_INITIAL_INSTANCES=1
    export BASE_PROFILE_QOS_INSTANCE_HAS_BUCKETS=1
fi

if [ "$read_verbosity" = "" ]; then
    export VERBOSITY=1
else
    export VERBOSITY=$read_verbosity
fi

if [ "$read_bestEffort" = "1" ]; then
    export RELIABILITY=BEST_EFFORT_RELIABILITY_QOS
else
    export RELIABILITY=RELIABLE_RELIABILITY_QOS
fi

if [ "$read_batchSize" = "" ]; then
    export BATCHING=0
    export BATCH_SIZE=8192
else
    export BATCHING=true
    export BATCH_SIZE=$read_batchSize
fi

if [ "$read_sendQueueSize" = "" ]; then
    export SEND_QUEUE_SIZE=50
else
    export SEND_QUEUE_SIZE=$read_sendQueueSize
fi

if [ "$BATCHING" = "0" ]; then
    export THROUGHPUT_QOS_INITIAL_SAMPLES=$SEND_QUEUE_SIZE
    export THROUGHPUT_QOS_MAX_SAMPLES=$SEND_QUEUE_SIZE
    export THROUGHPUT_QOS_MAX_SAMPLES_PER_INSTANCE=$SEND_QUEUE_SIZE
else
    export THROUGHPUT_QOS_INITIAL_SAMPLES=$SEND_QUEUE_SIZE
    export THROUGHPUT_QOS_MAX_SAMPLES=LENGTH_UNLIMITED
    export THROUGHPUT_QOS_MAX_SAMPLES_PER_INSTANCE=LENGTH_UNLIMITED
fi

################################################################################
############################ GENERAL CONFIGURATION #############################
################################################################################

export THROUGHPUT_QOS_HIGH_WATERMARK=`LC_NUMERIC="en_US.UTF-8" printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.9 | bc)`
export THROUGHPUT_QOS_LOW_WATERMARK=`LC_NUMERIC="en_US.UTF-8" printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.1 | bc)`
export THROUGHPUT_QOS_HB_PER_MAX_SAMPLES=`LC_NUMERIC="en_US.UTF-8" printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.1 | bc)`


CFG_FILE=$script_location/../$SERVICE_NAME``_service/$SERVICE_NAME``_cfg.xml

if [ "$read_NDDSHOME" != "" ]; then
    NDDSHOME = $read_NDDSHOME
fi

if [ "$NDDSHOME" = "" ]; then
    echo -e "${ERROR_TAG} The NDDSHOME variable is not set"
    exit -1
fi

export EXECUTABLE_NAME=$NDDSHOME/bin/rti"$SERVICE_NAME"service

# I have maintained this variable in case we want to added as a parameter on
#   the future. For example:
#   export TEST_THREAD_POOL_SIZE=$number_of_threads
export TEST_THREAD_POOL_SIZE=1

# The name of the recording service script has change on Galleon from
# rtirecord to rtirecordingservice, so if rtirecordingservice script does
# not exist, lets try with rtirecord.
if [ "$SERVICE_NAME" = "recording" ]; then
    if [ ! -e $EXECUTABLE_NAME ]; then
        echo [WARNING]: $EXECUTABLE_NAME has not been found.
        echo "Assuming a pre-6.0.0 RTI Connext DDS version!"
        echo "Recorder 5.3.1 does not have multi-threading option. A single thread will be used."
        export EXECUTABLE_NAME=$NDDSHOME/bin/rtirecord
        CFG_FILE=$script_location/../$SERVICE_NAME``_service/$SERVICE_NAME``_531_cfg.xml
        TEST_THREAD_POOL_SIZE = 1;
    fi
fi

if [ "$read_asynchronous" = "1" ]; then
    export PUBLISH_MODE=DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS
else
    export PUBLISH_MODE=SYNCHRONOUS_PUBLISH_MODE_QOS
fi
export command_line_parameter="$EXECUTABLE_NAME -cfgFile $CFG_FILE -cfgName "$SERVICE_NAME"_service_test -verbosity $VERBOSITY"

if [ "$SERVICE_NAME" = "routing" ]; then
    export command_line_parameter="$command_line_parameter -domainIdBase $DOMAIN_BASE"
fi


################################################################################
############################ SPECIFIC CONFIGURATION ############################
################################################################################

# In order to reach this point, SERVICE_NAME shall be set so there is no need
#   to check the variable.
# If other service is added, you can include here the non-common parameters.
if [ "$SERVICE_NAME" = "recording" ]; then
    if [ "$read_exec_time" != "" ]; then
        export command_line_parameter="timeout --preserve-status --signal=SIGINT $read_exec_time $command_line_parameter"
    fi

    if [ "$read_destination" != "" ]; then
        export destination=$read_destination
    else
        export destination=/tmp
    fi

elif [ "$SERVICE_NAME" = "routing" ]; then
    if [ "$read_exec_time" != "" ]; then
        export command_line_parameter="$command_line_parameter -stopAfter $read_exec_time"
    fi
fi

################################################################################
#################################### OUTPUT ####################################
################################################################################
echo " "
echo "=========================================================================="
echo "         Domain: $DOMAIN_BASE"
if [ "$destination" != "" ]; then
    echo "       Location: $destination"
fi
echo "           Type: $TYPE_NAME"
echo "    Reliability: $RELIABILITY"
echo "       Batching: $BATCHING"
if [ "$BATCHING" != "0" ]; then
    echo "     Batch size: $BATCH_SIZE"
fi
if [ "$read_asynchronous" = "1" ]; then
    echo "   Asynchronous: $read_asynchronous"
fi
if [ "$read_unbounded" = "1" ]; then
    echo "      Unbounded: $read_unbounded"
fi
echo " sendQueue Size: $SEND_QUEUE_SIZE"
if [ "$read_exec_time" != "" ]; then
    echo " Execution Time: $read_exec_time"
fi
echo "       NDDSHOME: $read_NDDSHOME"
echo "=========================================================================="
echo " "
echo "[Running] $command_line_parameter"
echo " "

################################################################################
################################## EXECUTION ###################################
################################################################################

if [ "$destination" != "" ]; then
    # This is only are going to happens with Recoring Service
    cd $destination
    /bin/rm -rf rti_recorder_default*.dat
fi

#Execute the Service.
$command_line_parameter
