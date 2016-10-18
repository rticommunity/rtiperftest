# Release Notes

## RTI Perftest 2.0 Compatibility

*RTI Perftest* 2.0 is designed to compile and work against the *RTI Connext DDS* 5.2.x releases.

However, certain features are not compatible with all the *RTI Connext DDS* versions, since the build scripts make use of certain specific parameters in *Rtiddsgen* that might change or not be present between releases:

- The `--secure` and `--openssl-home` parameters will not work for versions previous to *RTI Connext DDS* 5.2.5.
- The C# code generation against *RTI Connext DDS 5.2.0.x* is not supported. Users can disable its compilation by adding the `--skip-cs-build` flag.

## What's New in 2.0

### Platform support and build system

*RTI Perftest 2.0* makes use of the *RTI Connext DDS* *Rtiddsgen* tool in order to generate part of its code and also the makefile/project files used to compile that code.

Therefore, all the already generated makefiles and *Visual Studio* solutions have been removed and now the build system depends on 2 scripts: `build.sh` for Unix-based systems and `build.bat` for Windows systems.

*RTI Perftest* scripts works for every platform for which *Rtiddsgen* can generate an example, except for those in which *Rtiddsgen* doesn't generate regular makefiles or *Visual Studio Solutions* but specific project files. That is the case of *Android* platforms as well as the *iOS* ones.

Certain platforms will compile with the out of-the-box code and configurations, but further tuning could be needed in order to make the application run in the specific platform. The reason is usually the memory consumption of the application or the lack of support of the platform for certain features (like a file system).


### Improved directory structure

*RTI Perftest 2.0* directory structure has been cleaned up, having now a much more compact and consistent schema.

### Github

*RTI Perftest* development has been moved to a *GitHub* project. This will allow more frequently updates and code contributions.

The URL of the project is the following: [github.com/rticommunity/rtiperftest](github.com/rticommunity/rtiperftest).

### Numeration schema

*RTI Perftest* development and releases are now decoupled from *RTI Connext DDS* ones, therefore, and to avoid future numeration conflicts, *RTI Perftest* moved to a different numeration schema.

The compatibility between *RTI Perftest* versions and *RTI Connext DDS* ones will be clearly stated in the release notes of every *RTI Perftest* release, as well as in the top level `README.md` file.

### Documentation

Documentation is no longer provided as a PDF document, but as *markdown* files as well as in *html* format. You will be able to access to the documentation from the *RTI Community* page, as well as from the *GitHub* project.

### Support for UDPv6

Added command line parameter to force communication via UDPv6. By specifying `-enableUdpv6` you will only communicate data by using the UDPv6 transport.

The use of this feature will imply setting the `NDDS_DISCOVERY_PEERS` environment variable to (at least) one valid IPv6 address.

### Support for Dynamic data

Added command line parameter to specify the use of the Dynamic Data API instead of the regular *Rtiddsgen* generated code use.

### Simplified execution in VxWorks kernel mode

The execution in *VxWorks OS kernel mode* has been simplified for the user. Now the user can make use of `subscriber_main()` and `publisher_main()` and modify its content with all the parameters required for the tests.

### Decreased Memory Requirements for Latency Performance Test

The default number of iterations (samples sent by the performance test publisher side) when performing a latency test has been updated. Before, the default value was `100,000,000`. This value was used to internally allocate certain buffers, which imposed large memory requirements. The new value is `10,000,000` (10 times less).

## What's Fixed in 2.0

### RTI Perftest behavior when using multiple publishers

The previous behavior specified that an *RTI Perftest Subscriber* in a scenario with multiple *RTI Perftest Publishers* would stop receiving samples and exit after receiving the last sample from the *RTI Perftest* Publisher with `pid=0`. This behavior could lead into an hang state if some *RTI Perftest Publishers* with different `pid` were still missing to send new samples. 

The new behavior makes the *RTI Perftest Subscriber* wait until all the Perftest Publishers finish sending all their samples and then exit.

### Possible `std::bad_alloc` and Segmentation Fault in Latency Test in case of insufficient memory

When performing a latency performance test with traditional or modern C++, the test tries to allocate certain arrays of unsigned longs. These arrays can be quite large. On certain embedded platforms, due to memory limitations, this caused a `std::bad_alloc` error that was not properly captured, and a segmentation fault. This problem has been resolved. Now the performance test will inform you of the memory allocation issue and exit properly.

### Default Max Number of Instances on Subscriber Side Changed to `DDS_LENGTH_UNLIMITED`

In the previous release, if you did not set the maximum number of instances on the subscriber side, it would default to one instance. Therefore the samples for all instances except the first one were lost.

The new default maximum number of instances on the subscriber side has been changed from one to `DDS_LENGTH_UNLIMITED`. You can change this limit manually by setting the parameter `-instances <number>`.

### Error when using Shared Memory and Large Samples

When using *RTI Perftest* with large samples and enabling shared memory we could get into the following error:

```
Large data settings enabled (-dataLen > 63000).
[D0001|ENABLE]NDDS_Transport_Shmem_Property_verify:received_message_count_max < 1
[D0001|ENABLE]NDDS_Transport_Shmem_newI:Invalid transport properties.
```

## Known Issues

### Compiling manually on Windows when using the *RTI Security* plugin

*rtiddsgen* generated solutions for Windows allow 4 different configurations: 

* Debug
* Debug DLL
* Release
* Release DLL

However, *RTI Perftest 2.0* new build system is focused to only compile one of those modes at a time. Choosing the compilation mode should be done via the `-debug` and `-dynamic` flags.
