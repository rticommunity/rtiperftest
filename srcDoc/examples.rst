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
   command-line.

-  If you are running on 2 unequal machines, i.e., one machine is faster
   (has better processors) than another, you will see better performance
   by running the Publisher on the slower machine.

-  To measure CPU usage while running these tests, use "-cpu" argument
   or TOP utility.

RTI Connext DDS Professional
----------------------------

1-to-1, Multicast, Best Latency as a Function of Message Size
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen 200 -batchSize <bytes> -sendQueueSize <number> -pubRate <count> -transport UDPv4 -multicast -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -transport UDPv4 -multicast

To adjust throughput, experiment with the value of ``-pubRate <count>``.

1-to-1, Multicast, Reliable UDPv4, All Sizes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -sendQueueSize 32 -latencyCount 10000 -scan -transport UDPv4 -multicast

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -transport UDPv4 -multicast

1-to-1, Unicast, Best-Effort, UDPv4|SHMEM, 1 Size
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -sendQueueSize 32  -latencyCount 1000 -dataLen 1024 -bestEffort -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 1024 -bestEffort

1-to-1, Multicast, Reliable, UDPv4, Batching Enabled
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -sendQueueSize 32 -latencyCount 1000 -dataLen 200 -batchSize 6400 -transport UDPv4 -multicast -executionTime 100

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 200 -batchSize 6400 -transport UDPv4 -multicast

1-to-2, Multicast, Reliable, UDPv4
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -dataLen 63000 -secureSign -secureEncryptData -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -dataLen 63000 -secureSign -secureEncryptData


1-to-1, RawTransport, Unicast, BestEffort (Same Machine)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -rawTransport -dataLen 63000 -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport


1-to-2, RawTransport, Unicast, BestEffort (Same Machine)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -rawTransport -peer 127.0.0.1 -peer 127.0.0.1:4 -numSubscribers 2 -dataLen 63000 -executionTime 100

-  Subscriber 1

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -sidMultiSubTest 0

-  Subscriber 2

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -sidMultiSubTest 4



1-to-1, RawTransport, Unicast, BestEffort (Different Machine)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -rawTransport -peer 10.70.1.50 -nic 10.70.1.49  -dataLen 63000 -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -peer 10.70.1.49 -nic 10.70.1.50

1-to-2, RawTransport, Multicast, BestEffort
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -rawTransport -multicastAddr 225.0.0.1 -nic 10.70.1.1 -numSubscribers 2 -dataLen 63000 -executionTime 100

-  Subscriber 1

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -multicastAddr 225.0.0.1 -nic 10.70.2.1

-  Subscriber 2

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -multicastAddr 225.0.0.1 -nic 10.70.2.2


1-to-1, RawTransport, SharedMemory, Unicast, BestEffort
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -rawTransport -transport SHMEM -dataLen 63000 -executionTime 100

-  Subscriber

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -transport SHMEM -rawTransport


1-to-2, RawTransport, SharedMemory, Unicast, BestEffort
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -rawTransport -transport SHMEM -peer 127.0.0.1:0 -peer 127.0.0.1:4 -numSubscribers 2 -dataLen 63000 -executionTime 100

-  Subscriber 1

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -transport SHMEM -sidMultiSubTest 0

-  Subscriber 2

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -rawTransport -transport SHMEM -sidMultiSubTest 4


.. _section-large_sample:

RTI Connext DDS Micro
---------------------

1-to-1, Unicast, Best Latency as a Function of Message Size
-------------------------------------------------------------

-  Publisher:

::

    bin/<arch>/release/perftest_cpp_micro -pub -noPrint -nic <ipaddr> -domain <ID> -latencyCount 1 -dataLen <length> -latencyTest -executionTime 100

-  Subscriber:

::

    bin/<arch>/release/perftest_cpp_micro -sub -noPrint -nic <ipaddr> -domain <ID>

Modify ``-dataLen <bytes>`` to see latencies for different data sizes.
Set ``-executionTime <seconds>`` to be >=100 for statistically better
results.

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

