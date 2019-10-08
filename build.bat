
@echo off
setlocal EnableDelayedExpansion

set script_location=%~dp0
set "idl_location=%script_location%srcIdl"
set "common_cpp_folder=%script_location%srcCppCommon"
set "classic_cpp_folder=%script_location%srcCpp"
set "modern_cpp_folder=%script_location%srcCpp03"
set "cs_folder=%script_location%srcCs"
set "java_folder=%script_location%srcJava"
set "java_scripts_folder=%script_location%resource\scripts\java_execution_scripts"
set "bin_folder=%script_location%bin"
set "cStringifyFile_script=%script_location%resource\scripts\cStringifyFile.pl"
set "qos_file=%script_location%perftest_qos_profiles.xml"

@REM # By default we will build pro, not micro
set BUILD_MICRO=0
set CMAKE_GENERATOR="NMake Makefiles"

@REM # In case we build micro, which version.
set BUILD_MICRO_24x_COMPATIBILITY=0
set MICRO_UNBOUNDED_SEQUENCE_SIZE=1048576

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
set PERL_EXEC=perl

set RELEASE_DEBUG=release
set STATIC_DYNAMIC=static
set USE_SECURE_LIBS=0
set LEGACY_DD_IMPL=0

@REM Starting with 5.2.6 (rtiddsgen 2.3.6) the name of the solutions is different
set /a rtiddsgen_version_number_new_solution_name=236

@REM # Needed when compiling statically using security
set RTI_OPENSSLHOME=""

set "classic_cpp_lang_string=C++"
set "modern_cpp_lang_string=C++03"
set "cs_lang_string=C#"
set "java_lang_string=java"

@REM # Variables for customType
set "custom_type_folder=%idl_location%\customType"
set USE_CUSTOM_TYPE=0
set USE_CUSTOM_TYPE_FLAT=0
set "custom_type=" @REM # Type of the customer
set "custom_type_flat=" @REM # Type of the customer
@REM # Name of the file with the type. "TSupport.h"
set "custom_type_file_name_support="
@REM # Intermediate file for including the custom type file #include "file.idl"
set "custom_idl_file=%custom_type_folder%\custom.idl"

