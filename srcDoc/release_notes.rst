.. _section-release_notes:

Release Notes
=============

Release Notes 4.0
--------------------

What's New in 4.0
~~~~~~~~~~~~~~~~~~~~

Support for **RTI Connext TSS 3.1.2** |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++

We have added support for **RTI Connext TSS 3.1.2** compiled against *RTI
Connext Pro 6.1.1.4* or against *RTI Connext Micro 2.4.13.4*.

This support has been added for *Linux*, since *RTI Connext TSS 3.1.2* does not
support *Windows* as a host or target platform.

Examples of how to compile *Perftest* for *RTI Connext TSS 3.1.2* can be found
in section :ref:`section-linux_compilation_examples`.

Running *Perftest* against *RTI Connext TSS 3.1.2* is the same as
running it when compiled against *RTI Connext Pro* or *RTI Connext Micro*,
except for some command-line arguments that are available for *RTI Connext Pro*
and/or *RTI Connext Micro*, but that aren't available for *RTI Connext TSS*.
(See :ref:`section-not_available_params_tss`). The argument
``-loaningSendReceive`` has been added to make *Perftest* use
*RTI Connext Pro*/*RTI Connext Micro* loaning functions for receiving and
sending samples, instead of the pure TSS approach that's followed
by default. Find more information on this argument in section
:ref:`section-pubsub_command_line_parameters`.

Support for *RTI Connext DDS 7.0.0* |newTag|
++++++++++++++++++++++++++++++++++++++++++++

We modified *RTI Perftest* to add support for *RTI Connext DDS 7.0.0*, since we
were using some internal APIs to retrieve certain information that have changed
from the previous version.

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

What's Fixed in 4.0
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

Compilation issue when enabling security in static mode |fixedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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

Deprecations in 4.0
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
