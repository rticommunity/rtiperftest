.. _section-release_notes:

Release Notes
=============

.. raw:: html

    <p style="color:#004C97"; align="centerw"><strong>
    Now you can extend RTI Perftest to work with other ecosystems, use more
    friendly output formats for visualization or comparison and perform tests
    using your own data as payload!
    </strong></p>

Release Notes Develop
---------------------

What's New in Develop
~~~~~~~~~~~~~~~~~~~~~

RTI Perftest Modern C++ API now compiles with C++11 |enhancedTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

The RTI Perftest Modern C++ API has been updated to require a C++11 compiler. The
command-line option for the language passed to the ``rtiddsgen`` script is now
``C++11`` instead of ``C++03``. The name of the folder with the code for this API
has been updated to ``srcCpp11`` and the generated executable is now named as
``perftest_cpp11`` instead of ``perftest_cpp03``.

Support for RTI Connext DDS Professional's Network Capture Feature |newTag|
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

RTI Perftest added support in this version for the Network Capture capability
feature, introduced in *RTI Connext DDS Professional 6.1.0*. This support is
exclusive to the Traditional C++ API implementation.

In order to control the use of this new option, two new command-line parameters
were added: ``-networkCapture``, to enable the use of the feature and
``-doNotDropNetworkCapture``, to control if the output file produced by the
Network Capture feature is retained or deleted after the run (due to the nature of *RTI
Perftest*, the size of the file might be huge) See :ref:`section-command_line_parameters` for
more information about the parameters.

Support for the new WAN transport |newTag|
++++++++++++++++++++++++++++++++++++++++++

RTI Perftest now supports testing the new WAN transport in the *Tradditional C++
API Implementation*. It can be accessed as a new `-transport` command-line option
argument. See :ref:`section-command_examples` section for an example of its
usage. In order to use this option, the ``-transportPublicAddress`` command-line option
is required in one of the sides, see :ref:`section-command_line_parameters` for
more information.

What's Fixed in Develop
~~~~~~~~~~~~~~~~~~~~~~~

:doc:`Previous Release Notes<./old_release_notes>`
--------------------------------------------------

.. |latestReleaseHeader| image:: _static/Perftest_latest_release_header.png
.. |previousReleasesHeader| image:: _static/Perftest_previous_releases_header.png
.. |newTag| image:: _static/new.png
.. |fixedTag| image:: _static/fixed.png
.. |enhancedTag| image:: _static/enhanced.png
