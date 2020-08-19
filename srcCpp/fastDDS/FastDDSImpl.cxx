/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "FastDDSImpl.h"

using namespace eprosima::fastdds::dds;
using namespace eprosima::fastrtps::rtps;


// Listeners used by the DDS Entities

class ParticipantListener : public DomainParticipantListener {
public:
    ParticipantListener() : DomainParticipantListener()
    {
    }

    virtual ~ParticipantListener()
    {
    }

    virtual void on_inconsistent_topic(Topic *topic, InconsistentTopicStatus status)
    {
        (void) status;
        fprintf(stderr,
                "Found inconsistent topic. Expecting %s of type %s.\n",
                topic->get_name().c_str(),
                topic->get_type_name().c_str());
        fflush(stderr);
    }

    virtual void on_offered_incompatible_qos(
            DataWriter *writer,
            const OfferedIncompatibleQosStatus &status)
    {
        fprintf(stderr,
               "Found incompatible reader for writer %s QoS is %d.\n",
               writer->get_topic()->get_name().c_str(),
               status.last_policy_id);
        fflush(stderr);
    }

    virtual void on_requested_incompatible_qos(
            DataReader *reader,
            const RequestedIncompatibleQosStatus &status)
    {
        fprintf(stderr,
               "Found incompatible writer for reader %s QoS is %d.\n",
               reader->get_topicdescription()->get_name().c_str(),
               status.last_policy_id);
        fflush(stderr);
    }
};

class WriterListener : public DataWriterListener {
public:
    unsigned long _matchedSubscribers;

    WriterListener() : DataWriterListener(), _matchedSubscribers(0)
    {
    }

    virtual ~WriterListener()
    {
    }

    virtual void on_publication_matched(
            DataWriter *writer,
            const PublicationMatchedStatus &info)
    {
        _matchedSubscribers = info.current_count;
    }
};

template <typename T>
class ReaderListener : public DataReaderListener {
public:
    typename T::type _sample;
    SampleInfo _sampleInfo;
    TestMessage _message;
    IMessagingCB *_callback;
    unsigned int _matchedPublishers;

    ReaderListener(IMessagingCB *callback)
            : _message(), _callback(callback), _matchedPublishers(0)
    {
    }

    virtual void on_data_available(DataReader *reader)
    {
        // printf("On data available called\n");
        // if ((reader->get_status_mask() & StatusMask::data_available()) == 0) {
        //     std::cerr << "Status mask does NOT contain data_available. Why is this called?" << std::endl;
        //     std::cerr << "Reader mask is " << reader->get_status_mask() << std::endl;
        //     std::cerr << "On_data_available mask is " << StatusMask::data_available() << std::endl;
        //     return;
        // } else {
        //     std::cerr << "Status mask does contain data_available" << std::endl;
        // }

        /*
         * This is a temporal fix, uncomment code above to show a weird behavior
         * when using a listener that does NOT have the on_data_available call.
         */
        if (_callback == nullptr) {
            return;
        }
        
        /*
         * FastDDS does not have "take()". This may impact in the overall
         * performance when the receive queue is full of unread samples.
         */
        ReturnCode_t retCode =
                reader->take_next_sample((void *) &_sample, &_sampleInfo);

        if (retCode == ReturnCode_t::RETCODE_NO_DATA) {
            fprintf(stderr, "called back no data\n");
            return;
        }

        else if (retCode != ReturnCode_t::RETCODE_OK) {
            fprintf(stderr, "Error during taking data\n");
            return;
        }

        if (_sampleInfo.instance_state == eprosima::fastdds::dds::ALIVE) {
            _message.entity_id = _sample.entity_id();
            _message.seq_num = _sample.seq_num();
            _message.timestamp_sec = _sample.timestamp_sec();
            _message.timestamp_usec = _sample.timestamp_usec();
            _message.latency_ping = _sample.latency_ping();
            _message.size = (int) _sample.bin_data().size();
            //_message.data = _sample.bin_data();
          #ifdef DEBUG_PING_PONG
            std::cout << "<< got sample " << _sample.seq_num() << std::endl;
          #endif

            _callback->ProcessMessage(_message);
        }
    }

