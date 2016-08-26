@ECHO off

REM Runs all the tests for the NiniEdit program.  
REM Usage: RunTests.bat Tests\[CONFIG FILE]
REM The test files are in the Test directory
REM All tests are run in verbose mode

SET PROGRAM=Bin\DotNet\Release\NiniEdit.exe --verbose
REM SET PROGRAM=mono Bin\Mono\Release\NiniEdit.exe --verbose

SET TESTFILE=%1

ECHO ------
ECHO TEST: Lists usage
%PROGRAM% --help

ECHO ------
ECHO TEST: Prints version
%PROGRAM% -V %TESTFILE%

ECHO ------
ECHO TEST: Lists configs
%PROGRAM% -l %TESTFILE%

ECHO ------
ECHO TEST: Adds config "TestConfig"
%PROGRAM% --add TestConfig %TESTFILE%
%PROGRAM% -l %TESTFILE%

ECHO ------
ECHO TEST: Removes config "TestConfig"
%PROGRAM% --remove TestConfig %TESTFILE%
%PROGRAM% -l %TESTFILE%

ECHO ------
ECHO TEST: Lists keys in "Logging"
%PROGRAM% --config Logging --list-keys %TESTFILE%

ECHO ------
ECHO TEST: Sets key "TestKey"
%PROGRAM% --config Logging --set-key TestKey,TestValue %TESTFILE%
%PROGRAM% --config Logging --list-keys %TESTFILE%

ECHO ------
ECHO TEST: Prints "TestKey" value: "TestValue"
%PROGRAM% --config Logging --get-key TestKey %TESTFILE%

ECHO ------
ECHO TEST: Removes key "TestKey"
%PROGRAM% --config Logging --remove-key TestKey %TESTFILE%
%PROGRAM% --config Logging --list-keys %TESTFILE%

ECHO ------
SET NEWFILE=Tests\NewTest.ini
ECHO TEST: Create file with two keys: %NEWFILE%
%PROGRAM% --new --set-type ini %NEWFILE%
%PROGRAM% --add Test %NEWFILE%
%PROGRAM% --config Test --set-key TestKey1,TestValue1 %NEWFILE%
%PROGRAM% --config Test --set-key TestKey2,TestValue2 %NEWFILE%
%PROGRAM% --config Test --list-keys %NEWFILE%
DEL %NEWFILE%
