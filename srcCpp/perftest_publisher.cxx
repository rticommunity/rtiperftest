/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "RTIDDSImpl.h"
#include "perftest_cpp.h"
#include "CpuMonitor.h"

#if defined(RTI_WIN32)
  #pragma warning(push)
  #pragma warning(disable : 4996)
  #define STRNCASECMP _strnicmp
#elif defined(RTI_VXWORKS)
  #define STRNCASECMP strncmp
#else
  #define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

int  perftest_cpp::_SubID = 0;
bool perftest_cpp::printIntervals = true;
bool perftest_cpp::showCpu = false;

/* Clock related variables */
struct RTIClock* perftest_cpp::_Clock = RTIHighResolutionClock_new();
struct RTINtpTime perftest_cpp::_ClockTime_aux = RTI_NTP_TIME_ZERO;
RTI_UINT64 perftest_cpp::_Clock_sec = 0;
RTI_UINT64 perftest_cpp::_Clock_usec = 0;

bool perftest_cpp::_testCompleted = false;
bool perftest_cpp::_testCompleted_scan = true; // In order to enter into the scan test
const int timeout_wait_for_ack_sec = 0;
const unsigned int timeout_wait_for_ack_nsec = 100000000;
const Perftest_ProductVersion_t perftest_cpp::_version = {2, 3, 2, 0};

/*
 * PERFTEST-108
 * If we are performing a latency test, the default number for _NumIter will be
 * 10 times smaller than the default when performing a throughput test. This
 * will allow Perftest to work better in embedded platforms since the _NumIter
 * parameter sets the size of certain arrays in the latency test mode.
 */
const unsigned long long numIterDefaultLatencyTest = 10000000;

#ifdef RTI_WIN32
LARGE_INTEGER perftest_cpp::_ClockFrequency = {0, 0};
/* This parameter is not thread safe */
HANDLE perftest_cpp::_hTimerQueue = NULL;
HANDLE perftest_cpp::_hTimer = NULL;
#endif

/*********************************************************
 * Main
 */
int main(int argc, char *argv[])
{
    try {
        perftest_cpp app;
        return app.Run(argc, argv);
    } catch (const std::exception &ex) {
        // This will catch DDS exceptions
        fprintf(stderr, "Exception in perftest_cpp::Run(): %s.\n", ex.what());
        return -1;
    }
}

#if defined(RTI_VXWORKS)
int publisher_main()
{
    const char *argv[] = {"perftest_cpp", "-pub"};
    int argc = sizeof(argv)/sizeof(const char*);

    return main(argc, (char **) argv);
}

int subscriber_main()
{
    const char *argv[] = {"perftest_cpp", "-sub"};
    int argc = sizeof(argv)/sizeof(const char*);

    return main(argc, (char **) argv);
}
#endif

int perftest_cpp::Run(int argc, char *argv[])
{
    try {
        PM::GetInstance().initialize();
    } catch(std::exception &ex) {
        // call destructor
        // p.shutdown();
        fprintf(stderr, "Exception in PM::GetInstance().initialize(): %s.\n", ex.what());
        return -1;
    }
    if (PM::GetInstance().check_help(argc, argv)) {
        return 0;
    }
    if (!PM::GetInstance().parse(argc, argv)) {
        return -1;
    }

    if (!PM::GetInstance().validate_group()) {
        return -1;
    }

//     printf("batching: %d\n", PM::GetInstance().get<int>("batching"));
//     std::vector<unsigned long long> scan = PM::GetInstance().get_vector<unsigned long long>("scan");
//     printf("scan: \n");
//     for (unsigned int i = 0; i < scan.size(); i++) {
//         printf("\t%llu\n", scan[i]);
//     }
//     std::vector<unsigned long long> cft = PM::GetInstance().get_vector<unsigned long long>("cft");
//     printf("cft: \n");
//     for (unsigned int i = 0; i < cft.size(); i++) {
//         printf("\t%llu\n", cft[i]);
//     }
//     printf("sidMultiSubTest: %d\n", PM::GetInstance().get<int>("sidMultiSubTest"));
//     printf("dataLen: %d\n", PM::GetInstance().get<int>("dataLen"));
//     printf("instances: %d\n", PM::GetInstance().get<int>("instances"));
//     printf("numPublishers: %d\n", PM::GetInstance().get<int>("numPublishers"));

//     //TRANSPORT
//     printf("nic: %s\n", PM::GetInstance().get<std::string>("nic").c_str());
//     printf("transport: %s\n", PM::GetInstance().get<std::string>("transport").c_str());
//     printf("multicast: %d\n", PM::GetInstance().get<bool>("multicast"));
//     printf("multicastAddr: %s\n", PM::GetInstance().get<std::string>("multicastAddr").c_str());
//     printf("transportVerbosity: %s\n", PM::GetInstance().get<std::string>("transportVerbosity").c_str());
//     printf("transportServerBindPort: %s\n", PM::GetInstance().get<std::string>("transportServerBindPort").c_str());
//     printf("transportWan: %s\n", PM::GetInstance().get<std::string>("transportWan").c_str());
//     printf("transportWanServerPort: %s\n", PM::GetInstance().get<std::string>("transportWanServerPort").c_str());
//     printf("transportWanId: %s\n", PM::GetInstance().get<std::string>("transportWanId").c_str());
//     printf("transportSecureWan: %d\n", PM::GetInstance().get<bool>("transportSecureWan"));
//     printf("transportPublicAddress: %s\n", PM::GetInstance().get<std::string>("transportPublicAddress").c_str());
//     printf("transportWanServerAddress: %s\n", PM::GetInstance().get<std::string>("transportWanServerAddress").c_str());
//     printf("transportCertAuthority: %s\n", PM::GetInstance().get<std::string>("transportCertAuthority").c_str());
//     printf("transportCertFile: %s\n", PM::GetInstance().get<std::string>("transportCertFile").c_str());
//     printf("transportPrivateKey: %s\n", PM::GetInstance().get<std::string>("transportPrivateKey").c_str());

//   #ifdef RTI_SECURE_PERFTEST
//     printf("secureEncryptDiscovery: %d\n", PM::GetInstance().get<bool>("secureEncryptDiscovery"));
//     printf("secureSign: %d\n", PM::GetInstance().get<bool>("secureSign"));
//     printf("secureEncryptData: %d\n", PM::GetInstance().get<bool>("secureEncryptData"));
//     printf("secureGovernanceFile: %s\n", PM::GetInstance().get<std::string>("secureGovernanceFile").c_str());
//     printf("securePermissionsFile: %s\n", PM::GetInstance().get<std::string>("securePermissionsFile").c_str());
//     printf("secureCertAuthority: %s\n", PM::GetInstance().get<std::string>("secureCertAuthority").c_str());
//     printf("secureCertFile: %s\n", PM::GetInstance().get<std::string>("secureCertFile").c_str());
//     printf("securePrivateKey: %s\n", PM::GetInstance().get<std::string>("securePrivateKey").c_str());
//   #endif
//     return 0;

    PrintVersion();

    if (!ParseConfig(argc, argv)) {
        return -1;
    }

    if (PM::GetInstance().get<int>("unbounded") == 0) {
        if (PM::GetInstance().get<bool>("keyed")) {
            _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>();
        } else {
            _MessagingImpl = new RTIDDSImpl<TestData_t>();
        }
    } else {
        if (PM::GetInstance().get<bool>("keyed")) {
            _MessagingImpl = new RTIDDSImpl<TestDataKeyedLarge_t>();
        } else {
            _MessagingImpl = new RTIDDSImpl<TestDataLarge_t>();
        }
    }

    if (!_MessagingImpl->Initialize(_MessagingArgc, _MessagingArgv))
    {
        return -1;
    }

    PrintConfiguration();

    if (PM::GetInstance().get<bool>("pub")) {
        return Publisher();
    } else {
        return Subscriber();
    }
}

