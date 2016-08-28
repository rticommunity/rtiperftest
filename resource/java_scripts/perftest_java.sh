#!/bin/bash

filename=$0
script_location=`cd "\`dirname "$filename"\`"; pwd`

# You can override the following settings with the correct location of Java
JAVA=`which java`

# Make sure JAVA and NDDSHOME are set correctly
test -z "$JAVA" && echo "[ERROR]: java not found in \$PATH" && exit -1
test -z "$NDDSHOME" && echo "[ERROR]: The NDDSHOME environment variable is not set." && exit -1
test -z "$RTI_PERFTEST_ARCH" && echo "[ERROR]: The RTI_PERFTEST_ARCH environment variable is not set." && exit -1

export LD_LIBRARY_PATH=${NDDSHOME}/lib/java:${LD_LIBRARY_PATH}
export LD_LIBRARY_PATH=${NDDSHOME}/lib/${RTI_PERFTEST_ARCH}:${LD_LIBRARY_PATH}
export DYLD_LIBRARY_PATH=$LD_LIBRARY_PATH


$JAVA -Xmx1500m -cp "$script_location/perftest_java.jar:$NDDSHOME/lib/java/nddsjava.jar" \
    com.rti.perftest.ddsimpl.PerfTestLauncher $@
