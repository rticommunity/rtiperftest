
@echo off
setlocal EnableDelayedExpansion

set script_location=%~dp0
set "idl_location=%script_location%srcIdl"
set "classic_cpp_folder=%script_location%srcCpp"
set "modern_cpp_folder=%script_location%srcCpp03"
set "cs_folder=%script_location%srcCs"
set "java_folder=%script_location%srcJava"
set "java_scripts_folder=%script_location%resource\java_scripts"
set "bin_folder=%script_location%bin"

@REM # By default we will build pro, not micro
set BUILD_MICRO=0
set BUILD_MICRO_24x_COMPATIBILITY=0
<<<<<<< HEAD
set MICRO_UNBOUNDED_SEQUENCE_SIZE=1048576
=======
>>>>>>> 20ac0a912d34a75df5555316900160bde38c9587

@REM # Default values:
set BUILD_CPP=1
set BUILD_CPP03=1
set BUILD_CS=1
set BUILD_JAVA=1
set CMAKE_EXE=cmake
set MSBUILD_EXE=msbuild
set JAVAC_EXE=javac
set JAVA_EXE=java
set JAR_EXE=jar

set RELEASE_DEBUG=release
set STATIC_DYNAMIC=static
set USE_SECURE_LIBS=0

@REM Starting with 5.2.6 (rtiddsgen 2.3.6) the name of the solutions is different
set /a rtiddsgen_version_number_new_solution_name=236

@REM # Needed when compiling statically using security
set RTI_OPENSSLHOME=""

@REM CMAKE requires a cmake generator
set cmake_generator="Visual Studio 10 2010"

set "classic_cpp_lang_string=C++"
set "modern_cpp_lang_string=C++03"
set "cs_lang_string=C#"
set "java_lang_string=java"

::------------------------------------------------------------------------------

@REM # Initial message
echo ================================== PERFTEST: ===================================

::------------------------------------------------------------------------------

@REM # PARSE ARGUMENTS

if "%1"=="" (
	call:help
	exit /b 1
)
:parse_arguments
if NOT "%1"=="" (
		if "%1"=="--clean" (
			call:clean
			exit /b 0
		) ELSE if "%1"=="--help" (
				call:help
				exit /b 0
		) ELSE if "%1"=="-h" (
				call:help
				exit /b 0
		) ELSE if "%1"=="--micro-24x-compatibility" (
				SET BUILD_MICRO=1
				SET BUILD_MICRO_24x_COMPATIBILITY=1
		) ELSE if "%1"=="--micro" (
				SET BUILD_MICRO=1
		) ELSE if "%1"=="--skip-java-build" (
				SET BUILD_JAVA=0
		) ELSE if "%1"=="--skip-cpp-build" (
				SET BUILD_CPP=0
		) ELSE if "%1"=="--skip-cpp03-build" (
				SET BUILD_CPP03=0
		) ELSE if "%1"=="--skip-cs-build" (
				SET BUILD_CS=0
		) ELSE if "%1"=="--java-build" (
				SET BUILD_JAVA=1
				SET BUILD_CPP=0
				SET BUILD_CPP03=0
				SET BUILD_CS=0
		) ELSE if "%1"=="--cpp-build" (
				SET BUILD_JAVA=0
				SET BUILD_CPP=1
				SET BUILD_CPP03=0
				SET BUILD_CS=0
		) ELSE if "%1"=="--cpp03-build" (
				SET BUILD_JAVA=0
				SET BUILD_CPP=0
				SET BUILD_CPP03=1
				SET BUILD_CS=0
		) ELSE if "%1"=="--cs-build" (
				SET BUILD_JAVA=0
				SET BUILD_CPP=0
				SET BUILD_CPP03=0
				SET BUILD_CS=1
		) ELSE if "%1"=="--debug" (
				SET RELEASE_DEBUG=debug
		) ELSE if "%1"=="--dynamic" (
				SET STATIC_DYNAMIC=dynamic
		) ELSE if "%1"=="--secure" (
				SET USE_SECURE_LIBS=1
		) ELSE if "%1"=="--msbuild" (
				SET MSBUILD_EXE=%2
				SHIFT
		) ELSE if "%1"=="--java-home" (
				SET "JAVAC_EXE=%2\bin\javac"
				SET "JAVA_EXE=%2\bin\java"
				SET "JAR_EXE=%2\bin\jar"
				SHIFT
		) ELSE if "%1"=="--platform" (
				SET architecture=%2
				SHIFT
		) ELSE if "%1"=="--openssl-home" (
				SET "RTI_OPENSSLHOME=%2"
				SHIFT
		) ELSE if "%1"=="--nddshome" (
				SET "NDDSHOME=%2"
				SHIFT
		) ELSE if "%1"=="--rtimehome" (
				SET "RTIMEHOME=%2"
				SHIFT
		) ELSE if "%1"=="--cmake" (
				SET "CMAKE_EXE=%2"
				SHIFT
		) ELSE if "%1"=="--cmake-generator" (
				SET "cmake_generator=%2"
				SHIFT
		) ELSE (
				echo [ERROR]: Unknown argument "%1"
				call:help
				exit /b 1
		)
		SHIFT
		GOTO :parse_arguments
)

