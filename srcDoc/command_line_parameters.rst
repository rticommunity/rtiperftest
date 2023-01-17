.. _section-command_line_parameters:

Command-Line Parameters
=======================

All parameters are optional and case-insensitive; partial matches
are allowed (such as ``-h`` instead of ``-help``), except in the C# API
implementation, where the full name has to be provided.

Some parameters only make sense in the publishing or subscribing
application. The parameters are presented in the following tables, based
on whether they may be used in a publishing application, a subscribing
application, or both:

-  :ref:`Test Parameters for Publishing and Subscribing Applications`
-  :ref:`Test Parameters only for Publishing Applications`
-  :ref:`Test Parameters only for Subscribing Applications`
-  :ref:`Test Parameters to Control Connext DDS Secure Options`

As you will see in the tables, the ``-pub`` parameter specifies a
publishing application and ``-sub`` specifies a subscribing
application. If you do not specify ``-pub``, then ``-sub`` is
assumed.

Some of these parameters are valid when using *RTI Connext DDS Professional*, 
*RTI Connext DDS Micro*, and the Raw Transport feature; some of them are 
available for just some of these implementations. Whether or not a parameter is available 
is stated in the descriptions of the parameters and in the ``-help`` 
information displayed by the application. 

For additional information on setting the parameters, see sections:

-  :ref:`Publication rate and spinning vs. sleeping`
-  :ref:`Send-queue size and queue-full behavior`
-  :ref:`Number of iterations vs. latency count`
-  :ref:`Warming up`
-  :ref:`WaitSet Event count and delay`
-  :ref:`How to measure latency for a given throughput`
-  :ref:`Auto-tuning and turbo mode`

.. _Test Parameters for Publishing and Subscribing Applications:

.. _section-pubsub_command_line_parameters:

Test Parameters for Publishing and Subscribing Applications
------------------------------------------------------------

-  ``-bestEffort``

   Use best-effort communication.

   **Default:** ``false`` (use reliable communication)

   For an introduction to the RTI reliability model, see
   `Reliable Communications, in the RTI Connext DDS Core Libraries User’s Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/reliable.htm>`__.

-  ``-dataLen <bytes>``

   Length of payload in bytes for each send.

   | **Default:** ``100 bytes``
   | **Range:** ``28 - 2147482620 bytes``

   The lower limit is the number of "overhead" bytes in the message
   (i.e., the timestamp, sequence number, and other metadata used by
   the test); the upper limit ensures that, when the overhead of the
   wire protocol is added, it doesn't overflow the UDP maximum datagram
   size of 64KB.

   If ``<bytes>`` is bigger than 64900, *RTI Perftest* will enable the
   use of *Asynchronous Publishing* and *Unbounded Sequences*. When using
   *RTI Connext DDS Micro*, the type is not really unbounded; the size is
   given by the ``MICRO_UNBOUNDED_SEQUENCE_SIZE`` constant, which can be
   modified in the ``build.bat`` and ``build.sh`` scripts.

   If ``-scan`` is specified, this value is ignored.

-  ``-verbosity``

   Run with different levels of verbosity for *Connext DDS*.

   | ``0`` - ``SILENT``
   | ``1`` - ``ERROR`` (default)
   | ``2`` - ``WARNING``
   | ``3`` - ``ALL``

-  ``-dynamicData``

   Run using the Dynamic Data API functions instead of the *rtiddsgen*
   generated calls.

   This parameter is not available when compiling against *Connext DDS
   Micro*.

   **Default:** ``false``

   .. note::

       Not yet supported in the C# API implementation.

.. _FlatData:

-  ``-flatData``

   Use the FlatData language binding API to build samples where the
   in-memory representation matches the wire representation.

   This parameter is not available when compiling against *Connext DDS
   Micro* or a *Connext DDS* version prior to 6.0.0.

   **Default:** ``false``

   .. note::

       Not available in the C# API implementation.

-  ``-zeroCopy``

   Use Zero Copy transfer over shared memory. This feature accomplishes zero
   copies by using the shared memory (SHMEM) builtin transport to send 16-byte
   references to samples within a SHMEM segment owned by the *DataWriter*.

   This parameter can only be used along with ``-flatData`` and the SHMEM builtin
   transport.

   This parameter is not available when compiling against *Connext DDS
   Micro* or a *Connext DDS* version prior to 6.0.0.

   **Default:** ``false``

   .. note::

       Not available in the C# API implementation.

-  ``-checkConsistency``

   This option is only valid when using ``-zeroCopy``. When using it, the code
   will check on the *DataReader* side if the sample read by the *DataReader* is
   consistent (issue a call to ``is_data_consistent()``).

   **Default:** ``false``

   .. note::

       Not available in the C# API implementation.

-  ``-preallocateFragmentedSamples``

   Prevent dynamic allocation of the buffer used for storing received fragments.
   Useful for data bigger than 5MB to reduce latency.

   **Default:** ``false``

   .. note::

       Not available in the C# API implementation.

-  ``-durability <0|1|2|3>``

   Sets the Durability kind:

   | ``0`` - ``VOLATILE`` (default)
   | ``1`` - ``TRANSIENT LOCAL``
   | ``2`` - ``TRANSIENT``
   | ``3`` - ``PERSISTENT``

   For an introduction to the RTI durability model, see
   `Basic QoS, in the RTI Connext DDS Getting Started Guide <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/getting_started_guide/cpp11/intro_qos.html>`__. 
   See also: 
   `Mechanisms for Achieving Information Durability and Persistence, in the RTI Connext DDS Core Libraries User’s Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/MechanismsForDurabilityAndPersistence.htm>`__.

   ``PERSISTENT`` is not available when compiling against *Connext DDS Micro*.

