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
IF NOT DEFINED RTI_OPENSSL_PATH (
    ECHO "[INFO]: The RTI_OPENSSL_PATH environment variable is not set. Needed for DTLS, TLS and RTI Security Libraries."
)

set PATH=%NDDSHOME%\lib\java;%PATH%
set PATH=%NDDSHOME%\lib\%RTI_PERFTEST_ARCH%;%PATH%
set PATH=%RTI_OPENSSL_PATH%\%RTI_PERFTEST_ARCH%\release\bin;%PATH%

:getarg
shift
if "%~1"=="" goto continue

set args=%args% %1
goto getarg
:continue
java -Xmx800m -cp "%script_location%\perftest_java.jar;%NDDSHOME%\lib\java\nddsjava.jar" com.rti.perftest.ddsimpl.PerfTestLauncher %args%
:endscript