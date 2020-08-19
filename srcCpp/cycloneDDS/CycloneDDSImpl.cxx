/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "CycloneDDSImpl.h"

/*
 * TODO
 * We should use the function inside CycloneDDSImpl instead of this one or
 * alternatively remove the other one and use this.
 */
const std::string GetDDSVersionString()
{
    return std::string(DDS_PROJECT_NAME) + " " + std::string(DDS_VERSION);
};

// Listeners used by the DDS Entities

static void participant_on_inconsistent_topic(
        dds_entity_t topic,
        const dds_inconsistent_topic_status_t status,
        void *arg)
{
    (void) topic;
    (void) status;
    (void) arg;
    fprintf(stderr, "Found inconsistent topic.\n");
    fflush(stderr);
}

static void participant_on_offered_incompatible_qos(
        dds_entity_t writer,
        const dds_offered_incompatible_qos_status_t status,
        void *arg)
{
    (void) writer;
    (void) status;
    (void) arg;
    fprintf(stderr, "Found incompatible reader for writer.\n");
    fflush(stderr);
}

static void participant_on_requested_incompatible_qos(
        dds_entity_t reader,
        const dds_requested_incompatible_qos_status_t status,
        void *arg)
{
    (void) reader;
    (void) status;
    (void) arg;
    fprintf(stderr, "Found incompatible writer for reader.\n");
    fflush(stderr);
}

// Max samples per take
#define MAX_SAMPLES 1000

template <typename T>
class ReaderListener {
public:
    void *samplePointers[MAX_SAMPLES];
    T samples[MAX_SAMPLES];
    dds_sample_info_t sampleInfo[MAX_SAMPLES];
    TestMessage message;
    IMessagingCB *callback;

    ReaderListener(IMessagingCB *callback) : message(), callback(callback)
    {
        memset(samples, 0, sizeof(samples));
        for (unsigned int i = 0; i < MAX_SAMPLES; i++) {
            samplePointers[i] = &samples[i];
        }
    }
};

template <typename T>
static void reader_on_data_available(dds_entity_t reader, void *arg)
{
    //printf("Sample received!\n");
    ReaderListener<T> *listenerInfo = (ReaderListener<T> *) arg;

    int samplecount = dds_take(
            reader,
            listenerInfo->samplePointers,
            listenerInfo->sampleInfo,
            MAX_SAMPLES,
            MAX_SAMPLES);
    if (samplecount < 0) {
        fprintf(stderr,
                "[Error]: reader_on_data_available error %s\n",
                dds_strretcode(-samplecount));
        return;
    }

    for (int i = 0; i < samplecount; i++) {

        if (listenerInfo->sampleInfo[i].valid_data) {
            T *valid_sample = (T *)(&listenerInfo->samples[i]);

            listenerInfo->message.entity_id = valid_sample->entity_id;
            listenerInfo->message.seq_num = valid_sample->seq_num;
            listenerInfo->message.timestamp_sec = valid_sample->timestamp_sec;
            listenerInfo->message.timestamp_usec = valid_sample->timestamp_usec;
            listenerInfo->message.latency_ping = valid_sample->latency_ping;
            listenerInfo->message.size = valid_sample->bin_data._length;
            listenerInfo->message.data = (char *) valid_sample->bin_data._buffer;

            listenerInfo->callback->ProcessMessage(listenerInfo->message);
        }

    }
}

// Auxiliary functions for sequences (CycloneDDS seems that is lacking these)

void octet_seq_ensure_length(dds_sequence &sequence, unsigned int length)
{
    if (sequence._maximum < length) {
        free(sequence._buffer);
        sequence._buffer = (uint8_t *) dds_alloc(length);
        sequence._maximum = length;
        sequence._length = length;
    } else {
        sequence._length = length;
    }
}

void octet_seq_ensure_free(dds_sequence &sequence)
{
    if (sequence._length < 0) {
        free(sequence._buffer);
        sequence._length = 0;
    }
}

/*********************************************************
 * Constructor
 */
