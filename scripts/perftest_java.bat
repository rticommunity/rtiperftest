@REM (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
@REM Permission to modify and use for internal purposes granted.
@REM This software is provided "as is", without warranty, express or implied.

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

