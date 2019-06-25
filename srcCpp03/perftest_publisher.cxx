/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "perftest_cpp.h"
#include "CpuMonitor.h"

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #define STRNCASECMP _strnicmp
  #pragma warning(push)
  #pragma warning(disable : 4996)
#elif defined(RTI_VXWORKS)
  #define STRNCASECMP strncmp
#else
  #define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

#if defined(RTI_ANDROID)

#include <android/log.h>

typedef int (*RTIAndroidOnCoutMethod)(const char *text);
static RTIAndroidOnCoutMethod publisher_onCout = NULL;
extern "C" void RTIAndroid_registerOnCout(RTIAndroidOnCoutMethod onCout) {
    publisher_onCout = onCout;
}

#define RTI_ANDROID_BUFFER_SIZE 512

class RTIAndroidBuffer : public std::streambuf {
  public:
    RTIAndroidBuffer() {
        this->setp(buffer, buffer + RTI_ANDROID_BUFFER_SIZE - 1);
    }

  private:
    int overflow(int c) {
        if (c == traits_type::eof()) {
            *this->pptr() = traits_type::to_char_type(c);
            this->sbumpc();
        }
        return this->sync()? traits_type::eof(): traits_type::not_eof(c);
    }

    int sync() {
        int rc = 0;
        if (this->pbase() != this->pptr()) {
            char writebuf[RTI_ANDROID_BUFFER_SIZE + 1];
            memcpy(writebuf, this->pbase(), this->pptr() - this->pbase());
            writebuf[this->pptr() - this->pbase()] = '\0';

            /* forward message */
            if (publisher_onCout != NULL) {
                rc = publisher_onCout(writebuf);
            } else {
                rc = __android_log_write(ANDROID_LOG_INFO, "RTIConnextLog", writebuf) >  0;
            }

            this->setp(buffer, buffer + RTI_ANDROID_BUFFER_SIZE - 1);
        }
        return rc;
    }

    char buffer[RTI_ANDROID_BUFFER_SIZE];
};

#endif

bool perftest_cpp::_testCompleted = false;
bool perftest_cpp::_testCompleted_scan = true; // In order to enter into the scan mode

/* Clock related variables */
struct RTIClock* perftest_cpp::_Clock = RTIHighResolutionClock_new();
struct RTINtpTime perftest_cpp::_ClockTime_aux = RTI_NTP_TIME_ZERO;
RTI_UINT64 perftest_cpp::_Clock_sec = 0;
RTI_UINT64 perftest_cpp::_Clock_usec = 0;

const long timeout_wait_for_ack_sec = 0;
const unsigned long timeout_wait_for_ack_nsec = 100000000;
const Perftest_ProductVersion_t perftest_cpp::_version = {9, 9, 9, 9};
ThreadPriorities _threadPriorities;

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
    try {
        perftest_cpp app;
        return app.Run(argc, argv);
    } catch (const std::exception &ex) {
        // This will catch DDS exceptions
        std::cerr << "[Error] Exception in perftest_cpp::Run(): " << ex.what() << "\n";
        return -1;
    }
}

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
#endif

/*********************************************************
 * Run
 */
int perftest_cpp::Run(int argc, char *argv[]) {

    PrintVersion();

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

    if (_PM.get<int>("unbounded") == 0) {
        if (_PM.get<bool>("keyed")) {
            _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>();
        } else {
            _MessagingImpl = new RTIDDSImpl<TestData_t>();
        }
    } else {
        if (_PM.get<bool>("keyed")) {
            _MessagingImpl = new RTIDDSImpl<TestDataKeyedLarge_t>();
        } else {
            _MessagingImpl = new RTIDDSImpl<TestDataLarge_t>();
        }
    }

    if (!_MessagingImpl->Initialize(_PM, this)) {
        return -1;
    }

    PrintConfiguration();

    if (_PM.get<bool>("pub")) {
        return RunPublisher();
    } else {
        return RunSubscriber();
    }
}

void perftest_cpp::MilliSleep(unsigned int millisec) {
    rti::util::sleep(
            dds::core::Duration::from_millisecs(millisec));
}

void perftest_cpp::ThreadYield() {
  #ifdef RTI_WIN32
    Sleep(0);
  #else
    sched_yield();
  #endif
}

const rti::core::ProductVersion perftest_cpp::GetDDSVersion()
{
    return rti::core::ProductVersion::current();
}

const Perftest_ProductVersion_t perftest_cpp::GetPerftestVersion()
{
    return _version;
}

void perftest_cpp::PrintVersion()
{
    Perftest_ProductVersion_t perftestV = perftest_cpp::GetPerftestVersion();
    rti::core::ProductVersion ddsV = perftest_cpp::GetDDSVersion();

    if (perftestV.major == 9
            && perftestV.minor == 9
            && perftestV.release == 9) {
        printf("RTI Perftest Master");
    } else {
        printf("RTI Perftest %d.%d.%d",
                perftestV.major,
                perftestV.minor,
                perftestV.release);

        if (perftestV.revision != 0) {
            printf(".%d", perftestV.revision);
        }
    }

    printf(" (RTI Connext DDS %d.%d.%d)\n",
           ddsV.major_version(),
           ddsV.minor_version(),
           ddsV.release_version());
    fflush(stdout);
}

