.. _section-tuning_os:

Optimizing Your OS For Network Performance
==========================================

The network stacks of popular operating systems are not always tuned for
maximum performance out of the box. RTI has found that the following
configuration changes frequently improve performance for a broad set of
demanding applications. Consider testing your network performance with
and without these changes to learn if they can benefit your system.

Optimizing Linux Systems
************************

Edit the file ``/etc/sysctl.conf`` and add the following:

::

    net.core.wmem_max = 16777216
    net.core.wmem_default = 131072
    net.core.rmem_max = 16777216
    net.core.rmem_default = 131072
    net.ipv4.tcp_rmem = 4096 131072 16777216
    net.ipv4.tcp_wmem = 4096 131072 16777216
    net.ipv4.tcp_mem = 4096 131072 16777216

    net.core.netdev_max_backlog = 30000
    net.ipv4.ipfrag_high_threshold = 8388608

    run /sbin/sysctl -p

More information about Linux OS tuning can be found in `RTI's Community Portal
<https://community.rti.com/howto/improve-rti-connext-dds-network-performance-linux>`__.

Optimizing Windows Systems
**************************

1. From the Start button, select Run..., then enter ``regedit``.

2. Change this entry:
   ``HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\ Services\Tcpip\Parameters``

   -  Add the ``DWORD`` key: ``MaximumReassemblyHeaders``
   -  Set the value to ``0xffff`` (this is the max value).

3. Change this entry:
   ``HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\ Services\AFD\Parameters``

   -  Add the ``DWORD`` key: ``FastSendDatagramThreshold``
   -  Set the value to ``65536`` (``0x10000``).

4. Reboot your machine for the changes to take effect.

Read the following articles in RTI's Community portal to see more possible
improvements for your Windows system

- `Improve Windows performance under heavy load <https://community.rti.com/kb/how-can-i-improve-performance-windows-under-heavy-load>`__.
- `Reduce latency in Windows by disabling the Interrupt coalescing <https://community.rti.com/kb/how-reduce-latency-windows-disabling-interrupt-coalescing>`__.