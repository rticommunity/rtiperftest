.. _section-testing_performance:

Testing Performance
===================

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