const DDS_ProductVersion_t perftest_cpp::GetDDSVersion()
{
    return NDDSConfigVersion::get_instance().get_product_version();
}

const Perftest_ProductVersion_t perftest_cpp::GetPerftestVersion()
{
    return _version;
}

void perftest_cpp::PrintVersion()
{
    Perftest_ProductVersion_t perftestV = perftest_cpp::GetPerftestVersion();
    DDS_ProductVersion_t ddsV = perftest_cpp::GetDDSVersion();

    printf("RTI Perftest %d.%d.%d",
            perftestV.major,
            perftestV.minor,
            perftestV.release);
    if (perftestV.revision != 0) {
        printf(".%d", perftestV.revision);
    }
    printf(" (RTI Connext DDS %d.%d.%d)\n",
            ddsV.major,
            ddsV.minor,
            ddsV.release);

}

// Set the default values into the array _scanDataLenSizes vector
void set_default_scan_values(
        std::vector<unsigned long> & _scanDataLenSizes)
{
    _scanDataLenSizes.clear();
    _scanDataLenSizes.push_back(32);
    _scanDataLenSizes.push_back(64);
    _scanDataLenSizes.push_back(128);
    _scanDataLenSizes.push_back(256);
    _scanDataLenSizes.push_back(512);
    _scanDataLenSizes.push_back(1024);
    _scanDataLenSizes.push_back(2048);
    _scanDataLenSizes.push_back(4096);
    _scanDataLenSizes.push_back(8192);
    _scanDataLenSizes.push_back(16384);
    _scanDataLenSizes.push_back(32768);
    _scanDataLenSizes.push_back(63000);
}

/*********************************************************
 * Destructor
 */
perftest_cpp::~perftest_cpp()
{

    for (int i = 0; i< _MessagingArgc; ++i) {
        if (_MessagingArgv[i] != NULL) {
            DDS_String_free(_MessagingArgv[i]);
        }
    }

    if (_MessagingArgv != NULL) {
        delete []_MessagingArgv;
    }

    if(_MessagingImpl != NULL){
        delete _MessagingImpl;
    }

    if (perftest_cpp::_Clock != NULL) {
        RTIHighResolutionClock_delete(perftest_cpp::_Clock);
    }

  #ifdef RTI_WIN32
    if (_hTimerQueue != NULL) {
        DeleteTimerQueue(_hTimerQueue);
    }
  #endif

    fprintf(stderr,"Test ended.\n");
    fflush(stderr);
}

/*********************************************************
 * Constructor
 */
perftest_cpp::perftest_cpp()
{
    _DataLen = 100;
    _isScan = false;
    _SpinLoopCount = 0;
    _SleepNanosec = 0;
    _NumPublishers = 1;
    _InstanceCount = 1;
    _MessagingImpl = NULL;
    _MessagingArgv = NULL;
    _MessagingArgc = 0;
    _useCft = false;

#ifdef RTI_WIN32
    if (_hTimerQueue == NULL) {
        _hTimerQueue = CreateTimerQueue();
    }
    QueryPerformanceFrequency(&_ClockFrequency);
#endif
};


/*********************************************************
 * ParseArgs
 */
