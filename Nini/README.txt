---------------------------------------------------------
Nini - An uncommonly powerful .NET configuration library
---------------------------------------------------------

Homepage: http://nini.sourceforge.net/
Author:   Brent R. Matzelle - bmatzelle [at] yahoo [dot] com

ABOUT
-----
Nini is an uncommonly powerful .NET configuration library designed to help 
build highly configurable applications quickly.

INSTALL
-------
* Take the DLL for your .NET Framework version out of the Bin directory 

  * Bin\DotNet\1.0 - DLL built with the MS .NET Framework 1.0

  * Bin\DotNet\1.1 - DLL built with the MS .NET Framework 1.1
  
  * Bin\DotNetCompact\1.0 - DLL built for the MS .NET Compact Framework 1.0

  * Bin\Mono\1.1 - DLL built with Mono 1.1
  
* Add the DLL as a reference in your project.  In Visual Studio right-click on 
  the References item in the project menu, click on the Browse button and 
  select Nini.dll.
  
* You can also add Nini to all projects on your machine by adding it to the
  global assembly cache.  To do this run the following command:
  $> gacutil.exe /i Nini.dll

* To check if your install was successful add the following to a .NET project 
  file (this assumes C#, use the appropriate version for C++/VB.NET, etc):

  using Nini.Config;

  If it compiles without any errors then you've succeeded.

BUILDING FROM SOURCE
--------------------
There are several methods to build Nini.  These builds all write files to the 
same directory structure as found in the INSTALL section above.

* Visual Studio .NET
  At this time only a Visual Studio .NET 2002 solution file and project 
  file are supplied.  They are located in the Source directory (Nini.sln).
  To load this file simply double click the solutions and click 
  Build -> Build Solution.
  
  * Building for the .NET Compact Framework
    This requires that you have Visual Studio .NET 2003 installed.  Open
    the compact project file (NiniCompact.csdproj).  Build the solution
    in the same way as documented above.

* NAnt (http://nant.sourceforge.net)
  In the Source directory there is a Cyrus.build NAnt file.  Here is how to 
  build for each runtime from the command line:
  
  To build MS .NET Framework 1.0
  $> nant build-dotnet-1.0
  
  To build MS .NET Framework 1.1
  $> nant build-dotnet-1.1
  
  To build MS .NET Compact Framework 1.0
  *** There is no way to do this with NAnt at this time ***
  
  To build Mono 1.1
  $> nant build-mono

* Note: If you would like to run the unit test then download and install 
  NUnit (http://nunit.org/).

DOCUMENTATION
-------------
* You can find all documentation in the Docs directory.  Here is a description
  of all directories beneath this directory:

  * Docs\Manual - Contains the Nini manual files.  If you'd like to quickly get 
                  started with Nini then read this first.

  * Docs\Reference\chm - Contains the compiled reference documentation.
  
  * Docs\Reference\html - Contains the individual HTML-based reference.
  
  * Docs\Reference\xml - Contains the XML source for the reference 
                         documentation.  If you'd like to add more documentation
						 to the project then start here.

EXAMPLES
--------
The Nini project contains some example applications to help you get started.  
These files are located in the Examples directory.  

* NiniEdit
  This program is a fully-functional command line application 
  that edits INI, XML, and .NET configuration file types.  Reference the 
  README.txt file for more information about it.

QUESTIONS, HELP, & SUGGESTIONS
------------------------------
Go to the following places for help using Nini or if you'd like to request 
new features for Nini:

* Help Forum
  http://sourceforge.net/forum/forum.php?forum_id=379750

* Mailing List
  http://lists.sourceforge.net/lists/listinfo/nini-general

SUPPORTING NINI
---------------
If your project (commercial or otherwise) uses Nini please let me know about it.
I would like to post it on the project home page if possible.  Email me at 
bmatzelle [at] yahoo [dot] com.   

Thank you for trying Nini!

-------------------------------------
Copyright (c) 2006 Brent R. Matzelle
-------------------------------------
