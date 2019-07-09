.. _section-command_line_parameters:

Command-Line Parameters
=======================

Several parameters are available; you can enter them on the command
line. All parameters are optional and case-insensitive; partial matches
are allowed (such as ``-h`` instead of ``-help``).

Some parameters only make sense in the publishing or subscribing
application. The parameters are presented in the following tables, based
on whether they may be used in a publishing application, a subscribing
application, or both:

-  :ref:`Test Parameters for Publishing and Subscribing Applications`
-  :ref:`Test Parameters Only For Publishing Applications`
-  :ref:`Test Parameters Only For Subscribing Applications`
-  :ref:`Test Parameters to Control RTI Connext DDS Secure Options`

As you will see in the tables, the ``-pub`` parameter specifies a
publishing application and the ``-sub`` specifies a subscribing
application. If you do not specify See ``-pub`` then ``-sub`` is
assumed.

Some of these parameters are valid when using *RTI Connext DDS Pro* and
*RTI Connext DDS Micro* and *Raw Transport*, however, some of them are just
available for some of the implementations, this should be stated both in each
of the descriptions of the parameters and in the ``-help`` information displayed
by the application.

For additional information on setting the parameters, see sections:

-  :ref:`Publication rate and Spinning vs. Sleeping`
-  :ref:`Send-Queue Size and Queue-Full Behavior`
-  :ref:`Number of Iterations vs. Latency Count`
-  :ref:`Warming Up`
-  :ref:`WaitSet Event Count and Delay`
-  :ref:`How to Measure Latency for a Given Throughput`
-  :ref:`Auto Tuning and Turbo Mode`

.. _Test Parameters for Publishing and Subscribing Applications:

Test Parameters for Publishing and Subscribing Applications
------------------------------------------------------------

-  ``-bestEffort``

   Use best-effort communication.

   **Default:** ``false`` (use reliable communication).

   For an introduction to the RTI reliability model, see the Strict
   Reliability design pattern in the RTI Connext DDS Core Libraries
   Getting Started Guide. See also: Reliable Communications in the RTI
   Connext DDS Core Libraries User’s Manual.

-  ``-dataLen <bytes>``

   Length of payload in bytes for each send.

   | **Default:** ``100 bytes.``
   | **Range:** ``28 - 2147482620 bytes``

   The lower limit is the number of "overhead" bytes in the message
   (i.e., the timestamp, sequence number, and other meta-data used by
   the test); the upper limit ensures that, when the overhead of the
   wire protocol is added, it doesn't overflow the UDP maximum datagram
   size of 64KB.

   If ``<bytes>`` is bigger than 63000 *RTI Perftest* will enable the
   use of *Asynchronous Publishing* and *Unbounded Sequences*. When using
   *RTI Connext DDS Micro*, the type is not really unbounded, the size is
   given by the ``MICRO_UNBOUNDED_SEQUENCE_SIZE`` constant, that can be
   modified in the ``build.bat`` and ``build.sh`` scripts.

   If ``-scan`` is specified, this value is ignored.

-  ``-verbosity``

   Run with different levels of verbosity for RTI Connext DDS.

   | ``0`` - ``SILENT``
   | ``1`` - ``ERROR`` (default)
   | ``2`` - ``WARNING``
   | ``3`` - ``ALL``

-  ``-dynamicData``

   Run using the Dynamic Data API functions instead of the *rtiddsgen*
   generated calls.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** false

-  ``-durability <0|1|2|3>``

   Sets the Durability kind:

   | ``0`` - ``VOLATILE`` (default)
   | ``1`` - ``TRANSIENT LOCAL``
   | ``2`` - ``TRANSIENT``
   | ``3`` - ``PERSISTENT``

   For an introduction to the RTI durability model, see the Historical
   Data design pattern in the RTI Connext DDS Core Libraries Getting
   Started Guide. See also: Mechanisms for Achieving Information
   Durability and Persistence, Chapter 12, in the RTI Connext DDS Core
   Libraries User’s Manual.

   ``PERSISTENT`` is not available when compiling against *RTI Connext DDS
   Micro*.

-  ``-domain <ID>``

   Domain ID.

   The publisher and subscriber applications must use the same domain ID
   in order to communicate.

   | **Default:** ``1``
   | **Range:** ``0 - 200``

   See Choosing a Domain ID and Creating Multiple Domains, Section
   8.3.4, in the RTI Connext DDS Core Libraries User’s Manual.

