/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "FastDDSImpl.h"

using namespace eprosima::fastdds::dds;


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


/*********************************************************
 * ReaderListener
 */

template <typename T>
class ReaderListener : public DataReaderListener {
public:
    typename T::type _sample;
    SampleInfo _sampleInfo;
    TestMessage _message;
    ReturnCode_t retCode;
    IMessagingCB *_callback;

    ReaderListener(IMessagingCB *callback) : _message(), _callback(callback)
    {
    }

    void on_data_available(DataReader *reader)
    {
        /*
         * FastDDS does not have "take()". This may impact in the overall
         * performance when the receive queue is full of unread samples.
         */
        retCode = reader->take_next_sample((void *) &_sample, &_sampleInfo);

        if (retcode == RETCODE_NO_DATA) {
            fprintf(stderr, "called back no data\n");
            return;
        }

        else if (retcode != RETCODE_OK) {
            fprintf(stderr, "Error during taking data %d\n", retcode);
            return;
        }

        if (_sampleInfo.instance_state == ALIVE) {
            _message.entity_id = _sample.entity_id();
            _message.seq_num = _sample.seq_num();
            _message.timestamp_sec = _sample.timestamp_sec();
            _message.timestamp_usec = _sample.timestamp_usec();
            _message.latency_ping = _sample.latency_ping();
            _message.size = (int) _sample.bin_data().size();
            //_message.data = _sample.bin_data();

            _callback->ProcessMessage(this->_message);
        }
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

        // if (_reader != NULL) {
        //     DDSDataReaderListener* reader_listener = _reader->get_listener();
        //     if (reader_listener != NULL) {
        //         delete(reader_listener);
        //     }
        //     _subscriber->delete_datareader(_reader);
        // }

        // eprosima::fastdds::dds::DomainParticipantListener *listener = _participant->get_listener();
        // if (listener != NULL) {
        //     delete(listener);
        // }

    if (_publisher != nullptr) {
        _participant->delete_publisher(_publisher);
    }
    if (_subscriber != nullptr) {
        _participant->delete_subscriber(_subscriber);
    }
    // if (topic_ != nullptr)
    // {
    //     _participant->delete_topic(topic_);
    // }


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

    // stringStream << "\n" << _transport.printTransportConfigurationSummary();

    // const std::vector<std::string> peerList = _PM->get_vector<std::string>("peer");
    // if (!peerList.empty()) {
    //     stringStream << "\tInitial peers: ";
    //     for (unsigned int i = 0; i < peerList.size(); ++i) {
    //         stringStream << peerList[i];
    //         if (i == peerList.size() - 1) {
    //             stringStream << "\n";
    //         } else {
    //             stringStream << ", ";
    //         }
    //     }
    // }

    return stringStream.str();
}

/*********************************************************
 * FastDDSPublisher
 */
using namespace eprosima::fastrtps::rtps;

template<typename T>
class FastDDSPublisher : public IMessagingWriter
{
protected:
    ParameterManager *_PM;
    DataWriter *_writer;
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
              _pongSemaphore(pongSemaphore),
              _numInstances(num_instances),
              _instanceCounter(0),
              _instancesToBeWritten(instancesToBeWritten)
    {
        _isReliable = _writer->get_qos().reliability().kind
                        == RELIABLE_RELIABILITY_QOS;

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
            if (!PerftestSemaphore_take(_pongSemaphore, timeout)) {
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
        if (!isCftWildCardKey) {
            _writer->write((void *) &_data, _instanceHandles[key]);
        } else {
            _writer->write((void *) &_data, _instanceHandles.back());
        }
        return true;
    }
};

/*********************************************************
 * FastDDSSubscriber
 */

template <typename T>
class FastDDSSubscriber : public IMessagingReader
{
  protected:
    ParameterManager       *_PM;

    void Shutdown()
    {
    }

    void WaitForWriters(int numPublishers)
    {
        while (true) {
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

  public:

    FastDDSSubscriber(ParameterManager *PM)
    {
        _PM = PM;
    }

    ~FastDDSSubscriber()
    {
        Shutdown();
    }


    bool unblock()
    {
        return true;
    }

    unsigned int getSampleCount()
    {
        return 0;
    }

    unsigned int getSampleCountPeak()
    {
        return 0;
    }

    TestMessage *ReceiveMessage()
    {
        return NULL;
    }
};

/*********************************************************
 * Initialize
 */
template <typename T>
bool FastDDSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    // Assign ParameterManager
    _PM = &PM;
    return true;

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

    /*
     * We create the domain Participant QoS first and assing the values if we
     * need to.
     */
    DomainParticipantQos participantQos;
    // Configure Domain Participant QoS
    // TODO if (!configureDomainParticipantQos(participantQos)) {
    //     return false;
    // }

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
    ReturnCode_t retCode = _type.register_type(_participant);
    if (retCode != ReturnCode_t::RETCODE_OK) {
        fprintf(stderr,"Problem registering type.\n");
        return false;
    }

    _publisher = _participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    if (_publisher == nullptr) {
        fprintf(stderr, "Problem creating publisher.\n");
        return false;
    }

    _subscriber = _participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
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
IMessagingWriter *FastDDSImpl<T>::CreateWriter(const char *topic_name)
{
    DataWriterQos dwQos;
    DataWriter *writer = nullptr;
    std::string qos_profile = "";

    Topic *topic = _participant->create_topic(
            std::string(topic_name),
            _type.get_type_name(),
            TOPIC_QOS_DEFAULT);

    if (topic == nullptr) {
        fprintf(stderr, "Problem creating topic %s.\n", topic_name);
        return nullptr;
    }

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile (%s).\n", topic_name);
        return NULL;
    }

    // if (!setup_DW_QoS(dwQos, qos_profile, topic_name)) {
    //     fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
    //     return NULL;
    // }

    WriterListener *dwListener = new WriterListener();
    writer = _publisher->create_datawriter(
            topic,
            dwQos,
            dwListener,
            StatusMask::subscription_matched());
    if (writer == NULL) {
        fprintf(stderr, "Problem creating writer.\n");
        return NULL;
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
        const char *topic_name,
        IMessagingCB *callback)
{
    DataReaderQos drQos;
    DataReader *reader = nullptr;
    std::string qos_profile = "";

    Topic *topic = _participant->create_topic(
            std::string(topic_name),
            _type.get_type_name(),
            TOPIC_QOS_DEFAULT);
    if (topic == nullptr) {
        fprintf(stderr, "Problem creating topic %s.\n", topic_name);
        return nullptr;
    }

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return nullptr;
    }

    // TODO
    // if (!setup_DR_QoS(dr_qos, qos_profile, topic_name)) {
    //     fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
    //     return NULL;
    // }

    reader = _subscriber->create_datareader(
            topic,
            drQos,
            callback != nullptr ? new ReaderListener<T>(callback) : nullptr,
            callback != nullptr ? StatusMask::data_available()
                                : StatusMask::none());
    if (reader == nullptr) {
        fprintf(stderr, "Problem creating reader.\n");
        return nullptr;
    }

    // // Save the reference.
    // if (!strcmp(topic_name, THROUGHPUT_TOPIC_NAME) ||
    //     !strcmp(topic_name, LATENCY_TOPIC_NAME)) {
    //     _reader = reader;
    // }

    // return new FastDDSSubscriber<T>(reader, _PM);
    return new FastDDSSubscriber<T>(_PM);
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
