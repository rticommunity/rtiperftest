.. _section-execution:

Execution
=========

The test is provided in C++ (Modern and Traditional APIs), C#, and Java.
The list below identifies how to run the executables, once you have
built them, and how to pass configuration parameters to them. For
detailed descriptions of the test parameters, see See *Test Parameters
Section*. For example test configurations, see the *Example Command
Lines* section.

When running the test, keep in mind that a throughput test will
necessarily place a heavy load on your network and potentially on your
CPU(s) as well. For the most accurate results, and the fewest complaints
from your coworkers and IT department, run the test when and where you
have a subnet to yourself. The test is designed to measure latency under
loaded network conditions; it will produce those loads itself: There is
no need to produce them externally (and your throughput results may not
be meaningful if you do).

C++ and C# executables
----------------------

The C++ and C# executables are in these directories:

::

    <installation directory>/bin/<architecture>/release
    <installation directory>/bin/<architecture>/debug

Where ``<architecture>`` depends on your architecture, such as
``i86Linux3gcc4.8.2`` or ``i86Win32VS2012``.

You can differentiate the executables for the two C++ implementations
(Traditional and Modern) by the name: the Traditional C++ API
implementation uses ``perftest_cpp`` and the Modern C++ API
implementation is named ``perftest_cpp03``.

The test uses an XML configuration file and locates this file based on
paths relative to the directory from which the test is run. Therefore,
to use this configuration file without the need of adding extra command
line parameters:

Traditional C++:

::

    bin/<architecture>/<release or debug>/perftest_cpp <-pub|-sub(default)> [parameters]

Modern C++:

::

    bin/<architecture>/<release or debug>/perftest_cpp03 <-pub|-sub(default)> [parameters]

C#:

::

    bin/<architecture>/<release or debug>/perftest_cs <-pub|-sub(default)> [parameters]

When using dynamic linking
~~~~~~~~~~~~~~~~~~~~~~~~~~

If you compiled the performance test executable dynamically add the
``$NDDSHOME/lib/<architecture>`` folder to:

-  The ``$LD_LIBRARY_PATH`` variable if you are on Linux systems.
-  The ``$DYLD_LIBRARY_PATH`` variable if you are on OSX.
-  The ``%PATH%`` variable (if you are on Windows).

When using *RTI Secure DDS Plugin* and using dynamic linking
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In such case, add the *OpenSSL* libraries in
``$OPENSSLHOME/<debug or release>/lib`` to the ``$LD_LIBRARY_PATH``,
``$DYLD_LIBRARY_PATH`` or ``%PATH%`` variable:

-  On Linux systems, add ``$OPENSSLHOME/<debug or release>/lib`` to
   ``$LD_LIBRARY_PATH``
-  On OSX systems, add ``$OPENSSLHOME/<debug or release>/lib`` to
   ``$DYLD_LIBRARY_PATH``
-  On Windows systems, add ``%OPENSSLHOME$/<debug or release>/bin`` to
   ``%PATH%``
   
When using *RTI Perftest* in *VxWorks* kernel mode
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After compiling for *VxWorks* (see `Compilation <https://github.com/rticommunity/rtiperftest/blob/master/srcDoc/compilation.rst>`__), the shared object need to be loaded in the kernel, by doing:

``ld 1 < bin/<architecture>/release/perftest_cpp.so``

Now, the code can be executed by calling the ``perftest_main_cpp`` function with a string as an argument containing all the `command line parameters <https://github.com/rticommunity/rtiperftest/blob/master/srcDoc/command_line_parameters.rst>`__.

E.g. ``perftest_cpp_main "-pub -domain 0 -latencyCount 1 -dataLen 1000 -latencyTest -multicast"``

Java executable
---------------

*RTI Perftest* provides a *.sh* script and a *.bat* script to run the
Java ``jar`` file. Those scripts are located in:

-  ``bin/<debug or release>/perftest_java.sh`` for UNIX-based systems.
-  ``bin/<debug or release>/perftest_java.bat`` for Windows systems.

When using the *RTI Perftest* scripts, you will need to set the
environment variable ``$RTI_PERFTEST_ARCH`` to your specific
architecture as well as your ``$NDDSHOME``.

**For example**: If you are using a Windows 32-bit architecture and the
*Visual Studio 2012* libraries:

::

    set NDDSHOME=<path to your RTI Connext DDS installation>
    set RTI_PERFTEST_ARCH=i86Win32VS2012

If you are using the Linux i86Linux3gcc4.8.2 architecture:

::

    export NDDSHOME=<path to your RTI Connext DDS installation>
    export RTI_PERFTEST_ARCH=i86Linux3gcc4.8.2

Make sure the correct ``java`` executable is in your path before running
the script.

Alternatively, you can directly run the Java ``jar`` files:

::

    java -cp "<RTI Perftest Path>/bin/<release or debug>/perftest_java.jar:<NDDSHOME>/lib/java/<nddsjava.jar or nddsjava.jar>" com.rti.perftest.ddsimpl.PerfTestLauncher