-  ``-help``

   Print an informative message with all the available command-line
   parameters and exit.

-  ``-instances <int>``

   Set the number of instances to use in the test. The publishing and
   subscribing applications must specify the same number of instances.

   This option only makes sense when testing a keyed data type; to do
   so, use See ``-keyed``.

   | **Default:** ``1``
   | **Range:** ``> 0``

-  ``-keyed``

   Specify the use of a keyed type.

   **Default:** ``Unkeyed`` type.

-  ``-multicast``

   Use multicast to receive data. In addition, the Datawriter heartbeats
   will be sent using multicast instead of unicast.

   The following default multicast addresses will be used for each of the topics::

   | **latency:** ``239.255.1.2``
   | **throughput:** ``239.255.1.1``
   | **announcement:** ``239.255.1.100``

   See ``-multicastAddr <address>`` for how to change these IP addresses.

   **Default:** Do not use multicast.

-  ``-multicastAddr <address>``

   Enable the use of multicast. In addition, the Datawriter heartbeats
   will be sent using multicast instead of unicast.

   The <address> will be used by the 3 topics **latency:**, **throughput:**
   and **announcement:**.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** Do not use multicast.

-  ``-noDirectCommunication``

   Indicates if the subscribing application will receive samples from
   the publishing application when RTI Persistence Service is used.

   Only applies when ``-durability <0|1|2|3>`` is ``TRANSIENT (2)`` or
   ``PERSISTENT (3)``.

   If set to ``true`` (the default), the subscribing application gets
   samples from the publishing application and *RTI Persistence
   Service*. This mode provides low latency between endpoints.

   If set to ``false``, the subscribing application only gets samples
   from *RTI Persistence Service*. This brokered communication pattern
   provides a way to guarantee eventual consistency.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``true`` (direct communication)

-  ``-noPositiveAcks``

   Disable use of positive ACKs in the reliable protocol.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``true`` (use positive ACKs)

   See ``-qosFile <filename>`` option for more information.

-  ``-noPrintIntervals``

   Prevent printing of statistics at intervals during the test.

   By default, statistics are printed every second in the subscribing
   application, and after receiving every latency echo in the publishing
   application.

-  ``-qosFile <filename>``

   Path to the XML file containing DDS QoS profiles.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``perftest_qos_profiles.xml``

   | The default file contains these QoS profiles:
   | The ``ThroughputQos``, ``LatencyQos``, and ``AnnouncementQos``
     profiles are used by default.

   **Note:** some QoS values are ‘hard-coded’ in the application,
   therefore setting them in the XML file has no effect; see the See
   Note:.

   See comments in ``perftest_qos_profiles.xml``, as well as
   **Configuring QoS with XML, Chapter 17** in the *RTI Connext DDS Core
   Libraries* User’s Manual.

-  ``-qosLibrary <library name>``

   Name of QoS Library for DDS Qos profiles.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``PerftestQosLibrary``

-  ``-noXmlQos``

   Avoid loading the QoS from the xml profile, instead, they will be
   loaded from a string in code.

   This implies that changes in the XML profile will not be used.

   This option is recommended for OS without a file-system.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Noste:** This option is only present in ``C++`` traditional and new
   PSM.

-  ``-useReadThread``

   Use a separate thread (instead of a callback) to read data.

   See :ref:`WaitSet Event Count and Delay`.

   **Default:** use callback for subscriber

-  ``-waitsetDelayUsec <usec>``

   Process incoming data in groups, based on time, rather than
   individually.

   Only used if the See ``-useReadThread`` option is specified on the
   subscriber side.

   See :ref:`WaitSet Event Count and Delay`.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``100``
   | **Range:** ``>= 0``

-  ``-waitsetEventCount <count>``

   Process incoming data in groups, based on the number of samples,
   rather than individually.

   Only used if the See ``-useReadThread`` option is specified on the
   subscriber side.

   See :ref:`WaitSet Event Count and Delay`.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``5``
   | **Range:** ``>= 1``

-  ``-asynchronous``

   Enable asynchronous publishing in the DataWriter QoS, even for data sizes
   smaller than ``MAX_SYNCRONOUS_SIZE`` (63000 Bytes).

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``Not set``