bool perftest_cpp::ParseConfig(int argc, char *argv[])
{
    _MessagingArgc = 0;
    _MessagingArgv = new char*[argc];

    if (_MessagingArgv == NULL) {
        fprintf(stderr, "Problem allocating memory\n");
        return false;
    }

    for (int i=0; i<argc; i++) {
        _MessagingArgv[i] = NULL;
    }

    const char *usage_string =
        /**************************************************************************/
        "Usage:\n"
        "       perftest_cpp [options]\n"
        "\nWhere [options] are (case insensitive, partial match OK):\n\n"
        "\t-help                   - Print this usage message and exit\n"
        "\t-pub                    - Set test to be a publisher\n"
        "\t-sub                    - Set test to be a subscriber (default)\n"
        "\t-sidMultiSubTest <id>   - Set the id of the subscriber in a\n"
        "\t                          multi-subscriber test, default 0\n"
        "\t-pidMultiPubTest <id>   - Set id of the publisher in a multi-publisher \n"
        "\t                          test, default 0. Only publisher 0 sends \n"
        "\t                          latency pings\n"
        "\t-dataLen <bytes>        - Set length of payload for each send\n"
        "\t                          default 100.\n"
        "\t-unbounded <allocation_threshold> - Use unbounded Sequences\n"
        "\t                                   <allocation_threshold> is optional, default 2*dataLen up to 63000 Bytes.\n"
        "\t-numIter <count>        - Set number of messages to send, default is\n"
        "\t                          100000000 for Throughput tests or 10000000\n"
        "\t                          for Latency tests. See -executionTime.\n"
        "\t-instances <count>      - Set the number of instances (keys) to iterate\n"
        "\t                          over when publishing, default 1\n"
        "\t-writeInstance <instance> - Set the instance number to be sent. \n"
        "\t                          -WriteInstance parameter cannot be bigger than the number of instances.\n"
        "\t                          Default 'Round-Robin schedule'\n"
        "\t-sleep <millisec>       - Time to sleep between each send, default 0\n"
        "\t-latencyCount <count>   - Number of samples (or batches) to send before\n"
        "\t                          a latency ping packet is sent, default\n"
        "\t                          10000 if -latencyTest is not specified,\n"
        "\t                          1 if -latencyTest is specified\n"
        "\t-numSubscribers <count> - Number of subscribers running in test, \n"
        "\t                          default 1\n"
        "\t-numPublishers <count>  - Number of publishers running in test, \n"
        "\t                          default 1\n"
        "\t-scan <size1>:<size2>:...:<sizeN> - Run test in scan mode, traversing\n"
        "\t                                    a range of sample data sizes from\n"
        "\t                                    [32,63000] or [63001,2147483128] bytes,\n"
        "\t                                    in the case that you are using large data or not.\n"
        "\t                                    The list of sizes is optional.\n"
        "\t                                    Default values are '32:64:128:256:512:1024:2048:4096:8192:16384:32768:63000'\n"
        "\t                                    Default: Not set\n"
        "\t-noPrintIntervals       - Don't print statistics at intervals during \n"
        "\t                          test\n"
        "\t-useReadThread          - Use separate thread instead of callback to \n"
        "\t                          read data\n"
        "\t-latencyTest            - Run a latency test consisting of a ping-pong \n"
        "\t                          synchronous communication\n"
        "\t-verbosity <level>      - Run with different levels of verbosity:\n"
        "\t                          0 - SILENT, 1 - ERROR, 2 - WARNING,\n"
        "\t                          3 - ALL. Default: 1\n"
        "\t-pubRate <samples/s>:<method>    - Limit the throughput to the specified number\n"
        "\t                                   of samples/s, default 0 (don't limit)\n"
        "\t                                   [OPTIONAL] Method to control the throughput can be:\n"
        "\t                                   'spin' or 'sleep'\n"
        "\t                                   Default method: spin\n"
        "\t-keyed                  - Use keyed data (default: unkeyed)\n"
        "\t-executionTime <sec>    - Set a maximum duration for the test. The\n"
        "\t                          first condition triggered will finish the\n"
        "\t                          test: number of samples or execution time.\n"
        "\t                          Default 0 (don't set execution time)\n"
        "\t-writerStats            - Display the Pulled Sample count stats for\n"
        "\t                          reliable protocol debugging purposes.\n"
        "\t                          Default: Not set\n"
        "\t-cpu                   -  Display the cpu percent use by the process\n"
        "\t                          Default: Not set\n"
        "\t-cft <start>:<end>      - Use a Content Filtered Topic for the Throughput topic in the subscriber side.\n"
        "\t                          Specify 2 parameters: <start> and <end> to receive samples with a key in that range.\n"
        "\t                          Specify only 1 parameter to receive samples with that exact key.\n"
        "\t                          Default: Not set\n";

    if (argc < 1)
    {
        fprintf(stderr, "%s", usage_string);
        fflush(stderr);
        RTIDDSImpl<TestData_t>().PrintCmdLineHelp();
        return false;
    }

    int i;
    for (i = 1; i < argc; ++i)
    {
        if (IS_OPTION(argv[i], "-help")) {
            fprintf(stderr, "%s", usage_string);
            fflush(stderr);
            RTIDDSImpl<TestData_t>().PrintCmdLineHelp();
            return false;
        }
    }

    // Load command line parameters.
    for (i = 0; i < argc; ++i)
    {
        if (IS_OPTION(argv[i], "-pub"))
        {
        }
        else if (IS_OPTION(argv[i], "-sub"))
        {
        }
        else if (IS_OPTION(argv[i], "-sidMultiSubTest"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-pidMultiPubTest"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-numIter"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-dataLen"))
        {
            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <length> after -dataLen\n");
                return false;
            }

            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < (unsigned long)OVERHEAD_BYTES)
            {
                fprintf(stderr, "-dataLen must be >= %d\n", OVERHEAD_BYTES);
                return false;
            }
            if (_DataLen > (unsigned long)MAX_PERFTEST_SAMPLE_SIZE)
            {
                fprintf(stderr,"-dataLen must be <= %d\n", MAX_PERFTEST_SAMPLE_SIZE);
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-unbounded")) {
            if ((i == (argc-1)) || *argv[i+1] == '-')
            {
            } else {
                ++i;
            }
        }
        else if (IS_OPTION(argv[i], "-spin"))
        {
            fprintf(stderr,"-spin option is deprecated. It will be removed "
                    "in upcoming releases.\n");
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr,"Missing <count> after -spin\n");
                return false;
            }
            _SpinLoopCount = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-sleep"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-latencyCount"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-numSubscribers"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-numPublishers"))
        {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -numPublishers\n");
                return false;
            }
            _NumPublishers = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-scan"))
        {
            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);
            _MessagingArgc++;
            _isScan = true;
            if ((i != (argc-1)) && *argv[1+i] != '-') {
                _scanDataLenSizes.clear();
                ++i;
                unsigned long aux_scan;
                char * pch;
                _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

                if (_MessagingArgv[_MessagingArgc] == NULL) {
                    fprintf(stderr, "Problem allocating memory\n");
                    return false;
                }
                _MessagingArgc++;

                pch = strtok (argv[i], ":");
                while (pch != NULL) {
                    if (sscanf(pch, "%lu", &aux_scan) != 1) {
                        fprintf(stderr, "-scan <size> value must have the format '-scan <size1>:<size2>:...:<sizeN>'\n");
                        return false;
                    }
                    _scanDataLenSizes.push_back(aux_scan);
                    pch = strtok (NULL, ":");
                }
                if (_scanDataLenSizes.size() < 2) {
                    fprintf(stderr, "'-scan <size1>:<size2>:...:<sizeN>' the number of size should be equal or greater then two.\n");
                    return false;
                }
                std::sort(_scanDataLenSizes.begin(), _scanDataLenSizes.end());
                if (_scanDataLenSizes[0] < (unsigned long)OVERHEAD_BYTES) {
                    fprintf(stderr, "-scan sizes must be >= %d\n",
                            OVERHEAD_BYTES);
                    return false;
                }
                if (_scanDataLenSizes[_scanDataLenSizes.size() - 1] >
                        (unsigned long)MAX_PERFTEST_SAMPLE_SIZE) {
                    fprintf(stderr,"-scan sizes must be <= %d\n",
                            MAX_PERFTEST_SAMPLE_SIZE);
                    return false;
                }
            } else { // Set default values
                set_default_scan_values(_scanDataLenSizes);
            }
        }
        else if (IS_OPTION(argv[i], "-noPrintIntervals") ) {
        }
        else if (IS_OPTION(argv[i], "-useReadThread") )
        {
        }
        else if (IS_OPTION(argv[i], "-bestEffort")) {
        } else if (IS_OPTION(argv[i], "-latencyTest"))
        {
        }
        else if (IS_OPTION(argv[i], "-instances"))
        {
            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -instances\n");
                return false;
            }

            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            _InstanceCount = strtol(argv[i], NULL, 10);

            if (_InstanceCount <= 0)
            {
                fprintf(stderr, "instance count cannot be negative or null\n");
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-verbosity"))
        {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-pubRate")) {
            ++i;
        }
        else if (IS_OPTION(argv[i], "-keyed")) {
        }
        else if (IS_OPTION(argv[i], "-writerStats")) {
        }
        else if (IS_OPTION(argv[i], "-executionTime"))
        {
            ++i;
        } else if (IS_OPTION(argv[i], "-cpu"))
        {
        } else if (IS_OPTION(argv[i], "-cft"))
        {
            _useCft = true;
            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <start>:<end> after -cft\n");
                return false;
            }

            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;
        } else
        {
            _MessagingArgv[_MessagingArgc] = DDS_String_dup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;
        }
    }

    // Validate and manage the parameter

    // Manage parameter -sleep
    _SleepNanosec = 1000000 * PM::GetInstance().get<unsigned int>("sleep");

    // Manage parameter -printIterval
    // It is copied because it is used in the critical patch
    perftest_cpp::printIntervals =
            PM::GetInstance().get<bool>("noPrintIntervals");

    // Manage parameter -cpu
    // It is copied because it is used in the critical patch
    perftest_cpp::showCpu = PM::GetInstance().get<bool>("cpu");

    // Manage parameter -sidMultiSubTest
    // It is copied because it is used in the critical patch
    perftest_cpp::_SubID = PM::GetInstance().get<int>("sidMultiSubTest");

    if(PM::GetInstance().get<bool>("latencyTest")) {
        if(PM::GetInstance().get<int>("pidMultiPubTest") != 0) {
            fprintf(stderr, "Only the publisher with ID = 0 can run the latency test\n");
            return false;
        }

        // With latency test, latency should be 1
        if(!PM::GetInstance().is_set("latencyCount")) {
            PM::GetInstance().set<unsigned long long>("latencyCount",1);
        }

        /*
         * PERFTEST-108
         * If we are in a latency test, the default value for _NumIter has to
         * be smaller (to avoid certain issues in platforms with low memory).
         * Therefore, unless we explicitly changed the _NumIter value we will
         * use a smaller default: "numIterDefaultLatencyTest"
         */
        if (!PM::GetInstance().is_set("numIter")) {
            PM::GetInstance().set<unsigned long long>(
                    "numIter",
                    numIterDefaultLatencyTest);
        }
    }

    if(!PM::GetInstance().is_set("latencyCount")) {
        PM::GetInstance().set<unsigned long long>("latencyCount",10000);
    }

    if (PM::GetInstance().get<unsigned long long>("numIter") <
            PM::GetInstance().get<unsigned long long>("latencyCount")) {
        fprintf(stderr,
                "numIter (%llu) must be greater than latencyCount (%llu).\n",
                PM::GetInstance().get<unsigned long long>("numIter"),
                PM::GetInstance().get<unsigned long long>("latencyCount"));
        return false;
    }

    //manage the parameter: -pubRate -sleep -spin
    if (PM::GetInstance().is_set("pubRate")) {
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

    // Manage the parameter: -unbounded
    if (PM::GetInstance().is_set("unbounded")) {
        if (PM::GetInstance().get<int>("unbounded") == 0) {
            PM::GetInstance().set<unsigned long long>(
                    "unbounded",
                    (std::min)(2 * _DataLen, (unsigned long)MAX_BOUNDED_SEQ_SIZE));
        }
    }

    if (_isScan) {
        // TODO sort the scan vector
        _DataLen = _scanDataLenSizes[_scanDataLenSizes.size() - 1]; // Max size
        if (PM::GetInstance().get<unsigned long long>("executionTime") == 0){
            PM::GetInstance().set<unsigned long long>("executionTime", 60);
        }
        // Check if large data or small data
        if (_scanDataLenSizes[0] < (unsigned long) (std::min)(MAX_SYNCHRONOUS_SIZE,MAX_BOUNDED_SEQ_SIZE)
                && _scanDataLenSizes[_scanDataLenSizes.size() - 1] > (unsigned long) (std::min)(MAX_SYNCHRONOUS_SIZE,MAX_BOUNDED_SEQ_SIZE)) {
            fprintf(stderr, "The sizes of -scan [");
            for (unsigned int i = 0; i < _scanDataLenSizes.size(); i++) {
                fprintf(stderr, "%lu ", _scanDataLenSizes[i]);
            }
            fprintf(stderr, "] should be either all smaller or all bigger than %d.\n",
                    (std::min)(MAX_SYNCHRONOUS_SIZE,MAX_BOUNDED_SEQ_SIZE));
            return false;
        }
    }

    /* Check if we need to enable Large Data. This works also for -scan */
    if (_DataLen > (unsigned long) (std::min)(
            MAX_SYNCHRONOUS_SIZE,
            MAX_BOUNDED_SEQ_SIZE)) {
        if (PM::GetInstance().get<int>("unbounded") == 0) {
            PM::GetInstance().set<unsigned long long>("unbounded", MAX_BOUNDED_SEQ_SIZE);
        }
    } else { /* No Large Data */
        if (PM::GetInstance().get<int>("unbounded") != 0) {
            fprintf(stderr, "Unbounded will be ignored since large data is not presented.\n");
            PM::GetInstance().set<unsigned long long>("unbounded", 0);
        }
    }

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
void perftest_cpp::PrintConfiguration()
{
    std::ostringstream stringStream;

    // Throughput/Latency mode
    if (PM::GetInstance().get<bool>("pub")) {
        stringStream << "\nMode: ";

        if (PM::GetInstance().get<bool>("latencyTest")) {
            stringStream << "LATENCY TEST (Ping-Pong test)\n";
        } else {
            stringStream << "THROUGHPUT TEST\n"
                         << "      (Use \"-latencyTest\" for Latency Mode)\n";
        }
    }

    stringStream << "\nPerftest Configuration:\n";

    // Reliable/Best Effort
    stringStream << "\tReliability: ";
    if (!PM::GetInstance().get<bool>("bestEffort")) {
        stringStream << "Reliable\n";
    } else {
        stringStream << "Best Effort\n";
    }

    // Keyed/Unkeyed
    stringStream << "\tKeyed: ";
    if (PM::GetInstance().get<bool>("keyed")) {
        stringStream << "Yes\n";
    } else {
        stringStream << "No\n";
    }

    // Publisher/Subscriber and Entity ID
    if (PM::GetInstance().get<bool>("pub")) {
        stringStream << "\tPublisher ID: "
                     << PM::GetInstance().get<int>("pidMultiPubTest")
                     << "\n";
    } else {
        stringStream << "\tSubscriber ID: " << _SubID << "\n";
    }

    if (PM::GetInstance().get<bool>("pub")) {
        // Latency Count
        stringStream << "\tLatency count: 1 latency sample every "
                     << PM::GetInstance().get<unsigned long long>("latencyCount")
                     << " samples\n";

        // Scan/Data Sizes
        stringStream << "\tData Size: ";
        if (_isScan) {
            for (unsigned long i = 0; i < _scanDataLenSizes.size(); i++ ) {
                stringStream << _scanDataLenSizes[i];
                if (i == _scanDataLenSizes.size() - 1) {
                    stringStream << "\n";
                } else {
                    stringStream << ", ";
                }
            }
        } else {
            stringStream << _DataLen << "\n";
        }

        // Batching
        int batchSize = _MessagingImpl->GetBatchSize();
        stringStream << "\tBatching: ";
        if (batchSize > 0) {
            stringStream << batchSize << " Bytes (Use \"-batchSize 0\" to disable batching)\n";
        } else if (batchSize == 0) {
            stringStream << "No (Use \"-batchSize\" to setup batching)\n";
        } else { // < 0
            stringStream << "Disabled by RTI Perftest.\n";
            if (batchSize == -1) {
                stringStream << "\t\t  BatchSize is smaller than 2 times\n"
                             << "\t\t  the minimum sample size.\n";
            } else if (batchSize == -2) {
                stringStream << "\t\t  BatchSize cannot be used with\n"
                             << "\t\t  Large Data.\n";
            }
        }

        // Publication Rate
        stringStream << "\tPublication Rate: ";
        if (PM::GetInstance().is_set("pubRate")) {
            stringStream << PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").first
                         << " Samples/s (";
            if (PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").second == "spin") {
                stringStream << "Spin)\n";
            } else {
                stringStream << "Sleep)\n";
            }
        } else {
            stringStream << "Unlimited (Not set)\n";
        }
        // Execution Time or Num Iter
        if (PM::GetInstance().get<unsigned long long>("executionTime") > 0) {
            stringStream << "\tExecution time: "
                         << PM::GetInstance().get<unsigned long long>("executionTime")
                         << " seconds\n";
        } else {
            stringStream << "\tNumber of samples: "
                         << PM::GetInstance().get<unsigned long long>("numIter")
                         << "\n";
        }
    }

    // Listener/WaitSets
    stringStream << "\tReceive using: ";
    if (PM::GetInstance().get<bool>("useReadThread")) {
        stringStream << "WaitSets\n";
    } else {
        stringStream << "Listeners\n";
    }

    stringStream << _MessagingImpl->PrintConfiguration();
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

    IMessagingWriter *_writer;
    IMessagingReader *_reader;
    unsigned long * _last_seq_num;

    int _num_publishers;
    std::vector<int> _finished_publishers;
    CpuMonitor cpu;
    bool _useCft;
    bool change_size;

  public:

    ThroughputListener(IMessagingWriter *writer, IMessagingReader *reader = NULL, bool UseCft = false, int numPublishers = 1)
    {
        packets_received = 0;
        bytes_received = 0;
        missing_packets = 0;
        end_test = false;
        change_size = false;
        last_data_length = -1;
        interval_data_length = -1;
        interval_packets_received = 0;
        interval_bytes_received = 0;
        interval_missing_packets = 0;
        interval_time = 0;
        missing_packets_percent = 0.0;
        begin_time = 0;
        _writer = writer;
        _reader = reader;
        _last_seq_num = new unsigned long[numPublishers];
        _useCft = UseCft;

        for (int i=0; i<numPublishers; i++) {
            _last_seq_num[i] = 0;
        }

        _num_publishers = numPublishers;
    }

    ~ThroughputListener() {
        if (_last_seq_num != NULL) {
            delete []_last_seq_num;
        }
    }

    void ProcessMessage(TestMessage &message)
    {
        if (message.entity_id >= _num_publishers ||
            message.entity_id < 0) {
            printf("ProcessMessage: message content no valid. message.entity_id out of bounds\n");
            return;
        }
        // Check for test initialization messages
        if (message.size == perftest_cpp::INITIALIZE_SIZE)
        {
            _writer->Send(message);
            _writer->Flush();
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
                print_summary(message);
                end_test = true;
            }
            return;
        }

        // Send back a packet if this is a ping
        if ((message.latency_ping == perftest_cpp::_SubID) ||
                (_useCft && message.latency_ping != -1)) {
            _writer->Send(message);
            _writer->Flush();
        }

        // Always check if need to reset internals
        if (message.size == perftest_cpp::LENGTH_CHANGED_SIZE)
        {
            print_summary(message);
            change_size = true;
            return;
        }

        // case where not running a scan
        if (message.size != last_data_length)
        {
            packets_received = 0;
            bytes_received = 0;
            missing_packets = 0;

            for (int i=0; i<_num_publishers; i++) {
                _last_seq_num[i] = 0;
            }

            begin_time = perftest_cpp::GetTimeUsec();

            if (!perftest_cpp::printIntervals) {
                printf("\n\n********** New data length is %d\n",
                       message.size + perftest_cpp::OVERHEAD_BYTES);
                fflush(stdout);
            }
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

    void print_summary(TestMessage &message){

        // store the info for this interval
        unsigned long long now = perftest_cpp::GetTimeUsec();

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

            std::string outputCpu = "";
            if (perftest_cpp::showCpu) {
                outputCpu = cpu.get_cpu_average();
            }
            printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  "
                   "Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%) %s\n",
                   interval_data_length + perftest_cpp::OVERHEAD_BYTES,
                   interval_packets_received,
                   interval_packets_received*1000000/interval_time,
                   interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                   interval_missing_packets,
                   missing_packets_percent,
                   outputCpu.c_str()
            );
            fflush(stdout);
        }

        packets_received = 0;
        bytes_received = 0;
        missing_packets = 0;
        // length changed only used in scan mode in which case
        // there is only 1 publisher with ID 0
        _last_seq_num[0] = 0;
        begin_time = now;
    }
};

