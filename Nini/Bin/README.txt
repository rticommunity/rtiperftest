-------------------------
NINI BINARY RELEASE FILES
-------------------------

This directory is where all binary release files are located.  These files 
are the official files that are recommended for use in your production 
applications.

DIRECTORIES
-----------
Note that there are several layers of directories in the Bin directory.  Each 
directory corresponds to a special version of Nini.  Here is an explanation 
of each directory:

- DotNet directory
This contains all Windows Nini binaries.  Underneath this directory is a 
directory for each subversion fo the framework (i.e. "1.0", "1.1", etc for 
the 1.0 and 1.1 version of the .NET Framework.  It is recommended that you 
use the binary that is appropriate for your development environment.  

- DotNetCompact directory
This contains all Windows .NET Compact Framework binaries.  

- Mono directory
This contains all Mono binaries.  Read about the Mono project here:

http://www.mono-project.com/

These binaries should run on any platform that Mono runs on.  This currently 
includes Windows, Linux, and Mac OS X.  SPARC and AMD64 support if not alreadly 
complete will be coming soon.

VERSION INFORMATION
-------------------
Each DLL is signed with a the type of platform it was developed with and 
the release number.  You can see this information on Windows by right-clicking
on the DLL and clicking Properties then selecting the Version tab.  You will
see information like this:

File version : 0.9.2.0
Description  : Nini for .NET Framework 1.1

SIGNED
------
These DLLs have all been signed so that you can directly Nini to your 
.NET Framework's Global Assembly Cache (GAC). You can do this by placing 
Nini.dll somewhere on your hard drive and running the gacutil.exe program 
like so:

> gacutil.exe /i C:\WINDOWS\system32\Nini.dll