-  ``-flowController <flow>``

   Specify the name of the flow controller that will be used by the
   DataWriters. This will only have effect if the DataWriter uses
   Asynchronous Publishing either because it is using samples greater
   than 63000 Bytes or because the ``-asynchronous`` option is present.

   There are several flow controllers predefined:

   ['default','10Gbps','1Gbps'].

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*, in this case *RTI Perftest* will use the default Flow Controller.

   | **Default:** ``default``
   | **Values:** ``['default','10Gbps','1Gbps']``

-  ``-cpu``

   Display the ``cpu`` used by the *RTI Perftest* process.

   **Default:** ``not set``

-  ``-unbounded <allocation_threshold>``

    Use *Unbounded Sequences* in the data type of IDL.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``2 * dataLen up to 63000 bytes.``\  **Range:** ``28 - 63000 bytes``

-  ``-peer <address>|<addres>[:<id>]``

   Adds a peer to the peer host address list. If ``-rawTransport`` is used, a
   optional ID of the subscriber could beprovied. This argument may be repeated
   to indicate multiple peers.

   **Default:**
   ``Not set. RTI Perftest will use the default initial peers (localhost, shared-memory and multicast).``

-  ``-threadPriorities X:Y:Z``

    This Command Line parameter is supported only for the C++ and C++03 API
    Implementations.

    Set the priorities for the application Threads:
        X -- For the Main Thread, which will be the one sending the data. Also
             for the Asynchronous thread if that one is used.
        Y -- For the Receive Threads, If the -useReadThread is used, also for
             the thread created to receive and process data.
        Z -- For the rest of the threads created by the middleware: Event and
             Database Threads.

    Three default values: h (high), n (normal) and l (low) can be used
    instead of numbers.

    To see what values can be used for the different threads see
    *RTI Connext DDS Core Libraries Platform Notes Version 5.3.1*

    - Table 6.7 Thread-Priority Definitions for Linux Platforms
    - Table 8.6 Thread-Priority Definitions for OS X Platforms
    - Table 12.7 Thread-Priority Definitions for Windows Platforms

   This parameter is not available when compiling against *RTI Connext DDS
   Micro* nor for *Raw Transport*.

   **Default:**
   ``Not set. The priority will not be modified.``

Transport Specific Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~

By default, *RTI Perftest* will try to use the transport settings provided via the
`xml` configuration file. However, it is possible to override these values directly
by using the `Transport` spececific command-line parameters.

-  ``-transport <TRANSPORT NAME>``

   Set the transport to be used. The rest of the transports will be disabled.

   | **Options Pro:** ``UDPv4``, ``UDPv6``, ``SHMEM``, ``TCP``, ``TLS``, ``DTLS`` and ``WAN``.
   | **Default Pro:** ``Transport defined in the XML profile. (UDPv4 and SHMEM if no changes).``

   | **Options Micro:** ``UDPv4``, ``SHMEM``.
   | **Default Micro:** ``UDPv4``.

   | **Options Raw Transport:** ``UDPv4``, ``SHMEM``.
   | **Default Raw Transport:** ``UDPv4``.

-  ``-allowInterfaces <ipaddr> / -nic <ipaddr>``

  Restrict RTI Connext DDS to sending output through this interface.
  The value should be the IP address assigned to any of the available network
  interfaces on the machine. On UNIX systems the name of the interface is also
  valid. This command line parameter is mapped to the "allow_interfaces_list"
  property in RTI Connext DDS.

  By default, RTI Connext DDS will attempt to contact all possible
  subscribing nodes on all available network interfaces. Even on a
  multi-NIC machine, the performance over one NIC vs. another may be
  different (e.g., Gbit vs. 100 Mbit), so choosing the correct NIC is
  critical for a proper test.

  When compiling against *RTI Connext DDS Micro*, this option should always use
  the name of the interface, not the ip address (which it is valid when compiling
  against *RTI Connext DDS Pro*).

-  ``-transportVerbosity <level>``

   Especific verbosity of the transport plugin.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``0`` (Errors only).

-  ``-transportServerBindPort <port>``

   For TCP and TLS. Port used by the transport to accept TCP/TLS connections.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``7400``

-  ``-transportWan``

   For TCP and TLS. Use tcp across LANs and firewalls.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``Not set``, LAN Mode.

