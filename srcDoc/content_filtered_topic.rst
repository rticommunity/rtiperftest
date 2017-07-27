.. _section-content_filtered_topic:

Using RTI Perftest with RTI Content-Filtered Topics
===================================================

*RTI Perftest* can be used to test latency and throughput scenarios
using Content-Filtered Topics (*CFTs*). This is specially useful in
scenarios with many subscribers.

Using *CFTs* will allow you to:
-------------------------------

-  Limit the number of data samples a DataReader has to process, which
   results in less CPU consumption.
-  Reduce the amount of data sent over the network.

Command-Line Parameters:
------------------------

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
-------------------------------------------------------

The following are examples of how to run *RTI Perftest* for the
different scenarios using *CFT*.

Latency test, 1 Publisher and 2 Subscribers, Publisher sending to only 1 of them
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

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
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

-  *RTI Perftest* Publisher:

::

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -domain <ID> -numSubscribers 2 -latencyCount 1 -dataLen <length> -latencyTest -executionTime 100 -keyed -instances 2

-  *RTI Perftest* Subscriber 1:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -dataLen <length> -sidMultiSubTest 0 -cft 0 -keyed

-  *RTI Perftest* Subscriber 2:

::

    bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -dataLen <length> -sidMultiSubTest 1 -cft 1 -keyed
