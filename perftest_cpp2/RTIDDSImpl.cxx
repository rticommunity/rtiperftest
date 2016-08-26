/* ===================================================================
 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 Permission to modify and use for internal purposes granted.
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 5.2.0,21jul15,jm  PERF-53 Changes for CR-789.
 5.2.0,09may15,jm  PERFTEST-86 Reader's max instances not modified now, set to
                   DDS_LENGTH_UNLIMITED via perftest.xml.
 5.2.0,27apr14,jm  PERFTEST-86 Removing .ini support. Fixing warnings.
 5.2.0,03nov14,jm  PERF-53 Created. Using ../perftest_cpp as template for the
                   Product behavior.
===================================================================== */

#include "RTIDDSImpl.h"
#include "perftest_cpp.h"


#ifdef RTI_WIN32
#pragma warning(push)
#pragma warning(disable : 4996)
#define STRNCASECMP     _strnicmp
#else
#define STRNCASECMP     strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

template <typename T>
RTIDDSImpl<T>::RTIDDSImpl():
        _SendQueueSize(50),
        _DataLen(100),
        _DomainID(1),
        _Nic(""),
        _ProfileFile("perftest.xml"),
        _TurboMode(false),
        _AutoThrottle(false),
        _IsReliable(true),
        _IsMulticast(false),
        _BatchSize(0),
        _InstanceCount(1),
        _InstanceMaxCountReader(dds::core::LENGTH_UNLIMITED), //(-1)
        _InstanceHashBuckets(dds::core::LENGTH_UNLIMITED), //(-1)
        _Durability(0), // DDS_VOLATILE_DURABILITY_QOS;
        _DirectCommunication(true),
        _KeepDurationUsec(1000),
        _UsePositiveAcks(true),
        _UseSharedMemory(false),
        _LatencyTest(false),
        _UseTcpOnly(false),
        _IsDebug(false),
        _isLargeData(false),
        _isScan(false),
        _WaitsetEventCount(5),
        _WaitsetDelayUsec(100),
        _HeartbeatPeriod(dds::core::Duration::zero()),
        _FastHeartbeatPeriod(dds::core::Duration::zero()),
        THROUGHPUT_MULTICAST_ADDR("239.255.1.1"),
        LATENCY_MULTICAST_ADDR("239.255.1.2"),
        ANNOUNCEMENT_MULTICAST_ADDR("239.255.1.100"),
        _ProfileLibraryName("PerftestQosLibrary"),
        _participant(dds::core::null),
        _subscriber(dds::core::null),
        _publisher(dds::core::null),
        _reader(dds::core::null),
        _pongSemaphore(RTI_OSAPI_SEMAPHORE_KIND_BINARY,NULL)
    {}



/*********************************************************
 * Shutdown
 */
template <typename T>
void RTIDDSImpl<T>::Shutdown()
{
    /* RTI Connext provides finalize_instance() method on
     * domain participant factory for people who want to release memory used
     * by the participant factory. Uncomment the following block of code for
     * clean destruction of the singleton. */
    /* _participant.finalize_participant_factory(); */

}

/*********************************************************
 * PrintCmdLineHelp
 */
