.. _section-release_notes:

Release Notes
=============

Release Notes 4.1.1
-------------------

What's Fixed in 4.1.1
~~~~~~~~~~~~~~~~~~~~~~~

Terminated connection in VxWorks after finishing test for *Traditional* and *Modern* C++ |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

When using the *Traditional* or *Modern* C++ API implementations of *RTI Perftest* in *VxWorks*,
the connection would be terminated when the Publisher and Subscriber sides were deleting the
entities to finish the test.

The problem would reproduce only when using *stdout* and not if using a file to output the
results of the test.

This issue has been fixed.

Updated property names for *RTI Connext DDS LightWeight Security* PSK |fixedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The property names for the *RTI Connext DDS LightWeight Security* PSK have been updated to
match the new names used in *RTI Connext DDS 7.3.0*.

Release Notes 4.1
---------------------

What's New in 4.1
~~~~~~~~~~~~~~~~~~~~~

Support for the LightWeight Security library |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In this release we added support for the *RTI Connext LightWeight Security Library*.

If *RTI Perftest* is compiled dynamically (``--dynamically`` in the build scripts ``build.sh`` / ``.bat``),
then no new parameters are needed. However, when compiling statically, a new command line
option has been added to the build scripts (``--lightWeightSecurity``) to force the linking
against that library instead of the regular (Full) *RTI Connext Secure* library.

Default value for `openssl` if none is provided |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In previous releases, when compiling statically against *RTI Connext* with the *Security Libraries*,
a path to the ssl crypto libraries had to be provided (*openSSL* or *WolfSSL*).
Starting in this release, if no path is provided, *RTI Perftest* will try to search
in the *RTI Connext* Installation (``$NDDSHOME``), in the default folder where the *OpenSSL Libraries* bundle is installed.

Provide `ssl` version |newTag|
++++++++++++++++++++++++++++++

If more than one set of cryptography libraries or versions (`openSSL` or `wolfSSL`) are found
in the *RTI Connext DDS* Middleware installation, by using the the `--openssl-version`
parameter, you can select the desired one.

Secure parameters have been simplified |newTag|
+++++++++++++++++++++++++++++++++++++++++++++++

In previous releases, *Perftest* had the option of building the name of the
security governance file based on several command-line options. These options
would determine if the governance would be signed, the kind of RTPS protection,
DATA protection, etc. These options required having a governance file
for every combination of security options.

*Perftest* now uses a simpler option (also present in previous releases),
``-secureGovernanceFile``, so that *Perftest* can be directly pointed to the file to
use.

Added Command-Line Option to enable AAD |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++

Starting in this new release, *Perftest* has a new Command-Line Option: ``-secureEnableAAD``
which sets the right property to enable the "Additional Authenticated Data".


New option to save all latency times into a .csv file |newTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The *RTI Perftest* publisher side has a new command-line option: ``--latencyFile <file>``.
This option will, at the end of the test, save all the time values obtained for all the latency
samples (those samples for which *Perftest* calculates  the Round-Trip Time) into a file.

You should use this option when all the latency time values are required and the final
summary information is not enough.

Use this option in conjunction with ``--noPrint`` when doing
a latency test (``--lantecyTest``) or when latency samples are printed very often on the
publisher side, since the printing operation is more costly and may affect the result of the
test.

Find more information in the :ref:`Test Parameters only for Publishing Applications` section.


What's Fixed in 4.1
~~~~~~~~~~~~~~~~~~~~~~~

Issue when using multicast in rawTransport mode |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

In previous releases, when using multicast and raw transport in a multi-subscriber scenario,
only the subscriber with ID 0 would receive the packets correctly since the receive port was
incorrectly calculated. This issue has been fixed.

Error in C++11, C#, and Java when using security |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The following error could appear when using the *C++11*, *C#*, or *Java* API
implementations and enabling the *Security Plugins*:

```
[CREATE Participant] RTI_Security_Cryptography_register_participant:inconsistent configuration: protection_kind has WITH_ORIGIN_AUTHENTICATION, but cryptography.max_receiver_specific_macs < 2
[CREATE Participant] DDS_DomainParticipantTrustPlugins_getLocalParticipantSecurityState:!security function register_local_participant returned NULL
[CREATE Participant] DDS_DomainParticipant_createI:!get local participant security state
[CREATE Participant] DDS_DomainParticipantFactory_create_participant_disabledI:!create participant
```

This issue would not happen when using the Traditional *C++* implementation. The issue has been resolved.

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
default value has been upgraded to ``aes-256-gcm``. While this new value offers a
higher level of security, ``aes-128-gcm`` remains being secure and slightly more
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
