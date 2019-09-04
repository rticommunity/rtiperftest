Characterize the performance of Connext DDS in a given environment using RTI Perftest
=====================================================================================

This article is meant to be a quick guide about the initial steps we recommend to follow to profile and
characterize the performance between 2 machines in a given environment. This means understanding the maximum
throughput that **RTI Connext DDS** can maintain in a 1 to 1 communication, as well as the average latency we
can expect when sending packets.

For this demonstration we will use a couple Raspberry Pi boards, connected to a switch. See below the
information about the environment

   | Target machines: **Raspberry Pi 3B+**
   |                  OS:
   |                  NIC: 100Mbps
   | Switch: 1Gbps switch
   | //TODO add info about the machines to use ``

Prepare the tools
~~~~~~~~~~~~~~~~~

In order to perform this test we will only need **RTI Perftest 3.0** (Perftest). We will compile it against **RTI Connext DDS
Professional 6.0.0** and **RTI Connext DDS Micro 3.0.0**.

Get Perftest
^^^^^^^^^^^^

Getting the tool is fairly easy, in fact, you have 3 different ways in which you can access to it:

-  You can clone it from the official *Github* repository:

   Go to the `release_page <https://github.com/rticommunity/rtiperftest/releases>`_ for **Perftest** and
   check what is the latest release, then clone that release using `git`. At this point the latest release is 3.0:

    .. code::

        git clone -b release/3.0 https://github.com/rticommunity/rtiperftest.git

   This command will download the *Github* repository in a folder named
   ``rtiperftest`` and move to the ``release/3.0`` branch.
   If you don't include the ``-b release/3.0``, you will clone the ``master`` branch
   of the product.

-  You can download a `zip` file containing the **Perftest** source files from
   the **Github** page:
   `github.com/rticommunity/rtiperftest <https://github.com/rticommunity/rtiperftest>`__.
   Once the zip file is downloaded you will need to extract its content,
   this will create a folder named ``rtiperftest``.

-  You can download a `zip/tar.gz` file containing the **Perftest** executable statically
   compiled for some of the most common platforms from the **Github** release page:
   `https://github.com/rticommunity/rtiperftest/releases <https://github.com/rticommunity/rtiperftest/releases>`__
   in the "Binaries" section. Once the zip file is downloaded you will need to extract its content, this will
   create a folder with the binaries for your architecture.

All this information is covered in the `download <https://github.com/rticommunity/rtiperftest/blob/3.0/srcDoc/download.rst>`__
section of the **Perftest** documentation.

Compile against RTI Connext DDS Professional 6.0.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

If you already got the compiled binaries, you can skip this step. Else, you will need to compile the
binaries. This process is covered in the `compilation <https://github.com/rticommunity/rtiperftest/blob/3.0/srcDoc/compilation.rst>`__
section of the **Perftest** documentation, so we will just summarize.

We want to build **Perftest** for the *Raspberry Pi* target libraries, this architecture is `armv6vfphLinux3.xgcc4.7.2`, so
we will need to cross-compile this architecture. This process should be really simple with *Perftest*, since we will just need
to have in the `$PATH` environment variable the path to the compiler and linker for the given architecture.

Therefore the command we will need to execute should look like this:

    .. code::

        export PATH=<Path to the compiler and linker for armv6vfphLinux3.xgcc4.7.2>:$PATH
        ./build.sh --platform armv6vfphLinux3.xgcc4.7.2 --nddshome <Path to your nddshome> --cpp-build

Alternatively, you can just point to the compiler and linker using the ``--compiler`` and ``--linker``
command line options. As you can see we also specified the ``--cpp-build`` option, this is because we
are going to use only that executable to test with.

After executing this, you should have a statically linked binary in `./bin/armv6vfphLinux3.xgcc4.7.2/release`,
this is all you should need for your testing.

Compile against RTI Connext DDS Micro 3.0.0
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

This process should be equivalent to the one described in the previous step, and it is also covered
in the `compilation <https://github.com/rticommunity/rtiperftest/blob/3.0/srcDoc/compilation.rst>`__
section of the **Perftest** documentation.

Therefore the command we will need to execute should look like this:

    .. code::

        export PATH=<Path to the compiler and linker for armv6vfphLinux3.xgcc4.7.2>:$PATH
        ./build.sh --micro --platform armv6vfphLinux3.xgcc4.7.2 --rtimehome <Path to your rtimehome>

After executing this, you should have a statically linked binary in `./bin/armv6vfphLinux3.xgcc4.7.2/release`,
this is all you should need for your testing.

Tests
~~~~~

Our goal is to caracterize how *Connext DDS* behaves in the communication between 2 *Raspberry Pi* nodes connected
to one switch. The first thing we will need to know is what is the *minimum latency* and *maximum throughput*
achievable in that environment, independent on using DDS or not. This means that we need to know
how fast can we send with a simple UDP-socket communication. Luckily this is something that we can get with
**Perftest**: By using the `-rawTransport` option, we skip the use of *RTPS* and *DDS* and we
just send using UDPv4 sockets.

