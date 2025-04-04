/*
 * (c) 2005-2024  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#define STRINGIFY(x) #x
#define TO_STRING(x) STRINGIFY(x)

#include "perftest_cpp.h"
#ifdef RTI_PERF_TSS
  #include "RTITSSImpl.h"
#elif defined(PERFTEST_RTI_PRO)
  #include "RTIRawTransportImpl.h"
  #include "RTIDDSImpl.h"
#elif defined(PERFTEST_RTI_MICRO)
  #include "RTIDDSImpl.h"
#elif defined(PERFTEST_CERT)
  #include "RTICertImpl.h"
#endif
#include "CpuMonitor.h"
#include "Infrastructure_common.h"

/*
 * We set 28 as the default value since this matches with the Micro use-case
 * and the default Pro use-case.
 */
unsigned int perftest_cpp::OVERHEAD_BYTES = 28;

#ifdef PERFTEST_RTI_PRO
#ifdef RTI_ANDROID

#include <android/log.h>
typedef int (*RTIAndroidOnPrintfMethod)(const char *format, va_list ap);
static RTIAndroidOnPrintfMethod publisher_onPrintf = NULL;

#define printf Android_printf
#define fprintf Android_fprintf

static int Android_printf(const char *format, ...) {
    int len;
    va_list ap;
    va_start(ap, format);

    if (publisher_onPrintf!= NULL) {
        len = publisher_onPrintf(format, ap);
    } else {
        len = __android_log_vprint(ANDROID_LOG_INFO, "RTIConnextLog", format, ap);
    }

    va_end(ap);
    return len;
}

static int Android_fprintf(FILE *fptr, const char *format, ...) {
    int len;
    va_list ap;
    va_start(ap, format);

    if (publisher_onPrintf!= NULL) {
        len = publisher_onPrintf(format, ap);
    } else {
        len = __android_log_vprint(ANDROID_LOG_INFO, "RTIConnextLog", format, ap);
    }

    va_end(ap);
    return len;
}

extern "C" void RTIAndroid_registerOnPrintf(RTIAndroidOnPrintfMethod onPrintf) {
    publisher_onPrintf = onPrintf;
}

#endif // RTI_ANDROID
#endif // RTI_PRO

bool perftest_cpp::_testCompleted = false;
const int timeout_wait_for_ack_sec = 0;
const unsigned int timeout_wait_for_ack_nsec = 100000000;
const Perftest_ProductVersion_t perftest_cpp::_version = {9, 9, 9, 9};

/*
 * PERFTEST-108
 * If we are performing a latency test, the default number for _NumIter will be
 * 10 times smaller than the default when performing a throughput test. This
 * will allow Perftest to work better in embedded platforms since the _NumIter
 * parameter sets the size of certain arrays in the latency test mode.
 */
const unsigned long long numIterDefaultLatencyTest = 10000000;

/*********************************************************
 * Main
 */
int main(int argc, char *argv[])
{

    perftest_cpp::_testCompleted = false;

    try {
        perftest_cpp app;
        return app.Run(argc, argv);
    } catch (const std::exception &ex) {
        // This will catch DDS exceptions
        fprintf(stderr, "Exception in perftest_cpp::Run(): %s.\n", ex.what());
        return -1;
    }
}

#if defined(PERFTEST_RTI_PRO) || defined(PERFTEST_RTI_MICRO)
#if defined(RTI_VXWORKS)
int perftest_cpp_main(char *args)
{
    std::vector<char *> arguments;
    char *next = NULL;
    char **argv = NULL;
    int argc = 0;

    // Run() expects also the executable name argv[0]
    arguments.push_back((char *) "perftest_cpp");

    // split args by " " and add each one to dynamic array
    next = strtok(args, " ");
    while (next != NULL) {
        arguments.push_back(next);
        next = strtok(NULL, " ");
    }

    // Copy dynamic array to the original
    argc = arguments.size();
    argv = new char*[argc];

    std::copy(arguments.begin(), arguments.end(), argv);

    // Call original main function with the splitted arguments
    return main(argc, argv);
}
#endif // RTI_VXWORKS
#endif // defined(PERFTEST_RTI_PRO) || defined(PERFTEST_RTI_MICRO)

int perftest_cpp::Run(int argc, char *argv[])
{
    unsigned short mask;
    print_version();

    try {
        _PM.initialize();
    } catch(std::exception &ex) {
        fprintf(stderr, "Exception in _PM.initialize(): %s.\n", ex.what());
        return -1;
    }
    if (_PM.check_help(argc, argv)) {
        return 0;
    }
    if (!_PM.parse(argc, argv)) {
        return -1;
    }
    if (!_PM.check_incompatible_parameters()) {
        return -1;
    }
    if (!validate_input()) {
        return -1;
    }

    if (_threadPriorities.isSet
            && !_threadPriorities.set_main_thread_priority()) {
        return -1;
    }

  #if defined(PERFTEST_RTI_PRO) || defined(PERFTEST_RTI_MICRO) || defined(RTI_PERF_TSS)
    if (_PM.get<bool>("rawTransport")) {
      #ifdef PERFTEST_RTI_PRO
        _MessagingImpl = new RTIRawTransportImpl();
      #endif
    } else {
        mask = (_PM.get<int>("unbounded") != 0) << 0;
        mask += _PM.get<bool>("keyed") << 1;
        mask += _PM.get<bool>("flatdata") << 2;
        mask += _PM.get<bool>("zerocopy") << 3;

      #ifndef RTI_PERF_TSS
        switch (mask)
        {
            case 0: // = 0000 (bounded)
                _MessagingImpl = new RTIDDSImpl<TestData_t>();
                break;

            case 1: // unbounded = 0001
                _MessagingImpl = new RTIDDSImpl<TestDataLarge_t>();
                break;

            case 2: // keyed = 0010
                _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>();
                break;

            case 3: // unbounded + keyed = 0011
                _MessagingImpl = new RTIDDSImpl<TestDataKeyedLarge_t>();
                break;

        #ifdef RTI_FLATDATA_AVAILABLE
          #ifdef RTI_ZEROCOPY_AVAILABLE
            case 15: // unbounded + keyed + flat + zero = 1111
                _MessagingImpl = new RTIDDSImpl_FlatData<TestDataKeyedLarge_ZeroCopy_w_FlatData_t>(true);
                break;

            case 14: // keyed + flat + zero = 1110
                _MessagingImpl = new RTIDDSImpl_FlatData<TestDataKeyed_ZeroCopy_w_FlatData_t>(true);
                break;

            case 13: // unbounded + flat + zero = 1101
                _MessagingImpl = new RTIDDSImpl_FlatData<TestDataLarge_ZeroCopy_w_FlatData_t>(true);
                break;

            case 12: // flat + Zero = 1100
                _MessagingImpl = new RTIDDSImpl_FlatData<TestData_ZeroCopy_w_FlatData_t>(true);
                break;
          #endif
            case 7: // unbounded + keyed + flat = 0111
                _MessagingImpl = new RTIDDSImpl_FlatData<TestDataKeyedLarge_FlatData_t>();
                break;

            case 6: // Keyed + flat = 0110
                _MessagingImpl = new RTIDDSImpl_FlatData<TestDataKeyed_FlatData_t>();
                break;

            case 5: // unbounded + flat = 0101
                _MessagingImpl = new RTIDDSImpl_FlatData<TestDataLarge_FlatData_t>();
                break;

            case 4: // flat = 0100
                _MessagingImpl = new RTIDDSImpl_FlatData<TestData_FlatData_t>();
                break;
                break;
        #endif

            default:
                break;
        }
      #else /* RTI_PERF_TSS defined */
        switch (mask)
        {
            case 0: // = 0000 (bounded)
                _MessagingImpl = new RTITSSImpl<FACE::DM::TestData_t,
                                        TestData_t::TypedTS,
                                        TestData_t::Read_Callback>(
                                            "FACE::DM::TestData_t");
                break;

            case 1: // unbounded = 0001
                _MessagingImpl = new RTITSSImpl<FACE::DM::TestDataLarge_t,
                                        TestDataLarge_t::TypedTS,
                                        TestDataLarge_t::Read_Callback>(
                                            "FACE::DM::TestDataLarge_t");
                break;

            case 2: // keyed = 0010
                _MessagingImpl = new RTITSSImpl<FACE::DM::TestDataKeyed_t,
                                        TestDataKeyed_t::TypedTS,
                                        TestDataKeyed_t::Read_Callback>(
                                            "FACE::DM::TestDataKeyed_t");
                break;

            case 3: // unbounded + keyed = 0011
                _MessagingImpl = new RTITSSImpl<FACE::DM::TestDataKeyedLarge_t,
                                        TestDataKeyedLarge_t::TypedTS,
                                        TestDataKeyedLarge_t::Read_Callback>(
                                            "FACE::DM::TestDataKeyedLarge_t");
                break;
        }
      #endif /* RTI_PERF_TSS */
    }
  #elif defined(PERFTEST_CERT)

    mask = _PM.get<bool>("keyed") << 0;
    #ifdef RTI_ZEROCOPY_AVAILABLE
    mask += _PM.get<bool>("zerocopy") << 1;
    #endif

    switch (mask)
        {
            case 0: // = 0000 (Not keyed)
                _MessagingImpl = new RTICertImpl<TestData_t, TestData_tSeq>(
                        TestData_tTypeSupport_get_type_name(),
                        TestData_tTypePlugin_get());
                break;

            case 1: // Keyed = 0001
                _MessagingImpl = new RTICertImpl<TestDataKeyed_t, TestDataKeyed_tSeq>(
                        TestDataKeyed_tTypeSupport_get_type_name(),
                        TestDataKeyed_tTypePlugin_get());
                break;

    #ifdef RTI_ZEROCOPY_AVAILABLE

            case 2: // = 0010 (zerocopy + Not keyed)
                _MessagingImpl = new RTICertImpl_ZCopy<TestData_Cert_ZCopy_t, TestData_Cert_ZCopy_tSeq>(
                        TestData_Cert_ZCopy_tTypeSupport_get_type_name(),
                        TestData_Cert_ZCopy_tTypePlugin_get());
                break;

            case 3: // zerocopy + Keyed = 0011
                _MessagingImpl = new RTICertImpl_ZCopy<TestDataKeyed_Cert_ZCopy_t, TestDataKeyed_Cert_ZCopy_tSeq>(
                        TestDataKeyed_Cert_ZCopy_tTypeSupport_get_type_name(),
                        TestDataKeyed_Cert_ZCopy_tTypePlugin_get());
                break;

    #endif
            default:
                break;
        }
  #else // No middleware is passed as -DPERFTEST_...

    fprintf(stderr,
            "[Error] There is no implementation for this middleware for the "
            "_MessagingImpl object.\n");

  #endif

    if (_MessagingImpl == NULL || !_MessagingImpl->initialize(_PM, this)) {
        fprintf(stderr,
                "[Error]: _MessagingImpl was null or it could not be "
                "initialized\n");
        return -1;
    }

    std::string outputFormat = _PM.get<std::string>("outputFormat");
    if (outputFormat == "csv") {
        _printer = new PerftestCSVPrinter();
    } else if (outputFormat == "json") {
        _printer = new PerftestJSONPrinter();
    } else if (outputFormat == "legacy") {
        _printer = new PerftestLegacyPrinter();
    }

    if (!_printer->initialize(&_PM)) {
        fprintf(stderr,
            "[Error] Issue initializing printer class.\n");
        return -1;
    }

    print_configuration();

    if (_PM.get<bool>("pub")) {
        return Publisher();
    } else {
        return Subscriber();
    }
}

