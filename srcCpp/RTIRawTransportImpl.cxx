/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "RTIRawTransportImpl.h"

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

std::vector<NDDS_Transport_SendResource_t> peerData::resourcesList;

/*********************************************************
 * Constructor
 */
RTIRawTransportImpl::RTIRawTransportImpl()
        : _dataLen(100),
          _domainID(1),
          _latencyTest(false),
          _isDebug(false),
          _isLargeData(false),
          _isScan(false),
          _isPublisher(false),
          _batchSize(0),
          _peerHostCount(0),
          _basePort(7400),
          _useBlocking(true),
          _transport(),
          _pongSemaphore(NULL),
          _plugin(NULL),
          _workerFactory(NULL),
          _exclusiveArea(NULL),
          _tssFactory(NULL),
          _PM(NULL)
{

    // Set default interface
    _transport.allowInterfaces = std::string("127.0.0.1");

    // Reserve space for the peers
    peerData::resourcesList.reserve(RTIPERFTEST_MAX_PEERS);

}

/*********************************************************
 * Shutdown
 */
void RTIRawTransportImpl::Shutdown()
{

    for (unsigned int i = 0; i < peerData::resourcesList.size(); i++) {
        if (peerData::resourcesList[i] != NULL && _plugin != NULL) {
            _plugin->destroy_sendresource_srEA(
                    _plugin, _peersDataList[i].resource);
            peerData::resourcesList[i] = NULL;
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
int RTIRawTransportImpl::GetBatchSize()
{
    return _batchSize;
}

unsigned long RTIRawTransportImpl::GetInitializationSampleCount()
{
    return 0;
}

NDDS_Transport_Plugin *RTIRawTransportImpl::getPlugin()
{
    return _plugin;
}

std::vector<peerData> RTIRawTransportImpl::getPeersData()
{
    return _peersDataList;
}

RTIOsapiSemaphore *RTIRawTransportImpl::getPongSemaphore()
{
    return _pongSemaphore;
}
struct REDAWorkerFactory *RTIRawTransportImpl::getWorkerFactory()
{
    return _workerFactory;
}

RTIOsapiThreadTssFactory *RTIRawTransportImpl::getTssFactory()
{
    return _tssFactory;
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
 * PrintCmdLineHelp
 */
void RTIRawTransportImpl::PrintCmdLineHelp() {
    /**************************************************************************/
    std::string usage_string = std::string(
            "\t-domain <ID>                  - RTI DDS Domain, default 1\n") +
            "\t-bestEffort                   - Run test in best effort mode, default reliable\n" +
            "\t                                Default: \"default\" (If using asynchronous).\n" +
            "\t-peer <address>               - Adds a peer to the peer host address list.\n" +
            "\t                                This argument may be repeated to indicate multiple peers\n" +
            "\t-noBlockingSockets            - Control blocking behavior of send sockets to never block.\n" +
            "\t                                CHANGING THIS FROM THE DEFAULT CAN CAUSE SIGNIFICANT PERFORMANCE PROBLEMS.\n" +
            "\n";
    usage_string += _transport.helpMessageString();

    fprintf(stderr, "%s", usage_string.c_str());
}

/*********************************************************
 * parseConfig
 */
bool RTIRawTransportImpl::parseConfig(int argc, char *argv[]) {
    unsigned long _scanMaxSize = 0;
    int i;

    bool found = false;
    std::vector<std::string>::iterator it;

    /* Parameters not supported by sockets */
    std::vector<std::string> noRawTransportParamsV;

    /*C++98 dont support extended initializer lists*/
    noRawTransportParamsV.push_back(std::string("-unbounded"));
    noRawTransportParamsV.push_back(std::string("-sendQueueSize"));
    noRawTransportParamsV.push_back(std::string("-heartbeatPeriod"));
    noRawTransportParamsV.push_back(std::string("-fastHeartbeatPeriod"));
    noRawTransportParamsV.push_back(std::string("-qosFile"));
    noRawTransportParamsV.push_back(std::string("-qosLibrary"));
    noRawTransportParamsV.push_back(std::string("-durability"));
    noRawTransportParamsV.push_back(std::string("-dynamicData"));
    noRawTransportParamsV.push_back(std::string("-noDirectCommunication"));
    noRawTransportParamsV.push_back(std::string("-instances"));
    noRawTransportParamsV.push_back(std::string("-instanceHashBuckets"));
    noRawTransportParamsV.push_back(std::string("-keepDurationUsec"));
    noRawTransportParamsV.push_back(std::string("-noPositiveAcks"));
    noRawTransportParamsV.push_back(std::string("-waitsetDelayUsec"));
    noRawTransportParamsV.push_back(std::string("-waitsetEventCount"));
    noRawTransportParamsV.push_back(std::string("-enableAutoThrottle"));
    noRawTransportParamsV.push_back(std::string("-enableTurboMode"));
    noRawTransportParamsV.push_back(std::string("-noXmlQos"));
    noRawTransportParamsV.push_back(std::string("-asynchronous"));
    noRawTransportParamsV.push_back(std::string("-flowController"));
    noRawTransportParamsV.push_back(std::string("-cft"));
    noRawTransportParamsV.push_back(std::string("-writeInstance"));
    noRawTransportParamsV.push_back(std::string("-enableTCP"));
    noRawTransportParamsV.push_back(std::string("-enableUDPv6"));
    noRawTransportParamsV.push_back(std::string("-allowInterfaces"));
    noRawTransportParamsV.push_back(std::string("-transportServerBindPort"));
    noRawTransportParamsV.push_back(std::string("-transportWan"));
    noRawTransportParamsV.push_back(std::string("-transportCertAuthority"));
    noRawTransportParamsV.push_back(std::string("-transportCertFile"));
    noRawTransportParamsV.push_back(std::string("-transportPrivateKey"));
    noRawTransportParamsV.push_back(std::string("-transportWanServerAddress"));
    noRawTransportParamsV.push_back(std::string("-transportWanServerPort"));
    noRawTransportParamsV.push_back(std::string("-transportWanId"));
    noRawTransportParamsV.push_back(std::string("-transportSecureWan"));

    std::string paramsInfo = std::string(
            "Parameters not supported by sockets: (Delete them and try again)\n");

    // /* Print all the non compatibles params together and return */
    // for (int j = 0; j < argc; ++j ) {
    //     it = find(
    //             noRawTransportParamsV.begin(),
    //             noRawTransportParamsV.end(),
    //             argv[j]);
    //     if (it != noRawTransportParamsV.end()) {
    //         paramsInfo += std::string("\t" + std::string(argv[j]) + "\n" );
    //         found = true;
    //     }
    // }

    // if (found) {
    //     fprintf(stderr, "%s", paramsInfo.c_str());
    //     return false;
    // }

    if (_isScan) {
        _dataLen = _scanMaxSize;
    }

    if (_batchSize != 0 && _dataLen > _batchSize) {
        fprintf(stderr, "\t -batchSize must be bigger or equal than -datalen\n");
        return false;
    }

    if (_batchSize != 0 && _latencyTest) {
        fprintf(
                stderr,
                "\t -latencyTest not supported with batchign. (Remove "
                "-batchSize)\n");
        return false;
    }

    if (_dataLen > (unsigned long)MAX_SYNCHRONOUS_SIZE) {
        fprintf(stderr, "Large data settings enabled.\n");
        _isLargeData = true;
    }

    if (!_transport.parseTransportOptions(argc, argv)) {
        fprintf(stderr, "Failure parsing the transport options.\n");
        return false;
    }

    if (_transport.useMulticast && (_peerHostCount > 0)) {
        fprintf(
                stderr,
                "\tFor multicast, if you want to send to other IP, "
                "use multicastAddr\n");
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
    stringStream << "\tDomain: " << _domainID << "\n";

    stringStream << "\n" << _transport.printTransportConfigurationSummary();

    // Blocking sockets
    stringStream << "\tBlocking Sockets: ";
    if (_useBlocking) {
        stringStream << "Yes\n";
    } else {
        stringStream << "No\n";
    }

    // Ports
    stringStream << "\tThe following ports will be used: ";
    if (_isPublisher) {
        stringStream << getReceiveUnicastPort(ANNOUNCEMENT_TOPIC_NAME) << " - "
                     << getReceiveUnicastPort(LATENCY_TOPIC_NAME) << "\n";
    } else {
        stringStream << getReceiveUnicastPort(THROUGHPUT_TOPIC_NAME) << "\n";
    }

    // set initial peers and not use multicast
    if (_peerHostCount > 0 && !isMulticast()) {
        stringStream << "\tInitial peers: ";
        for (int i = 0; i < _peerHostCount; ++i) {
            stringStream << _peerHost[i];
            stringStream << ((i+1 == _peerHostCount)? "\n" : ", ");
        }
    }

    return stringStream.str();
}

/*********************************************************
 * RTIPublisher
 */
class RTIRawTransportPublisher : public IMessagingWriter{
  private:

    /* --- Transport members --- */
    RTIRawTransportImpl *_parent;
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_Buffer_t _sendBuffer;
    struct REDAWorker *_worker;
    unsigned int _workerTssKey;

    /* --- Perftest members --- */
    std::vector<peerData> _peersDataList;
    TestData_t _data;
    RTIOsapiSemaphore *_pongSemaphore;

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
        _plugin = parent->getPlugin();
        _peersDataList = parent->getPeersData();
        _pongSemaphore = parent->getPongSemaphore();

        if (parent->GetBatchSize() == 0) {
            _batchBufferSize = NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
            _useBatching = false;
        } else {
            _batchBufferSize = parent->GetBatchSize();
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
            if (_parent->getWorkerFactory() != NULL) {
                REDAWorkerFactory_destroyWorker(
                        _parent->getWorkerFactory(),
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
            _worker = RawTransportGetWorkerPerThread(
                    _parent->getWorkerFactory(),
                    _parent->getTssFactory(),
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

        if (!success) {
            fprintf(stderr, "Fail to serialize data\n");
            return false;
        }

        /* _data is been serialize (copied). It's right to unloan then. */
        _data.bin_data.unloan();

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
        _plugin = parent->getPlugin();

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
            if (_parent->getWorkerFactory() != NULL) {
                REDAWorkerFactory_destroyWorker(
                        _parent->getWorkerFactory(),
                        _worker);
            } else {
                fprintf(stderr, "Error, workerFactory destroy before worker\n");
            }
        }
    }

    TestMessage *ReceiveMessage() {

        int result = 0;

        if (_worker == NULL) {
            _worker = RawTransportGetWorkerPerThread(
                    _parent->getWorkerFactory(),
                    _parent->getTssFactory(),
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
                    NULL, &_data, &_stream, RTI_TRUE, RTI_TRUE, NULL);

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
                    RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);

            if (_readThreadSemaphore == NULL) {
                fprintf(
                        stderr,
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

    if (!validate_input() {
        return false;
    }

    /* Only if we run latency test we need to wait for pongs after sending pings */
    if (_latencyTest) {
        _pongSemaphore =
                RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL);

        if (_pongSemaphore == NULL) {
            fprintf(
                    stderr,
                    "Fail to create a Semaphore for RTIRawTransportImpl\n");
            return NULL;
        }
    }

    if (!configureSocketsTransport()) {
        return false;
    }

    return true;
}

/*********************************************************
 * GetUnicastPort
 */
unsigned int RTIRawTransportImpl::getSendUnicastPort(
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
            _domainID, /* domainId */
            _isPublisher ? subId + 1 : 0, /* participantId */
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
            _domainID, /* domainId */
            _isPublisher ? 0 : perftest_cpp::_SubID + 1, /* participantId */
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
    if (!isMulticast()) {
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
    bool isMulticastAddr = false;

    NDDS_Transport_Address_t actualAddr;
    int actualPort = 0;
    RTIBool shared = false;
    unsigned int j = 0;

    // If multicat, then take the multicast address.
    if (_transport.useMulticast
            && getMulticastTransportAddr(topicName, multicastAddr)) {
        isMulticastAddr = true;
    } else if (_transport.useMulticast) {
        fprintf(stderr, "Bad configuration for multicast (sockets)\n");
        return NULL;
    }

    /* _peerHostCount is garanteed to be 1 if multicast is enabled */
    for (int i = 0; i < _peerHostCount; i++) {
        shared = false;
        actualAddr = isMulticastAddr ? multicastAddr : _peersMap[i].first;

        // Calculate the port of the new send resource.
        actualPort = getSendUnicastPort(topicName, _peersMap[i].second);

        for (j = 0; j < peerData::resourcesList.size() && !shared; ++j) {
            // Try to share the resource
            shared = _plugin->share_sendresource_srEA(
                    _plugin,
                    &peerData::resourcesList[j],
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

            peerData::resourcesList.push_back(resource);
        }
        /* This data will be use by the writer to send to multiples peers. */
        _peersDataList.push_back(
                peerData(
                        shared ? &peerData::resourcesList[j-1]
                                : &peerData::resourcesList.back(),
                        actualAddr,
                        actualPort));
    }

    return new RTIRawTransportPublisher(this);
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
    bool isMulticastAddr = false;
    RTIBool result = true;

    /* If multicat, then take the multicast address. */
    if (_transport.useMulticast
            && getMulticastTransportAddr(topicName, multicastAddr)) {
        isMulticastAddr = true;
    } else if (_transport.useMulticast) {
        fprintf(stderr, "Bad configuration for multicast (RawTransport)\n");
        return NULL;
    }

    /* Calculate the port of the new receive resource. */
    recvPort = getReceiveUnicastPort(topicName);

    result = _plugin->create_recvresource_rrEA(
            _plugin,
            &recvResource,
            &recvPort,
            (isMulticastAddr)? &multicastAddr : NULL,
            NDDS_TRANSPORT_PRIORITY_DEFAULT);

    if (!result) {
        fprintf(
            stderr,
            "Create_recvresource_rrEA error. Maybe the port %d is been use.\n"
            "Check if you have other pub/sub with the same id\n",
            recvPort);
        return NULL;
    }

    return new RTIRawTransportSubscriber(
            this,
            recvResource,
            recvPort);
}

bool RTIRawTransportImpl::configureSocketsTransport()
{
    char *interfaceAddr = NULL; /*WARNING: interface is a reserved word on VS*/
    interfaceAddr = DDS_String_dup(_transport.allowInterfaces.c_str());
    if (interfaceAddr == NULL) {
        fprintf(
                stderr,
                "Fail allocating memory on configureSocketsTransport\n");
        return false;
    }

    /* If no peer is given, assume a default one */
    if (_peerHostCount == 0) {
        _peerHost[0] = (char *) "127.0.0.1";
        _peerHostCount = 1;
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

            if (_transport.useMulticast) {
                udpv4_prop.parent.allow_multicast_interfaces_list = &interfaceAddr;
                udpv4_prop.parent.allow_multicast_interfaces_list_length = 1;
                udpv4_prop.reuse_multicast_receive_resource = 1;
                udpv4_prop.multicast_enabled = 1;
                udpv4_prop.multicast_ttl = 1;
                udpv4_prop.unicast_enabled = 0;
            }

            if (_useBlocking) {
                udpv4_prop.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_ALWAYS;
            } else {
                // This will reduce the package lost but affect on the
                // performance.
                udpv4_prop.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_NEVER;
            }

            udpv4_prop.no_zero_copy = false;
            udpv4_prop.parent.message_size_max =
                    NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
            /*
             * Minimum number of gather-send buffers that must be supported by a
             * Transport Plugin implementation.
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
                fprintf(
                        stderr,
                        "Input interface (%s) not recognize\n",
                        interfaceAddr);
                return false;
            }

            /* Check if the multicast addres is correct */
            if (_transport.useMulticast
                    && getNumMulticastInterfaces(udpPlugin) <= 0) {
                fprintf(
                        stderr,
                        "The interface (%s) does not have multicast-enabled\n",
                        interfaceAddr);
                return false;
            }

            /* Peers address and ID parse to NDDS_Transport_Address_t */
            for (int i = 0; i < _peerHostCount; i++) {
                id_sub = 0;
                /* Regular expression to identify ADDRESS:ID */
                sscanf(_peerHost[i], "%[^:]:%d", addr_sub, &id_sub);

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

        //TODO: move to validate function when parameter manager merge
        if (!_useBlocking) {
            fprintf(stderr, "SHMEM dont support -noBlockingSockets\n");
            return false;
        }

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

        for (int i = 0; i < _peerHostCount; i++) {
            id_sub = 0;
            sscanf(_peerHost[i], "%[^:]:%d", addr_sub, &id_sub);

            _peersMap.push_back(
                    std::pair<NDDS_Transport_Address_t, int>(addr, id_sub));
        }

        /*
         * For SHMEM we dont want to print any interface on the
         * printTransportConfigurationSummary()
         */
        _transport.allowInterfaces = std::string();

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

int getNumMulticastInterfaces(struct NDDS_Transport_UDP *plugin)
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
struct REDAWorker *RawTransportGetWorkerPerThread(
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
