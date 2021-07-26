.. _section-release_notes:

Release Notes
=============

.. raw:: html

    <p style="color:#004C97"; align="centerw"><strong>
    The New C# API is now supported in RTI Perftest! Check the performance improvements
    with respect to the old one! Also, a lot of new improvements and fixes to make
    Perftest even more stable, reliable and configurable.
    </strong></p>

Release Notes Master
--------------------

What's New in Master
~~~~~~~~~~~~~~~~~~~~

Support for the new C# API |newTag|
+++++++++++++++++++++++++++++++++++

*RTI Perftest*'s C# API implementation has been fully rewritten
to support the *RTI Connext DDS* new C# implementation.

The build system now allows building for all supported platform and not only for Windows. The code
has been improved, not only to support the new API, but also to follow the C# coding standards.

The old *Perftest C# API implementation* has been replaced with the new one. In order to test with
that implementation, some changes are required. See the **Using the Old C# Implementation**
section in :ref:`section-compatibility` for more details.

New command-line option for Real-Time WAN Transport to specify host Port |enhancedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* now supports configuring the host port when using Real-Time WAN Transport.

The configuration can be done using the new command-line option:
``-transportHostPort <port>``

This feature is intended to be used in conjuction with the
``-transportPublicAddress`` option for Real-Time WAN Transport.

Improved documentation about configuration settings for *Waitsets* |enhancedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The documentation about ``-waitsetDelayUsec`` and ``-waitsetEventCount`` has been
improved, clarifying the effect in both latency and throughput as well as the
recommended values when performing a latency test.

Switched to C++11 clock implementation in Modern C++ API |enhancedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* for *Modern C++* compilation now requires *C++11* compatibility.
To simplify the *Modern C++* API implementation, *RTI Perftest* now uses the *C++11* clocks, instead
of the ones provided by *RTI Connext DDS*.

This enhancement resolves the issue ``PERF-300``.

Build options for the different APIs are now stackable |enhancedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* has improved the behavior when selecting the APIs to build when using the
the ``build.sh`` and ``build.bat`` scripts.

These options are: ``--cpp-build``, ``--cpp11-build``, ``--java-build`` and ``--cs-build``.
In the past, when providing more than one of these parameters, *RTI Perftest* would build
just the last one provided. Now, the options are stackable, meaning that if you specify
``--cpp-build --cs-build`` both APIs will be compiled one after the other.

This enhancement resolves the issue ``PERF-313``.

What's Fixed in Master
~~~~~~~~~~~~~~~~~~~~~~~

Compiler build option not passed correctly to ``cmake`` when compiling *Connext DDS Micro* |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The compiler build option is used to specify a compiler different than the system
default. This option is useful when you need to cross-compile for an architecture
that is not your build machine's architecture.

This command-line option was passed correctly when using *Connext DDS Professional*
but not when using *Connext DDS Micro*.

``CPUMonitor`` class not correctly protected in *VxWorks* |fixedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Compiling *RTI Perftest* for some *VxWorks* platforms could cause missing symbols when
loading the modules into the kernel. For example:

.. code-block:: console

    -> ld 1 < bin/armv8Vx7SR0660llvm10.0.1/release/perftest_cpp11.so
    Warning: module 0xffff8000008722d0 holds reference to undefined symbol __floatunditf.
    ld(): module contains undefined symbol(s) and may be unusable.

This issue was caused by the ``CPUMonitor`` class, which is not supported in *VxWorks* but
was only partially protected. This issue has been resolved.

``CPUMonitor`` warning not displayed if feature is not requested |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In previous *RTI Perftest* versions, a message was displayed on all platforms
where the ``-cpu`` command-line option was not supported. This message unnecessarily
added to the verbosity for customers testing in these OSes:

.. code-block:: console

    [WARNING] get CPU consumption feature is not available in this OS

Now this warning is displayed only if ``-cpu`` is entered as a command-line option.

Crash in *VxWorks kernel mode* and incorrect behavior when running Perftest multiple times |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In previous versions of *RTI Perftest*, the Traditional and Modern C++ API implementations
failed to run multiple times on *VxWorks* in *kernel mode* if the ``-executionTime``
command-line option was provided. This was due to an issue where some static variables
were initialized when loading the libraries, but not reset when calling the initialization
Therefore, the second run's last value came from the previous run.

This fix resolves the issue ``PERF-301``.

*Modern C++* API implementation not returning loaned memory for samples fast enough |fixedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The *Modern C++* API Implementation for *RTI Perftest* retained loaned samples
for too long after reading them from the *DataReader*. In some scenarios (where
other errors would also be involved), retaining the loaned samples for too long
led to issues deleting the *DataReaders* at the end of the test, showing
errors similar to the following:

.. code-block:: console

    [D0047|Sub(80000009)|T=Latency|DELETE Reader] PRESPsService_destroyLocalEndpointWithCursor:outstanding loans <<<
    [D0047|Sub(80000009)|T=Latency|DELETE Reader] PRESPsService_destroyLocalEndpoint:!delete local reader
    [D0047|Sub(80000009)|T=Latency|DELETE Reader] DDS_DataReader_deleteI:!delete PRESLocalEndpoint
    [D0047|Sub(80000009)|T=Latency|DELETE Reader] DDS_Subscriber_delete_datareader:!delete reader

This fix resolves the issue ``PERF-312``.

Deprecations in Develop
~~~~~~~~~~~~~~~~~~~~~~~

``-scan`` option will be removed in future versions of *RTI Perftest*
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The ``-scan`` command-line option is currently available in the *Traditional C++*,
*Modern C++*, and *Java API* implementations of *RTI Perftest* (not available in the
*Modern C# API implementation*), but in future versions it will be removed.

:doc:`Previous Release Notes<./old_release_notes>`
--------------------------------------------------

.. |newTag| image:: _static/new.png
.. |fixedTag| image:: _static/fixed.png
.. |enhancedTag| image:: _static/enhanced.png