template <typename T>
void RTIDDSImpl<T>::PrintCmdLineHelp() {
    std::string usage_string =
    "\t-sendQueueSize <number> - Sets number of samples (or batches) in send\n"
    "\t                          queue, default 50\n"
    "\t-domain <ID>            - RTI DDS Domain, default 1\n"
    "\t-qosprofile <filename>  - Name of XML file for DDS Qos profiles, \n"
    "\t                          default perftest.xml\n"
    "\t-nic <ipaddr>           - Use only the nic specified by <ipaddr>.\n"
    "\t                          If unspecificed, use all available interfaces\n"
    "\t-multicast              - Use multicast to send data, default not to\n"
    "\t                          use multicast\n"
    "\t-nomulticast            - Do not use multicast to send data (default)\n"
    "\t-multicastAddress <ipaddr>   - Multicast address to use for receiving \n"
    "\t                          latency/announcement (pub) or \n"
    "\t                          throughtput(sub) data.\n"
    "\t                          If unspecified: latency 239.255.1.2,\n"
    "\t                                          announcement 239.255.1.100,\n"
    "\t                                          throughput 239.255.1.1\n"
    "\t-bestEffort             - Run test in best effort mode, default reliable\n"
    "\t-batchSize <bytes>      - Size in bytes of batched message, default 0\n"
    "\t                          (no batching)\n"
    "\t-noPositiveAcks         - Disable use of positive acks in reliable \n"
    "\t                          protocol, default use positive acks\n"
    "\t-keepDurationUsec <usec> - Minimum time (us) to keep samples when\n"
    "\t                          positive acks are disabled, default 1000 us\n"
    "\t-enableSharedMemory     - Enable use of shared memory transport and \n"
    "\t                          disable all the other transports, default\n"
    "\t                          shared memory not enabled\n"
    "\t-enableTcpOnly          - Enable use of TCP transport and disable all\n"
    "\t                          the other transports, default do not use\n"
    "\t                          tcp transport\n"
    "\t-heartbeatPeriod <sec>:<nanosec>     - Sets the regular heartbeat period\n"
    "\t                          for throughput DataWriter, default 0:0\n"
    "\t                          (use XML QoS Profile value)\n"
    "\t-fastHeartbeatPeriod <sec>:<nanosec> - Sets the fast heartbeat period\n"
    "\t                          for the throughput DataWriter, default 0:0\n"
    "\t                          (use XML QoS Profile value)\n"
    "\t-durability <0|1|2|3>   - Set durability QOS, 0 - volatile,\n"
    "\t                          1 - transient local, 2 - transient, \n"
    "\t                          3 - persistent, default 0\n"
    "\t-noDirectCommunication  - Use brokered mode for persistent durability\n"
    "\t-instanceHashBuckets <#count> - Number of hash buckets for instances.\n"
    "\t                          If unspecified, same as number of\n"
    "\t                          instances.\n"
    "\t-waitsetDelayUsec <usec>  - UseReadThread related. Allows you to\n"
    "\t                          process incoming data in groups, based on the\n"
    "\t                          time rather than individually. It can be used\n"
    "\t                          combined with -waitsetEventCount,\n"
    "\t                          default 100 usec\n"
    "\t-waitsetEventCount <count> - UseReadThread related. Allows you to\n"
    "\t                          process incoming data in groups, based on the\n"
    "\t                          number of samples rather than individually. It\n"
    "\t                          can be used combined with -waitsetDelayUsec,\n"
    "\t                          default 5\n"
    "\t-enableAutoThrottle     - Enables the AutoThrottling feature in the\n"
    "\t                          throughput DataWriter (pub)\n"
    "\t-enableTurboMode        - Enables the TurboMode feature in the\n"
    "\t                          throughput DataWriter (pub)\n";

    std::cerr << usage_string << std::endl;
}

/*********************************************************
 * ParseConfig
 */
