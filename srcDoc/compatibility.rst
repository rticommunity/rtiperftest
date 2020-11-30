.. _section-compatibility:

Compatibility
=============

*RTI Perftest 3.1* is designed to be compatible with the *RTI Connext DDS*
middleware. It has been compiled and tested against:

- *RTI Connext DDS Professional* 6.0.0 and above. Nonetheless previous versions
  starting after 5.2.X should be compatible with certain exceptions.

- *RTI Connext DDS Micro* 3.0.0 and above versions. Versions for 2.4.11 and above
  should also be compatible by adding the `--micro-24x-compatibility` command-line
  option.

Using Security
~~~~~~~~~~~~~~

Governance and Permission files have been updated to be compatible with
the latest release for *RTI Connext DDS*, and are compatible with *RTI
Connext DDS Professional* 5.2.7 and greater.

If you are compiling *RTI Perftest* against 5.2.5, you will need to get
the certificates from the ``release/2.0`` branch. You can do that by
using the following git command from the top-level directory of your
repository:

::

    git checkout release/2.0 -- resource/secure

*RTI Connext DDS 6.0.1 Security Plugins* changes
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

In order to add compatibility for *RTI Connext DDS 6.0.1* including the
*Security Plugins* on Windows, a change that breaks compatibility was made:

When building a Windows application, *RTI Perftest* must now link against
``libssl`` and ``libcrypto`` instead of ``ssleay32`` and ``libeay32``, as documented
`here <https://community.rti.com/static/documentation/connext-dds/6.0.1/doc/manuals/migration_guide/601/product601/security601.html>`__.

This breaks compatibility against previous versions of *RTI Connext DDS*. In order
to compile against previous versions a manual change has to be made in the `build.bat`
script:

Search and replace all references for ``crypt32 libcryptoz libsslz`` with ``libeay32z ssleay32z``.

Compilation Restrictions
~~~~~~~~~~~~~~~~~~~~~~~~

*RTI Perftest* is designed to compile and work against the *RTI Connext
DDS* 5.2.x, 5.3.x and 6.0.0 releases.

Certain features, however, are not compatible with all the *RTI Connext
DDS* versions, since the build scripts make use of certain specific
parameters in *rtiddsgen* that might change or not be present between
releases:

-  The ``--secure`` and ``--openssl-home`` parameters will not work for
   versions prior to *RTI Connext DDS* 5.2.5.

-  Java code generation against *RTI Connext DDS 5.2.0.x* will fail
   out-of-the-box. You can disable this by adding the ``--skip-java-build``
   flag. See the Known Issues section for more information and
   alternatives.

-  C# code generation against *RTI Connext DDS 5.2.0.x* is not
   supported. You can disable this by adding the ``--skip-cs-build``
   flag.


Known Issues
~~~~~~~~~~~~

Compilation Errors in Microsoft Visual Studio 2017 Express
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Due to `this issue <https://community.rti.com/static/documentation/connext-dds/6.0.0/doc/manuals/connext_dds/code_generator/html_files/RTI_CodeGenerator_ReleaseNotes/index.htm#code_generator/ReleaseNotes/KnownIssues/Known_Issues.htm?Highlight=RTI_VS_WINDOWS_TARGET_PLATFORM_VERSION>`__
documented in the Know Issues for *RTI Connext DDS*, when compiling with
*Visual Studio 2017 Express*, you need to set the `RTI_VS_WINDOWS_TARGET_PLATFORM_VERSION`
as follows to avoid compilation errors:

::

    set RTI_VS_WINDOWS_TARGET_PLATFORM_VERSION=10.0.16299.0

[RTI Issue ID CODEGENII-800]

Shared Memory issues when running the Modern C++ API or .Net Implementation
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* uses `UDPv4` and `SHMEM` by default; however certain operating
systems don't support Shared Memory, or the default configuration is not enough for
*RTI Connext DDS* to work properly. In these cases *RTI Perftest* will show
errors when trying to create the Participant entity:

