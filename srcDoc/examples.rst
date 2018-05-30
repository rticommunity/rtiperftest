.. _section-command_examples:

======================
Use-Cases And Examples
======================

Example Command-Lines for Running the Performance Test
======================================================

The followings are examples of how to run the performance test for
different use cases.

-  The tests below print final results only; if you want to see
   intermediate values, remove the ``-noprint`` argument from the
   command line.

-  If you are running on 2 unequal machines, i.e., one machine is faster
   (has better processors) than another, you will see better performance
   by running the Publisher on the slower machine.

-  To measure CPU usage while running these tests, use "-cpu" argument
   or TOP utility.

1-to-1, Multicast, Best Latency as a Function of Message Size
-------------------------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -latencyCount 1 -dataLen <length> -latencyTest -multicast -executionTime 100

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -multicast

Modify ``-dataLen <bytes>`` to see latencies for different data sizes.
Set ``-executionTime <seconds>`` to be >=100 for statistically better
results.

1-to-1, Multicast, Maximum Throughput as a Function of Message Size (with Batching)
-----------------------------------------------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen <length> -batchSize <bytes> -sendQueueSize <number> -multicast -executionTime 100

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noprint -nic <ipaddr> -multicast

To achieve maximum throughput, start by setting See
``-batchSize <bytes>`` to ``6400``, then increase the size to see if you
get better throughput.

The largest valid batch size is ``63000 bytes``.

For maximum throughput, start by setting ``-sendQueueSize <number>`` to
``30``; the best value will usually be between ``30-50``.

Note: For larger data sizes (``8000 bytes`` and higher), batching often
does not improve throughput, at least for 1-Gig networks.

1-to-1, Multicast, Latency vs. Throughput for 200-byte Messages (with Batching)
-------------------------------------------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen 200 -batchSize <bytes> -sendQueueSize <number> -pubRate <count> -transport UDPv4 -multicast -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -transport UDPv4 -multicast

To adjust throughput, experiment with the value of ``-pubRate <count>``.

1-to-1, Multicast, Reliable UDPv4, All Sizes
--------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -sendQueueSize 32 -latencyCount 10000 -scan -transport UDPv4 -multicast

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -transport UDPv4 -multicast

1-to-1, Unicast, Best-Effort, UDPv4|SHMEM, 1 Size
---------------------------------------======----

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -sendQueueSize 32  -latencyCount 1000 -dataLen 1024 -bestEffort -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 1024 -bestEffort

1-to-1, Multicast, Reliable, UDPv4, Batching Enabled
----------------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -sendQueueSize 32 -latencyCount 1000 -dataLen 200 -batchSize 6400 -transport UDPv4 -multicast -executionTime 100

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 200 -batchSize 6400 -transport UDPv4 -multicast

1-to-2, Multicast, Reliable, UDPv4
----------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -pidMultiPubTest 0 -sendQueueSize 32 -numSubscribers 2 -latencyCount 1000 -dataLen 200 -transport UDPv4 -multicast -executionTime 100

-  Subscriber 1:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -transport UDPv4 -multicast -sidMultiSubTest 0

-  Subscriber 2:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -transport UDPv4 -multicast -sidMultiSubTest 1

2-to-1, Multicast, Reliable, UDPv4
----------------------------------

-  Publisher 1:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -pidMultiPubTest 0 -sendQueueSize 32 -numSubscribers 1 -latencyCount 1000 -dataLen 200 -multicast -executionTime 100

-  Publisher 2:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -pidMultiPubTest 1 -sendQueueSize 32 -numSubscribers 1 -latencyCount 1000 -dataLen 200 -multicast -executionTime 100

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 200 -numPublishers 2 -sidMultiSubTest 0 -multicast

1-to-1, Unicast, Reliable Using Security: Signing Packages, Encrypting Data
---------------------------------------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -dataLen 63000 -secureSign -secureEncryptData -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 63000 -secureSign -secureEncryptData


.. _section-large_sample:

Use-Cases
=========

Large Samples
-------------

*RTI Perftest* can send samples from 28 Bytes to 2,147,483,135 Bytes (2
GBytes - 512 Bytes - 8 Bytes), which corresponds to the maximum payload
that *RTI Connext DDS* is able to send in a single sample.

The size of data is configured by the Command-Line Parameter
``-dataLen <bytes>``. Depending on this parameter, *RTI Perftest* will
automatically configure certain *RTI Connext DDS* behaviors.

In the case that the sample size is smaller or equal to 63000 Bytes,
*RTI Perftest* will, by default, use types with Bounded-Sequences (bound
set to 63000 elements). If the sample size is bigger than 63000 Bytes,
*RTI Perftest* will automatically switch to equivalent types to the ones
mentioned previously, but with Unbounded-Sequences.

The reason behind this behavior is that in the case when *RTI Perftest*
uses Unbounded-Sequences, *RTI Connext DDS* will not pre-allocate the
sequences to their maximum size (as opposite as when using bounded
sequences). For Unbounded-Members, the code generated by *RTI Connext
DDS* will de-serialize the samples by dynamically allocating and
de-allocating memory to accommodate the actual size of the unbounded
member. Unbounded-Sequences and strings are also supported with
DynamicData (Command-Line parameter ``-DynamicData``).

