/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "RTISocketImpl.h"

/*TODO: check if this can be remove*/
#ifdef RTI_SECURE_PERFTEST
#include "security/security_default.h"
#endif

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

std::string valid_flow_controller_socket[] = {"default", "1Gbps", "10Gbps"};

const int RTISocketImpl::resources_per_participant = 3;

/*********************************************************
 * Shutdown
 */
void RTISocketImpl::Shutdown()
{

    if (_plugin != NULL) {
        _plugin->delete_cEA(_plugin, NULL);
    }

    if (_workerFactory != NULL && _worker != NULL) {
        REDAWorkerFactory_destroyWorker(_workerFactory, _worker);
    }
    if (_exclusiveArea != NULL){
        REDAWorkerFactory_destroyExclusiveArea(_workerFactory, _exclusiveArea);
    }
    if (_workerFactory != NULL) {
        REDAWorkerFactory_delete(_workerFactory);
    }

    if (_pongSemaphore != NULL) {
        RTIOsapiSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }

}

/*********************************************************
 * PrintCmdLineHelp
 */
void RTISocketImpl::PrintCmdLineHelp() {
    /**************************************************************************/
    std::string usage_string = std::string(
            "\t-sendQueueSize <number>       - Sets number of samples (or batches) in send\n") +
            "\t                                queue, default 50\n" +
            "\t-domain <ID>                  - RTI DDS Domain, default 1\n" +
            "\t-bestEffort                   - Run test in best effort mode, default reliable\n"
            "\t                                " +
            "\t                                Default: \"default\" (If using asynchronous).\n" +
            "\t-peer <address>               - Adds a peer to the peer host address list.\n" +
            "\t                                This argument may be repeated to indicate multiple peers\n" +
            "\n";
    usage_string += _transport.helpMessageString();

    fprintf(stderr, "%s", usage_string.c_str());
}

/*********************************************************
 * ParseConfig
 */
