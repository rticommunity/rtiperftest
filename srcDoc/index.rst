.. raw:: html

    <p style="color:#004C97"; align="center"><font size="+12">RTI Perftest</font></p>

Overview
========

**RTI Perftest** is a command line tool intended to measure the minimum latency,
maximum throughput and loaded latency in a very configurable set of scenarios.
It can help you answer questions such as:

* For a given publishing configuration (e.g., queue size, batching settings), sample size, and subscribing configuration (e.g., queue size, Listener vs. WaitSet), what is the throughput of my network?
* When my network is heavily loaded, what latency can I expect?
* For a given configuration, what is the best-case latency with no other traffic on the network?

How does it work
----------------

The publishing side of the test writes data as fast as it can. Every few samples
(configured through the command line), it sends a special sample requesting an echo
from the subscribing side. It uses this ping -> pong exchange to measure the
round-trip time latency (RTT) and then the one-way latency (RTT/2).

The publisher prints the latency test results while the subscriber prints the throughput results.

.. figure:: _static/PerfTest_Overview_Diagram.png
   :alt: PerfTest Overview Diagram

**RTI Perftest** allows 2 operational modes: *Throughput Test* and *Latency Test*.

Learn more in the :ref:`section-introduction` section.

Features
--------

**RTI Perftest** features include:

- Supports **RTI Connext DDS Professional** and **RTI Connext DDS Micro**.
- Suppots multiple platform including Windows, Linux, Mac, VxWorks or Android.
- Supports multiple test setups - Multicast, One-to-many communication (Pub -> Sub), Latency test, Throughput test, Latency vs Throughput test.
- Supports using raw transports (UDPv4 Sockets and Shared Memory segments).
- Suports benchmarking Connext Pro and Micro features such as Flat Data and Zero Copy.

Getting Started
---------------

This is an example of how you may give instructions on setting up your project locally.
To get a local copy up and running follow these simple example steps.

Download
++++++++

**RTI Perftest** bundle is provided in 3 different ways:

1. Clone and compile from the official Github repository:

.. code-block:: console

   git clone https://github.com/rticommunity/rtiperftest.git

2. Download and compile from: `https://github.com/rticommunity/rtiperftest
<https://github.com/rticommunity/rtiperftest>`_.

3. Download the executables for popular platforms from the binaries section in the
**RTI Perftest** release page `here
<https://github.com/rticommunity/rtiperftest/releases>`_.

To learn more about the supported platforms and installation, please refer to the :ref:`section-download` section.

Requisites
++++++++++

If you need to compile **RTI Perftest** these are the 2 main requisites. This is not
needed if you downloaded the executables already compiled, you can skip this section.

- **RTI Connext DDS Professional** or **RTI Connext DDS Micro** should be installed
  in the system where the build.sh script is going to run. The Target libraries for
  the platform to be generated should also be installed.
- The `$NDDSHOME` environment variable should be set correctly. Alternatively,
  `$NDDSHOME` can be passed directly to the `build.sh` script by using the `--nddshome <PATH>` command-line option.
- If you intend to compile and test using *RTI Security Plugins*, you will need to
  link against the OpenSSL libraries for your architecture.

Learn more in the :ref:`section-compilation` section.

Compile
+++++++

For Linux, MacOS, QNX, VxWorks, Lynx, and Android, **RTI Perftest** makes use of a script in the top-level directory named ``build.sh``.
On Windows it uses an equivalent script named ``build.bat``. To build using these scripts, simply invoke them with the
command-line parameters desired.

For example, for a given architecture (``x64Darwin15clang7.0``) for C++ (traditional and modern) and Java, the command would be:

.. code-block:: console

   ./build.sh --platform x64Darwin15clang7.0

If you want to build the C# API implementation:

.. code-block:: console

   ./build.sh --cs-build

Learn more about compilation for other platforms and examples in the :ref:`section-compilation` section.

Usage
-----

The following are examples of how to run the performance test for two use cases:

* The tests below print final results only; if you want to see intermediate values, remove the ``-noprint`` argument from the command line.
* If you are running on two unequal machines---i.e., one machine is faster (has better processors) than another---you will see better performance by running the Publisher on the slower machine.

Example 1: 1-to-1, Unicast, Best Latency as a Function of Message Size
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Publisher:

.. code-block:: console

  bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen <length> -latencyTest -executionTime 100

Subscriber:

.. code-block:: console

  bin/<arch>/release/perftest_cpp -sub -noPrint -nic <ipaddr> -domain <ID> -multicast

Modify `-dataLen <bytes> ` to see latencies for different data sizes. Set `-executionTime <seconds>`
to be >=100 for statistically better results.

Example 2: 1-to-1, Multicast, Maximum Throughput as a Function of Message Size (with Batching)
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Publisher:

.. code-block:: console

    bin/<arch>/release/perftest_cpp -pub -noPrint -nic <ipaddr> -dataLen <length> -batchSize <bytes> -multicast -executionTime 100

Subscriber:

.. code-block:: console

    bin/<arch>/release/perftest_cpp -sub -noprint -nic <ipaddr> -multicast

To achieve maximum throughput, start by setting See ``-batchSize <bytes>`` to 8192, then increase the size to see if you get better throughput.

*Note: Batching will not be enabled if the data length is larger than 1/2 the batch size.*

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