    virtual void on_subscription_matched(
            DataReader *reader,
            const SubscriptionMatchedStatus &info)
    {
        _matchedPublishers = info.current_count;
    }
};

template <typename T>
FastDDSImpl<T>::FastDDSImpl()
    : _parent(nullptr),
      _PM(nullptr),
      _pongSemaphore(nullptr),
      _factory(nullptr),
      _participant(nullptr),
      _publisher(nullptr),
      _subscriber(nullptr),
      _type(new T())
    {
        _qoSProfileNameMap[LATENCY_TOPIC_NAME] = std::string("LatencyQos");
        _qoSProfileNameMap[ANNOUNCEMENT_TOPIC_NAME] = std::string("AnnouncementQos");
        _qoSProfileNameMap[THROUGHPUT_TOPIC_NAME] = std::string("ThroughputQos");

    }

/*********************************************************
 * Shutdown
 */
template <typename T>
void FastDDSImpl<T>::Shutdown()
{

    if (_participant != NULL) {

        if (_publisher != nullptr) {
            _participant->delete_publisher(_publisher);
        }
        if (_subscriber != nullptr) {
            _participant->delete_subscriber(_subscriber);
        }
        _factory->delete_participant(_participant);
    }

    if (_pongSemaphore != NULL) {
        PerftestSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }
}

/*********************************************************

 * Validate and manage the parameters
 */
template <typename T>
bool FastDDSImpl<T>::validate_input()
{

    // Manage transport parameter
    if (!_transport.validate_input()) {
        fprintf(stderr, "Failure validating the transport options.\n");
        return false;
    };

    /*
     * Manage parameter -verbosity.
     * Setting verbosity if the parameter is provided
     */
    if (_PM->is_set("verbosity")) {
        PerftestConfigureVerbosity(_PM->get<int>("verbosity"));
    }

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
template <typename T>
std::string FastDDSImpl<T>::PrintConfiguration()
{

    std::ostringstream stringStream;

    // Domain ID
    stringStream << "\tDomain: " << _PM->get<int>("domain") << "\n";

    // XML File
    stringStream << "\tXML File: ";
    if (_PM->get<bool>("noXmlQos")) {
        stringStream << "Disabled\n";
    } else {
        stringStream << _PM->get<std::string>("qosFile") << "\n";
    }

    stringStream << "\n" << _transport.printTransportConfigurationSummary();

    const std::vector<std::string> peerList =
            _PM->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        stringStream << "\tInitial peers: ";
        for (unsigned int i = 0; i < peerList.size(); ++i) {
            stringStream << peerList[i];
            if (i == peerList.size() - 1) {
                stringStream << "\n";
            } else {
                stringStream << ", ";
            }
        }
    }

    return stringStream.str();
}

/*********************************************************
 * FastDDSPublisher
 */

template<typename T>
class FastDDSPublisher : public IMessagingWriter
{
protected:
    ParameterManager *_PM;
    DataWriter *_writer;
    int failures;
    PerftestSemaphore *_pongSemaphore;
    unsigned long _numInstances;
    unsigned long _instanceCounter;
    long _instancesToBeWritten;
    typename T::type _data;
    std::vector<InstanceHandle_t> _instanceHandles;
    bool _isReliable;

public:
    FastDDSPublisher(
            DataWriter *writer,
            unsigned long num_instances,
            PerftestSemaphore *pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
            : _PM(PM),
              _writer(writer),
              failures(0),
              _pongSemaphore(pongSemaphore),
              _numInstances(num_instances),
              _instanceCounter(0),
              _instancesToBeWritten(instancesToBeWritten)
    {
        _isReliable = _writer->get_qos().reliability().kind
                        == ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;

        if (_PM->get<bool>("keyed")) {
            for (unsigned long i = 0; i < _numInstances; ++i) {
                for (int c = 0; c < KEY_SIZE; c++) {
                    _data.key()[c] = (unsigned char) (i >> c * 8);
                }
                _instanceHandles.push_back(_writer->register_instance(&_data));
            }

            // Register the key of MAX_CFT_VALUE
            for (int c = 0; c < KEY_SIZE; c++) {
                _data.key()[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
            }
            _instanceHandles.push_back(_writer->register_instance(&_data));
        }
    }

    ~FastDDSPublisher()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_writer->get_listener() != nullptr) {
            delete(_writer->get_listener());
            _writer->set_listener(nullptr);
        }
    }

