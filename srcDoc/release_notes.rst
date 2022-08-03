.. _section-release_notes:

Release Notes
=============

.. raw:: html

    <p style="color:#004C97"; align="centerw"><strong>
    The New C# API is now supported in RTI Perftest! Check the performance improvements
    with respect to the old one! Also, a lot of new improvements and fixes to make
    Perftest even more stable, reliable and configurable.
    </strong></p>

Release Notes Develop
---------------------

What's New in Develop
~~~~~~~~~~~~~~~~~~~~~

Support for **wolfSSL** for *Linux* and *QNX* when using the *Security Plugins* |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*Perftest* now adds support for *wolfSSL* in addition to *OpenSSL* when using
*RTI Connext DDS Secure 6.1.1*. This support has only been added for *Linux*
and *QNX* platforms, as these are the ones supported by *RTI Connext DDS Secure 6.1.1*.

A new parameter (``--wolfSSL-home``) has been added to the compilation script in order
to be able to specify the location of the libraries when compiling statically.

New command-line option to output data to a file |newTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*Perftest* has added a new command-line parameter, ``-outputFile``. This parameter
enables you to output the performance data to a file instead of printing it on the screen.

The output content is the same as when using the ``>`` or ``>>``,
options in the command-line: the performance data is sent to the specified file,
while the summary information and errors are still printed on the screen.

Set default Encryption Algorithm to aes-128-gcm |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In previous versions *Perftest* would use the default value for the Encryption
algorithm when using *RTI Connext DDS Secure*. However, starting in 7.0.0. The
default value has been upgraded to `aes-256-gcm`. While this new value offers a
higher level of security, `aes-128-gcm`remains being secure and slightly more
efficient CPU/Performance wise. For that reason as well as for comparison with
previous versions of *RTI Connext DDS Secure*, *Perftest* has adopted that new
value.

In addition, a new parameter (``-secureEncryptionAlgorithm``) has been added to support
manually setting the desired value.

What's Fixed in Develop
~~~~~~~~~~~~~~~~~~~~~~~

Unclear table output headers |enhancedTag|
++++++++++++++++++++++++++++++++++++++++++

The output headers displayed by *Perftest* during and after the test have been updated to
show a clearer description of the content of the tables.

In addition, when the ``-noPrintIntervals`` option is used, the header is a single line, which
simplifies parsing it later on.

Bug in C# API when testing with large data sizes and unbounded types |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

A bug in *Perftest*'s C# API implementation made it impossible
to test using large data types (``dataLen`` larger than ``65470`` bytes) or when forcing
the use of unbounded sequences (``-unbounded``). This problem has been fixed.

Compilation issue when enabling security in static mode|fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

When trying to compile statically (default behavior), *Perftest*
would try to find and link against the Openssl libraries ``cryptoz`` and
``sslz``; however, these names are no longer used. The right names of the
libraries are ``crypto`` and ``ssl``.

Fixed warning in Modern C++ implementation |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The following warning may appear when compiling *Perftest*'s Modern C++ API implementation:

.. code-block:: console

    RTIDDSImpl.cxx: In instantiation of 'void RTIPublisherBase<T>::wait_for_ack(long int, long unsigned int) [with T = rti::flat::Sample<TestDataLarge_ZeroCopy_w_FlatData_tOffset>]':
    RTIDDSImpl.cxx:595:10:   required from here
    RTIDDSImpl.cxx:600:15: warning: catching polymorphic type 'const class dds::core::TimeoutError' by value [-Wcatch-value=]
    600 |             } catch (const dds::core::TimeoutError) {} // Expected exception
        |               ^~~~~

This warning has been fixed.

Fixed unhandled exception in Modern C++ API implementation |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

When using *Perftest*'s Modern C++ API implementation with the ``-bestEffort`` command-line option 
an unhandled exception might be raised if a sample wasn't answered before a certain ammount of time
(which could happen if the sample was lost or coudn't be replied). This exception was caught at the ``main()``
level, stopping the flow of the program, however it should simply be ignored (and treat the failure as a sample lost).
This issue has been corrected.

Issue compiling Connext DDS Micro on Windows |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++

The build scripts for *Windows* (``build.bat``) failed with the following error when trying to compile *Perftest*
against *RTI Connext DDS Micro*. The error displayed was:

.. code-block:: console

    CMake Error: Unknown argument --target
    CMake Error: Run 'cmake --help' for all supported options.

This problem has been fixed.

Clock skews caused publisher side to hang |fixedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++

Clock skews caused the publisher side to hang. This problem was usually
seen in operating systems (such as VxWorks) with low-resolution clocks.

This problem has been resolved.

Release Notes 3.3
--------------------

What's New in 3.3
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

What's Fixed in 3.3
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