::------------------------------------------------------------------------------

if !BUILD_MICRO! == 1 (

	@REM # Is RTIMEHOME set?
	if not exist "%RTIMEHOME%" (
		@REM # Is NDDSHOME set?
		if not exist "%NDDSHOME%" (
			echo [ERROR]: Nor RTIMEHOME nor NDDSHOME variables are set.
			exit /b 1
		) else (
			echo [WARNING]: The RTIMEHOME variable is not set, using NDDSHOME.
		)
	) else (
		set "NDDSHOME=!RTIMEHOME!"
	)

	call !MSBUILD_EXE! /version > nul
	if not !ERRORLEVEL! == 0 (
		echo [WARNING]: !MSBUILD_EXE! executable not found, perftest_cpp will not be built.
		set BUILD_MICRO=0
	)

) else (

	if not exist "%NDDSHOME%" (
			echo [ERROR]: The NDDSHOME variable is not set.
			exit /b 1
	)

	if !BUILD_CPP! == 1 (
		call !MSBUILD_EXE! /version > nul
		if not !ERRORLEVEL! == 0 (
			echo [WARNING]: !MSBUILD_EXE! executable not found, perftest_cpp will not be built.
			set BUILD_CPP=0
		)
	)
	if !BUILD_CPP03! == 1 (
		call !MSBUILD_EXE! /version > nul
		if not !ERRORLEVEL! == 0 (
			echo [WARNING]: !MSBUILD_EXE! executable not found, perftest_cpp03 will not be built.
			set BUILD_CPP03=0
		)
	)
	if !BUILD_CS! == 1 (
		call !MSBUILD_EXE! /version > nul
		if not !ERRORLEVEL! == 0 (
			echo [WARNING]: !MSBUILD_EXE! executable not found, perftest_cs will not be built.
			set BUILD_CS=0
		)
	)

	if !BUILD_JAVA! == 1 (
		call !JAVAC_EXE! -version > nul 2>nul
		if not !ERRORLEVEL! == 0 (
			echo [WARNING]: !JAVAC_EXE! executable not found, perftest_java will not be built.
			set BUILD_JAVA=0
		)
	)
)

::------------------------------------------------------------------------------

if !BUILD_MICRO! == 1 (

	set BUILD_CPP=0
	set BUILD_CPP03=0
	set BUILD_JAVA=0
	set BUILD_CS=0

	set "rtiddsgen_executable=!NDDSHOME!/rtiddsgen/scripts/rtiddsgen.bat"
) else (
	@REM # This calls the function in charge of getting the name of the solution for C++
	@REM # given the architecture.
	call::get_solution_name

    set "rtiddsgen_executable=!NDDSHOME!/bin/rtiddsgen.bat"
)