Apart from the use of Unbounded-Sequences, by setting samples bigger
than 63000 Bytes, *RTI Perftest* will enable the use of *Asynchronous
Publishing*, as set *RTI Connext DDS* default flow controller.

The case where the user wants to use Unbounded-Sequences, Asynchronous
Publishing or a Flow Controller different than the default one but the
sample size is smaller than 63000 Bytes is also supported. These
behaviors can be achieved by using the Command-Line Parameters
``-unbounded <managerMemory>``, ``-asynchronous`` and
``-flowController``, see the **Test Parameters** section for more
details.

--------------

Adjusting the configuration
~~~~~~~~~~~~~~~~~~~~~~~~~~~

Find here an example where the sample size is configured to 1GB:

-  Publisher:

::

    bin/<architecture>/<release or debug>/perftest_cpp -pub -dataLen 1073741824

-  Subscriber:

::

    bin/<architecture>/<release or debug>/perftest_cpp -sub -dataLen 1073741824

This is a perfectly valid configuration, accepted by *RTI Perftest*,
however, in certain cases, the communication in this scenario will be
limited or non-optimal, due to the large sample size. Therefore some
extra tuning might be required:

By using the parameter ``-sendQueueSize <number>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The default value for the Send Queue in the Writer side is ``50``. That
might be a very high value, and the *RTI Connext DDS* middleware might
not perform in optimal conditions. Therefore, for large data samples it
is recommended to reduce the send Queue to lower values.

By using the parameter ``-pubRate <samples/s>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This parameter can be used in order to limit the frequency in which *RTI
Perftest* publishes samples. This can help reducing the number of
packages in the network, helping to achieve better Latency and
Throughput numbers.

By using a flow controller ``-flowController <default,1Gbps,10Gbps>``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Since the sample size is bigger than 63000 Bytes, *RTI Perftest* will
enable Asynchronous Publing. By enabling that, you also make use of the
default *Flow Controller*, which might not be optimal. It might be a
good practice to specify a *Flow Controller* tailored for the network
where the scenario is.

*RTI Perftest* provides options to use a flow controller designed for a
10Gbps network and a 1Gbps one. However, by accessing to the
``perftest_qos_profiles.xml`` configuration file it is possible to
modify these 2 Flow Controllers and tailor them to specific network
requirements.

.. code:: xml

    <qos_profile name="BaseProfileQos">
        <participant_qos>
            . . .
            <property>
                <value>
                    <element>
                        <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.max_tokens</name>
                        <value>300</value>
                    </element>
                    <element>
                        <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.tokens_added_per_period</name>
                        <value>200</value>
                    </element>
                    <element>
                        <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.bytes_per_token</name>
                        <value>65536</value>
                    </element>
                    <element>
                        <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.period.sec</name>
                        <value>0</value>
                    </element>
                    <element>
                        <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.period.nanosec</name>
                        <value>10000000</value>
                    </element>
                </value>
            </property>
            . . .
        </participant_qos>
    </qos_profile>

The specific values for the Flow Controller and the Send Queue will
highly depend on the scenario and machines performing the test, but as a
general suggestion, these changes are recommended:

-  Publisher:

::

    bin/<architecture>/<release or debug>/perftest_cpp -pub -dataLen 1073741824 -sendQueueSize 1 -flowController 1Gbps

-  Subscriber:

::

    bin/<architecture>/<release or debug>/perftest_cpp -sub -dataLen 1073741824

Large Samples in Java
~~~~~~~~~~~~~~~~~~~~~

When using the *RTI Perftest* implementation for *Java* and large data
samples, the following error may appear:

::

    Exception in thread "main" java.lang.OutOfMemoryError: Java heap space

The the memory reserved for the heap is not enough in this case, the way
how to solve this is by increasing the size we allow *Java* to reserve.
This can be done by using the Command-Line Parameter ``-Xmx`` in the
scripts used to run the Java examples: ``bin/Release/perftest_java.sh``
and ``bin\Release\perftest_java.bat``. The increased amount will depend
on the ``-dataLen`` parameter and the memory specifications of device
where *RTI Perftest* is running.


Content-Filtered Topics
-----------------------

*RTI Perftest* can be used to test latency and throughput scenarios
using Content-Filtered Topics (*CFTs*). This is specially useful in
scenarios with many subscribers.

Using *CFTs* will allow you to:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Limit the number of data samples a DataReader has to process, which
   results in less CPU consumption.
-  Reduce the amount of data sent over the network.

Command-Line Parameters:
~~~~~~~~~~~~~~~~~~~~~~~~

To enable the use of CFTs on the subscriber side, the following
parameter is required:

-  ``-cft <start>:<end>``

   Use a Content-Filtered Topic for the Throughput topic in the
   subscriber side. Specify 2 parameters: and to receive samples with a
   key in that range. Specify only 1 parameter to receive samples with
   that exact key.

If no parameter is specified on the publisher side, *RTI Perftest* will
send as many instances as specified (using the ``-instances``
command-line parameter). However, you can change that behavior by using
the following parameter:

-  ``-writeInstance <instance>``

   Set the number of instances to be sent.

Example Command Lines for Running the Performance Test:
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The following are examples of how to run *RTI Perftest* for the
different scenarios using *CFT*.

Latency test, 1 Publisher and 2 Subscribers, Publisher sending to only 1 of them
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  *RTI Perftest* Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -numSubscribers 2 -latencyCount 1 -dataLen <length> -latencyTest -executionTime 100 -writeInstance 0 -keyed -instances 2

-  *RTI Perftest* Subscriber 1:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -dataLen <length> -sidMultiSubTest 0 -cft 0 -keyed

-  *RTI Perftest* Subscriber 2:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -dataLen <length> -sidMultiSubTest 1 -cft 1 -keyed

Latency test, 1 Publisher and 2 Subscribers, Publisher sending using a Round-Robin schedule
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  *RTI Perftest* Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -numSubscribers 2 -latencyCount 1 -dataLen <length> -latencyTest -executionTime 100 -keyed -instances 2

-  *RTI Perftest* Subscriber 1:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -dataLen <length> -sidMultiSubTest 0 -cft 0 -keyed

-  *RTI Perftest* Subscriber 2:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -dataLen <length> -sidMultiSubTest 1 -cft 1 -keyed


.. _section-routing_service:

RTI Routing-Service
-------------------------------------------

This wrapper has been created to test the effects of introducing *RTI
Routing Service* when using *RTI Perftest* in latency and Throughput. It
consists of a set of 2 files:

-  A compatible XML configuration file for *RTI Routing Service*
   parameterized to use different environment variables depending on the
   scenario to test.
-  A wrapper script to launch *RTI Routing Service* which will set the
   environment variables needed by the XML configuration file previously
   mentioned. It contains several command-line parameters to control the
   scenario to be tested.

Command-Line Parameters
~~~~~~~~~~~~~~~~~~~~~~~

-  ``-domain <ID>``

   Domain ID.

   *RTI Routing Service* will route between the provided domain (ID) and
   (ID + 1).

   | **Default:** ``0``
   | **Range:** ``0 - 200``

-  ``-sendQueueSize <number>``

   Size of the send queue for the Writers used in *RTI Routing Service*

   | **Default:** ``50``
   | **Range:** ``[1-100 million]``

-  ``-bestEffort``

   Use best-effort reliability settings.

   **Default:** ``false`` (use reliable communication).

-  ``-asynchronous``

   Enable asynchronous publishing in the DataWriter QoS.

   **Default:** ``Not set``

-  ``-unbounded``

   Use *Unbounded Sequences* and Large samples.

   **Default:** ``Not set``

-  ``-verbosity``

   Specify the verbosity level for *RTI Routing Service*

   | ``0`` - ``SILENT``
   | ``1`` - ``ERROR`` (default) ``2`` - ``WARNING``
   | ``3`` - ``ALL``

-  ``-keyed``

   Specify the use of a keyed type.

   **Default:** ``Unkeyed`` type.

-  ``-batchSize <bytes>``

   Enable batching and set the maximum batched message size.

   | **Default:** ``0`` (batching disabled)
   | **Range:** ``1 to 63000``

-  ``-executionTime <sec>``

   Limit the test duration by specifying the number of seconds to keep
   *RTI Routing Service* running.

   **Default:** Not set, infinite.

-  ``-nddshome``

   Path to the *RTI Connext DDS* installation. If this parameter is not
   present, the ``$NDDSHOME`` variable will be used.

Example Command Lines for Running the Performance Test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The followings are examples of how to run the performance test for
different use cases.

Minimum Latency -- 1 *Routing Service*
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  *RTI Perftest* Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -latencyCount 1 -dataLen <length> -latencyTest -executionTime 100

-  *RTI Routing Service* wrapper script:

::

    resource/routing_service/routingservice_wrapper.sh -domain <ID> -executionTime 120

-  *RTI Perftest* Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID+1> -dataLen <length>

Maximum Throughput -- 1 *Routing Service*
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  *RTI Perftest* Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -batchSize <bytes> -sendQueueSize <number> -executionTime 100 -dataLen <length>

-  *RTI Routing Service* wrapper script:

::

    resource/routing_service/routingservice_wrapper.sh -domain <ID> -executionTime 120 -batchSize <bytes> -sendQueueSize <number>

-  *RTI Perftest* Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID+1> -dataLen <length>

Maximum Throughput -- 2 *Routing Service*
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

-  *RTI Perftest* Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -batchSize <bytes> -sendQueueSize <number> -executionTime 100 -dataLen <length>

-  *RTI Routing Service 1* wrapper script:

::

    resource/routing_service/routingservice_wrapper.sh -domain <ID> -executionTime 120 -batchSize <bytes> -sendQueueSize <number>

-  *RTI Routing Service 2* wrapper script:

::

    resource/routing_service/routingservice_wrapper.sh -domain <ID+1> -executionTime 120 -batchSize <bytes> -sendQueueSize <number>

-  *RTI Perftest* Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID+2> -dataLen <length>