*CFTs* support is just implemented for *RTI Connext DDS Professional*.

Using *CFTs* will allow you to
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  Limit the number of data samples a DataReader has to process, which
   results in less CPU consumption.
-  Reduce the amount of data sent over the network.

Command-Line Parameters
~~~~~~~~~~~~~~~~~~~~~~~

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

Example Command Lines for Running the Performance Test
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
-------------------

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

Using Custom Types
------------------

The Custom Types feature allows you to use your own customized types instead of
the one provided by *RTI Perftest*. It is designed in such a way that the number
of changes in the code and configuration files is minimal.

Briefly, the steps you need to perform to use your custom type in *RTI Perftest* are as follows:

-  Copy your IDL files into `~/rtiperftest/srcIdl/custom/`
-  Implement the API custom type functions of customtype.cxx
-  Run the build script with the command-line parameter ``--customType <type>``
-  Run *RTI Perftest* as usual.

Full example using Custom Types
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The custom type that will be used for this example is the following:

::

    const long SIZE_TEST_SEQ = 100;
    const long SIZE_TEST_STRING = 128;
    enum TestEnum {
        ENUM1,
        ENUM2
    };//@Extensibility FINAL_EXTENSIBILITY
    struct StringTest {
        string<SIZE_TEST_STRING> test_string;
    };//@Extensibility FINAL_EXTENSIBILITY
    struct SeqTest {
        sequence<long, SIZE_TEST_SEQ> test_seq;
    };//@Extensibility FINAL_EXTENSIBILITY
    struct Test {
        long test_long;
        TestEnum test_enum;
        StringTest test_string;
        SeqTest test_seq;
    };//@Extensibility FINAL_EXTENSIBILITY

These are the steps needed to use the above type in *RTI Perftest* for the
C++ (Traditional) API:



1. Copy your IDL files into `~/rtiperftest/srcIdl/customType/` folder.

