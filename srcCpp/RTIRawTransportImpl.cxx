/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "RTIRawTransportImpl.h"

#if defined(RTI_WIN32)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif

std::vector<NDDS_Transport_SendResource_t> PeerData::resourcesList;

/*********************************************************
 * Constructor
 */
RTIRawTransportImpl::RTIRawTransportImpl()
        : _transport(),
          _pongSemaphore(NULL),
          _plugin(NULL),
          _workerFactory(NULL),
          _exclusiveArea(NULL),
          _tssFactory(NULL),
          _PM(NULL)
{

    // Reserve space for the peers
    PeerData::resourcesList.reserve(RTIPERFTEST_MAX_PEERS);

}

/*********************************************************
 * Shutdown
 */
void RTIRawTransportImpl::Shutdown()
{

    for (unsigned int i = 0; i < PeerData::resourcesList.size(); i++) {
        if (PeerData::resourcesList[i] != NULL && _plugin != NULL) {
            _plugin->destroy_sendresource_srEA(
                    _plugin, _peersDataList[i].resource);
            PeerData::resourcesList[i] = NULL;
        }
    }

    if (_plugin != NULL) {
        _plugin->delete_cEA(_plugin, NULL);
    }
    if (_exclusiveArea != NULL){
        REDAWorkerFactory_destroyExclusiveArea(_workerFactory, _exclusiveArea);
    }
    if (_workerFactory != NULL) {
        REDAWorkerFactory_delete(_workerFactory);
    }
    if (_tssFactory != NULL){
        RTIOsapiThread_deleteTssFactory(_tssFactory);
    }
    if (_pongSemaphore != NULL) {
        RTIOsapiSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }

}


/*********************************************************
 * Getters
 */
unsigned long RTIRawTransportImpl::GetInitializationSampleCount()
{
    return 0;
}

NDDS_Transport_Plugin *RTIRawTransportImpl::get_plugin()
{
    return _plugin;
}

std::vector<PeerData> RTIRawTransportImpl::get_peers_data()
{
    return _peersDataList;
}

RTIOsapiSemaphore *RTIRawTransportImpl::get_pong_semaphore()
{
    return _pongSemaphore;
}
struct REDAWorkerFactory *RTIRawTransportImpl::get_worker_factory()
{
    return _workerFactory;
}

RTIOsapiThreadTssFactory *RTIRawTransportImpl::get_tss_factory()
{
    return _tssFactory;
}

ParameterManager *RTIRawTransportImpl::get_parameter_manager()
{
    return _PM;
}

/*********************************************************
 * SupportFunctions
 */
bool RTIRawTransportImpl::SupportsListener()
{
    return false;
}

bool RTIRawTransportImpl::SupportsDiscovery()
{
    return false;
}

/*********************************************************
 * validate_input
 */
