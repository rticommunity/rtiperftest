----------------------------------------------------
Nini Editor - Command-line configuration file editor
----------------------------------------------------

ABOUT
-----
This program is a configuration file editor that utilizes the Nini library.
It edits any file-based configuration types including INI, XML, and .NET 
configuration files.  It can list, add, and remove configs as well as list, 
set, add, and remove config keys.  The Nini Editor determines from the file
extension of the input file how to load it.  If the extension is ".ini" for
instance it knows to load it up as an INI file.

This application serves as a good example for how you can use Nini in your
own projects.  It makes heavy use of the ArgvConfigSource library to parse
command line options.  It is licensed under the same license as everything
in the Nini project so feel free to use it for your own purposes.

BUILDING
--------
* Visual Studio .NET
  At this time only a Visual Studio .NET 2002 solution file and project 
  file are supplied (NiniEdit.sln).  To load this file simply open the 
  solutions file and click Build -> Build Solution.

* NAnt (http://nant.sourceforge.net)
  In the Source directory there is a Cyrus.build NAnt file.  Here is how to 
  build for each runtime:
  
  To build with the MS .NET Framework 1.0
  > nant build-dotnet-1.0
  
  To build with the MS .NET Framework 1.1
  > nant build-dotnet-1.1
  
  To build with Mono
  > nant build-mono

INSTALL
-------
Just place "niniedit.exe" and "Nini.dll" somewhere in your path and run it 
via the console for your operating system (cmd.exe on Windows).

USING THE PROGRAM
-----------------
Here are a couple common options to get you started.  Run these in a 
console (cmd.exe on Windows):

Get the usage options:
> niniedit --help

List all configs in a file:
> niniedit --list Example.ini

Add a new config to the file:
> niniedit --add NewConfig Example.ini

List all keys for a chosen config:
> niniedit --list-keys --config SomeConfig Example.ini

Set or add a key in a chosen config:
> niniedit --set-key NewKey,NewValue --config SomeConfig Example.ini


-------------------------------------
Copyright (c) 2006 Brent R. Matzelle
-------------------------------------