-  ``-domain <ID>``

   Domain ID.

   The publisher and subscriber applications must use the same domain ID
   in order to communicate.

   | **Default:** ``1``
   | **Range:** ``0 - 200``

   See 
   `Choosing a Domain ID and Creating Multiple Domains, in the RTI Connext DDS Core Libraries User’s Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/ChoosingDomainID.htm>`__. 


-  ``-help``

   Print an informative message with all the available command-line
   parameters, and exit.

-  ``-instances <int>``

   Set the number of instances to use in the test. The publishing and
   subscribing applications must specify the same number of instances.

   This option only makes sense when testing a keyed data type; to do
   so, also use the ``-keyed`` parameter.

   | **Default:** ``1``
   | **Range:** ``> 0``

-  ``-keyed``

   Specify the use of a keyed type.

   **Default:** ``Unkeyed`` type

-  ``-multicast``

   Use multicast to receive data. In addition, the *Datawriter* heartbeats
   will be sent using multicast instead of unicast.

   The following default multicast addresses will be used for each of the topics::

   | **latency:** ``239.255.1.2``
   | **throughput:** ``239.255.1.1``
   | **announcement:** ``239.255.1.100``

   See ``-multicastAddr <address>`` for how to change these IP addresses.

   **Default:** Do not use multicast

-  ``-multicastAddr <address>``

   Enable the use of multicast. In addition, the *Datawriter* heartbeats
   will be sent using multicast instead of unicast.

   The <address> will be used by the three *Perftest* topics **Latency**,
   **Throughput**, and **Announcement**.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** Do not use multicast

-  ``-noDirectCommunication``

   Test how fast communication is if you use *Persistence Service* 
   to send samples.

   Only applies when ``-durability <0|1|2|3>`` is ``TRANSIENT (2)`` or
   ``PERSISTENT (3)``.

   "Direct communication" means that the *Perftest* publishing application 
   sends samples directly to the *Perftest* subscribing application, regardless 
   of whether a *Persistence Service* instance is found. Direct communication is 
   the default mode, providing low latency between endpoints.

   If ``-noDirectCommunication`` is enabled, the subscribing application only 
   gets samples from *Persistence Service*. This brokered communication pattern
   provides a way to guarantee eventual consistency.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** Not enabled (use direct communication)

-  ``-noPositiveAcks``

   Disable use of positive ACKs in the reliable protocol.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** Not enabled (use positive ACKs)

   See the ``-qosFile <filename>`` option for more information.

-  ``-noPrintIntervals``

   Prevent printing of statistics at intervals during the test.

   By default, statistics are printed every second in the subscribing
   application and after receiving every latency echo in the publishing
   application.

-  ``-outputFile <filename>``

   Output the data to a file instead of printing it on the screen. Summary information
   will still be displayed in the console. If the file cannot be opened or created,
   an error message will be printed and the test will end.

   Default: ``stdout`` (Output is displayed on the terminal)

-  ``-qosFile <filename>``

   Path to the XML file containing DDS QoS profiles.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** ``perftest_qos_profiles.xml``

   The default file contains these QoS profiles:
   ``ThroughputQos``, ``LatencyQos``, and ``AnnouncementQos``,
   which are used by default.

   .. note::

       Some QoS values are ‘hard-coded’ in the application.
       Therefore, setting them in the XML file has no effect.
       See comments in ``perftest_qos_profiles.xml``, as well as in 
       `Configuring QoS with XML, in the RTI Connext DDS Core Libraries User’s Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/XMLConfiguration.htm>`__. 

-  ``-qosLibrary <library name>``

   Name of the QoS Library for DDS Qos profiles.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** ``PerftestQosLibrary``

-  ``-noXmlQos``

   Avoid loading the QoS from the XML profile. Instead, load the QoS
   from a string in code. If you use this option, changes in the XML profile 
   will not be used. This parameter is recommended for operating systems without
   a file-system.

   This parameter is not available when compiling against *Connext DDS Micro*.

   .. note::

       This option is only present in the Traditional and Modern C++ API implementations.

-  ``-useReadThread``

   Use a separate thread (instead of a callback) to read data. 
   See :ref:`WaitSet Event Count and Delay`.

   **Default:** false (use callback for subscriber)

-  ``-waitsetDelayUsec <usec>``

   Process incoming data in groups, based on time, rather than
   individually.

   Increasing this value may result in better throughput results. Decreasing
   this value to its minimum, 0, means that there is no delay: the waitset
   wakes up as soon as you receive the event. In practice, a 0 value typically
   means that the receive thread processes samples individually,
   improving latency results (decreasing latency).

   Only used if ``-useReadThread`` is specified on the
   subscriber side.

   See :ref:`WaitSet Event Count and Delay`.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** ``100``
   | **Range:** ``>= 0``

-  ``-waitsetEventCount <count>``

   Process incoming data in groups, based on the number of samples,
   rather than individually.

   Increasing this value may result in better throughput results. Decreasing
   this value to 1 means that events (new arrived samples in this case) are processed
   individually rather than in batches, improving the latency results
   (decreasing latency).

   Only used if ``-useReadThread`` is specified on the
   subscriber side.

   See :ref:`WaitSet Event Count and Delay`.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** ``5``
   | **Range:** ``>= 1``

