@REM $Id: perftest_java.bat,v 1.3 2015/04/30 11:25:24 juanjo Exp $
@REM
@REM (c) Copyright, Real-Time Innovations, 2005.
@REM All rights reserved.
@REM
@REM No duplications, whole or partial, manual or electronic, may be made
@REM without express written permission.  Any such copies, or
@REM revisions thereof, must display this notice unaltered.
@REM This code contains trade secrets of Real-Time Innovations, Inc.
@REM
@REM modification history
@REM --------------------
@REM 30apr15,jmc PERFTEST-88 Changed \class to \lib\java in the classpath
@REM 11apr09,jsr Added RTI_PERFTEST_ARCH variable for arch flexibility
@REM 22oct08,rbw Fixed JAR file name
@REM 07oct08,rbw Refactored launcher class
@REM 04apr08,rbw Harmonized output paths with other architectures
@REM 02apr08,rbw Increase heap size; get 'java' from path; use release libs
@REM 02apr08,rbw Created

@REM =====================================================================

@echo off

REM If Java compiler is not in your search path, set it here:
REM set Path="C:\Program Files\Java\jdk1.6.0_10\bin;%Path%
REM Make sure NDDSHOME is set correctly

IF NOT DEFINED NDDSHOME (
    ECHO "NDDSHOME environment variable is not set"
    GOTO endscript
)

REM IF NOT DEFINED RTI_PERFTEST_ARCH (
REM    ECHO "RTI_PERFTEST_ARCH environment variable is not set"
REM    GOTO endscript
REM )

REM Attempt to set Path from which to load native libraries.
REM If RTI_PERFTEST_ARCH is set (e.g. to i86Win32VS2008), you don't have to
REM separately set the Path.

set Path=%NDDSHOME%\lib\%RTI_PERFTEST_ARCH%;%Path%

setlocal
set dir=%~dp0
set cmd=%~n0
set args=%1

:getarg
shift
if "%~1"=="" goto continue

set args=%args% %1
goto getarg
:continue
@REM %JAVA_HOME%\bin\java -jar perftest.jar %args%
java -Xmx800m -cp bin\Release\perftestdds.jar;"%NDDSHOME%\lib\java\nddsjava.jar" com.rti.perftest.ddsimpl.PerfTestLauncher %args%
:endscript

