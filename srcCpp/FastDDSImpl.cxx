/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "FastDDSImpl.h"

using namespace eprosima::fastdds::dds;

template <typename T>
FastDDSImpl<T>::FastDDSImpl()
    : _parent(NULL),
        _pongSemaphore(NULL),
        _factory(NULL),
        _participant(NULL),
        _publisher(NULL),
        _subscriber(NULL),
        _type(new T())
    {
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

    stringStream << "Test: ";

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

 public:
    FastDDSPublisher(
            unsigned long num_instances,
            PerftestSemaphore *pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
    {
        _PM = PM;
    }

    ~FastDDSPublisher()
    {
    }

    void Shutdown()
    {
    }

    void Flush()
    {
    }

    void WaitForReaders(int numSubscribers)
    {
    }

    bool waitForPingResponse()
    {
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout)
    {
        return true;
    }

    bool notifyPingResponse()
    {
        return true;
    }

    unsigned int getPulledSampleCount()
    {
        return 0;
    }

    unsigned int getSampleCount()
    {
        return 0;
    }

    unsigned int getSampleCountPeak()
    {
        return 0;
    }

    void waitForAck(int sec, unsigned int nsec) {
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
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

    // std::bitset<FASTDDS_STATUS_COUNT> a = StatusMask::inconsistent_topic() | StatusMask::offered_incompatible_qos();
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
    return new FastDDSPublisher<T>(
            _PM->get<long>("instances"),
            NULL,
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
    return new FastDDSSubscriber<T>(_PM);
}

template class FastDDSImpl<TestDataKeyed_tPubSubType>;
template class FastDDSImpl<TestData_tPubSubType>;
template class FastDDSImpl<TestDataKeyedLarge_tPubSubType>;
template class FastDDSImpl<TestDataLarge_tPubSubType>;

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(pop)
#endif
