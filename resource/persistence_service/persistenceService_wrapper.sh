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
          export NDDSHOME=$2
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

export DOMAIN_BASE=1
if [ "$read_domain" != "" ]; then
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

export THROUGHPUT_QOS_HIGH_WATERMARK=`LC_NUMERIC="en_US.UTF-8" printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.9 | bc)`
export THROUGHPUT_QOS_LOW_WATERMARK=`LC_NUMERIC="en_US.UTF-8" printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.1 | bc)`
export THROUGHPUT_QOS_HB_PER_MAX_SAMPLES=`LC_NUMERIC="en_US.UTF-8" printf "%.0f" $(expr $SEND_QUEUE_SIZE*0.1 | bc)`


if [ "$NDDSHOME" = "" ]; then
    echo -e "${ERROR_TAG} The NDDSHOME variable is not set"
    exit -1
elif [ ! -d $NDDSHOME ]; then 
    echo -e "${ERROR_TAG} The NDDSHOME path does not exist: $NDDSHOME"
    exit -1
fi

export EXECUTABLE_NAME=$NDDSHOME/bin/rtipersistenceservice

if [ "$read_asynchronous" = "1" ]; then
    export PUBLISH_MODE=DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS
else
    export PUBLISH_MODE=SYNCHRONOUS_PUBLISH_MODE_QOS
fi

export command_line_parameter="$EXECUTABLE_NAME -cfgFile $script_location/persistenceService_cfg.xml -cfgName PS_PerfTest -domainId $DOMAIN_BASE -verbosity $VERBOSITY"

################################################################################

echo -e "\n"
echo -e "=========================================================================="
echo -e "         Domain: $DOMAIN_BASE"
echo -e "           Type: $TYPE_NAME"
echo -e "    Reliability: $RELIABILITY"
echo -e "       Batching: $BATCHING"
if [ "$BATCHING" != "0" ]; then
    echo -e "     Batch size: $BATCH_SIZE"
fi
if [ "$read_asynchronous" = "1" ]; then
    echo -e "   Asynchronous: $read_asynchronous"
fi
if [ "$read_unbounded" = "1" ]; then
    echo -e "      Unbounded: $read_unbounded"
fi
echo -e " sendQueue Size: $SEND_QUEUE_SIZE"
if [ "$read_exec_time" != "" ]; then
    echo -e " Execution Time: $read_exec_time"
fi
echo -e "       NDDSHOME: $read_NDDSHOME"
echo -e "==========================================================================\n"
echo -e "[Running] $command_line_parameter\n"

################################################################################
$command_line_parameter