const Perftest_ProductVersion_t perftest_cpp::get_perftest_version()
{
    return _version;
}

void perftest_cpp::print_version()
{
    Perftest_ProductVersion_t perftestV = perftest_cpp::get_perftest_version();

    if (perftestV.major == 0
            && perftestV.minor == 0
            && perftestV.release == 0) {
        fprintf(stderr, "RTI Perftest Develop");
    } else if (perftestV.major == 9
            && perftestV.minor == 9
            && perftestV.release == 9) {
        fprintf(stderr, "RTI Perftest Master");
    } else {
        fprintf(stderr, "RTI Perftest %d.%d.%d",
                perftestV.major,
                perftestV.minor,
                perftestV.release);

        if (perftestV.revision != 0) {
            fprintf(stderr, ".%d", perftestV.revision);
        }
    }

  #ifdef PERFTEST_COMMIT_ID
    fprintf(stderr, " %s", PERFTEST_COMMIT_ID);
  #endif
  #ifdef PERFTEST_BRANCH_NAME
    fprintf(stderr, " (%s)", PERFTEST_BRANCH_NAME);
  #endif

    //TODO: Change this so it uses a function from the MessagingImpl object
    fprintf(stderr, " (%s)\n", GetMiddlewareVersionString().c_str());
    fflush(stderr);
}

/*********************************************************
 * Destructor
 */
perftest_cpp::~perftest_cpp()
{
    if(_MessagingImpl != NULL){
        delete _MessagingImpl;
    }

    if (_printer != NULL) {
        delete _printer;
    }

    fprintf(stderr, "Test ended.\n");
    fflush(stderr);
}

/*********************************************************
 * Constructor
 */
perftest_cpp::perftest_cpp()
#if defined(PERFTEST_RTI_PRO)
    : _PM(Middleware::RTIDDSPRO)
#elif defined(PERFTEST_RTI_MICRO)
    : _PM(Middleware::RTIDDSMICRO)
#elif defined(RTI_PERF_TSS_PRO)
    : _PM(Middleware::RTITSSPRO)
#elif defined(RTI_PERF_TSS_MICRO)
    : _PM(Middleware::RTITSSMICRO)
#elif defined(PERFTEST_CERT)
    : _PM(Middleware::RTICERT)
#else
    : _PM()
#endif
{
    subID = 0;
    printIntervals = true;
    showCpu = false;

    _SpinLoopCount = 0;
    _SleepNanosec = 0;
    _MessagingImpl = NULL;
    _printer = NULL;

    /*
     * We use rand to generate the key of a SHMEM segment when
     * we estimate the maximum buffer size for SHMEM
     */
    srand(time(NULL));
};


/*********************************************************
 * Validate and manage the parameters
 */
