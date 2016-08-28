@echo off

setlocal
set cmd=%~n0
set args=%1
set script_location=%~dp0

IF NOT DEFINED NDDSHOME (
    ECHO "[ERROR]: The NDDSHOME environment variable is not set."
    GOTO endscript
)
IF NOT DEFINED RTI_PERFTEST_ARCH (
    ECHO "[ERROR]: The RTI_PERFTEST_ARCH environment variable is not set."
    GOTO endscript
)

set PATH=%NDDSHOME%\lib\java;%PATH%
set PATH=%NDDSHOME%\lib\%RTI_PERFTEST_ARCH%;%PATH%

:getarg
shift
if "%~1"=="" goto continue

set args=%args% %1
goto getarg
:continue
java -Xmx800m -cp "%script_location%\perftest_java.jar;%NDDSHOME%\lib\java\nddsjava.jar" com.rti.perftest.ddsimpl.PerfTestLauncher %args%
:endscript