#!/bin/bash

################################################################################
filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`
ERROR_TAG="${RED}[ERROR]:${NC}"
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

export THROUGHPUT_QOS_HIGH_WATERMARK=`printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.9 | bc)`
export THROUGHPUT_QOS_LOW_WATERMARK=`printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.1 | bc)`
export THROUGHPUT_QOS_HB_PER_MAX_SAMPLES=`printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.1 | bc)`


if [ "$read_NDDSHOME" = "" ]; then
    if [ "$NDDSHOME" = "" ]; then
        echo -e "${ERROR_TAG} The NDDSHOME variable is not set"
        exit -1
    else
        export EXECUTABLE_NAME=$NDDSHOME/bin/rtirecordingservice
    fi
else
    export EXECUTABLE_NAME=$read_NDDSHOME/bin/rtirecordingservice
fi
if [ "$read_asynchronous" = "1" ]; then
    export PUBLISH_MODE=DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS
else
    export PUBLISH_MODE=SYNCHRONOUS_PUBLISH_MODE_QOS
fi
export command_line_parameter="$EXECUTABLE_NAME -cfgFile $script_location/recordingservice_cfg.xml -cfgName RecordingServiceTest -domainIdBase $DOMAIN_BASE -verbosity $VERBOSITY"

if [ "$read_exec_time" != "" ]; then
    export command_line_parameter="timeout --preserve-status --signal=SIGINT $read_exec_time $command_line_parameter"
fi

export TEST_THREAD_POOL_SIZE=1

if [ "$read_destination" != "" ]; then
    export destination=$read_destination
else
    export destination=/tmp
fi

################################################################################
echo " "
echo "=========================================================================="
echo "         Domain: $DOMAIN_BASE"
echo "       Location: $destination"
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


cd $destination
/bin/rm -rf rti_recorder_default*.dat
$command_line_parameter