-  ``-asynchronous``

   Enable asynchronous publishing in the *DataWriter* QoS.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** Not set

-  ``-flowController <flow>``

   Specify the name of the flow controller that will be used by the
   *DataWriters*. This parameter will only have effect if the *DataWriter* uses
   Asynchronous Publishing, either because it is using samples greater
   than the maximum synchronous size in bytes or because the ``-asynchronous``
   option is present.

   There are several flow controllers predefined: ['default','10Gbps','1Gbps'].

   This parameter is not available when compiling against *Connext DDS
   Micro*; in this case, *RTI Perftest* will use the default FlowController.

   | **Default:** ``default``
   | **Values:** ``['default','10Gbps','1Gbps']``

-  ``-cpu``

   Display the ``cpu`` used by the *RTI Perftest* process.

   **Default:** Not set

-  ``-unbounded <allocation_threshold>``

   Use unbounded sequences in the data type of the IDL.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** ``2 * dataLen up to 1 MB``
   **Range:** ``28 B - 1 MB``

-  ``-peer <address>|<address>[:<id>]``

   Adds a peer to the peer host address list. If ``-rawTransport`` is used, 
   you can provide an optional subscriber ID (<address>[:<id>]) or a range of subscriber IDs
   for that specific address (<address>[:<first_id>-<last_id>]). This argument may be repeated to 
   indicate multiple peers. For example: -peer 1.1.1.1 -peer 2.2.2.2 -peer 3.3.3.3.

   **Default:**
   Not set. *RTI Perftest* will use the default initial peers (localhost, shared-memory, and multicast).

-  ``-threadPriorities X:Y:Z``

   This command-line parameter is supported only for the Traditional C++ and
   Modern C++ API implementations. It sets the priorities for the application threads:

    - **X** for the main thread, which is the one sending the data, or 
      for the asynchronous publishing thread if that one is used.
    - **Y** for the receive threads created by *Connext DDS* or, if ``-useReadThread`` is used, for
      the *Perftest* thread that is created to receive and process data.
    - **Z** for the rest of the threads created by *Connext DDS*: event and database threads.

   This parameter accepts either three numeric values (whichever numeric values you choose) 
   representing the priority of each of the threads or three characters representing 
   the priorities. These characters are h (high), n (normal), and l (low). 

   To see what values can be used for the different threads, see the 
   following tables in the
   `RTI Connext DDS Core Libraries Platform Notes <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/platform_notes/index.htm>`__:

    - "Thread-Priority Definitions for Linux Platforms" table
    - "Thread-Priority Definitions for macOS Platforms" table
    - "Thread-Priority Definitions for Windows Platforms" table
    - "Thread-Priority Definitions for QNX Platforms" table

   This parameter is not available when compiling against *RTI Connext DDS
   Micro* or when using the Raw Transport feature.

   **Default:**
   Not set. The priority will not be modified.

-  ``-cacheStats``

   Enable extra messages showing the reader/writer queue sample count and
   sample count peak.

   The Publisher side also shows the writer's *Pulled Sample Count*.

   The frequency of these log messages will be determined by the
   ``-latencyCount`` on the Publisher side, since the message is only shown
   after a *latency ping*. On the Subscriber side, the message will be shown once
   every second.

   This option is available only for the Traditional C++ API implementation.

   **Default:** Not enabled

-  ``-showResourceLimits``

   Show the resource limits for all different readers and writers.

   This option is available only for the Traditional and Modern C++
   API implementations.

   **Default:** Not enabled

-  ``-outputFormat <format>``

   Specify the format for the printed data to facilitate its display or to export it.

   The following formats are supported:

   ['csv','json','legacy'].

   | **Default:** ``csv``
   | **Values:** ``['csv','json','legacy']``

-  ``-noOutputHeaders``

   Skip the print of the header rows for the *RTI Perftest* output.

   By default, all header rows are printed for each interval and summary.

   | **Default:** Not enabled

-  ``-compressionId``

   Set the compression algorithm to be used.
   By default, compression is disabled.
   If batching is enabled, only `ZLIB` is supported.
   For both latency and throughput tests, the compression setting must be
   provided to both Publisher and Subscriber to have accurate results.

   This feature is only available for *RTI Connext DDS Professional 6.1.0* and
   above, in the Traditional C++ API implementation.

   | **Default:** ``MASK_NONE``
   | **Values:** ``['ZLIB','LZ4','BZIP2']``

- ``-compressionLevel``

   Set the compression level. The value 1 represents the fastest compression
   time and the lowest compression ratio. The value 10 represents the slowest
   compression time but the highest compression ratio. A value of 0 disables
   compression.

   This feature is only available for *RTI Connext DDS Professional 6.1.0* and
   above, in the Traditional C++ API implementation.

   | **Default:** ``10``

- ``-compressionThreshold``

   Set the compression threshold. This is the threshold, in bytes, above which a
   serialized sample will be eligible to be compressed.
   The default value is 0, so if compression has been enabled, all the samples
   will be compressed.

   This feature is only available for *RTI Connext DDS Professional 6.1.0* and
   above, excluding C#.

   | **Default:** ``0``

- ``-networkCapture``

   Enable the *RTI Connext DDS Professional* network capture feature
   during the test.

   This feature is only available for *RTI Connext DDS Professional 6.1.0*
   and above, in the Traditional C++ API implementation.

   | **Default:** ``Not enabled``