::------------------------------------------------------------------------------
if !BUILD_CPP! == 1 (

	call::solution_compilation_flag_calculation

	set "additional_header_files=MessagingIF.h RTIDDSImpl.h perftest_cpp.h qos_string.h CpuMonitor.h PerftestTransport.h Infrastructure_common.h Infrastructure_pro.h"
	set "additional_source_files=RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx Infrastructure_common.cxx Infrastructure_pro.cxx"

	if !USE_SECURE_LIBS! == 1 (
		set "ADDITIONAL_DEFINES=RTI_SECURE_PERFTEST"
		if "x!STATIC_DYNAMIC!" == "xdynamic" (
			set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_PERFTEST_DYNAMIC_LINKING"
		) else (
			if "x!RTI_OPENSSLHOME!" == "x" (
				echo [ERROR]: In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option.
				exit /b 1
			)
			set rtiddsgen_extra_options=-additionalRtiLibraries nddssecurity -additionalLibraries "libeay32z ssleay32z"
			set rtiddsgen_extra_options=!rtiddsgen_extra_options! -additionalLibraryPaths "!RTI_OPENSSLHOME!\static_!RELEASE_DEBUG!\lib"
			echo [INFO] Using security plugin. Linking Statically.
		)

		set "additional_header_files=!additional_header_files! PerftestSecurity.h"
		set "additional_source_files=!additional_source_files! PerftestSecurity.cxx"
	)
	set "ADDITIONAL_DEFINES=/0x !ADDITIONAL_DEFINES!"

	@REM # Generate files for srcCpp
	echo[
	echo [INFO]: Generating types and makefiles for %classic_cpp_lang_string%
	call "%rtiddsgen_executable%" -language %classic_cpp_lang_string% -unboundedSupport -replace^
	-create typefiles -create makefiles -platform %architecture%^
	-additionalHeaderFiles "!additional_header_files!"^
	-additionalSourceFiles "!additional_source_files!" -additionalDefines "!ADDITIONAL_DEFINES!"^
	!rtiddsgen_extra_options!^
	-d "%classic_cpp_folder%" "%idl_location%\perftest.idl"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %classic_cpp_lang_string%.
		exit /b 1
	)
	call copy "%classic_cpp_folder%"\perftest_publisher.cxx "%classic_cpp_folder%"\perftest_subscriber.cxx

	echo[
	echo [INFO]: Compiling %classic_cpp_lang_string%
	echo call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!" /p:Platform="!win_arch!"  "%classic_cpp_folder%"\%solution_name_cpp%
	call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!"  /p:Platform="!win_arch!"  "%classic_cpp_folder%"\%solution_name_cpp%
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %classic_cpp_lang_string%.
		exit /b 1
	)

	echo [INFO]: Copying perftest_cpp executable file:
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%classic_cpp_folder%"\objs\%architecture%\perftest_publisher.exe "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp.exe
)

::------------------------------------------------------------------------------

if !BUILD_CPP03! == 1 (

	call::solution_compilation_flag_calculation

	if !USE_SECURE_LIBS! == 1 (
		set "ADDITIONAL_DEFINES=RTI_SECURE_PERFTEST"
		if "x!STATIC_DYNAMIC!" == "xdynamic" (
			set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_PERFTEST_DYNAMIC_LINKING"
		) else (
			if "x!RTI_OPENSSLHOME!" == "x" (
				echo [ERROR]: In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option.
				exit /b 1
			)
			set rtiddsgen_extra_options=-additionalRtiLibraries nddssecurity -additionalLibraries "libeay32z ssleay32z"
			set rtiddsgen_extra_options=!rtiddsgen_extra_options! -additionalLibraryPaths "!RTI_OPENSSLHOME!\static_!RELEASE_DEBUG!\lib"
			echo [INFO] Using security plugin. Linking Statically.
		)
	)
	set "ADDITIONAL_DEFINES=/0x !ADDITIONAL_DEFINES!"

	@REM #Generate files for srcCpp03
	echo[
	echo [INFO]: Generating types and makefiles for %modern_cpp_lang_string%
	call "%rtiddsgen_executable%" -language %modern_cpp_lang_string% -unboundedSupport -replace^
	-create typefiles -create makefiles -platform %architecture%^
	-additionalHeaderFiles "MessagingIF.h RTIDDSImpl.h perftest_cpp.h qos_string.h CpuMonitor.h PerftestTransport.h"^
	-additionalSourceFiles "RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx" -additionalDefines "!ADDITIONAL_DEFINES!"^
	!rtiddsgen_extra_options!^
	-d "%modern_cpp_folder%" "%idl_location%\perftest.idl"

	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %modern_cpp_lang_string%.
		exit /b 1
	)
	call copy "%modern_cpp_folder%"\perftest_publisher.cxx "%modern_cpp_folder%"\perftest_subscriber.cxx

	echo[
	echo [INFO]: Compiling %modern_cpp_lang_string%
	echo call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!"  /p:Platform="!win_arch!"  "%modern_cpp_folder%"\%solution_name_cpp%
	call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!"  /p:Platform="!win_arch!"  "%modern_cpp_folder%"\%solution_name_cpp%
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %modern_cpp_lang_string%.
		exit /b 1
	)

	echo [INFO]: Copying perftest_cpp executable file:
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%modern_cpp_folder%"\objs\%architecture%\perftest_publisher.exe "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp03.exe
)