template <typename T>
CycloneDDSImpl<T>::CycloneDDSImpl(dds_topic_descriptor_t topicDescriptor)
    : _parent(nullptr),
      _PM(nullptr),
      _pongSemaphore(nullptr),
      _topicDescriptor(topicDescriptor)
    {
        _qoSProfileNameMap[LATENCY_TOPIC_NAME] = std::string("LatencyQos");
        _qoSProfileNameMap[ANNOUNCEMENT_TOPIC_NAME] = std::string("AnnouncementQos");
        _qoSProfileNameMap[THROUGHPUT_TOPIC_NAME] = std::string("ThroughputQos");

    }

/*********************************************************
 * Shutdown
 */
template <typename T>
void CycloneDDSImpl<T>::Shutdown()
{
    //TODO
    // if (_participant != nullptr) {

    //     if (_publisher != nullptr) {
    //         _participant->delete_publisher(_publisher);
    //     }
    //     if (_subscriber != nullptr) {
    //         _participant->delete_subscriber(_subscriber);
    //     }
    //     _factory->delete_participant(_participant);
    // }

    if (_pongSemaphore != nullptr) {
        PerftestSemaphore_delete(_pongSemaphore);
        _pongSemaphore = nullptr;
    }
}

/*********************************************************
 * get_middleware_version_string
 */
template <typename T>
const std::string CycloneDDSImpl<T>::get_middleware_version_string()
{
    return std::string(DDS_PROJECT_NAME) + " " + std::string(DDS_VERSION);
}

/*********************************************************
 * configure_middleware_verbosity
 */
template <typename T>
void CycloneDDSImpl<T>::configure_middleware_verbosity(int verbosityLevel)
{
    std::string verbosityString = "severe";
    switch (verbosityLevel) {
    case 0:
        fprintf(stderr, "Setting verbosity to NONE\n");
        verbosityString = "none";
        break;
    case 1:
        fprintf(stderr, "Setting verbosity to SEVERE\n");
        verbosityString = "severe";
        break;
    case 2:
        fprintf(stderr, "Setting verbosity to FINE\n");
        verbosityString = "fine";
        break;
    case 3:
        fprintf(stderr, "Setting verbosity to FINEST\n");
        verbosityString = "finest";
        break;
    default:
        fprintf(stderr,
                "[Error]: Invalid value for the verbosity parameter. Using "
                "default\n");
        break;
    }

    std::ostringstream stringStream;
    stringStream << "<Tracing>"
                 << "<Verbosity>"
                 << verbosityString
                 << "</Verbosity>"
                 << "<OutputFile>stdout</OutputFile>"
                 << "</Tracing>";
    _verbosityString = stringStream.str();
}

/*********************************************************
 * Validate and manage the parameters
 */
template <typename T>
bool CycloneDDSImpl<T>::validate_input()
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
        configure_middleware_verbosity(_PM->get<int>("verbosity"));
    }

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
template <typename T>
std::string CycloneDDSImpl<T>::PrintConfiguration()
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
 * CycloneDDSPublisher
 */