- ``-doNotDropNetworkCapture``

   Do not drop the capture file generated at the end of the test, if the
   ``-networkCapture`` feature is in use.

   This feature is only available for *RTI Connext DDS Professional 6.1.0*
   and above, in the Traditional C++ API implementation.

   | **Default:** Not set: *RTI Perftest* will delete the file

- ``-loaningSendReceive``

   Only available when compiling for *RTI Connext TSS*.

   Make use of the underlying DDS loan function to avoid copying the sample
   at the application level.

   | **Default:** Not set: Perftest will avoid using underlying DDS functions by default.


Transport-Specific Options
--------------------------

By default, *RTI Perftest* will try to use the transport settings provided via the
XML configuration file. However, it is possible to override these values directly
by using the transport-specific command-line parameters.

-  ``-transport <TRANSPORT NAME>``

   Set the transport to be used. The rest of the transports will be disabled.

   | **Options in Connext DDS Professional:** ``UDPv4``, ``UDPv6``, ``SHMEM``, ``TCP``, ``TLS``, ``DTLS``, ``WAN`` and ``UDPv4_WAN``
   | **Default in Connext DDS Professional:** Transport defined in the XML profile (``UDPv4`` and ``SHMEM`` if the XML profile is not changed)

   | **Options in Connext DDS Micro:** ``UDPv4``, ``SHMEM``
   | **Default in Connext DDS Micro:** ``UDPv4``

   | **Options for Raw Transport:** ``UDPv4``, ``SHMEM``
   | **Default for Raw Transport:** ``UDPv4``

-  ``-allowInterfaces <ipaddr> / -nic <ipaddr>``

   Restrict *Connext DDS* to sending output through this interface.
   The value should be the IP address assigned to any of the available network
   interfaces on the machine. On Windows systems, use the name of the
   interface. This command-line parameter is mapped to the **allow_interfaces_list**
   property in *Connext DDS*.

   By default, *Connext DDS* will attempt to contact all possible
   subscribing nodes on all available network interfaces. Even on a
   multi-NIC machine, the performance over one NIC vs. another may be
   different (e.g., Gbit vs. 100 Mbit), so choosing the correct NIC is
   critical for a proper test.

   When compiling against *Connext DDS Micro*, this option should always use
   the name of the interface, not the IP address (which is valid when compiling
   against *Connext DDS Professional*).

   .. note::

       Only one NIC can be specified in the C# API implementation.

-  ``-transportVerbosity <level>``

   Verbosity of the transport plugin.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** ``0`` (errors only)

-  ``-transportServerBindPort <port>``

   For TCP and TLS. Port used by the transport to accept TCP/TLS connections.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** ``7400``

-  ``-transportWan``

   For TCP and TLS. Use TCP across LANs and firewalls.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** Not set (LAN mode)

-  ``-transportPublicAddress <ipaddr>``

   For TCP and TLS: public IP address and port (WAN address and port)
   associated with the transport instantiation. Format is 
   ``<public_ip>:<public_port>``.

   For UDPv4_WAN: public address of the UDPv4_WAN transport instantiation. Format is
   ``<public_ip>:<public_port>``.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** Not set

-  ``-transportHostPort <port>``

   For UDPv4_WAN. Internal host port
   associated with the transport instantiation.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** public port specified as part of ``-transportPublicAddress``
  

-  ``-transportWanServerAddress <ipaddr>``

   For WAN transport. Address where to find the WAN server.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** Not set

-  ``-transportWanServerPort <ipaddr>``

   For WAN transport. Port where to find the WAN server.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** ``Not set``

-  ``-transportWanId <id>``

   For WAN transport. ID to be used for the WAN transport. Required when using WAN.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** Not set

-  ``-transportSecureWan``

   For WAN transport. Use DTLS security over WAN.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** Not set

-  ``-transportCertAuthority <file>``

   For TLS, DTLS, and Secure WAN. Certificate authority file to be used by TLS.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default for Publisher:** ``./resource/secure/pub.pem``
   | **Default for Subscriber:** ``./resource/secure/sub.pem``

-  ``-transportCertFile <file>``

   For TLS, DTLS, and Secure WAN. Certificate file to be used by TLS.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default:** ``./resource/secure/cacert.pem``

-  ``-transportPrivateKey <file>``

   For TLS, DTLS, and Secure WAN. Private key file to be used by TLS.

   This parameter is not available when compiling against *Connext DDS Micro*.

   | **Default for Publisher:** ``./resource/secure/pubkey.pem``
   | **Default for Subscriber:** ``./resource/secure/subkey.pem``

.. _Test Parameters only for Publishing Applications:

Test Parameters only for Publishing Applications
------------------------------------------------

-  ``-batchSize <bytes>``

   Enable batching and set the maximum batched message size.
   Disabled automatically if using large data.

   | **Default:** ``0`` (batching disabled)
   | **Range:** ``1`` to maximum synchronous size

   For more information on batching data for high throughput, see the
   **High Throughput for Streaming Data** design pattern in the *RTI Connext DDS Core
   Libraries Getting Started Guide*. See also: **How to Measure Latency
   for a Given Throughput** and the **BATCH QosPolicy** section in
   the *RTI Connext DDS Core Libraries User’s Manual*.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

-  ``-enableAutoThrottle``

   Enable the Auto Throttling feature. See :ref:`Auto-tuning and turbo mode`.

   This parameter is not available when compiling against *RTI Connext DDS
   Micro*.

   **Default:** feature is disabled.

-  ``-enableTurboMode``

   Enables the Turbo Mode feature. See :ref:`Auto-tuning and turbo mode`.
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
   **Range:** must be <= ``-numIter``

