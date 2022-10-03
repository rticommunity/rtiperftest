.. _section-download:

Download
========

Supported Platforms
-------------------

*RTI Perftest* makes use of the *RTI Code Generator* (*rtiddsgen*) tool
to generate part of its code. It uses the makefile/project files
created by *Code Generator* to compile its code.

*RTI Perftest* scripts work for every platform for which *rtiddsgen*
can generate an example, except for those in which *rtiddsgen* doesn't
generate regular makefiles or *Visual Studio Solutions* but specific
project files, as is the case with *Android* and *iOS* platforms. See
the `Core Libraries Release Notes, Supported Operating Systems <https://community.rti.com/static/documentation/connext-dds/6.1.0/doc/manuals/connext_dds_professional/release_notes/index.htm#release_notes/System_Requirements.htm>`_, for a list of
supported platforms.

Some platforms compile with *RTI Perftest's* out of-the-box code and
configuration, but require further tuning to make the
application run on the platform, usually because of the application's
memory consumption or the platform's lack of support for certain features
(like a file system).

.. note::

   The compilation of *RTI Perftest* C# API for Windows using
   *Microsoft Visual Studio 2008 Express* or *2010 Express* is not
   supported.

Download instructions
---------------------

Download the *RTI Perftest* bundle in one of three ways:

-  Clone it from the official `GitHub <https://github.com/rticommunity/rticonnextdds-getting-started>`_ repository:

   ``git clone -b master https://github.com/rticommunity/rtiperftest.git``

   This command downloads the GitHub repository into a folder named
   ``rtiperftest`` and moves the repository to the ``master`` tag.

..

-  Download a Zip file containing the *RTI Perftest* source files from
   the *RTI Perftest* GitHub page:
   `github.com/rticommunity/rtiperftest <https://github.com/rticommunity/rtiperftest>`__.
   Extract the Zip file's contents; they will go into a folder named ``rtiperftest``.

..

-  Download a ``.zip/.tar.gz`` file containing the *RTI Perftest* executable statically
   compiled for some of the most common platforms, from the *RTI Perftest* GitHub release page:
   `https://github.com/rticommunity/rtiperftest/releases <https://github.com/rticommunity/rtiperftest/releases>`__ (in the "Binaries" section).
   Extract the file's contents; they will go into a folder with the binaries for your
   architecture.

If you need help with the download and installation process, contact `support@rti.com <support@rti.com>`__.
