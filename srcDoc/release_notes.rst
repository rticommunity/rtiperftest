.. _section-release_notes:

Release Notes
=============

Compatibility Master
--------------------

Using Security
~~~~~~~~~~~~~~

Governance and Permission files have been updated to be compatible with
the latest release for *RTI Connext DDS*, and are compatible with *RTI
Connext DDS* 5.2.7 and greater.

If you are compiling *RTI Perftest* against 5.2.5, you will need to get
the certificates from the ``release/2.0`` branch. You can do that by
using the following git command from the top-level directory of your
repository:

::

    git checkout release/2.0 -- resource/secure

Compilation Restrictions
~~~~~~~~~~~~~~~~~~~~~~~~

*RTI Perftest* is designed to compile and work against the *RTI Connext
DDS* 5.2.x and 5.3.x releases.

However, certain features are not compatible with all the *RTI Connext
DDS* versions, since the build scripts make use of certain specific
parameters in *Rtiddsgen* that might change or not be present between
releases:

-  The ``--secure`` and ``--openssl-home`` parameters will not work for
   versions prior to *RTI Connext DDS* 5.2.5.

-  Java code generation against *RTI Connext DDS 5.2.0.x* will fail out
   of the box. You can disable this by adding the ``--skip-java-build``
   flag. See the Known issues section for more information and
   alternatives.

-  C# code generation against *RTI Connext DDS 5.2.0.x* is not
   supported. You can disable this by adding the ``--skip-cs-build``
   flag.

Release Notes Master
--------------------

What's New in Master
~~~~~~~~~~~~~~~~~~~~

