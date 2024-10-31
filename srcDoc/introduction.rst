.. _section-introduction:

Introduction
============

Testing Performance
-------------------

This section describes the combined latency and throughput
test application for *RTI Connext*.

The test measures what is sometimes called loaded latency—latency at
a given throughput level. It can help you answer questions such as:

-  For a given publishing configuration (e.g., queue size, batching
   settings), sample size, and subscribing configuration (e.g., queue
   size, Listener vs. WaitSet), what is the throughput of my network?

-  When my network is heavily loaded, what latency can I expect?

-  For a given configuration, what is the best-case latency with no
   other traffic on the network?

For examples of specific configurations, see :ref:`section-command_line_parameters`.

This performance test is implemented in C++ (Traditional and Modern
APIs), C#, and Java.

.. note::

   All references in this document to “C++” refer to both the
   Traditional and Modern C++ APIs, unless otherwise noted.

Middleware
----------

*RTI Perftest* can be built against *RTI Connext Professional*, *RTI Connext
Micro*, and *RTI Connext Cert*.

*RTI Perftest* can also be used to test raw ``UDPv4`` or ``SHMEM`` performance.
To do this, *RTI Perftest* uses the *RTI Connext Professional* internal
transport layer to interact with sockets. In this mode, *RTI Perftest* skips
the DDS protocol to directly send the data, which is serialized using the same
functions that *RTI Connext Professional* uses.

Overview
--------

The publishing side of the test writes data as fast as it can. Every few
samples (configured through the command line), it sends a special sample
requesting an echo from the subscribing side. It uses this
"request -> echo" exchange to measure round-trip time (RTT) latency.
One-way latency is calculated as RTT/2.

.. figure:: _static/PerfTest_Overview_Diagram.png
   :alt: PerfTest Overview Diagram
   :align: center

   Perftest Overview Diagram

As you will see in :ref:`section-command_line_parameters`, there are many
command-line options, including ones to designate whether the application will
act as the publisher or subscriber.

You will start multiple copies of the application (typically one publisher
and one or more subscribers):

-  The publishing application publishes throughput data; it also
   subscribes to the latency echoes.
-  The subscribing applications subscribe to the throughput data, in
   which the echo requests are embedded; they also publish the latency
   echoes.

The publisher prints the latency test results, and the subscriber
prints the throughput results.

Latency Test vs. Throughput Test
--------------------------------

*RTI Perftest* allows two operational modes: **Throughput Test** and **Latency Test**.

Throughput test
~~~~~~~~~~~~~~~

The throughput test is the default mode when *RTI Perftest* publisher and subscriber
applications are started.

In this mode, the publisher side starts sending samples as fast
as possible. Once every ``latencyCount`` samples (which for a throughput
test is 10000 samples by default), *RTI Perftest* marks a sample to be answered
by the subscriber side. The answer is the exact same sample. The
round-trip time (RTT) is calculated from the time it took for the sample to be
sent, plus the time it took for the publisher to receive it.

Doing a throughput test provides the maximum throughput (Mbps) at which
the publisher will be able to send samples to a subscriber. The latency obtained
in this test will also be displayed by the publisher application; however,
that latency is affected by the fact that the publisher and subscriber will
be flooding the network and filling all their internal queues (sending and
receiver queues, as well as the NIC queues). Therefore, to measure the pure
latency in the best-case scenario, you should use the **Latency Test** mode.

Latency test
~~~~~~~~~~~~

In this mode, the publisher side changes its behavior: every sample it sends
is marked as a latency sample (``latencyCount`` is equal to 1 in this case).
The publisher also waits until the sample is sent back and received by the publisher
before sending the next sample (called "stop-and-wait" mode or
"ping-pong" mode).

In this way, the latency test mode is able to send samples, ensuring all the *RTI Connext
DDS* queues are empty.

The only requirement to change the operation mode to do a latency test is to
provide the ``-latencyTest`` command-line parameter to the publisher side. See
:ref:`section-command_line_parameters` for more details.