bool RTIRawTransportImpl::validate_input() {

    // Manage parameter -batchSize
    if (_PM->get<long>("batchSize") > 0) {
        // We will not use batching for a latency test
        if (_PM->get<bool>("latencyTest")) {
            if (_PM->is_set("batchSize")) {
                fprintf(stderr, "Batching cannot be used in a Latency test.\n");
                return false;
            } else {
                _PM->set<long>("batchSize", 0); // Disable Batching
            }
        }
    }

    if ((unsigned long) _PM->get<long>("batchSize")
        < _PM->get<unsigned long long>("dataLen") * 2) {
        /*
         * We don't want to use batching if the batch size is not large
         * enough to contain at least two samples (in this case we avoid the
         * checking at the middleware level).
         */
        if (_PM->is_set("batchSize") || _PM->is_set("scan")) {
            /*
             * Batchsize disabled. A message will be print if _batchSize < 0
             * in perftest_cpp::PrintConfiguration()
             */
            _PM->set<long>("batchSize", -1);
        } else {
            _PM->set<long>("batchSize", 0);  // Disable Batching
        }
    }


    if (_PM->get<unsigned long long>("dataLen")
            > (unsigned long) MAX_SYNCHRONOUS_SIZE) {
        fprintf(stderr,
                "The maximun dataLen for rawTransport is %d.\n",
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);
        return false;
    }

    /*
     * Manage parameter -verbosity.
     * Setting verbosity if the parameter is provided
     */
    if (_PM->is_set("verbosity")) {
        switch (_PM->get<int>("verbosity")) {
            case 0:
                NDDSConfigLogger::get_instance()->set_verbosity(
                        NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
            case 1:
                NDDSConfigLogger::get_instance()->set_verbosity(
                        NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
            case 2:
                NDDSConfigLogger::get_instance()->set_verbosity(
                        NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
            case 3: NDDSConfigLogger::get_instance()->set_verbosity(
                        NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
            default:
                fprintf(stderr, "Invalid value for the '-verbosity' parameter\n");
                return false;
        }
    }

    // Manage transport parameter
    if (!_transport.validate_input()) {
        fprintf(stderr, "Failure validation the transport options.\n");
        return false;
    };

    // Manage parameter -peerRT
    if (_PM->get_vector<std::string>("peerRT").size() >= RTIPERFTEST_MAX_PEERS) {
        fprintf(stderr,
                "The maximun of 'initial_peers' is %d\n",
                RTIPERFTEST_MAX_PEERS);
        return false;
    }

    // Manage parameter -multicast
    if (_PM->get<bool>("multicast")
            && _PM->get_vector<std::string>("peerRT").size() > 0) {
        fprintf(stderr,
                "\tFor multicast, if you want to send to other IP, "
                "use multicastAddr\n");
        return false;
    }

    // Manage parameter -noBlockingSockets
    if (_PM->get<bool>("noBlockingSockets")
            && _PM->get<std::string>("transport") == "SHMEM") {
        fprintf(stderr, "SHMEM dont support -noBlockingSockets\n");
        return false;
    }

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
std::string RTIRawTransportImpl::PrintConfiguration()
{
    std::ostringstream stringStream;

    // Meedleware
    stringStream << "\tMiddleware: RawTransport\n";

    // Domain ID
    stringStream << "\tDomain: " << _PM->get<int>("domain") << "\n";

    stringStream << "\n" << _transport.printTransportConfigurationSummary();

    // Blocking sockets
    stringStream << "\tBlocking Sockets: "
                 << (_PM->get<bool>("noBlockingSockets")? "Yes\n" : "No\n");

    // Ports
    stringStream << "\tThe following ports will be used: ";
    if (_PM->get<bool>("pub")) {
        stringStream << getReceiveUnicastPort(ANNOUNCEMENT_TOPIC_NAME) << " - "
                     << getReceiveUnicastPort(LATENCY_TOPIC_NAME) << "\n";
    } else {
        stringStream << getReceiveUnicastPort(THROUGHPUT_TOPIC_NAME) << "\n";
    }

    // Set initial peers
    if (_PM->get_vector<std::string>("peerRT").size() > 0 && !is_multicast()) {
        stringStream << "\tInitial peers: ";
        for (unsigned int i = 0;
                i < _PM->get_vector<std::string>("peerRT").size(); ++i) {
            stringStream << _PM->get_vector<std::string>("peerRT")[i];
            stringStream
                    << ((i + 1 == _PM->get_vector<std::string>("peerRT").size())
                            ? "\n"
                            : ", ");
        }
    }

    return stringStream.str();
}

/*********************************************************
 * RTIPublisher
 */
class RTIRawTransportPublisher : public IMessagingWriter {
  private:

    /* --- Transport members --- */
    RTIRawTransportImpl *_parent;
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_Buffer_t _sendBuffer;
    struct REDAWorker *_worker;
    unsigned int _workerTssKey;

    /* --- Perftest members --- */
    std::vector<PeerData> _peersDataList;
    TestData_t _data;
    RTIOsapiSemaphore *_pongSemaphore;
    ParameterManager *_PM;

    /* --- Buffers management --- */
    unsigned int _batchBufferSize;
    bool _useBatching;
    struct RTICdrStream _stream;
    struct PRESTypePluginDefaultEndpointData _endPointData;


  public:
    RTIRawTransportPublisher(RTIRawTransportImpl *parent)
            : _parent(parent),
              _worker(NULL),
              _workerTssKey(0)
    {
        _plugin = parent->get_plugin();
        _peersDataList = parent->get_peers_data();
        _pongSemaphore = parent->get_pong_semaphore();
        _PM = parent->get_parameter_manager();

        if (_PM->get<long>("batchSize") <= 0) {
            _batchBufferSize = NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
            _useBatching = false;
        } else {
            _batchBufferSize = _PM->get<long>("batchSize");
            _useBatching = true;
        }

        RTIOsapiHeap_allocateBuffer(
            &_sendBuffer.pointer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

        if (_sendBuffer.pointer == NULL) {
            Shutdown();
            throw std::runtime_error(
                    "Error allocating memory for the send buffer\n");
        }
        _sendBuffer.length = 0;


        RTICdrStream_init(&_stream);
        RTICdrStream_set(
                &_stream,
                (char *)_sendBuffer.pointer,
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        _endPointData._maxSizeSerializedSample
                = TestData_tPlugin_get_serialized_sample_max_size(
                        NULL,
                        RTI_TRUE,
                        RTICdrEncapsulation_getNativeCdrEncapsulationId(),
                        0);
    }

    ~RTIRawTransportPublisher()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_sendBuffer.pointer != NULL) {
            RTIOsapiHeap_freeBuffer(_sendBuffer.pointer);
            _sendBuffer.pointer = NULL;
        }

        if (_worker != NULL) {
            if (_parent->get_worker_factory() != NULL) {
                REDAWorkerFactory_destroyWorker(
                        _parent->get_worker_factory(),
                        _worker);
            } else {
                fprintf(stderr, "Error, workerFactory destroy before worker\n");
            }
        }
    }

    bool SendMessage()
    {
        bool success = true;

        if (_worker == NULL) {
            _worker = raw_transport_get_worker_per_thread(
                    _parent->get_worker_factory(),
                    _parent->get_tss_factory(),
                    &_workerTssKey);
        }

        for(unsigned int i = 0; i < _peersDataList.size(); i++) {
            if(!_plugin->send(
                    _plugin,
                    _peersDataList[i].resource,
                    &_peersDataList[i].transportAddr,
                    _peersDataList[i].port,
                    NDDS_TRANSPORT_PRIORITY_DEFAULT,
                    &_sendBuffer,
                    1, /* Number of buffer on buffer_in (_sendBuffer) */
                    _worker)){
                success = false;
                /*
                 * No need of print error. This wil be represented as lost
                 * packets
                 */
            }

        }

        return success;
    }

    void Flush()
    {
        /* If there is no data, no need to flush */
        if (_sendBuffer.length == 0) {
            return;
        }

        SendMessage();
        /* No need of print error. This will be represented as lost packets */

        /* If the send is done, reset the stream to fill the buffer again */
        RTICdrStream_set(
                &_stream,
                (char *)_sendBuffer.pointer,
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        _sendBuffer.length = 0;
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        RTIBool success = false;
        int serializeSize =  message.size
                + perftest_cpp::OVERHEAD_BYTES
                + RTI_CDR_ENCAPSULATION_HEADER_SIZE;

        _data.entity_id = message.entity_id;
        _data.seq_num = message.seq_num;
        _data.timestamp_sec = message.timestamp_sec;
        _data.timestamp_usec = message.timestamp_usec;
        _data.latency_ping = message.latency_ping;
        _data.bin_data.loan_contiguous(
                (DDS_Octet *) message.data,
                message.size,
                message.size);

        /*
         * If there is no more space on the buffer to allocate the new message
         * flush before add a new one.
         */
        if ((unsigned int) (_sendBuffer.length + serializeSize)
                > _batchBufferSize) {
            Flush();
        }

        success = TestData_tPlugin_serialize(
                (PRESTypePluginEndpointData) &_endPointData,
                &_data,
                &_stream,
                RTI_TRUE,
                RTICdrEncapsulation_getNativeCdrEncapsulationId(),
                RTI_TRUE,
                NULL);

        /* _data is been serialize (copied). It's right to unloan then. */
        _data.bin_data.unloan();

        if (!success) {
            fprintf(stderr, "Fail to serialize data\n");
            return false;
        }

        _sendBuffer.length = RTICdrStream_getCurrentPositionOffset(&_stream);

        if (!_useBatching) {
            Flush();
        }

        return true;
    }

    void WaitForReaders(int numSubscribers) {
        /* --- Dummy Function --- */
    }

    bool waitForPingResponse() {
        if (_pongSemaphore != NULL) {
            if (!RTIOsapiSemaphore_take(_pongSemaphore, NULL)) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout) {
        struct RTINtpTime blockDurationIn;
        RTINtpTime_packFromMillisec(blockDurationIn, 0, timeout);

        if (_pongSemaphore != NULL) {
            if (!RTIOsapiSemaphore_take(_pongSemaphore, &blockDurationIn)) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse() {
        if (_pongSemaphore != NULL) {
            if (!RTIOsapiSemaphore_give(_pongSemaphore)) {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount() {
        /* --- Dummy Function --- */
        return 0;
    };

    void wait_for_acknowledgments(long sec, unsigned long nsec) {
        /* --- Dummy Function --- */
    }
};

/*********************************************************
 * RTISubscriber
 */
class RTIRawTransportSubscriber : public IMessagingReader
{
  private:
    /* --- Transport members --- */
    RTIRawTransportImpl *_parent;
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_SendResource_t _recvResource;
    NDDS_Transport_Buffer_t _recvBuffer;
    NDDS_Transport_Port_t _recvPort;
    NDDS_Transport_Message_t _transportMessage;
    struct REDAWorker *_worker;
    unsigned int _workerTssKey;

    /* --- Perftest members --- */
    TestMessage _message;
    TestData_t _data;
    char *_payload;
    int _payload_size;
    struct RTIOsapiSemaphore *_readThreadSemaphore;
    ParameterManager *_PM;

    /* --- Buffer Management --- */
    struct RTICdrStream _stream;
    bool _noData;


public:
    RTIRawTransportSubscriber(
        RTIRawTransportImpl * parent,
        NDDS_Transport_SendResource_t recvResource,
        NDDS_Transport_Port_t recvPort)
                : _parent(parent),
                  _recvResource(recvResource),
                  _recvPort(recvPort),
                  _worker(NULL),
                  _workerTssKey(0),
                  _readThreadSemaphore(NULL),
                  _noData(true)
    {
        /* --- Parents Members --- */
        _plugin = parent->get_plugin();
        _PM = parent->get_parameter_manager();

        /* --- Buffer Management --- */
        _recvBuffer.length = 0;
        _recvBuffer.pointer = NULL;

        /*
         * Similar to NDDS_Transport_Message_t message =
         *      NDDS_TRANSPORT_MESSAGE_INVALID;
         */
        _transportMessage.buffer.pointer = NULL;
        _transportMessage.buffer.length = 0;
        _transportMessage.loaned_buffer_param = NULL;

        /* --- Maximun size of UDP package --- */
        RTIOsapiHeap_allocateBufferAligned(
            &_recvBuffer.pointer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX,
            RTI_OSAPI_ALIGNMENT_DEFAULT);
        if (_recvBuffer.pointer == NULL) {
            Shutdown();
            throw std::runtime_error("RTIOsapiHeap_allocateBuffer Error\n");
        }

        _recvBuffer.length = NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;

        _data.bin_data.maximum(NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX
                - perftest_cpp::OVERHEAD_BYTES);

        RTICdrStream_init(&_stream);
    }

    ~RTIRawTransportSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {

        if (_recvBuffer.pointer != NULL) {
            RTIOsapiHeap_freeBufferAligned(_recvBuffer.pointer);
            _recvBuffer.pointer = NULL;
        }

        if (_recvResource != NULL && _plugin != NULL && _worker != NULL) {
            _plugin->unblock_receive_rrEA(_plugin, &_recvResource, _worker);

            if (_readThreadSemaphore != NULL) {
                if (RTIOsapiSemaphore_take(_readThreadSemaphore, NULL)
                        != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                    fprintf(stderr, "Unexpected error taking semaphore\n");
                    return;
                }
            }
            _plugin->destroy_recvresource_rrEA(_plugin, &_recvResource);

            /* Make a give before destroy the semaphore */
            if (_readThreadSemaphore != NULL) {
                if (RTIOsapiSemaphore_give(_readThreadSemaphore)
                        != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                    fprintf(stderr, "Unexpected error giving semaphore\n");
                    return;
                }
            }
        }

        if (_readThreadSemaphore != NULL) {
            RTIOsapiSemaphore_delete(_readThreadSemaphore);
            _readThreadSemaphore = NULL;
        }

        if (_worker != NULL) {
            if (_parent->get_worker_factory() != NULL) {
                REDAWorkerFactory_destroyWorker(
                        _parent->get_worker_factory(),
                        _worker);
            } else {
                fprintf(stderr, "Error, workerFactory destroy before worker\n");
            }
        }
    }

    TestMessage *ReceiveMessage() {

        int result = 0;

        if (_worker == NULL) {
            _worker = raw_transport_get_worker_per_thread(
                    _parent->get_worker_factory(),
                    _parent->get_tss_factory(),
                    &_workerTssKey);
        }

        while (true) {
            if (_noData) {
                result = 0;
                result = _plugin->receive_rEA(
                        _plugin,
                        &_transportMessage,
                        &_recvBuffer,
                        &_recvResource,
                        _worker);
                if (!result) {
                    /*
                     * If the _transportMessage.buffer.length == 0, this
                     * method was unblocked by the unblock_receive_rrEA() call.
                     * So it's not an error.
                     */
                    if (_transportMessage.buffer.length != 0) {
                        fprintf(stderr, "Fail to receive data\n");
                    }
                    return NULL;
                }

                RTICdrStream_set(&_stream,
                        (char *)_transportMessage.buffer.pointer,
                        _transportMessage.buffer.length);

                _noData = false;
            }

            /* May have hit end condition */
            if (RTICdrStream_getCurrentPositionOffset(&_stream)
                    >= _transportMessage.buffer.length) {
                if (_plugin->return_loaned_buffer_rEA != NULL
                    && _transportMessage.loaned_buffer_param != (void *) -1) {
                    _plugin->return_loaned_buffer_rEA(
                            _plugin,
                            &_recvResource,
                            &_transportMessage,
                            _worker);
                }

                _noData = true;
                continue;
            }

            TestData_tPlugin_deserialize_sample(
                    NULL,
                    &_data,
                    &_stream,
                    RTI_TRUE,
                    RTI_TRUE,
                    NULL);

            _message.entity_id = _data.entity_id;
            _message.seq_num = _data.seq_num;
            _message.timestamp_sec = _data.timestamp_sec;
            _message.timestamp_usec = _data.timestamp_usec;
            _message.latency_ping = _data.latency_ping;
            _message.size = _data.bin_data.length();
            _message.data = (char *)_data.bin_data.get_contiguous_bufferI();

            return &_message;

        }

    }

    struct RTIOsapiSemaphore *GetReadThreadSemaphore()
    {
        if (_readThreadSemaphore == NULL) {
            _readThreadSemaphore = RTIOsapiSemaphore_new(
                    RTI_OSAPI_SEMAPHORE_KIND_BINARY,
                    NULL);

            if (_readThreadSemaphore == NULL) {
                fprintf(stderr,
                        "Fail to create a Semaphore for RTIRawTransportImpl\n");
                return NULL;
            }
            if (RTIOsapiSemaphore_give(_readThreadSemaphore)
                    != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return NULL;
            }
        }

        return _readThreadSemaphore;
    }

    void WaitForWriters(int numPublishers) {
        /*Dummy Function*/
    }
};

/*********************************************************
 * Initialize
 */
bool RTIRawTransportImpl::Initialize(ParameterManager &PM)
{
    _PM = &PM;

    /* Set a default interface */
    if (_PM->get<std::string>("allowInterfaces").empty()) {
        _PM->set<std::string>("allowInterfaces", std::string("127.0.0.1"));
    }

    _transport.initialize(_PM);

    if (!validate_input()) {
        return false;
    }

    /* Only if we run latency test we need to wait for pongs after sending pings */
    if (_PM->get<bool>("latencyTest")) {
        _pongSemaphore =
                RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);

        if (_pongSemaphore == NULL) {
            fprintf(stderr,
                    "Fail to create a Semaphore for RTIRawTransportImpl\n");
            return NULL;
        }
    }

    if (!configure_sockets_transport()) {
        return false;
    }

    return true;
}

/*********************************************************
 * GetUnicastPort
 */
unsigned int RTIRawTransportImpl::get_send_unicast_port(
        const char *topicName,
        unsigned int subId)
{
    /* subId = 0  by default */
    unsigned int portOffset = 0;

    if (!strcmp(topicName, ANNOUNCEMENT_TOPIC_NAME)) {
        portOffset = 1;
    }

    struct DDS_RtpsWellKnownPorts_t wellKnownPorts =
            DDS_RTPS_WELL_KNOWN_PORTS_DEFAULT;

    return PRESRtps_getWellKnownUnicastPort(
            _PM->get<int>("domain"), /* domainId */
            _PM->get<bool>("pub") ? subId + 1 : 0, /* participantId */
            wellKnownPorts.port_base,
            wellKnownPorts.domain_id_gain,
            wellKnownPorts.participant_id_gain,
            wellKnownPorts.builtin_unicast_port_offset + portOffset);
}

unsigned int
RTIRawTransportImpl::getReceiveUnicastPort(const char *topicName)
{
    unsigned int portOffset = 0;

    if (!strcmp(topicName, ANNOUNCEMENT_TOPIC_NAME)) {
        portOffset = 1;
    }

    /* Get the default values to calculate the port the same way as do DDS */
    struct DDS_RtpsWellKnownPorts_t wellKnownPorts =
            DDS_RTPS_WELL_KNOWN_PORTS_DEFAULT;

    return PRESRtps_getWellKnownUnicastPort(
            _PM->get<int>("domain"), /* domainId */
            _PM->get<bool>("pub") ? 0 : perftest_cpp::_SubID + 1, /* participantId */
            wellKnownPorts.port_base,
            wellKnownPorts.domain_id_gain,
            wellKnownPorts.participant_id_gain,
            wellKnownPorts.builtin_unicast_port_offset + portOffset);
}


/*********************************************************
 * getMulticastTransportAddr
 */
bool RTIRawTransportImpl::getMulticastTransportAddr(
        const char *topicName,
        NDDS_Transport_Address_t &addr)
{
    /*  Precondition */
    if (!is_multicast()) {
        return false;
    }

    RTIBool success = true;
    RTIOsapiMemory_zero(&addr, sizeof(NDDS_Transport_Address_t));

    if (strcmp(topicName, THROUGHPUT_TOPIC_NAME) != 0
            && strcmp(topicName, LATENCY_TOPIC_NAME) != 0
            && strcmp(topicName, ANNOUNCEMENT_TOPIC_NAME) != 0) {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME,
                LATENCY_TOPIC_NAME,
                ANNOUNCEMENT_TOPIC_NAME);
        return false;
    }

    success = NDDS_Transport_UDP_string_to_address_cEA(
            _plugin,
            &addr,
            _transport.getMulticastAddr(topicName).c_str());

    return success == 1;
}

/*********************************************************
 * CreateWriter
 */
IMessagingWriter *RTIRawTransportImpl::CreateWriter(const char *topicName)
{

    NDDS_Transport_Address_t multicastAddr;
    bool is_multicastAddr = false;

    NDDS_Transport_Address_t actualAddr;
    int actualPort = 0;
    RTIBool shared = false;
    unsigned int j = 0;

    // If multicat, then take the multicast address.
    if (_PM->get<bool>("multicast")
            && getMulticastTransportAddr(topicName, multicastAddr)) {
        is_multicastAddr = true;
    } else if (_PM->get<bool>("multicast")) {
        fprintf(stderr, "Bad configuration for multicast (sockets)\n");
        return NULL;
    }

    /*
     * _PM->get_vector<std::string>("peerRT").size() is garanteed to be 1 if
     * multicast is enabled
     */
    for (unsigned int i = 0;
            i < _PM->get_vector<std::string>("peerRT").size(); i++) {
        shared = false;
        actualAddr = is_multicastAddr ? multicastAddr : _peersMap[i].first;

        // Calculate the port of the new send resource.
        actualPort = get_send_unicast_port(topicName, _peersMap[i].second);

        for (j = 0; j < PeerData::resourcesList.size() && !shared; ++j) {
            // Try to share the resource
            shared = _plugin->share_sendresource_srEA(
                    _plugin,
                    &PeerData::resourcesList[j],
                    &actualAddr,
                    actualPort,
                    NDDS_TRANSPORT_PRIORITY_DEFAULT);
        }
        if (!shared) {
            /* If the resource is not shared, then create a new one and store it */
            NDDS_Transport_SendResource_t resource;
            if (!_plugin->create_sendresource_srEA(
                    _plugin,
                    &resource,
                    &actualAddr,
                    actualPort,
                    NDDS_TRANSPORT_PRIORITY_DEFAULT)) {
                fprintf(
                        stderr,
                        "create_sendresource_srEA error.\n");
                return NULL;
            }

            PeerData::resourcesList.push_back(resource);
        }
        /* This data will be use by the writer to send to multiples peers. */
        _peersDataList.push_back(
                PeerData(
                        shared ? &PeerData::resourcesList[j-1]
                                : &PeerData::resourcesList.back(),
                        actualAddr,
                        actualPort));
    }
    try {
        return new RTIRawTransportPublisher(this);
    } catch (const std::exception &ex) {
        fprintf(stderr,
                "Exception in RTIRawTransportImpl::CreateWriter(): %s.\n",
                ex.what());
        return NULL;
    }
}

/*********************************************************
 * CreateReader
 */
IMessagingReader *
RTIRawTransportImpl::CreateReader(const char *topicName, IMessagingCB *callback)
{
    NDDS_Transport_RecvResource_t recvResource = NULL;
    NDDS_Transport_Port_t recvPort = 0;
    NDDS_Transport_Address_t multicastAddr;
    bool is_multicastAddr = false;
    RTIBool result = true;

    /* If multicat, then take the multicast address. */
    if (_PM->get<bool>("multicast")
            && getMulticastTransportAddr(topicName, multicastAddr)) {
        is_multicastAddr = true;
    } else if (_PM->get<bool>("multicast")) {
        fprintf(stderr, "Bad configuration for multicast (RawTransport)\n");
        return NULL;
    }

    /* Calculate the port of the new receive resource. */
    recvPort = getReceiveUnicastPort(topicName);

    result = _plugin->create_recvresource_rrEA(
            _plugin,
            &recvResource,
            &recvPort,
            (is_multicastAddr)? &multicastAddr : NULL,
            NDDS_TRANSPORT_PRIORITY_DEFAULT);

    if (!result) {
        fprintf(
            stderr,
            "Create_recvresource_rrEA error. Maybe the port %d is been use.\n"
            "Check if you have other pub/sub with the same id\n",
            recvPort);
        return NULL;
    }
    try {
        return new RTIRawTransportSubscriber(this, recvResource, recvPort);
    } catch (const std::exception &ex) {
        fprintf(stderr,
                "Exception in RTIRawTransportImpl::CreateReader(): %s.\n",
                ex.what());
        return NULL;
    }

}

bool RTIRawTransportImpl::configure_sockets_transport()
{
    char *interfaceAddr = NULL; /*WARNING: interface is a reserved word on VS*/
    interfaceAddr
            = DDS_String_dup(_PM->get<std::string>("allowInterfaces").c_str());
    std::vector<std::string> peers = _PM->get_vector<std::string>("peerRT");
    if (peers.empty()) {
        peers.push_back("127.0.0.1");
    }

    if (interfaceAddr == NULL) {
        fprintf(stderr,
                "Fail allocating memory on configure_sockets_transport\n");
        return false;
    }

    /* If no peer is given, assume a default one */
    if (_PM->get_vector<std::string>("peerRT").size() == 0) {
        std::vector<std::string> auxVector;
        auxVector.push_back(std::string( "127.0.0.1"));
        _PM->set<std::vector<std::string> >("peerRT", auxVector);
    }

    /* --- Worker configure --- */
    _workerFactory = REDAWorkerFactory_new(128); // Number of worker to store
    if (_workerFactory == NULL) {
        fprintf(stderr, "Fail to create Worker Factory\n");
        return false;
    }

    _exclusiveArea = REDAWorkerFactory_createExclusiveArea(_workerFactory, 1);
    if (_exclusiveArea == NULL) {
        fprintf(stderr, "Fail to create exclusive area\n");
        return false;
    }

    /* --- TssFactory --- */
    _tssFactory = RTIOsapiThread_createTssFactory();
    if (_tssFactory == NULL) {
        fprintf(stderr, "Fail to create thread-specific storage factory\n");
        return false;
    }

    switch (_transport.transportConfig.kind) {

    /* --- Transport configure --- */
    case TRANSPORT_NOT_SET:
        /*Default transport for sockets is UDPv4*/
        _transport.transportConfig.kind = TRANSPORT_UDPv4;
        _transport.transportConfig.nameString = "UDPv4";

        case TRANSPORT_UDPv4:
        {
            struct NDDS_Transport_UDPv4_Property_t udpv4_prop =
                    NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
            NDDS_Transport_Address_t addr;
            char addr_sub[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
            int id_sub = 0;

            /*_Plugin properties configure for UDPv4*/
            udpv4_prop.parent.allow_interfaces_list = &interfaceAddr;
            udpv4_prop.parent.allow_interfaces_list_length = 1;

            if (_PM->get<bool>("multicast")) {
                udpv4_prop.parent.allow_multicast_interfaces_list = &interfaceAddr;
                udpv4_prop.parent.allow_multicast_interfaces_list_length = 1;
                udpv4_prop.reuse_multicast_receive_resource = 1;
                udpv4_prop.multicast_enabled = 1;
                udpv4_prop.multicast_ttl = 1;
                udpv4_prop.unicast_enabled = 0;
            }

            if (!_PM->get<bool>("noBlockingSockets")) {
                udpv4_prop.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_ALWAYS;
            } else {
                /*
                 * This will reduce the package lost but affect on the
                 * performance
                 */
                udpv4_prop.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_NEVER;
            }

            udpv4_prop.no_zero_copy = false;
            udpv4_prop.parent.message_size_max =
                    NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
            /*
             * Minimum number of gather-send buffers that must be supported by a
             * Transport Plugin implementation.
             * Actually, we only use one.
             */
            udpv4_prop.parent.gather_send_buffer_count_max =
                    NDDS_TRANSPORT_PROPERTY_GATHER_SEND_BUFFER_COUNT_MIN;

            /*
             * The default value for send_socket_buffer_size and receive is
             * 131072 and does not allow to go lower.
             * NDDS_TRANSPORT_UDPV4_SEND_SOCKET_BUFFER_SIZE_DEFAULT   (131072)
             * NDDS_TRANSPORT_UDPV4_RECV_SOCKET_BUFFER_SIZE_DEFAULT   (131072)
             *
             * This could be necessary to modify for large data support.
             */

            /* Setting the transport properties */
            _plugin = NDDS_Transport_UDPv4_new(&udpv4_prop);

            if (_plugin == NULL) {
                fprintf(stderr, "Fail to create transport plugin\n");
                return false;
            }

            /*
             * This is to avoid the translation from a interface name in to
             * a address and then check if the given nic is correct.
             *
             * If you want to retrieve the address/name of the
             * interface:
             *      udpPlugin->_interfaceArray[0]._interface.address;
             *      udpPlugin->_interfaceArray[0]._interfaceName;
             */
            struct NDDS_Transport_UDP *udpPlugin;
            udpPlugin = (struct NDDS_Transport_UDP *) _plugin;
            if (udpPlugin->_interfacesCount == 0) {
                fprintf(stderr,
                        "Input interface (%s) not recognize\n",
                        interfaceAddr);
                return false;
            }

            /* Check if the multicast address is correct */
            if (_PM->get<bool>("multicast")
                    && get_num_multicast_interfaces(udpPlugin) <= 0) {
                fprintf(stderr,
                        "The interface (%s) does not have multicast-enabled\n",
                        interfaceAddr);
                return false;
            }

            /* Peers address and ID parse to NDDS_Transport_Address_t */
            for (unsigned int i = 0; i < peers.size(); i++) {
                id_sub = 0;
                /* Regular expression to identify ADDRESS:ID */
                sscanf(peers[i].c_str(), "%[^:]:%d", addr_sub, &id_sub);

                if (NDDS_Transport_UDP_string_to_address_cEA(
                        _plugin,
                        &addr,
                        addr_sub)) {
                    _peersMap.push_back(
                            std::pair<NDDS_Transport_Address_t, int>(
                                    addr, id_sub));
                }
            }

            if (_peersMap.size() == 0) {
                fprintf(stderr, "Any peer correspond to a valid address\n");
                return false;
            }

            break;
    }

    case TRANSPORT_SHMEM:
    {
        struct NDDS_Transport_Shmem_Property_t shmem_prop =
                NDDS_TRANSPORT_SHMEM_PROPERTY_DEFAULT;

        /* _Plugin configure for shared memory */
        shmem_prop.parent.message_size_max =
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
        shmem_prop.receive_buffer_size =
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX * 10;
        shmem_prop.received_message_count_max =
                shmem_prop.receive_buffer_size / perftest_cpp::OVERHEAD_BYTES;
        /*
         * The total number of bytes that can be buffered in the receive
         * queue is calculated with the following formula:
         * size = receive_buffer_size + message_size_max +
         *          received_message_count_max * fixedOverhead
         */

        _plugin = NDDS_Transport_Shmem_new(&shmem_prop);

        if (_plugin == NULL) {
            fprintf(stderr, "Fail to create transport plugin\n");
            return false;
        }

        /* Peers parse to NDDS_Transport_Address_t */
        NDDS_Transport_Address_t addr = NDDS_TRANSPORT_ADDRESS_INVALID;
        char addr_sub[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
        int id_sub = 0;

        for (unsigned int i = 0; i < peers.size(); i++) {
            id_sub = 0;
            sscanf(peers[i].c_str(), "%[^:]:%d", addr_sub, &id_sub);

            _peersMap.push_back(
                    std::pair<NDDS_Transport_Address_t, int>(addr, id_sub));
        }

        /*
         * For SHMEM we dont want to print any interface on the
         * printTransportConfigurationSummary()
         */
        _PM->set<std::string>("allowInterfaces", std::string(""));

        break;
    }

    case TRANSPORT_UDPv6:
    case TRANSPORT_TCPv4:
    case TRANSPORT_TLSv4:
    case TRANSPORT_DTLSv4:
    case TRANSPORT_WANv4:
    default:
        fprintf(stderr, "RawTransport only support UDPv4 & SHMEM\n");
        return false;

    } /* End Switch */



    _transport.printTransportConfigurationSummary();

    delete interfaceAddr;

    return true;
}

int get_num_multicast_interfaces(struct NDDS_Transport_UDP *plugin)
{
    int count = 0;

    if (plugin == NULL) {
        return count;
    }

    for (int i = 0; i < plugin->_interfacesCount; i++) {
        if (plugin->_interfaceArray[i]._interfaceFlags
                & RTI_OSAPI_SOCKET_INTERFACE_FLAG_MULTICAST) {
            ++count;
        }
    }
    return count;
}

/*
 * Get the worker for the current thread, creating it if necessary.
 * This simulate the DDS_DomainParticipantGlobals_get_worker_per_threadI function
 */
struct REDAWorker *raw_transport_get_worker_per_thread(
        REDAWorkerFactory *workerFactory,
        RTIOsapiThreadTssFactory *tssFactory,
        unsigned int *workerTssKey)
{
    RTIBool workerSet = false;
    struct REDAWorker *worker = NULL;

    /* --- Test preconditions --- */
    if (workerFactory == NULL) {
        fprintf(stderr, "Fail precondition, workerFactory == NULL\n");
        return NULL;
    }
    if (tssFactory == NULL) {
        fprintf(stderr, "Fail precondition, tssFactory == NULL\n");
        return NULL;
    }

    /* --- Get tssKey --- */
    if (*workerTssKey == 0) {
        /* Create a TssKey */
        if (!RTIOsapiThread_createKey(workerTssKey, tssFactory)) {
            fprintf(stderr, "Fail to get thread-specific key\n");
            return NULL;
        }
    }

    /* --- Get worker --- */
    worker = (struct REDAWorker *) RTIOsapiThread_getTss(*workerTssKey);
    if (worker == NULL) {
        char workerName[20];
        /*
         * Print 16 digits long hexadecimal (016lx). We use "l" to support
         * the new 64-bit threadID data-type
         */
        sprintf(workerName, "U%016llx", RTIOsapiThread_getCurrentThreadID());
        /* No worker: create a new one */
        worker = REDAWorkerFactory_createWorker(workerFactory, workerName);
        if (worker == NULL) {
            /* failed to create worker */
            fprintf(stderr, "Fail to create a worker\n");
            return NULL;
        } else {
            /* worker created successfully: add to TSS */
            workerSet = RTIOsapiThread_setTss(*workerTssKey, worker);
            if (!workerSet) {
                /* Failed to add to TSS: pretend it was never created. */
                if (worker != NULL) {
                    REDAWorkerFactory_destroyWorker(
                            workerFactory,
                            worker);
                }
                worker = NULL;
            }
        }
    }

    return worker;
}


#ifdef RTI_WIN32
#pragma warning(pop)
#endif