template<typename T>
class CycloneDDSPublisher : public IMessagingWriter
{
protected:
    ParameterManager *_PM;
    dds_entity_t _writer;
    PerftestSemaphore *_pongSemaphore;
    unsigned long _numInstances;
    unsigned long _instanceCounter;
    long _instancesToBeWritten;
    T _data;
    TestDataKeyedLarge_t _data_2;
    std::vector<dds_instance_handle_t> _instanceHandles;
    bool _isReliable;
    dds_return_t retCode;

public:
    CycloneDDSPublisher(
            dds_entity_t writer,
            unsigned long num_instances,
            PerftestSemaphore *pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
            : _PM(PM),
              _writer(writer),
              _pongSemaphore(pongSemaphore),
              _numInstances(num_instances),
              _instanceCounter(0),
              _instancesToBeWritten(instancesToBeWritten),
              _isReliable(!_PM->is_set("bestEffort"))
    {
        memset(&_data, 0, sizeof(_data));
    }

    ~CycloneDDSPublisher()
    {
        Shutdown();
    }

    void Shutdown()
    {
        // if (_writer->get_listener() != nullptr) {
        //     delete(_writer->get_listener());
        //     _writer->set_listener(nullptr);
        // }
    }

    void Flush()
    {
    }

    void WaitForReaders(int numSubscribers)
    {
        dds_set_status_mask(_writer, DDS_PUBLICATION_MATCHED_STATUS);
        dds_publication_matched_status_t status;
        dds_get_publication_matched_status(_writer, &status);
        while (status.total_count < numSubscribers) {
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
            dds_get_publication_matched_status(_writer, &status);
        }
    }

    bool waitForPingResponse()
    {
        if(_pongSemaphore != nullptr) {
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
        if(_pongSemaphore != nullptr) {
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
        if(_pongSemaphore != nullptr) {
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
        // Not supported in Cyclone
        return 0;
    }

    void waitForAck(int sec, unsigned int nsec) {
        if (_isReliable) {
            retCode = dds_wait_for_acks(_writer, dds_duration_t (sec * DDS_NSECS_IN_SEC + nsec));
            if (retCode != DDS_RETCODE_OK) {
                fprintf(stderr,"WaitForAck issue (retCode %d)\n", retCode);
            }
        } else {
            PerftestClock::milliSleep(nsec / DDS_NSECS_IN_MSEC);
        }
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        _data.entity_id = message.entity_id;
        _data.seq_num = message.seq_num;
        _data.timestamp_sec =message.timestamp_sec;
        _data.timestamp_usec = message.timestamp_usec;
        _data.latency_ping = message.latency_ping;
        octet_seq_ensure_length(_data.bin_data, message.size);

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
            _data.key[c] = (unsigned char) (key >> c * 8);
        }
      #ifdef DEBUG_PING_PONG
        std::cerr << " -- -- before sending it in the write\n";
      #endif
        // I could not find a write() call that receives an instance handle.
        retCode = dds_write(_writer, &_data);
        if (retCode != DDS_RETCODE_OK) {
            fprintf(stderr, "Could not write: %s\n", dds_strretcode(-retCode));
            return false;
        }
      #ifdef DEBUG_PING_PONG
        else std::cerr << ">> wrote sample " << _data.seq_num << std::endl;
      #endif

        return true;
    }
};

/*********************************************************
 * CycloneDDSSubscriber
 */

template <typename T>
class CycloneDDSSubscriber : public IMessagingReader
{
public:
    dds_entity_t _reader;
    ParameterManager *_PM;
    TestMessage _message;
    int _dataIdx;
    bool _noData;
    bool _endTest;
    bool _useReceiveThread;
    dds_entity_t waitSet;
    dds_attach_t waitSetResults[2];
    void *samplePointers[MAX_SAMPLES];
    T samples[MAX_SAMPLES];
    dds_sample_info_t sampleInfo[MAX_SAMPLES];

    CycloneDDSSubscriber(dds_entity_t reader, dds_entity_t participant, ParameterManager *PM)
            : _reader(reader),
              _PM(PM),
              _message(),
              _dataIdx(0),
              _noData(true),
              _endTest(false)
    {

        memset(samples, 0, sizeof(samples));
        for (unsigned int i = 0; i < MAX_SAMPLES; i++) {
            samplePointers[i] = &samples[i];
        }
        /*
         * This is the way used in the cycloneDDS to see if a Listener is set.
         * You cannot check if the listener you get is null (that would return
         * a "RETCODE_BAD_PARAMETER" errorCode), so you need to get it and
         * check if there is a valid pointer to the specific callback you are
         * looking for is not DDS_LUNSET
         */
        dds_listener_t *listener = dds_create_listener(nullptr);
        dds_return_t retCode = dds_get_listener(_reader, listener);

        if (retCode != DDS_RETCODE_OK) {
            fprintf(stderr, "[ERROR] dds_get_listener (retCode %d)\n", retCode);
        }

        void *callback;
        dds_lget_data_available (listener, (dds_on_data_available_fn*)&callback);
        _useReceiveThread = (callback == DDS_LUNSET);

        if (_useReceiveThread) {

            waitSet = dds_create_waitset(participant);
            dds_entity_t rdcond =
                    dds_create_readcondition(_reader, DDS_ANY_STATE);
            retCode = dds_waitset_attach(waitSet, rdcond, _reader);
            if (retCode < 0) {
                fprintf(stderr,
                        "[ERROR] dds_waitset_attach: %s\n",
                        dds_strretcode(-retCode));
            }

            retCode = dds_waitset_attach(waitSet, waitSet, waitSet);
            if (retCode < 0) {
                fprintf(stderr,
                        "[ERROR] dds_waitset_attach: %s\n",
                        dds_strretcode(-retCode));
            }
        }
    }

    ~CycloneDDSSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_useReceiveThread) {
            dds_return_t retCode = dds_waitset_detach(waitSet, waitSet);
            if (retCode < 0) {
                fprintf(stderr,
                        "[ERROR] dds_waitset_detach: %s\n",
                        dds_strretcode(-retCode));
            }
            retCode = dds_delete(waitSet);
            if (retCode < 0) {
                fprintf(stderr,
                        "[ERROR] dds_delete waitset: %s\n",
                        dds_strretcode(-retCode));
            }
        }
    }

    void WaitForWriters(int numPublishers)
    {
        dds_subscription_matched_status_t status;
        dds_get_subscription_matched_status(_reader, &status);
        while (status.total_count < numPublishers) {
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
            dds_get_subscription_matched_status(_reader, &status);
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
        dds_return_t retCode;
        int seqLength;

        while (!_endTest) {
            // no outstanding reads
            if (_noData) {
                retCode = dds_waitset_wait(
                        waitSet,
                        waitSetResults,
                        sizeof(waitSetResults) / sizeof(waitSetResults[0]),
                        DDS_INFINITY);
                if (retCode < 0) {
                    printf("dds_waitset_wait RetCode: %s\n",
                           dds_strretcode(-retCode));
                    continue;
                }

                seqLength = dds_take(
                        _reader,
                        samplePointers,
                        sampleInfo,
                        MAX_SAMPLES,
                        MAX_SAMPLES);
                if (seqLength < 0) {
                    fprintf(stderr,
                            "[Error]: dds_take error %s\n",
                            dds_strretcode(-seqLength));
                    return nullptr;
                }

                _dataIdx = 0;
                _noData = false;
            }

            // check to see if hit end condition
            if (_dataIdx == seqLength) {
                _noData = true;
                continue;
            }

            // skip non-valid data
            while ((sampleInfo[_dataIdx].valid_data == false)
                   && (++_dataIdx < seqLength)) {
                // No operation required
            }

            // may have hit end condition
            if (_dataIdx == seqLength) {
                continue;
            }

            T *valid_sample = (T *) (&samples[_dataIdx]);

            _message.entity_id = valid_sample->entity_id;
            _message.seq_num = valid_sample->seq_num;
            _message.timestamp_sec = valid_sample->timestamp_sec;
            _message.timestamp_usec = valid_sample->timestamp_usec;
            _message.latency_ping = valid_sample->latency_ping;
            _message.size = valid_sample->bin_data._length;
            _message.data = (char *) valid_sample->bin_data._buffer;

            ++_dataIdx;

            return &_message;
        }  // end while
        return nullptr;
    }
};

/*********************************************************
 * configure_participant_qos
 */
template <typename T>
bool CycloneDDSImpl<T>::configure_participant_qos(dds_qos_t *qos)
{
    /*
     * Empty. We leave this function as a placeHolder in case we need to add
     * Partipant QoS settings.
     */
    return true;
}


/*********************************************************
 * set_cycloneDDS_URI
 */
template <typename T>
bool CycloneDDSImpl<T>::set_cycloneDDS_URI()
{

    std::ostringstream stringStream;

    // Verbosity Information
    stringStream << _verbosityString;

    /*
     * CycloneDDS/Domain/Discovery/Peers/Peer
     *
     * CycloneDDS/Domain/Internal/SendAsync
     * This element controls whether the actual sending of packets occurs
     * on the same thread that prepares them, or is done asynchronously by
     * another thread.
     */

    //==========================================================================
    // Everything that goes under "<Internal>"
    stringStream << "<Internal>";

    stringStream << "<HeartbeatInterval>10 ms</HeartbeatInterval>";

    // MaxQueuedRexmitMessages By default is 200
    stringStream << "<MaxQueuedRexmitMessages>1000</MaxQueuedRexmitMessages>";

    //SynchronousDeliveryPriorityThreshold The default value is: "inf"
    stringStream << "<SynchronousDeliveryPriorityThreshold>"
                 << (_PM->is_set("asynchronousReceive")? "1":"0")
                 << "</SynchronousDeliveryPriorityThreshold>";

    stringStream << "</Internal>";

    //==========================================================================
    // Everything that goes under "<General>"
    stringStream << "<General>";

    // Transport Specific Configurations
    if (_PM->is_set("allowInterfaces")) {
        stringStream << "<NetworkInterfaceAddress>"
                     << _PM->get<std::string>("allowInterfaces")
                     << "</NetworkInterfaceAddress>";
    }

    if (_PM->is_set("multicast")) {
        stringStream << "<AllowMulticast>true</AllowMulticast>"
                     << "<EnableMulticastLoopback>true</EnableMulticastLoopback>"
                     << "<PreferMulticast>true</PreferMulticast>";
    }

    // MaxMessageSize The default value is: "14720 B"
    stringStream << "<MaxMessageSize>65500B</MaxMessageSize>";
    // FragmentSize The default value is: "1344 B". They use 4000B in their test
    stringStream << "<FragmentSize>65500B</FragmentSize>";

    stringStream << "</General>";

    //==========================================================================
    // Everything that goes under "<Sizing>"
    stringStream << "<Sizing>";
    // ReceiveBufferSize The default value is: "1 MiB"
    stringStream << "<ReceiveBufferSize>2097152B</ReceiveBufferSize>";
    stringStream << "</Sizing>";

    if (_PM->is_set("printCycloneDdsUriXml")) {
        std::cerr << std::endl
                  << "CYCLONEDDS_URI:" << std::endl
                  << stringStream.str() << std::endl;
    }

    dds_return_t retCode = ddsrt_setenv(
            "CYCLONEDDS_URI",
            stringStream.str().c_str());
    if (retCode != DDS_RETCODE_OK) {
        fprintf(stderr,"dds_set_status_mask error (retCode %d)\n", retCode);
        return false;
    }

    return true;
}

/*********************************************************
 * configure_writer_qos
 */
template <typename T>
bool CycloneDDSImpl<T>::configure_writer_qos(
        dds_qos_t *qos,
        std::string qosProfile)
{

    // RELIABILITY AND DURABILITY
    if (qosProfile != "AnnouncementQos") {

        if (_PM->get<bool>("bestEffort")) {
            qos->reliability.kind = DDS_RELIABILITY_BEST_EFFORT;
        } else {
            qos->reliability.kind = DDS_RELIABILITY_RELIABLE;
            qos->reliability.max_blocking_time = DDS_INFINITY;
        }

        if (_PM->is_set("durability")) {
            qos->durability.kind = (dds_durability_kind) _PM->get<int>("durability");
        } else {
            qos->durability.kind = DDS_DURABILITY_VOLATILE;
        }

        // HISTORY
        if (_PM->is_set("keepLast") || _PM->is_set("keepLastDepth")) {
            qos->history.kind = DDS_HISTORY_KEEP_LAST;
            if (_PM->is_set("keepLastDepth")) {
                qos->history.depth = _PM->get<int>("keepLastDepth");
            } else {
                qos->history.depth = _PM->get<int>("sendQueueSize");
            }
        } else {
            qos->history.kind = DDS_HISTORY_KEEP_ALL;
        }

    } else { // AnnouncementQoS
        qos->reliability.kind = DDS_RELIABILITY_RELIABLE;
        qos->durability.kind = DDS_DURABILITY_TRANSIENT_LOCAL;
    }

    // RESOURCE LIMITS
    if (qosProfile == "ThroughputQos") {
        qos->resource_limits.max_samples = _PM->get<int>("sendQueueSize");
        if (_PM->get<bool>("keyed")) {
            qos->resource_limits.max_instances = _PM->get<long>("instances");
        }
    } else if (qosProfile == "LatencyQos") {
        // qos->resource_limits.max_samples = 0;
    } else if (qosProfile == "AnnouncementQos") {
        // qos->resource_limits.max_samples = 0;
    } else {
        fprintf(stderr,
                "[Error]: Cannot find settings for qosProfile %s\n",
                qosProfile.c_str());
        return false;
    }

    /* Enable write batching, by default disabled */
    if (_PM->is_set("enableBatching")) {
        dds_write_set_batch (true);
    }

    // SUMMARY FOR THE RESOURCE LIMITS
    if (_PM->get<bool>("showResourceLimits")) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DW ("
                    << qosProfile
                    << " topic):\n"
                    << "\tSamples (Max): "
                    << qos->resource_limits.max_samples
                    << "\n";

        if (_PM->get<bool>("keyed")) {
            stringStream << "\tInstances (Max): "
                        << qos->resource_limits.max_instances
                        << "\n";
            stringStream << "\tMax Samples per Instance: "
                        << qos->resource_limits.max_samples_per_instance
                        << "\n";

        }
        stringStream << "\tDurability is: "
                    << qos->durability.kind
                    << "\n";
        stringStream << "\tHistory is: "
                    << qos->history.kind
                    << "\n";
        stringStream << "\tReliability is: "
                    << qos->reliability.kind
                    << "\n";
        stringStream << "\tMax blocking time is (s/ns): "
                    << qos->reliability.max_blocking_time
                    << "\n";
        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }
    return true;
}

/*********************************************************
 * configure_reader_qos
 */
template <typename T>
bool CycloneDDSImpl<T>::configure_reader_qos(
        dds_qos_t *qos,
        std::string qosProfile)
{

    // RELIABILITY AND DURABILITY
    if (qosProfile != "AnnouncementQos") {

        if (_PM->get<bool>("bestEffort")) {
            qos->reliability.kind = DDS_RELIABILITY_BEST_EFFORT;
        } else {
            qos->reliability.kind = DDS_RELIABILITY_RELIABLE;
            qos->reliability.max_blocking_time = DDS_INFINITY;
        }

        if (_PM->is_set("durability")) {
            qos->durability.kind = (dds_durability_kind) _PM->get<int>("durability");
        } else {
            qos->durability.kind = DDS_DURABILITY_VOLATILE;
        }

        // HISTORY
        if (_PM->is_set("keepLast") || _PM->is_set("keepLastDepth")) {
            qos->history.kind = DDS_HISTORY_KEEP_LAST;
            if (_PM->is_set("keepLastDepth")) {
                qos->history.depth = _PM->get<int>("keepLastDepth");
            } else {
                qos->history.depth = _PM->get<int>("sendQueueSize");
            }
        } else {
            qos->history.kind = DDS_HISTORY_KEEP_ALL;
        }

    } else { // AnnouncementQoS
        qos->reliability.kind = DDS_RELIABILITY_RELIABLE;
        qos->durability.kind = DDS_DURABILITY_TRANSIENT_LOCAL;
    }

    // RESOURCE LIMITS
    if (qosProfile == "ThroughputQos") {
        if (_PM->is_set("receiveQueueSize")) {
            qos->resource_limits.max_samples = _PM->get<int>("receiveQueueSize");
        }
        qos->resource_limits.max_samples = 10000;
    } else if (qosProfile == "LatencyQos") {
        qos->resource_limits.max_samples = 100;
    } else if (qosProfile == "AnnouncementQos") {
        qos->resource_limits.max_samples = 100;
    } else {
        fprintf(stderr,
                "[Error]: Cannot find settings for qosProfile %s\n",
                qosProfile.c_str());
        return false;
    }

    if (_PM->is_set("receiveQueueSize")) {
        qos->resource_limits.max_samples = _PM->get<int>("receiveQueueSize");
    }

    if (_PM->get<bool>("showResourceLimits")) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DR ("
                    << qosProfile
                    << " topic):\n"
                    << "\tSamples (Max): "
                    << qos->resource_limits.max_samples
                    << "\n";

        if (_PM->get<bool>("keyed")){
            stringStream << "\tInstances (Max): "
                        << qos->resource_limits.max_instances
                        << "\n";
            stringStream << "\tMax Samples per Instance: "
                        << qos->resource_limits.max_samples_per_instance
                        << "\n";

        }
        stringStream << "\tDurability is: "
                    << qos->durability.kind
                    << "\n";
        stringStream << "\tHistory is: "
                    << qos->history.kind
                    << "\n";
        stringStream << "\tReliability is: "
                    << qos->reliability.kind
                    << "\n";
        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }

    return true;
}

/*********************************************************
 * Initialize
 */
template <typename T>
bool CycloneDDSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    // Assign ParameterManager
    _PM = &PM;
    _transport.initialize(_PM);
    dds_return_t retCode;

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

    /*
     * Prior to create any Cyclone DDS entity, we will set the CYCLONEDDS_URI
     * which is used to pick certain QoS and settings, like the verbosity,
     * the nic or the HB periods.
     */
    if (!set_cycloneDDS_URI()) {
        return false;
    }


    dds_qos_t *participantQos;
    participantQos = dds_create_qos();
    if (!configure_participant_qos(participantQos)) {
        return false;
    }

    dds_listener_t *participantlistener = nullptr;
    participantlistener = dds_create_listener(nullptr);
    dds_lset_inconsistent_topic(
            participantlistener,
            participant_on_inconsistent_topic);
    dds_lset_offered_incompatible_qos(
            participantlistener,
            participant_on_offered_incompatible_qos);
    dds_lset_requested_incompatible_qos(
            participantlistener,
            participant_on_requested_incompatible_qos);
    // Create the participant
    _participant = dds_create_participant(
            _PM->get<int>("domain"),
            participantQos,
            participantlistener);
    dds_delete_qos(participantQos);
    if (_participant < 0) {
        fprintf(stderr,
                "Problem creating participant: %s\n",
                dds_strretcode(-_participant));
        return false;
    }

    dds_qos_t *publisherQos;
    publisherQos = dds_create_qos();
    publisherQos->presentation.access_scope =
            dds_presentation_access_scope_kind::DDS_PRESENTATION_TOPIC;
    publisherQos->presentation.ordered_access = true;
    _publisher = dds_create_publisher(_participant, publisherQos, nullptr);
    dds_delete_qos(publisherQos);
    if (_publisher < 0) {
        fprintf(stderr,
                "Problem creating publisher: %s\n",
                dds_strretcode(-_publisher));
        return false;
    }

    dds_qos_t *subscriberQos;
    subscriberQos = dds_create_qos();
    subscriberQos->presentation.access_scope =
            dds_presentation_access_scope_kind::DDS_PRESENTATION_TOPIC;
    subscriberQos->presentation.ordered_access = true;
    _subscriber = dds_create_subscriber(_participant, subscriberQos, nullptr);
    dds_delete_qos(subscriberQos);
    if (_subscriber < 0) {
        fprintf(stderr,
                "Problem creating subscriber: %s\n",
                dds_strretcode(-_subscriber));
        return false;
    }

    return true;
}

/*********************************************************
 * GetInitializationSampleCount
 */
template <typename T>
unsigned long CycloneDDSImpl<T>::GetInitializationSampleCount()
{
    return 50;
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *CycloneDDSImpl<T>::CreateWriter(const char *topicName)
{
    if (_participant < 0) {
        fprintf(stderr, "Participant is not valid\n");
        return nullptr;
    }

    dds_entity_t topic = dds_create_topic(
            _participant,
            &_topicDescriptor,
            topicName,
            nullptr,
            nullptr);

    if (topic < 0) {
        fprintf(stderr,
                "Problem creating topic %s. %s\n",
                topicName,
                dds_strretcode(-topic));
        return nullptr;
    }

    std::string qosProfile = get_qos_profile_name(topicName);
    if (qosProfile.empty()) {
        fprintf(stderr, "Problem getting qos profile (%s).\n", topicName);
        return nullptr;
    }

    dds_qos_t *dwQos;
    dwQos = dds_create_qos();
    if (!configure_writer_qos(dwQos, qosProfile)) {
        fprintf(stderr,
                "Problem creating additional QoS settings with %s profile.\n",
                qosProfile.c_str());
        return nullptr;
    }

    dds_entity_t writer = dds_create_writer(_publisher, topic, dwQos, nullptr);
    dds_delete_qos(dwQos);
    if (writer < 0) {
        fprintf(stderr,
                "Problem creating writer: %s\n",
                dds_strretcode(-writer));
        return nullptr;
    }
    dds_return_t retCode = dds_set_status_mask(
            writer,
            DDS_PUBLICATION_MATCHED_STATUS);
    if (retCode != DDS_RETCODE_OK) {
        fprintf(stderr,"dds_set_status_mask error (retCode %d)\n", retCode);
    }


    return new CycloneDDSPublisher<T>(
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
IMessagingReader *CycloneDDSImpl<T>::CreateReader(
        const char *topicName,
        IMessagingCB *callback)
{

   if (_participant < 0) {
        fprintf(stderr, "Participant is not valid\n");
        return nullptr;
    }

    dds_entity_t topic = dds_create_topic(
            _participant,
            &_topicDescriptor,
            topicName,
            nullptr,
            nullptr);

    if (topic < 0) {
        fprintf(stderr,
                "Problem creating topic %s. %s\n",
                topicName,
                dds_strretcode(-topic));
        return nullptr;
    }

    std::string qosProfile = get_qos_profile_name(topicName);
    if (qosProfile.empty()) {
        fprintf(stderr, "Problem getting qos profile (%s).\n", topicName);
        return nullptr;
    }

    dds_qos_t *drQos;
    drQos = dds_create_qos();
    if (!configure_reader_qos(drQos, qosProfile)) {
        fprintf(stderr,
                "Problem creating additional QoS settings with %s profile.\n",
                qosProfile.c_str());
        return nullptr;
    }

    dds_listener_t *dataReaderListener = nullptr;
    if (callback != nullptr) {
        ReaderListener<T> *listenerInfo = new ReaderListener<T>(callback);
        dataReaderListener = dds_create_listener(listenerInfo);
        dds_lset_data_available(dataReaderListener, reader_on_data_available<T>);
    }

    dds_entity_t reader = dds_create_reader(
            _subscriber,
            topic,
            drQos,
            dataReaderListener);
    dds_delete_qos(drQos);
    if (reader < 0) {
        fprintf(stderr,
                "Problem creating writer: %s\n",
                dds_strretcode(-reader));
        return nullptr;
    }
    dds_return_t retCode = dds_set_status_mask(
            reader,
            DDS_SUBSCRIPTION_MATCHED_STATUS | DDS_DATA_AVAILABLE_STATUS);
    if (retCode != DDS_RETCODE_OK) {
        fprintf(stderr,"dds_set_status_mask error (retCode %d)\n", retCode);
    }

    return new CycloneDDSSubscriber<T>(reader, _participant, _PM);
}

template <typename T>
const std::string CycloneDDSImpl<T>::get_qos_profile_name(const char *topicName)
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

template class CycloneDDSImpl<TestDataKeyedLarge_t>;
template class CycloneDDSImpl<TestDataLarge_t>;