2. The following functions should be implemented (optionally) to properly
   initialize and set the Custom Type structures.

    - **initialize_custom_type_data**:
        This function is used to initialize your data.
        Using this function, you will be able to allocate memory or set an immutable
        field of the data.
        The function takes one argument:

            - A reference to custom type data.

    ::

        bool initialize_custom_type_data(RTI_CUSTOM_TYPE &data)
        {
            bool success = true;
            if (!data.test_seq.test_seq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
                success = false;
            }
            data.test_enum = ENUM1;
            return success;
        }

    - **register_custom_type_data**:
        This function is used to set your data before being registered. It is only
        required for key types. Set the key field of the data based on the key input.
        There is a one-to-one mapping between an input key
        and an instance.
        The function takes two arguments:

            - A reference to custom type data.
            - A specific number unique for every key.

    ::

        void register_custom_type_data(RTI_CUSTOM_TYPE & data, unsigned long key)
        {
            data.test_long = key;
        }

    - **set_custom_type_data**:
        This function is used to set your data before it is sent.
        It is called every time the data is sent.
        You must set the Custom type data before it is sent with the right
        "key" value and the "targetDataLen".
        The function takes three arguments:

            - A reference to custom type data.
            - A specific number unique for every key.
            - The target size set by the command-line parameter ``-dataLen <bytes>``
              minus the overhead of *RTI Perftest*. If applicable, you can use this
              value to set the content of the data.

    ::

        bool set_custom_type_data(
                RTI_CUSTOM_TYPE & data,
                unsigned long key,
                int targetDataLen)
        {
            bool success = true;
            data.test_long = key;
            if (sprintf(data.test_string.test_string, "Hello World! %lu", key) < 0) {
                success = false;
            }
            return success;
        }

    - **finalize_custom_type_data**:
        This function is used to remove your data. It is called in the destructor.
        The function takes one argument:

            - A reference to custom type data.

    ::

        bool finalize_custom_type_data(RTI_CUSTOM_TYPE & data)
        {
            bool success = true;
            if (!data.test_seq.test_seq.maximum(0)) {
                success = false;
            }
            return success;
        }

    - **initialize_custom_type_dynamic_data**:
        This function is used to initialize your DynamicData.
        Using this function, you will be able to allocate memory or set an immutable
        field of the data.
        The function takes one argument:

            - A reference to the full DDS_DynamicData object that includes custom_type.

    ::

        bool initialize_custom_type_dynamic_data(DDS_DynamicData & data)
        {
            bool success = true;
            if (!longSeq.ensure_length(SIZE_TEST_SEQ, SIZE_TEST_SEQ)) {
                success = false;
                fprintf(stderr, "longSeq.ensure_length failed.\n");
            }
            return success;
        }

    - **register_custom_type_dynamic_data**:
        This function is used to set your DynamicData before it has been registered.
        It is only required for key types.
        Set the key field of the data based on the key input.
        There is a one-to-one mapping between an input key and an instance.
        The function takes two arguments:

            - A reference to the full DDS_DynamicData object that includes custom_type.
            - A specific number unique for every key.

    ::

        void register_custom_type_dynamic_data(DDS_DynamicData & data, unsigned long key)
        {
            DDS_ReturnCode_t retcode = data.set_long(
                    "custom_type.test_long",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    key);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
            }
        }

    - **set_custom_type_dynamic_data**:
        This function is used to set your DynamicData before it is sent.
        It is called every time the data is sent.
        Set the Custom type data before it is sent with the right "key"
        value and the "targetDataLen".
        The function takes three arguments:

            - A reference to the full DDS_DynamicData object that includes custom_type.
            - A specific number unique for every key.
            - The target size set by the command-line parameter ``-dataLen <bytes>``
              minus the overhead of *RTI Perftest*. If applicable, you can use this
              value to set the content of the data.

    ::

        bool set_custom_type_dynamic_data(
                DDS_DynamicData & data,
                unsigned long key,
                int targetDataLen)
        {
            DDS_ReturnCode_t retcode;
            char test_string[SIZE_TEST_STRING]; //size of member_name
            bool success = true;
            DDS_DynamicData customTypeData(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);
            DDS_DynamicData testSeqData(NULL, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT);

            retcode = data.bind_complex_member(
                    customTypeData,
                    "custom_type",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "bind_complex_member(custom_type) failed: %d.\n",
                        retcode);
                success = false;
            }

            retcode = customTypeData.set_long(
                    "test_long",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    key);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_long) failed: %d.\n", retcode);
                success = false;
            }

            retcode = customTypeData.set_long(
                    "test_enum",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    ENUM1);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_enum) failed: %d.\n", retcode);
                success = false;
            }

            if (snprintf(test_string, SIZE_TEST_STRING, "Hello World! %lu", key) < 0) {
                success = false;
            }
            retcode = customTypeData.set_string(
                    "test_string.test_string",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                    test_string);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_string(test_string) failed: %d.\n", retcode);
                success = false;
            }

            retcode = customTypeData.bind_complex_member(
                    testSeqData,
                    "test_seq",
                    DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "bind_complex_member(testSeqData) failed: %d.\n",
                        retcode);
                success = false;
            }
            retcode = testSeqData.set_long_seq(
                        "test_seq",
                        DDS_DYNAMIC_DATA_MEMBER_ID_UNSPECIFIED,
                        longSeq);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(test_seq) failed: %d.\n", retcode);
                success = false;
            }
            retcode = customTypeData.unbind_complex_member(testSeqData);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "unbind_complex_member(testSeqData) failed: %d.\n",
                        retcode);
                success = false;
            }
            retcode = data.unbind_complex_member(custom_type_data);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "unbind_complex_member(custom_type) failed: %d.\n",
                        retcode);
                success = false;
            }
            return success;
        }

    - **finalize_custom_type_dynamic_data**:
        This function is used to remove your data. It is called in the destructor.
        The function takes one argument:

            - A reference to the full DDS_DynamicData object that includes custom_type.

    ::

        bool finalize_custom_type_dynamic_data(DDS_DynamicData & data)
        {
            bool success = true;
            if (!longSeq.ensure_length(0, 0)) {
                success = false;
                fprintf(stderr, "longSeq.ensure_length failed.\n");
            }
            DDS_ReturnCode_t retcode = data.clear_all_members();
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "clear_all_members failed: %d.\n", retcode);
                success = false;
            }
            return success;
        }