-  ``-latencyTest``

   Run a latency test consisting of a ping-pong. The publisher sends a ping,
   then blocks until it receives a pong from the subscriber.

   Can only be used on a publisher whose ``pidMultiPubTest`` = ``0`` (see
   ``-pidMultiPubTest <id>``).

   **Default:** ``false``

-  ``-lowResolutionClock``

   Enables measurement of latency for systems in which the
   clock resolution is not good enough and the measurements per samples are
   not accurate.

   If the machine where *RTI Perftest* is being executed has a low resolution
   clock, the regular logic might not report accurate latency numbers. Therefore,
   *RTI Perftest* implements a simple solution to get a rough estimate of the
   latency:

   Before sending the first sample, *RTI Perftest* records the time; right after
   receiving the last pong, the time is recorded again. Under the assumption that
   the processing time is negligible, the average latency is calculated as half of
   the time taken divided by the number of samples sent.

   This calculation only makes sense if latencyCount = 1 (Latency Test), since
   it assumes that every single ping is answered.

   **Default:** Not set

   .. note::

       Not available in the C# API implementation.

-  ``-numIter <count>``

   Number of samples to send. See :ref:`Number of Iterations vs. Latency Count` and
   :ref:`Warming Up`. If you set ``scan`` = ``true``, you cannot set this option
   (see ``-scan``).

   | **Default:** ``100000000`` for throughput tests or ``10000000``
                   for latency tests (when ``-latencyTest`` is specified);
                   see also ``-executionTime``
   | **Range:** ``latencyCount`` (adjusted value) or higher (see
     ``-latencyCount <count>``)

-  ``-numSubscribers <count>``

   Have the publishing application wait for this number of subscribing
   applications to start.

   **Default:** ``1``

-  ``-pidMultiPubTest <id>``

   Set the ID of the publisher in a multi-publisher test.

   Use a unique value for each publisher running on the same host that
   uses the same domain ID.

   | **Default:** ``0``
   | **Range:** ``0`` to ``n-1``, inclusive, where n is the number of
     publishers in a multi-publisher test

-  ``-pub``

   Set test to be a publisher.

   **Default:** ``-sub``

-  ``-pubRate <sample/s>:<method>``

   Limit the throughput to the specified number of samples per second.
   The method to control the throughput rate can be: 'spin' or 'sleep'.

   If the method selected is 'sleep', *RTI Perftest* will control the rate
   by calling the **sleep()** function between writing samples. If the
   method selected is 'spin', *RTI Perftest* will control the rate by
   calling the **spin()** function (active wait) between writing samples.

   .. note::

      The resolution provided by using 'spin' is generally better
      than the 'sleep' one, especially for fast sending rates (where the
      time needed between sending samples is very small). However,
      'spin' will also result in a higher CPU consumption.

   | **Default samples:** ``0`` (no limit)
   | **Range samples:** ``1`` to ``10000000``

   | **Default method:** ``spin``
   | **Values method:** ``spin or sleep``

-  ``-scan <size1>:<size2>:...:<sizeN>``

   .. note::

      This command-line option is deprecated and will not be available in future versions
      of *RTI Perftest*.

   Run test in scan mode. The list of sizes is optional and can be either in the
   [32,64900] range or in the [64970,2147482620] range (Large Data cannot be tested
   in the same scan test as small data sizes). Default values to test with are
   '32:64:128:256:512:1024:2048:4096:8192:16384:32768:64900'.
   
   The ``-executionTime`` parameter is applied for every size of the scan.
   If ``-executionTime`` is not set, a timeout of 60 seconds will be applied.

   **Default:** ``false`` (no scan)

   .. note::

       Not available in the C# API implementation.

-  ``-sendQueueSize <number>``

   Size of the send queue.

   When ``-batchSize <bytes>`` is used, the size is the number of
   batches.

   See :ref:`Send-queue size and queue-full behavior`.

   | **Default:** ``50``
   | **Range:** ``[1-100 million]`` or ``-1`` (indicating an unlimited
     length)

-  ``-initialBurstSize <number>``

   Set the size of the initial burst of samples sent from the Publisher side to
   the Subscriber side. These samples are all marked as latency samples, and
   they are answered back by the Subscriber side.

   The use of this initial burst is to ensure all the queues are initialized and
   no time is lost in the initialization process when measuring the performance.

   This parameter is only available for the Traditional and Modern C++ API
   implementations.

   | **Default:** Calculated by the *RTI Perftest*
   | **Range:** ``[0 - Max Long Size]``

-  ``-sleep <millisec>``

   Time to sleep between each send.

   See :ref:`Publication rate and spinning vs. sleeping`.

   | **Default:** ``0``
   | **Range:** ``0`` or higher

-  ``-writerStats``

   Enable extra messages showing the writer's Pulled Sample Count
   on the Publisher side. The frequency of these log messages will be
   determined by the ``-latencyCount``, since the message is only shown
   after a *latency ping*.

   **Default:** Not enabled

-  ``-writeInstance <instance>``

   Set the instance number to be sent.

   | **Default:** Round-Robin schedule
   | **Range:** ``0 and instances``

-  ``-showSerializationTime``

   Show serialization/deserialization times for the sample size(s) of the test.
   This time will be shown after the test concludes.
   This command-line parameter is only present in the Traditional C++ API implementation.

   **Default:** Not enabled

