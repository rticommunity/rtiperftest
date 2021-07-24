@echo off

setlocal
set cmd=%~n0
set args=%1
set script_location=%~dp0

:getarg
shift
if "%~1"=="" goto continue

set args=%args% %1
goto getarg
:continue
