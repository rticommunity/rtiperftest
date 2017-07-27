.. _section-supported_platfomrs:

Supported Platforms
===================

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

If you need help with the compilation or execution process, contact
support@rti.com.
