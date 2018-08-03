.. _section-introduction:

Introduction
============

Testing Performance
-------------------

This document describes how to run a combined latency and throughput
test application for *RTI Connext DDS*.

The test measures what is sometimes called loaded latency -- latency at
a given throughput level. It can help you answer questions such as:

-  For a given publishing configuration (e.g., queue size, batching
   settings), sample size, and subscribing configuration (e.g., queue
   size, Listener vs. WaitSet) what is the throughput of my network?

-  When my network is heavily loaded, what latency can I expect?

-  For a given configuration, what is the best-case latency with no
   other traffic on the network?

For examples of specific configurations, see See Example Command Lines
for Running the Performance Test.

This performance test is implemented in C++ (Traditional and Modern
APIs), C# and Java.

**Note:** All references in this document to “C++” refer to both the
Traditional and Modern C++ APIs unless otherwise noted.

Overview
--------

The publishing side of the test writes data as fast as it can. Every few
samples (configured through the command-line), it sends a special sample
requesting an echo from the subscribing side. It uses this
``request -> echo`` exchange to measure round-trip latency.

.. figure:: _static/PerfTest_Overview_Diagram.png
   :alt: PerfTest Overview Diagram

   PerfTest Overview Diagram

As you will see in Section 8, there are several command-line options,
including ones to designate whether the application will act as the
publisher or subscriber.

You will start multiple copies of the application (typically 1 publisher
and 1 or more subscribers):

-  The publishing application publishes throughput data; it also
   subscribes to the latency echoes.
-  The subscribing applications subscribe to the throughput data, in
   which the echo requests are embedded; they also publish the latency
   echoes.

The publisher prints the latency test results meanwhile the subscriber
prints the throughput results.