@REM # Variables for FlatData
set "flatdata_size=10485760" @REM # 10MB
set flatdata_ddsgen_version=300
set FLATDATA_AVAILABLE=0
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
		) ELSE if "%1"=="--legacy-DynamicData" (
				SET LEGACY_DD_IMPL=1
		) ELSE if "%1"=="--secure" (
				SET USE_SECURE_LIBS=1
		) ELSE if "%1"=="--msbuild" (
				SET MSBUILD_EXE=%2
				SHIFT
		) ELSE if "%1"=="--perl" (
				SET PERL_EXEC=%2
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
				SET "CMAKE_GENERATOR=%2"
				SHIFT
		) ELSE if "%1"=="--customType" (
				SET USE_CUSTOM_TYPE=1
				SET "custom_type=%2"
				if "!custom_type!"== "" (
					echo [ERROR]: --customType should be followed by the name of the type.
					call:help
					exit /b 1
				)
				SHIFT
		) ELSE if "%1"=="--customTypeFlatData" (
				SET USE_CUSTOM_TYPE_FLAT=1
				SET "custom_type_flat=%2"
				if "!custom_type_flat!"== "" (
					echo [ERROR]: --customTypeFlatData should be followed by the name of the type.
					call:help
					exit /b 1
				)
				SHIFT
		) ELSE if "%1"=="--flatdata-max-size" (
				SET "flatdata_size=%2"
				if "!flatdata_size!"  LEQ "0" (
					echo [ERROR]: "--flatdata-max-size n" requires "n > 0."
					exit /b 1
				)
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

if "x!architecture!" == "x" (
	echo [ERROR]: The platform argument is missing.
	exit /b 1
)


if not x%architecture:INtime=%==x%architecture% (
		set "executable_extension=.rta"
) else (
		set "executable_extension=.exe"
)

::------------------------------------------------------------------------------

if !BUILD_MICRO! == 1 (

	@REM # Is RTIMEHOME set?
	if not exist "%RTIMEHOME%" (
		@REM # Is NDDSHOME set?
		if not exist "%NDDSHOME%" (
			echo [ERROR]: Nor RTIMEHOME nor NDDSHOME variables are set or the paths do not exist.
			exit /b 1
		) else (
			echo [WARNING]: The RTIMEHOME variable is not set or the path does not exist, using NDDSHOME instead.
		)
	) else (
		set "NDDSHOME=!RTIMEHOME!"
	)

	call !MSBUILD_EXE! /version > nul
	if not !ERRORLEVEL! == 0 (
		echo [WARNING]: !MSBUILD_EXE! executable not found, perftest_cpp_micro will not be built.
		set BUILD_MICRO=0
		exit /b 1
	)

) else (

	if not exist "%NDDSHOME%" (
			echo [ERROR]: The NDDSHOME variable is not set or the path does not exist.
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

if !BUILD_CPP! == 1 (
	call !PERL_EXEC! -version > nul 2>nul
	if not !ERRORLEVEL! == 0 (
		echo [WARNING]: PERL not found, !classic_cpp_folder!\qos_string.h will not be updated.
	) else (
		!PERL_EXEC! !cStringifyFile_script! !qos_file! PERFTEST_QOS_STRING > !classic_cpp_folder!\qos_string.h
		echo [INFO]: QoS String !classic_cpp_folder!\qos_string.h updated successfully.
	)
)

if !BUILD_CPP03! == 1 (
	call !PERL_EXEC! -version > nul 2>nul
	if not !ERRORLEVEL! == 0 (
		echo [WARNING]: PERL not found, !modern_cpp_folder!\qos_string.h will not be updated.
	) else (
		!PERL_EXEC! !cStringifyFile_script! !qos_file! PERFTEST_QOS_STRING > !modern_cpp_folder!\qos_string.h
		echo [INFO]: QoS String !modern_cpp_folder!\qos_string.h updated successfully.
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

	call::copy_src_cpp_common
	call::solution_compilation_flag_calculation
	call::get_flatdata_available

	REM # Generate files for the custom type files
	set "additional_defines_custom_type="
	set "additional_header_files_custom_type="
	set "additional_source_files_custom_type="

	if !USE_CUSTOM_TYPE! == 1 (
		REM # Search the file which contains "Struct ${custom_type} {" and include it to ${custom_idl_file}
		set found_idl=0
		for %%i in (%custom_type_folder%\*) do (
			FINDSTR /C:"struct %custom_type% {" %%i  >NUL 2>&1
			if !ERRORLEVEL! == 0 (
				REM # found
				set custom_type_file_name_support=%%~niSupport.h
				echo #include "%%~nxi" > %custom_idl_file%
				set found_idl=1
			)
		)
		if !found_idl! == 0 (
			echo [ERROR]: Cannot find an idl file with the %custom_type% structure for custom type test.
			exit /b 1
		)

		call copy /Y %custom_type_folder%\* %idl_location%\
		set "additional_header_files_custom_type=CustomType.h"
		set "additional_source_files_custom_type=CustomType.cxx"
		REM # Find all the files in the folder ${custom_type_folder}
		REM # Run codegen with all those files
		for %%i in (%custom_type_folder%\*) do (
			call "%rtiddsgen_executable%" -language %classic_cpp_lang_string% -unboundedSupport -I %idl_location% -replace^
			-create typefiles -d "%classic_cpp_folder%" %%i
			if not !ERRORLEVEL! == 0 (
				echo [ERROR]:Failure generating code for %classic_cpp_lang_string% with the file %%i."
				exit /b 1
			)
			set "additional_header_files_custom_type=%%~niPlugin.h %%~ni.h %%~niSupport.h !additional_header_files_custom_type!"
			set "additional_source_files_custom_type=%%~niPlugin.cxx %%~ni.cxx %%~niSupport.cxx !additional_source_files_custom_type!"
		)
		set "additional_header_files_custom_type=!additional_header_files_custom_type! "
		set "additional_source_files_custom_type=!additional_source_files_custom_type! "
		REM # Adding RTI_USE_CUSTOM_TYPE as a macro
		set "additional_defines_custom_type= -D RTI_CUSTOM_TYPE=%custom_type%"

		if !USE_CUSTOM_TYPE_FLAT! == 1 (
			set "additional_defines_custom_type=!additional_defines_custom_type! -D RTI_CUSTOM_TYPE_FLATDATA=%custom_type_flat%"
		)
	)

	if !LEGACY_DD_IMPL! == 1 (
		echo [INFO]: Allow the use of both legacy and new Dynamic Data Impl.
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_LEGACY_DD_IMPL"
	)

	set "ADDITIONAL_DEFINES=RTI_LANGUAGE_CPP_TRADITIONAL"

	if !USE_SECURE_LIBS! == 1 (
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_SECURE_PERFTEST"
		if "x!STATIC_DYNAMIC!" == "xdynamic" (
			set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_PERFTEST_DYNAMIC_LINKING"
		) else (
			if "x!RTI_OPENSSLHOME!" == "x" (
				echo [ERROR]: In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option.
				exit /b 1
			)
			set additional_rti_libs=nddssecurity !additional_rti_libs!
			set rtiddsgen_extra_options=-additionalLibraries "libeay32z ssleay32z"
			set rtiddsgen_extra_options=!rtiddsgen_extra_options! -additionalLibraryPaths "!RTI_OPENSSLHOME!\static_!RELEASE_DEBUG!\lib"
			echo [INFO] Using security plugin. Linking Statically.
		)
		set "additional_header_files=PerftestSecurity.h !additional_header_files!"
		set "additional_source_files=PerftestSecurity.cxx !additional_source_files!"
	)

	if !FLATDATA_AVAILABLE! == 1 (
        @REM On Windows we always enable ZeroCopy if FlatData is available.
		set additional_rti_libs=nddsmetp !additional_rti_libs!
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_FLATDATA_MAX_SIZE=!flatdata_size! RTI_FLATDATA_AVAILABLE RTI_ZEROCOPY_AVAILABLE"
		set "additional_defines_flatdata=-D "RTI_FLATDATA_AVAILABLE" -D "RTI_ZEROCOPY_AVAILABLE" -D "RTI_FLATDATA_MAX_SIZE=!flatdata_size!""
	)

	if !USE_CUSTOM_TYPE! == 1 (
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_CUSTOM_TYPE=%custom_type% RTI_CUSTOM_TYPE_FILE_NAME_SUPPORT=!custom_type_file_name_support!"

		if !USE_CUSTOM_TYPE_FLAT! == 1 (
			set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_CUSTOM_TYPE_FLATDATA=%custom_type_flat%"
		)
	)

	where git >nul 2>nul
	If !ERRORLEVEL! == 0 (
		for /f "tokens=* USEBACKQ" %%F in (`git rev-parse --short HEAD`) do (
			set commit_id=%%F
		)
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! PERFTEST_COMMIT_ID=\"!commit_id!\""
	)

	set "ADDITIONAL_DEFINES=/0x !ADDITIONAL_DEFINES!"
	set "additional_header_files=!additional_header_files_custom_type!!additional_header_files!RTIRawTransportImpl.h Parameter.h ParameterManager.h ThreadPriorities.h RTIDDSLoggerDevice.h MessagingIF.h RTIDDSImpl.h perftest_cpp.h qos_string.h CpuMonitor.h PerftestTransport.h Infrastructure_common.h Infrastructure_pro.h"
	set "additional_source_files=!additional_source_files_custom_type!!additional_source_files!RTIRawTransportImpl.cxx Parameter.cxx ParameterManager.cxx ThreadPriorities.cxx RTIDDSLoggerDevice.cxx RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx Infrastructure_common.cxx Infrastructure_pro.cxx"

	if !FLATDATA_AVAILABLE! == 1 (
		set "additional_header_files=!additional_header_files! perftest_ZeroCopy.h perftest_ZeroCopyPlugin.h perftest_ZeroCopySupport.h"
		set "additional_source_files=!additional_source_files! perftest_ZeroCopy.cxx perftest_ZeroCopyPlugin.cxx perftest_ZeroCopySupport.cxx"
	)

	echo[
	echo "%rtiddsgen_executable%" -language %classic_cpp_lang_string%^
	-unboundedSupport -replace -create typefiles -create makefiles^
	-platform %architecture%^
	!additional_defines_flatdata!^
	-additionalHeaderFiles "!additional_header_files!"^
	-additionalSourceFiles "!additional_source_files!"^
	-additionalDefines "!ADDITIONAL_DEFINES!"^
	-additionalRtiLibraries "!additional_rti_libs!"^
	!rtiddsgen_extra_options! !additional_defines_custom_type!^
	-d "%classic_cpp_folder%" "%idl_location%\perftest.idl"

	echo[
	echo [INFO]: Generating types and makefiles for %classic_cpp_lang_string%
	call "%rtiddsgen_executable%" -language %classic_cpp_lang_string%^
	-unboundedSupport -replace -create typefiles -create makefiles^
	-platform %architecture%^
	!additional_defines_flatdata!^
	-additionalHeaderFiles "!additional_header_files!"^
	-additionalSourceFiles "!additional_source_files!"^
	-additionalDefines "!ADDITIONAL_DEFINES!"^
	-additionalRtiLibraries "!additional_rti_libs!"^
	!rtiddsgen_extra_options! !additional_defines_custom_type!^
	-d "%classic_cpp_folder%" "%idl_location%\perftest.idl"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %classic_cpp_lang_string%.
		call::clean_src_cpp_common
		exit /b 1
	)

    @REM # Generate ZeroCopy types avoiding performance degradation issue
	if !FLATDATA_AVAILABLE! == 1 (
		echo[
		echo "%rtiddsgen_executable%" -language %classic_cpp_lang_string%^
		!additional_defines_flatdata!^
		-replace -create typefiles^
		-platform %architecture%^
		!rtiddsgen_extra_options! !additional_defines_custom_type!^
		-d "%classic_cpp_folder%" "%idl_location%\perftest_ZeroCopy.idl"

		echo[
		echo [INFO]: Generating Zero Copy code
		call "%rtiddsgen_executable%" -language %classic_cpp_lang_string%^
		!additional_defines_flatdata!^
		-replace -create typefiles^
		-platform %architecture%^
		!rtiddsgen_extra_options! !additional_defines_custom_type!^
		-d "%classic_cpp_folder%" "%idl_location%\perftest_ZeroCopy.idl"
		if not !ERRORLEVEL! == 0 (
			echo [ERROR]: Failure generating code for %classic_cpp_lang_string%.
			call::clean_src_cpp_common
			exit /b 1
		)
	)

	call copy "%classic_cpp_folder%"\perftest_publisher.cxx "%classic_cpp_folder%"\perftest_subscriber.cxx

	echo[
	echo [INFO]: Compiling %classic_cpp_lang_string%
	echo call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!" /p:Platform="!win_arch!"  "%classic_cpp_folder%"\%solution_name_cpp%
	call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!"  /p:Platform="!win_arch!"  "%classic_cpp_folder%"\%solution_name_cpp%
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %classic_cpp_lang_string%.
		call::clean_src_cpp_common
		exit /b 1
	)

	echo [INFO]: Copying perftest_cpp executable file:
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%classic_cpp_folder%"\objs\%architecture%\perftest_publisher"%executable_extension%" "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp"%executable_extension%"
	call::clean_src_cpp_common

	if "x!STATIC_DYNAMIC!" == "xdynamic" (
		echo [INFO]: Code compiled dynamically, Add "NDDSHOME/lib/%platform%"
		if !USE_SECURE_LIBS! == 1 (
			echo and <OPENSSL_HOME>\!RELEASE_DEBUG!\bin
		)
		echo to your PATH variable
	)
)

::------------------------------------------------------------------------------

if !BUILD_CPP03! == 1 (
	call::copy_src_cpp_common
	call::solution_compilation_flag_calculation
	call::get_flatdata_available

	set "ADDITIONAL_DEFINES=RTI_LANGUAGE_CPP_MODERN"
	set additional_rti_libs=

	if !USE_SECURE_LIBS! == 1 (
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_SECURE_PERFTEST"
		if "x!STATIC_DYNAMIC!" == "xdynamic" (
			set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_PERFTEST_DYNAMIC_LINKING"
		) else (
			if "x!RTI_OPENSSLHOME!" == "x" (
				echo [ERROR]: In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option.
				exit /b 1
			)
			set additional_rti_libs=nddssecurity !additional_rti_libs!
			set rtiddsgen_extra_options=-additionalLibraries "libeay32z ssleay32z"
			set rtiddsgen_extra_options=!rtiddsgen_extra_options! -additionalLibraryPaths "!RTI_OPENSSLHOME!\static_!RELEASE_DEBUG!\lib"
			echo [INFO] Using security plugin. Linking Statically.
		)
	)

	where git >nul 2>nul
	If !ERRORLEVEL! == 0 (
		for /f "tokens=* USEBACKQ" %%F in (`git rev-parse --short HEAD`) do (
			set commit_id=%%F
		)
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! PERFTEST_COMMIT_ID=\"!commit_id!\""
	)

	if !FLATDATA_AVAILABLE! == 1 (
		set "additional_rti_libs=nddsmetp !additional_rti_libs!"
		set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_FLATDATA_MAX_SIZE=!flatdata_size! RTI_FLATDATA_AVAILABLE"
		set "additional_defines_flatdata=-D "RTI_FLATDATA_AVAILABLE" -D "RTI_FLATDATA_MAX_SIZE=!flatdata_size!""
	)

	set "ADDITIONAL_DEFINES=/0x !ADDITIONAL_DEFINES!"

	set "additional_header_files=ThreadPriorities.h Parameter.h ParameterManager.h MessagingIF.h RTIDDSImpl.h perftest_cpp.h qos_string.h CpuMonitor.h PerftestTransport.h"
	set "additional_source_files=ThreadPriorities.cxx Parameter.cxx ParameterManager.cxx RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx"

	if !FLATDATA_AVAILABLE! == 1 (
		set "additional_header_files=!additional_header_files! perftest_ZeroCopy.hpp perftest_ZeroCopyPlugin.hpp"
		set "additional_source_files=!additional_source_files! perftest_ZeroCopy.cxx perftest_ZeroCopyPlugin.cxx"
	)

	echo[
	echo [INFO] "%rtiddsgen_executable%" -language %modern_cpp_lang_string% ^
	-unboundedSupport -replace -create typefiles -create makefiles^
	-platform %architecture%^
	!additional_defines_flatdata!^
	-additionalHeaderFiles "!additional_header_files!"^
	-additionalSourceFiles "!additional_source_files!"^
	-additionalDefines "!ADDITIONAL_DEFINES!"^
	-additionalRtiLibraries "!additional_rti_libs!"^
	!rtiddsgen_extra_options!^
	-d "%modern_cpp_folder%" "%idl_location%\perftest.idl"

	@REM #Generate files for srcCpp03
	echo[
	echo [INFO]: Generating types and makefiles for %modern_cpp_lang_string%
	call "%rtiddsgen_executable%" -language %modern_cpp_lang_string% ^
	-unboundedSupport -replace -create typefiles -create makefiles^
	-platform %architecture%^
	!additional_defines_flatdata!^
	-additionalHeaderFiles "!additional_header_files!"^
	-additionalSourceFiles "!additional_source_files!"^
	-additionalDefines "!ADDITIONAL_DEFINES!"^
	-additionalRtiLibraries "!additional_rti_libs!"^
	!rtiddsgen_extra_options!^
	-d "%modern_cpp_folder%" "%idl_location%\perftest.idl"

	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure generating code for %modern_cpp_lang_string%.
		call::clean_src_cpp_common
		exit /b 1
	)

	if !FLATDATA_AVAILABLE! == 1 (
		echo[
		echo "%rtiddsgen_executable%" -language %modern_cpp_lang_string%^
		!additional_defines_flatdata!^
		-replace -create typefiles -platform %architecture%^
		!rtiddsgen_extra_options!^
		-d "%modern_cpp_folder%" "%idl_location%\perftest_ZeroCopy.idl"


		@REM # Generate Zero Copy types avoiding performance degradation issue
		echo[
		echo [INFO]: Generating Zero Copy code
		call "%rtiddsgen_executable%" -language %modern_cpp_lang_string%^
		!additional_defines_flatdata!^
		-replace -create typefiles -platform %architecture%^
		!rtiddsgen_extra_options!^
		-d "%modern_cpp_folder%" "%idl_location%\perftest_ZeroCopy.idl"
		if not !ERRORLEVEL! == 0 (
			echo [ERROR]: Failure generating code for %modern_cpp_lang_string%.
			call::clean_src_cpp_common
			exit /b 1
		)
	)

	call copy "%modern_cpp_folder%"\perftest_publisher.cxx "%modern_cpp_folder%"\perftest_subscriber.cxx

	echo[
	echo [INFO]: Compiling %modern_cpp_lang_string%
	echo call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!"  /p:Platform="!win_arch!"  "%modern_cpp_folder%"\%solution_name_cpp%
	call !MSBUILD_EXE! /p:Configuration="!solution_compilation_mode_flag!"  /p:Platform="!win_arch!"  "%modern_cpp_folder%"\%solution_name_cpp%
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %modern_cpp_lang_string%.
		call::clean_src_cpp_common
		exit /b 1
	)

	echo [INFO]: Copying perftest_cpp executable file:
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!
	copy /Y "%modern_cpp_folder%"\objs\%architecture%\perftest_publisher"%executable_extension%" "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp03"%executable_extension%"
	call::clean_src_cpp_common

	if "x!STATIC_DYNAMIC!" == "xdynamic" (
		echo [INFO]: Code compiled dynamically, Add "NDDSHOME/lib/%platform%"
		if !USE_SECURE_LIBS! == 1 (
			echo and <OPENSSL_HOME>\!RELEASE_DEBUG!\bin
		)
		echo to your PATH variable
	)
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
	copy /Y "%cs_folder%"\%cs_bin_path%\perftest_publisher"%executable_extension%" "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cs"%executable_extension%"
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

	call::copy_src_cpp_common
	call::solution_compilation_flag_calculation

	set "ADDITIONAL_DEFINES=RTI_LANGUAGE_CPP_TRADITIONAL"

	if !BUILD_MICRO_24x_COMPATIBILITY! == 1 (
		set "ADDITIONAL_DEFINES=RTI_MICRO_24x_COMPATIBILITY !ADDITIONAL_DEFINES!"
	) else (

		if !USE_SECURE_LIBS! == 1 (

			if "x!RTI_OPENSSLHOME!" == "x" (
				echo [ERROR]: In order to link statically using the security plugin you need to also provide the OpenSSL home path by using the --openssl-home option.
				exit /b 1
			)

			set "ADDITIONAL_DEFINES=!ADDITIONAL_DEFINES! RTI_SECURE_PERFTEST"
			set "additional_rti_libraries=rti_me_netioshmem rti_me_netioshmem rti_me_seccore"

			set rtiddsgen_extra_options= -additionalLibraries "libeay32z ssleay32z"
			set rtiddsgen_extra_options=!rtiddsgen_extra_options! -additionalLibraryPaths "!RTI_OPENSSLHOME!/static_!RELEASE_DEBUG!/lib"
			echo [INFO] Using security plugin. Linking Statically.
		) else (
			set "additional_rti_libraries=nddsmetp"
		)
		set rtiddsgen_extra_options=!rtiddsgen_extra_options! -sequenceSize !MICRO_UNBOUNDED_SEQUENCE_SIZE! -additionalRtiLibraries "!additional_rti_libraries!"
	)

	set "ADDITIONAL_DEFINES=RTI_WIN32 RTI_MICRO !ADDITIONAL_DEFINES!"
	set "additional_header_files=ParameterManager.h Parameter.h ThreadPriorities.h MessagingIF.h RTIDDSImpl.h perftest_cpp.h CpuMonitor.h PerftestTransport.h Infrastructure_common.h Infrastructure_micro.h PerftestSecurity.h"
	set "additional_source_files=ParameterManager.cxx Parameter.cxx ThreadPriorities.cxx RTIDDSImpl.cxx CpuMonitor.cxx PerftestTransport.cxx Infrastructure_common.cxx Infrastructure_micro.cxx PerftestSecurity.cxx"

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
	call copy "%idl_location%\perftest.idl" "%classic_cpp_folder%"\perftest.idl
	call echo. > "%classic_cpp_folder%"\perftestApplication.h
	call echo. > "%classic_cpp_folder%"\perftestApplication.cxx

	echo[
	echo [INFO]: Compiling %classic_cpp_lang_string%

	if x!CMAKE_GENERATOR! == x"NMake Makefiles" (
		echo[
		echo [INFO]: Using NMake for the CMake generator, use --cmake-generator to specify other.
	)
	cd "%classic_cpp_folder%"
	call !CMAKE_EXE! -DCMAKE_BUILD_TYPE=!RELEASE_DEBUG! --target perftest_publisher -G !CMAKE_GENERATOR! -B./perftest_build -H. -DRTIME_TARGET_NAME=%architecture% -DPLATFORM_LIBS="netapi32.lib;advapi32.lib;user32.lib;winmm.lib;WS2_32.lib;"
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %classic_cpp_lang_string%.
		cd ..
		exit /b 1
	)

	call !CMAKE_EXE! --build ./perftest_build --config !RELEASE_DEBUG! --target perftest_publisher
	if not !ERRORLEVEL! == 0 (
		echo [ERROR]: Failure compiling code for %classic_cpp_lang_string%.
		cd ..
		exit /b 1
	)
	cd ..

	echo[
	echo [INFO]: Copying perftest_cpp executable file:
	md "%bin_folder%"\%architecture%\!RELEASE_DEBUG!

	if x!CMAKE_GENERATOR! == x"NMake Makefiles" (
		copy /Y "%classic_cpp_folder%"\objs\%architecture%\perftest_publisher.exe "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp_micro.exe
	) else (
		copy /Y "%classic_cpp_folder%"\objs\%architecture%\!RELEASE_DEBUG!\perftest_publisher.exe "%bin_folder%"\%architecture%\!RELEASE_DEBUG!\perftest_cpp_micro.exe
	)
)


echo[
echo ================================================================================
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

::------------------------------------------------------------------------------
@REM #FUNCTIONS:

:get_ddsgen_version
	for /F "delims=" %%i in ('"%NDDSHOME%\bin\rtiddsgen.bat" -version ^| findstr /R /C:rtiddsgen') do (
		set version_line=%%i
	)
	set version_string=%version_line:~49,6%

	for /F "tokens=1,2,3 delims=." %%a in ("%version_string%") do (
		set Major=%%a
		set Minor=%%b
		set Revision=%%c
	)

	set /a version_number=%Major%%Minor%%Revision%
GOTO:EOF

:get_flatdata_available
	call::get_ddsgen_version

	if %version_number% GEQ %flatdata_ddsgen_version% (
		echo [INFO] FlatData is available
		set FLATDATA_AVAILABLE=1
	)
goto:EOF

:get_solution_name
	call::get_ddsgen_version

	if not x%architecture:INtime=%==x%architecture% (
		set begin_sol=perftest_publisher-
		set begin_sol_cs=perftest-
		set win_arch=INtime
	) else if not x%architecture:x64=%==x%architecture% (
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

	if %version_number% GEQ %rtiddsgen_version_number_new_solution_name% (
		set solution_name_cpp=perftest_publisher-%architecture%%extension%
		set solution_name_cs=perftest-%architecture%.sln
	) else (
		set solution_name_cpp=%begin_sol%%end_sol%%extension%
		set solution_name_cs=%begin_sol_cs%csharp.sln
	)
	set cs_bin_path=bin\%cs_64%!RELEASE_DEBUG!-%end_sol%
GOTO:EOF

:help
	echo[
	echo This scripts accepts the following parameters:
	echo[
	echo.    --micro                      Build RTI Perftest for RTI Connext Micro
	echo.                                 By default RTI Perftest will assume it will be
	echo.                                 built against RTI Connext DDS Professional.
	echo.    --platform your_arch         Platform for which build.sh is going to compile
	echo.                                 RTI Perftest.
	echo.    --nddshome path              Path to the *RTI Connext DDS Professional
	echo.                                 installation*. If this parameter is not present
	echo.                                 the $NDDSHOME variable should be set.
	echo.                                 If provided when building micro, it will be
	echo.                                 used as $RTIMEHOME
	echo.    --rtimehome path             Path to the *RTI Connext DDS Micro
	echo.                                 installation*. If this parameter is not present
	echo.                                 the $RTIMEHOME variable should be set
	echo.    --skip-java-build            Avoid Java ByteCode generation creation.
	echo.                                 (No effect when building for Micro)
	echo.    --skip-cpp-build             Avoid C++ code generation and compilation.
	echo.    --skip-cpp03-build           Avoid C++ New PSM code generation and
	echo.                                 compilation.
	echo.                                 (No effect when building for Micro)
	echo.    --skip-cs-build              Avoid C Sharp code generation and compilation.
	echo.                                 (No effect when building for Micro)
	echo.    --java-build                 Only Java ByteCode generation creation.
	echo.                                 (No effect when building for Micro)
	echo.    --cpp-build                  Only C++ code generation and compilation.
	echo.    --cpp03-build                Only C++ New PSM code generation and
	echo.                                 compilation.
	echo.                                 (No effect when building for Micro)
	echo.    --cs-build                   Only C Sharp code generation and compilation.
	echo.                                 (No effect when building for Micro)
	echo.    --cmake  path                Path to the CMAKE executable. If this
	echo.                                 parameter is not present, Cmake variable
	echo.                                 should be available from your $PATH variable.
	echo.    --cmake-generator g          CMake generator to use. By default, NMake
	echo.                                 makefiles will be generated.
	echo.    --perl path                  Path to PERL executable. If this parameter is
	echo.                                 not present, the path to PERL should be
	echo.                                 available from your \$PATH variable.
	echo.    --java-home path             Path to the Java JDK home folder. If this
	echo.                                 parameter is not present, javac, jar and java
	echo.                                 executables should be available from your
	echo.                                 $PATH variable.
	echo.                                 (No effect when building for Micro)
	echo.    --debug                      Compile against the RTI Connext Debug
	echo.                                 libraries. Default is against release ones.
	echo.    --dynamic                    Compile against the RTI Connext Dynamic
	echo.                                 libraries. Default is against static ones.
	echo.                                 (No effect when building for Micro)
	echo.    --secure                     Enable the security options for compilation.
	echo.                                 Default is not enabled.
	echo.    --openssl-home path          Path to the openssl home. This will be used
	echo.                                 when compiling statically and using security
	echo.                                 Note: For Micro provide this path with /
	echo.                                 instead of \, required by cmake.
	echo.    --clean                      If this option is present, the build.sh script
	echo.                                 will clean all the generated code and binaries
	echo.                                 from previous executions.
	echo.    --customType type            Use the Custom type feature with your type.
	echo.                                 See details and examples of use in the
	echo.                                 documentation.
	echo.    --customTypeFlatData type    Use the Custom type feature with your FlatData
	echo.                                 type. See details and examples of use in the
	echo.                                 documentation.
	echo.    --flatdata-max-size size     Specify the maximum bounded size in bytes
	echo.                                 for sequences when using FlatData language
	echo.                                 binding. Default 10MB
	echo.    --help -h                    Display this message.
	echo[
	echo ================================================================================
GOTO:EOF

:clean_src_cpp_common
	@REM # Remove copied file from srcCommon
	for %%i in (%common_cpp_folder%\*) do (
		del %modern_cpp_folder%\%%~nxi > nul 2>nul
		del %classic_cpp_folder%\%%~nxi > nul 2>nul
	)
GOTO:EOF

:copy_src_cpp_common
	@REM # Copy file from srcCommon to srcCpp and srcCpp03
	for %%i in (%common_cpp_folder%\*) do (
		call copy /Y %common_cpp_folder%\%%~nxi %modern_cpp_folder%\ > nul 2>nul
		call copy /Y %common_cpp_folder%\%%~nxi %classic_cpp_folder%\ > nul 2>nul
	)

GOTO:EOF

:clean_custom_type_files
	@REM # Remove generated files of the customer type
	for %%i in (%custom_type_folder%\*) do (
		del %idl_location%\%%~nxi > nul 2>nul
		del %script_location%\srcCpp\%%~niPlugin.* > nul 2>nul
		del %script_location%\srcCpp\%%~ni.* > nul 2>nul
		del %script_location%\srcCpp\%%~niSupport.* > nul 2>nul
	)
	del %custom_idl_file% > nul 2>nul

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
	del %script_location%srcCpp\perftest_ZeroCopy.* > nul 2>nul
	del %script_location%srcCpp\perftest_ZeroCopyPlugin.* > nul 2>nul
	del %script_location%srcCpp\perftest_ZeroCopySupport.* > nul 2>nul
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
	call::clean_custom_type_files
	call::clean_src_cpp_common

	echo[
	echo ================================================================================
GOTO:EOF