-  ``-transportPublicAddress <ipaddr>``

   For TCP and TLS. Public IP address and port (WAN address and port) (separated by ‘:’)
   associated with the transport instantiation.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``Not set``

-  ``-transportWanServerAddress <ipaddr>``

   For WAN transport. Address where to find the WAN Server.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``Not set``

-  ``-transportWanServerPort <ipaddr>``

   For WAN transport. Port where to find the WAN Server.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``Not set``

-  ``-transportWanId <id>``

   For WAN transport. Id to be used for the WAN transport. Required when using WAN.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``Not set``

-  ``-transportSecureWan``

   For WAN transport. Use DTLS security over WAN.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``Not set``

-  ``-transportCertAuthority <file>``

   For TLS, DTLS and Secure WAN. Certificate authority file to be used by TLS.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default for Publisher:** ``./resource/secure/pub.pem``
   | **Default for Subscriber:** ``./resource/secure/sub.pem``

-  ``-transportCertFile <file>``

   For TLS, DTLS and Secure WAN. Certificate file to be used by TLS.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default:** ``./resource/secure/cacert.pem``

-  ``-transportPrivateKey <file>``

   For TLS, DTLS and Secure WAN. Private key file to be used by TLS.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   | **Default for Publisher:** ``./resource/secure/pubkey.pem``
   | **Default for Subscriber:** ``./resource/secure/subkey.pem``

.. _Test Parameters Only For Publishing Applications:

Test Parameters Only For Publishing Applications
------------------------------------------------

-  ``-batchSize <bytes>``

   Enable batching and set the maximum batched message size.
   Disabled automatically if using large data.

   | **Default:** ``0`` (batching disabled)
   | **Range:** ``1 to 63000``

   For more information on batching data for high throughput, see the
   **High Throughput design pattern** in the *RTI Connext DDS Core
   Libraries Getting Started Guide*. See also: **How to Measure Latency
   for a Given Throughput and the BATCH QosPolicy, Section 6.5.2** in
   the *RTI Connext DDS Core Libraries Getting User’s Manual*.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

-  ``-enableAutoThrottle``

   Enable the Auto Throttling feature. See :ref:`Auto Tuning and Turbo Mode`.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** feature is disabled.

-  ``-enableTurboMode``

   Enables the Turbo Mode feature. See :ref:`Auto Tuning and Turbo Mode`.
   When turbo mode is enabled, See ``-batchSize <bytes>`` is ignored.
   Disabled automatically if using large data or asynchronous.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** feature is disabled.

