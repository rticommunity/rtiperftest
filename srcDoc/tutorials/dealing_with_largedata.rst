Dealing with LargeData using RTI Perftest
=====================================================================================

This article is meant to be a quick guide about the initial steps we recommend to follow to profile and
characterize the performance between 2 machines in a given environment when dealing with large data. 
This means understanding the maximum throughput that **RTI Connext DDS Pro** can
achieve on a 1-to-1 communication, as well as the average latency we can expect
when sending packets.

For this demonstration we will use a couple of Raspberry Pi boards, connected to a switch. See below the
information about the environment

   | Target machines: 2 x **Raspberry Pi 2 Model B**
   |                  OS: Raspbian GNU/Linux
   |                  CPU: ARMv7 Processor rev 5 (v7l)
   |                  NIC: 100Mbps
   | Switch: 1Gbps switch

RTI FlatData and Zero Copy
^^^^^^^^^^^^^^^^^^^^^^^^^^

**RTI Connext DDS 6.0.0** came with support for
*RTI FlatData* language binding and Zero Copy transfer over shared memory.
These two new features allow the user of DDS to deal faster with large data
(on the order of MBs) by reducing the number of copies of the object to be sent,
as well as the time needed to serialize and deserialize the object.

As we can see in the image below, when data is sent from a *Data Writer* to a
*Data Reader*, it is copied four times for both UDP and Shared Memory.
The first one is the serialization of the data to be sent from the in-memory
representation of the publisher to a common format suitable for storage or
transmission.

The second one, in case we are using shared memory, is a copy of the serialized
data to the shared memory segment, or a copy made by the socket receive
operation on the *Data Reader* side in case we are using UDP.

The third one is the reassembly of the serialized data after it is received by
the subscriber. And finally, the fourth one is the deserialization of the data
in the subscriber into its in-memory representation.

.. image:: performance_validation_files/DDS_copies.png

In *FlatData* the cost of serialization and deserialization is zero since the
in-memory representation of the data to be sent matches the representation used
to send the data to its destination. Therefore, by using *FlatData*, only two
copies are needed when sharing data across participants.

.. image:: performance_validation_files/FlatData_copies.png

Zero Copy goes one step further reducing the number of copies from four to
zero by sharing a reference to the *Data Writer* memory address that stores
the data to be sent instead of sending the data itself. This does not only reduces
the number of copies, but it also makes the latency independent of the data size.

All this theory sounds good but lets put it into practice and see how *FlatData*
and Zero Copy are able to achieve even better results than the prevously obtained
for **Connext DDS Pro**.

Throughput Test
---------------

We will first run *Perftest* between our two Raspberry Pi comparing *FlatData*
against regular data. Since Zero Copy needs from Shared Memory to be used, we
will not get any results for it with this experiment yet.

* **Publisher side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -pub -nic eth0 -noPrint -exec 23 -flatData -datalen $DATALEN -sendqueuesize 5;
        done

* **Subscriber side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -sub -nic eth0 -noPrint -flatData -datalen $DATALEN -sendqueuesize 5;
        done

See below the output results of executing this test. Again, the information displayed here is
only what the subscriber side showed.

Throughput Results -- FlatData vs Regular Data (UDPv4)
::::::::::::::::::::::::::::::::::::::::::::::::::::::

    .. csv-table::
        :align: center
        :header-rows: 1

        "Size", "Regular Data (Mbps)", "FlatData (Mbps)"
        63001, 95.6,	94
        512000, 96, 96.2
        1048576, 94.9, 96.2
        5242880, 94.3, 97.6
        10485760, 86.7, 94.3
        26214400, 72, 91.2
        52428800, 52.4, 83.9

Explit why sendqueuesize

You will have noticed the new parameter *-sendqueuesize*. It specifies the
initial length for the queue that Connext will allocate for sending samples.
In other words, the send window size.

By default, this value is 50 but since we are sending up to 50MB on this test,
we would need to allocate space for 50 + 1 samples, which are about 2.5 GB, more
than the amount of RAM available on these Raspberry Pi.

As can be seen, the throughput on FlatData is considerable higher than on
regular data due to being able to avoid copying the sample for serialization
and deserialization.

