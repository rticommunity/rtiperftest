.. raw:: html

    <p style="color:#004C97"; align="center"><font size="+12">RTI Perftest</font></p>

Overview
========

*RTI Perftest* is a command-line tool for measuring the minimum latency,
maximum throughput, and loaded latency in a configurable set of scenarios.
It can help you answer questions such as:

- For a given publishing configuration (e.g., queue size, batching settings), 
  sample size, and subscribing configuration (e.g., queue size, Listener vs. WaitSet), 
  what is the throughput of my network?
- When my network is heavily loaded, what latency can I expect?
- For a given configuration, what is the best-case latency with no other traffic 
  on the network?

*RTI Perftest* works by making the publishing side of the test write data as fast 
as it can. Every few samples (configured through the command line), *RTI Perftest*
sends a special sample requesting an echo from the subscribing side. *RTI Perftest*
uses this ping -> pong exchange to measure the round-trip time (RTT) for latency.
From the RTT, it calculates the one-way latency time (RTT/2).

The Perftest Publisher prints the latency test results, and the Perftest Subscriber
prints the throughput results.

.. figure:: _static/PerfTest_Overview_Diagram.png
   :alt: PerfTest Overview Diagram
   :align: center

*RTI Perftest* allows two operational modes: **Throughput Test** and **Latency Test**.

Learn more in the :ref:`section-introduction` section.

Features
========

*RTI Perftest* supports the following functionality:

- *RTI Connext DDS Professional* and *RTI Connext DDS Micro*.
- Multiple platforms, including Windows®, Linux®, macOS®, VxWorks®, and Android™.
- Multiple test setups: multicast, one-to-many communication (Pub -> Sub),
  latency test, throughput test, and latency vs. throughput test.
- Raw transports (UDPv4 sockets and shared memory segments).
- Benchmarking of *RTI Connext DDS Professional* and *RTI Connext DDS Micro*
  features such as *FlatData™ language binding* and *Zero Copy transfer over shared
  memory.*

Getting Started QuickGuide
==========================

To get a local copy of your project up and running quickly, follow these simple
example steps.

Download
--------

Get the *RTI Perftest* bundle in one of three different ways:

- Clone and compile from the official Github repository:

  .. code-block:: console

   git clone https://github.com/rticommunity/rtiperftest.git

- Download and compile from: `https://github.com/rticommunity/rtiperftest
  <https://github.com/rticommunity/rtiperftest>`_.

..

- Download the executables for popular platforms from the binaries section in the
  *RTI Perftest* release page `here <https://github.com/rticommunity/rtiperftest/releases>`_.

To learn more about the supported platforms and installation, please refer to
the more detailed :ref:`section-download` section.

Prerequisites
-------------

If you need to compile *RTI Perftest*, there are a few prerequisites; however,
if you downloaded the executables already compiled, you can skip these steps:

- *RTI Connext DDS Professional* or *RTI Connext DDS Micro* should be installed
  in the system where the ``build.sh`` script is going to run. The target libraries for
  the platform to be generated should also be installed.
- The ``$NDDSHOME`` environment variable should be set correctly. Or, pass
  ``$NDDSHOME`` directly to the ``build.sh`` script by using the ``--nddshome <PATH>``
  command-line option.
- If you will compile and test using *RTI Security Plugins*, 
  link against the OpenSSL libraries for your architecture.

Learn more in the :ref:`section-compilation` section.

Compile
-------

For Linux, macOS, QNX, VxWorks, Lynx, and Android, *RTI Perftest* makes use of a
script in the top-level directory named ``build.sh``. On Windows, it uses an equivalent
script named ``build.bat``. To build using these scripts, simply invoke them with the
command-line parameters desired.

For example, for a given architecture (``x64Darwin15clang7.0``), for C++ (traditional
and modern) and Java, the command would be:

.. code-block:: console

   ./build.sh --platform x64Darwin15clang7.0

If you want to build the C# API implementation:

.. code-block:: console

   ./build.sh --cs-build

Learn more about compilation for other platforms and examples in the :ref:`section-compilation` section.

Usage QuickGuide
================

The following two examples show how to run the performance test for two
use cases. Find more examples in :ref:`section-command_examples`.

* The tests below print final results only; if you want to see intermediate values,
  remove the ``-noprint`` argument from the command line.
* If you are running on two unequal machines—that is, one machine is faster (has
  better processors) than the other—you will see better performance by running
  the publisher on the slower machine.

Example 1: 1-to-1, Unicast, Best Latency as a Function of Message Size
----------------------------------------------------------------------

Publisher:

.. code-block:: console

  bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen <length> -latencyTest -executionTime 100

Subscriber:

.. code-block:: console

  bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -multicast

Modify ``-dataLen <bytes>`` to see latencies for different data sizes. Set
``-executionTime <seconds>`` to be >=100 for statistically better results.

Example 2: 1-to-1, Multicast, Maximum Throughput as a Function of Message Size (with Batching)
----------------------------------------------------------------------------------------------

Publisher:

.. code-block:: console

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen <length> -batchSize <bytes> -multicast -executionTime 100

Subscriber:

.. code-block:: console

    bin/<arch>/release/perftest_cpp -sub -noprint -nic <ipaddr> -multicast

To achieve maximum throughput, start by setting ``-batchSize <bytes>`` to 8192,
then increase the size to see if you get better throughput.

.. note::

   Batching will not be enabled if the data length is larger than half the batch size.

Explore the documentation for more information.

Table of Contents
=================

.. toctree::
    :maxdepth: 2
    :numbered:

    introduction
    download
    compilation
    execution
    command_line_parameters
    examples
    tutorials
    tuning_os
    extending_perftest
    compatibility
    release_notes