/*********************************************************
 * Used for receiving data using a thread instead of callback
 *
 */
static void *ThroughputReadThread(void *arg)
{
    ThroughputListener *listener = static_cast<ThroughputListener *>(arg);
    TestMessage *message = NULL;

    while (!listener->end_test)
    {
        // Receive message should block until a message is received
        message = listener->_reader->ReceiveMessage();

        if (message != NULL)
        {
            listener->ProcessMessage(*message);
        }
    }

    return NULL;
}

/*********************************************************
 * Subscriber
 */
int perftest_cpp::Subscriber()
{
    ThroughputListener *reader_listener = NULL;
    IMessagingReader   *reader;
    IMessagingWriter   *writer;
    IMessagingWriter   *announcement_writer;

    // create latency pong writer
    writer = _MessagingImpl->CreateWriter(LATENCY_TOPIC_NAME);

    if (writer == NULL) {
        fprintf(stderr, "Problem creating latency writer.\n");
        return -1;
    }

    // Check if using callbacks or read thread
    if (!PM::GetInstance().get<bool>("useReadThread")) {
        // create latency pong reader
        reader_listener = new ThroughputListener(writer, NULL, _useCft, _NumPublishers);
        reader = _MessagingImpl->CreateReader(
                THROUGHPUT_TOPIC_NAME,
                reader_listener);
        if (reader == NULL)
        {
            fprintf(stderr, "Problem creating throughput reader.\n");
            return -1;
        }
    } else {
        reader = _MessagingImpl->CreateReader(
                THROUGHPUT_TOPIC_NAME,
                NULL);
        if (reader == NULL)
        {
            fprintf(stderr, "Problem creating throughput reader.\n");
            return -1;
        }
        reader_listener = new ThroughputListener(writer, reader, _useCft, _NumPublishers);

        RTIOsapiThread_new("ReceiverThread",
                            RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
                            RTI_OSAPI_THREAD_OPTION_DEFAULT,
                            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
                            NULL,
                            ThroughputReadThread,
                            reader_listener);
    }

    // Create announcement writer
    announcement_writer = _MessagingImpl->CreateWriter(
            ANNOUNCEMENT_TOPIC_NAME);

    if (announcement_writer == NULL) {
        fprintf(stderr, "Problem creating announcement writer.\n");
        return -1;
    }

    // Synchronize with publishers
    fprintf(stderr,"Waiting to discover %d publishers ...\n", _NumPublishers);
    fflush(stderr);
    reader->WaitForWriters(_NumPublishers);
    announcement_writer->WaitForReaders(_NumPublishers);

    /*
     * Announcement message that will be used by the announcement_writer
     * to send information to the Publisher. This message size will indicate
     * different things.
     *
     * We will use 3 sizes: INITIALIZE_SIZE, LENGTH_CHANGED_SIZE and FINISHED_SIZE,
     * msg.data will be used as the payload of such messages, so we choose the
     * greatest size:
     */
    TestMessage announcement_msg;
    announcement_msg.entity_id = _SubID;
    announcement_msg.size = perftest_cpp::INITIALIZE_SIZE;
    announcement_msg.data = new char[LENGTH_CHANGED_SIZE];

    // Send announcement message
    announcement_writer->Send(announcement_msg);
    announcement_writer->Flush();


    fprintf(stderr,"Waiting for data...\n");
    fflush(stderr);

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

    if (perftest_cpp::showCpu) {
        reader_listener->cpu.initialize();
    }

    now = GetTimeUsec();

    while (true) {
        prev_time = now;
        MilliSleep(1000);
        now = GetTimeUsec();

        if (reader_listener->change_size) { // ACK change_size
            announcement_msg.entity_id = _SubID;
            announcement_msg.size =  LENGTH_CHANGED_SIZE;
            announcement_writer->Send(announcement_msg);
            announcement_writer->Flush();
            reader_listener->change_size = false;
        }

        if (reader_listener->end_test) { // ACK end_test
            announcement_msg.entity_id = _SubID;
            announcement_msg.size = FINISHED_SIZE;
            announcement_writer->Send(announcement_msg);
            announcement_writer->Flush();
            break;
        }

        if (!perftest_cpp::printIntervals) {
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
                std::string outputCpu = "";
                if (perftest_cpp::showCpu) {
                    outputCpu = reader_listener->cpu.get_cpu_instant();
                }
                printf("Packets: %8llu  Packets/s: %7llu  Packets/s(ave): %7.0lf  "
                       "Mbps: %7.1lf  Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%) %s\n",
                        last_msgs, mps, mps_ave,
                        bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                        reader_listener->missing_packets,
                        missing_packets_percent,
                        outputCpu.c_str()
                );
                fflush(stdout);
            }
        }
    }

    perftest_cpp::MilliSleep(2000);

    if (reader != NULL)
    {
        reader->Shutdown();
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
    std::vector<int> subscriber_list;

    AnnouncementListener() {
    }

    void ProcessMessage(TestMessage& message) {
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
        if ((message.size == perftest_cpp::INITIALIZE_SIZE
                || message.size == perftest_cpp::LENGTH_CHANGED_SIZE)
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

    unsigned long long latency_sum ;
    unsigned long long latency_sum_square;
    unsigned long long count;
    unsigned long      latency_min;
    unsigned long      latency_max;
    int                last_data_length;
    unsigned long     *_latency_history;
    unsigned long      clock_skew_count;
    unsigned int       _num_latency;
    IMessagingWriter *_writer;
 public:
    IMessagingReader *_reader;
    CpuMonitor cpu;

  public:

    LatencyListener(unsigned int num_latency, IMessagingReader *reader,
            IMessagingWriter *writer)
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
    }

    void print_summary_latency(){

        double latency_ave;
        double latency_std;
        std::string outputCpu = "";
        if (count == 0)
        {
            return;
        }

        if (clock_skew_count != 0) {
            fprintf(stderr,"The following latency result may not be accurate because clock skew happens %lu times\n",
                        clock_skew_count);
            fflush(stderr);
        }

        // sort the array (in ascending order)
        std::sort(_latency_history, _latency_history+count);
        latency_ave = (double)latency_sum / count;
        latency_std = sqrt((double)latency_sum_square / (double)count - (latency_ave * latency_ave));

        if (perftest_cpp::showCpu) {
            outputCpu = cpu.get_cpu_average();
        }

        printf("Length: %5d  Latency: Ave %6.0lf us  Std %6.1lf us  "
               "Min %6lu us  Max %6lu us  50%% %6lu us  90%% %6lu us  99%% %6lu us  99.99%% %6lu us  99.9999%% %6lu us %s\n",
               last_data_length + perftest_cpp::OVERHEAD_BYTES,
               latency_ave, latency_std, latency_min, latency_max,
               _latency_history[count*50/100],
               _latency_history[count*90/100],
               _latency_history[count*99/100],
               _latency_history[(int)(count*(9999.0/10000))],
               _latency_history[(int)(count*(999999.0/1000000))],
               outputCpu.c_str()
        );
        fflush(stdout);

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

    void ProcessMessage(TestMessage &message)
    {
        unsigned long long now, sentTime;
        unsigned long latency;
        int sec;
        unsigned int usec;
        double latency_ave;
        double latency_std;
        std::string outputCpu = "";

        now = perftest_cpp::GetTimeUsec();

        switch (message.size)
        {
            // Initializing message, don't process
            case perftest_cpp::INITIALIZE_SIZE:
                return;

            // Test finished message
            case perftest_cpp::FINISHED_SIZE:
                return;

            // Data length is changing size
            case perftest_cpp::LENGTH_CHANGED_SIZE:
                print_summary_latency();
                return;

            default:
                break;
        }

        if (last_data_length != message.size)
        {
            latency_sum = 0;
            latency_sum_square = 0;
            latency_min = perftest_cpp::LATENCY_RESET_VALUE;
            latency_max = 0;
            count = 0;
        }

        sec = message.timestamp_sec;
        usec = message.timestamp_usec;
        sentTime = ((unsigned long long)sec << 32) | (unsigned long long)usec;

        if (now >= sentTime)
        {
            latency = (unsigned long)(now - sentTime);

            // keep track of one-way latency
            latency /= 2;
        }
        else
        {
            fprintf(stderr,"Clock skew suspected: received time %llu usec, sent time %llu usec",
                            now, sentTime);
                ++clock_skew_count;
            return;
        }

        // store value for percentile calculations
        if (_latency_history != NULL)
        {
            if (count >= _num_latency)
            {
                fprintf(stderr,"Too many latency pongs received.  Do you have more than 1 app with -pidMultiPubTest = 0 or -sidMultiSubTest 0?\n");
                return;
            }
            else
            {
                _latency_history[count] = latency;
            }
        }

        if (latency_min == perftest_cpp::LATENCY_RESET_VALUE) {
            latency_min = latency;
            latency_max = latency;
        }
        else {
            if (latency < latency_min) {
                latency_min = latency;
            } else if (latency > latency_max) {
                latency_max = latency;
            }
        }

        ++count;
        latency_sum += latency;
        latency_sum_square += ((unsigned long long)latency * (unsigned long long)latency);

        // if data sized changed, print out stats and zero counters
        if (last_data_length != message.size)
        {
            last_data_length = message.size;

            if (!perftest_cpp::printIntervals) {
                printf("\n\n********** New data length is %d\n",
                       last_data_length + perftest_cpp::OVERHEAD_BYTES);
            }
        }
        else {
            if (!perftest_cpp::printIntervals) {
                latency_ave = (double)latency_sum / (double)count;
                latency_std = sqrt(
                        (double)latency_sum_square / (double)count - (latency_ave * latency_ave));

                if (perftest_cpp::showCpu) {
                    outputCpu = cpu.get_cpu_instant();
                }
                printf("One way Latency: %6lu us  Ave %6.0lf us  Std %6.1lf us  Min %6lu us  Max %6lu %s\n",
                        latency,
                        latency_ave,
                        latency_std,
                        latency_min,
                        latency_max,
                        outputCpu.c_str()
                );
            }
        }

        if(_writer != NULL) {
            _writer->notifyPingResponse();
        }
    }
};

/*********************************************************
 * Used for receiving data using a thread instead of callback
 *
 */
static void *LatencyReadThread(void *arg)
{
    LatencyListener *listener = static_cast<LatencyListener *>(arg);
    TestMessage *message = NULL;

    while (!listener->end_test)
    {
        // Receive message should block until a message is received
        message = listener->_reader->ReceiveMessage();

        if (message != NULL)
        {
            listener->ProcessMessage(*message);
        }

    /*
    * TODO:
    *
    * To support -latencyTest plus -useReadThread we need to signal
    * --HERE-- the internal semaphore used in RTIDDSImpl.cxx as
    * we now do in the listener on_data_available callback
    * inside RTIDDSImpl.cxx
    *
    */
    }

    return NULL;
}

/*********************************************************
 * Publisher
 */
int perftest_cpp::Publisher()
{
    LatencyListener *reader_listener = NULL;
    AnnouncementListener  *announcement_reader_listener = NULL;
    IMessagingReader *announcement_reader;
    unsigned long num_latency;
    unsigned long announcementSampleCount = 50;
    unsigned int samplesPerBatch = 1;

    // create throughput/ping writer
    IMessagingWriter *writer = _MessagingImpl->CreateWriter(
            THROUGHPUT_TOPIC_NAME);

    if (writer == NULL)
    {
        fprintf(stderr,"Problem creating throughput writer.\n");
        return -1;
    }

    samplesPerBatch = GetSamplesPerBatch();

    // calculate number of latency pings that will be sent per data size
    num_latency = (unsigned long)((PM::GetInstance().get<unsigned long long>("numIter") /
            samplesPerBatch) /
            PM::GetInstance().get<unsigned long long>("latencyCount"));
    if ((PM::GetInstance().get<unsigned long long>("numIter") /
            samplesPerBatch) %
            PM::GetInstance().get<unsigned long long>("latencyCount") > 0) {
        num_latency++;
    }

    if (samplesPerBatch > 1) {
        // in batch mode, might have to send another ping
        ++num_latency;
    }

    IMessagingReader *reader;
    // Only publisher with ID 0 will send/receive pings
    if (PM::GetInstance().get<int>("pidMultiPubTest") == 0)
    {
        // Check if using callbacks or read thread
        if (!PM::GetInstance().get<bool>("useReadThread")) {
            // create latency pong reader
            // the writer is passed for ping-pong notification in LatencyTest
            reader_listener = new LatencyListener(
                    num_latency,
                    NULL,
                    PM::GetInstance().get<bool>("latencyTest") ? writer : NULL);
            reader = _MessagingImpl->CreateReader(
                    LATENCY_TOPIC_NAME,
                    reader_listener);
            if (reader == NULL)
            {
                fprintf(stderr,"Problem creating latency reader.\n");
                return -1;
            }
        }
        else
        {
            reader = _MessagingImpl->CreateReader(
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
                    PM::GetInstance().get<bool>("latencyTest") ? writer : NULL);

            RTIOsapiThread_new("ReceiverThread",
                                RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
                                RTI_OSAPI_THREAD_OPTION_DEFAULT,
                                RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
                                NULL,
                                LatencyReadThread,
                                reader_listener);
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
    announcement_reader_listener = new AnnouncementListener();
    announcement_reader = _MessagingImpl->CreateReader(
            ANNOUNCEMENT_TOPIC_NAME,
            announcement_reader_listener);
    if (announcement_reader == NULL)
    {
        fprintf(stderr,"Problem creating announcement reader.\n");
        return -1;
    }

    unsigned long long spinPerUsec = 0;
    unsigned long sleepUsec = 1000;
    DDS_Duration_t sleep_period = {0,0};

    if (PM::GetInstance().is_set("pubRate")) {
        if (PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").second == "spin") {
            spinPerUsec = NDDSUtility::get_spin_per_microsecond();
            /* A return value of 0 means accuracy not assured */
            if (spinPerUsec == 0) {
                fprintf(stderr,
                        "Error initializing spin per microsecond. '-pubRate'"
                        "cannot be used\nExiting...\n");
                return -1;
            }
            _SpinLoopCount = 1000000 * spinPerUsec /
                    PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").first;
        } else { // sleep count
            _SleepNanosec = 1000000000 /
                    PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").first;
        }
    }

    fprintf(stderr,
            "Waiting to discover %d subscribers ...\n",
            PM::GetInstance().get<int>("numSubscribers"));
    fflush(stderr);
    writer->WaitForReaders(PM::GetInstance().get<int>("numSubscribers"));

    // We have to wait until every Subscriber sends an announcement message
    // indicating that it has discovered every Publisher
    fprintf(stderr,"Waiting for subscribers announcement ...\n");
    fflush(stderr);
    while (PM::GetInstance().get<int>("numSubscribers")
            > (int)announcement_reader_listener->subscriber_list.size()) {
        MilliSleep(1000);
    }

    // Allocate data and set size
    TestMessage message;
    message.entity_id = PM::GetInstance().get<int>("pidMultiPubTest");
    message.data = new char[(std::max)((int)_DataLen, (int)LENGTH_CHANGED_SIZE)];

    if (perftest_cpp::showCpu && PM::GetInstance().get<int>("pidMultiPubTest") == 0) {
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
            _MessagingImpl->GetInitializationSampleCount(),
            _InstanceCount);

    fprintf(stderr,
            "Sending %lu initialization pings ...\n",
            initializeSampleCount);
    fflush(stderr);

    for (unsigned long i = 0; i < initializeSampleCount; i++) {
        // Send test initialization message
        writer->Send(message, true);
    }
    writer->Flush();

    fprintf(stderr,"Publishing data ...\n");
    fflush(stderr);

    // Set data size, account for other bytes in message
    message.size = (int)_DataLen - OVERHEAD_BYTES;

    // Sleep 1 second, then begin test
    MilliSleep(1000);

    int num_pings = 0;
    unsigned int scan_count = 0;
    int pingID = -1;
    int current_index_in_batch = 0;
    int ping_index_in_batch = 0;
    bool sentPing = false;

    unsigned long long time_now = 0, time_last_check = 0, time_delta = 0;
    unsigned long pubRate_sample_period = 1;
    unsigned long rate = 0;

    time_last_check = perftest_cpp::GetTimeUsec();

    /* Minimum value for pubRate_sample_period will be 1 so we execute 100 times
       the control loop every second, or every sample if we want to send less
       than 100 samples per second */
    if (PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").first > 100) {
        pubRate_sample_period =
                PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").first /
                100;
    }

    if (PM::GetInstance().get<unsigned long long>("executionTime") > 0 && !_isScan) {
        SetTimeout(PM::GetInstance().get<unsigned long long>("executionTime"));
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
     * - writerStats
     */
    const unsigned long long numIter =
            PM::GetInstance().get<unsigned long long>("numIter");
    const unsigned long long latencyCount =
            PM::GetInstance().get<unsigned long long>("latencyCount");
    const int numSubscribers = PM::GetInstance().get<int>("numSubscribers");
    const bool bestEffort = PM::GetInstance().get<bool>("bestEffort");
    const bool latencyTest = PM::GetInstance().get<bool>("latencyTest");
    const int pidMultiPubTest = PM::GetInstance().get<int>("pidMultiPubTest");
    const bool pubRateMethodSpin =
            PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").second == "spin";
    const unsigned long pubRate =
            PM::GetInstance().get_pair<unsigned long, std::string>("pubRate").first;
    const bool writerStats = PM::GetInstance().get<bool>("writerStats");
    /********************
     *  Main sending loop
     */
    for (unsigned long long loop = 0;
            (_isScan || (loop < numIter)) && (!_testCompleted);
            ++loop) {

        /* This if has been included to perform the control loop
           that modifies the publication rate according to -pubRate */
        if ((PM::GetInstance().is_set("pubRate")) &&
                (loop > 0) &&
                (loop % pubRate_sample_period == 0)) {

            time_now = perftest_cpp::GetTimeUsec();

            time_delta = time_now - time_last_check;
            time_last_check = time_now;
            rate = (pubRate_sample_period * 1000000) / (unsigned long)time_delta;
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
            sleep_period.nanosec = (DDS_UnsignedLong)_SleepNanosec;
            NDDSUtility::sleep(sleep_period);
        }

        pingID = -1;

        // only send latency pings if is publisher with ID 0
        // In batch mode, latency pings are sent once every LatencyCount batches
        if ((pidMultiPubTest == 0) && (((loop / samplesPerBatch) % latencyCount) == 0)) {
            /* In batch mode only send a single ping in a batch.
             *
             * However, the ping is sent in a round robin position within
             * the batch.  So keep track of which position(index) the
             * current sample is within the batch, and which position
             * within the batch should contain the ping. Only send the ping
             * when both are equal.
             *
             * Note when not in batch mode, current_index_in_batch = ping_index_in_batch
             * always.  And the if() is always true.
             */
            if ( current_index_in_batch == ping_index_in_batch  && !sentPing )
            {
                // If running in scan mode, dataLen under test is changed
                // after executionTime
                if (_isScan && _testCompleted_scan) {
                    _testCompleted_scan = false;
                    SetTimeout(
                            PM::GetInstance().get<unsigned long long>("executionTime"),
                            _isScan);

                    // flush anything that was previously sent
                    writer->Flush();
                    writer->waitForAck(
                            timeout_wait_for_ack_sec,
                            timeout_wait_for_ack_nsec);

                    if (scan_count == _scanDataLenSizes.size()) {
                        break; // End of scan test
                    }

                    message.size = LENGTH_CHANGED_SIZE;
                    // must set latency_ping so that a subscriber sends us
                    // back the LENGTH_CHANGED_SIZE message
                    message.latency_ping = num_pings % numSubscribers;

                    /*
                     * If the Throughput topic is reliable, we can send the packet and do
                     * a wait for acknowledgements. However, if the Throughput topic is
                     * Best Effort, wait_for_acknowledgments() will return inmediately.
                     * This would cause that the Send() would be exercised too many times,
                     * in some cases causing the network to be flooded, a lot of packets being
                     * lost, and potentially CPU starbation for other processes.
                     * We can prevent this by adding a small sleep() if the test is best
                     * effort.
                     */
                    announcement_reader_listener->subscriber_list.clear();
                    while ((int)announcement_reader_listener->subscriber_list.size()
                            < numSubscribers) {
                        writer->Send(message, true);
                        writer->Flush();
                        writer->waitForAck(
                            timeout_wait_for_ack_sec,
                            timeout_wait_for_ack_nsec);
                    }

                    message.size = _scanDataLenSizes[scan_count++] - OVERHEAD_BYTES;
                    /* Reset _SamplePerBatch */
                    samplesPerBatch = GetSamplesPerBatch();

                    ping_index_in_batch = 0;
                    current_index_in_batch = 0;
                }

                // Each time ask a different subscriber to echo back
                pingID = num_pings % numSubscribers;
                unsigned long long now = GetTimeUsec();
                message.timestamp_sec = (int)((now >> 32) & 0xFFFFFFFF);
                message.timestamp_usec = (unsigned int)(now & 0xFFFFFFFF);
                ++num_pings;
                ping_index_in_batch = (ping_index_in_batch + 1) % samplesPerBatch;
                sentPing = true;

                if (writerStats && !perftest_cpp::printIntervals) {
                    printf("Pulled samples: %7d\n", writer->getPulledSampleCount());
                }
            }
        }
        current_index_in_batch = (current_index_in_batch + 1) % samplesPerBatch;

        message.seq_num = (unsigned long) loop;
        message.latency_ping = pingID;
        writer->Send(message);
        if(latencyTest && sentPing) {
            if (!bestEffort) {
                writer->waitForPingResponse();
            } else {
                /* time out in milliseconds */
                writer->waitForPingResponse(200);
            }
        }


        // come to the beginning of another batch
        if (current_index_in_batch == 0)
        {
            sentPing = false;
        }
    }

    // In case of batching, flush
    writer->Flush();

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
        writer->Send(message, true);
        writer->Flush();
        writer->waitForAck(
                timeout_wait_for_ack_sec,
                timeout_wait_for_ack_nsec);
        i++;
    }

    if (PM::GetInstance().get<int>("pidMultiPubTest") == 0) {
        reader_listener->print_summary_latency();
        reader_listener->end_test = true;
    } else {
        fprintf(
            stderr,
            "Latency results are only shown when -pidMultiPubTest = 0\n");
    }

    if (PM::GetInstance().get<bool>("writerStats")) {
        printf("Pulled samples: %7d\n", writer->getPulledSampleCount());
    }

    if (reader_listener != NULL) {
        delete(reader_listener);
    }

    if (reader != NULL) {
        delete(reader);
    }

    if (writer != NULL) {
        delete(writer);
    }

    if (announcement_reader != NULL) {
        delete(announcement_reader);
    }

    if (announcement_reader_listener != NULL) {
        delete(announcement_reader_listener);
    }

    delete []message.data;

    if (_testCompleted) {
        fprintf(stderr,"Finishing test due to timer...\n");
    } else {
        fprintf(stderr,"Finishing test...\n");
    }
    fflush(stderr);

    return 0;
}


/*********************************************************
 * Utility functions
 */

inline void perftest_cpp::SetTimeout(unsigned int executionTimeInSeconds,
        bool _isScan) {
    if (_isScan) {
      #ifdef RTI_WIN32
        CreateTimerQueueTimer(&_hTimer, _hTimerQueue, (WAITORTIMERCALLBACK)Timeout_scan,
                NULL , executionTimeInSeconds * 1000, 0, 0);
      #else
        signal(SIGALRM, Timeout_scan);
        alarm(executionTimeInSeconds);
      #endif
    } else {
        fprintf(stderr,"Setting timeout to %u seconds\n", executionTimeInSeconds);
      #ifdef RTI_WIN32
        CreateTimerQueueTimer(&_hTimer, _hTimerQueue, (WAITORTIMERCALLBACK)Timeout,
                NULL , executionTimeInSeconds * 1000, 0, 0);
      #else
        signal(SIGALRM, Timeout);
        alarm(executionTimeInSeconds);
      #endif
    }
}

inline unsigned long long perftest_cpp::GetTimeUsec() {
    perftest_cpp::_Clock->getTime(
            perftest_cpp::_Clock,
            &perftest_cpp::_ClockTime_aux);
    RTINtpTime_unpackToMicrosec(
            perftest_cpp::_Clock_sec,
            perftest_cpp::_Clock_usec,
            perftest_cpp::_ClockTime_aux);
    return perftest_cpp::_Clock_usec + 1000000 * perftest_cpp::_Clock_sec;
}

inline unsigned int perftest_cpp::GetSamplesPerBatch() {
    int batchSize = _MessagingImpl->GetBatchSize();
    unsigned int samplesPerBatch;

    if (batchSize > 0) {
        samplesPerBatch = batchSize / (int) _DataLen;
        if (samplesPerBatch == 0) {
            samplesPerBatch = 1;
        }
    } else {
        samplesPerBatch = 1;
    }

    return samplesPerBatch;
}

#ifdef RTI_WIN32
inline VOID CALLBACK perftest_cpp::Timeout(PVOID lpParam, BOOLEAN timerOrWaitFired) {
    /* This is to avoid the warning of non using lpParam */
    (void) lpParam;
    _testCompleted = true;
}

inline VOID CALLBACK perftest_cpp::Timeout_scan(PVOID lpParam, BOOLEAN timerOrWaitFired) {
    /* This is to avoid the warning of non using lpParam */
    (void) lpParam;
    _testCompleted_scan = true;
}

  #pragma warning(pop)
#else
inline void perftest_cpp::Timeout(int sign) {
    _testCompleted = true;
}

inline void perftest_cpp::Timeout_scan(int sign) {
    _testCompleted_scan = true;
}
#endif