/*********************************************************
 * Constructor
 */
perftest_cpp::perftest_cpp() :
        _SpinLoopCount(0),
        _SleepNanosec(0),
        _MessagingImpl(NULL)
{
}
;

/*********************************************************
 * Destructor
 */
perftest_cpp::~perftest_cpp() {

    try {
        if (_MessagingImpl != NULL) {
            delete _MessagingImpl;
        }

        if (perftest_cpp::_Clock != NULL) {
            RTIHighResolutionClock_delete(perftest_cpp::_Clock);
        }

    } catch (const std::exception& ex) {
        // This will catch DDS exceptions
        std::cerr << "[Error] Exception in perftest_cpp::~perftest_cpp(): "
                << ex.what() << "\n";
    }

    std::cerr << "[Info] Test ended." << std::endl;
}

/*********************************************************
 * Validate and manage the parameters
 */
bool perftest_cpp::validate_input()
{

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
            std::cerr << "[Error] Only the publisher with ID = 0 can run the"
                      << " latency test"
                      << std::endl;
            throw std::logic_error("[Error] Error parsing commands");
        }

        // With latency test, latency should be 1
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
        std::cerr << "[Error] numIter "
                  << _PM.get<unsigned long long>("numIter")
                  << " must be greater than latencyCount "
                  << _PM.get<unsigned long long>("latencyCount")
                  << std::endl;
        throw std::logic_error("[Error] Error parsing commands");
    }

    // Manage the parameter: -cft
    if (_PM.is_set("cft")) {
        const std::vector<unsigned long long> cftRange =
                _PM.get_vector<unsigned long long>("cft");
        if (cftRange.size() > 2) {
            std::cerr << "[Error] '-cft' value must have the format <start>:<end>"
                      << std::endl;
            throw std::logic_error("[Error] Error parsing commands");
        } else if (cftRange[0] > cftRange[1]) {
            std::cerr << "[Error] '-cft' <start> value cannot be bigger than <end>"
                     << std::endl;
            throw std::logic_error("[Error] Error parsing commands");
        }
    }

    // Manage the parameter: -pubRate -sleep -spin
    if (_PM.is_set("pubRate")) {
        if (_SpinLoopCount > 0) {
            std::cerr << "[Error] '-spin' is not compatible with -pubRate. "
                "Spin/Sleep value will be set by -pubRate." << std::endl;
            _SpinLoopCount = 0;
        }
        if (_SleepNanosec > 0) {
            std::cerr << "[Error] '-sleep' is not compatible with -pubRate. "
                "Spin/Sleep value will be set by -pubRate." << std::endl;
            _SleepNanosec = 0;
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

    // Manage the parameter: -scan
    if (_PM.is_set("scan")) {
        const std::vector<unsigned long long> scanList =
                _PM.get_vector<unsigned long long>("scan");
        // Max size of scan
        _PM.set<unsigned long long>("dataLen", scanList[scanList.size() - 1]);
        if (_PM.get<unsigned long long>("executionTime") == 0){
            _PM.set<unsigned long long>("executionTime", 60);
        }
        // Check if large data or small data
        if (scanList[0] < (unsigned long long)(std::min)
                    (MAX_SYNCHRONOUS_SIZE, MAX_BOUNDED_SEQ_SIZE)
                && scanList[scanList.size() - 1] > (unsigned long long)(std::min)
                    (MAX_SYNCHRONOUS_SIZE, MAX_BOUNDED_SEQ_SIZE)) {
            std::cerr << "[Error] The sizes of -scan [";
            for (unsigned int i = 0; i < scanList.size(); i++) {
                fprintf(stderr, "%llu ", scanList[i]);
            }
            std::cerr << "] should be either all smaller or all bigger than "
                      << (std::min)(MAX_SYNCHRONOUS_SIZE, MAX_BOUNDED_SEQ_SIZE)
                      << std::endl;
            throw std::logic_error("[Error] Error parsing commands");
        }
    }

    // Check if we need to enable Large Data. This works also for -scan
    if (_PM.get<unsigned long long>("dataLen") > (unsigned long long) (std::min)(
            MAX_SYNCHRONOUS_SIZE, MAX_BOUNDED_SEQ_SIZE)) {
        if (_PM.get<int>("unbounded") == 0) {
            _PM.set<int>("unbounded", MAX_BOUNDED_SEQ_SIZE);
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

        // Scan/Data Sizes
        stringStream << "\tData Size: ";
        if (_PM.is_set("scan")) {
            const std::vector<unsigned long long> scanList =
                    _PM.get_vector<unsigned long long>("scan");

            for (unsigned long i = 0; i < scanList.size(); i++) {
                stringStream << scanList[i];
                if (i == scanList.size() - 1) {
                    stringStream << "\n";
                } else {
                    stringStream << ", ";
                }
            }
        } else {
            stringStream << _PM.get<unsigned long long>("dataLen") << "\n";
        }

        // Batching
        stringStream << "\tBatching: ";
        if (_PM.get<long>("batchSize") > 0) {
            stringStream << _PM.get<long>("batchSize")
                         << " Bytes (Use \"-batchSize 0\" to disable batching)\n";
        } else if (_PM.get<long>("batchSize") == 0) {
            stringStream << "No (Use \"-batchSize\" to setup batching)\n";
        } else { // < 0
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
                             << "\t\t  Large Data.\n";
            }
        }

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
        } else {
            stringStream << "Unlimited (Not set)\n";
        }
        // Execution Time or Num Iter
        if (_PM.get<unsigned long long>("executionTime") > 0) {
            stringStream << "\tExecution time: "
                         << _PM.get<unsigned long long>("executionTime")
                         << " seconds\n";
        } else {
            stringStream << "\tNumber of samples: "
                         << _PM.get<unsigned long long>("numIter")
                         << "\n";
        }
    } else  {
        if (_PM.get<unsigned long long>("dataLen") > MAX_SYNCHRONOUS_SIZE) {
            stringStream << "\tExpecting Large Data Type\n";
        }
    }

    // Listener/WaitSets
    stringStream << "\tReceive using: ";
    if (_PM.get<bool>("useReadThread")) {
        stringStream << "WaitSets\n";
    } else {
        stringStream << "Listeners\n";
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

    stringStream << _MessagingImpl->PrintConfiguration();
    std::cerr << stringStream.str() << std::endl;

}

/*********************************************************
 * Listener for the Subscriber side
 *
 * Keeps stats on data received per second.
 * Returns a ping for latency packets
 */
class ThroughputListener: public IMessagingCB {
private:

    ParameterManager *_PM;
    int  subID;
    bool printIntervals;
    bool showCpu;

public:

    unsigned long long packets_received;
    unsigned long long bytes_received;
    unsigned long long missing_packets;
    int last_data_length;

    // store info for the last data set
    int interval_data_length;
    unsigned long long interval_packets_received;
    unsigned long long interval_bytes_received;
    unsigned long long interval_missing_packets;
    unsigned long long interval_time, begin_time;
    float missing_packets_percent;

    IMessagingWriter *_writer;
    IMessagingReader *_reader;

    std::vector<unsigned long> _last_seq_num;

    int _num_publishers;
    std::vector<int> _finished_publishers;
    CpuMonitor cpu;
    bool _useCft;
    bool change_size;

    ThroughputListener(
            ParameterManager &PM,
            IMessagingWriter *writer,
            IMessagingReader *reader = NULL,
            bool UseCft = false,
            int numPublishers = 1):
                packets_received(0),
                bytes_received(0),
                missing_packets(0),
                last_data_length(-1),
                interval_data_length(-1),
                interval_packets_received(0),
                interval_bytes_received(0),
                interval_missing_packets(0),
                interval_time(0),
                begin_time(0),
                missing_packets_percent(0.0),
                _writer(writer),
                _reader(reader),
                _last_seq_num(numPublishers),
                _num_publishers(numPublishers),
                _useCft(UseCft),
                change_size(false)

    {
        end_test = false;

        _PM = &PM;

        printIntervals = !_PM->get<bool>("noPrintIntervals");
        showCpu = _PM->get<bool>("cpu");
        subID = _PM->get<int>("sidMultiSubTest");
    }

    ~ThroughputListener() {}

    void ProcessMessage(TestMessage &message)
    {
        int size = message.size;

        if (message.entity_id >= _num_publishers ||
            message.entity_id < 0) {
            std::cerr << "[Error] ProcessMessage: message content no valid."
                    "message.entity_id out of bounds." << std::endl;
            return;
        }
        // Check for test initialization messages
        if (size == perftest_cpp::INITIALIZE_SIZE) {
            _writer->send(message);
            _writer->flush();
            return;

        } else if (size == perftest_cpp::FINISHED_SIZE) {

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

            if (end_test) {
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
        if ((message.latency_ping == subID)
                || (_useCft && message.latency_ping != -1)) {
            _writer->send(message);
            _writer->flush();
        }

        // Always check if need to reset internals
        if (size == perftest_cpp::LENGTH_CHANGED_SIZE) {
            print_summary(message);
            change_size = true;
            return;
        }

        // case where not running a scan
        if (size != last_data_length) {
            packets_received = 0;
            bytes_received = 0;
            missing_packets = 0;

            for (int i = 0; i < _num_publishers; i++) {
                _last_seq_num[i] = 0;
            }

            begin_time = perftest_cpp::GetTimeUsec();

            if (printIntervals) {
                std::cout << "\n\n********** New data length is "
                        << size + perftest_cpp::OVERHEAD_BYTES
                        << std::endl;

                fflush(stdout);
            }
        }

        last_data_length = message.size;
        ++packets_received;
        bytes_received += (unsigned long long) (message.size
                + perftest_cpp::OVERHEAD_BYTES);

        if (!_useCft) {
            // detect missing packets
            if (_last_seq_num[message.entity_id] == 0) {
                _last_seq_num[message.entity_id] = message.seq_num;
            } else {
                if (message.seq_num != ++_last_seq_num[message.entity_id]) {
                    // only track if skipped, might have restarted pub
                    if (message.seq_num > _last_seq_num[message.entity_id]) {
                        missing_packets += message.seq_num
                                - _last_seq_num[message.entity_id];
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
                    if (message.seq_num > _last_seq_num[message.entity_id]) {
                        missing_packets += message.seq_num
                                - _last_seq_num[message.entity_id];
                    }
                }
            }

            interval_time = now - begin_time;
            interval_packets_received = packets_received;
            interval_bytes_received = bytes_received;
            interval_missing_packets = missing_packets;
            interval_data_length = last_data_length;
            missing_packets_percent = 0.0;

            // Calculations of missing package percent
            if (interval_packets_received + interval_missing_packets != 0) {
                missing_packets_percent = (float) ((interval_missing_packets * 100.0)
                        / (float) (interval_packets_received
                        + interval_missing_packets));
            }

            std::string outputCpu = "";
            if (showCpu) {
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
 */
static void *ThroughputReadThread(void *arg) {

    ThroughputListener *listener = static_cast<ThroughputListener *>(arg);
    listener->_reader->ReceiveAndProccess(listener);

    return NULL;
}

/*********************************************************
 * Subscriber
 */
int perftest_cpp::RunSubscriber()
{

    ThroughputListener *reader_listener = NULL;
    IMessagingReader   *reader = NULL;
    IMessagingWriter   *writer = NULL;
    IMessagingWriter   *announcement_writer = NULL;

    // create latency pong writer
    writer = _MessagingImpl->CreateWriter(LATENCY_TOPIC_NAME);

    // Check if using callbacks or read thread
    if (!_PM.get<bool>("useReadThread")) {
        // create latency pong reader
        reader_listener = new ThroughputListener(
                _PM,
                writer,
                NULL,
                _PM.is_set("cft"),
                _PM.get<int>("numPublishers"));
        reader = _MessagingImpl->CreateReader(
                THROUGHPUT_TOPIC_NAME,
                reader_listener);
        if (reader == NULL) {
            std::cerr << "[Error] Problem creating throughput reader."
                      << std::endl;
            return -1;
        }
    } else {
        std::cerr << "[Info] Using reading thread." << std::endl;
        reader = _MessagingImpl->CreateReader(
                THROUGHPUT_TOPIC_NAME,
                NULL);
        if (reader == NULL) {
            std::cerr << "[Error] Problem creating throughput reader."
                      << std::endl;
            return -1;
        }
        reader_listener = new ThroughputListener(
                _PM,
                writer,
                reader,
                _PM.is_set("cft"),
                _PM.get<int>("numPublishers"));

        int threadPriority = RTI_OSAPI_THREAD_PRIORITY_DEFAULT;
        int threadOptions = RTI_OSAPI_THREAD_OPTION_DEFAULT;

        if (_threadPriorities.isSet) {
            threadPriority = _threadPriorities.receive;
            threadOptions = DDS_THREAD_SETTINGS_REALTIME_PRIORITY
                    | DDS_THREAD_SETTINGS_PRIORITY_ENFORCE;
        }

        struct RTIOsapiThread *receiverThread = NULL;
        receiverThread = RTIOsapiThread_new(
                "ReceiverThread",
                threadPriority,
                threadOptions,
                RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
                NULL,
                ThroughputReadThread,
                reader_listener);
        if (receiverThread == NULL) {
            std::cerr << "[Error] Problem creating ReceiverThread for ThroughputReadThread."
                    << std::endl;
            return -1;
        }

    }

    // Create announcement writer
    announcement_writer =
            _MessagingImpl->CreateWriter(ANNOUNCEMENT_TOPIC_NAME);

    // Synchronize with publishers
    std::cerr << "[Info] Waiting to discover "
              << _PM.get<int>("numPublishers")
              << " publishers ..."
              << std::endl;
    reader->waitForWriters(_PM.get<int>("numPublishers"));
    announcement_writer->waitForReaders(_PM.get<int>("numPublishers"));

    /*
     * Announcement message that will be used by the announcement_writer
     * to send information to the Publisher. This message size will indicate
     * different things.
     */
    TestMessage announcement_msg;
    announcement_msg.entity_id = subID;
    announcement_msg.size = INITIALIZE_SIZE;

    // Send announcement message
    announcement_writer->send(announcement_msg);

    announcement_writer->flush();
    std::cerr << "[Info] Waiting for data..." << std::endl;

    // wait for data
    unsigned long long prev_time = 0, now = 0, delta = 0;
    unsigned long long prev_count = 0;
    unsigned long long prev_bytes = 0;
    unsigned long long ave_count = 0;
    int last_data_length = -1;
    unsigned long long mps = 0, bps = 0;
    double mps_ave = 0.0, bps_ave = 0.0;
    unsigned long long msgsent, bytes, last_msgs, last_bytes;
    float missing_packets_percent = 0.0;

    if (showCpu) {
        reader_listener->cpu.initialize();
    }

    now = GetTimeUsec();

    while (true) {
        prev_time = now;
        MilliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        now = GetTimeUsec();

        if (reader_listener->change_size) { // ACK change_size
            announcement_msg.entity_id = subID;
            announcement_msg.size = LENGTH_CHANGED_SIZE;
            announcement_writer->send(announcement_msg);
            announcement_writer->flush();
            reader_listener->change_size = false;
        }

        if (reader_listener->end_test) { // ACK end_test
            announcement_msg.entity_id = subID;
            announcement_msg.size = FINISHED_SIZE;
            announcement_writer->send(announcement_msg);
            announcement_writer->flush();
            break;
        }

        if (printIntervals) {

            if (last_data_length != reader_listener->last_data_length) {
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
            bps_ave = bps_ave + (double) (bps - bps_ave) / (double) ave_count;
            mps_ave = mps_ave + (double) (mps - mps_ave) / (double) ave_count;

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
                if (showCpu) {
                    outputCpu = reader_listener->cpu.get_cpu_instant();
                }
                printf("Packets: %8llu  Packets/s: %7llu  Packets/s(ave): %7.0lf  "
                       "Mbps: %7.1lf  Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%) %s\n",
                        last_msgs, mps, mps_ave, bps * 8.0 / 1000.0 / 1000.0,
                        bps_ave * 8.0 / 1000.0 / 1000.0,
                        reader_listener->missing_packets,
                        missing_packets_percent,
                        outputCpu.c_str()
                );
                fflush(stdout);
            }

        }
    }

    perftest_cpp::MilliSleep(1000);

    if (announcement_writer != NULL) {
        delete(announcement_writer);
    }

    if (writer != NULL) {
        delete(writer);
    }

    if (reader != NULL) {
        reader->Shutdown();
        delete(reader);
    }

    if (reader_listener != NULL) {
        delete(reader_listener);
    }

    std::cerr << "[Info] Finishing test..." <<std::endl;
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

    AnnouncementListener() {}

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

    ParameterManager *_PM;
    int  subID;
    bool printIntervals;
    bool showCpu;

 public:
    IMessagingReader *_reader;
    CpuMonitor cpu;

    LatencyListener(
            unsigned int num_latency,
            IMessagingReader *reader,
            IMessagingWriter *writer,
            ParameterManager &PM)
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
                std::cerr << "[Error]: LatencyListener: Not able to allocate "
                          << _num_latency
                          << " elements in _latency_history array"
                          << std::endl;
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

        subID = _PM->get<int>("sidMultiSubTest");
        printIntervals = !_PM->get<bool>("noPrintIntervals");
        showCpu = _PM->get<bool>("cpu");
    }

    ~LatencyListener()
    {
        if (_latency_history != NULL) {
            delete [] _latency_history;
        }

        if (_reader != NULL) {
            _reader->Shutdown();
            delete(_reader);
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

        if (now >= sentTime) {
            latency = (unsigned long)(now - sentTime);

            // keep track of one-way latency
            latency /= 2;
        } else {
            fprintf(stderr,
                    "Clock skew suspected: received time %llu usec, "
                    "sent time %llu usec\n",
                    now, sentTime);
            ++clock_skew_count;
            return;
        }

        // store value for percentile calculations
        if (_latency_history != NULL) {
            if (count >= _num_latency){
                fprintf(stderr,
                        "Too many latency pongs received.  Do you have more "
                        "than 1 app with -pidMultiPubTest = 0 or "
                        "-sidMultiSubTest 0?\n");
                return;
            } else {
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
        if (last_data_length != message.size) {
            last_data_length = message.size;

            if (printIntervals) {
                printf("\n\n********** New data length is %d\n",
                       last_data_length + perftest_cpp::OVERHEAD_BYTES);
            }
        } else {
            if (printIntervals) {
                latency_ave = (double)latency_sum / (double)count;
                latency_std = sqrt(
                    (double)latency_sum_square / (double)count - (latency_ave * latency_ave));

                if (showCpu) {
                    outputCpu = cpu.get_cpu_instant();
                }
                printf("One way Latency: %6lu us  Ave %6.0lf us  Std %6.1lf us "
                        " Min %6lu us  Max %6lu %s\n",
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

    void print_summary_latency(){
        double latency_ave;
        double latency_std;
        std::string outputCpu = "";

        if (count == 0) {
            return;
        }

        if (clock_skew_count != 0) {
            fprintf(stderr,
                    "The following latency result may not be accurate because "
                    "clock skew happens %lu times\n",
                    clock_skew_count);
            fflush(stderr);
        }

        // sort the array (in ascending order)
        std::sort(_latency_history, _latency_history + count);
        latency_ave = (double)latency_sum / count;
        latency_std = sqrt(
                (double)latency_sum_square
                / (double)count - (latency_ave * latency_ave));

        if (showCpu) {
            outputCpu = cpu.get_cpu_average();
        }

        printf("Length: %5d  Latency: Ave %6.0lf us  Std %6.1lf us  "
                "Min %6lu us  Max %6lu us  50%% %6lu us  90%% %6lu us"
                "  99%% %6lu us  99.99%% %6lu us  99.9999%% %6lu us %s\n",
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
    }
};

/*********************************************************
 * Used for receiving data using a thread instead of callback
 *
 */
static void *LatencyReadThread(void *arg)
{
    LatencyListener *listener = static_cast<LatencyListener *>(arg);
    TestMessage *message;

    while (!listener->end_test)
    {
        // Receive message should block until a message is received
        message = listener->_reader->ReceiveMessage();

        if (message != NULL)
        {
            listener->ProcessMessage(*message);
        }

    /*
    * TODO: -latencyTest plus -useReadThread
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
int perftest_cpp::RunPublisher()
{
    LatencyListener  *reader_listener = NULL;
    IMessagingReader *reader;
    IMessagingWriter *writer;
    AnnouncementListener  *announcement_reader_listener = NULL;
    IMessagingReader *announcement_reader;
    unsigned long num_latency;
    unsigned long announcementSampleCount = 50;
    unsigned int samplesPerBatch = GetSamplesPerBatch();
    struct RTIOsapiThread *executionTimeoutThread = NULL;

    // create throughput/ping writer
    writer = _MessagingImpl->CreateWriter(THROUGHPUT_TOPIC_NAME);

    // Calculate number of latency pings that will be sent per data size
    num_latency = (unsigned long)((_PM.get<unsigned long long>("numIter") /
            samplesPerBatch) / _PM.get<unsigned long long>("latencyCount"));
    if ((_PM.get<unsigned long long>("numIter") / samplesPerBatch) %
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
                    _PM);
            reader = _MessagingImpl->CreateReader(
                    LATENCY_TOPIC_NAME,
                    reader_listener);
            if (reader == NULL) {
                std::cerr << "[Error] Problem creating latency reader."
                        << std::endl;
                return -1;
            }
        } else {

            std::cerr << "[Debug] Using Read Thread." << std::endl;
            reader = _MessagingImpl->CreateReader(
                    LATENCY_TOPIC_NAME,
                    NULL);
            if (reader == NULL) {
                std::cerr << "[Error] Problem creating latency reader."
                        << std::endl;
                return -1;
            }
            reader_listener = new LatencyListener(
                    num_latency,
                    reader,
                    _PM.get<bool>("latencyTest") ? writer : NULL,
                    _PM);

            int threadPriority = RTI_OSAPI_THREAD_PRIORITY_DEFAULT;
            int threadOptions = RTI_OSAPI_THREAD_OPTION_DEFAULT;

            if (_threadPriorities.isSet) {
                threadPriority = _threadPriorities.receive;
                threadOptions = DDS_THREAD_SETTINGS_REALTIME_PRIORITY
                        | DDS_THREAD_SETTINGS_PRIORITY_ENFORCE;
            }

            struct RTIOsapiThread *receiverThread = NULL;
            receiverThread = RTIOsapiThread_new(
                    "ReceiverThread",
                    threadPriority,
                    threadOptions,
                    RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
                    NULL,
                    LatencyReadThread,
                    reader_listener);
            if (receiverThread == NULL) {
                std::cerr << "[Error] Problem creating ReceiverThread for LatencyReadThread."
                        << std::endl;
                return -1;
            }
        }
    } else {
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
    if (announcement_reader == NULL) {
        std::cerr << "[Error] Problem creating announcement reader." << std::endl;
        return -1;
    }

    unsigned long long spinPerUsec = 0;
    unsigned long sleepUsec = 1000;
    if (_PM.is_set("pubRate")) {
        if ( _PM.get_pair<unsigned long long, std::string>("pubRate").second
                == "spin") {
            spinPerUsec = rti::util::spin_per_microsecond();
            /* A return value of 0 means accuracy not assured */
            if (spinPerUsec == 0) {
                std::cerr << "[Error] Error initializing spin per microsecond. "
                          << "'-pubRate' cannot be used\nExiting..."
                          << std::endl;
                return -1;
            }
            _SpinLoopCount = 1000000 * spinPerUsec /
                    _PM.get_pair<unsigned long long, std::string>("pubRate").first;
        } else { // sleep count
            _SleepNanosec = 1000000000/
                    (unsigned long)_PM.get_pair
                            <unsigned long long, std::string>("pubRate").first;
        }
    }

    std::cerr << "[Info] Waiting to discover "
              << _PM.get<int>("numSubscribers")
              << " subscribers ..."
              << std::endl;
    writer->waitForReaders(_PM.get<int>("numSubscribers"));

    // We have to wait until every Subscriber sends an announcement message
    // indicating that it has discovered every RunPublisher
    std::cerr << "[Info] Waiting for subscribers announcement ..." << std::endl;
    while (_PM.get<int>("numSubscribers")
            > (int)announcement_reader_listener->subscriber_list.size()) {
        MilliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
    }

    // Allocate data and set size
    TestMessage message;
    message.entity_id = _PM.get<int>("pidMultiPubTest");
    //message.size = (std::max)(_PM.get<unsigned long long>("dataLen"), LENGTH_CHANGED_SIZE);
    //message.bin_data.resize((std::max)(_PM.get<unsigned long long>("dataLen"), LENGTH_CHANGED_SIZE));


    if ( showCpu && _PM.get<int>("pidMultiPubTest") == 0) {
        reader_listener->cpu.initialize();
    }

    if (INITIALIZE_SIZE > MAX_PERFTEST_SAMPLE_SIZE) {
        std::cerr << "[Error] INITIALIZE_SIZE > MAX_PERFTEST_SAMPLE_SIZE" << std::endl;
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
            (unsigned long)_PM.get<long>("instances"));

    std::cerr << "[Info] Sending " << initializeSampleCount
              << " initialization pings ..." << std::endl;

    for (unsigned long i = 0; i < initializeSampleCount; i++) {
        // Send test initialization message
        writer->send(message, true);
    }
    writer->flush();

    std::cerr << "[Info] Publishing data ..." << std::endl;

    // Set data size, account for other bytes in message
    message.size = (int)_PM.get<unsigned long long>("dataLen") - OVERHEAD_BYTES;
    //message.data.resize(message.size);

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

    struct ScheduleInfo schedInfo = {
            (unsigned int)_PM.get<unsigned long long>("executionTime"),
            Timeout
    };

    time_last_check = perftest_cpp::GetTimeUsec();

    /* Minimum value for spin_sample_period will be 1 so we execute 100 times
       the control loop every second, or every sample if we want to send less
       than 100 samples per second */
    if (_PM.get_pair<unsigned long long, std::string>("pubRate").first > 100) {
        pubRate_sample_period =
                (unsigned long)(_PM.get_pair<unsigned long long, std::string>(
                    "pubRate").first
                / 100);
    }

    if (_PM.get<unsigned long long>("executionTime") > 0
            && !_PM.is_set("scan")) {
        executionTimeoutThread = SetTimeout(schedInfo);
        if (executionTimeoutThread == NULL) {
            std::cerr << "[Error] Problem creating timeoutThread for executionTime."
                    << std::endl;
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
     * - writerStats
     * - isScan
     * - scanList
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
    const bool writerStats = _PM.get<bool>("writerStats");
    const bool isScan = _PM.is_set("scan");
    const std::vector<unsigned long long> scanList =
            _PM.get_vector<unsigned long long>("scan");
    const bool isSetPubRate = _PM.is_set("pubRate");

    struct ScheduleInfo schedInfo_scan = {
            (unsigned int)_PM.get<unsigned long long>("executionTime"),
            Timeout_scan
    };

    /********************
     *  Main sending loop
     */
    for (unsigned long long loop = 0;
            ((isScan) || (loop < numIter)) && (!_testCompleted);
            ++loop ) {

        /* This if has been included to perform the control loop
           that modifies the publication rate according to -pubRate */
        if (isSetPubRate && (loop > 0) && (loop % pubRate_sample_period == 0)) {

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
                    _SleepNanosec += sleepUsec; //plus 1 MicroSec
                } else if (rate < pubRate && _SleepNanosec > sleepUsec) {
                    _SleepNanosec -=  sleepUsec; //less 1 MicroSec
                } else if (rate < pubRate && _SleepNanosec <= sleepUsec) {
                    _SleepNanosec = 0;
                }
            }
        }

        if (_SpinLoopCount > 0) {
            rti::util::spin(_SpinLoopCount);
        }

        if (_SleepNanosec > 0) {
            rti::util::sleep(dds::core::Duration(0,(unsigned int)_SleepNanosec));
        }

        pingID = -1;

        // only send latency pings if is publisher with ID 0
        // In batch mode, latency pings are sent once every LatencyCount batches
        if ((pidMultiPubTest == 0) && (((loop / samplesPerBatch)
                % latencyCount) == 0) ) {

            /* In batch mode only send a single ping in a batch.
             *
             * However, the ping is sent in a round robin position within
             * the batch.  So keep track of which position(index) the
             * current sample is within the batch, and which position
             * within the batch should contain the ping. Only send the ping
             * when both are equal.
             *
             * Note when not in batch mode, current_index_in_batch = ping_index_in_batch
             * always. And the if() is always true.
             */
            if ( current_index_in_batch == ping_index_in_batch  && !sentPing) {
                // If running in scan mode, dataLen under test is changed
                // after _executionTime
                if (isScan && _testCompleted_scan) {
                    _testCompleted_scan = false;
                    executionTimeoutThread = SetTimeout(schedInfo_scan);
                    if (executionTimeoutThread == NULL) {
                        std::cerr << "[Error] Problem creating timeoutThread for executionTime."
                                << std::endl;
                        return -1;
                    }

                    // flush anything that was previously sent
                    writer->flush();
                    writer->waitForAck(
                        timeout_wait_for_ack_sec,
                        timeout_wait_for_ack_nsec);

                    if (scan_count == scanList.size()) {
                        break; // End of scan test
                    }


                    message.size = LENGTH_CHANGED_SIZE;
                    //message.data.resize(message.size);
                    // must set latency_ping so that a subscriber sends us
                    // back the LENGTH_CHANGED_SIZE message
                    message.latency_ping = num_pings % numSubscribers;

                    /*
                     * If the Throughput topic is reliable, we can send the packet and do
                     * a wait for acknowledgements. However, if the Throughput topic is
                     * Best Effort, waitForAck() will return inmediately.
                     * This would cause that the Send() would be exercised too many times,
                     * in some cases causing the network to be flooded, a lot of packets being
                     * lost, and potentially CPU starbation for other processes.
                     * We can prevent this by adding a small sleep() if the test is best
                     * effort.
                     */
                    announcement_reader_listener->subscriber_list.clear();
                    while ((int)announcement_reader_listener->subscriber_list.size()
                            < numSubscribers) {
                        writer->send(message, true);
                        writer->flush();
                        writer->waitForAck(
                            timeout_wait_for_ack_sec,
                            timeout_wait_for_ack_nsec);
                    }

                    message.size = (int)scanList[scan_count++] - OVERHEAD_BYTES;
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

                if (writerStats && printIntervals) {
                    printf("Pulled samples: %7d\n",
                            writer->getPulledSampleCount());
                }
            }
        }
        current_index_in_batch = (current_index_in_batch + 1) % samplesPerBatch;

        message.seq_num = (unsigned long) loop;
        message.latency_ping = pingID;
        writer->send(message);
        if(latencyTest && sentPing) {
            if (!bestEffort) {
                writer->waitForPingResponse();
            } else {
                /* time out in milliseconds */
                writer->waitForPingResponse(200);
            }
        }


        // come to the beginning of another batch
        if (current_index_in_batch == 0) {
            sentPing = false;
        }
    }

    // In case of batching, flush
    writer->flush();

    // Test has finished, send end of test message, send multiple
    // times in case of best effort
    if (FINISHED_SIZE > MAX_PERFTEST_SAMPLE_SIZE) {
        std::cerr << "[Error]  FINISHED_SIZE < MAX_PERFTEST_SAMPLE_SIZE" << std::endl;
        return -1;
    }

    message.size = FINISHED_SIZE;
    message.data.resize(message.size);
    unsigned long i = 0;
    while (announcement_reader_listener->subscriber_list.size() > 0
            && i < announcementSampleCount) {
        writer->send(message, true);
        writer->flush();
        writer->waitForAck(
            timeout_wait_for_ack_sec,
            timeout_wait_for_ack_nsec);
        i++;
    }

    if (_PM.get<int>("pidMultiPubTest") == 0) {
        reader_listener->print_summary_latency();
        reader_listener->end_test = true;
    } else {
        std::cerr << "[Info] Latency results are only shown when "
                  << "-pidMultiPubTest = 0" << std::endl;
    }

    if (_PM.get<bool>("writerStats")) {
        printf("Pulled samples: %7d\n", writer->getPulledSampleCount());
    }

    if (writer != NULL) {
        delete(writer);
    }

    if (announcement_reader_listener != NULL) {
        delete(announcement_reader_listener);
    }

    if (reader_listener != NULL) {
        delete(reader_listener);
    }

    if (reader != NULL) {
        reader->Shutdown();
        delete(reader);
    }

    if (announcement_reader != NULL) {
        announcement_reader->Shutdown();
        delete(announcement_reader);
    }


    if (_testCompleted) {
        // Delete timeout thread
        if (executionTimeoutThread != NULL) {
            RTIOsapiThread_delete(executionTimeoutThread);
        }

        std::cerr <<  "[Info] Finishing test due to timer..." << std::endl;
    } else {
        std::cerr <<  "[Info] Finishing test..." << std::endl;
    }



    return 0;
}

/*********************************************************
 * Utility functions
 */

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

void *perftest_cpp::waitAndExecute(void *scheduleInfo) {
    ScheduleInfo *info = static_cast<ScheduleInfo *>(scheduleInfo);

    perftest_cpp::MilliSleep(info->timer * 1000u);
    info->handlerFunction();

    return NULL;
}

inline RTIOsapiThread *perftest_cpp::SetTimeout(ScheduleInfo &info) {
    struct RTIOsapiThread *timerThread = NULL;

    timerThread = RTIOsapiThread_new(
            "timerThread",
            RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
            RTI_OSAPI_THREAD_OPTION_DEFAULT,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
            NULL,
            waitAndExecute,
            &info);

    return timerThread;
}

inline unsigned int perftest_cpp::GetSamplesPerBatch() {
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

inline void perftest_cpp::Timeout() {
    _testCompleted = true;
}
inline void perftest_cpp::Timeout_scan() {
    _testCompleted_scan = true;
}