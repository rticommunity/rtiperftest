# Executing the test

The test is provided in C++ (Modern and Traditional APIs), C#, and Java. The list below identifies how to run the executables, once you have built them, and how to pass configuration parameters to them. For detailed descriptions of the test parameters, see See *Test Parameters Section*. For example test configurations, see the *Example Command Lines* section.

When running the test, keep in mind that a throughput test will necessarily place a heavy load on your network and potentially on your CPU(s) as well. For the most accurate results, and the fewest complaints from your coworkers and IT department, run the test when and where you have a subnet to yourself. The test is designed to measure latency under loaded network conditions; it will produce those loads itself: There is no need to produce them externally (and your throughput results may not be meaningful if you do).

## C++ and C# executables

The C++ and C# executables are in these directories:

```
<installation directory>/bin/<architecture>/release
<installation directory>/bin/<architecture>/debug
```

Where `<architecture>` depends on your architecture, such as `i86Linux3gcc4.8.2` or `i86Win32VS2012`.

You can differentiate the executables for the two C++ implementations (Traditional and Modern) by the name: the Traditional C++ API implementation uses `perftest_cpp` and the Modern C++ API implementation is named `perftest_cpp03`.

The test uses an XML configuration file and locates this file based on paths relative to the directory from which the test is run. Therefore, to use this configuration file without the need of adding extra command line parameters:

Traditional C++:

```
bin/<architecture>/<release or debug>/perftest_cpp <-pub | -sub (default)> [parameters]
```

Modern C++:

```
bin/<architecture>/<release or debug>/perftest_cpp03 <-pub | -sub (default)> [parameters]
```

C#:

```
bin/<architecture>/<release or debug>/perftest_cs <-pub | -sub (default)> [parameters]
```

### When using dynamic linking

If you compiled the performance test executable dynamically add the `$NDDSHOME/lib/<architecture>` folder to:

- The `$LD_LIBRARY_PATH` variable if you are on Linux systems.
- The `$DYLD_LIBRARY_PATH` variable if you are on OSX. 
- The `%PATH%` variable (if you are on Windows).

### When using *RTI Secure DDS Plugin* and using dynamic linking

In such case, add the *OpenSSL* libraries in `$OPENSSLHOME/<debug or release>/lib` to the `$LD_LIBRARY_PATH`, `$DYLD_LIBRARY_PATH` or `%PATH%` variable:

- On Linux systems, add `$OPENSSLHOME/<debug or release>/lib` to `$LD_LIBRARY_PATH`
- On OSX systems, add `$OPENSSLHOME/<debug or release>/lib` to `$DYLD_LIBRARY_PATH`
- On Windows systems, add `%OPENSSLHOME$/<debug or release>/bin` to `%PATH%`

## Java executable

*RTI Perftest* provides a *.sh* script and a *.bat* script to run the Java `jar` file. Those scripts are located in:

- `bin/<debug or release>/perftest_java.sh` for UNIX-based systems.
- `bin/<debug or release>/perftest_java.bat` for Windows systems.

When using the *RTI Perftest* scripts, you will need to set the environment variable `$RTI_PERFTEST_ARCH` to your specific architecture as well as your `$NDDSHOME`.

**For example**:
If you are using a Windows 32-bit architecture and the *Visual Studio 2012* libraries:

```
set NDDSHOME=<path to your RTI Connext DDS installation>
set RTI_PERFTEST_ARCH=i86Win32VS2012
```

If you are using the Linux i86Linux3gcc4.8.2 architecture:

```
export NDDSHOME=<path to your RTI Connext DDS installation>
export RTI_PERFTEST_ARCH=i86Linux3gcc4.8.2
```
 
Make sure the correct `java` executable is in your path before running the script.

Alternatively, you can directly run the Java `jar` files:

```
java -cp "<RTI Perftest Path>/bin/<release or debug>/perftest_java.jar:<NDDSHOME>/lib/java/<nddsjava.jar or nddsjava.jar>" com.rti.perftest.ddsimpl.PerfTestLauncher
```

In such case, add the `$NDDSHOME/lib/<architecture>` and the `$NDDSHOME/lib/java` folders to:

- The `$LD_LIBRARY_PATH` variable if you are on Linux systems.
- The `$DYLD_LIBRARY_PATH` variable if you are on OSX. 
- The `%PATH%` variable (if you are on Windows).


### When using Java on UNIX-based systems with RTI Secure DDS

In such case, add the *OpenSSL* libraries in `$OPENSSLHOME/<debug or release>/lib` to the `$LD_LIBRARY_PATH` or `%PATH%` variable:

- On Linux systems, add `$OPENSSLHOME/<debug or release>/lib` to `$LD_LIBRARY_PATH`
- On OSX systems, add `$OPENSSLHOME/<debug or release>/lib` to `$DYLD_LIBRARY_PATH`
- On Windows systems, add `%OPENSSLHOME$/<debug or release>/bin` to `%PATH%`

## Launching the application

The test uses an XML configuration file. It locates this file based on its path relative to the directory from which the test is run. To use this configuration file move to *RTI Perftest* top-level location.

Start the test applications. You can start the publisher or subscribers first, the order does not matter. When selecting your optional parameters, choose parameters that allow the test to run for at least 15 seconds to get any kind of meaningful results. The longer it runs, the more accurate the results will be.

Ideally, you should run the test for at least 180 seconds.

### C++ Traditional API

```
bin/<architecture>/<release or debug>/perftest_cpp <-pub | -sub (default)> [parameters]
```

### C++ Modern API
```
bin/<architecture>/<release or debug>/perftest_cpp03 <-pub | -sub (default)> [parameters]
```

### C# API 
```
bin/<architecture>/<release or debug>/perftest_cs <-pub | -sub (default)> [parameters]
```
 
### Java API
```
bin/<release or debug>/perftest_java <-pub | -sub (default)> [parameters]
```

where `<architecture>` depends on your architecture, such as `x64Linux3gcc4.8.2` or `i86Win32VS2012`.

After the publisher recognizes that the specified number of subscribers (see the 
See `-numSubscribers <count>` option) are online and the subscriber recognizes that the specified number of publishers (see the See `-numPublishers <count>` option) are online, the test begins.