bool RTISocketImpl::ParseConfig(int argc, char *argv[]) {
    unsigned long _scan_max_size = 0;
    int i;

    /* Parameters not supported by sockets */
    std::vector<std::string> no_socket_params_v;

    /*C++98 dont support extended initializer lists*/
    no_socket_params_v.push_back(std::string("-unbounded"));
    no_socket_params_v.push_back(std::string("-sendQueueSize"));
    no_socket_params_v.push_back(std::string("-heartbeatPeriod"));
    no_socket_params_v.push_back(std::string("-fastHeartbeatPeriod"));
    no_socket_params_v.push_back(std::string("-qosFile"));
    no_socket_params_v.push_back(std::string("-qosLibrary"));
    no_socket_params_v.push_back(std::string("-durability"));
    no_socket_params_v.push_back(std::string("-dynamicData"));
    no_socket_params_v.push_back(std::string("-noDirectCommunication"));
    no_socket_params_v.push_back(std::string("-instances"));
    no_socket_params_v.push_back(std::string("-instanceHashBuckets"));
    no_socket_params_v.push_back(std::string("-keepDurationUsec"));
    no_socket_params_v.push_back(std::string("-noPositiveAcks"));
    no_socket_params_v.push_back(std::string("-waitsetDelayUsec"));
    no_socket_params_v.push_back(std::string("-waitsetEventCount"));
    no_socket_params_v.push_back(std::string("-enableAutoThrottle"));
    no_socket_params_v.push_back(std::string("-enableTurboMode"));
    no_socket_params_v.push_back(std::string("-noXmlQos"));
    no_socket_params_v.push_back(std::string("-asynchronous"));
    no_socket_params_v.push_back(std::string("-flowController"));
    no_socket_params_v.push_back(std::string("-cft"));
    no_socket_params_v.push_back(std::string("-writeInstance"));
    no_socket_params_v.push_back(std::string("-enableTCP"));
    no_socket_params_v.push_back(std::string("-enableUDPv6"));
    no_socket_params_v.push_back(std::string("-allowInterfaces"));
    no_socket_params_v.push_back(std::string("-transportServerBindPort"));
    no_socket_params_v.push_back(std::string("-transportWan"));
    no_socket_params_v.push_back(std::string("-transportCertAuthority"));
    no_socket_params_v.push_back(std::string("-transportCertFile"));
    no_socket_params_v.push_back(std::string("-transportPrivateKey"));
    no_socket_params_v.push_back(std::string("-transportWanServerAddress"));
    no_socket_params_v.push_back(std::string("-transportWanServerPort"));
    no_socket_params_v.push_back(std::string("-transportWanId"));
    no_socket_params_v.push_back(std::string("-transportSecureWan"));

    std::string params_info = std::string(
            "Parameters not supported by sockets: (Delete them and try again)\n");

    /* Print all the non compatibles params together and return */
    bool found = false;
    std::vector<std::string>::iterator it;
    for (int i = 0; i < argc; ++i ) {
        it = find(
                no_socket_params_v.begin(),
                no_socket_params_v.end(),
                argv[i]);
        if (it != no_socket_params_v.end()) {
            params_info += std::string("\t" + std::string(argv[i]) + "\n" );
            found = true;
        }
    }

    if (found) {
        fprintf(stderr, "%s", params_info.c_str());
        return false;
    }


    // Command line params
    for (i = 0; i < argc; ++i) {
        if (IS_OPTION(argv[i], "-pub")) {
            _isPublisher = true;
        }
        else if (IS_OPTION(argv[i], "-scan")) {
            _isScan = true;
            if ((i != (argc - 1)) && *argv[1 + i] != '-') {
                ++i;
                unsigned long aux_scan;
                char *pch;
                pch = strtok(argv[i], ":");
                while (pch != NULL) {
                    if (sscanf(pch, "%lu", &aux_scan) != 1) {
                        fprintf(stderr, "-scan <size> value must have the format '-scan <size1>:<size2>:...:<sizeN>'\n");
                        return false;
                    }
                    pch = strtok(NULL, ":");
                    if (aux_scan >= _scan_max_size) {
                        _scan_max_size = aux_scan;
                    }
                }
            }
        }
        else if (IS_OPTION(argv[i], "-dataLen")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <length> after -dataLen\n");
                return false;
            }

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < (unsigned long)perftest_cpp::OVERHEAD_BYTES) {
                fprintf(stderr, "-dataLen must be >= %d\n", perftest_cpp::OVERHEAD_BYTES);
                return false;
            }

            if (_DataLen > (unsigned long)MAX_PERFTEST_SAMPLE_SIZE) {
                fprintf(stderr, "-dataLen must be <= %d\n", MAX_PERFTEST_SAMPLE_SIZE);
                return false;
            }

        }
        else if (IS_OPTION(argv[i], "-domain")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <id> after -domain\n");
                return false;
            }
            _DomainID = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-multicast")) {
            _IsMulticast = true;
            if ((i != (argc - 1)) && *argv[i + 1] != '-') {
                i++;
                _multicastAddrString = argv[i];
            }
        }
        else if (IS_OPTION(argv[i], "-nomulticast")) {
            _IsMulticast = false;
        }
        else if (IS_OPTION(argv[i], "-bestEffort")) {
            _IsReliable = false;
        }
        else if (IS_OPTION(argv[i], "-verbosity")) {
            errno = 0;
            int verbosityLevel = strtol(argv[++i], NULL, 10);

            if (errno) {
                fprintf(stderr, "Unexpected value after -verbosity\n");
                return false;
            }

            switch (verbosityLevel) {
            case 0:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
            case 1:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
            case 2:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
            case 3:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
            default:
                fprintf(stderr,
                        "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                verbosityLevel = 1;
                break;
            }
        }
        else if (IS_OPTION(argv[i], "-latencyTest")) {
            _LatencyTest = true;
        }
        else if (IS_OPTION(argv[i], "-peer")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <address> after -peer\n");
                return false;
            }
            if (_peer_host_count +1 < RTIPERFTEST_MAX_PEERS) {
                _peer_host[_peer_host_count++] = DDS_String_dup(argv[i]);
            } else {
                fprintf(stderr,"The maximun of -initial peers is %d\n", RTIPERFTEST_MAX_PEERS);
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-numPublishers")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <count> after -numPublishers\n");
                return false;
            }
            _NumPublishers = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-numSubscribers")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <count> after -numSubscribers\n");
                return false;
            }
            _NumSubscribers = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-batchSize")) {

            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <#bytes> after -batchSize\n");
                return false;
            }
            _batchSize = strtol(argv[i], NULL, 10);

            if (_batchSize < 0 || _batchSize > (unsigned int)MAX_SYNCHRONOUS_SIZE) {
                fprintf(stderr, "Batch size '%d' should be between [0,%d]\n",
                        _batchSize,
                        MAX_SYNCHRONOUS_SIZE);
                return false;
            }
        }
        else {

            if (i > 0) {
                std::map<std::string, unsigned int> transportCmdOpts =
                    PerftestTransport::getTransportCmdLineArgs();

                std::map<std::string, unsigned int>::iterator it =
                    transportCmdOpts.find(argv[i]);
                if (it != transportCmdOpts.end()) {
                    /*
                     * Increment the counter with the number of arguments
                     * obtained from the map.
                     */
                    i = i + it->second;
                    continue;
                }

                fprintf(stderr, "%s: not recognized\n", argv[i]);
                return false;
            }
        }
    }

    if (_isScan) {
        _DataLen = _scan_max_size;
    }

    /* If no peer is given, assume a default one */
    if (_peer_host_count == 0) {
        if (_IsMulticast) {
            _peer_host[0] = _multicastAddrString;
        } else {
            _peer_host[0] = (char *)"127.0.0.1";
        }
        _peer_host_count = 1;
    }

    if (_peer_host_count != 1 || _NumSubscribers > 1) {
        fprintf(stderr, "\tOne to many send option it's not supported."
                "Please, set only one Subscriber and one Publisher\n");
        return false;
    }

    if (_batchSize != 0 && _DataLen > _batchSize) {
        fprintf(stderr, "\t -batchSize must be bigger or equal than -datalen\n");
        return false;
    }

    if (_batchSize != 0 && _LatencyTest) {
        fprintf(
                stderr,
                "\t -latencyTest not supported with batchign. (Remove "
                "-batchSize)\n");
        return false;
    }

    if (_DataLen > (unsigned long)MAX_SYNCHRONOUS_SIZE) {
        fprintf(stderr, "Large data settings enabled.\n");
        _isLargeData = true;
    }

    if (!_transport.parseTransportOptions(argc, argv)) {
        fprintf(stderr, "Failure parsing the transport options.\n");
        return false;
    };

    return true;
}