-  ``-loadDataFromFile <filePath>``

   Use this option to send data from a file. By default, *RTI Perftest* will use
   an empty (newly created) `char` array for the payload of each of the samples.

   If this option is in use, the content of the provided file will be loaded
   into memory (up to a configurable maximum size, see `-maximumAllocableBufferSize`),
   and divided into buffers of `-datalen` size. If the `-datalen` command line
   is not provided, *RTI Perftest* will set this option to the file size.

   **Default:** Not enabled

   .. note::

       Only available in the Tradditional C++ API implementation.

-  ``-maximumAllocableBufferSize <bytes>``

   When ``-loadDataFromFile`` is in use, this option controls the maximum
   amount of memory that can be used to load the file into memory.

   **Default:** ``1`` GB

   .. note::

       Not available in the C# API implementation.


.. _Test Parameters only for Subscribing Applications:

Test Parameters only for Subscribing Applications
-------------------------------------------------

-  ``-numPublishers <count>``

   The subscribing application will wait for this number of publishing
   applications to start.

   **Default:** ``1``

-  ``-sidMultiSubTest <id>``

   ID of the subscriber in a multi-subscriber test. Use a unique value for each
   subscriber running on the same host that uses the same domain ID.

   | **Default:** ``0``
   | **Range:** ``0`` to n-1, inclusive, where n is the number of
     subscribers in a multi-subscriber test

-  ``-sub``

   Set test to be a subscriber.

   **Default:** ``-sub``

-  ``-cft <start>:<end>``

   Use a ContentFilteredTopic for the Throughput topic on the
   subscriber side. Specify two parameters to receive samples with a
   key in that range. Specify one parameter to receive samples with
   that exact key.

   This parameter is not available when compiling against *Connext DDS
   Micro*.

   **Default:** Not set

-  ``-checkConsistency``

   Check the consistency of samples sent with Zero Copy transfer over shared
   memory.

   The Publisher may be reusing memory to send different samples before the
   original samples are processed by the subscriber, leading to inconsistent samples.
   Inconsistent samples will be reported as lost.

   See information about checking data consistency in
   `Using Zero Copy Transfer Over Shared Memory, in the RTI Connext DDS Core Libraries User’s Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/SendingLDZeroCopyUsing.htm>`__.

   This parameter can only be used along with ``-zeroCopy``.

   This parameter is not available when compiling against *Connext DDS
   Micro* or a *Connext DDS* release before 6.0.0.

   **Default:** Not set

   .. note::

       Not available in the C# API implementation.


.. _Test Parameters to Control Connext DDS Secure Options:

Test Parameters to Control Connext DDS Secure Options
-----------------------------------------------------

-  ``-secureGovernanceFile <file>``

   Governance file. If specified, the authentication, signing, and
   encryption arguments are ignored. The governance document
   configuration will be used instead.

   **Default:** Not set

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

  | **Default for Publisher:** ``./resource/secure/pubkey.pem`` 
  | **Default for Subscriber:** ``./resource/secure/subkey.pem``

-  ``-secureEncryptionAlgorithm <Algorithm>``

   Set the Security Encryption Algorithm.

  | **Default:** ``aes-128-gcm``

-  ``-secureEnableAAD``

   Enable Additional Authenticated Data when using Security.

  | **Default:** Not Enabled.


Raw Transport Options
---------------------
Raw Transport is a *Perftest* feature. It is the ability to use direct socket
or shared memory segments so that you can compare these with *Connext DDS*
UDPv4 and shared memory communications.

.. note::

    These options are only available in the Tradditional C++ API
    implementation.

-  ``-rawTransport``

   Use sockets as a transport instead of a DDS protocol. This option supports
   ``UDPv4`` and shared memory (``SHMEM``).
   Some of the *Connext DDS* parameters are not supported when using sockets.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** Not set

-  ``-noBlockingSockets``

   Control blocking behavior of send sockets to never block.
   CHANGING THIS FROM THE DEFAULT CAN CAUSE SIGNIFICANT PERFORMANCE PROBLEMS.

   This parameter is not available when compiling against *Connext DDS Micro*.

   **Default:** Not set. Always block.

Additional Information about Parameters
---------------------------------------

Secure certificates, Governance files, and Permissions files
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*RTI Perftest* provides a set of already generated certificates,
governance files, and permission files to be loaded when using the *RTI Security
Plugins* libraries. Both Governance and Permissions files are already
signed, so no action is required by the user. These files are located in
``$(RTIPERFTESTHOME)/resource/secure``.

In addition to the already signed Governance and Permissions files, the
original files are also provided (not signed) as well as a ``bash``
script with the steps to generate all the signed files. Those files can
be found in ``$(RTIPERFTESTHOME)/resource/secure/input``; the script is
in ``$(RTIPERFTESTHOME)/resource/secure/make.sh``.

.. _Publication rate and spinning vs. sleeping:

Publication rate and spinning vs. sleeping
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When the publisher is writing as fast as it can, sooner or later, it is
likely to get ahead of the subscriber. There are a few things you can do in
this case:

-  Nothing—for reliable communication, ``write()`` will block until
   the subscriber(s) catch up.

-  Slow the writing down by sleeping (see ``-sleep <millisec>``). This
   approach is friendlier to the other processes on the host because it
   does not monopolize the CPU. However, context switching is expensive
   enough that you can't actually "sleep" for amounts of time on the
   order of microseconds, so you could end up sleeping too long and
   hurting performance. Note that operating systems, including Linux and Windows,
   have a minimum resolution for sleeping, such as 
   1 or 10 ms. If you specify a sleep period less
   than the minimum, the OS may sleep for its minimum resolution.