    void Flush()
    {
    }

    void WaitForReaders(int numSubscribers)
    {
        WriterListener *listener = (WriterListener *) _writer->get_listener();
        if (listener == nullptr) {
            fprintf(stderr,"Could not get listener from writer.\n");
            return;
        }
        while (listener->_matchedSubscribers < numSubscribers) {
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    bool waitForPingResponse()
    {
        if(_pongSemaphore != NULL) {
            if (!PerftestSemaphore_take(
                    _pongSemaphore,
                    PERFTEST_SEMAPHORE_TIMEOUT_INFINITE)) {
                fprintf(stderr,"Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout)
    {
        if(_pongSemaphore != NULL) {
            if (!PerftestSemaphore_take(
                    _pongSemaphore,
                    timeout)) {
                fprintf(stderr,"Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse()
    {
        if(_pongSemaphore != NULL) {
            if (!PerftestSemaphore_give(_pongSemaphore)) {
                fprintf(stderr,"Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount()
    {
        // Not supported yet
        return 0;
    }

    unsigned int getSampleCount()
    {
        // Not supported yet
        return 0;
    }

    unsigned int getSampleCountPeak()
    {
        // Not supported in Micro
        return 0;
    }

    void waitForAck(int sec, unsigned int nsec) {
        if (_isReliable) {
            _writer->wait_for_acknowledgments(Duration_t(sec, nsec));
        } else {
            PerftestClock::milliSleep(nsec / 1000000);
        }
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        _data.entity_id(message.entity_id);
        _data.seq_num(message.seq_num);
        _data.timestamp_sec(message.timestamp_sec);
        _data.timestamp_usec(message.timestamp_usec);
        _data.latency_ping(message.latency_ping);
        _data.bin_data().resize(message.size);
        // _data.bin_data(message.data);

        // Calculate the key
        long key = 0;
        if (!isCftWildCardKey) {
            if (_numInstances > 1) {
                if (_instancesToBeWritten == -1) {
                    key = _instanceCounter++ % _numInstances;
                } else {  // send sample to a specific subscriber
                    key = _instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        for (int c = 0; c < KEY_SIZE; c++) {
            _data.key()[c] = (unsigned char) (key >> c * 8);
        }

        // Write call
        // if (_PM->get<bool>("keyed")) {
        //     if (!isCftWildCardKey) {
        //         _writer->write((void *) &_data, _instanceHandles[key]);
        //     } else {
        //         _writer->write((void *) &_data, _instanceHandles.back());
        //     }
        // } else {
          #ifdef DEBUG_PING_PONG
            std::cerr << " -- -- before sending it in the write\n";
          #endif
            while(!_writer->write((void *) &_data)) {
              #ifdef DEBUG_PING_PONG
                std::cerr << "-- -- -- Still trying to send\n";
              #endif
            }
          #ifdef DEBUG_PING_PONG
            std::cerr << ">> wrote sample " << _data.seq_num() << std::endl;
          #endif
        // }

        return true;
    }
};

/*********************************************************
 * FastDDSSubscriber
 */

template <typename T>
class FastDDSSubscriber : public IMessagingReader
{
public:
    DataReader *_reader;
    ParameterManager *_PM;
    TestMessage _message;
    bool _endTest;
    bool _useReceiveThread;
    typename T::type _sample;
    SampleInfo _sampleInfo;

    FastDDSSubscriber(DataReader *reader, ParameterManager *PM)
            : _reader(reader),
              _PM(PM),
              _message(),
              _endTest(false),
              _useReceiveThread(_reader->get_listener() == nullptr)
    {
        // // null listener means using receive thread
        // _useReceiveThread = _reader->get_listener() == nullptr;
    }

    ~FastDDSSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
    }

    void WaitForWriters(int numPublishers)
    {
        ReaderListener<T> *listener = (ReaderListener<T> *) _reader->get_listener();
        if (listener == nullptr) {
            fprintf(stderr,"Could not get listener from reader.\n");
            return;
        }
        while (listener->_matchedPublishers < numPublishers) {
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    bool unblock()
    {
        _endTest = true;
        return true;
    }

    unsigned int getSampleCount()
    {
        // Not implemented
        return 0;
    }

    unsigned int getSampleCountPeak()
    {
        // Not implemented
        return 0;
    }

    TestMessage *ReceiveMessage()
    {
        Duration_t timeout(2,0);

        while (!this->_endTest) {

            /*
             * If wait_for_unread_message returns true, it means that there is
             * data there. If it returns false, it timed out.
             */
            if (_reader->wait_for_unread_message(timeout)) {

                // In FastDDS we need to take samples one by one.
                ReturnCode_t retCode = _reader->take_next_sample(
                        (void *) &_sample,
                        &_sampleInfo);

                if (retCode == ReturnCode_t::RETCODE_NO_DATA) {
                    fprintf(stderr, "called back no data\n");
                    return nullptr;
                } else if (retCode != ReturnCode_t::RETCODE_OK) {
                    fprintf(stderr, "Error during taking data\n");
                    return nullptr;
                }

                if (_sampleInfo.instance_state 
                        == eprosima::fastdds::dds::ALIVE) {
                    _message.entity_id = _sample.entity_id();
                    _message.seq_num = _sample.seq_num();
                    _message.timestamp_sec = _sample.timestamp_sec();
                    _message.timestamp_usec = _sample.timestamp_usec();
                    _message.latency_ping = _sample.latency_ping();
                    _message.size = (int) _sample.bin_data().size();
                    //_message.data = _sample.bin_data();

                    return &_message;
                }
            } // wait_for_unread
        } // while _endTest
        
        // If the while have not returned already data, we return nullptr;
        return nullptr;
    }
};



/*********************************************************
 * configure_participant_qos
 */
template <typename T>
bool FastDDSImpl<T>::configure_participant_qos(DomainParticipantQos &qos)
{
    qos = _factory->get_default_participant_qos();

    // Set initial peers and not use multicast
    const std::vector<std::string> peerList =
            _PM->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        Locator_t initial_peer;
        for (unsigned int i = 0; i < peerList.size(); ++i) {
            IPLocator::setIPv4(
                    initial_peer,
                    const_cast<char *>(peerList[i].c_str()));
            // initial_peer.port = 7412; // TODO: Do we need to enable this?
            // 7400 + 250 * domainID + 10 + 2 * participantID
            qos.wire_protocol().builtin.initialPeersList.push_back(
                    initial_peer);
        }

        // TODO: Do we need to disable the multicast receive addresses?
        Locator_t default_unicast_locator;
        qos.wire_protocol().builtin.metatrafficUnicastLocatorList.push_back(
                default_unicast_locator);
    }


    if (!PerftestConfigureTransport(_transport, qos, _PM)) {
        return false;
    }

    return true;
}

/*********************************************************
 * configure_writer_qos
 */
template <typename T>
bool FastDDSImpl<T>::configure_writer_qos(
        DataWriterQos &qos,
        std::string qosProfile)
{

    // RELIABILITY AND DURABILITY
    if (qosProfile != "AnnouncementQos") {

        if (_PM->get<bool>("bestEffort")) {
            qos.reliability().kind = ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        } else {
            qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
            qos.reliability().max_blocking_time = c_TimeInfinite;
        }

        if (_PM->get<bool>("noPositiveAcks")){
            qos.reliable_writer_qos().disable_positive_acks.enabled = true;
            if (_PM->is_set("keepDurationUsec")) {
                qos.reliable_writer_qos().disable_positive_acks.duration =
                        eprosima::fastrtps::Duration_t(
                                _PM->get<unsigned long long>("keepDurationUsec")
                                * 1e-6);
            }
        }

        if (_PM->is_set("durability")) {
            qos.durability().kind =(DurabilityQosPolicyKind)_PM->get<int>("durability");
        } else {
            qos.durability().kind = DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
        }

    } else { // AnnouncementQoS
        qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    }

    // HISTORY
    if (_PM->is_set("keepLast") || _PM->is_set("keepLastDepth")) {
        qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
        if (_PM->is_set("keepLastDepth")) {
            qos.history().depth = _PM->get<int>("keepLastDepth");
        } else {
            qos.history().depth = _PM->get<int>("sendQueueSize");
        }
    } else {
        qos.history().kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    }

    // // PUBLISH MODE
    if (_PM->get<bool>("asynchronous")) {
        qos.publish_mode().kind = PublishModeQosPolicyKind::ASYNCHRONOUS_PUBLISH_MODE;
    }

    // TIMES
    if (qosProfile == "ThroughputQos") {
        qos.reliable_writer_qos().times.heartbeatPeriod.seconds = 0;
        qos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 10000000;
        qos.reliable_writer_qos().times.initialHeartbeatDelay = c_TimeZero;
        qos.reliable_writer_qos().times.nackResponseDelay = c_TimeZero;
    } else if (qosProfile == "LatencyQos") {
        qos.reliable_writer_qos().times.heartbeatPeriod.seconds = 0;
        qos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 10000000;
        qos.reliable_writer_qos().times.initialHeartbeatDelay = c_TimeZero;
        qos.reliable_writer_qos().times.nackResponseDelay = c_TimeZero;
    } else if (qosProfile == "AnnouncementQos") {
        qos.reliable_writer_qos().times.heartbeatPeriod.seconds = 0;
        qos.reliable_writer_qos().times.heartbeatPeriod.nanosec = 10000000;
        qos.reliable_writer_qos().times.initialHeartbeatDelay = c_TimeZero;
        qos.reliable_writer_qos().times.nackResponseDelay = c_TimeZero;
    } else {
        fprintf(stderr,
                "[Error]: Cannot find settings for qosProfile %s\n",
                qosProfile.c_str());
        return false;
    }

    // RESOURCE LIMITS
    if (qosProfile == "ThroughputQos") {
        qos.resource_limits().allocated_samples = _PM->get<int>("sendQueueSize");
        qos.resource_limits().max_samples = _PM->get<int>("sendQueueSize"); //0 is Length unlimited.
        if (_PM->get<bool>("keyed")) {
            qos.resource_limits().max_instances = _PM->get<long>("instances");
            qos.resource_limits().max_samples_per_instance = 0;
        }
    } else if (qosProfile == "LatencyQos") {
        qos.resource_limits().allocated_samples = 128;
        qos.resource_limits().max_samples = 0;
        if (_PM->get<bool>("keyed")) {
            qos.resource_limits().max_instances = 0;
            qos.resource_limits().max_samples_per_instance = 0;
        }
    } else if (qosProfile == "AnnouncementQos") {
        qos.resource_limits().allocated_samples = 50;
        qos.resource_limits().max_samples = 0;
        if (_PM->get<bool>("keyed")) {
            qos.resource_limits().max_instances = 0;
            qos.resource_limits().max_samples_per_instance = 0;
        }
    } else {
        fprintf(stderr,
                "[Error]: Cannot find settings for qosProfile %s\n",
                qosProfile.c_str());
        return false;
    }

    // // SUMMARY FOR THE RESOURCE LIMITS
    if (_PM->get<bool>("showResourceLimits")) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DW (" 
                    << qosProfile
                    << " topic):\n"
                    << "\tSamples (Allocated/Max): "
                    << qos.resource_limits().allocated_samples
                    << "/"
                    << qos.resource_limits().max_samples
                    << "\n";

        if (_PM->get<bool>("keyed")) {
            stringStream << "\tInstances (Max): "
                        << qos.resource_limits().max_instances
                        << "\n";
            stringStream << "\tMax Samples per Instance: "
                        << qos.resource_limits().max_samples_per_instance
                        << "\n";

        }
            stringStream << "\tHeartbeat period (s/ns): "
                        << qos.reliable_writer_qos().times.heartbeatPeriod.seconds
                        << ","
                        << qos.reliable_writer_qos().times.heartbeatPeriod.nanosec
                        << "\n";
            stringStream << "\tDurability is: "
                        << qos.durability().kind
                        << "\n";
            stringStream << "\tReliability is: "
                        << qos.reliability().kind
                        << "\n";
            stringStream << "\tMax blocking time is (s/ns): "
                        << qos.reliability().max_blocking_time.seconds
                        << ","
                        << qos.reliability().max_blocking_time.nanosec
                        << "\n";
        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }
    return true;
}


/*********************************************************
 * configure_reader_qos
 */
template <typename T>
bool FastDDSImpl<T>::configure_reader_qos(
        DataReaderQos &qos,
        std::string qosProfile)
{
    if (qosProfile != "AnnouncementQos") {

        if (_PM->get<bool>("bestEffort")) {
            qos.reliability().kind = ReliabilityQosPolicyKind::BEST_EFFORT_RELIABILITY_QOS;
        } else {
            qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
            qos.reliability().max_blocking_time = c_TimeInfinite;
        }

        if (_PM->get<bool>("noPositiveAcks")){
            qos.reliable_reader_qos().disable_positive_ACKs.enabled = true;
            if (_PM->is_set("keepDurationUsec")) {
                qos.reliable_reader_qos().disable_positive_ACKs.duration =
                        eprosima::fastrtps::Duration_t(
                                _PM->get<unsigned long long>("keepDurationUsec")
                                * 1e-6);
            }
        }

        if (_PM->is_set("durability")) {
            qos.durability().kind =(DurabilityQosPolicyKind)_PM->get<int>("durability");
        } else {
            qos.durability().kind = DurabilityQosPolicyKind::VOLATILE_DURABILITY_QOS;
        }
    } else { // AnnouncementQoS
        qos.reliability().kind = ReliabilityQosPolicyKind::RELIABLE_RELIABILITY_QOS;
        qos.durability().kind = DurabilityQosPolicyKind::TRANSIENT_LOCAL_DURABILITY_QOS;
    }

    // HISTORY
    if (_PM->is_set("keepLast") || _PM->is_set("keepLastDepth")) {
        qos.history().kind = HistoryQosPolicyKind::KEEP_LAST_HISTORY_QOS;
        if (_PM->is_set("keepLastDepth")) {
            qos.history().depth = _PM->get<int>("keepLastDepth");
        } else {
            qos.history().depth = _PM->get<int>("sendQueueSize");
        }
    } else {
        qos.history().kind = HistoryQosPolicyKind::KEEP_ALL_HISTORY_QOS;
    }


    // RESOURCE LIMITS
    if (qosProfile == "ThroughputQos") {
        qos.resource_limits().allocated_samples = 128;
        if (_PM->is_set("receiveQueueSize")) {
            qos.resource_limits().allocated_samples = _PM->get<int>("receiveQueueSize");
        }
        qos.resource_limits().max_samples = 10000;
        if (_PM->get<bool>("keyed")) {
            qos.resource_limits().max_instances = 0;
            qos.resource_limits().max_samples_per_instance = 0;
        }
    } else if (qosProfile == "LatencyQos") {
        qos.resource_limits().allocated_samples = 100;
        qos.resource_limits().max_samples = 100;
        if (_PM->get<bool>("keyed")) {
            qos.resource_limits().max_instances = 0;
            qos.resource_limits().max_samples_per_instance = 0;
        }
    } else if (qosProfile == "AnnouncementQos") {
        qos.resource_limits().allocated_samples = 100;
        qos.resource_limits().max_samples = 100;
        if (_PM->get<bool>("keyed")) {
            qos.resource_limits().max_instances = 0;
            qos.resource_limits().max_samples_per_instance = 0;
        }
    } else {
        fprintf(stderr,
                "[Error]: Cannot find settings for qosProfile %s\n",
                qosProfile.c_str());
        return false;
    }

    if (_PM->is_set("receiveQueueSize")) {
        qos.resource_limits().max_samples = _PM->get<int>("receiveQueueSize");
    }

    if (_PM->get<bool>("showResourceLimits")) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DR (" 
                    << qosProfile
                    << " topic):\n"
                    << "\tSamples (Allocated/Max): "
                    << qos.resource_limits().allocated_samples
                    << "/"
                    << qos.resource_limits().max_samples
                    << "\n";

        if (_PM->get<bool>("keyed")){
            stringStream << "\tInstances (Max): "
                        << qos.resource_limits().max_instances
                        << "\n";
            stringStream << "\tMax Samples per Instance: "
                        << qos.resource_limits().max_samples_per_instance
                        << "\n";

        }
        stringStream << "\tDurability is: "
                    << qos.durability().kind
                    << "\n";
        stringStream << "\tReliability is: "
                    << qos.reliability().kind
                    << "\n";
        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }

    return true;
}

/*********************************************************
 * Initialize
 */
template <typename T>
bool FastDDSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    // Assign ParameterManager
    _PM = &PM;
    _transport.initialize(_PM);
    ReturnCode_t retCode = ReturnCode_t::RETCODE_OK;

    if (!validate_input()) {
        return false;
    }

    /*
     * Only if we run the latency test we need to wait
     * for pongs after sending pings
     */
    _pongSemaphore = _PM->get<bool>("latencyTest")
            ? PerftestSemaphore_new()
            : nullptr;

    _factory = DomainParticipantFactory::get_instance();

    DomainParticipantQos participantQos;
    if (!configure_participant_qos(participantQos)) {
        return false;
    }

    StatusMask statusMask = StatusMask::inconsistent_topic();
    statusMask << StatusMask::offered_incompatible_qos();
    statusMask << StatusMask::requested_incompatible_qos();
    
    // Creates the participant
    _participant = _factory->create_participant(
            _PM->get<int>("domain"),
            participantQos,
            new ParticipantListener(),
            statusMask);
    if (_participant == nullptr) {
        fprintf(stderr,"Problem creating participant.\n");
        return false;
    }

    // Register type
    retCode = _type.register_type(_participant);
    if (retCode != ReturnCode_t::RETCODE_OK) {
        fprintf(stderr,"Problem registering type.\n");
        return false;
    }

    PublisherQos publisherQos;
    publisherQos.presentation().access_scope =
            PresentationQosPolicyAccessScopeKind::TOPIC_PRESENTATION_QOS;
    publisherQos.presentation().ordered_access = true;
    _publisher = _participant->create_publisher(publisherQos, nullptr);
    if (_publisher == nullptr) {
        fprintf(stderr, "Problem creating publisher.\n");
        return false;
    }

    SubscriberQos subscriberQos;
    subscriberQos.presentation().access_scope =
            PresentationQosPolicyAccessScopeKind::TOPIC_PRESENTATION_QOS;
    subscriberQos.presentation().ordered_access = true;
    _subscriber = _participant->create_subscriber(subscriberQos, nullptr);
    if (_subscriber == nullptr) {
        fprintf(stderr, "Problem creating subscriber.\n");
        return false;
    }

    return true;
}

/*********************************************************
 * GetInitializationSampleCount
 */
template <typename T>
unsigned long FastDDSImpl<T>::GetInitializationSampleCount()
{
    return 50;
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *FastDDSImpl<T>::CreateWriter(const char *topicName)
{
    DataWriter *writer = nullptr;

    if (_participant == nullptr) {
        fprintf(stderr, "Participant is null\n");
        return nullptr;
    }

    Topic *topic = _participant->create_topic(
            std::string(topicName),
            _type.get_type_name(),
            TOPIC_QOS_DEFAULT);

    if (topic == nullptr) {
        fprintf(stderr, "Problem creating topic %s.\n", topicName);
        return nullptr;
    }

    std::string qosProfile = get_qos_profile_name(topicName);
    if (qosProfile.empty()) {
        fprintf(stderr, "Problem getting qos profile (%s).\n", topicName);
        return nullptr;
    }

    DataWriterQos dwQos;
    if (!configure_writer_qos(dwQos, qosProfile)) {
        fprintf(stderr,
                "Problem creating additional QoS settings with %s profile.\n",
                qosProfile.c_str());
        return nullptr;
    }

    WriterListener *dwListener = new WriterListener();
    writer = _publisher->create_datawriter(
            topic,
            dwQos,
            dwListener,
            StatusMask::subscription_matched());
    if (writer == nullptr) {
        fprintf(stderr, "Problem creating writer.\n");
        return nullptr;
    }

    return new FastDDSPublisher<T>(
            writer,
            _PM->get<long>("instances"),
            _pongSemaphore,
            _PM->get<long>("writeInstance"),
            _PM);
}

/*********************************************************
 * CreateReader
 */
template <typename T>
IMessagingReader *FastDDSImpl<T>::CreateReader(
        const char *topicName,
        IMessagingCB *callback)
{
    DataReader *reader = nullptr;

    Topic *topic = _participant->create_topic(
            std::string(topicName),
            _type.get_type_name(),
            TOPIC_QOS_DEFAULT);
    if (topic == nullptr) {
        fprintf(stderr, "Problem creating topic %s.\n", topicName);
        return nullptr;
    }

    std::string qosProfile = get_qos_profile_name(topicName);
    if (qosProfile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return nullptr;
    }

    DataReaderQos drQos;
    if (!configure_reader_qos(drQos, qosProfile)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qosProfile.c_str());
        return NULL;
    }

    StatusMask mask = StatusMask::publication_matched();
    if (callback != nullptr) {
        mask << StatusMask::data_available();
    }

    reader = _subscriber->create_datareader(
            topic,
            drQos,
            new ReaderListener<T>(callback),
            mask);
    if (reader == nullptr) {
        fprintf(stderr, "Problem creating reader.\n");
        return nullptr;
    }

    return new FastDDSSubscriber<T>(reader, _PM);
}

template <typename T>
const std::string FastDDSImpl<T>::get_qos_profile_name(const char *topicName)
{
    if (_qoSProfileNameMap[std::string(topicName)].empty()) {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME,
                LATENCY_TOPIC_NAME,
                ANNOUNCEMENT_TOPIC_NAME);
    }

    /* If the topic name dont match any key return a empty string */
    return _qoSProfileNameMap[std::string(topicName)];
}

template class FastDDSImpl<TestDataKeyed_tPubSubType>;
template class FastDDSImpl<TestData_tPubSubType>;
template class FastDDSImpl<TestDataKeyedLarge_tPubSubType>;
template class FastDDSImpl<TestDataLarge_tPubSubType>;

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(pop)
#endif