template <typename T>
bool RTIDDSImpl<T>::ParseConfig(int argc, char *argv[])
{
    int i;
    int sec = 0;
    unsigned int nanosec = 0;

    // TODO IMPLEMENT CONFIGFILE

    // now load everything else, command line params override config file
    for (i = 0; i < argc; ++i) {
        if (IS_OPTION(argv[i], "-scan")) {
            _isScan = true;

        } else if (IS_OPTION(argv[i], "-dataLen")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <length> after -dataLen\n"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < perftest_cpp::OVERHEAD_BYTES) {
                std::cerr << "[Error] -dataLen must be >= "
                        << perftest_cpp::OVERHEAD_BYTES << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (_DataLen > TestMessage::MAX_DATA_SIZE) {
                std::cerr << "[Error] -dataLen must be <= "
                        << TestMessage::MAX_DATA_SIZE << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (_DataLen > MAX_BINDATA_SIZE) {
                std::cerr << "[Error] -dataLen must be <= " << MAX_BINDATA_SIZE
                << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-sendQueueSize")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <count> after -sendQueueSize"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _SendQueueSize = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-heartbeatPeriod")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <period> after -heartbeatPeriod"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i], "%d:%d", &sec, &nanosec) != 2) {
                std::cerr
                        << "[Error] -heartbeatPeriod value must have the format <sec>:<nanosec>"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (sec > 0 || nanosec > 0) {
                _HeartbeatPeriod = dds::core::Duration(sec, nanosec);
            }
        } else if (IS_OPTION(argv[i], "-fastHeartbeatPeriod")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr
                        << "[Error] Missing <period> after -fastHeartbeatPeriod"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i], "%d:%d", &sec, &nanosec) != 2) {
                std::cerr
                        << "[Error] -fastHeartbeatPeriod value must have the format <sec>:<nanosec>"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (sec > 0 || nanosec > 0) {
                _FastHeartbeatPeriod = dds::core::Duration(sec, nanosec);
            }
        } else if (IS_OPTION(argv[i], "-domain")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <id> after -domain" << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _DomainID = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-qosprofile")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <filename> after -qosprofile"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _ProfileFile = argv[i];
        } else if (IS_OPTION(argv[i], "-multicast")) {
            _IsMulticast = true;
        } else if (IS_OPTION(argv[i], "-nomulticast")) {
            _IsMulticast = false;
        } else if (IS_OPTION(argv[i], "-multicastAddress")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr
                        << "[Error] Missing <multicast address> after -multicastAddress"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            THROUGHPUT_MULTICAST_ADDR = argv[i];
            LATENCY_MULTICAST_ADDR = argv[i];
            ANNOUNCEMENT_MULTICAST_ADDR = argv[i];
        } else if (IS_OPTION(argv[i], "-nic")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <address> after -nic"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _Nic = argv[i];
        } else if (IS_OPTION(argv[i], "-bestEffort")) {
            _IsReliable = false;
        } else if (IS_OPTION(argv[i], "-durability")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <kind> after -durability"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _Durability = strtol(argv[i], NULL, 10);

            if ((_Durability < 0) || (_Durability > 3)) {
                std::cerr << "[Error] durability kind must be 0(volatile), "
                        "1(transient local), 2(transient), or 3(persistent)."
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-noDirectCommunication")) {
            _DirectCommunication = false;
        } else if (IS_OPTION(argv[i], "-instances")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <count> after -instances"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _InstanceCount = strtol(argv[i], NULL, 10);
            _InstanceMaxCountReader = _InstanceCount;

            if (_InstanceCount <= 0) {
                std::cerr << "[Error] Instance count cannot be negative or zero"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-instanceHashBuckets")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr
                        << "[Error] Missing <count> after -instanceHashBuckets"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _InstanceHashBuckets = strtol(argv[i], NULL, 10);

            if (_InstanceHashBuckets <= 0 && _InstanceHashBuckets != -1) {
                std::cerr
                        << "[Error] Instance hash buckets cannot be negative or zero"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-batchSize")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <#bytes> after -batchSize"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _BatchSize = strtol(argv[i], NULL, 10);

            if (_BatchSize < 0) {
                std::cerr << "[Error] Batch size cannot be negative"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-keepDurationUsec")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <usec> after -keepDurationUsec"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _KeepDurationUsec = strtol(argv[i], NULL, 10);
            if (_KeepDurationUsec < 0) {
                std::cerr << "[Error] Keep duration usec cannot be negative"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-noPositiveAcks")) {
            _UsePositiveAcks = false;
        } else if (IS_OPTION(argv[i], "-enableSharedMemory")) {
            _UseSharedMemory = true;
        } else if (IS_OPTION(argv[i], "-enableTcpOnly")) {
            _UseTcpOnly = true;
        }

        else if (IS_OPTION(argv[i], "-debug")) {
            rti::config::Logger::instance().verbosity(
                    rti::config::Verbosity::STATUS_ALL);
        } else if (IS_OPTION(argv[i], "-waitsetDelayUsec")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "Missing <usec> after -waitsetDelayUsec"
                        << std::endl;
                ;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _WaitsetDelayUsec = (unsigned int) strtol(argv[i], NULL, 10);
            if (_WaitsetDelayUsec < 0) {
                std::cerr << "waitset delay usec cannot be negative"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-waitsetEventCount")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "Missing <count> after -waitsetEventCount"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _WaitsetEventCount = strtol(argv[i], NULL, 10);
            if (_WaitsetEventCount < 0) {
                std::cerr << "waitset event count cannot be negative"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-latencyTest")) {
            _LatencyTest = true;
        } else if (IS_OPTION(argv[i], "-enableAutoThrottle")) {
            std::cerr << "[Info] Auto Throttling enabled. Automatically "
                    "adjusting the DataWriter\'s writing rate" << std::endl;
            _AutoThrottle = true;
        } else if (IS_OPTION(argv[i], "-enableTurboMode")) {
            _TurboMode = true;
        } else if (IS_OPTION(argv[i], "-configFile")) {
            /* Ignore config file */
            ++i;
        } else {
            if (i > 0) {
                std::cerr << argv[i] << " not recognized" << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        }
    }

    if (_DataLen > TestMessage::MAX_SYNCHRONOUS_SIZE) {
        if (_isScan) {
            std::cerr << "[Info] DataLen will be ignored since -scan is present."
                    << std::endl;
        } else {
            std::cerr << "[Info] Large data settings enabled (-dataLen > "
                    << TestMessage::MAX_SYNCHRONOUS_SIZE << ")." << std::endl;
            _isLargeData = true;
        }
    }
    if (_isLargeData && _BatchSize > 0) {
        std::cerr << "[Error] Batch cannot be enabled if using large data"
                << std::endl;
        throw std::logic_error("[Error] Error parsing commands");
    }

    if (_isLargeData && _TurboMode) {
        std::cerr << "[Error] Turbo Mode cannot be used with asynchronous writing. "
                "It will be ignored."  << std::endl;
        throw std::logic_error("[Error] Error parsing commands");
    }

    return true;
}

/*********************************************************
 * DomainListener
 */
class DomainListener : public dds::domain::NoOpDomainParticipantListener
{

public:
    void on_inconsistent_topic(dds::topic::AnyTopic& topic,
            const dds::core::status::InconsistentTopicStatus& /*status*/) {
        std::cerr << "Found inconsistent topic. Expecting " << topic.name()
                << " of type " << topic.type_name() << std::endl;
    }

    void on_offered_incompatible_qos(dds::pub::AnyDataWriter& writer,
            const ::dds::core::status::OfferedIncompatibleQosStatus& status) {
        std::cerr << "Found incompatible reader for writer "
                << writer.topic_name() << " QoS is " << status.last_policy_id()
                << std::endl;
    }

    void on_requested_incompatible_qos(dds::sub::AnyDataReader& reader,
            const dds::core::status::RequestedIncompatibleQosStatus& status) {
        std::cerr << "Found incompatible writer for reader "
                << reader.topic_name() << " QoS is " << status.last_policy_id()
                << std::endl;
    }
};

/*********************************************************
 * RTIPublisher
 */
template<typename T>
class RTIPublisher: public IMessagingWriter {
private:
    dds::pub::DataWriter<T> _writer;
    T data;
    int _num_instances;
    unsigned long _instance_counter;
    dds::core::InstanceHandleSeq _instance_handles;
    bool _useSemaphore;
    rti::core::Semaphore& _pongSemaphore;

public:
    RTIPublisher(dds::pub::DataWriter<T> writer, int num_instances,
            rti::core::Semaphore& pongSemaphore, bool useSemaphore) :
            _writer(writer), _num_instances(num_instances), _instance_counter(
                    0), _useSemaphore(useSemaphore), _pongSemaphore(
                    pongSemaphore) {

        for (int i = 0; i < _num_instances; ++i) {
            data.key()[0] = (char) (i);
            data.key()[1] = (char) (i >> 8);
            data.key()[2] = (char) (i >> 16);
            data.key()[3] = (char) (i >> 24);

            _instance_handles.push_back(_writer.register_instance(data));
        }
    }

    void Flush() {
        _writer->flush();
    }

    bool Send(TestMessage &message) {
        data.entity_id(message.entity_id);
        data.seq_num(message.seq_num);
        data.timestamp_sec(message.timestamp_sec);
        data.timestamp_usec(message.timestamp_usec);
        data.latency_ping(message.latency_ping);

        data.bin_data().resize(message.size);
        //data.bin_data(message.data);

        int key = 0;
        if (_num_instances > 1) {
            key = _instance_counter++ % _num_instances;
            data.key()[0] = (char) (key);
            data.key()[1] = (char) (key >> 8);
            data.key()[2] = (char) (key >> 16);
            data.key()[3] = (char) (key >> 24);
        }
        _writer.write(data, _instance_handles[key]);
        return true;
    }

    void waitForReaders(int numSubscribers) {
        while (_writer.publication_matched_status().current_count()
                < numSubscribers) {
            perftest_cpp::MilliSleep(1000);
        }
    }

    void waitForPingResponse() {
        if (_useSemaphore) {
            _pongSemaphore.take();
        }
    }

    /* time out in milliseconds */
    void waitForPingResponse(int timeout) {
        RTINtpTime blockDurationIn;
        blockDurationIn.sec = timeout;
        blockDurationIn.frac = 0;

        if (_useSemaphore) {
            _pongSemaphore.take(&blockDurationIn);
        }
    }

    void notifyPingResponse() {
        if (_useSemaphore) {
            _pongSemaphore.give();
        }
    }
};

/*********************************************************
 * ReceiverListener
 */
template<typename T>
class ReceiverListener: public dds::sub::NoOpDataReaderListener<T> {
private:
    TestMessage _message;
    IMessagingCB *_callback;
    dds::sub::LoanedSamples<T> samples;
public:

    ReceiverListener(IMessagingCB *callback) :
            _callback(callback) {
    }

    void on_data_available(dds::sub::DataReader<T> &reader) {
        samples = reader.take();

        for (unsigned int i = 0; i < samples.length(); ++i) {
            if (samples[i].info().valid()) {
                const T & sample = samples[i].data();
                _message.entity_id = sample.entity_id();
                _message.seq_num = sample.seq_num();
                _message.timestamp_sec = sample.timestamp_sec();
                _message.timestamp_usec = sample.timestamp_usec();
                _message.latency_ping = sample.latency_ping();
                _message.size = sample.bin_data().size();
                //_message.data = sample.bin_data();
                _callback->ProcessMessage(_message);
            }
        }
    }
};

/*********************************************************
 * RTISubscriber
 */
template<typename T>
class RTISubscriber: public IMessagingReader {
private:
    dds::sub::DataReader<T> _reader;
    TestMessage _message;
    dds::core::cond::WaitSet _waitset;
    int _data_idx;
    bool _no_data;

public:

    RTISubscriber(dds::sub::DataReader<T> reader, int _WaitsetEventCount,
            unsigned int _WaitsetDelayUsec) :
            _reader(reader), _waitset(
                    rti::core::cond::WaitSetProperty(_WaitsetEventCount,
                            dds::core::Duration::from_microsecs(
                                    _WaitsetDelayUsec))) {
        // null listener means using receive thread
        if (_reader.listener() == NULL) {

            // Using status conditions:
            dds::core::cond::StatusCondition reader_status(_reader);
            reader_status.enabled_statuses(
                    dds::core::status::StatusMask::data_available());
            _waitset += reader_status;

            /* Uncomment these lines and comment previous ones to use Read
             * conditions instead of status conditions
             *
             * dds::sub::cond::ReadCondition read_condition(_reader,
             * dds::sub::status::DataState::any_data());
             * _waitset += read_condition; */
        }

        _no_data = true;
        _data_idx = 0;
    }

    void Shutdown() {
    }

    TestMessage *ReceiveMessage() {

        int seq_length;

        while (true) {

            if (_no_data) {
                _waitset.wait(dds::core::Duration::infinite());

            }
            dds::sub::LoanedSamples<T> samples = _reader.take();

            _data_idx = 0;
            _no_data = false;

            seq_length = samples.length();
            if (_data_idx == seq_length) {
                _no_data = true;
                continue;
            }

            // skip non-valid data
            while ((!samples[_data_idx].info().valid())
                    && (++_data_idx < seq_length))
                ;

            // may have hit end condition
            if (_data_idx == seq_length) {
                continue;
            }

            const T& data = samples[_data_idx].data();
            _message.entity_id = data.entity_id();
            _message.seq_num = data.seq_num();
            _message.timestamp_sec = data.timestamp_sec();
            _message.timestamp_usec = data.timestamp_usec();
            _message.latency_ping = data.latency_ping();
            _message.size = data.bin_data().size();
            //_message.data = samples[_data_idx].data().bin_data();

            ++_data_idx;

            return &_message;
        }
        return NULL;
    }

    void ReceiveAndProccess(IMessagingCB *listener) {
        while (!listener->end_test) {

            _waitset.dispatch(dds::core::Duration::infinite());
            dds::sub::LoanedSamples<T> samples = _reader.take();

            for (unsigned int i = 0; i < samples.length(); ++i) {
                if (samples[i].info().valid()) {
                    const T & sample = samples[i].data();
                    _message.entity_id = sample.entity_id();
                    _message.seq_num = sample.seq_num();
                    _message.timestamp_sec = sample.timestamp_sec();
                    _message.timestamp_usec = sample.timestamp_usec();
                    _message.latency_ping = sample.latency_ping();
                    _message.size = sample.bin_data().size();
                    //_message.data = sample.bin_data();
                    listener->ProcessMessage(_message);
                }
            }
        }
    }

    void waitForWriters(int numPublishers) {

        while (true) {
            if (_reader.subscription_matched_status().current_count() >= numPublishers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }
};

/*********************************************************
 * Initialize
 */
template <typename T>
bool RTIDDSImpl<T>::Initialize(int argc, char *argv[])
{
    using namespace rti::core::policy;
    ParseConfig(argc, argv);

    // setup the QOS profile file to be loaded
    dds::core::QosProvider qos_provider(_ProfileFile, "PerftestQosLibrary::BaseProfileQos");
    dds::domain::qos::DomainParticipantQos qos = qos_provider.participant_qos();

    std::map<std::string, std::string> properties = qos.policy<Property>().get_all();

    // set transports to use
    qos << rti::core::policy::TransportBuiltin(TransportBuiltinMask::udpv4());
    if (_UseTcpOnly) {
        qos << TransportBuiltin(TransportBuiltinMask::none());
        properties["dds.transport.load_plugins"] = "dds.transport.TCPv4.tcp1";
    } else {
        if (_UseSharedMemory) {
            qos << TransportBuiltin(TransportBuiltinMask::shmem());
        }
    }

    if (_AutoThrottle) {
        properties["dds.domain_participant.auto_throttle.enable"] = "true";
    }

    if (!_UseTcpOnly) {
        if ((_Nic != NULL) && (strlen(_Nic) >= 0)) {
            properties["dds.transport.UDPv4.builtin.parent.allow_interfaces"] = _Nic;
        }

        // Shem transport properties
        int received_message_count_max = 1024 * 1024 * 2 / _DataLen;

        char buf[64];
        sprintf(buf,"%d", received_message_count_max);
        properties["dds.transport.shmem.builtin.received_message_count_max"] = buf;
    }

    //We have to copy the properties to the participant_qos object
    qos << rti::core::policy::Property(properties.begin(),properties.end(), true);

    DomainListener *listener = new DomainListener;

    // Creates the participant
    _participant = dds::domain::DomainParticipant(_DomainID, qos, listener,
            dds::core::status::StatusMask::inconsistent_topic() |
            dds::core::status::StatusMask::offered_incompatible_qos() |
            dds::core::status::StatusMask::requested_incompatible_qos() );

    // Create the _publisher and _subscriber
    _publisher = dds::pub::Publisher(_participant, qos_provider.publisher_qos());

    _subscriber = dds::sub::Subscriber(_participant, qos_provider.subscriber_qos());

    return true;

}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *RTIDDSImpl<T>::CreateWriter(const char *topic_name)
{
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    dds::topic::Topic<T> topic(_participant, topic_name);

    std::string qos_profile = "";
    if (topic_name == perftest_cpp::_ThroughputTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "ThroughputQos";
        } else {
            qos_profile = "NoAckThroughputQos";
        }
    } else if (topic_name == perftest_cpp::_LatencyTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "LatencyQos";
        } else {
            qos_profile = "NoAckLatencyQos";
        }
    } else if (topic_name == perftest_cpp::_AnnouncementTopicName) {
        qos_profile = "AnnouncementQos";
    } else {
        std::cerr << "[Error] Topic name must either be "
                << perftest_cpp::_ThroughputTopicName << " or "
                << perftest_cpp::_LatencyTopicName << " or "
                << perftest_cpp::_AnnouncementTopicName << std::endl;
        throw std::logic_error("[Error] Topic name");
    }

    std::string lib_name(_ProfileLibraryName);
    std::string profile_name = lib_name + "::" + qos_profile;
    dds::core::QosProvider qos_provider(_ProfileFile, profile_name);
    dds::pub::qos::DataWriterQos dw_qos = qos_provider.datawriter_qos();

    Reliability qos_reliability = dw_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dw_qos.policy<ResourceLimits>();
    DataWriterResourceLimits qos_dw_resource_limits =
            dw_qos.policy<DataWriterResourceLimits>();
    Durability qos_durability = dw_qos.policy<Durability>();
    PublishMode dwPublishMode= dw_qos.policy<PublishMode>();
    rti::core::policy::DataWriterProtocol dw_DataWriterProtocol =
            dw_qos.policy<rti::core::policy::DataWriterProtocol>();
    RtpsReliableWriterProtocol dw_reliableWriterProtocol =
            dw_DataWriterProtocol.rtps_reliable_writer();


    // This will allow us to load some properties.
    std::map<std::string, std::string> properties;

    if (_UsePositiveAcks) {
        dw_reliableWriterProtocol.disable_positive_acks_min_sample_keep_duration(
                dds::core::Duration((int) _KeepDurationUsec / 1000000,
                        _KeepDurationUsec % 1000000));
    }

    if ((_isLargeData) && (!_isScan)) {
        std::cerr << "[Info] Using asynchronous write for "
                  << topic_name << std::endl;
        dwPublishMode = PublishMode::Asynchronous(
                "dds.flow_controller.token_bucket.fast_flow");
    }

    // only force reliability on throughput/latency topics
    if (topic_name != perftest_cpp::_AnnouncementTopicName) {
        if (_IsReliable) {
            qos_reliability = Reliability::Reliable(dds::core::Duration::infinite());
        } else {
            qos_reliability = Reliability::BestEffort();
        }
    }

    // These QOS's are only set for the Throughput datawriter
    if (qos_profile == "ThroughputQos" ||
        qos_profile == "NoAckThroughputQos")
    {
        if (_BatchSize > 0) {
            dw_qos << Batch::EnabledWithMaxDataBytes(_BatchSize);
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_SendQueueSize);
        } else {
            qos_resource_limits.max_samples(_SendQueueSize);
        }

        if (_HeartbeatPeriod.sec() > 0 || _HeartbeatPeriod.nanosec() > 0) {
            // set the heartbeat_period
            dw_reliableWriterProtocol.heartbeat_period(_HeartbeatPeriod);
            // make the late joiner heartbeat compatible
            dw_reliableWriterProtocol.late_joiner_heartbeat_period(_HeartbeatPeriod);
        }

        if (_FastHeartbeatPeriod.sec() > 0 || _FastHeartbeatPeriod.nanosec() > 0) {
            // set the fast_heartbeat_period
            dw_reliableWriterProtocol.fast_heartbeat_period(_FastHeartbeatPeriod);
        }

        if (_AutoThrottle) {
            properties["dds.data_writer.auto_throttle.enable"] = "true";
        }

        if (_TurboMode) {
            properties["dds.data_writer.enable_turbo_mode.enable"] = "true";

            dw_qos << Batch::Disabled();
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_SendQueueSize);
        }

        qos_resource_limits->initial_samples(_SendQueueSize);
        qos_resource_limits.max_samples_per_instance(qos_resource_limits.max_samples());

        if (_Durability == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);

        dw_reliableWriterProtocol.heartbeats_per_max_samples(_SendQueueSize / 10);
        dw_reliableWriterProtocol.low_watermark(_SendQueueSize * 1 / 10);
        dw_reliableWriterProtocol.high_watermark(_SendQueueSize * 9 / 10);
        dw_reliableWriterProtocol.max_send_window_size(_SendQueueSize);
        dw_reliableWriterProtocol.min_send_window_size(_SendQueueSize);
    }

    if ( (qos_profile == "LatencyQos" || qos_profile =="NoAckLatencyQos")
            && !_DirectCommunication
            && (_Durability == DDS_TRANSIENT_DURABILITY_QOS
                    || _Durability == DDS_PERSISTENT_DURABILITY_QOS)) {
        if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    qos_resource_limits.max_instances(_InstanceCount);
    qos_resource_limits->initial_instances(_InstanceCount);

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            qos_resource_limits->instance_hash_buckets(_InstanceHashBuckets);
        } else {
            qos_resource_limits->instance_hash_buckets(_InstanceCount);
        }
    }

    dw_qos << qos_reliability;
    dw_qos << qos_resource_limits;
    dw_qos << qos_dw_resource_limits;
    dw_qos << qos_durability;
    dw_qos << dwPublishMode;
    dw_DataWriterProtocol.rtps_reliable_writer(dw_reliableWriterProtocol);
    dw_qos << dw_DataWriterProtocol;
    dw_qos << Property(properties.begin(), properties.end(), true);

    dds::pub::DataWriter<T> writer(_publisher, topic, dw_qos);

    return new RTIPublisher<T>(writer, _InstanceCount, _pongSemaphore, _LatencyTest);
}

/*********************************************************
 * CreateReader
 */
template <typename T>
IMessagingReader *RTIDDSImpl<T>::CreateReader(const char *topic_name,
                                           IMessagingCB *callback)
{
    using namespace dds::core::policy;

    dds::topic::Topic<T> topic(_participant, topic_name);

    std::string qos_profile;
    if (topic_name == perftest_cpp::_ThroughputTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "ThroughputQos";
        } else {
            qos_profile = "NoAckThroughputQos";
        }
    } else if (topic_name == perftest_cpp::_LatencyTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "LatencyQos";
        } else {
            qos_profile = "NoAckLatencyQos";
        }
    } else if (topic_name == perftest_cpp::_AnnouncementTopicName) {
        qos_profile = "AnnouncementQos";
    } else {
        std::cerr << "[Error] Topic name must either be "
                << perftest_cpp::_ThroughputTopicName << " or "
                << perftest_cpp::_LatencyTopicName << " or "
                << perftest_cpp::_AnnouncementTopicName << std::endl;
        throw std::logic_error("[Error] Topic name");
    }

    std::string lib_name(_ProfileLibraryName);
    std::string profile_name = lib_name + "::" + qos_profile;
    dds::core::QosProvider qos_provider(_ProfileFile, profile_name);
    dds::sub::qos::DataReaderQos dr_qos = qos_provider.datareader_qos();


    Reliability qos_reliability = dr_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dr_qos.policy<ResourceLimits>();
    Durability qos_durability = dr_qos.policy<Durability>();


    // only force reliability on throughput/latency topics
    if (topic_name != perftest_cpp::_AnnouncementTopicName) {
        if (_IsReliable) {
            qos_reliability = dds::core::policy::Reliability::Reliable();
        } else {
            qos_reliability = dds::core::policy::Reliability::BestEffort();
        }
    }

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos" ||
        qos_profile == "NoAckThroughputQos") {

        if (_Durability == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    if ((qos_profile == "LatencyQos" ||
        qos_profile == "NoAckLatencyQos") &&
        !_DirectCommunication &&
        (_Durability == DDS_TRANSIENT_DURABILITY_QOS ||
         _Durability == DDS_PERSISTENT_DURABILITY_QOS)) {
        if (_Durability == DDS_TRANSIENT_DURABILITY_QOS){
            qos_durability = dds::core::policy::Durability::TransientLocal();
        }
        else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    qos_resource_limits->initial_instances(_InstanceCount);
    qos_resource_limits->max_instances(_InstanceMaxCountReader);

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            qos_resource_limits->instance_hash_buckets(_InstanceHashBuckets);
        } else {
            qos_resource_limits->instance_hash_buckets(_InstanceCount);
        }
    }

    if (!_UseTcpOnly && _IsMulticast) {

       const char *multicast_addr;

        if (strcmp(topic_name, perftest_cpp::_ThroughputTopicName) == 0) {
            multicast_addr = THROUGHPUT_MULTICAST_ADDR;
        } else if (strcmp(topic_name, perftest_cpp::_LatencyTopicName) == 0) {
            multicast_addr = LATENCY_MULTICAST_ADDR;
        } else {
            multicast_addr = ANNOUNCEMENT_MULTICAST_ADDR;
        }

        dds::core::StringSeq transports;
        transports.push_back("udpv4");
        rti::core::TransportMulticastSettings multicast_settings(
                transports, multicast_addr, 0);
        rti::core::TransportMulticastSettingsSeq multicast_seq;
        multicast_seq.push_back(multicast_settings);

        dr_qos << rti::core::policy::TransportMulticast(multicast_seq,
                rti::core::policy::TransportMulticastKind::AUTOMATIC);

    }

    dr_qos << qos_reliability;
    dr_qos << qos_resource_limits;
    dr_qos << qos_durability;

    dds::sub::DataReader<T> reader(dds::core::null);
    if (callback != NULL) {
        ReceiverListener<T> *reader_listener = new ReceiverListener<T>(callback);
        reader = dds::sub::DataReader<T>(_subscriber, topic, dr_qos,
                reader_listener, dds::core::status::StatusMask::data_available());
    } else {
        reader = dds::sub::DataReader<T>(_subscriber, topic, dr_qos);
    }

    if (!strcmp(topic_name, perftest_cpp::_ThroughputTopicName) ||
        !strcmp(topic_name, perftest_cpp::_LatencyTopicName)) {
        _reader = reader;
    }

    return new RTISubscriber<T>(reader,_WaitsetEventCount, _WaitsetDelayUsec);
}

template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;

#ifdef RTI_WIN32
#pragma warning(pop)
#endif