-  Set a publication rate (see ``-pubRate <count>:<method>``). This approach
   will make *Perftest* automatically set the rate of the write call so
   you can get the number of samples per second requested (if possible).
   This option allows you to choose ``sleep()`` between calls or ``spin()``.
   The second approach will add a pause without yielding the CPU to other
   processes, making it easier to "sleep" for very short periods of time. 
   Avoid spinning on a single-core machine, since the code that would break 
   you out of the spin may not be able to execute in a timely manner.

-  Let the publisher automatically adjust the writing rate (see
   ``-enableAutoThrottle``). This option enables the Auto Throttle
   feature (introduced in *Connext DDS* 5.1.0), and its usage is
   preferred over ``-spin <count>`` because the amount of spin is
   automatically determined by the publisher based on the number of
   unacknowledged samples in the send queue.

See also: :ref:`Send-Queue Size and Queue-Full Behavior`.

.. _Send-Queue Size and Queue-Full Behavior:

Send-queue size and queue-full behavior
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

-  Enlarge your publisher's queue (see ``-sendQueueSize <number>``).
   Doing so means your publisher has to block less often. However,
   it may also let the publisher get even further ahead of slower
   subscribers, increasing the number of dropped and resent packets,
   hurting throughput. Experimenting with the send queue size is one of
   the easy things you can do to squeeze a little more throughput from
   your system.