::------------------------------------------------------------------------------

if %BUILD_CS% == 1 (

	@REM Generate files for srcCs
	echo[
	echo [INFO]: Generating types and makefiles for %cs_lang_string%
	call "%rtiddsgen_executable%" -language %cs_lang_string% -unboundedSupport -replace^
	-create typefiles -create makefiles -platform %architecture%^
	-additionalSourceFiles "RTIDDSImpl.cs MessagingIF.cs CpuMonitor.cs PerftestTransport.cs"^
	-additionalDefines "/0x" -d "%cs_folder%" "%idl_location%\perftest.idl"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %cs_lang_string%.
		exit /b 1
	)
	call copy "%cs_folder%"\perftest_publisher.cs "%cs_folder%"\perftest_subscriber.cs

	echo[
	echo [INFO]: Compiling %cs_lang_string%
	echo call !MSBUILD_EXE! /p:Configuration=!RELEASE_DEBUG! /p:Platform="!cs_win_arch!" "%cs_folder%"\%solution_name_cs%
	call !MSBUILD_EXE! /p:Configuration=!RELEASE_DEBUG! /p:Platform="!cs_win_arch!" "%cs_folder%"\%solution_name_cs%
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %cs_lang_string%.
		exit /b 1
	)

	echo [INFO]: Copying files
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%cs_folder%"\%cs_bin_path%\perftest_publisher.exe "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cs.exe
	copy /Y "%cs_folder%"\%cs_bin_path%\perftest_*.dll "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%cs_folder%"\%cs_bin_path%\nddsdotnet*.dll "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
)

::------------------------------------------------------------------------------