Print a summary with the main setting of the test *RTI Perftest* will run (#46)(#67)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest* provides a great number of command-line parameters, plus the option
of using the *xml configuration file* for modying *RTI Connext DDS QoS*. This could
lead to some confusion with regards to the test that will run when executing the application.

In order to make this clear, *RTI Perftest* now shows a summary at the beginning of
the test with most of the relevant parameters being used for such test. This is done
for both *Publisher* and *Subscriber* sides.


Added command-line parameters to simplify single API build (#50)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest Build scripts* now support building a single API using the
following command-line parameters:

    --java-build
    --cpp03-build
    --cpp-build
    --cs-build

Added RTI Perftest and RTI Connext DDS information at beginning of the test (#54)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Starting with this release, RTI Perftest will print at the beginning of the test
its version and the version of RTI Connext DDS used to compile against.

Automatically regenerate `qos_string.h` file if `perftest_qos_profiles.xml` is modified (#63)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Starting with this release, *RTI Perftest* will udpate `qos_string.h` file
with the content of `perftest_qos_profiles.xml` every time that *RTI Perftest*
is built for C++ and C++ New PSM.

What's Fixed in Master
~~~~~~~~~~~~~~~~~~~~~~

Improve Dynamic Data Send() and Receive() operations (#55)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Dynamic Data Send() and Received() functions have been optimized
reducing the time of setting and getting the samples.

The result of these optimizations is that RTI Perftest now minimizes the time
employed in the application-related tasks, therefore maximizing the time for
sending and receiving calls. This allows to do a fair comparison between
Dynamic Data results and Generated Type-Code Data results.

Incorrect Latency maximum calculation in certain scenarios with low resolution clocks (#58)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases, if the clock provided by the system had low resolution, many of the
*Latency* times calculated by sending and receiving back samples would end up being `0us`.
*RTI Perftest* would assume in those cases this value was a initialization value and it
would reset the maximum latency.

This behavior has been fixed.

Improve behavior when using the `-scan` command-line option and Best Effort (#59)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases, the use of `-scan` in combination with *Best Effort* would cause
to send too many times certain packets used to signalize the change of sizes and the
initialization and finalization of the test.

In certain scenarios, mostly local tests where *RTI Perftest* Publishers and Subscribers
were in the same machine and that machine had limitations with respect to the CPU, this
would cause the *Scan* test to not work properly, since the *Publisher* would make use of
the CPU and network intensively, potentially starving the subscriber side and making the
test hang.

This behavior has been fixed.

Reduce memory consumption in the Subscriber side (#74)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The *initial_samples* value for the *ThroughputQoS* QoS profile has been updated
to a lower number. This profile is used by the Subscriber side to create a
*DDS DataWriter*.

The reason why this value has been updated is to decrease the memory consumption
of the *RTI Perftest* Subscriber side.

In order to ensure that this change does not affect to the overall performance of
the application, the initial burst of samples sent by the Publisher side has been
also reviewed, to always send a burst big enough to ensure that the allocations in
both publisher and subscriber sides are done before the test starts.


Release Notes 2.3.2
-------------------

What's Fixed in 2.3.2
~~~~~~~~~~~~~~~~~~~~~~

Classic C++ Semaphore Take() and Give() operations not checking for errors properly (#47)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous versions, the semaphore Take() and Give() operations
were not being checked for error in a correct way in the Classic C++ API implementation.
This has been fixed.

Update Security Certificates and Governance files (#49)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Security Certificates and Governance files used when enabling security options
in RTI Perftest have been regenerated and signed again, since they had expired.

The script used for updating the files has been improved to generate certificates
valid for a longer period of time (from one year to ten years).

Release Notes 2.3.1
--------------------

What's Fixed in 2.3.1
~~~~~~~~~~~~~~~~~~~~~

`Keep Duration` not configurable when using `-noPositiveAcks` (#39)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous versions, if the `-noPositiveAcks` command line parameter was provided
the *Disable Positive Acks Keep Duration* QoS setting would be ignored both when
provided via XML configuration or via command line parameter (deprecated option),
instead, *RTI Perftest* would always use the default value set up via code.

This behavior has been fixed. We also took the oportunity to simplify and clarify
the XML configurations when disabling positive Acks.

Update Security Certificates and Governance files (#49)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The Security Certificates and Governance files used when enabling security options
in RTI Perftest have been regenerated and signed again, since they had expired.

The script used for updating the files has been improved to generate certificates
valid for a longer period of time (from one year to ten years).

Release Notes 2.3.1
--------------------

What's Fixed in 2.3.1
~~~~~~~~~~~~~~~~~~~~~~

Segmentation fault when using multiple publishers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous versions, in scenarios with multiple publishers, every *RTI Perftest*
publisher application with `-pidMultiPubTest` different than 0 would crash in the
process of printing the latency statistics. This behavior has been fixed.

Release Notes 2.3
-----------------

What's New in 2.3
~~~~~~~~~~~~~~~~~

Added Support for DTLS
^^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest* now supports the use of the *DTLS* plugin. The out of the
box configuration allows the application to work using *DTLS* by just specifying
``-transport DTLS``, however we also included command-line parameters to specify:

- The Certificates and the public/private keys.
- The verbosity.

See the *Test Parameters* section for more information about how to configure DTLS.

Added Support for TLS
^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest* now supports the use of *TLS* on top of the *TCP* plugin.
The out of the box configuration allows the application to work using *TLS*
by just specifying ``-transport TLS``, however we also included command-line
parameters to specify:

- The Certificates and the public/private keys.
- The verbosity.
- The Server Bind Port.
- The use of WAN mode.
- The use of a Public Address.

See the *Test Parameters* section for more information about how to configure TLS.

Enhanced TCP Functionalities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

As part of the changes for adding support for *TLS*, more functionalities have
been included for *TCP*, including options to specify:

- The verbosity.
- The Server Bind Port.
- The use of WAN mode.
- The use of a Public Address.

See the *Test Parameters* section for more information about how to configure TCP.

Added Support for WAN
^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest* now supports the use of the *WAN* transport plugin.
In order to use this transport the command-line option ``-transport WAN`` needs
to be specified, we also included command-line parameters to specify:

- The WAN Server Address and Port
- The WAN ID.
- The Certificates and the public/private keys in case of using Secure WAN.
- The verbosity.
- The Server Bind Port.

See the *Test Parameters* section for more information about how to configure WAN.

Default Values for ``Reliability`` and ``Transport`` can be Modified via XML
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Starting with this release, the Reliability and Transport settings are not set
via code for the different languages, but are set in the XML profile. 
This allows you to easily modify these settings without needing to recompile.

These settings can still be modified via command-line parameters.

Added Command-Line Parameter ``-qosLibrary``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Starting with this release, the QoS Library can be selected using the new
``-qosLibrary`` option.

This command-line option, combined with ``-qosFile``, allows you to use custom
QoS profiles that inherit from the default one (``perftest_qos_profiles.xml``).

A simple example is provided here:
``resource/profile_examples/custom_perftest_qos_profiles.xml``.

Changed Name for Command-Line Option from ``-qosProfile`` to ``-qosFile``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Starting with this release, the ``-qosProfile`` command-line parameter has been
changed to ``-qosFile`` to better reflect its use.

Improved ``-scan`` Command-line Parameter Functionality
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
In the previous release, using ``-scan`` caused *RTI Perftest* to execute with 
a predefined set of values for -dataLen, and with execution durations related to 
the number of latency pings. This behavior has been changed. Now ``-scan`` allows 
you to specify a set of -datalen sizes to be used (or you can use the default set). 
In addition, the value specified for the '-executionTime' parameter is now used 
for each execution during the scan, regardless of the number of latency pings.

When using ``-batchSize`` at the same time as ``-scan`` and not using large
data, the same batch size will be applied to all the data sizes being used by
``-scan``.

Deprecated Some Command-Line Parameters
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

To simplify the number of parameters *RTI Perftest* accepts, we reviewed and 
deprecated some parameters. These parameters will still work for this 
release, but they will be deleted or altered for future ones.

-  Deprecated ``-instanceHashBuckets <n>``

The associated value will be the same as the number of instances.

-  Deprecated ``-keepDurationUsec <usec>``

The value will be set in the QoS in the case of using -noPositiveAcks.

-  Combined ``-multicast`` and ``-multicastAddress <address>``.

The resulting command can be used as ``-multicast`` keeping its original behavior
or as ``-multicast <address>``, which will enable multicast and use <address> as
the multicast receive address.

-  Deprecated ``-nomulticast``

The default behavior is to not use multicast, so this command-line option was
redundant.

-  Updated ``-unbounded <managerMemory>`` to ``-unbounded <allocator_threshold>``

Instead of ``managerMemory``, use ``allocator_threshold``, since it better reflects
the use of the value. The new default is ``2 * dataLen`` up to ``63000``.
The associated documentation has also been improved.

-  Deprecated ``-heartbeatPeriod <sec>:<nanosec>`` and
   ``-fastHeartbeatPeriod <sec>:<nanosec>``

These parameters can still be changed via XML.

-  Deprecated ``-spin <count>``

This option made no sense after the -sleep and -pubRate alternatives were implemented.

What's Fixed in 2.3
~~~~~~~~~~~~~~~~~~~

Failure when Using ``-peer`` Command-Line Parameter for C#
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Using the ``-peer`` option in the C# implementation caused 
*RTI Perftest* to fail due to an issue reserving memory. This behavior
has been fixed.

``-nic`` Command-Line Parameter not Working when Using UDPv6 Transport
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``-nic`` command-line parameter was not taken into account when 
using the UDPv6 transport. This behavior has been fixed.


Failure when Using -batchSize or -enableTurboMode if -dataLen Exceeded Async Publishing Threshold
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Using ``-batchSize`` along with a ``-dataLen`` value greater than the asynchronous 
publishing threshold caused the application to show an error and exit. 
Starting with this release, the ``-batchSize`` option will be ignored in this scenario 
(and a warning message displayed). 

This change (ignoring ``-batchSize``) won't be applied if you explicitly set ``-asynchronous``; 
in this case, the behavior will remain the same as before (it will show an error and exit).

This change also applies to the use of ``-enableTurboMode``.

Issues when Finishing Performance Test or Changing Sample Size
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In order to make the mechanism to finish the performance test or change sample sizes
more robust, we now use the ``Announcement`` topic on the Subscriber side to notify
the Publisher side of the arrival of special samples sent to signal a change of sample 
size or to signal that the test is finishing. In previous releases, this process was 
not reliable and may have caused hangs in certain scenarios.

Unreliable Behavior Finishing Tests when Using ContentFilteredTopic (CFT)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases when using CFTs, in order to finish a test, the Publisher
needed to send as many samples signaling that the test is finishing as the
number of instances that were being used by the test (1 sample per instance). 
This could result in a very long process, and in scenarios where the reliability 
was set to BEST_EFFORT, in a higher chance of losing one of those samples, 
making the test hang.

This behavior has been modified by using a specific key for the signaling
messages, so they are not filtered by the CFTs.

Release Notes v2.2
------------------

What's New
~~~~~~~~~~

Added command-line parameters "-asynchronous" and "-flowController ``<``\ flow\ ``>``"
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases Asynchronous Publishing was only enabled for the
DataWriters when the samples were greater than 63000 bytes and in such
case, RTI Perftest would only use a custom flow controller defined for
1Gbps networks.

This behavior has been modified: Starting with this release,
Asynchronous Publishing will be activated if the samples to send are
bigger than 63000 bytes or if the ``-asynchronous`` command-line
parameter is used. In that case, *RTI Perftest* will use the ``Default``
flow controller. However, now you can change this behavior by specifying
the ``-flowController`` option, which allows you to specify if you want
to use the default flow controller, a 1Gbps flow controller, or a 10Gbps
one.

Improved "-pubRate" command-line parameter capabilities
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases the "-pubRate" command-line option would only use
the ``spin`` function to control the publication rate, which could have
negative effects related with high CPU consumption for certain
scenarios. Starting with this release, a new modifier has been added to
this option so it is possible to use the both "spin" and "sleep" as a
way to control the publication rate.

Added command-line parameter to get the CPU consumption of the process
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Starting with this release, it is possible to display the *CPU
consumption* of the *RTI Perftest* process by adding the Command-Line
Parameter ``-cpu``.

Better support for large data samples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Prior to this release, the maximum sample size allowed by *RTI Perftest*
was set to 131072 bytes. The use of bigger sizes would imply changes in
the ``perftest.idl`` file and source code files. Starting with this
release, the maximum data length that *RTI Perftest* allows has
increased to 2,147,483,135 bytes, which corresponds to 2 Gbytes - 512
bytes - 8 bytes, the maximum data length that *RTI Connext DDS* can
send.

The sample size can be set via the ``-dataLen <bytes>`` command-line
parameter. If this value is larger than 63,000 bytes *RTI Perftest* will
enable the use of *Asynchronous Publishing* and *Unbounded Sequences*.

It is also possible to enable the use of *Unbounded Sequences* or
*Asynchronous Publishing* independently of the sample size by specifying
the command-line parameters ``unbounded <allocation_threshold>`` and
``-asynchronous``.

Added command-line parameter "-peer" to specify the discovery peers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases the only way to provide the Initial Peers was
either adding them to the QoS XML file or by using the environment
variable ``NDDS_DISCOVERY_PEERS``. Now it is possible to use a new
command-line parameter: ``-peer <address>`` with the peer address.

Now providing RTI Routing Service configuration files to test performance along with RTI Perftest
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A new configuration file and wrapper script have been added for testing
RTI Perftest using one or several RTI Routing Service applications in
between Publisher and Subscriber. A new section has been added to the
documentation with all the configuration parameters: `Using RTI Perftest
with RTI Routing-Service <routing_service.md>`__.

Changed Announcement QoS profile to use "Transient local" Durability settings
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases, the announcement topic DataWriters and DataReaders
were set to have a ``Volatile`` Durability QoS. In certain complex
scenarios, that could result in incorrect communication, which could
cause the RTI Perftest Publisher and Subscribers to get stuck and not
transmit data. By changing this topic to use Transient Local Durability,
these scenarios are avoided.

This should not have any effect on the latency of throughput reported by
RTI Perftest (as the main Throughput and Latency topics still have the
same configuration).

Added new functionality: Content Filtered Topic.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases the only way to provide scalability was by using
multicast and unicast. Now you can also choose which subscriber will
receive the samples by using the parameter ``-cft``. You can also
determine which sample will be sent by the publisher with the parameter
``-writeInstance``.

What's Fixed
~~~~~~~~~~~~~~~~~~~

Conflicts when using "-multicast" and "-enableSharedMemory" at the same time
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases, using "-multicast" in conjunction with
"-enableSharedMemory" may have caused the middleware to fail while
trying to access multicast resources although it was set to use only
shared memory. This behavior has been fixed.

"-nic" command-line parameter not working when using TCP transport
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases the ``-nic`` command-line parameter was not taken
into account when using the TCP transport. This behavior has been fixed.

Batching disabled when sample size was greater than or equal to batch size
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases the Batching Parameters were set unconditionally,
now the Batching QoS will be only applied if the Batch size is strictly
greater than the sample size.

Changed name of the "-enableTcp" option
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases, the command-line option to use TCP for
communication was named ``-enableTcpOnly``. This is was inconsistent
with other transport options, so the name of the command has been
changed to ``-enableTcp``.

Dynamic Data not working properly when using large samples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases the following error could happen when using the
``-dynamicData`` command-line parameter in conjunction with ``-dataLen``
greater than 63000 bytes:

::

    DDS_DynamicDataStream_assert_array_or_seq_member:!sparsely stored member exceeds 65535 bytes
    DDS_DynamicData_set_octet_array:field bin_data (id=0) not found
    Failed to set uint8_t array

This error has been fixed starting in this release by resetting the
members of the Dynamic Data object before repopulating it.


Release Notes v2.1
------------------

What's New
~~~~~~~~~~~~~~~~~

Multicast Periodic Heartbeats when the ``-multicast`` command-line parameter is present
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases, the Writer side sent heartbeats via unicast even
if the command-line parameter ``-multicast`` was present. Now heartbeats
will be sent via multicast when ``-multicast`` is used. This change
should not affect one-to-one scenarios, but it will reduce the number of
heartbeats the Publisher side has to send in scenarios with multiple
subscribers.

Added command-line parameter to get the *Pulled Sample Count* in the Publisher side
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``-writerStats`` command-line parameter now enables the some extra
debug log messages shown in the *Publisher* side of *RTI Perftest*.
These messages will contain the total number of samples being "pulled"
by the *Subscriber* side.

Added extra logic to be able to support *RTI Connext DDS 5.2.7* on Windows Systems
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The names of the solutions generated by *rtiddsgen* for Windows
architectures changed in Code Generator 3.2.6 (included with *RTI
Connext DDS 5.2.7*). The solution name now includes the *rtiddsgen*
version number. Therefore the *RTIPerftest*'s ``build.bat`` script now
must query the *rtiddsgen* version and adjust the name of the generated
solutions it needs to call to compile.

This change should not be noticed by the user, as the script will
automatically handle the task of determining the version of *rtiddsgen*.

Added command-line parameter to avoid loading QoS from xml in C++.
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If the ``-noXmlQos`` option is provided to *RTI Perftest* it will not
try to load the QoS from the ``xml`` file, instead it will load the QoS
from a string provided in the code. This string contains the same values
the ``xml`` file provides.

This option is only present for the Modern and Traditional C++ PSM API
code.

Note that changes in the ``xml`` will be ignored if this option is
present.

Updated Secure Certificates, Governance and Permission Files
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Governance and Permission files have been updated to be compatible with
the latest release for *RTI Connext DDS*, and are compatible with *RTI
Connext DDS* 5.2.7 and greater.

If you are compiling *RTI Perftest* against 5.2.5, you will need to get
the certificates from the ``release/2.0`` branch. You can do that by
using the following git command from the top-level directory of your
repository:

::

    git checkout release/2.0 -- resource/secure

What's Fixed
~~~~~~~~~~~~~~~~~~~

"--nddshome" Command-Line Option did not Work in ``build.bat`` Script -- Windows Systems Only
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There was an error in the ``build.sh`` script logic when checking for
the existence of the compiler executable files. This problem has been
resolved.

``build.sh`` script did not make sure executable existed before starting compilation
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Part of the ``build.sh`` script logic to check the existence of the
compiler executable files was not being called properly. This issue is
now fixed.

Incorrect ``high_watermark`` value when ``sendQueueSize`` is set to 1
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Setting the command-line parameter ``-sendQueueSize`` to 1 caused *RTI
Perftest* to fail, since it mistakenly set the ``high_watermark`` value
equal to the ``low_watermark``. This problem has been resolved. Now the
``high_watermark`` is always greater than the ``low_watermark``.

Batching settings not correctly set in the ``C++03`` code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Settings related to batching in the XML configuration
(``perftest_qos_profiles.xml``) were not being used. This problem has
been resolved.

``dds.transport.shmem.builtin.received_message_count_max`` incorrectly set in Java code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The ``dds.transport.shmem.builtin.received_message_count_max`` property
was incorrectly set to 1 in every case. This erroneous behavior, which
was introduced in *RTI Perftest 2.0*, has been resolved.

Command-line parameter for setting the *RTI Connext DDS* verbosity
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In previous releases of RTI Perftest, the RTI Connext DDS verbosity
could only be modified by using the command-line parameter ``-debug``.
This parameter would set the verbosity to ``STATUS_ALL``, with no option
to select an intermediate verbosity.

This behavior has been modified. The command-line parameter ``-debug``
has been changed to ``-verbosity,`` which can be followed by one of the
verbosity levels (Silent, Error, Warning, or All).

The default verbosity is Error.

Release Notes v2.0
------------------

What's New
~~~~~~~~~~~~~~~~~

Platform support and build system
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest 2.0* makes use of the *RTI Connext DDS* *Rtiddsgen* tool
in order to generate part of its code and also the makefile/project
files used to compile that code.

Therefore, all the already generated makefiles and *Visual Studio*
solutions have been removed and now the build system depends on 2
scripts: ``build.sh`` for Unix-based systems and ``build.bat`` for
Windows systems.

*RTI Perftest* scripts works for every platform for which *Rtiddsgen*
can generate an example, except for those in which *Rtiddsgen* doesn't
generate regular makefiles or *Visual Studio Solutions* but specific
project files. That is the case of *Android* platforms as well as the
*iOS* ones.

Certain platforms will compile with the out of-the-box code and
configurations, but further tuning could be needed in order to make the
application run in the specific platform. The reason is usually the
memory consumption of the application or the lack of support of the
platform for certain features (like a file system).

Improved directory structure
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

*RTI Perftest 2.0* directory structure has been cleaned up, having now a
much more compact and consistent schema.

Github
^^^^^^

*RTI Perftest* development has been moved to a *GitHub* project. This
will allow more frequently updates and code contributions.

The URL of the project is the following:
`github.com/rticommunity/rtiperftest <github.com/rticommunity/rtiperftest>`__.

Numeration schema
^^^^^^^^^^^^^^^^^

*RTI Perftest* development and releases are now decoupled from *RTI
Connext DDS* ones, therefore, and to avoid future numeration conflicts,
*RTI Perftest* moved to a different numeration schema.

The compatibility between *RTI Perftest* versions and *RTI Connext DDS*
ones will be clearly stated in the release notes of every *RTI Perftest*
release, as well as in the top-level ``README.md`` file.

Documentation
^^^^^^^^^^^^^

Documentation is no longer provided as a PDF document, but as *markdown*
files as well as in *html* format. You will be able to access to the
documentation from the *RTI Community* page, as well as from the
*GitHub* project.

Support for UDPv6
^^^^^^^^^^^^^^^^^

Added command-line parameter to force communication via UDPv6. By
specifying ``-enableUdpv6`` you will only communicate data by using the
UDPv6 transport.

The use of this feature will imply setting the ``NDDS_DISCOVERY_PEERS``
environment variable to (at least) one valid IPv6 address.

Support for Dynamic data
^^^^^^^^^^^^^^^^^^^^^^^^

Added command-line parameter to specify the use of the Dynamic Data API
instead of the regular *Rtiddsgen* generated code use.

Simplified execution in VxWorks kernel mode
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The execution in *VxWorks OS kernel mode* has been simplified for the
user. Now the user can make use of ``subscriber_main()`` and
``publisher_main()`` and modify its content with all the parameters
required for the tests.

Decreased Memory Requirements for Latency Performance Test
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The default number of iterations (samples sent by the performance test
publisher side) when performing a latency test has been updated. Before,
the default value was ``100,000,000``. This value was used to internally
allocate certain buffers, which imposed large memory requirements. The
new value is ``10,000,000`` (10 times less).

What's Fixed
~~~~~~~~~~~~~~~~~~~

RTI Perftest behavior when using multiple publishers
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The previous behavior specified that an *RTI Perftest Subscriber* in a
scenario with multiple *RTI Perftest Publishers* would stop receiving
samples and exit after receiving the last sample from the *RTI Perftest*
Publisher with ``pid=0``. This behavior could lead into an hang state if
some *RTI Perftest Publishers* with different ``pid`` were still missing
to send new samples.

The new behavior makes the *RTI Perftest Subscriber* wait until all the
Perftest Publishers finish sending all their samples and then exit.

Possible ``std::bad_alloc`` and Segmentation Fault in Latency Test in case of insufficient memory
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When performing a latency performance test with traditional or modern
C++, the test tries to allocate certain arrays of unsigned longs. These
arrays can be quite large. On certain embedded platforms, due to memory
limitations, this caused a ``std::bad_alloc`` error that was not
properly captured, and a segmentation fault. This problem has been
resolved. Now the performance test will inform you of the memory
allocation issue and exit properly.

Default Max Number of Instances on Subscriber Side Changed to ``DDS_LENGTH_UNLIMITED``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

In the previous release, if you did not set the maximum number of
instances on the subscriber side, it would default to one instance.
Therefore the samples for all instances except the first one were lost.

The new default maximum number of instances on the subscriber side has
been changed from one to ``DDS_LENGTH_UNLIMITED``. You can change this
limit manually by setting the Parameter ``-instances <number>``.

Error when using Shared Memory and Large Samples
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When using *RTI Perftest* with large samples and enabling shared memory
we could get into the following error:

::

    Large data settings enabled (-dataLen > 63000).
    [D0001|ENABLE]NDDS_Transport_Shmem_Property_verify:received_message_count_max < 1
    [D0001|ENABLE]NDDS_Transport_Shmem_newI:Invalid transport properties.

Known Issues
------------

Building RTI Perftest Java API against RTI Connext DDS 5.2.0.x
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Due to the changes added in order to support larger data sizes, *RTI
Perftest* now makes use of *Unbounded Sequences*. This feature was not
added to *RTI Connext DDS* in *5.2.0.x*, so the following error will be
reported when trying to compile the Java API:

::

    [INFO]: Generating types and makefiles for java.
    [INFO]: Command: "/home/test/nevada/bin/rtiddsgen" -language java -unboundedSupport -replace -package com.rti.perftest.gen -d "/home/test/test-antonio/srcJava" "/home/test/test-antonio/srcIdl/perftest.idl"
    ERROR com.rti.ndds.nddsgen.Main Fail:  -unboundedSupport is only supported with C, C++, C++/CLI, or C# code generation
    rtiddsgen version 2.3.0
    Usage: rtiddsgen [-help]
    . . .
    INFO com.rti.ndds.nddsgen.Main Done (failures)
    [ERROR]: Failure generating code for java.

In order to avoid this compilation error, 2 changes are needed:

-  In the ``build.sh`` or ``build.bat`` scripts, modify the call for
   *Rtiddsgen* and remove the ``-unboundedSupport`` flag.

-  In the ``srcIdl/perftest.idl`` file, modify the ``TestDataLarge_t``
   and ``TestDataLargeKeyed_t`` types and add a bound to the
   ``bin_data`` member: ``sequence<octet,LIMIT> bin_data;``.

Publication rate precision on Windows Systems when using "sleep" instead of "spin"
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When using the ``-pubRate <#>:sleep`` or ``-sleep`` command-line
parameters on Windows Systems the ``sleep()`` precision will be accurate
up to 10 milliseconds. This means that for publication rates of more
than 10000 samples per second we recommend using the "<#>:spin" option
instead.

Compiling manually on Windows Systems when using the *RTI Security* plugin
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

*rtiddsgen* generated solutions for Windows Systems allow 4 different
configurations:

-  Debug
-  Debug DLL
-  Release
-  Release DLL

However, the new *RTI Perftest* build system is focused on only
compiling one of those modes at a time. To choose the compilation mode,
use the ``-debug`` and ``-dynamic`` flags.