-  ``-executionTime <sec>``

   Allows you to limit the test duration by specifying the number of
   seconds to run the test.

   The first condition triggered will finish the test: ``-numIter`` or
   ``-executionTime <sec>``.

   **Default:** 0 (i.e. don't set execution time)

-  ``-latencyCount <count>``

   Number samples to send before a latency ping packet is sent.

   See :ref:`Number of Iterations vs. Latency Count`.

   **Default:** ``-1`` (if ``-latencyTest`` is not specified,
   automatically adjusted to 10000 or ``-numIter`` whichever is less; 
   if -latency Test is specified, automatically adjusted to 1).

   **Range:** must be ``<= -numIter``

-  ``-latencyTest``

   Run a latency test consisting of a ping-pong.

   The publisher sends a ping, then blocks until it receives a pong from
   the subscriber.

   Can only be used on a publisher whose ``pidMultiPubTest = 0`` (see
   See ``-pidMultiPubTest <id>``).

   **Default:** ``false``

-  ``-lowResolutionClock``

   This option enables the measurement of the latency for systems where the
   clock resolution is not good enough and the measurements per samples are
   not accurate.

   If the machine where *RTI Perftest* is being executed has a low resolution
   clock, the regular logic might not report accurate latency numbers. Therefore
   *RTI Perftest* implements a simple solution to get a rough estimation of the
   latency:

   Before sending the first sample *RTI Perftest* takes the time and right after
   receiving the last pong the time is taken again. Then, under the assumption that
   the processing time is negligible, the average latency is calculated as half of 
   the taken time divided by the number of samples sent.
   
   This calculation does only make sense if latencyCount = 1 (Latency Test), since
   it assumes that every single ping is answered.

   **Default:** ``not set``

-  ``-numIter <count>``

   Number of samples to send.

   See :ref:`Number of Iterations vs. Latency Count` and See :ref:`Warming Up`.

   If you set ``scan`` = ``true``, you cannot set this option (See
   ``-scan``).

   | **Default:** ``100000000`` for throughput tests or ``10000000``
                   for latency tests (when ``-latencyTest`` is specified);
                   also, see ``-executionTime``
   | **Range:** ``latencyCount`` (adjusted value) or higher (see
     ``-latencyCount <count>``).

-  ``-numSubscribers <count>``

   Have the publishing application wait for this number of subscribing
   applications to start.

   **Default:** ``1``

-  ``-pidMultiPubTest <id>``

   Set the ID of the publisher in a multi-publisher test.

   Use a unique value for each publisher running on the same host that
   uses the same domain ID.

   | **Default:** ``0``
   | **Range:** ``0 to n-1``, inclusive, where n is the number of
     publishers in a multi-publisher test.

-  ``-pub``

   Set test to be a publisher.

   **Default:** ``-sub``

-  ``-pubRate <sample/s>:<method>``

   Limit the throughput to the specified number of samples per second.
   The method to control the throughput rate can be: 'spin' or 'sleep'.

   If the method selected is 'sleep', RTI Perftest will control the rate
   by calling the sleep() function between writing samples. If the
   method selected is 'spin', RTI Perftest will control the rate by
   calling the spin() function (active wait) between writing samples.

   Note: The resolution provided by using 'spin' is generally better
   than the 'sleep' one, specially for fast sending rates (where the
   time needed to spend between sending samples is very small). However
   this will also result in a higher CPU consumption.

   | **Default samples:** ``0`` (no limit)
   | **Range samples:** ``1 to 10000000``

   | **Default method:** ``spin``
   | **Values method:** ``spin or sleep``

-  ``-scan <size1>:<size2>:...:<sizeN>``

   Run test in scan mode. The list of sizes is optional and can be either in the
   [32,63000] range or the [63001,2147482620] range (Large Data cannot be tested
   in the same scan test as small data sizes). Default values to test with are
   '32:64:128:256:512:1024:2048:4096:8192:16384:32768:63000'
   The ``-executionTime`` parameter is applied for every size of the scan.
   If ``-executionTime`` is not set, a timeout of 60 seconds will be applied.

   **Default:** ``false`` (no scan)

-  ``-sendQueueSize <number>``

   Size of the send queue.

   When ``-batchSize <bytes>`` is used, the size is the number of
   batches.

   See Send-Queue Size and Queue-Full Behavior.

   | **Default:** ``50``
   | **Range:** ``[1-100 million]`` or ``-1`` (indicating an unlimited
     length).

-  ``-sleep <millisec>``

   Time to sleep between each send.

   See Spinning vs. Sleeping.

   | **Default:** ``0``
   | **Range:** ``0`` or higher

-  ``-writerStats``

   Enable extra messages showing the Pulled Sample Count of the Writer
   in the Publisher side.

   The frequency of these log messages will be determined by the
   ``-latencyCount`` since the message is only shown after a *latency
   ping*.

   **Default:** ``Not enabled``

-  ``-writeInstance <instance>``

   Set the instance number to be sent.

   | **Default:** ``Round-Robin schedule``
   | **Range:** ``0 and instances``

.. _Test Parameters Only For Subscribing Applications:

Test Parameters Only For Subscribing Applications
-------------------------------------------------

-  ``-numPublishers <count>``

   The subscribing application will wait for this number of publishing
   applications to start.

   **Default:** ``1``

-  ``-sidMultiSubTest <id>``

   ID of the subscriber in a multi-subscriber test.

   Use a unique value for each subscriber running on the same host that
   uses the same domain ID.

   | **Default:** ``0``
   | **Range:** ``0 to n-1``, inclusive, where n is the number of
     subscribers in a multi-subscriber test.

-  ``-sub``

   Set test to be a subscriber.

   **Default:** ``-sub``

-  ``-cft <start>:<end>``

   Use a Content Filtered Topic for the Throughput topic in the
   subscriber side Specify 2 parameters: and to receive samples with a
   key in that range. Specify only 1 parameter to receive samples with
   that exact key.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``Not set``

.. _Test Parameters to Control RTI Connext DDS Secure Options:

Test Parameters to Control RTI Connext DDS Secure Options
---------------------------------------------------------

-  ``-secureEncryptDiscovery``

   Encrypt discovery traffic.

   **Default:** Not set.

-  ``-secureSign``

   Sign discovery and user data packages.

   **Default:** Not set.

-  ``-secureEncryptData``

   Encrypt at the user data level.

   **Default:** Not set.

-  ``-secureEncryptSM``

   Encrypt at the RTPS sub-message level.

   **Default:** Not set.

-  ``-secureGovernanceFile <file>``

   Governance file. If specified, the authentication, signing, and
   encryption arguments are ignored. The governance document
   configuration will be used instead.

   **Default:** Not set.

-  ``-securePermissionsFile <file>``

   Permissions file to be used.

   | **Default for Publisher:**
     ``./resource/secure/signed_PerftestPermissionsPub.xml``
   | **Default for Subscriber:**
     ``./resource/secure/signed_PerftestPermissionsSub.xml``

-  ``-secureCertAuthority <file>``

   Certificate authority file to be used.

   | **Default for Publisher:** ``./resource/secure/pub.pem``
   | **Default for Subscriber:** ``./resource/secure/sub.pem``

-  ``-secureCertFile <file>``

   Certificate file to be used.

   **Default:** ``./resource/secure/cacert.pem``

-  ``-securePrivateKey <file>``

   Private key file to be used.

   **Default for Publisher:** ``./resource/secure/pubkey.pem`` **Default
   for Subscriber:** ``./resource/secure/subkey.pem``


RawTransport Specific Options
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
-  ``-rawTransport``

   Use sockets as a transport instead of DDS protocol. It supports 
   ``UDPv4`` and Shared Memory (``SHMEM``).
   Some of the *RTI Connext DDS* parameters are not supported when using
   sockets.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``Not set``

-  ``-noBlockingSockets``

   Control blocking behavior of send sockets to never block.
   CHANGING THIS FROM THE DEFAULT CAN CAUSE SIGNIFICANTPERFORMANCE PROBLEMS.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** ``Not set. Always Block``

Additional information about the parameters
-------------------------------------------

Secure Certificates, Governance and Permission Files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RTI Perftest provides a set of already generated certificates,
governance and permission files to be loaded when using the *RTI Connext DDS Secure
Libraries*. Both governance files and permission files are already
signed, so no action is required by the user. These files are located in
``$(RTIPERFTESTHOME)/resource/secure``.

In addition to the already signed governance and permission files, the
original files are also provided (not signed) as well as a ``bash``
script with the steps to generate all the signed files. Those files can
be found in ``$(RTIPERFTESTHOME)/resource/secure/input``; the script is
in ``$(RTIPERFTESTHOME)/resource/secure/make.sh``.

.. _Publication rate and Spinning vs. Sleeping:

Publication rate and Spinning vs. Sleeping
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When the publisher is writing as fast as it can, sooner or later, it is
likely to get ahead of the subscriber. There are 4 things you can do in
this case:

1. Nothing -- for reliable communication, ``write()`` will block until
   the subscriber(s) catch up.

2. Slow the writing down by sleeping (See ``-sleep <millisec>``). This
   approach is friendlier to the other processes on the host because it
   does not monopolize the CPU. However, context switching is expensive
   enough that you can't actually "sleep" for amounts of time on the
   order of microseconds, so you could end up sleeping too long and
   hurting performance. (Operating systems (including Linux and Windows)
   have a minimum resolution for sleeping; i.e., you can only sleep for
   a period of 1 or 10 ms. If you specify a sleep period that is less
   than that minimum, the OS may sleep for its minimum resolution.)

3. Set a publication rate (See ``-pubRate <count>:<method>``). This approach
   will make *RTI Perftest* automatically set the rate of the write call so
   you can get the number of samples per second requested (if possible).
   This option allows to choose to use ``sleep()`` between calls or ``spin()``.
   This second approach will add a pause without yielding the CPU to other
   processes, making it easier to "sleep" for very short periods of time. 
   Avoid spinning on a single-core machine, as the code that would break 
   you out of the spin may not be able to execute in a timely manner.

4. Let the publisher automatically adjust the writing rate (See
   ``-enableAutoThrottle``). This option enables the Auto Throttle
   feature introduced in RTI Connext DDS 5.1.0 and its usage is
   preferred over See ``-spin <count>`` because the amount of spin is
   automatically determined by the publisher based on the number of
   unacknowledged samples in the send queue.

See also: :ref:`Send-Queue Size and Queue-Full Behavior`.

.. _Send-Queue Size and Queue-Full Behavior:

Send-Queue Size and Queue-Full Behavior
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In many distributed systems, a data producer will often outperform data
consumers. That means that, if the communications are to be reliable,
the producer must be throttled in some way to allow the consumers to
keep up. In some situations, this may not be a problem, because data may
simply not be ready for publication at a rate sufficient to overwhelm
the subscribers. If you're not so lucky, your publisher's queue of
unacknowledged data will eventually fill up. When that happens, if data
is not to be lost, the publication will have to block until space
becomes available. Blocking can cost you in terms of latency.

To avoid the cost of blocking, consider the following:

-  Enlarge your publisher's queue (See ``-sendQueueSize <number>``).
   Doing so will mean your publisher has to block less often. However,
   it may also let the publisher get even further ahead of slower
   subscribers, increasing the number of dropped and resent packets,
   hurting throughput. Experimenting with the send queue size is one of
   the easy things you can do to squeeze a little more throughput from
   your system.

-  Enable Auto Throttling (See ``-enableAutoThrottle``). This option
   enables the Auto Throttle feature introduced in *RTI Connext DDS
   5.1.0*. When this option is used, the publisher automatically adjusts
   the writing rate based on the number of unacknowledged samples in the
   send queue to avoid blocking.

**Note:**

The following values in the ``DataWriterProtocolQosPolicy`` are
‘hard-coded’ in the application, therefore setting these values in the
XML QoS profile will have no effect:

-  ``rtps_reliable_writer.heartbeats_per_max_samples`` is set to
   (``sendQueueSize/10``)
-  ``rtps_reliable_writer.low_watermark`` is set to
   (``sendQueueSize * 0.10``)
-  ``rtps_reliable_writer.high_watermark`` is set to
   (``sendQueueSize * 0.90``)

For more information on the send queue size, see the ``RESOURCE_LIMITS``
QosPolicy, **Section 6.5.20** in the *RTI Connext DDS Core Libraries
User’s Manual* (specifically, the ``max_samples`` field).

.. _Number of Iterations vs. Latency Count:

Number of Iterations vs. Latency Count
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When configuring the total number of samples to send during the test
(See ``-numIter <count>``) and the number of samples to send between
latency pings (See ``-latencyCount <count>``), keep these things in
mind:

-  Don't send latency pings too often. One of the purposes of the test
   is to measure the throughput that the middleware is able to achieve.
   Although the total throughput is technically the total data sent on
   both the throughput and latency topics, for the sake of simplicity,
   the test measures only the former. The implicit assumption is that
   the latter is negligible by comparison. If you violate this
   assumption, your throughput test results will not be meaningful.

-  Keep the number of iterations large enough to send many latency pings
   over the course of the test run. Your latency measurements, and the
   spread between them, will be of higher quality if you are able to
   measure more data points.

-  When selecting See ``-numIter <count>``, choose a value that allows
   the test to run for at least a minute to get accurate results. Set
   See ``-numIter <count>`` to be millions for small message sizes
   (<1k); reduce as needed for larger sizes (otherwise the tests will
   take longer and longer to complete).

.. _Warming Up:

Warming Up
~~~~~~~~~~

When running the performance test in *Java*, and to a lesser extent,
*C#*, you may observe that throughput slowly increases through the first
few incremental measurements and then levels off. This improvement
reflects the background activity of the just-in-time (JIT) compiler and
optimizer on these platforms. For the best indication of steady-state
performance, be sure to run the test for a number of samples (See
``-numIter <count>``) sufficient to smooth out this start-up artifact.

.. _WaitSet Event Count and Delay:

WaitSet Event Count and Delay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*RTI Connext DDS*, and by extension, this performance test, gives you
the option to either process received data in the middleware's receive
thread, via a listener callback, or in a separate thread (See
``-useReadThread``) via an object called a WaitSet. The latter approach
can be beneficial in that it decouples the operation of your application
from the middleware, so that your processing will not interfere with
*Connext DDS*'s internal activities. However, it does introduce
additional context switches into your data receive path. When data is
arriving at a high rate, these context switches can adversely impact
performance when they occur with each data sample.

To improve efficiency, the command-line parameters
``-waitsetDelayUsec <usec>`` and ``-waitsetEventCount <count>`` allow
you to process incoming data in groups, based on the number of samples
and/or time, rather than individually, reducing the number of context
switches. Experiment with these values to optimize performance for your
system.

For more information, see these sections in the *RTI Connext DDS Core
Libraries User’s Manual*: **Receive Threads (Section 19.3)** and
**Conditions and WaitSets (Section 4.6)**.

.. _How to Measure Latency for a Given Throughput:

How to Measure Latency for a Given Throughput
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want to measure the minimum latency for a given throughput, you
have to use the command-line parameters ``-sleep <millisec>``,
``-spin <count>`` and ``-batchSize <bytes>`` to experimentally set the
throughput level for a given test run.

For example, suppose you want to generate a graph of latency vs.
throughput for a packet size of ``200 bytes`` and throughput rates of
``1000``, ``10K``, ``20K``, ``50K``, ``100K``, ``500K``, and
``Max messages`` per second.

For throughput rates under 1000 messages per second, use ``-sleep <ms>``
to throttle the publishing application. For example, ``-sleep 1`` will
produce a throughput of approximately 1000 messages/second; ``-sleep 2``
will produce a throughput of approximately 500 messages/second.

For throughput rates higher than 1000 messages per second, use
``-spin <spin count>`` to cause the publishing application to busy wait
between sends. The ``<spin count>`` value needed to produce a given
throughput must be experimentally determined and is highly dependent on
processor performance. For example ``-spin 19000`` may produce a message
rate of 10000 messages/second with a slow processor but a rate of 14000
messages/second with a faster processor.

Use batching when you want to measure latency for throughput rates
higher than the maximum rates of sending individual messages. First,
determine the maximum throughput rate for the data size under test
without batching (omit See ``-batchSize <bytes>``). For example, on a
1-Gigabyte network, for a data size of ``200 bytes``, the maximum
throughput will be about 70,000 messages/sec. We will refer to this
value as ``max_no_batch``.

For all throughput rates less than ``max_no_batch`` (e.g., 70,000
messages/sec.), do not use batching, as this will increase the latency.

Use batching to test for throughput rates higher than ``max_no_batch``:
start by setting ``-batchSize`` to a multiple of the data size. For
example, if the data size is ``200 bytes``, use ``-batchSize 400`` (this
will put 2 messages in each batch), ``-batchSize 800`` (4 per batch),
etc. This will allow you to get throughput/latency results for
throughputs higher than the ``max_no_batch`` throughput rate.

**Note:** For larger data sizes (``8000 bytes`` and higher), batching
often does not improve throughput, at least for 1-Gigabyte networks.

.. _Auto Tuning and Turbo Mode:

Auto Tuning and Turbo Mode
~~~~~~~~~~~~~~~~~~~~~~~~~~

*RTI Connext DDS* includes since 5.1.0 two features that allow the middleware
to auto-tune the communications to achieve better performance. These
features are **Auto Throttling** and **Turbo Mode**. For more
information about both features, refer to **Sections 10.4, Auto
Throttling for DataWriter Performance -- Experimental Feature** and
**6.5.2.4 Turbo Mode: Automatically Adjusting the Number of Bytes in a
Batch -- Experimental** Feature in the *RTI Connext DDS Core Libraries
User's Manual*. The performance test application includes two
command-line options to enable these features: ``-enableAutoThrottle``
and ``-enableTurboMode``.

With Auto Throttling, the publisher automatically adjusts the writing
rate based on the number of unacknowledged samples in the send queue to
avoid blocking and provide the best latency/throughput tradeoff.

With Turbo Mode, the size of a batch is automatically adjusted to
provide the best latency for a given write rate. For slow write rates,
the batch size will be smaller to minimize the latency penalty. For high
write rates, the batch size will be bigger to increase throughput. When
turbo mode is used, the command-line option See ``-batchSize <bytes>``
is ignored.

To achieve the best latency under maximum throughput conditions, use See
``-enableAutoThrottle`` and See ``-enableTurboMode`` in combination.