We will be doing a *Latency Test* and a *Throughput Test* (See `this <https://github.com/rticommunity/rtiperftest/blob/3.0/srcDoc/introduction.rst#latency-test-vs-throughput-test>`__ section to understand the
differences).

Once that is done, we will have a baseline which is going to tell us the minimum latency we can expect
and the maximum throughput achievable in the system, even when not using *RTPS* and *DDS*. The next step
is then to execute **Perftest** compiled for *Professional* and for *Micro* and see the equivalent results.

UDPv4 Communication (Raw Transport)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Throughput Test
---------------

The maximum throughput of this scenario will be limited by several factors: If the size of the samples we
are sending is small, the CPU consumption will be high, since it will need to iterate through the process
of sending the samples to the NIC quite often. If the size of the sample is big enough, then the problem
is the physical limitations of the network itself, how fast the nics and the switch are.

In our specific case, the switch is a 1Gbps switch which should not be the cap, since the *Raspberry Pi* nics
we are using have 100Mbps NICs. Then, 100Mbps is our maximum theoretical throughput.

Given all this information, the right way to perform the test is by iterating through different data sizes. We
will use the following commands:

* **Publisher side**

    .. code::

        for DATALEN in 32 64 128 256 512 1024 2048 8192 16384 32768 63000; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -pub -peer 10.45.3.119 -nic eth0 -raw -pub -noPrint -exec 20 -datalen $DATALEN -batchSize 0;
        done

* **Subscriber side**

    .. code::

        for DATALEN in 32 64 128 256 512 1024 2048 8192 16384 32768 63000; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -sub -peer 10.45.3.120 -nic eth0 -raw -noPrint -datalen $DATALEN;;
        done

Some comments about the parameters we used:

* In `Raw Transport Mode` the `-scan` option is not available, that is why we need to iterate through
  the different data sizes using a for loop (in `bash`).

* In `Raw Transport Mode` we do not have a discovery mechanishm, as we do have when
  Using **RTI Connext DDS**, therefore, it is required to use the `-peer` parameter.

* In throughput mode, by default **Perftest** uses "batching", we want to disable it for this
  test, that can be done by using `-batchSize 0`.

See below the output results of executing this test. The information displayed here is
only what the subscriber side showed, since all the information displayed in the publisher
side is related to latency not about throughput.

Throughput Results-- RAW Transport (UDPv4)
::::::::::::::::::::::::::::::::::::::::::

    .. csv-table::
        :align: center
        :header-rows: 1

        "Size", "Packets", "Packets/s (ave)", "Mbps (ave)", "Lost", "Lost (%)"
        32, 503906, 25193, 6.4, 975, 0.19
        64,454201,22697,11.6,1608,0.35
        128,465202,23259,23.8,1170,0.25
        256,454120,22706,46.5,12466,2.67
        512,400530,20043,82.1,7027,1.72
        1024,223798,11191,91.7,4718,2.06
        2048,114800,5737,94.0,119,0.10
        8192,29247,1461,95.8,4,0.01
        16384,14446,722,94.6,0,0.00
        32768,7307,365,95.7,3,0.04
        63000,3819,190,96.2,0,0.00

Latency Test
------------

Now we want to measure the minimum latency we can expect in the system when the network
is not saturated, this can be done again with **Perftest**, doing a "Latency Test". In order
to do that, you only need to add `-latencyTest` to the previous command line parameters of the
publisher side.

* **Publisher side**

    .. code::

        for DATALEN in 32 64 128 256 512 1024 2048 8192 16384 32768 63000; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -pub -peer 10.45.3.119 -nic eth0 -raw -pub -noPrint -exec 20 -datalen $DATALEN -latencyTest;
        done

* **Subscriber side**

    .. code::

        for DATALEN in 32 64 128 256 512 1024 2048 8192 16384 32768 63000; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -sub -peer 10.45.3.120 -nic eth0 -raw -noPrint -datalen $DATALEN;;
        done

Remember that in this case we are interested in the latency results, not in the
throughput results (we are doing a ping-pong test, so we cannot expect high throguhput),
therefore we need to look to the results displayed in the publisher side.

Latency Results -- RAW Transport (UDPv4)
::::::::::::::::::::::::::::::::::::::::

    .. csv-table::
        :align: center
        :header-rows: 1

        "Size", "Ave (us)", "Std (us)", "Min (us)", "Max (us)", "50% (us)", "90% (us)", "99% (us)", "99.99% (us)", "99.9999% (us)"
        32,357,77.7,310,6094,355,371,470,5436,6094
        64,370,76.5,305,3935,365,387,491,3693,3935
        128,386,88.3,318,6573,381,403,512,5549,6573
        256,419,82.0,360,6451,416,438,546,4810,6451
        512,485,72.5,435,5913,479,503,610,4571,5913
        1024,608,96.5,545,6507,602,633,757,6435,6507
        2048,809,102.2,736,5605,797,845,994,5318,5605
        8192,1412,106.1,1325,5969,1400,1456,1608,5969,5969
        16384,2107,222.5,1931,9573,2096,2153,2338,9573,9573
        32768,3693,223.2,3477,8656,3696,3768,4046,8656,8656
        63000,6601,212.9,6424,10706,6595,6752,7002,10706,10706