::

    [D0001|ENABLE]NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key 0x40894a
    [D0001|ENABLE]NDDS_Transport_Shmem_create_recvresource_rrEA:failed to initialize shared memory resource segment for key 0x40894c
    [D0001|ENABLE]DDS_DomainParticipantPresentation_reserve_participant_index_entryports:!enable reserve participant index
    [D0001|ENABLE]DDS_DomainParticipant_reserve_participant_index_entryports:Unusable shared memory transport. For a more in-depth explanation of the possible problem and solution, please visit http://community.rti.com/kb/osx510.
    [D0001|ENABLE]DDS_DomainParticipant_enableI:Automatic participant index failed to initialize. PLEASE VERIFY CONSISTENT TRANSPORT / DISCOVERY CONFIGURATION.
    [NOTE: If the participant is running on a machine where the network interfaces can change, you should manually set wire protocol's participant id]
    DDSDomainParticipant_impl::createI:ERROR: Failed to auto-enable entity

These errors are handled and filtered in the *RTI Perftest* implementation for
the *Traditional* C++ and Java APIs, but this is still not possible for the
*Modern* C++ and .Net API.

For more information about how to configure Shared Memory, see http://community.rti.com/kb/osx510.

If you want to skip the use of Shared Memory in *RTI Perftest*, specify the transport using `-transport <kind>`, for example, `-transport UDPv4`.

Warning when compiling the *Traditional* C++ API Implementation
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* might show these warnings when compiling the *Traditional* C++
API implementation for *RTI Connext DDS Pro* (in versions prior to 6.0.0) and
for *RTI Connext DDS Micro*:

::

    In file included from perftestSupport.h:15:0,
                    from perftestSupport.cxx:11:
    perftest.h:29:25: warning: ‘THROUGHPUT_TOPIC_NAME’ defined but not used [-Wunused-variable]
    static const DDS_Char * THROUGHPUT_TOPIC_NAME= "Throughput";
                            ^
    perftest.h:30:25: warning: ‘LATENCY_TOPIC_NAME’ defined but not used [-Wunused-variable]
    static const DDS_Char * LATENCY_TOPIC_NAME= "Latency";
                            ^
    perftest.h:31:25: warning: ‘ANNOUNCEMENT_TOPIC_NAME’ defined but not used [-Wunused-variable]
    static const DDS_Char * ANNOUNCEMENT_TOPIC_NAME= "Announcement";
                            ^

These warnings are the result of a known issue in *RTI Code Generator (rtiddsgen)* (CODEGENII-873) related to the way in which
the code for a const string is generated. This issue will be fixed in future releases of *RTI Connext DDS Micro* and has been
already fixed for *RTI Connext DDS Pro* 6.0.0.


Building RTI Perftest Java API against RTI Connext DDS 5.2.0.x
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Due to the changes added to support larger data sizes, *RTI
Perftest* now makes use of *Unbounded Sequences*. This feature was not
added to *RTI Connext DDS* in *5.2.0.x*, so the following error will be
reported when trying to compile the Java API:

::

    [INFO]: Generating types and makefiles for java.
    [INFO]: Command: "/home/test/nevada/bin/rtiddsgen" -language java -unboundedSupport -replace -package com.rti.perftest.gen -d "/home/test/test-antonio/srcJava" "/home/test/test-antonio/srcIdl/perftest.idl"
    ERROR com.rti.ndds.nddsgen.Main Fail:  -unboundedSupport is only supported with C, C++, C++/CLI, or C# code generation
    rtiddsgen version 2.3.0
    Usage: rtiddsgen [-help]
    . . .
    INFO com.rti.ndds.nddsgen.Main Done (failures)
    [ERROR]: Failure generating code for java.

To avoid this compilation error, two changes are needed:

-  In the ``build.sh`` or ``build.bat`` scripts, modify the call for
   *rtiddsgen* and remove the ``-unboundedSupport`` flag.

-  In the ``srcIdl/perftest.idl`` file, modify the ``TestDataLarge_t``
   and ``TestDataLargeKeyed_t`` types, and add a bound to the
   ``bin_data`` member: ``sequence<octet,LIMIT> bin_data;``.

Publication rate precision on Windows systems when using "sleep" instead of "spin"
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

When using the ``-pubRate <#>:sleep`` or ``-sleep`` command-line
parameters on Windows systems, the ``sleep()`` precision will be accurate
up to 10 milliseconds. This means that for publication rates of more
than 10,000 samples per second we recommend using the "<#>:spin" option
instead.

Compiling manually on Windows systems when using the *RTI Security* plugin
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*rtiddsgen*-generated solutions for Windows systems allow four different
configurations:

-  Debug
-  Debug DLL
-  Release
-  Release DLL

The new *RTI Perftest* build system, however, is focused on compiling
only one of those modes at a time. To choose the compilation mode,
use the ``-debug`` and ``-dynamic`` flags.

Warnings Compiling on Windows systems when using the *RTI Security* plugin
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

We have found that in certain instalations of *Openssl* a missing `pdb` file
causes several warnings when compiling statically *RTI Perftest* for C++
(Traditional and Modern implementations). The warning that will show should be
similar to this one:

::

    libeay32z.lib(wp_block.obj) : warning LNK4099: PDB 'lib.pdb' was not found with
    'libeay32z.lib(wp_block.obj)' or at 'rtiperftest\srcCpp03\objs\i86Win32VS2015\lib.pdb';
    linking object as if no debug info [srcCpp03\perftest_publisher-i86Win32VS2015.vcxproj]

    403 Warning(s)
    0 Error(s)

This warning should be innocuous.

Dynamic compilation modes for *RTI Connext DDS Micro*
+++++++++++++++++++++++++++++++++++++++++++++++++++++

When building against the *RTI Connext DDS Micro* libraries, only the static
compilation modes are supported. Therefore the ``--dynamic`` option will have
no effect.

``rtiddsgen`` code generator will fail with the following message: ``Option
-sharedLib is not supported by this version of rtiddsgen``.