if %BUILD_JAVA% == 1 (

	@REM Generate files for Java
	echo[
	echo [INFO]: Generating types and makefiles for %java_lang_string%
	call "%rtiddsgen_executable%" -language %java_lang_string% -unboundedSupport -replace^
	-package com.rti.perftest.gen -d "%java_folder%" "%idl_location%\perftest.idl"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %java_lang_string%.
		exit /b 1
	)
	call md "%java_folder%"\class
	call md "%java_folder%"\jar\!RELEASE_DEBUG!

	echo[
	if x!RELEASE_DEBUG! == xrelease (
		echo [INFO]: Compiling against nddsjava.jar library
		set "rti_ndds_java_jar=%NDDSHOME%/lib/java/nddsjava.jar"
	) else (
		echo [INFO]: Compiling against nddsjavad.jar library
	set "rti_ndds_java_jar=%NDDSHOME%/lib/java/nddsjavad.jar"
	)

	echo[
	echo [INFO]: Doing javac
	call !JAVAC_EXE! -d "%java_folder%"/class -cp "!rti_ndds_java_jar!"^
	"%java_folder%"/com/rti/perftest/*.java^
	"%java_folder%"/com/rti/perftest/ddsimpl/*.java^
	"%java_folder%"/com/rti/perftest/gen/*.java^
	"%java_folder%"/com/rti/perftest/harness/*.java
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %java_lang_string%.
		exit /b 1
	)

	echo [INFO]: Generating jar
	!JAR_EXE! cf "%java_folder%"\jar\!RELEASE_DEBUG!\perftest_java.jar^
	-C "%java_folder%"\class .

	echo [INFO]: Copying files
	md "%bin_folder%"\!RELEASE_DEBUG!
	copy "%java_folder%"\jar\!RELEASE_DEBUG!\perftest_java.jar "%bin_folder%"\!RELEASE_DEBUG!\perftest_java.jar
	copy "%java_scripts_folder%"\perftest_java.bat "%bin_folder%"\!RELEASE_DEBUG!\perftest_java.bat
	echo [INFO]: Files copied

	echo[
	echo [INFO]: You can run the java .jar file by using the following command:
	echo[
	echo "!JAVA_EXE!" -Xmx1500m -cp "%bin_folder%\!RELEASE_DEBUG!\perftest_java.jar;%NDDSHOME%\lib\java\nddsjava.jar" com.rti.perftest.ddsimpl.PerfTestLauncher
	echo[
	echo You will need to set the PATH variable for this script.
	echo[
	echo [INFO]: Or by running:
	echo[
	echo "%bin_folder%"\!RELEASE_DEBUG!\perftest_java.bat
	echo[
	echo You will need to set the NDDSHOME and RTI_PERFTEST_ARCH for this script.
	echo[
	echo [INFO]: Compilation successful

)

::------------------------------------------------------------------------------
if !BUILD_MICRO! == 1 (

	call::solution_compilation_flag_calculation

	if !BUILD_MICRO_24x_COMPATIBILITY! == 1 (
		set "ADDITIONAL_DEFINES=RTI_MICRO_24x_COMPATIBILITY !ADDITIONAL_DEFINES!"
	) else (
		set "rtiddsgen_extra_options=!rtiddsgen_extra_options! -sequenceSize !MICRO_UNBOUNDED_SEQUENCE_SIZE! -additionalRtiLibraries nddsmetp"
	)

	set "ADDITIONAL_DEFINES=RTI_WIN32 RTI_MICRO !ADDITIONAL_DEFINES!"
	set "additional_header_files=MessagingIF.h RTIDDSImpl.h perftest_cpp.h CpuMonitor.h PerftestTransport.h Infrastructure_common.h Infrastructure_micro.h"
	set "additional_source_files=RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx Infrastructure_common.cxx Infrastructure_micro.cxx"

	@REM # Generate files for srcCpp
	echo[
	echo [INFO]: Generating types and makefiles for %classic_cpp_lang_string%
	call "%rtiddsgen_executable%" -micro -language %classic_cpp_lang_string% -replace^
	-create typefiles -create makefiles^
	-additionalHeaderFiles "!additional_header_files!"^
	-additionalSourceFiles "!additional_source_files!" -additionalDefines "!ADDITIONAL_DEFINES!"^
	!rtiddsgen_extra_options!^
	-d "%classic_cpp_folder%" "%idl_location%\perftest.idl"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %classic_cpp_lang_string%.
		exit /b 1
	)
	call copy "%classic_cpp_folder%"\perftest_publisher.cxx "%classic_cpp_folder%"\perftest_subscriber.cxx
	call type NULL > "%classic_cpp_folder%"\perftestApplication.h
	call type NULL > "%classic_cpp_folder%"\perftestApplication.cxx
	call copy "%idl_location%\perftest.idl" "%classic_cpp_folder%"\perftest.idl

	echo[
	echo [INFO]: Compiling %classic_cpp_lang_string%

	cd "%classic_cpp_folder%"
	call !CMAKE_EXE! -DCMAKE_BUILD_TYPE=!RELEASE_DEBUG! --target perftest_publisher -G %cmake_generator% -B./perftest_build -H. -DRTIME_TARGET_NAME=%architecture% -DPLATFORM_LIBS="netapi32.lib;advapi32.lib;user32.lib;winmm.lib;WS2_32.lib;"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %classic_cpp_lang_string%.
		cd ..
		exit /b 1
	)
	call !CMAKE_EXE! --build ./perftest_build --config !RELEASE_DEBUG!
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %classic_cpp_lang_string%.
		cd ..
		exit /b 1
	)
	cd ..

	echo [INFO]: Copying perftest_cpp executable file:
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%classic_cpp_folder%"\objs\%architecture%\!RELEASE_DEBUG!\perftest_publisher.exe "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp_micro.exe
)

echo[
echo ================================================================================
GOTO:EOF

::------------------------------------------------------------------------------
@REM #FUNCTIONS:

:get_solution_name

	@REM #The name of the solution will depend on the rtiddsgen version
	for /F "delims=" %%i in ('"%NDDSHOME%\bin\rtiddsgen.bat" -version ^| findstr /R /C:rtiddsgen') do (
		set version_line=%%i
	)
	set version_string=%version_line:~49,6%

	for /F "tokens=1,2,3 delims=." %%a in ("%version_string%") do (
		set Major=%%a
		set Minor=%%b
		set Revision=%%c
	)

	if not x%architecture:x64=%==x%architecture% (
		set begin_sol=perftest_publisher-64-
		set begin_sol_cs=perftest-64-
		set cs_64=x64\
		set win_arch=x64
		set cs_win_arch=x64
	) else (
		set begin_sol=perftest_publisher-
		set begin_sol_cs=perftest-
		set win_arch=win32
		set cs_win_arch=x86
	)
	if not x%architecture:VS2008=%==x%architecture% (
		set end_sol=vs2008
		set extension=.vcproj
	)
	if not x%architecture:VS2010=%==x%architecture% (
		set end_sol=vs2010
		set extension=.vcxproj
	)
	if not x%architecture:VS2012=%==x%architecture% (
		set end_sol=vs2012
		set extension=.vcxproj
	)
	if not x%architecture:VS2013=%==x%architecture% (
		set end_sol=vs2013
		set extension=.vcxproj
	)
	if not x%architecture:VS2015=%==x%architecture% (
		set end_sol=vs2015
		set extension=.vcxproj
	)
	if not x%architecture:VS2017=%==x%architecture% (
		set end_sol=vs2017
		set extension=.vcxproj
	)

	set /a version_number=%Major%%Minor%%Revision%

	if %version_number% GEQ %rtiddsgen_version_number_new_solution_name% (
		set solution_name_cpp=perftest_publisher-%architecture%%extension%
		set solution_name_cs=perftest-%architecture%.sln
	) else (
		set solution_name_cpp=%begin_sol%%end_sol%%extension%
		set solution_name_cs=%begin_sol_cs%csharp.sln
	)
	set cs_bin_path=bin\%cs_64%!RELEASE_DEBUG!-%end_sol%

GOTO:EOF

:solution_compilation_flag_calculation
  echo[

	set "solution_compilation_mode_flag="
	if x!STATIC_DYNAMIC!==xdynamic (
		set "solution_compilation_mode_flag= DLL"
	)
	if x!RELEASE_DEBUG!==xdebug (
		set solution_compilation_mode_flag=debug!solution_compilation_mode_flag!
	) else (
		set solution_compilation_mode_flag=release!solution_compilation_mode_flag!
	)

	echo [INFO]: Compilation flag for msbuild is: !solution_compilation_mode_flag!

GOTO:EOF

:help
	echo[
	echo This scripts accepts the following parameters:
	echo[
	echo.    --micro                Build RTI Perftest for RTI Connext Micro
	echo.                           By default RTI Perftest will assume it will be
	echo.                           built against RTI Connext DDS Professional
	echo.    --platform your_arch   Platform for which build.sh is going to compile
	echo.                           RTI Perftest.
	echo.    --nddshome path        Path to the *RTI Connext DDS Professional
	echo.                           installation*. If this parameter is not present
	echo.                           the $NDDSHOME variable should be set.
	echo.                           If provided when building micro, it will be
	echo.                           used as $RTIMEHOME
	echo.    --rtimehome path       Path to the *RTI Connext DDS Micro
	echo.                           installation*. If this parameter is not present
	echo.                           the $RTIMEHOME variable should be set
	echo.    --skip-java-build      Avoid Java ByteCode generation creation.
	echo.                           (No effect when building for Micro)
	echo.    --skip-cpp-build       Avoid C++ code generation and compilation.
	echo.    --skip-cpp03-build     Avoid C++ New PSM code generation and
	echo.                           compilation.
	echo.                           (No effect when building for Micro)
	echo.    --skip-cs-build        Avoid C Sharp code generation and compilation.
	echo.                           (No effect when building for Micro)
	echo.    --java-build           Only Java ByteCode generation creation.
	echo.    --cpp-build            Only C++ code generation and compilation.
	echo.    --cpp03-build          Only C++ New PSM code generation and
	echo.                           compilation.
	echo.    --cs-build             Only C Sharp code generation and compilation.
	echo.    --make  path           Path to the GNU make executable. If this
	echo.                           parameter is not present, GNU make variable
	echo.                           should be available from your $PATH variable.
	echo.    --cmake  path          Path to the CMAKE executable. If this
	echo.                           parameter is not present, Cmake variable
	echo.                           should be available from your $PATH variable.
	echo.    --cmake-generator g    CMake generator to use
	echo.    --java-home path       Path to the Java JDK home folder. If this
	echo.                           parameter is not present, javac, jar and java
	echo.                           executables should be available from your
	echo.                           $PATH variable.
	echo.                           (No effect when building for Micro)
	echo.    --debug                Compile against the RTI Connext Debug
	echo.                           libraries. Default is against release ones.
	echo.    --dynamic              Compile against the RTI Connext Dynamic
	echo.                           libraries. Default is against static ones.
	echo.                           (No effect when building for Micro)
	echo.    --secure               Enable the security options for compilation.
	echo.                           Default is not enabled.
	echo.                           (No effect when building for Micro)
	echo.    --openssl-home path    Path to the openssl home. This will be used
	echo.                           when compiling statically and using security
	echo.                           (No effect when building for Micro)
	echo.    --clean                If this option is present, the build.sh script
	echo.                           will clean all the generated code and binaries
	echo.                           from previous executions.
	echo.    --help -h              Display this message.
	echo[
	echo ================================================================================
GOTO:EOF

:clean
	echo[
	echo Cleaning generated files.

	rmdir /s /q %script_location%srcCpp\objs > nul 2>nul
	del %script_location%srcCpp\*.vcxproj > nul 2>nul
	del %script_location%srcCpp\*.vcproj > nul 2>nul
	del %script_location%srcCpp\*.filters > nul 2>nul
	del %script_location%srcCpp\*.sln > nul 2>nul
	del %script_location%srcCpp\perftest.* > nul 2>nul
	del %script_location%srcCpp\perftestPlugin.* > nul 2>nul
	del %script_location%srcCpp\perftestSupport.* > nul 2>nul
	del %script_location%srcCpp\perftest_subscriber.cxx > nul 2>nul
	del %script_location%srcCpp\perftestApplication.h > nul 2>nul
	del %script_location%srcCpp\perftestApplication.cxx > nul 2>nul
	del %script_location%srcCpp\perftest.idl > nul 2>nul
	del %script_location%srcCpp\CMakeLists.txt > nul 2>nul
	del %script_location%srcCpp\README.txt > nul 2>nul
	rmdir /s /q "%script_location%srcCpp\gen" > nul 2>nul
	rmdir /s /q "%script_location%srcCpp\perftest_build" > nul 2>nul
	rmdir /s /q %script_location%srcCpp03\objs > nul 2>nul
	del %script_location%srcCpp03\*.vcxproj > nul 2>nul
	del %script_location%srcCpp03\*.vcproj > nul 2>nul
	del %script_location%srcCpp03\*.filters > nul 2>nul
	del %script_location%srcCpp03\*.sln > nul 2>nul
	del %script_location%srcCpp03\perftest.* > nul 2>nul
	del %script_location%srcCpp03\perftestImplPlugin.* > nul 2>nul
	del %script_location%srcCpp03\perftestImpl.* > nul 2>nul
	del %script_location%srcCpp03\perftest_subscriber.cxx > nul 2>nul
	rmdir /s /q %script_location%srcCs\obj > nul 2>nul
	rmdir /s /q %script_location%srcCs\bin > nul 2>nul
	del %script_location%srcCs\*.vcxproj > nul 2>nul
	del %script_location%srcCs\*.vcproj > nul 2>nul
	del %script_location%srcCs\*.filters > nul 2>nul
	del %script_location%srcCs\*.sln > nul 2>nul
	del %script_location%srcCs\perftest.* > nul 2>nul
	del %script_location%srcCs\perftest_subscriber.cs > nul 2>nul
	rmdir /s /q %script_location%bin > nul 2>nul
	rmdir /s /q %script_location%srcJava\class > nul 2>nul
	rmdir /s /q %script_location%srcJava\jar > nul 2>nul

	echo[
	echo ================================================================================
GOTO:EOF
