.. _section-download:

Download
========

Supported Platforms
-------------------

*RTI Perftest* makes use of the *RTI Connext DDS* *Rtiddsgen* tool in
order to generate part of its code and also the makefile/project files
used to compile that code.

*RTI Perftest* scripts works for every platform for which *Rtiddsgen*
can generate an example, except for those in which *Rtiddsgen* doesn't
generate regular makefiles or *Visual Studio Solutions* but specific
project files. That is the case of *Android* platforms as well as the
*iOS* ones.

Certain platforms will compile with the out of-the-box code and
configurations, but further tuning could be needed in order to make the
application run in the specific platform. The reason is usually the
memory consumption of the application or the lack of support of the
platform for certain features (like a file system).

**Note:** The compilation of *RTI Perftest* C# API for Windows using
*Microsoft Visual Studio 2008 Express* or *2010 Express* is not
supported.

Download instructions
---------------------

*RTI Perftest* bundle is provided in 3 different ways:

-  You can clone it from the official *Github* repository:

   ``git clone -b master https://github.com/rticommunity/rtiperftest.git``

   This command will download the *Github* repository in a folder named
   ``rtiperftest`` and move to the ``master`` tag.


-  You can download a zip file containing the *RTI Perftest* source files from
   the *RTI Perftest* *Github* page:
   `github.com/rticommunity/rtiperftest <https://github.com/rticommunity/rtiperftest>`__.
   Once the zip file is downloaded you will need to extract its content,
   this will create a folder named ``rtiperftest``.


-  You can download a zip/tar.gz file containing the *RTI Perftest* executable statically
   compiled for some of the most common platforms from the *RTI Perftest Github* release page:
   `https://github.com/rticommunity/rtiperftest/releases <https://github.com/rticommunity/rtiperftest/releases>`__ in the "Binaries" section.
   Once the zip file is downloaded you will need to extract its content, this will create a folder
   with the binaries for your architecture.

If you need help with the download and installation process, contact `support@rti.com <support@rti.com>`__.