Note that at a first glance, it may appear as if the network were not being
saturated but take into account that for large data we need to wait until the
full sample is received before adding it to the total bytes received used to
calculate the average throughput achieved.

Lets now run the same test but this time on the single machine using Shared
Memory so we can use Zero Copy.

* **Publisher side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -pub -noPrint -transport SHMEM -exec 23 -zerocopy -datalen $DATALEN -sendqueuesize 5  -receivequeue 1;
        done

* **Subscriber side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -sub -noPrint -transport SHMEM -zerocopy -datalen $DATALEN -sendqueuesize 1  -receivequeue 5;
        done

Since Shared Memory resources are even more constrained that RAM on our Raspbian
System, we have to further restrict the amount of memory **RTI ConnextDDS** will
use. We do it by using *-receivequeuesize* which states the amount of samples
allocated on the receive window.

Throughput Results -- Regular Data vs FlatData vs Zero Copy (SHMEM)
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    .. csv-table::
        :align: center
        :header-rows: 1

        "Size", "Regular Data (Mbps)", "FlatData (Mbps), Zero Copy (Mbps)"
        63001, 723.3, 955.6, 2272.3
        512000, 1043.2, 1746.3, 17742.8
        1048576, 949.6, 1426.4, 36648.2
        5242880, 1013.1, 1576.9, 178604.3
        10485760, 1018.4, 1588.5, 359226
        26214400, 959.8, 1543.4, 912976.2
        52428800, 803.6, 1554.4, 1758308

As can be seen, *FlatData* still achieves better performance than regular data,
but Zero Copy outperforms them with a throughput that scales linearly with the
sample size since we are only sending a pointer to the object on the Data
Writer queue.

Note that for both cases explored (UDP and SHMEM) we were highly constrained by
the system and its nic. By using higher-end hardware, we can achieve much better
result. Please refer to the official benchmark page to see more.


Latency Test
------------

* **Publisher side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -pub -nic eth0 -noPrint -exec 23 -flatData -datalen $DATALEN -sendqueuesize 5 -latencyTest;
        done

* **Subscriber side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -sub -nic eth0 -noPrint -flatData -datalen $DATALEN -sendqueuesize 5;
        done

See below the output results of executing this test.

Latency Results -- Regular Data vs FlatData (UDPv4)
:::::::::::::::::::::::::::::::::::::::::::::::::::

    .. csv-table::
        :align: center
        :header-rows: 1

        "Size", "Regular Data (us)", "FlatData (us)"
        63001, 7492, 7423
        512000, 46716, 44441
        1048576, 93981, 89076
        5242880, 461325, 439020
        10485760, 927332, 876613
        26214400, 2313151, 2191383
        52428800, 4633484, 4380210

As for Throughput, the difference between FlatData and regular data is noticebly.
Still, take into account that we are really constrained by our nic so we cannot
see the potential difference; please take a moment to have a look at the
official benchmarks where we are not as constrained as here.

Again, lets run this same test on the single machine using Shared Memory so we
can see how Zero Copy performs.

* **Publisher side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -pub -noPrint -transport SHMEM -exec 23 -zerocopy -datalen $DATALEN -sendqueuesize 5  -receivequeue 1 -latencytest;
        done

* **Subscriber side**

    .. code::

        for DATALEN in 63001 512000 1048576 5242880 10485760 26214400 52428800; do
            bin/armv6vfphLinux3.xgcc4.7.2/release/perftest_cpp -sub -noPrint -transport SHMEM -zerocopy -datalen $DATALEN -sendqueuesize 1  -receivequeue 5;
        done


Latency Results -- Regular Data vs FlatData vs Zero Copy (SHMEM)
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

    .. csv-table::
        :align: center
        :header-rows: 1

        "Size", "Regular Data (us)", "FlatData (us)", "Zero Copy (us)"
        63001, 975, 870, 403
        512000, 5338, 2824, 406
        1048576, 11296, 6276, 411
        5242880, 51181, 26529, 425
        10485760, 99888, 52263, 442
        26214400, 248182, 128278, 412
        52428800, 1294789, 257941, 441

As we can see again ZeroCopy outperform regular data and Flat Data when using
Shared Memory. Furthermore, pay close attention to the average latency; it is
constant no matter the data size!

ZeroCopy should be your default option if communication between shared memory is
available.