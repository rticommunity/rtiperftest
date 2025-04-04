.. _section-execution:

Execution
=========

The *RTI Perftest* test is provided in C++ (modern and traditional),
C#, and Java. The sections below describe how to run the executables, once
you have built them, and how to pass configuration parameters to them. For
detailed descriptions of the test parameters, see :ref:`Test Parameters
for Publishing and Subscribing Applications`. For example test
configurations, see :ref:`section-command_line_examples`.

When running the test, keep in mind that a throughput test will
necessarily place a heavy load on your network and potentially on your
CPU(s) as well. For accurate results, and the fewest complaints
from your coworkers and IT department, run the test when and where you
have a subnet to yourself. The test is designed to measure latency under
loaded network conditions; it will produce those loads itself: there is
no need to produce them externally (and your throughput results may not
be meaningful if you do).

C++ and C# Executables
----------------------

The C++ and C# executables are in these directories:

.. code-block:: console

    <installation directory>/bin/<architecture>/release
    <installation directory>/bin/<architecture>/debug

The ``<architecture>`` depends on your architecture, such as
``i86Linux3gcc4.8.2`` or ``i86Win32VS2012``.

You can differentiate the executables for the two C++ implementations
(Traditional and Modern) by the name: the Traditional C++ API
implementation uses ``perftest_cpp`` and the Modern C++ API
implementation uses ``perftest_cpp11``.

The *RTI Perftst* test uses an XML configuration file and locates this file based on
paths relative to the directory from which the test is run. Therefore,
to use this configuration file without the need of adding extra command
line parameters:

Traditional C++:

.. code-block:: console

    bin/<architecture>/<release or debug>/perftest_cpp <-pub|-sub(default)> [parameters]

Modern C++:

.. code-block:: console

    bin/<architecture>/<release or debug>/perftest_cpp11 <-pub|-sub(default)> [parameters]

C#:

.. code-block:: console

    bin/<architecture>/<release or debug>/perftest_cs <-pub|-sub(default)> [parameters]

With dynamic linking
~~~~~~~~~~~~~~~~~~~~

If you compiled the performance test executable dynamically, add the
``$NDDSHOME/lib/<architecture>`` folder to:

-  the ``$LD_LIBRARY_PATH`` variable if you are on Linux systems
-  the ``$DYLD_LIBRARY_PATH`` variable if you are on macOS systems
-  the ``%PATH%`` variable if you are on Windows systems

When using *RTI Security Plugins* and using dynamic linking
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In this case, add the *OpenSSL* or *wolfSSL* libraries in
``$SSLHOME/<debug or release>/lib`` to:

-  the ``$LD_LIBRARY_PATH`` variable if you are on Linux systems
-  the  ``$DYLD_LIBRARY_PATH`` variable if you are on macOS systems
-  the  ``%PATH%`` variable if you are on Windows systems

With VxWorks kernel mode
~~~~~~~~~~~~~~~~~~~~~~~~

After compiling for VxWorks (see :ref:`section-compilation`),
the shared object needs to be loaded in the kernel:

.. code-block:: console

    ld 1 < bin/<architecture>/release/perftest_cpp.so

Now, the code can be executed by calling the ``perftest_main_cpp`` function
with a string as an argument containing all of the :ref:`section-command_line_parameters`.
For example:

.. code-block:: console

    perftest_cpp_main "-pub -domain 0 -latencyCount 1 -dataLen 1000 -latencyTest -multicast"

Java Executable
---------------

*RTI Perftest* provides a ``.sh`` script and ``.bat`` script to run the
Java ``.jar`` file. Those scripts are located in:

-  ``bin/<debug or release>/perftest_java.sh`` for Linux/macOS/QNX systems
-  ``bin/<debug or release>/perftest_java.bat`` for Windows systems

When using the *RTI Perftest* scripts, you will need to set the
environment variable ``$RTI_PERFTEST_ARCH`` to your specific
architecture as well as set your ``$NDDSHOME`` variable.

**For example**: If you are using a Windows 32-bit architecture and the
*Visual Studio 2012* libraries:

.. code-block:: console

    set NDDSHOME=<path to your RTI Connext DDS installation>
    set RTI_PERFTEST_ARCH=i86Win32VS2012

If you are using the Linux ``i86Linux3gcc4.8.2`` architecture:

.. code-block:: console

    export NDDSHOME=<path to your RTI Connext DDS installation>
    export RTI_PERFTEST_ARCH=i86Linux3gcc4.8.2

Make sure the correct ``java`` executable is in your path before running
the script.

Alternatively, you can directly run the Java ``.jar`` files:

.. code-block:: console

    java -cp "<RTI Perftest path>/bin/<release or debug>/perftest_java.jar:<NDDSHOME>/lib/java/<nddsjava.jar or nddsjava.jar>" com.rti.perftest.ddsimpl.PerfTestLauncher

If you run the ``.jar`` files directly, add the ``$NDDSHOME/lib/<architecture>``
and ``$NDDSHOME/lib/java`` folders to:

-  the ``$LD_LIBRARY_PATH`` variable if you are on Linux systems
-  the ``$DYLD_LIBRARY_PATH`` variable if you are on macOS systems
-  the ``%PATH%`` variable if you are on Windows systems

**When using Java on Linux/macOS systems with RTI Security Plugins**:
Add the *OpenSSL*/*wolfSSL* libraries in ``$SSLHOME/<debug or release>/lib`` to:

-  the ``$LD_LIBRARY_PATH`` variable if you are on Linux systems
-  the  ``$DYLD_LIBRARY_PATH`` variable if you are on macOS systems
-  the  ``%PATH%`` variable if you are on Windows systems

Launching the Application
-------------------------

The *RTI Perftest* test uses an XML configuration file. It locates this file based on
its path relative to the directory from which the test is run. To use
this configuration file, move to *RTI Perftest*'s top-level location.

Then start the test applications. You can start the publisher or subscribers
first, the order does not matter. When selecting your optional
parameters, choose parameters that allow the test to run for at least 15
seconds to get any kind of meaningful results. 

To choose the test run time, use 
the ``-executionTime`` command-line option or set the total number of
samples by using the ``-numIter`` command-line option.
The longer the test runs, the more accurate the results will be.
Ideally, run the test for at least 100 seconds.

Starting the C++ Traditional API test:

.. code-block:: console

    bin/<architecture>/<release or debug>/perftest_cpp <-pub|-sub(default)> [parameters]

Starting the C++ Modern API test:

.. code-block:: console

    bin/<architecture>/<release or debug>/perftest_cpp11 <-pub|-sub(default)> [parameters]

Starting the C# API test:

.. code-block:: console

    bin/<architecture>/<release or debug>/perftest_cs <-pub|-sub(default)> [parameters]

Starting the Java API test:

.. code-block:: console

    bin/<release or debug>/perftest_java <-pub|-sub(default)> [parameters]

The ``<architecture>`` depends on your architecture, such as
``x64Linux3gcc4.8.2`` or ``i86Win32VS2012``.

After the publisher recognizes that the specified number of subscribers
(see the ``-numSubscribers <count>`` option) are online and the
subscriber recognizes that the specified number of publishers (see the
``-numPublishers <count>`` option) are online, the test begins.

Output Example
--------------

The following is an example of the expected output from the performance
test.

-  Publisher:

.. code-block:: console

    > perftest_cpp -pub -noPrint -domain 27 -latencyCount 100000 -datalen 32 -exec 30 -nic lo0
    RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

    Mode: THROUGHPUT TEST
        (Use "-latencyTest" for Latency Mode)

    Perftest Configuration:
        Reliability: Reliable
        Keyed: No
        Publisher ID: 0
        Latency count: 1 latency sample every 100000 samples
        Data Size: 32
        Batching: 8192 Bytes (Use "-batchSize 0" to disable batching)
        Publication Rate: Unlimited (Not set)
        Execution time: 30 seconds
        Receive using: Listeners
        Domain: 27
        Dynamic Data: No
        Asynchronous Publishing: No
        XML File: perftest_qos_profiles.xml

    Transport Configuration:
        Kind: UDPv4 & SHMEM (taken from QoS XML file)
        Use Multicast: False

    Waiting to discover 1 subscribers ...
    Waiting for subscribers announcement ...
    Sending 50 initialization pings ...
    Publishing data ...
    Length:    32  Latency: Ave    100 us  Std  202.5 us  Min     62 us  Max   5543 us  50%     88 us  90%    107 us  99%    180 us  99.99%   5543 us  99.9999%   5543 us
    Finishing test...
    Test ended.

-  Subscriber

.. code-block:: console

    > perftest_cpp -noPrint -domain 27
    RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

    Perftest Configuration:
        Reliability: Reliable
        Keyed: No
        Subscriber ID: 0
        Receive using: Listeners
        Domain: 27
        Dynamic Data: No
        XML File: perftest_qos_profiles.xml

    Transport Configuration:
        Kind: UDPv4 & SHMEM (taken from QoS XML file)
        Use Multicast: False

    Waiting to discover 1 publishers ...
    Waiting for data ...
    Length:    32  Packets: 85900000  Packets/s(ave): 2880385  Mbps(ave):   737.4  Lost: 0 (0.00%)
    Length:    64  Packets: 66500000  Packets/s(ave): 2232093  Mbps(ave):  1142.8  Lost: 0 (0.00%)
    Length:   256  Packets: 37200000  Packets/s(ave): 1248203  Mbps(ave):  2556.3  Lost: 0 (0.00%)
    Length:  1024  Packets: 15200000  Packets/s(ave):  506920  Mbps(ave):  4152.7  Lost: 0 (0.00%)
    Length:  8192  Packets:  1900000  Packets/s(ave):   60350  Mbps(ave):  3955.2  Lost: 0 (0.00%)
    Length: 32768  Packets:  1600000  Packets/s(ave):   52506  Mbps(ave): 13764.4  Lost: 0 (0.00%)
    Length: 63000  Packets:  1100000  Packets/s(ave):   35481  Mbps(ave): 17882.6  Lost: 0 (0.00%)
    Finishing test...
    Test ended.
