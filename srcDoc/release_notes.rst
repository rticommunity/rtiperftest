.. _section-release_notes:

Release Notes
=============

.. raw:: html

    <p style="color:#004C97"; align="centerw"><strong>
    Now you can extend RTI Perftest to work with other ecosystems, use more
    friendly output formats for visualization or comparison, and perform tests
    using your own data as payload!
    </strong></p>

Release Notes Develop
---------------------

What's New in Develop
~~~~~~~~~~~~~~~~~~~~~

Support for *Connext DDS Professional* user data compression |newTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* now supports (in the *Traditional C++ API implementation*) testing
performance scenarios that enable the new compression feature introduced
in *Connext DDS Professional 6.1.0*.

The configuration can be done by using three new command-line options:
``-compressionLevel``, ``-compressionId`` and ``-compressionThreshold``.

See the :ref:`section-command_line_parameters` section for details about the parameters
and the :ref:`section-examples_compression` section for some examples of the usage of
this feature.

This feature is intended to be used in conjunction with the
``-loadDataFromFile`` option to simulate accurate scenarios where the
compression rate can be similar to a real case.

RTI Perftest Modern C++ API now compiles with C++11 |enhancedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The *RTI Perftest* Modern C++ API has been updated to require a C++11 compiler. The
command-line option for the language passed to the ``rtiddsgen`` script is now
``C++11`` instead of ``C++03``. The name of the folder with the code for this API
has been updated to ``srcCpp11`` and the generated executable is now named
``perftest_cpp11`` instead of ``perftest_cpp03``.

Support for Connext DDS Professional's network capture feature |newTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

*RTI Perftest* added support in this version for the network capture capability,
introduced in *Connext DDS Professional 6.1.0*. This support is
exclusive to the *Traditional C++ API implementation*.

In order to control the use of this new option, two new command-line parameters
were added: ``-networkCapture``, to enable the use of the feature and
``-doNotDropNetworkCapture``, to control if the output file produced by the
network capture feature is retained or deleted after the run (due to the nature of *RTI
Perftest*, the size of the file might be huge). See :ref:`section-command_line_parameters` for
more information about the parameters.

Support for new WAN transport |newTag|
++++++++++++++++++++++++++++++++++++++

*RTI Perftest* now supports testing the new WAN transport in the *Tradditional C++
API Implementation*. It can be accessed as a new `-transport` command-line option
argument. See :ref:`section-command_examples` for an example of its
usage. To use this option, the ``-transportPublicAddress`` command-line option
is required on one side, either the *Publisher* or *Subscriber* side.
See :ref:`section-command_line_parameters` for more information.

What's Fixed in Develop
~~~~~~~~~~~~~~~~~~~~~~~

Warning message when using security and a custom governance file was sent to ``stdout`` |fixedTag|
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

When using *RTI Security Plugins* and providing a custom governance
file (using the ``-secureGovernanceFile`` command-line option), a warning message
would appear to make explicit that every security configuration option would be
overridden by the content of the governance file. That message was sent in previous
releases to ``stdout`` instead of ``stderr``. Only data should be sent to
``stdout``, not messages. Sending only data to stdout allows the option
of doing a pipe of the *RTI Perftest* output to a file, obtaining a pure ``.csv`` file.

This issue has been corrected; the message is now sent to ``stderr``.

Fix incorrect schema location in the Governance files used by security |fixedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

All the `Governance files` pointed to a non-existent location for the xsd file.
The reference has been updated and it now points to the right url.

:doc:`Previous Release Notes<./old_release_notes>`
--------------------------------------------------

.. |newTag| image:: _static/new.png
.. |fixedTag| image:: _static/fixed.png
.. |enhancedTag| image:: _static/enhanced.png