3. Build *RTI Perftest* using ``--customType <type>``.

    ::

        ./build.sh --platform x64Linux3gcc5.4.0 --nddshome /home/rti_connext_dds-5.3.0 --cpp-build --customType Test

4. Launch *RTI Perftest*.

    ::

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -pub -executionTime 60 -noprint
        RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

        Custom Type provided: 'Test'

        Mode: THROUGHPUT TEST
            (Use "-latencyTest" for Latency Mode)

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Publisher ID: 0
            Latency count: 1 latency sample every 10000 samples
            Data Size: 100
            Batching: 8192 Bytes (Use "-batchSize 0" to disable batching)
            Publication Rate: Unlimited (Not set)
            Execution time: 60 seconds
            Receive using: Listeners
            Domain: 1
            Dynamic Data: No
            Asynchronous Publishing: No
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 subscribers ...
        Waiting for subscribers announcement ...
        Sending 4050 initialization pings ...
        Publishing data ...
        Setting timeout to 60 seconds
        Length:   464  Latency: Ave     39 us  Std   30.7 us  Min     21 us  Max    276 us  50%     30 us  90%     60 us  99%    276 us  99.99%    276 us  99.9999%    276 us
        Finishing test due to timer...
        Test ended.



    ::

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -sub -noprint
        RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

        Custom Type provided: 'Test'

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Subscriber ID: 0
            Receive using: Listeners
            Domain: 1
            Dynamic Data: No
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 publishers ...
        Waiting for data...
        Length:   464  Packets: 68081040  Packets/s(ave): 1134692  Mbps(ave):  4212.0  Lost:     0 (0.00%)
        Finishing test...
        Test ended.



5. Besides, you can launch *RTI Perftest* with your customType using DynamicData.

    ::

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -pub -executionTime 60 -noprint -dynamicData
        RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

        Custom Type provided: 'Test'

        Mode: THROUGHPUT TEST
            (Use "-latencyTest" for Latency Mode)

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Publisher ID: 0
            Latency count: 1 latency sample every 10000 samples
            Data Size: 100
            Batching: 8192 Bytes (Use "-batchSize 0" to disable batching)
            Publication Rate: Unlimited (Not set)
            Execution time: 60 seconds
            Receive using: Listeners
            Domain: 1
            Dynamic Data: Yes
            Asynchronous Publishing: No
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 subscribers ...
        Waiting for subscribers announcement ...
        Sending 4050 initialization pings ...
        Publishing data ...
        Setting timeout to 60 seconds
        Length:   464  Latency: Ave    158 us  Std  166.5 us  Min     71 us  Max    678 us  50%    105 us  90%    169 us  99%    678 us  99.99%    678 us  99.9999%    678 us
        Finishing test due to timer...
        Test ended.




    ::

        ~/rtiperftest$ ./bin/x64Linux3gcc5.4.0/release/perftest_cpp -sub -noprint -dynamicData
        RTI Perftest 2.3.2 (RTI Connext DDS 5.3.0)

        Custom Type provided: 'Test'

        Perftest Configuration:
            Reliability: Reliable
            Keyed: No
            Subscriber ID: 0
            Receive using: Listeners
            Domain: 1
            Dynamic Data: Yes
            XML File: perftest_qos_profiles.xml

        Transport Configuration:
            Kind: UDPv4 & SHMEM (taken from QoS XML file)
            Use Multicast: False

        Waiting to discover 1 publishers ...
        Waiting for data...
        Length:   464  Packets:  8146078  Packets/s(ave):  135770  Mbps(ave):   504.0  Lost:     0 (0.00%)
        Finishing test...
        Test ended.