-  Enable Auto Throttling (see ``-enableAutoThrottle``). This option
   enables the Auto Throttle feature (introduced in *Connext DDS*
   5.1.0. When this option is used, the publisher automatically adjusts
   the writing rate based on the number of unacknowledged samples in the
   send queue to avoid blocking.

.. note::

   The following values in the ``DataWriterProtocolQosPolicy`` are
   hard-coded in *Perftest*; therefore setting these values in the
   XML QoS profile will have no effect:

   -  ``rtps_reliable_writer.heartbeats_per_max_samples`` is set to
      (``sendQueueSize/10``)
   -  ``rtps_reliable_writer.low_watermark`` is set to
      (``sendQueueSize * 0.10``)
   -  ``rtps_reliable_writer.high_watermark`` is set to
      (``sendQueueSize * 0.90``)

For more information on the send queue size, see information about the
``max_samples`` field in
`RESOURCE_LIMITS QosPolicy, in the RTI Connext DDS Core Libraries User's Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/RESOURCE_LIMITS_QosPolicy.htm>`__.


.. _Number of Iterations vs. Latency Count:

Number of iterations vs. latency count
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When configuring the total number of samples to send during the test
(see ``-numIter <count>``) and the number of samples to send between
latency pings (see ``-latencyCount <count>``), keep these things in
mind:

-  Don't send latency pings too often. One of the purposes of the test
   is to measure the throughput that the middleware is able to achieve.
   Although the total throughput is technically the total data sent on
   both the Throughput and Latency topics, for the sake of simplicity,
   the test measures only the former. The implicit assumption is that
   the latter is negligible by comparison. If you violate this
   assumption, your throughput test results will not be meaningful.

-  Keep the number of iterations large enough to send many latency pings
   over the course of the test run. Your latency measurements, and the
   spread between them, will be of higher quality if you are able to
   measure more data points.

-  When setting ``-numIter <count>``, choose a value that allows
   the test to run for at least a minute to get accurate results. Set
   ``-numIter <count>`` to be millions for small message sizes
   (<1k); reduce as needed for larger sizes (otherwise the tests will
   take longer and longer to complete).

.. _Warming Up:

Warming up
~~~~~~~~~~

When running the performance test in Java and, to a lesser extent,
C#, you may observe that throughput slowly increases through the first
few incremental measurements and then levels off. This improvement
reflects the background activity of the just-in-time (JIT) compiler and
optimizer on these platforms. For the best indication of steady-state
performance, be sure to run the test for a number of samples (see
``-numIter <count>``) sufficient to smooth out this start-up artifact.

.. _WaitSet Event Count and Delay:

WaitSet event count and delay
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*Perftest*, like *Connext DDS*, gives you
the option to either process received data in the middleware's receive
thread, via a listener callback, or to process the data in a separate thread 
(see ``-useReadThread``) via an object called `Waitset`. The latter approach
can be beneficial in that it decouples the operation of your application
from the middleware, so that your processing will not interfere with
*Connext DDS*'s internal activities. However, it does introduce
additional context switches into your data receive path. When data is
arriving at a high rate, these context switches can adversely affect
performance when they occur with each data sample.

To improve efficiency, the command-line parameters
``-waitsetDelayUsec <usec>`` and ``-waitsetEventCount <count>`` allow
you to process incoming data in groups, based on the number of samples
and/or time, rather than individually, reducing the number of context
switches. Experiment with these values to optimize performance for your
system. Increasing these values may result in greater throughput.

Nonetheless, as explained in the documentation for each of these parameters,
in order to achieve better (smaller) latency results, an approach where we set
``-waitsetDelayUsec 0`` and ``-waitsetEventCount 1`` is recommended.

For more information, see
`Receive Threads, in the RTI Connext DDS Core Libraries User's Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/Receive_Threads.htm>`__
and 
`Conditions and WaitSets, in the RTI Connext DDS Core Libraries User's Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/Conditions_and_WaitSets.htm>`__.


.. _How to Measure Latency for a Given Throughput:

How to measure latency for a given throughput
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If you want to measure the minimum latency for a given throughput, you
have to use the command-line parameters ``-sleep <millisec>``,
``-spin <count>``, and ``-batchSize <bytes>`` to experimentally set the
throughput level for a given test run.

For example, suppose you want to generate a graph of latency vs.
throughput for a packet size of ``200 bytes`` and throughput rates of
``1000``, ``10K``, ``20K``, ``50K``, ``100K``, ``500K``, and
maximum messages per second.

For throughput rates under 1000 messages per second, use ``-sleep <ms>``
to throttle the publishing application. For example, ``-sleep 1`` will
produce a throughput of approximately 1000 messages/second; ``-sleep 2``
will produce a throughput of approximately 500 messages/second.

For throughput rates higher than 1000 messages per second, use
``-spin <spin count>`` to cause the publishing application to busy-wait
between sends. The ``<spin count>`` value needed to produce a given
throughput must be experimentally determined and is highly dependent on
processor performance. For example, ``-spin 19000`` may produce a message
rate of 10000 messages/second with a slow processor but a rate of 14000
messages/second with a faster processor.

Use batching when you want to measure latency for throughput rates
higher than the maximum rates of sending individual messages. First,
determine the maximum throughput rate for the data size under test
without batching (omit ``-batchSize <bytes>``). For example, on a
1-Gigabyte network, for a data size of ``200 bytes``, the maximum
throughput will be about 70,000 messages/sec. We will refer to this
value as ``max_no_batch``.

For all throughput rates less than ``max_no_batch`` (e.g., 70,000
messages/sec.), do not use batching, since this will increase the latency.

Use batching to test for throughput rates higher than ``max_no_batch``:
start by setting ``-batchSize`` to a multiple of the data size. For
example, if the data size is ``200 bytes``, use ``-batchSize 400`` (this
will put two messages in each batch), ``-batchSize 800`` (four messages per batch),
and so on. This will allow you to get throughput/latency results for
throughputs higher than the ``max_no_batch`` throughput rate.

.. note::

    For larger data sizes (``8000 bytes`` and higher), batching
    often does not improve throughput, at least not for 1-Gigabyte networks.

.. _Auto-tuning and turbo mode:

Auto-tuning and turbo mode
~~~~~~~~~~~~~~~~~~~~~~~~~~

Since release 5.1.0, *Connext DDS* includes two features that allow the middleware
to auto-tune communications to achieve better performance. These
features are Auto Throttling and Turbo Mode. For more
information, see
`Auto Throttling for DataWriter Performance—Experimental Feature, in the RTI Connext DDS Core Libraries User's Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/Auto_Throttling.htm>`__
and the section "Turbo Mode: Automatically Adjusting the Number of Bytes in a Batch—Experimental," in
`BATCH QosPolicy (DDS Extension), in the RTI Connext DDS Core Libraries User's Manual <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/users_manual/index.htm#users_manual/BATCH_Qos.htm>`__.
The performance test application includes two
command-line options to enable these features: ``-enableAutoThrottle``
and ``-enableTurboMode``.

With Auto Throttling, the publisher automatically adjusts the writing
rate based on the number of unacknowledged samples in the send queue to
avoid blocking and provide the best latency/throughput tradeoff.

With Turbo Mode, the size of a batch is automatically adjusted to
provide the best latency for a given write rate. For slow write rates,
the batch size will be smaller to minimize the latency penalty. For high
write rates, the batch size will be bigger to increase throughput. When
Turbo Mode is used, the command-line option ``-batchSize <bytes>``
is ignored.

To achieve the best latency under maximum throughput conditions, use
``-enableAutoThrottle`` and ``-enableTurboMode`` in combination.

.. _section-not_available_params_tss:

Perftest parameters not available when using Connext TSS
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following parameters are not available when using *Perftest* with *TSS*:

-  ``-dynamicData``

-  ``-flatData``

-  ``-zeroCopy``

-  ``-checkConsistency``

-  ``-preallocateFragmentedSamples``

-  ``-flowController <flow>``

-  ``-unbounded``

-  ``-peer``

-  ``-threadPriorities``

-  ``-cacheStats``

-  ``-compressionId``

- ``-compressionLevel``

- ``-compressionThreshold``

- ``-networkCapture``

- ``-doNotDropNetworkCapture``

-  ``-transportWan``

-  ``-transportPublicAddress``

-  ``-transportHostPort``

-  ``-transportWanServerAddress``

-  ``-transportWanServerPort``

-  ``-transportWanId``

-  ``-transportSecureWan``

-  ``-transportCertAuthority``

-  ``-transportCertFile``

-  ``-transportPrivateKey``

-  ``-writerStats``

-  ``-showSerializationTime``

-  ``-cft``

-  ``-crc``

-  ``-crcKind``

-  ``-enable-header-extension``

-  ``-checkConsistency``

-  ``-secureGovernanceFile <file>``

-  ``-securePermissionsFile <file>``

-  ``-secureCertAuthority <file>``

-  ``-secureCertFile``

-  ``-securePrivateKey``

-  ``-secureEncryptionAlgorithm``

-  ``-secureEnableAAD``

-  ``-securePSK``

-  ``-securePSKAlgorithm``

-  ``-lightWeightSecurity``

-  ``-rawTransport``

-  ``-noBlockingSockets``

Please keep in mind that *RTI Connext TSS* uses *RTI Connext Pro* or
*RTI Connext Micro* underneath, so any argument that's not available for Pro,
won't be available for TSS + Pro, and the same applies to Micro and TSS + Micro.