bool perftest_cpp::validate_input()
{
    // Manage parameter -batchSize for micro
  #if !defined(PERFTEST_RTI_PRO) && !defined(RTI_PERF_TSS_PRO)
    _PM.set("batchSize", 0);
  #endif

    // Manage parameter -sleep
    // It is copied because it is used in the critical path
    _SleepNanosec = 1000000 * (unsigned long)_PM.get<unsigned long long>("sleep");

    // Manage parameter -spin
    // It is copied because it is used in the critical path
    _SpinLoopCount = _PM.get<unsigned long long>("spin");

    // Manage parameter -printIterval
    // It is copied because it is used in the critical path
    printIntervals = !_PM.get<bool>("noPrintIntervals");

    // Manage parameter -cpu
    // It is copied because it is used in the critical path
    showCpu = _PM.get<bool>("cpu");

    // Manage parameter -sidMultiSubTest
    // It is copied because it is used in the critical path
    subID = _PM.get<int>("sidMultiSubTest");

    // Manage parameter -latencyTest
    if (_PM.get<bool>("latencyTest")) {
        if (_PM.get<int>("pidMultiPubTest") != 0) {
            fprintf(stderr,
                    "Only the publisher with ID = 0 can run the latency test\n");
            return false;
        }

        // In latency test mode, latencyCount should be 1
        if (!_PM.is_set("latencyCount")) {
            _PM.set<unsigned long long>("latencyCount", 1);
        }

        /*
         * PERFTEST-108
         * If we are in a latency test, the default value for _NumIter has to
         * be smaller (to avoid certain issues in platforms with low memory).
         * Therefore, unless we explicitly changed the _NumIter value we will
         * use a smaller default: "numIterDefaultLatencyTest"
         */
        if (!_PM.is_set("numIter")) {
            _PM.set<unsigned long long>("numIter", numIterDefaultLatencyTest);
        }
    }

    // Manage parameter -latencyCount
    if (!_PM.is_set("latencyCount")) {
        _PM.set<unsigned long long>("latencyCount", 10000);
    }
    if (_PM.get<unsigned long long>("numIter") <
            _PM.get<unsigned long long>("latencyCount")) {
        fprintf(stderr,
                "numIter (%llu) must be greater than latencyCount (%llu).\n",
                _PM.get<unsigned long long>("numIter"),
                _PM.get<unsigned long long>("latencyCount"));
        return false;
    }

    // Manage the parameter: -cft
    if (_PM.is_set("cft")) {
        const std::vector<unsigned long long> cftRange =
                _PM.get_vector<unsigned long long>("cft");
        if (cftRange.size() > 2) {
            fprintf(stderr,
                    "'-cft' value must have the format <start>:<end>\n");
            return false;
        } else if (cftRange[0] > cftRange[1]) {
            fprintf(stderr,
                    "'-cft' <start> value cannot be bigger than <end>\n");
            return false;
        }
    }

    // Manage the parameter: -pubRate -sleep -spin
    if (_PM.is_set("pubRate") || _PM.is_set("pubRatebps")) {
        if (_SpinLoopCount > 0) {
            fprintf(stderr, "'-spin' is not compatible with '-pubRate'. "
                    "Spin/Sleep value will be set by -pubRate.\n");
            _SpinLoopCount = 0;
        }
        if (_SleepNanosec > 0) {
            fprintf(stderr, "'-sleep' is not compatible with '-pubRate'. "
                    "Spin/Sleep value will be set by -pubRate.\n");
            _SleepNanosec = 0;
        }
    }

    if (_PM.is_set("loadDataFromFile")){
        /*
         * Load a file in memory if realPayload is set.
         * If dataLen is not defined, we will use the size of the file.
         * This function may take some time to load
         */
        if(!_fileDataLoader.initialize(
                _PM.get<std::string>("loadDataFromFile"),
                &_PM)) {
            fprintf(stderr, "Could not start the fileDataLoader.\n");
            return false;
        }

    }

    // Manage the parameter: -unbounded
    if (_PM.is_set("unbounded")) {
        if (_PM.get<int>("unbounded") == 0) { // Is the default
            _PM.set<int>("unbounded", (int)(std::min)(
                    2 * _PM.get<unsigned long long>("dataLen"),
                    (unsigned long long)MAX_BOUNDED_SEQ_SIZE));
        }
    }

    // Manage the parameter: -threadPriorities
    if (_PM.is_set("threadPriorities")) {
        const std::string priorities = _PM.get<std::string>("threadPriorities");
        if (!_threadPriorities.parse_priority(priorities)) {
            fprintf(stderr, "Could not set -threadPriorities.\n");
            return false;
        }
        _threadPriorities.isSet = true;
    }

    // Check if we need to enable the use of unbounded sequences.
    if (_PM.get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
        if (_PM.get<int>("unbounded") == 0) {
            _PM.set<int>("unbounded", MAX_BOUNDED_SEQ_SIZE);
        }
    }

    /* RawTransport only allow listeners by threads */
    if (_PM.get<bool>("rawTransport")) {
        _PM.set("useReadThread", true);
    }

    // Manage the lowResolutionClock parameter
    if (_PM.get<bool>("lowResolutionClock")) {
        if (_PM.get<unsigned long long>("latencyCount") != 1) {
            fprintf(stderr,
                "The -lowResolutionClock option should only be used if "
                "latencyCount is 1.\n");
            return false;
        }

        if (_PM.is_set("sleep")
                || _PM.is_set("pubRate")
                || _PM.is_set("spin")) {
            fprintf(stderr,
                "The -lowResolutionClock cannot be used with the "
                "sleep, spin or pubRate command-line options.\n");
            return false;
        }
    }

    #ifdef RTI_FLATDATA_AVAILABLE
      // Automatically enable FlatData when using Zero Copy
      if (_PM.get<bool>("zerocopy") && !_PM.get<bool>("flatdata")) {
           _PM.set<bool>("flatdata", true);
      }

      if (_PM.get<bool>("zerocopy") && 
            !(_PM.get<std::string>("transport") == "SHMEM" 
                || _PM.get<std::string>("transport") == "Use XML"
                || _PM.get<std::string>("transport") == "UDPv4 & SHMEM"
                || _PM.get<std::string>("transport") == "UDPv6 & SHMEM"
                || _PM.get<std::string>("transport") == "UDPv4 & UDPv6 & SHMEM")) {
          fprintf(stderr, "Zero Copy must be run with SHMEM as transport\n");
          return false;
      }

      if (_PM.get<bool>("checkconsistency") && !_PM.get<bool>("zerocopy")) {
          fprintf(stderr, "CheckConsistency can only be used along with Zero Copy\n");
          return false;
      }
    #endif

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
void perftest_cpp::print_configuration()
{
    std::ostringstream stringStream;

  #ifdef RTI_CUSTOM_TYPE
    stringStream << "\nCustom Type provided: '"
                 << TO_STRING(RTI_CUSTOM_TYPE)
                 << "'\n";
  #endif

    // Throughput/Latency mode
    if (_PM.get<bool>("pub")) {
        stringStream << "\nMode: ";
        if (_PM.get<bool>("latencyTest")) {
            stringStream << "LATENCY TEST (Ping-Pong test)\n";
        } else {
            stringStream << "THROUGHPUT TEST\n"
                         << "      (Use \"-latencyTest\" for Latency Mode)\n";
        }
    }

    stringStream << "\nPerftest Configuration:\n";

    // Reliable/Best Effort
    stringStream << "\tReliability: ";
    if (!_PM.get<bool>("bestEffort")) {
        stringStream << "Reliable\n";
    } else {
        stringStream << "Best Effort\n";
    }

    // Keyed/Unkeyed
    stringStream << "\tKeyed: ";
    if (_PM.get<bool>("keyed")) {
        stringStream << "Yes\n";
    } else {
        stringStream << "No\n";
    }

    // Publisher/Subscriber and Entity ID
    if (_PM.get<bool>("pub")) {
        stringStream << "\tPublisher ID: "
                     << _PM.get<int>("pidMultiPubTest")
                     << "\n";
    } else {
        stringStream << "\tSubscriber ID: " << subID << "\n";
    }

    if (_PM.get<bool>("pub")) {
        // Latency Count
        stringStream << "\tLatency count: 1 latency sample every "
                     << _PM.get<unsigned long long>("latencyCount")
                     << " samples\n";

        // Data Sizes
        stringStream << "\tData Size: ";
        stringStream << _PM.get<unsigned long long>("dataLen") << "\n";

      #ifdef PERFTEST_RTI_PRO
        // Batching
        stringStream << "\tBatching: ";
        if (_PM.get<long>("batchSize") > 0) {
            stringStream << _PM.get<long>("batchSize")
                         << " Bytes (Use \"-batchSize 0\" to disable batching)\n";
        } else if (_PM.get<long>("batchSize") == 0) {
            stringStream << "No (Use \"-batchSize\" to setup batching)\n";
        } else { // <= 0
            stringStream << "Disabled by RTI Perftest.\n";
            if (_PM.get<long>("batchSize") == -1) {
                if (_PM.get<bool>("latencyTest")) {
                    stringStream << "\t\t  BatchSize disabled for a Latency Test\n";
                } else {
                    stringStream << "\t\t  BatchSize is smaller than 2 times\n"
                                 << "\t\t  the sample size.\n";
                }
            } else if (_PM.get<long>("batchSize") == -2) {
                stringStream << "\t\t  BatchSize cannot be used with\n"
                             << "\t\t  Fragmented Samples.\n";
            } else if (_PM.get<long>("batchSize") == -3) {
                stringStream << "\t\t  BatchSize cannot be used with\n"
                             << "\t\t  FlatData and/or Zero-Copy.\n";
            } else if (_PM.get<long>("batchSize") == -4) {
                stringStream << "\t\t  BatchSize disabled by default.\n"
                             << "\t\t  when using -pubRate.\n";
            }
        }
      #endif

        // Publication Rate
        stringStream << "\tPublication Rate: ";
        if (_PM.is_set("pubRate")) {
            stringStream << _PM.get_pair<unsigned long long, std::string>("pubRate").first
                         << " Samples/s (";
            if (_PM.get_pair<unsigned long long, std::string>("pubRate").second
                    == "spin") {
                stringStream << "Spin)\n";
            } else {
                stringStream << "Sleep)\n";
            }
        } else if (_PM.is_set("pubRatebps")) {
            stringStream << _PM.get_pair<unsigned long long, std::string>("pubRatebps").first
                         << " bps (";
            if (_PM.get_pair<unsigned long long, std::string>("pubRatebps").second
                    == "spin") {
                stringStream << "Spin)\n";
            } else {
                stringStream << "Sleep)\n";
            }
        } else {
            stringStream << "Unlimited (Not set)\n";
        }

        // Execution Time or NumIter
        if (_PM.get<unsigned long long>("executionTime") > 0) {
            stringStream << "\tExecution time: "
                         << _PM.get<unsigned long long>("executionTime")
                         << " seconds\n";
        } else {
            stringStream << "\tNumber of samples: "
                         << _PM.get<unsigned long long>("numIter")
                         << "\n";
        }

        // Manage the lowResolutionClock parameter
        if (_PM.get<bool>("lowResolutionClock")) {
            stringStream << "\tLow resolution clock latency measurements.\n";
        }

    } else {
        stringStream << "\tData Size: " << _PM.get<unsigned long long>("dataLen");
        stringStream << std::endl;
    }

    // Listener/WaitSets
    if (!_PM.get<bool>("rawTransport")) {
        stringStream << "\tReceive using: ";
        if (_PM.get<bool>("useReadThread")) {
            stringStream << "WaitSets\n";
        } else {
            stringStream << "Listeners\n";
        }
    }

    // Thread priority
    if (_threadPriorities.isSet) {
        stringStream << "\tUsing thread priorities:" << std::endl;
        stringStream << "\t\tMain thread Priority: "
                << _threadPriorities.main << std::endl;
        stringStream << "\t\tReceive thread Priority: "
                << _threadPriorities.receive << std::endl;
        stringStream << "\t\tDataBase and Event threads Priority: "
                << _threadPriorities.dbAndEvent << std::endl;
    }

    stringStream << _MessagingImpl->print_configuration();

    if (_PM.get<bool>("cpu") && !CpuMonitor::available_in_os()) {
        fprintf(stderr,
                "\n[Warning] CPU consumption feature is not available in this OS.\n");
    }

    // We want to expose if we are using or not the unbounded type
    if (_PM.get<int>("unbounded")) {
        stringStream << "\n[IMPORTANT]: Using the Unbounded Sequence Type.";
        if (_PM.get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
            stringStream << " -datalen ("
                        << _PM.get<unsigned long long>("dataLen")
                        << ") is \n             larger than MAX_BOUNDED_SEQ_SIZE ("
                        << MAX_BOUNDED_SEQ_SIZE << ")";
        }
        stringStream << "\n";
    }

  #ifdef PERFTEST_CONNEXT_PRO_610
    if (_PM.get<std::string>("compressionId").find("NONE") == std::string::npos) {
        if (_PM.get<int>("compressionLevel") == 0) {
            stringStream << "\n[IMPORTANT]: This compression Level disabled "
                            "all compression.";
        } else if (
                _PM.get<int>("compressionThreshold") == DDS_LENGTH_UNLIMITED) {
            stringStream << "\n[IMPORTANT]: This compression Threshold "
                            "disabled all compression.";
        } else {
            stringStream << "\n[IMPORTANT]: Compression Enabled.";
            stringStream << "\n\t Compression Algorithm: " << _PM.get<std::string>("compressionId");
            stringStream << "\n\t Compression Level: " << _PM.get<int>("compressionLevel");
            stringStream << "\n\t Compression Threshold: " << _PM.get<int>("compressionThreshold");
        }
        stringStream << "\n";
    }
  #endif

    fprintf(stderr, "%s\n", stringStream.str().c_str());

}

/*********************************************************
 * Listener for the Subscriber side
 *
 * Keeps stats on data received per second.
 * Returns a ping for latency packets
 */
class ThroughputListener : public IMessagingCB
{
  private:
    ParameterManager *_PM;
    PerftestPrinter *_printer;
    int  subID;
    bool printIntervals;
    bool cacheStats;
    bool showCpu;

  public:

    unsigned long long packets_received;
    unsigned long long bytes_received;
    unsigned long long missing_packets;
    int                last_data_length;

    // store info for the last data set
    int                interval_data_length;
    unsigned long long interval_packets_received;
    unsigned long long interval_bytes_received;
    unsigned long long interval_missing_packets;
    unsigned long long interval_time, begin_time;
    float missing_packets_percent;
    unsigned int sample_count_peak;

    IMessagingWriter *_writer;
    IMessagingReader *_reader;
    unsigned long * _last_seq_num;

    int _num_publishers;
    std::vector<int> _finished_publishers;
    CpuMonitor cpu;
    bool _useCft;


    ThroughputListener(
            ParameterManager &PM,
            PerftestPrinter *printer,
            IMessagingWriter *writer,
            IMessagingReader *reader = NULL,
            bool UseCft = false,
            int numPublishers = 1)
    {
        packets_received = 0;
        bytes_received = 0;
        missing_packets = 0;
        end_test = false;
        last_data_length = -1;
        interval_data_length = -1;
        interval_packets_received = 0;
        interval_bytes_received = 0;
        interval_missing_packets = 0;
        sample_count_peak = 0;
        interval_time = 0;
        missing_packets_percent = 0.0;
        begin_time = 0;
        _writer = writer;
        _reader = reader;
        _last_seq_num = new unsigned long[numPublishers];
        _useCft = UseCft;

        for (int i = 0; i < numPublishers; i++) {
            _last_seq_num[i] = 0;
        }

        _num_publishers = numPublishers;

        _PM = &PM;
        _printer = printer;

        printIntervals = !_PM->get<bool>("noPrintIntervals");
        cacheStats = _PM->get<bool>("cacheStats");
        showCpu = _PM->get<bool>("cpu");
        subID = _PM->get<int>("sidMultiSubTest");
    }

    ~ThroughputListener() {
        if (_last_seq_num != NULL) {
            delete []_last_seq_num;
        }
    }

    void process_message(TestMessage &message)
    {
      #ifdef DEBUG_PING_PONG
        printf("-- ProcessMessage ...\n");
      #endif
        if (message.entity_id >= _num_publishers || message.entity_id < 0) {
            printf("ProcessMessage: message content no valid. message.entity_id out of bounds\n");
            return;
        }
        // Check for test initialization messages
        if (message.size == perftest_cpp::INITIALIZE_SIZE)
        {
            _writer->send(message);
            _writer->flush();
            return;
        }
        else if (message.size == perftest_cpp::FINISHED_SIZE)
        {
            /*
             * PERFTEST-97
             * We check the entity_id of the publisher to see if it has already
             * send a FINISHED_SIZE message. If he has we ignore any new one.
             * Else, we add it to a vector. Once that vector contains all the
             * ids of the publishers the subscriber is suppose to know, that
             * means that all the publishers have finished sending data samples,
             * so it is time to finish the subscriber.
             */
            if (std::find(
                        _finished_publishers.begin(),
                        _finished_publishers.end(),
                        message.entity_id)
                != _finished_publishers.end()) {
                return;
            }

            if (end_test)
            {
                return;
            }

            _finished_publishers.push_back(message.entity_id);

            if (_finished_publishers.size() >= (unsigned int)_num_publishers) {
                print_summary_throughput(message, true);
                end_test = true;
            }
            return;
        }

        // Send back a packet if this is a ping
        if ((message.latency_ping == subID)
                || (_useCft && message.latency_ping != -1)) {
          #ifdef DEBUG_PING_PONG
            printf("-- Answering Ping ...\n");
          #endif
            _writer->send(message);
            _writer->flush();
        }

        if (message.size != last_data_length) {
            packets_received = 0;
            bytes_received = 0;
            missing_packets = 0;

            for (int i=0; i<_num_publishers; i++) {
                _last_seq_num[i] = 0;
            }

            begin_time = PerftestClock::getInstance().getTime();
            _printer->_dataLength = message.size + perftest_cpp::OVERHEAD_BYTES;
            _printer->print_throughput_header();
        }

        last_data_length = message.size;
        ++packets_received;
        bytes_received += (unsigned long long) (message.size + perftest_cpp::OVERHEAD_BYTES);

        if (!_useCft) {
            // detect missing packets
            if (_last_seq_num[message.entity_id] == 0) {
                _last_seq_num[message.entity_id] = message.seq_num;
            } else {
                if (message.seq_num != ++_last_seq_num[message.entity_id]) {
                    // only track if skipped, might have restarted pub
                    if (message.seq_num > _last_seq_num[message.entity_id])
                    {
                        missing_packets +=
                                message.seq_num - _last_seq_num[message.entity_id];
                    }
                    _last_seq_num[message.entity_id] = message.seq_num;
                }
            }
        }
    }

    void print_summary_throughput(TestMessage &message, bool endTest = false){

        // store the info for this interval
        unsigned long long now = PerftestClock::getInstance().getTime();

        if (interval_data_length != last_data_length) {

            if (!_useCft) {
                // detect missing packets
                if (message.seq_num != _last_seq_num[message.entity_id]) {
                    // only track if skipped, might have restarted pub
                    if (message.seq_num > _last_seq_num[message.entity_id])
                    {
                        missing_packets +=
                                message.seq_num - _last_seq_num[message.entity_id];
                    }
                }
            }

            interval_time = now - begin_time;
            interval_packets_received = packets_received;
            interval_bytes_received = bytes_received;
            interval_missing_packets = missing_packets;
            interval_data_length = last_data_length;
            missing_packets_percent = 0;

            // Calculations of missing package percent
            if (interval_packets_received + interval_missing_packets != 0) {
                missing_packets_percent = (float) ((interval_missing_packets * 100.0)
                        / (float) (interval_packets_received
                        + interval_missing_packets));
            }

            double outputCpu = 0.0;
            if (showCpu) {
                outputCpu = cpu.get_cpu_average();
                cpu = CpuMonitor();
                cpu.initialize();
            }
            _printer->print_throughput_summary(
                    interval_data_length + perftest_cpp::OVERHEAD_BYTES,
                    interval_packets_received,
                    interval_time,
                    interval_bytes_received,
                    interval_missing_packets,
                    missing_packets_percent,
                    outputCpu);
            if (cacheStats) {
                printf("Samples Ping Reader Queue Peak: %4d\n", sample_count_peak);
            }

            fflush(stdout);
        } else if (endTest) {
            fprintf(stderr,
                    "\nNo samples have been received by the Subscriber side,\n"
                    "however 1 or more Publishers sent the finalization message.\n\n"
                    "There are several reasons why this could happen:\n"
                    "- If you are using large data, make sure to correctly adjust your\n"
                    "  sendQueue, reliability protocol and flowController.\n"
                    "- Make sure your -executionTime or -numIter in the Publisher side\n"
                    "  are big enough.\n"
                    "- Try sending at a slower rate -pubRate in the Publisher side.\n\n");
        }

        packets_received = 0;
        bytes_received = 0;
        missing_packets = 0;
        _last_seq_num[0] = 0;
        begin_time = now;
    }
};

/*********************************************************
 * Used for receiving data using a thread instead of callback
 *
 */
template<class ListenerType>
static void *ReadThread(void *arg)
{
    ListenerType *listener = static_cast<ListenerType *>(arg);
    TestMessage *message = NULL;

    PerftestSemaphore *semaphore = listener->get_synchronization_semaphore();

    if (semaphore == NULL) {
        fprintf(stderr,
                "Unexpected error creating a synchronization semaphore\n");
        return NULL;
    }

    while (!listener->end_test) {
        // Receive message should block until a message is received
        message = listener->_reader->receive_message();

        if (message != NULL) {
            listener->process_message(*message);
        }
    }

    // This will allow the main thread to delete the thread that used this function.

    if (!PerftestSemaphore_give(semaphore)) {
        fprintf(stderr, "Unexpected error giving semaphore\n");
        return NULL;
    }

    return NULL;
}

/*********************************************************
 * Subscriber
 */
int perftest_cpp::Subscriber()
{
    ThroughputListener *reader_listener = NULL;
    IMessagingReader *reader;
    IMessagingWriter *writer;
    IMessagingWriter *announcement_writer;
    struct PerftestThread *throughputThread = NULL;

    // create latency pong writer
    writer = _MessagingImpl->create_writer(LATENCY_TOPIC_NAME);
    if (writer == NULL) {
        fprintf(stderr, "Problem creating latency writer.\n");
        return -1;
    }

    // Check if using callbacks or read thread
    if (!_PM.get<bool>("useReadThread")) {
        // create latency pong reader
        reader_listener = new ThroughputListener(
                _PM,
                _printer,
                writer,
                NULL,
                _PM.is_set("cft"),
                _PM.get<int>("numPublishers"));
        reader = _MessagingImpl->create_reader(
                THROUGHPUT_TOPIC_NAME,
                reader_listener);
        if (reader == NULL)
        {
            fprintf(stderr, "Problem creating throughput reader.\n");
            return -1;
        }
    } else {
        reader = _MessagingImpl->create_reader(
                THROUGHPUT_TOPIC_NAME,
                NULL);
        if (reader == NULL)
        {
            fprintf(stderr, "Problem creating throughput reader.\n");
            return -1;
        }
        reader_listener = new ThroughputListener(
                _PM,
                _printer,
                writer,
                reader,
                _PM.is_set("cft"),
                _PM.get<int>("numPublishers"));

        int threadPriority = Perftest_THREAD_PRIORITY_DEFAULT;
        int threadOptions = Perftest_THREAD_OPTION_DEFAULT;

        if (_threadPriorities.isSet) {
            threadOptions = Perftest_THREAD_SETTINGS_REALTIME_PRIORITY
                    | Perftest_THREAD_SETTINGS_PRIORITY_ENFORCE;
            threadPriority = _threadPriorities.receive;
        }

        throughputThread = PerftestThread_new(
                "ReceiverThread",
                threadPriority,
                threadOptions,
                ReadThread<ThroughputListener>,
                reader_listener);
        if (throughputThread == NULL) {
            fprintf(stderr, "Problem creating ReceiverThread for ThroughputReadThread.\n");
            return -1;
        }
    }

    // Create announcement writer
    announcement_writer = _MessagingImpl->create_writer(
            ANNOUNCEMENT_TOPIC_NAME);

    if (announcement_writer == NULL) {
        fprintf(stderr, "Problem creating announcement writer.\n");
        return -1;
    }

    // Synchronize with publishers
    fprintf(stderr,
            "Waiting to discover %d publishers ...\n",
            _PM.get<int>("numPublishers"));
    fflush(stderr);
    reader->wait_for_writers(_PM.get<int>("numPublishers"));
    // In a multi publisher test, only the first publisher will have a reader.
    writer->wait_for_readers(1);
    announcement_writer->wait_for_readers(_PM.get<int>("numPublishers"));

    /*
     * Announcement message that will be used by the announcement_writer
     * to send information to the Publisher. This message size will indicate
     * different things.
     *
     * We will use 2 sizes: INITIALIZE_SIZE and FINISHED_SIZE,
     * msg.data will be used as the payload of such messages, so we choose the
     * greatest size:
     */
    TestMessage announcement_msg;
    announcement_msg.entity_id = subID;
    announcement_msg.size = perftest_cpp::INITIALIZE_SIZE;
    announcement_msg.data = new char[FINISHED_SIZE];
    memset(announcement_msg.data, 0, FINISHED_SIZE);

    // Send announcement message
    do {
        announcement_writer->send(announcement_msg);
        announcement_writer->flush();
        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        /*
         * Send announcement message until the publisher replies us.
         */
    } while (reader_listener->packets_received == 0);

    fprintf(stderr,"Waiting for data ...\n");
    fflush(stderr);

    // For Json format, print brackets at init
    _printer->print_initial_output();

    // wait for data
    unsigned long long prev_time = 0, now = 0, delta = 0;
    unsigned long long prev_count = 0;
    unsigned long long prev_bytes = 0;
    unsigned long long ave_count = 0;
    int last_data_length = -1;
    unsigned long long mps = 0, bps = 0;
    double mps_ave = 0.0, bps_ave = 0.0;
    unsigned long long msgsent, bytes, last_msgs, last_bytes;
    float missing_packets_percent = 0;

    const bool cacheStats = _PM.get<bool>("cacheStats");

    if (showCpu) {
        reader_listener->cpu.initialize();
    }

    now = PerftestClock::getInstance().getTime();

    while (true) {
        prev_time = now;
        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        now = PerftestClock::getInstance().getTime();

        if (reader_listener->end_test) { // ACK end_test
            announcement_msg.entity_id = subID;
            announcement_msg.size = FINISHED_SIZE;
            announcement_writer->send(announcement_msg);
            announcement_writer->flush();
            break;
        }

        if (printIntervals) {
            if (last_data_length != reader_listener->last_data_length)
            {
                last_data_length = reader_listener->last_data_length;
                prev_count = reader_listener->packets_received;
                prev_bytes = reader_listener->bytes_received;
                bps_ave = 0;
                mps_ave = 0;
                ave_count = 0;
                continue;
            }

            last_msgs = reader_listener->packets_received;
            last_bytes = reader_listener->bytes_received;
            msgsent = last_msgs - prev_count;
            bytes = last_bytes - prev_bytes;
            prev_count = last_msgs;
            prev_bytes = last_bytes;
            delta = now - prev_time;
            mps = (msgsent * 1000000 / delta);
            bps = (bytes * 1000000 / delta);

            // calculations of overall average of mps and bps
            ++ave_count;
            bps_ave = bps_ave + (double)(bps - bps_ave) / (double)ave_count;
            mps_ave = mps_ave + (double)(mps - mps_ave) / (double)ave_count;

            // Calculations of missing package percent
            if (last_msgs + reader_listener->missing_packets == 0) {
                missing_packets_percent = 0.0;
            } else {
                missing_packets_percent = (float)
                        ((reader_listener->missing_packets * 100.0)
                        / (float) (last_msgs + reader_listener->missing_packets));
            }

            if (last_msgs > 0) {
                double outputCpu = 0.0;
                if (showCpu) {
                    outputCpu = reader_listener->cpu.get_cpu_instant();
                }
                _printer->print_throughput_interval(
                        last_msgs,
                        mps,
                        mps_ave,
                        bps,
                        bps_ave,
                        reader_listener->missing_packets,
                        missing_packets_percent,
                        outputCpu);
                fflush(stdout);

                if (cacheStats) {
                    printf("Samples Ping Reader Queue: %4d (Peak: %4d)",
                            reader->get_sample_count(),
                            reader->get_sample_count_peak());
                    printf(" Samples Pong Writer Queue: %3d (Peak: %3d)\n",
                            writer->get_sample_count(),
                            writer->get_sample_count_peak());
                    reader_listener->sample_count_peak = reader->get_sample_count_peak();
                }
            }
        }
    }

    PerftestClock::milliSleep(2000);
    _printer->print_final_output();
    if (!finalize_read_thread(throughputThread, reader_listener)) {
        fprintf(stderr, "Error deleting throughputThread\n");
        return -1;
    }

    if (reader != NULL)
    {
        delete(reader);
    }

    if (reader_listener != NULL) {
        delete(reader_listener);
    }

    if (writer != NULL) {
        delete(writer);
    }

    if (announcement_writer != NULL) {
        delete(announcement_writer);
    }

    delete[] announcement_msg.data;

    fprintf(stderr,"Finishing test...\n");
    fflush(stderr);

    return 0;
}

/*********************************************************
 * Data listener for the Announcement
 *
 * Receives an announcement message from a Subscriber once
 * the subscriber has discovered every Publisher.
 */
class AnnouncementListener : public IMessagingCB
{
  public:
    int announced_subscribers;
    IMessagingReader *_reader;

  public:
    std::vector<int> subscriber_list;
    AnnouncementListener(IMessagingReader *reader = NULL)
            : announced_subscribers(0), _reader(reader)
    {}

    void process_message(TestMessage& message) {
        /*
         * The subscriber_list vector contains the list of discovered subscribers.
         *
         * - If the message.size is INITIALIZE or LENGTH_CHANGED and the
         *   subscriber is not in the list, it will be added.
         * - If the message.size is FINISHED_SIZE and the
         *   subscriber is in the list, it will be removed.
         *
         * The publisher access to this list to verify:
         * - If all the subscribers are discovered or notified about the length
         *   being changed.
         * - If all the subscribers are notified that the test has finished.
         */
        if (message.size == perftest_cpp::INITIALIZE_SIZE 
                && std::find(
                        subscriber_list.begin(),
                        subscriber_list.end(),
                        message.entity_id)
                    == subscriber_list.end()) {
            subscriber_list.push_back(message.entity_id);
        } else if (message.size == perftest_cpp::FINISHED_SIZE) {
            std::vector<int>::iterator position = std::find(
                    subscriber_list.begin(),
                    subscriber_list.end(),
                    message.entity_id);
            if (position != subscriber_list.end()) {
                subscriber_list.erase(position);
            }
        }
    }
};

/*********************************************************
 * Data listener for the Publisher side.
 *
 * Receives latency ping from Subscriber and does
 * round trip latency calculations
 */
class LatencyListener : public IMessagingCB
{
  private:

    unsigned long long latency_sum;
    unsigned long long latency_sum_square;
    unsigned long long count;
    unsigned long      latency_min;
    unsigned long      latency_max;
    int                last_data_length;
    unsigned long     *_latency_history;
    unsigned long      clock_skew_count;
    unsigned int       _num_latency;
    IMessagingWriter *_writer;
    ParameterManager *_PM;
    PerftestPrinter *_printer;
    int  subID;
    bool printIntervals;
    bool showCpu;

    void resetLatencyCounters() {
        latency_sum = 0;
        latency_sum_square = 0;
        latency_min = perftest_cpp::LATENCY_RESET_VALUE;
        latency_max = 0;
        count = 0;
    }

public:
    IMessagingReader *_reader;
    CpuMonitor cpu;

  public:

    LatencyListener(unsigned int num_latency,
            IMessagingReader *reader,
            IMessagingWriter *writer,
            ParameterManager &PM,
            PerftestPrinter *printer)
    {
        latency_sum = 0;
        latency_sum_square = 0;
        count = 0;
        latency_min = perftest_cpp::LATENCY_RESET_VALUE;
        latency_max = 0;
        last_data_length = 0;
        clock_skew_count = 0;

        if (num_latency > 0)
        {
            _num_latency = num_latency;

            /*
             * PERFTEST-109
             * _num_latency can be a big number, so this "new array" could easily
             * return a bad_alloc exception (specially in embedded platforms
             * with low memory settings). Therefore we catch the exception to
             * log this specific problem and then rethrow it.
             */
            try {
                _latency_history = new unsigned long[_num_latency];
            } catch(const std::bad_alloc&) {
                fprintf(
                        stderr,
                        "LatencyListener: Not able to allocate %ul "
                        "elements in _latency_history array",
                        _num_latency);
                throw;
            }

        } else {
            _latency_history = NULL;
            _num_latency = 0;
        }

        end_test = false;
        _reader = reader;
        _writer = writer;
        _PM = &PM;
        _printer = printer;

        subID = _PM->get<int>("sidMultiSubTest");
        printIntervals = !_PM->get<bool>("noPrintIntervals");
        showCpu = _PM->get<bool>("cpu");
    }

    void print_summary_latency(bool endTest = false){
        unsigned short mask;
        double latency_ave;
        double latency_std;
      #ifdef PERFTEST_RTI_PRO
        double serializeTime = -1;
        double deserializeTime = -1;
      #endif
        int totalSampleSize = last_data_length + perftest_cpp::OVERHEAD_BYTES;

        double outputCpu = 0.0;
        if (count == 0)
        {
            if (endTest) {
                fprintf(stderr,
                        "\nNo Pong samples have been received in the Publisher side.\n"
                        "If you are interested in latency results, you might need to\n"
                        "increase the Pong frequency (using the -latencyCount option).\n"
                        "Alternatively you can increase the number of samples sent\n"
                        "(-numIter) or the time for the test (-executionTime). If you\n"
                        "are sending large data, make sure you set the data size (-datalen)\n"
                        "in the Subscriber side.\n\n");
                fflush(stderr);
            }
            return;
        }

        if (clock_skew_count != 0) {
            fprintf(stderr,
                    "The following latency result may not be accurate because"
                    " clock skew happens %lu times\n",
                    clock_skew_count);
            fflush(stderr);
        }

        // Before Sorting the array, it is the right time to print into a file
        // if we have to.
        if (_PM->is_set("latencyFile")) {
            std::string file_name = _PM->get<std::string>("latencyFile");
            if (file_name.empty()) {
                file_name = "LatencySamples.csv";
            }
            FILE *output_file = fopen(file_name.c_str(), "a");
            if (output_file == NULL) {
                fprintf(stderr,
                        "[Error]: print_summary_latency error opening "
                        "file to save latency samples");
            } else {
                fprintf(stderr,
                        "Saving latency information in \"%s\".\n",
                        file_name.c_str());
                fprintf(output_file, "Sample Number, Value\n");
                for (unsigned int i = 0; i < count; i++) {
                    fprintf(output_file, "%u, %lu\n", i, _latency_history[i]);
                }
                fprintf(output_file, "\n");
                fclose(output_file);
            }

        }

        // sort the array (in ascending order)
        std::sort(_latency_history, _latency_history + count);
        latency_ave = (double)latency_sum / count;
        latency_std = sqrt((double)latency_sum_square / (double)count - (latency_ave * latency_ave));

        if (showCpu) {
            outputCpu = cpu.get_cpu_average();
            cpu = CpuMonitor();
            cpu.initialize();
        }

      #if defined(PERFTEST_RTI_PRO) && !defined(RTI_PERF_TSS)
        if (_PM->get<bool>("serializationTime")) {

            mask = (_PM->get<int>("unbounded") != 0) << 0;
            mask += _PM->get<bool>("keyed") << 1;
            mask += _PM->get<bool>("flatdata") << 2;
            mask += _PM->get<bool>("zerocopy") << 3;

            switch (mask)
            {
            case 0: // = 0000 (bounded)
                serializeTime = RTIDDSImpl<TestData_t>::
                        obtain_dds_serialize_time_cost(totalSampleSize);
                deserializeTime = RTIDDSImpl<TestData_t>::
                        obtain_dds_deserialize_time_cost(totalSampleSize);
                break;

            case 1: // unbounded = 0001
                serializeTime = RTIDDSImpl<TestDataLarge_t>::
                        obtain_dds_serialize_time_cost(totalSampleSize);
                deserializeTime = RTIDDSImpl<TestDataLarge_t>::
                        obtain_dds_deserialize_time_cost(totalSampleSize);
                break;

            case 2: // keyed = 0010
                serializeTime = RTIDDSImpl<TestDataKeyed_t>::
                        obtain_dds_serialize_time_cost(totalSampleSize);
                deserializeTime = RTIDDSImpl<TestDataKeyed_t>::
                        obtain_dds_deserialize_time_cost(totalSampleSize);
                break;

            case 3: // unbounded + keyed = 0011
                serializeTime = RTIDDSImpl<TestDataKeyedLarge_t>::
                        obtain_dds_serialize_time_cost(totalSampleSize);
                deserializeTime = RTIDDSImpl<TestDataKeyedLarge_t>::
                        obtain_dds_deserialize_time_cost(totalSampleSize);
                break;

        #ifdef RTI_FLATDATA_AVAILABLE
          #ifdef RTI_ZEROCOPY_AVAILABLE
            case 15: // unbounded + keyed + flat + zero = 1111
                serializeTime = RTIDDSImpl_FlatData<TestDataKeyedLarge_ZeroCopy_w_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestDataKeyedLarge_ZeroCopy_w_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;

            case 14: // keyed + flat + zero = 1110
                serializeTime = RTIDDSImpl_FlatData<TestDataKeyed_ZeroCopy_w_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestDataKeyed_ZeroCopy_w_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;

            case 13: // unbounded + flat + zero = 1101
                serializeTime = RTIDDSImpl_FlatData<TestDataLarge_ZeroCopy_w_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestDataLarge_ZeroCopy_w_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;

            case 12: // flat + Zero = 1100
                serializeTime = RTIDDSImpl_FlatData<TestData_ZeroCopy_w_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestData_ZeroCopy_w_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;
          #endif
            case 7: // unbounded + keyed + flat = 0111
                serializeTime = RTIDDSImpl_FlatData<TestDataKeyedLarge_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestDataKeyedLarge_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;

            case 6: // Keyed + flat = 0110
                serializeTime = RTIDDSImpl_FlatData<TestDataKeyed_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestDataKeyed_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;

            case 5: // unbounded + flat = 0101
                serializeTime = RTIDDSImpl_FlatData<TestDataLarge_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestDataLarge_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;

            case 4: // flat = 0100
                serializeTime = RTIDDSImpl_FlatData<TestData_FlatData_t>::
                        obtain_dds_serialize_time_cost_override(totalSampleSize);
                deserializeTime = RTIDDSImpl_FlatData<TestData_FlatData_t>::
                        obtain_dds_deserialize_time_cost_override(totalSampleSize);
                break;
        #endif

            default:
                break;
            }
        }
      #endif

      #ifndef PERFTEST_RTI_PRO
        _printer->print_latency_summary(
                latency_ave,
                latency_std,
                latency_min,
                latency_max,
                _latency_history,
                count,
                outputCpu);
      #else
        _printer->print_latency_summary(
                totalSampleSize,
                latency_ave,
                latency_std,
                latency_min,
                latency_max,
                _latency_history,
                count,
                serializeTime,
                deserializeTime,
                outputCpu);
      #endif

        latency_sum = 0;
        latency_sum_square = 0;
        latency_min = perftest_cpp::LATENCY_RESET_VALUE;
        latency_max = 0;
        count = 0;
        clock_skew_count = 0;

        return;
    }

    ~LatencyListener()
    {
        if (_latency_history != NULL) {
            delete []_latency_history;
        }
    }

    void process_message(TestMessage &message)
    {
        unsigned long long now, sentTime;
        unsigned long latency;
        int sec;
        unsigned int usec;
        double latency_ave;
        double latency_std;
        double outputCpu = 0.0;

        now = PerftestClock::getInstance().getTime();

        switch (message.size) {
            // Initializing message, don't process
            case perftest_cpp::INITIALIZE_SIZE:
                return;
            // Test finished message
            case perftest_cpp::FINISHED_SIZE:
                return;
            default:
                break;
        }

        if (last_data_length != message.size) {
            resetLatencyCounters();
        }

        sec = message.timestamp_sec;
        usec = message.timestamp_usec;
        sentTime = ((unsigned long long) sec << 32) | (unsigned long long) usec;

        if (now >= sentTime) {
            latency = (unsigned long) (now - sentTime);
            // keep track of one-way latency
            latency /= 2;

            // store value for percentile calculations
            if (_latency_history != NULL) {
                if (count >= _num_latency) {
                    fprintf(stderr,
                            "Too many latency pongs received."
                            " Do you have more than 1 app with "
                            "-pidMultiPubTest = 0 or"
                            " -sidMultiSubTest 0?\n");
                    return;
                } else {
                    _latency_history[count] = latency;
                }
            }

            if (latency_min == perftest_cpp::LATENCY_RESET_VALUE) {
                latency_min = latency;
                latency_max = latency;
            } else {
                if (latency < latency_min) {
                    latency_min = latency;
                } else if (latency > latency_max) {
                    latency_max = latency;
                }
            }

            ++count;
            latency_sum += latency;
            latency_sum_square +=
                    ((unsigned long long) latency
                     * (unsigned long long) latency);

            // if data sized changed, print out stats and zero counters
            if (last_data_length != message.size) {
                last_data_length = message.size;
                _printer->_dataLength =
                        last_data_length + perftest_cpp::OVERHEAD_BYTES;
                _printer->print_latency_header();
                resetLatencyCounters();
            } else {
                if (printIntervals) {
                    latency_ave = (double) latency_sum / (double) count;
                    latency_std =
                            sqrt((double) latency_sum_square / (double) count
                                 - (latency_ave * latency_ave));

                    if (showCpu) {
                        outputCpu = cpu.get_cpu_instant();
                    }
                    _printer->print_latency_interval(
                            latency,
                            latency_ave,
                            latency_std,
                            latency_min,
                            latency_max,
                            outputCpu);
                }
            }
        } else {
            fprintf(stderr,
                    "Clock skew suspected: received time %llu usec,"
                    " sent time %llu usec\n",
                    now,
                    sentTime);
            ++clock_skew_count;
        }

        if (_writer != NULL) {
            _writer->notify_ping_response();
        }
    }
};

void perftest_cpp::calculate_publication_rate()
{
    unsigned long long pubRate =
            (_PM.get_pair<unsigned long long, std::string>("pubRatebps").first
                / (8 * _PM.get<unsigned long long>("dataLen")));
    if (pubRate < 1) {
        pubRate = 1;
    }
    fprintf(stderr, "Calculated Publication Rate is: %lld samples/s\n", pubRate);

    _PM.set("pubRate", std::pair<unsigned long long, std::string>(
        pubRate,
        _PM.get_pair<unsigned long long, std::string>("pubRatebps").second
    ));

}

/*********************************************************
 * Publisher
 */
int perftest_cpp::Publisher()
{
    LatencyListener *reader_listener = NULL;
    AnnouncementListener  *announcement_reader_listener = NULL;
    IMessagingReader *announcement_reader;
    IMessagingReader *reader;
    struct PerftestThread *latencyReadThread = NULL;
    struct PerftestThread *executionTimeoutThread = NULL;
    unsigned long num_latency;
    unsigned long announcementSampleCount = 50;
    unsigned int samplesPerBatch = get_samples_per_batch();

    // create throughput/ping writer
    IMessagingWriter *writer = _MessagingImpl->create_writer(
            THROUGHPUT_TOPIC_NAME);

    if (writer == NULL)
    {
        fprintf(stderr, "Problem creating throughput writer.\n");
        return -1;
    }

    // Calculate number of latency pings that will be sent per data size
    num_latency = (unsigned long)((_PM.get<unsigned long long>("numIter") /
            samplesPerBatch) /
            _PM.get<unsigned long long>("latencyCount"));
    if ((_PM.get<unsigned long long>("numIter") /
            samplesPerBatch) %
            _PM.get<unsigned long long>("latencyCount") > 0) {
        num_latency++;
    }

    if (samplesPerBatch > 1) {
        // In batch mode, might have to send another ping
        ++num_latency;
    }

    // Only publisher with ID 0 will send/receive pings
    if (_PM.get<int>("pidMultiPubTest") == 0) {
        // Check if using callbacks or read thread
        if (!_PM.get<bool>("useReadThread")) {
            // create latency pong reader
            // the writer is passed for ping-pong notification in LatencyTest
            reader_listener = new LatencyListener(
                    num_latency,
                    NULL,
                    _PM.get<bool>("latencyTest") ? writer : NULL,
                    _PM,
                    _printer);
            reader = _MessagingImpl->create_reader(
                    LATENCY_TOPIC_NAME,
                    reader_listener);
            if (reader == NULL)
            {
                fprintf(stderr, "Problem creating latency reader.\n");
                return -1;
            }
        }
        else
        {
            reader = _MessagingImpl->create_reader(
                    LATENCY_TOPIC_NAME,
                    NULL);
            if (reader == NULL)
            {
                fprintf(stderr,"Problem creating latency reader.\n");
                return -1;
            }
            reader_listener = new LatencyListener(
                    num_latency,
                    reader,
                    _PM.get<bool>("latencyTest") ? writer : NULL,
                    _PM,
                    _printer);

            int threadPriority = Perftest_THREAD_PRIORITY_DEFAULT;
            int threadOptions = Perftest_THREAD_OPTION_DEFAULT;

            if (_threadPriorities.isSet) {
                threadPriority = _threadPriorities.receive;
                threadOptions = Perftest_THREAD_SETTINGS_REALTIME_PRIORITY
                        | Perftest_THREAD_SETTINGS_PRIORITY_ENFORCE;
            }

            latencyReadThread = PerftestThread_new(
                    "ReceiverThread",
                    threadPriority,
                    threadOptions,
                    ReadThread<LatencyListener>,
                    reader_listener);
            if (latencyReadThread == NULL) {
                fprintf(stderr, "Problem creating ReceiverThread for LatencyReadThread.\n");
                return -1;
            }

        }
    }
    else
    {
        reader = NULL;
    }

    /* Create Announcement reader
     * A Subscriber will send a message on this channel once it discovers
     * every Publisher
     */
    if (_MessagingImpl->supports_listeners()) {
        announcement_reader_listener = new AnnouncementListener();
    }

    announcement_reader = _MessagingImpl->create_reader(
            ANNOUNCEMENT_TOPIC_NAME,
            announcement_reader_listener);
    if (announcement_reader == NULL)
    {
        fprintf(stderr, "Problem creating announcement reader.\n");
        return -1;
    }

    struct PerftestThread *announcementReadThread = NULL;
    if (!_MessagingImpl->supports_listeners()) {
        announcement_reader_listener
                = new AnnouncementListener(announcement_reader);

        int threadPriority = Perftest_THREAD_PRIORITY_DEFAULT;
        int threadOptions = Perftest_THREAD_OPTION_DEFAULT;

        if (_threadPriorities.isSet) {
            threadPriority = _threadPriorities.receive;
            threadOptions = Perftest_THREAD_SETTINGS_REALTIME_PRIORITY
                    | Perftest_THREAD_SETTINGS_PRIORITY_ENFORCE;
        }

        announcementReadThread = PerftestThread_new(
                "announcementReadThread",
                threadPriority,
                threadOptions,
                ReadThread<AnnouncementListener>,
                announcement_reader_listener);
        if (latencyReadThread == NULL) {
            fprintf(stderr, "Problem creating Thread for announcementReadThread.\n");
            return -1;
        }
    }

    unsigned long long spinPerUsec = 0;
    unsigned long sleepUsec = 1000;
    DDS_Duration_t sleep_period = {0,0};

    if (_PM.is_set("pubRatebps")) {
        calculate_publication_rate();
    }

    if (_PM.is_set("pubRate")) {
        if (_PM.get_pair<unsigned long long, std::string>("pubRate").second
                == "spin") {
            spinPerUsec = NDDSUtility::get_spin_per_microsecond();
            /* A return value of 0 means accuracy not assured */
            if (spinPerUsec == 0) {
                fprintf(stderr,
                        "Error initializing spin per microsecond. '-pubRate'"
                        "cannot be used\nExiting...\n");
                return -1;
            }
            _SpinLoopCount = 1000000 * spinPerUsec /
                    _PM.get_pair<unsigned long long, std::string>("pubRate").first;
        } else { // sleep count
            _SleepNanosec = 1000000000 /
                    (unsigned long)_PM.get_pair
                            <unsigned long long, std::string>("pubRate").first;
        }
    }

    fprintf(stderr,
            "Waiting to discover %d subscribers ...\n",
            _PM.get<int>("numSubscribers"));
    fflush(stderr);
    writer->wait_for_readers(_PM.get<int>("numSubscribers"));
    // Only publisher with ID 0 will have a reader.
    if (reader != NULL) {
        reader->wait_for_writers(_PM.get<int>("numSubscribers"));
    }
    announcement_reader->wait_for_writers(_PM.get<int>("numSubscribers"));
    // We have to wait until every Subscriber sends an announcement message
    // indicating that it has discovered every Publisher
    fprintf(stderr,"Waiting for subscribers announcement ...\n");
    fflush(stderr);
    while (_PM.get<int>("numSubscribers")
            > (int)announcement_reader_listener->subscriber_list.size()) {
        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
    }

    // Allocate data and set size
    TestMessage message;
    message.entity_id = _PM.get<int>("pidMultiPubTest");

    if (_PM.is_set("loadDataFromFile")) {
        message.data = _fileDataLoader.get_next_buffer();
    } else {
        message.data = new char[(std::max)(
                (int) _PM.get<unsigned long long>("dataLen"),
                (int) FINISHED_SIZE)];
        memset(message.data,
               0,
               (std::max)(
                       (int) _PM.get<unsigned long long>("dataLen"),
                       (int) FINISHED_SIZE));
    }

    if (showCpu && _PM.get<int>("pidMultiPubTest") == 0) {
        reader_listener->cpu.initialize();
    }

    if (INITIALIZE_SIZE > MAX_PERFTEST_SAMPLE_SIZE) {
        fprintf(stderr,"Error: INITIALIZE_SIZE > MAX_PERFTEST_SAMPLE_SIZE\n");
        return -1;
    }

    message.size = INITIALIZE_SIZE;

    /*
     * Initial burst of data:
     *
     * The purpose of this initial burst of Data is to ensure that most
     * memory allocations in the critical path are done before the test begings,
     * for both the Writer and the Reader that receives the samples.
     * It will also serve to make sure that all the instances are registered
     * in advance in the subscriber application.
     *
     * We query the MessagingImplementation class to get the suggested sample
     * count that we should send. This number might be based on the reliability
     * protocol implemented by the middleware behind. Then we choose between that
     * number and the number of instances to be sent.
     */
    unsigned long initializeSampleCount = (std::max)(
            _MessagingImpl->get_initial_burst_size(),
            (unsigned long)_PM.get<long>("instances"));

    if (_PM.is_set("initialBurstSize")) {
        initializeSampleCount = _PM.get<long>("initialBurstSize");
    }

    fprintf(stderr,
            "Sending %lu initialization pings ...\n",
            initializeSampleCount);
    fflush(stderr);

    for (unsigned long i = 0; i < initializeSampleCount; i++) {
        // Send test initialization message
        writer->send(message, true);
    }
    writer->flush();

    fprintf(stderr, "Sending data ...\n");
    fflush(stderr);

    _printer->print_initial_output();

    // Set data size, account for other bytes in message
    message.size = (int)_PM.get<unsigned long long>("dataLen") - OVERHEAD_BYTES;

    // Sleep 1 second, then begin test
    PerftestClock::milliSleep(1000);

    int num_pings = 0;
    int pingID = -1;
    int current_index_in_batch = 0;
    int ping_index_in_batch = 0;
    bool sentPing = false;

    unsigned long long time_now = 0, time_last_check = 0, time_delta = 0;
    unsigned long long startTestTime = 0;
    unsigned long pubRate_sample_period = 1;
    unsigned long rate = 0;

    struct PerftestTimer::ScheduleInfo schedInfo = {
            (unsigned int)_PM.get<unsigned long long>("executionTime"),
            Timeout
    };

    time_last_check = PerftestClock::getInstance().getTime();

    /* Minimum value for pubRate_sample_period will be 1 so we execute 100 times
       the control loop every second, or every sample if we want to send less
       than 100 samples per second */
    if (_PM.get_pair<unsigned long long, std::string>("pubRate").first > 100) {
        pubRate_sample_period =
                (unsigned long)_PM.get_pair <unsigned long long, std::string>(
                        "pubRate").first
                / 100;
    }

    if (_PM.get<unsigned long long>("executionTime") > 0) {
        executionTimeoutThread = PerftestTimer::getInstance().setTimeout(schedInfo);
        if (executionTimeoutThread == NULL) {
            fprintf(stderr, "Problem creating timeoutThread for executionTime.\n");
            return -1;
        }
    }

    /*
     * Copy variable to no query the ParameterManager in every iteration.
     * They should not be modified:
     * - NumIter
     * - latencyCount
     * - numSubscribers
     * - bestEffort
     * - latencyTest
     * - pidMultiPubTest
     * - pubRateMethodSpin
     * - pubRate
     * - cacheStats
     * - isSetPubRate
     */
    const unsigned long long numIter = _PM.get<unsigned long long>("numIter");
    const unsigned long long latencyCount =
            _PM.get<unsigned long long>("latencyCount");
    const int numSubscribers = _PM.get<int>("numSubscribers");
    const bool bestEffort = _PM.get<bool>("bestEffort");
    const bool latencyTest = _PM.get<bool>("latencyTest");
    const int pidMultiPubTest = _PM.get<int>("pidMultiPubTest");
    const bool pubRateMethodSpin =
            _PM.get_pair<unsigned long long, std::string>("pubRate").second == "spin";
    const unsigned long pubRate =
            (unsigned long)_PM.get_pair<unsigned long long, std::string>("pubRate").first;
    const bool cacheStats = _PM.get<bool>("cacheStats");
    const bool isSetPubRate = _PM.is_set("pubRate");
    const bool useDatafromFile = _PM.is_set("loadDataFromFile");


    /*
     * If the machine where we are executing the publisher has a low resolution
     * clock, this logic might not report accurate latency numbers. Therefore
     * we are implementing a simple solution to get a rough estimation of the
     * latency:
     * Before sending the first sample we are taking the time and right after
     * receiving the last pong we are taking the time again. We will assume
     * that the average latency is equivalent to half of that time divided by
     * the number of iterations we did (which assumes that every sample is
     * answered -- that means that latencyCount = 1 -- Latency Test).
     */

    if (_PM.get<bool>("lowResolutionClock")) {
        startTestTime = PerftestClock::getInstance().getTime();
    }

    /********************
     *  Main sending loop
     */
    unsigned long long loop = 0;
    for (loop = 0; (loop < numIter) && (!_testCompleted); ++loop) {

        /* This if has been included to perform the control loop
           that modifies the publication rate according to -pubRate */
        if (isSetPubRate && (loop > 0) && (loop % pubRate_sample_period == 0)) {

            time_now = PerftestClock::getInstance().getTime();

            time_delta = time_now - time_last_check;
            time_last_check = time_now;
            if (time_delta > 0) {
                rate = (pubRate_sample_period * 1000000) / (unsigned long) time_delta;
            } else {
                rate = pubRate_sample_period * 1000000;
            }
            if (pubRateMethodSpin) {
                if (rate > pubRate) {
                    _SpinLoopCount += spinPerUsec;
                } else if (rate < pubRate && _SpinLoopCount > spinPerUsec) {
                    _SpinLoopCount -= spinPerUsec;
                } else if (rate < pubRate && _SpinLoopCount <= spinPerUsec) {
                    _SpinLoopCount = 0;
                }
            } else { // sleep
                if (rate > pubRate) {
                    _SleepNanosec += sleepUsec; // plus 1 MicroSec
                } else if (rate < pubRate && _SleepNanosec > sleepUsec) {
                    _SleepNanosec -=  sleepUsec; // less 1 MicroSec
                } else if (rate < pubRate && _SleepNanosec <= sleepUsec) {
                    _SleepNanosec = 0;
                }
            }
        }

        if (_SpinLoopCount > 0) {
            NDDSUtility::spin(_SpinLoopCount);
        }

        if (_SleepNanosec > 0) {
            sleep_period.sec = (long) (_SleepNanosec / 1000000000u);
            sleep_period.nanosec = (unsigned long) _SleepNanosec
                                    - (unsigned long) (sleep_period.sec * 1000000000);
            PerftestClock::sleep(sleep_period);
        }

        pingID = -1;

        // only send latency pings if is publisher with ID 0
        // In batch mode, latency pings are sent once every LatencyCount batches
        if ((pidMultiPubTest == 0) && (((loop / samplesPerBatch)
                % latencyCount) == 0)) {
            /* In batch mode only send a single ping in a batch.
             *
             * However, the ping is sent in a round robin position within
             * the batch.  So keep track of which position(index) the
             * current sample is within the batch, and which position
             * within the batch should contain the ping. Only send the ping
             * when both are equal.
             *
             * Note when not in batch mode,
             * current_index_in_batch = ping_index_in_batch
             * always. And the if() is always true.
             */
            if (current_index_in_batch == ping_index_in_batch && !sentPing) {

                // Each time ask a different subscriber to echo back
                pingID = num_pings % numSubscribers;
                unsigned long long now = PerftestClock::getInstance().getTime();
                message.timestamp_sec = (int)((now >> 32) & 0xFFFFFFFF);
                message.timestamp_usec = (unsigned int)(now & 0xFFFFFFFF);
                ++num_pings;
                ping_index_in_batch = (ping_index_in_batch + 1) % samplesPerBatch;
                sentPing = true;

                if (cacheStats && printIntervals) {
                    printf("Pulled samples: %3d, Samples Writer Queue: %3d (Peak: %3d)\n",
                            writer->get_pulled_sample_count(),
                            writer->get_sample_count(),
                            writer->get_sample_count_peak());
                }
            }
        }
        current_index_in_batch = (current_index_in_batch + 1) % samplesPerBatch;

        message.seq_num = (unsigned long) loop;
        message.latency_ping = pingID;
        if (useDatafromFile) {
            message.data = _fileDataLoader.get_next_buffer();
        }
        writer->send(message);
        if(latencyTest && sentPing) {
            if (!bestEffort) {
              #ifdef DEBUG_PING_PONG
                printf("-> Waiting for Ping Response\n");
              #endif
                writer->wait_for_ping_response();
              #ifdef DEBUG_PING_PONG
                printf("<- Got Ping Response\n");
              #endif
            } else {
                /* time out in milliseconds */
                writer->wait_for_ping_response(200);
            }
        }


        // come to the beginning of another batch
        if (current_index_in_batch == 0) {
            sentPing = false;
        }
    }

    // In case of batching, flush
    writer->flush();

    /*
     * This is where we report the time when using the low resolution clock
     * feature, as mentioned above, this time is a rough estimation.
     */
    if (_PM.get<bool>("lowResolutionClock")) {
        fprintf(stdout,
                "Average Latency time = %llu (us)\n",
                (PerftestClock::getInstance().getTime()
                    - startTestTime)
                        / (2 * loop));
    }
    // Test has finished, send end of test message, send multiple
    // times in case of best effort
    if (FINISHED_SIZE > MAX_PERFTEST_SAMPLE_SIZE) {
        fprintf(stderr,"Error: FINISHED_SIZE < MAX_PERFTEST_SAMPLE_SIZE\n");
        return -1;
    }

    message.size = FINISHED_SIZE;
    unsigned long i = 0;
    while (announcement_reader_listener->subscriber_list.size() > 0
            && i < announcementSampleCount) {
        writer->send(message, true);
        writer->flush();
        writer->wait_for_ack(
                timeout_wait_for_ack_sec,
                timeout_wait_for_ack_nsec);
        i++;
    }

    if (_PM.get<int>("pidMultiPubTest") == 0) {
        reader_listener->print_summary_latency(true);
        reader_listener->end_test = true;
    } else {
        fprintf(
            stderr,
            "Latency results are only shown when -pidMultiPubTest = 0\n");
    }

    if (_PM.get<bool>("cacheStats")) {
        printf("Pulled samples: %3d, Samples Writer Queue Peak: %3d\n",
                writer->get_pulled_sample_count(),
                writer->get_sample_count_peak());
    }

    if (!finalize_read_thread(latencyReadThread, reader_listener)) {
        fprintf(stderr, "Error deleting latencyReadThread\n");
        return -1;
    }

    if (!finalize_read_thread(announcementReadThread, announcement_reader_listener)) {
        fprintf(stderr, "Error deleting announcementReadThread\n");
        return -1;
    }

    if (reader != NULL) {
        delete reader;
    }

    if (announcement_reader != NULL) {
        delete announcement_reader;
    }

    if (writer != NULL) {
        delete writer;
    }

    if (reader_listener != NULL) {
        delete reader_listener;
    }

    if (announcement_reader_listener != NULL) {
        delete announcement_reader_listener;
    }

    /* The FileDataLoader class will remove this data, if in use */
    if (!useDatafromFile) {
        delete []message.data;
    }
    // For Json format, print last brackets
    _printer->print_final_output();
    if (_testCompleted) {
        // Delete timeout thread
#if !defined(PERFTEST_CERT)
        if (executionTimeoutThread != NULL) {
            PerftestThread_delete(executionTimeoutThread);
        }
#endif

        fprintf(stderr,"Finishing test due to timer...\n");
    } else {
        fprintf(stderr,"Finishing test...\n");
    }
    fflush(stderr);

    return 0;
}

template <class ListenerType>
bool perftest_cpp::finalize_read_thread(
        PerftestThread *thread,
        ListenerType *listener)
{
    if (listener != NULL) {
        listener->end_test = true;
    }

    if (thread != NULL) {
        if (listener != NULL
                && listener->_reader->unblock()
                && listener->sync_semaphore != NULL) {
            /*
             * If the thread is created but the creation of the semaphore fail,
             * sync_semaphore could be null
             */
            if (!PerftestSemaphore_take(
                    listener->sync_semaphore,
                    PERFTEST_SEMAPHORE_TIMEOUT_INFINITE)) {
                fprintf(stderr,"Unexpected error taking semaphore\n");
                return false;
            }
        }
#if !defined(PERFTEST_CERT)
        PerftestThread_delete(thread);
#endif
    }
    return true;
}

inline unsigned int perftest_cpp::get_samples_per_batch() {
    if (_PM.get<long>("batchSize")
            > (long)_PM.get<unsigned long long>("dataLen")) {
        return _PM.get<long>("batchSize") /
                (unsigned int)_PM.get<unsigned long long>("dataLen");
    } else {
        return 1;
    }
}

const ThreadPriorities perftest_cpp::get_thread_priorities()
{
    return _threadPriorities;
}

void perftest_cpp::Timeout() {
    _testCompleted = true;
}