In such case, add the ``$NDDSHOME/lib/<architecture>`` and the
``$NDDSHOME/lib/java`` folders to:

-  The ``$LD_LIBRARY_PATH`` variable if you are on Linux systems.
-  The ``$DYLD_LIBRARY_PATH`` variable if you are on OSX.
-  The ``%PATH%`` variable (if you are on Windows).

When using Java on UNIX-based systems with RTI Secure DDS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In such case, add the *OpenSSL* libraries in
``$OPENSSLHOME/<debug or release>/lib`` to the ``$LD_LIBRARY_PATH`` or
``%PATH%`` variable:

-  On Linux systems, add ``$OPENSSLHOME/<debug or release>/lib`` to
   ``$LD_LIBRARY_PATH``
-  On OSX systems, add ``$OPENSSLHOME/<debug or release>/lib`` to
   ``$DYLD_LIBRARY_PATH``
-  On Windows systems, add ``%OPENSSLHOME$/<debug or release>/bin`` to
   ``%PATH%``

Launching the application
-------------------------

The test uses an XML configuration file. It locates this file based on
its path relative to the directory from which the test is run. To use
this configuration file move to *RTI Perftest* top-level location.

Start the test applications. You can start the publisher or subscribers
first, the order does not matter. When selecting your optional
parameters, choose parameters that allow the test to run for at least 15
seconds to get any kind of meaningful results. The longer it runs, the
more accurate the results will be.

Ideally, you should run the test for at least 100 seconds.

C++ Traditional API
~~~~~~~~~~~~~~~~~~~

::

    bin/<architecture>/<release or debug>/perftest_cpp <-pub|-sub(default)> [parameters]

C++ Modern API
~~~~~~~~~~~~~~

::

    bin/<architecture>/<release or debug>/perftest_cpp03 <-pub|-sub(default)> [parameters]

C# API
~~~~~~

::

    bin/<architecture>/<release or debug>/perftest_cs <-pub|-sub(default)> [parameters]

Java API
~~~~~~~~

::

    bin/<release or debug>/perftest_java <-pub|-sub(default)> [parameters]

where ``<architecture>`` depends on your architecture, such as
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

::

    > perftest_cpp -pub -noPrint -domain 27 -latencyCount 100000 -scan 32:64:256:1024:8192:32768:63000 -exec 30 -nic lo0
    RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

    Mode: THROUGHPUT TEST
        (Use "-latencyTest" for Latency Mode)

    Perftest Configuration:
        Reliability: Reliable
        Keyed: No
        Publisher ID: 0
        Latency count: 1 latency sample every 100000 samples
        Data Size: 32, 64, 256, 1024, 8192, 32768, 63000
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
    Length:    64  Latency: Ave     70 us  Std   53.7 us  Min     45 us  Max   1076 us  50%     63 us  90%     84 us  99%    142 us  99.99%   1076 us  99.9999%   1076 us
    Length:   256  Latency: Ave     45 us  Std   12.0 us  Min     32 us  Max    106 us  50%     41 us  90%     69 us  99%     78 us  99.99%    106 us  99.9999%    106 us
    Length:  1024  Latency: Ave     28 us  Std    4.5 us  Min     25 us  Max     68 us  50%     27 us  90%     31 us  99%     52 us  99.99%     68 us  99.9999%     68 us
    Length:  8192  Latency: Ave     33 us  Std   12.6 us  Min     28 us  Max     86 us  50%     29 us  90%     34 us  99%     86 us  99.99%     86 us  99.9999%     86 us
    Length: 32768  Latency: Ave     46 us  Std   32.9 us  Min     35 us  Max    172 us  50%     36 us  90%     53 us  99%    172 us  99.99%    172 us  99.9999%    172 us
    Length: 63000  Latency: Ave     57 us  Std   25.2 us  Min     45 us  Max    136 us  50%     48 us  90%     55 us  99%    136 us  99.99%    136 us  99.9999%    136 us
    Finishing test...
    Test ended.

-  Subscriber

::

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
    Waiting for data...
    Length:    32  Packets: 85900000  Packets/s(ave): 2880385  Mbps(ave):   737.4  Lost: 0 (0.00%)
    Length:    64  Packets: 66500000  Packets/s(ave): 2232093  Mbps(ave):  1142.8  Lost: 0 (0.00%)
    Length:   256  Packets: 37200000  Packets/s(ave): 1248203  Mbps(ave):  2556.3  Lost: 0 (0.00%)
    Length:  1024  Packets: 15200000  Packets/s(ave):  506920  Mbps(ave):  4152.7  Lost: 0 (0.00%)
    Length:  8192  Packets:  1900000  Packets/s(ave):   60350  Mbps(ave):  3955.2  Lost: 0 (0.00%)
    Length: 32768  Packets:  1600000  Packets/s(ave):   52506  Mbps(ave): 13764.4  Lost: 0 (0.00%)
    Length: 63000  Packets:  1100000  Packets/s(ave):   35481  Mbps(ave): 17882.6  Lost: 0 (0.00%)
    Finishing test...
    Test ended.