/*********************************************************
 * RTIPublisher
 */
class RTISocketPublisher : public IMessagingWriter{
  private:
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_SendResource_t _sendResource;
    NDDS_Transport_Address_t _dst_address;
    NDDS_Transport_Port_t _send_port;
    NDDS_Transport_Buffer_t _send_buffer;

    struct REDAWorker *_worker;

    RTIOsapiSemaphore *_pongSemaphore;

    TestData_t _data;

    std::vector<std::pair<NDDS_Transport_Address_t, int> > _peersMap;

    int _NumSubscribers;

    unsigned int _batchBufferSize;
    bool _useBatching;

    struct RTICdrStream _stream;
    struct PRESTypePluginDefaultEndpointData _epd;

  public:
    RTISocketPublisher(
            NDDS_Transport_Plugin *plugin,
            NDDS_Transport_SendResource_t send_resource,
            NDDS_Transport_Address_t dst_address,
            NDDS_Transport_Port_t send_port,
            RTIOsapiSemaphore *pongSemaphore,
            std::vector<std::pair<NDDS_Transport_Address_t, int> > peersMap,
            int numSubscribers,
            struct REDAWorker *worker,
            unsigned int batchBufferSize) {

        _plugin = plugin;
        _sendResource = send_resource;
        _dst_address = dst_address;
        _send_port = send_port;
        _pongSemaphore = pongSemaphore;

        _NumSubscribers = numSubscribers;

        _worker = worker;

        _peersMap = peersMap;

        if (batchBufferSize == 0){
            _batchBufferSize = NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
            _useBatching = false;
        } else {
            _batchBufferSize = batchBufferSize;
            _useBatching = true;
        }

        RTIOsapiHeap_allocateBuffer(
            &_send_buffer.pointer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

        if (_send_buffer.pointer == NULL) {
            fprintf(stderr, "Error allocating memory for the send buffer\n");
        }
        _send_buffer.length = 0;


        RTICdrStream_init(&_stream);
        RTICdrStream_set(&_stream, (char *)_send_buffer.pointer,
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        _epd._maxSizeSerializedSample =
                TestData_tPlugin_get_serialized_sample_max_size(
                        NULL,
                        RTI_TRUE,
                        RTICdrEncapsulation_getNativeCdrEncapsulationId(), 0);

    }

    ~RTISocketPublisher() {
        Shutdown();
    }

    void Shutdown() {

        if (_sendResource != NULL && _plugin != NULL) {
            _plugin->destroy_sendresource_srEA(_plugin, &_sendResource);
        }

        if (_send_buffer.pointer != NULL) {
            RTIOsapiHeap_freeBuffer(_send_buffer.pointer);
        }
    }

    bool SendMessage() {

        /*
         * TODO: //Ask Fernando
         *
         * In the case we want a one to many option:
         *
         * The 1th option is be restrictive, and only allow one subscriber
         * per Peer.
         *
         * The 2th options is identify how many subscriber are per Peer
         * on the command line parameters like -peer x.x.x.x N
         *
         * The 3th option is always have the same numbers of subscriber per
         * Peer.
         */

        bool retCode = true;


        retCode = _plugin->send(
                _plugin,
                &_sendResource,
                &_peersMap[0].first,
                _send_port,
                NDDS_TRANSPORT_PRIORITY_DEFAULT,
                &_send_buffer,
                1,
                _worker);

        if (!retCode) {
            /**
             * TODO: Ask Fernando
             *
             * Set a log with warning level???? Not clear
             *
             * Keep count on writes errors and then output on intervals.?
             *
             * use -> RTILog_printContextAndMsg??
             *
             */
            //fprintf(stderr, "Write error using sockets\n");
            return false;
        }

        return retCode;

    }

    void Flush() {

        /* If there is no data, no need to flush */
        if (_send_buffer.length == 0) {
            return;
        }

        if (SendMessage() != 1) {
            //TODO: the same as sendMessage print problem.
            //fprintf(stderr, "Fail in flush() -> sendMessage()\n");
        }

        /* If the send it's done, reset the stream to fill the buffer again */
        RTICdrStream_set(
                &_stream,
                (char *)_send_buffer.pointer,
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        _send_buffer.length = 0;
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey) {

        bool retCode = RTI_FALSE;

        _data.entity_id = message.entity_id;
        _data.seq_num = message.seq_num;
        _data.timestamp_sec = message.timestamp_sec;
        _data.timestamp_usec = message.timestamp_usec;
        _data.latency_ping = message.latency_ping;
        _data.bin_data.loan_contiguous(
                (DDS_Octet *)message.data,
                message.size,
                message.size);

        int serialize_size =  message.size
                + perftest_cpp::OVERHEAD_BYTES
                + RTI_CDR_ENCAPSULATION_HEADER_SIZE;

        if ((unsigned int)(serialize_size + _send_buffer.length)
                > _batchBufferSize) {
            Flush();
        }

        retCode = TestData_tPlugin_serialize(
                (PRESTypePluginEndpointData)&_epd,
                &_data,
                &_stream,
                RTI_TRUE,
                RTICdrEncapsulation_getNativeCdrEncapsulationId(),
                RTI_TRUE,
                NULL);

        if (!retCode){
            fprintf(stderr, "Fail serializing data\n");
            return false;
        }

        _data.bin_data.unloan();

        _send_buffer.length = RTICdrStream_getCurrentPositionOffset(&_stream);

        if (_useBatching && ((unsigned int)_send_buffer.length <= _batchBufferSize)) {
            return true;
        }

        Flush();

        return true;
    }

    void WaitForReaders(int numSubscribers) {
        perftest_cpp::MilliSleep(1000);
        /*Dummy Function*/
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
        /*Dummy Function*/
        return 0;
    };

    void wait_for_acknowledgments(long sec, unsigned long nsec) {
        /*Dummy Function*/
    }
};

/*********************************************************
 * RTISubscriber
 */
class RTISocketSubscriber : public IMessagingReader
{
  private:
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_SendResource_t _recvResource;
    NDDS_Transport_Buffer_t _recvBuffer;
    NDDS_Transport_Port_t _recvPort;

    NDDS_Transport_Message_t _transportMessage;

    TestMessage _message;

    struct REDAWorker *_worker;

    TestData_t _data;
    char * _payload;
    int _payload_size;

    RTIOsapiSemaphore *_pongSemaphore;

    struct RTICdrStream _stream;
    bool _noData;

  public:
    RTISocketSubscriber(
        NDDS_Transport_Plugin *plugin,
        NDDS_Transport_SendResource_t recv_resource,
        NDDS_Transport_Port_t recv_port,
        struct REDAWorker *worker)
    {
        _plugin = plugin;
        _recvResource = recv_resource;
        _recvPort = recv_port;
        _recvBuffer.length = 0;
        _recvBuffer.pointer = NULL;
        _worker = worker;

        // Similar to NDDS_Transport_Message_t message =
        //      NDDS_TRANSPORT_MESSAGE_INVALID;
        _transportMessage.buffer.pointer = NULL;
        _transportMessage.buffer.length = 0;
        _transportMessage.loaned_buffer_param = NULL;

        /* Maximun size of UDP package*/
        RTIOsapiHeap_allocateBufferAligned(
            &_recvBuffer.pointer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

        /*
         * TODO:
         * What to do if the allocate fail?
         * Create a initialize function?
         */
        if (_recvBuffer.pointer == NULL) {
            fprintf(stderr, "RTIOsapiHeap_allocateArray error\n");
        } else {
            _recvBuffer.length = NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
        }

        _data.bin_data.maximum(NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        RTICdrStream_init(&_stream);
        _noData = true;
    }

    ~RTISocketSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {

        //_plugin->unblock_receive_rrEA(_plugin, &_recvResource, _worker);
        //perftest_cpp::MilliSleep(5000);
        if (_recvBuffer.pointer != NULL) {
            RTIOsapiHeap_freeBufferAligned(_recvBuffer.pointer);
        }

        if (_recvResource != NULL && _plugin != NULL) {
            _plugin->destroy_recvresource_rrEA(_plugin, &_recvResource);
        }
    }

    TestMessage *ReceiveMessage() {


        while (true) {
            if (_noData) {

                bool result = _plugin->receive_rEA(
                        _plugin,
                        &_transportMessage,
                        &_recvBuffer,
                        &_recvResource,
                        _worker);
                if (!result) {
                    fprintf(stderr, "error receiving data\n");
                    return NULL;
                }

                RTICdrStream_set(&_stream,
                        (char *)_transportMessage.buffer.pointer,
                        _transportMessage.buffer.length);

                _noData = false;
            }

            // may have hit end condition
            if (RTICdrStream_getCurrentPositionOffset(&_stream)
                    >= _transportMessage.buffer.length) {

                if (_plugin -> return_loaned_buffer_rEA != NULL
                        && _transportMessage.loaned_buffer_param != (void *)-1) {
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


    void WaitForWriters(int numPublishers) {
        /*Dummy Function*/
    }
};

/*********************************************************
 * Initialize
 */
bool RTISocketImpl::Initialize(int argc, char *argv[]) {
    if (!ParseConfig(argc, argv)) {
        return false;
    }

    printf("-- Using SOCKETS --\n");

    // only if we run the latency test we need to wait for pongs after sending pings
    _pongSemaphore = _LatencyTest
        ? RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL)
        : NULL;

    if (_LatencyTest && _pongSemaphore == NULL) {
        fprintf(stderr, "Fail creating a Semaphore for RTISocketImpl\n");
        return NULL;
    }

    if (!ConfigureSocketsTransport()) {
        return false;
    }

    return true;
}

/*********************************************************
 * ObtainSerializeTime
 */

double RTISocketImpl::ObtainSerializeTimeCost(int iterations, unsigned int sampleSize){

    struct timespec cgt1, cgt2;
    double serializeTime;

    struct RTICdrStream stream;
    struct PRESTypePluginDefaultEndpointData epd;

    char * buffer;

    RTIOsapiHeap_allocateBuffer(
            &buffer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

    if (buffer == NULL) {
        fprintf(stderr,
                "Error allocating memory for buffer on ObtainSerializeTimeCost\n");
    }

    RTICdrStream_init(&stream);
    RTICdrStream_set(
            &stream,
            buffer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

    epd._maxSizeSerializedSample =
            TestData_tPlugin_get_serialized_sample_max_size(
                    NULL,
                    RTI_TRUE,
                    RTICdrEncapsulation_getNativeCdrEncapsulationId(),
                    0);

    /* Created a dummy data to serialize*/
    TestData_t testData;
    testData.bin_data.maximum(NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);
    testData.entity_id = 0;
    testData.seq_num = 0;
    testData.timestamp_sec = 0;
    testData.timestamp_usec = 0;
    testData.latency_ping = 0;

    /* The buffer can be reuse, does not matter what it's inside */
    testData.bin_data.loan_contiguous(
            (DDS_Octet *) buffer, sampleSize, sampleSize);

    /* Serialize time calculating */
    clock_gettime(CLOCK_REALTIME, &cgt1);

    for (int i = 0; i < iterations; i++) {
        /*
         * Reset the stream to allocate the serialize data always at the
         * beggining
         */
        RTICdrStream_set(
                &stream,
                buffer,
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        TestData_tPlugin_serialize(
                (PRESTypePluginEndpointData) &epd,
                &testData,
                &stream,
                RTI_TRUE,
                RTICdrEncapsulation_getNativeCdrEncapsulationId(),
                RTI_TRUE,
                NULL);
    }

    clock_gettime(CLOCK_REALTIME, &cgt2);
    serializeTime = (double) (cgt2.tv_sec - cgt1.tv_sec)
            + (double) ((cgt2.tv_nsec - cgt1.tv_nsec) / (1.e+9));

    return serializeTime /= iterations;
}
/*********************************************************
 * ObtainDeserializeTime
 */

double ObtainDeserializeTimeCost(int iterations, unsigned int sampleSize) {
    struct timespec cgt1, cgt2;
    double deSerializeTime;

    struct RTICdrStream stream;
    struct PRESTypePluginDefaultEndpointData epd;

    char *buffer;

    RTIOsapiHeap_allocateBuffer(
            &buffer,
            NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

    if (buffer == NULL) {
        fprintf(stderr,
                "Error allocating memory for buffer on "
                "ObtainDeSerializeTimeCost\n");
    }

    RTICdrStream_init(&stream);
    RTICdrStream_set(&stream, buffer, NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

    epd._maxSizeSerializedSample =
            TestData_tPlugin_get_serialized_sample_max_size(
                    NULL,
                    RTI_TRUE,
                    RTICdrEncapsulation_getNativeCdrEncapsulationId(),
                    0);

    /* Created a dummy data to serialize */
    TestData_t testData;
    testData.bin_data.maximum(NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);
    testData.entity_id = 0;
    testData.seq_num = 0;
    testData.timestamp_sec = 0;
    testData.timestamp_usec = 0;
    testData.latency_ping = 0;
    testData.bin_data.loan_contiguous(
            (DDS_Octet *) buffer,
            sampleSize,
            sampleSize);

    /* Serialize the data */
    TestData_tPlugin_serialize(
            (PRESTypePluginEndpointData) &epd,
            &testData,
            &stream,
            RTI_TRUE,
            RTICdrEncapsulation_getNativeCdrEncapsulationId(),
            RTI_TRUE,
            NULL);

    /* Deserialize time calculations */
    clock_gettime(CLOCK_REALTIME, &cgt1);

    for (int i = 0; i < iterations; i++) {

        RTICdrStream_set(
                &stream,
                buffer,
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX);

        TestData_tPlugin_deserialize_sample(
                NULL,
                &testData,
                &stream,
                RTI_TRUE,
                RTI_TRUE,
                NULL);
    }

    clock_gettime(CLOCK_REALTIME, &cgt2);
    deSerializeTime = (double) (cgt2.tv_sec - cgt1.tv_sec)
            + (double) ((cgt2.tv_nsec - cgt1.tv_nsec) / (1.e+9));

    return deSerializeTime /= iterations;
}

/*********************************************************
 * CreateWriter
 */
IMessagingWriter *RTISocketImpl::CreateWriter(const char *topic_name)
{

    int result = 0;

    NDDS_Transport_SendResource_t send_resource = NULL;
    NDDS_Transport_Port_t send_port = 0;

    int portOffset = 0;
    if (!strcmp(topic_name, perftest_cpp::_LatencyTopicName)) {
        portOffset += 1;
    }
    if (!strcmp(topic_name, perftest_cpp::_ThroughputTopicName)) {
        portOffset += 2;
    }

    send_port = PRESRtps_getWellKnownUnicastPort(
            _DomainID,
            0,
            7400,
            250,
            2,
            portOffset);

    result = _plugin->create_sendresource_srEA(
            _plugin,
            &send_resource,
            &_nicAddress,
            send_port,
            NDDS_TRANSPORT_PRIORITY_DEFAULT);
    if (result != 1) {
        fprintf(stderr, "create_sendresource_srEA error\n");
        return NULL;
    }

    return new RTISocketPublisher(
            _plugin,
            send_resource,
            _nicAddress,
            send_port,
            _pongSemaphore,
            _peersMap,
            _NumSubscribers,
            _worker,
            _batchSize);
}

/*********************************************************
 * CreateReader
 */
IMessagingReader *RTISocketImpl::CreateReader(const char *topic_name, IMessagingCB *callback) {
    NDDS_Transport_RecvResource_t recv_resource = NULL;
    NDDS_Transport_Port_t recv_port = 0;
    NDDS_Transport_Address_t * dst_multicast_addr = NULL;

    if (_IsMulticast) {
        dst_multicast_addr = &_multicastAddrTransp;
    }

    // We need different ports for the differents resources.
    int portOffset = 0;
    if (!strcmp(topic_name, perftest_cpp::_LatencyTopicName)) {
        portOffset += 1;
    }
    if (!strcmp(topic_name, perftest_cpp::_ThroughputTopicName)) {
        portOffset += 2;
    }

    recv_port = PRESRtps_getWellKnownUnicastPort(
            _DomainID, // Domain id
            0, // Participant id
            7400, // Port base
            250, // Domain id gain
            0, // Participant id gain
            portOffset); // Port offset

    /*
     * TODO:
     * If a create receive resource fail, and take another port the correspond
     * send resource will dont know where is the receiver.
     *
     * 1 -> Dont allow the receiver take differente port
     * 2 -> Communicate the port to the 'publisher' via announcement chanel
     */
    int result = 1;
    unsigned int count = 0;
    while(!_plugin->create_recvresource_rrEA(
                _plugin,
                &recv_resource,
                &recv_port,
                dst_multicast_addr,
                NDDS_TRANSPORT_PRIORITY_DEFAULT)) {
        recv_port += RTISocketImpl::resources_per_participant;
        if (++count == RTIPERFTEST_MAX_PORT_ATTEMPT) {
            result = 0;
            break;
        }
    }

    if (result != 1) {
        fprintf(stderr, "Create_recvresource_rrEA error\n");
        return NULL;
    }

    return new RTISocketSubscriber(_plugin, recv_resource, recv_port, _worker);
}

bool RTISocketImpl::ConfigureSocketsTransport() {

    struct NDDS_Transport_UDPv4_Property_t updv4_prop =
            NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
    struct NDDS_Transport_Shmem_Property_t shmem_prop =
            NDDS_TRANSPORT_SHMEM_PROPERTY_DEFAULT;

    struct NDDS_Transport_UDP *udpPlugin;

    /* If none -nic parameter has been received, assume the default interface */
    if (_transport.allowInterfaces.empty()) {
        _transport.allowInterfaces = std::string("127.0.0.1");
    }

    char *interface = DDS_String_dup(_transport.allowInterfaces.c_str());

    /* Worker configure */
    _workerFactory = REDAWorkerFactory_new(256);
    if (_workerFactory == NULL) {
        fprintf(stderr, "Error creating Worker Factory\n");
        return false;
    }

    _worker = REDAWorkerFactory_createWorker(_workerFactory, "RTISocketImpl");

    if (_worker == NULL) {
        fprintf(stderr, "Error creating Worker\n");
        return false;
    }

    _exclusiveArea = REDAWorkerFactory_createExclusiveArea(_workerFactory, 1);
    if (_exclusiveArea == NULL) {
        fprintf(stderr, "Error creating exclusive area\n");
        return false;
    }

    switch (_transport.transportConfig.kind) {

    /* Transport configure */
    case TRANSPORT_DEFAULT:
        /*Default transport is UDPv4*/

    case TRANSPORT_UDPv4:

        /*_Plugin properties configure for UDPv4*/
        updv4_prop.parent.properties_bitmap |=
                NDDS_TRANSPORT_PROPERTY_BIT_POLLED;

        updv4_prop.parent.allow_interfaces_list = &interface;
        updv4_prop.parent.allow_interfaces_list_length = 1;

        if (_IsMulticast) {
            updv4_prop.parent.allow_multicast_interfaces_list = &interface;
            updv4_prop.parent.allow_multicast_interfaces_list_length = 1;
            updv4_prop.reuse_multicast_receive_resource = 1;
            updv4_prop.multicast_enabled = 1;
        }

        /*
         * updv4_prop.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_ALWAYS;
         * This will reduce the package loss but affect on the performance.
         */
        updv4_prop.no_zero_copy = false;
        updv4_prop.parent.message_size_max =
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
        updv4_prop.parent.gather_send_buffer_count_max =
                NDDS_TRANSPORT_PROPERTY_GATHER_SEND_BUFFER_COUNT_MIN;

        /*
         * updv4_prop.recv_socket_buffer_size = ;
         * updv4_prop.send_socket_buffer_size = ;
         *
         * The default value for send_socket_buffer_size and receive is
         * 131072 and does not allow to go lower.
         * NDDS_TRANSPORT_UDPV4_SEND_SOCKET_BUFFER_SIZE_DEFAULT   (131072)
         * NDDS_TRANSPORT_UDPV4_RECV_SOCKET_BUFFER_SIZE_DEFAULT   (131072)
         *
         * This could be necessary to modify for large data.
         */

        // Setting the transport properties
        _plugin = NDDS_Transport_UDPv4_new(&updv4_prop);

        /*
         * With this, It's avoid the translation from a interface name in to
         * a address.
         */
        udpPlugin = (struct NDDS_Transport_UDP*)_plugin;
        if (udpPlugin->_interfacesCount == 0) {
            fprintf(stderr, "Input interface (%s) not recognize\n", interface);
            return false;
        }


        _nicAddress = udpPlugin->_interfaceArray[0]._interface.address;
        if (_IsMulticast) {
            _multicastAddrTransp =
                    udpPlugin->_interfaceArray[1]._interface.address;
        }
        if (_IsMulticast && NDDS_Transport_UDPv4_get_num_multicast_interfaces(
                        udpPlugin) <= 0) {
                fprintf(stderr, "No multicast-enabled interfaces detected\n");
                return false;
        }

        /*TODO: ??
         * Change the way it's parse the -peer option to allow a number of
         * peers.
         */

        /* TODO: Remove on final version!
         * I keep this part, may be helpfull if we decided to support 1-many
         */
        /* Peers parse to NDDS_Transport_Address_t */
        NDDS_Transport_Address_t addr;
        for (int i = 0; i < _peer_host_count; i++) {
            if (NDDS_Transport_UDP_string_to_address_cEA(
                        _plugin,
                        &addr,
                        _peer_host[i])) {
                unsigned int j;
                for (j = 0; j < _peersMap.size(); j++) {
                    if (_peersMap[j].first.network_ordered_value
                            == addr.network_ordered_value) {
                        _peersMap[j].second++;
                    }
                }
                if (j+1 == _peersMap.size() || _peersMap.size() == 0) {
                    _peersMap.push_back(
                            std::pair<NDDS_Transport_Address_t, int> (addr,1));
                }
            }
        }

        if (_peersMap.size() == 0) {
            fprintf(stderr, "Any peer correspond to a valid address\n");
            return false;
        }

        break;

    case TRANSPORT_SHMEM:

        /*_Plugin configure for shared memory*/

        shmem_prop.parent.properties_bitmap |=
                NDDS_TRANSPORT_PROPERTY_BIT_POLLED;
        shmem_prop.parent.message_size_max =
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX;
        shmem_prop.receive_buffer_size =
                NDDS_TRANSPORT_UDPV4_PAYLOAD_SIZE_MAX * 10;

        // shmem_prop.parent.allow_interfaces_list = &interface;
        // shmem_prop.parent.allow_interfaces_list_length = 1;
        shmem_prop.received_message_count_max =
                shmem_prop.receive_buffer_size / perftest_cpp::OVERHEAD_BYTES;
        /*
         * The total number of bytes that can be buffered in the receive
         * queue is:
         * size = receive_buffer_size + message_size_max +
         *          received_message_count_max * fixedOverhead
         */

        _plugin = NDDS_Transport_Shmem_new(&shmem_prop);


        NDDS_Transport_Interface_t interface;
        RTI_INT32 foundMore;
        RTI_INT32 count;
        _plugin->get_receive_interfaces_cEA(
                _plugin, &foundMore, &count, &interface, 1);

        /*TODO: What if the user want force a diferent nic? Should be allow it */
        if (count != 1) {
            fprintf(stderr, "Any valid interface for SHMEM found\n");
            return false;
        }
        _nicAddress = interface.address;

        _peersMap.push_back(std::pair<NDDS_Transport_Address_t, int>(
                _nicAddress, 1));

        break;

    case TRANSPORT_UDPv6:
    case TRANSPORT_TCPv4:
    case TRANSPORT_TLSv4:
    case TRANSPORT_DTLSv4:
    case TRANSPORT_WANv4:
    default:
        fprintf(stderr, "Socket transport only support UDPv4 & SHMEM\n");
        return false;

    } // Switch

    if (_plugin == NULL) {
        fprintf(stderr, "Error creating transport plugin\n");
        return false;
    }

    _transport.printTransportConfigurationSummary();

    return true;
}


/* TODO: Actually dont used funtion. */
char *InterfaceNameToAddress(const char *nicName) {

    if (nicName == NULL) {
        return NULL;
    }

    char * address = DDS_String_dup(nicName);

    char *outInterfaceBuffer = NULL;
    int outInterfaceBufferSize = 0;
    RTIOsapiSocket_InterfaceDescription *outInterfaceArray = NULL;
    int outInterfaceCount = 0;

    NDDS_Transport_Address_t transportAddress = NDDS_TRANSPORT_ADDRESS_INVALID;
    bool found = false;

    /* Interface to address translation */
    /*
     * TODO:
     * Check for windows, this function may not work.
     */
    /*Get all the interfaces on the current PC*/
    RTIOsapiSocket_getInterfaces(
            &outInterfaceBuffer,
            &outInterfaceBufferSize,
            &outInterfaceArray,
            &outInterfaceCount,
            RTI_OSAPI_SOCKET_AF_INET,
            0,
            0,
            0);

    /* Compare the list of interfaces with the input -nic*/
    for (int i = 0; i < outInterfaceCount; i++) {
        if (!strcmp(outInterfaceArray[i].name, address)) {
            NDDS_Transport_SocketUtil_Address_to_transportAddress(
                    outInterfaceArray[i].address,
                    &transportAddress,
                    RTI_OSAPI_SOCKET_AF_INET);
            found = true;
        }
    }

    /* Check if it is a valid IPv4 address */
    if (!NDDS_Transport_Address_is_ipv4(&transportAddress)) {
        fprintf(stderr, "The interface it's not recognize\n");
        return NULL;
    }

    if (found) {
        NDDS_Transport_Address_to_string(
                &transportAddress,
                address,
                NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE);
    }

    return address;

}

int NDDS_Transport_UDPv4_get_num_multicast_interfaces(
        struct NDDS_Transport_UDP *plugin){
    int count = 0;
    int i;

    for (i=0; i<plugin->_interfacesCount; i++) {

        if (plugin->_interfaceArray[i]._interfaceFlags &
            RTI_OSAPI_SOCKET_INTERFACE_FLAG_MULTICAST ) {
            ++count;
        }
    }
    return count;
}


#ifdef RTI_WIN32
#pragma warning(pop)
#endif
