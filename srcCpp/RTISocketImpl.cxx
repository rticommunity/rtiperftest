/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "RTISocketImpl.h"

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

int RTISocketImpl::_WaitsetEventCount = 5;
unsigned int RTISocketImpl::_WaitsetDelayUsec = 100;

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

    REDAWorkerFactory_destroyWorker(_workerFactory, _worker);
    REDAWorkerFactory_destroyExclusiveArea(_workerFactory, _exclusiveArea);
    REDAWorkerFactory_delete(_workerFactory);

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

    /* Parameters ot supported by sockets */
    char *no_socket_params[] {
        (char *) "-unbounded",
        (char *) "-sendQueueSize",
        (char *) "-heartbeatPeriod",
        (char *) "-fastHeartbeatPeriod",
        (char *) "-qosFile",
        (char *) "-qosLibrary",
        (char *) "-durability",
        (char *) "-dynamicData",
        (char *) "-noDirectCommunication",
        (char *) "-instances",
        (char *) "-instanceHashBuckets",
        (char *) "-batchSize",
        (char *) "-keepDurationUsec",
        (char *) "-noPositiveAcks",
        (char *) "-waitsetDelayUsec",
        (char *) "-waitsetEventCount",
        (char *) "-enableAutoThrottle",
        (char *) "-enableTurboMode",
        (char *) "-noXmlQos",
        (char *) "-asynchronous",
        (char *) "-flowController",
        (char *) "-cft",
        (char *) "-writeInstance",
        (char *) "-enableTCP",
        (char *) "-enableUDPv6",
        (char *) "-allowInterfaces",
        (char *) "-transportServerBindPort",
        (char *) "-transportWan",
        (char *) "-transportCertAuthority",
        (char *) "-transportCertFile",
        (char *) "-transportPrivateKey",
        (char *) "-transportWanServerAddress",
        (char *) "-transportWanServerPort",
        (char *) "-transportWanId",
        (char *) "-transportSecureWan"
    };

    std::vector<std::string> no_socket_params_v(
        no_socket_params,
        no_socket_params + sizeof(no_socket_params) / sizeof(char*)
    );

    /* Print all the non compatibles params together and return */
    bool found = false;
    std::vector<std::string>::iterator it;
    for (int i = 0; i < argc; ++i ) {
        it = find(
                no_socket_params_v.begin(),
                no_socket_params_v.end(),
                argv[i]);
        if (it != no_socket_params_v.end()) {
            fprintf(stderr, "%s: not compatible with -socket\n", argv[i]);
            found = true;
        }
    }

    if (found) {
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
                THROUGHPUT_MULTICAST_ADDR = argv[i];
                LATENCY_MULTICAST_ADDR = argv[i];
                ANNOUNCEMENT_MULTICAST_ADDR = argv[i];
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

    if (_DataLen > (unsigned long)MAX_SYNCHRONOUS_SIZE) {
        fprintf(stderr, "Large data settings enabled.\n");
        _isLargeData = true;
    }

    if (!_transport.parseTransportOptions(argc, argv)) {
        fprintf(stderr,
                "Failure parsing the transport options.\n");
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
    NDDS_Transport_SendResource_t _send_resource;
    NDDS_Transport_Address_t _dst_address;
    NDDS_Transport_Port_t _send_port;
    NDDS_Transport_Buffer_t _send_buffer;

    struct REDAWorker *_worker;

    RTIOsapiSemaphore *_pongSemaphore;

    TestData_t _data;

    int _peer_host_count;
    char **_peer_host;

    int _NumSubscribers;

  public:
    RTISocketPublisher(
            NDDS_Transport_Plugin *plugin,
            NDDS_Transport_SendResource_t send_resource,
            NDDS_Transport_Address_t dst_address,
            NDDS_Transport_Port_t send_port,
            RTIOsapiSemaphore *pongSemaphore,
            int peer_host_count,
            char **peer_host,
            int numSubscribers,
            struct REDAWorker *worker) {

        _plugin = plugin;
        _send_resource = send_resource;
        _dst_address = dst_address;
        _send_port = send_port;
        _pongSemaphore = pongSemaphore;

        _NumSubscribers = numSubscribers;

        _worker = worker;

        _peer_host_count = peer_host_count;
        _peer_host = peer_host;

        _send_buffer.length = 0;
        _send_buffer.pointer = 0;

        _data.bin_data.maximum(0);
    }

    ~RTISocketPublisher() {
        Shutdown();
    }

    void Shutdown() {
        if (_send_resource != NULL && _plugin != NULL) {
            _plugin->destroy_sendresource_srEA(_plugin, &_send_resource);
        }

    }

    void Flush() {
        /* Dummy funtion */
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey) {

        _data.entity_id = message.entity_id;
        _data.seq_num = message.seq_num;
        _data.timestamp_sec = message.timestamp_sec;
        _data.timestamp_usec = message.timestamp_usec;
        _data.latency_ping = message.latency_ping;
        _data.bin_data.ensure_length(message.size, message.size);
        // _data.bin_data.loan_contiguous(
        //         (DDS_Octet *)message.data,
        //         message.size,
        //         message.size);

        _send_buffer.length = message.size + perftest_cpp::OVERHEAD_BYTES;
        _send_buffer.pointer = (char *)& _data;

        //printf("message size: %d\n", _send_buffer.length);

        //TODO: //Ask Fernando
        /**
         * The 1th option is be restrictive, and only allow one subcriber
         * per subscriber.
         *
         * The 2th options is identify how many subscriber are per Peer
         * on the command line parameters like -peer x.x.x.x N
         *
         * The 3th option is always have the same numbers of subscriber per
         * Peer.
         *
         */

        /*
         * We don't know how many subscriber there are per Peer.
         * So, we do as many send() as the maximum of subscribers that
         * can be on a single peer.
         */
        int sends_per_peer =  (_NumSubscribers / _peer_host_count);

        int actual_port;

        bool result = true;
        for (int i=0; i < _peer_host_count; i++) {

            actual_port = _send_port;


            /* Only take the address if it's not been use SHMEM */
            if (_plugin->property->classid != NDDS_TRANSPORT_CLASSID_SHMEM) {
                result = NDDS_Transport_UDP_string_to_address_cEA(
                    _plugin,
                    &_dst_address,
                    _peer_host[i]);

                if (result != 1){
                    fprintf(stderr,
                            "NDDS_Transport_UDP_string_to_address_cEA error\n");
                    return false;
                }
            }

            for (int j=0; j < sends_per_peer; j++) {

                actual_port += j * RTISocketImpl::resources_per_participant;

                result = _plugin->send(
                    _plugin,
                    &_send_resource,
                    &_dst_address,
                    actual_port,
                    0,
                    &_send_buffer,
                    1,
                    _worker);

                if (!result) {
                    /**
                     * TODO:
                     * Set a log with warning level???? Not clear
                     *
                     * use -> RTILog_printContextAndMsg??
                     *
                     */
                    fprintf(stderr, "Write error using sockets\n");
                    return false;

                }
            }

        }


        //_data.bin_data.unloan();

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
        /*TODO: Set wait function???? may dont have sense*/
        //perftest_cpp::MilliSleep(10);
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
    NDDS_Transport_SendResource_t _recv_resource;
    NDDS_Transport_Buffer_t _recv_buffer;
    NDDS_Transport_Port_t _recv_port;

    NDDS_Transport_Message_t _transp_message;

    TestMessage _message;

    struct REDAWorker *_worker;

    TestData_t *_data;
    char * _payload;
    int _payload_size;

    RTIOsapiSemaphore *_pongSemaphore;


  public:
    RTISocketSubscriber(
        NDDS_Transport_Plugin *plugin,
        NDDS_Transport_SendResource_t recv_resource,
        NDDS_Transport_Port_t recv_port,
        struct REDAWorker *worker)
    {
        _plugin = plugin;
        _recv_resource = recv_resource;
        _recv_port = recv_port;
        _recv_buffer.length = 0;
        _recv_buffer.pointer = NULL;
        _worker = worker;

        // Similar to NDDS_Transport_Message_t message =
        //      NDDS_TRANSPORT_MESSAGE_INVALID;
        _transp_message.buffer.pointer = NULL;
        _transp_message.buffer.length = 0;
        _transp_message.loaned_buffer_param = NULL;

        //_worker._name = "SubscriberWorker";

        unsigned int size = MAX_SYNCHRONOUS_SIZE;

        RTIOsapiHeap_allocateArray(
            &_recv_buffer.pointer,
            size,
            char);

        if (_recv_buffer.pointer == NULL)
        {
            fprintf(stderr, "RTIOsapiHeap_allocateArray error\n");
        }
        _recv_buffer.length = size;

        //_data.bin_data.maximum(0);
        _payload_size = 0;
        _payload = 0;

    }

    ~RTISocketSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {

        if (_recv_resource != NULL && _plugin != NULL) {
            /*TODO: fail when is destroy*/
            //_plugin->destroy_recvresource_rrEA(_plugin, &_recv_resource);
        }

    }

    TestMessage *ReceiveMessage() {

        bool result = _plugin->receive_rEA(
                _plugin,
                &_transp_message,
                &_recv_buffer,
                &_recv_resource,
                _worker);
        if (!result) {
            fprintf(stderr, "error receiving data\n");
            return NULL;
        }

        _data = reinterpret_cast<TestData_t *>(_transp_message.buffer.pointer);

        _message.entity_id = _data->entity_id;
        _message.seq_num = _data->seq_num;
        _message.timestamp_sec = _data->timestamp_sec;
        _message.timestamp_usec = _data->timestamp_usec;
        _message.latency_ping = _data->latency_ping;
        _message.size = _data->bin_data.length();
        _message.data = (char *)_data->bin_data.get_contiguous_bufferI();

        if (_plugin -> return_loaned_buffer_rEA != NULL) {
            _plugin->return_loaned_buffer_rEA(
                _plugin,
                &_recv_resource,
                &_transp_message,
                NULL);
        }

        return &_message;
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

    if(_peer_host_count == 0) {
        _peer_host[0] = (char *) "127.0.0.1";
        _peer_host_count = 1;
    }

    // only if we run the latency test we need to wait for pongs after sending pings
    _pongSemaphore = _LatencyTest
        ? RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL)
        : NULL;

    if (!configureSocketsTransport()) {
        return false;
    }

    return true;
}

/*********************************************************
 * CreateWriter
 */
IMessagingWriter *RTISocketImpl::CreateWriter(const char *topic_name)
{

    int result = 0;

    NDDS_Transport_SendResource_t send_resource = NULL;
    NDDS_Transport_Port_t send_port = 0;
    NDDS_Transport_Address_t dst_address =
        NDDS_TRANSPORT_ADDRESS_INVALID;

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

    /* Create send resource */
    if (_plugin->property->classid != NDDS_TRANSPORT_CLASSID_SHMEM){
        result = NDDS_Transport_UDP_string_to_address_cEA(
                _plugin,
                &dst_address,
                _transport.allowInterfaces.c_str());

        if (result != 1) {
            fprintf(stderr, "NDDS_Transport_UDP_string_to_address_cEA error\n");
            return NULL;
        }
    }

    printf("Creating send resource, topic name: %s \t port: %d\n",
            topic_name, send_port);

    result = _plugin->create_sendresource_srEA(
            _plugin,
            &send_resource,
            &dst_address,
            send_port,
            0);
    if (result != 1) {
        fprintf(stderr, "create_sendresource_srEA error\n");
        return NULL;
    }

    return new RTISocketPublisher(
            _plugin,
            send_resource,
            dst_address,
            send_port,
            _pongSemaphore,
            _peer_host_count,
            (char**)_peer_host,
            _NumSubscribers,
            _worker);

}

/*********************************************************
 * CreateReader
 */
IMessagingReader *RTISocketImpl::CreateReader(
    const char *topic_name,
    IMessagingCB *callback)
{
    int result = 0;
    NDDS_Transport_RecvResource_t recv_resource = NULL;
    NDDS_Transport_Port_t recv_port = 0;

    int portOffset = 3 * perftest_cpp::_SubID;
    if (!strcmp(topic_name, perftest_cpp::_LatencyTopicName) )
    {
        portOffset += 1;
    }
    if (!strcmp(topic_name, perftest_cpp::_ThroughputTopicName))
    {
        portOffset += 2;
    }

    // parameters order (domain_id, participant_id, port_base, domain_id_gain, participant_id_gain, port_offset)
    recv_port = PRESRtps_getWellKnownUnicastPort(
            _DomainID,
            0,
            7400,
            250,
            2,
            portOffset);

    printf("Creating receive resource, topic name: %s \t port: %d\n", topic_name, recv_port);

    /* Create receive resource */
    result = _plugin->create_recvresource_rrEA(
            _plugin,
            &recv_resource,
            &recv_port,
            NULL,
            0);

    if (result != 1) {
        fprintf(stderr, "Create_recvresource_rrEA error\n");
        return NULL;
    }

    return new RTISocketSubscriber(
            _plugin,
            recv_resource,
            recv_port,
            _worker);
}

bool RTISocketImpl::configureSocketsTransport()
{
    struct NDDS_Transport_UDPv4_Property_t updv4_prop =
            NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
    struct NDDS_Transport_Shmem_Property_t shmem_prop =
            NDDS_TRANSPORT_SHMEM_PROPERTY_DEFAULT;

    char * outInterfaceBuffer = NULL;
    int outInterfaceBufferSize = 0;
    RTIOsapiSocket_InterfaceDescription *outInterfaceArray = NULL;
    int outInterfaceCount = 0;

    /* If none -nic parameter has been received, assume the default interface */
    if (_transport.allowInterfaces.empty()) {
        _transport.allowInterfaces = std::string("127.0.0.1");
    }

    char *interface = new char[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    strcpy(interface, _transport.allowInterfaces.c_str());

    NDDS_Transport_Address_t transportAddress = NDDS_TRANSPORT_ADDRESS_INVALID;
    bool found = false;

    /*Worker configure*/
    _workerFactory = REDAWorkerFactory_new(200);
    if (_workerFactory == NULL)
    {
        fprintf(stderr, "Error creating Worker Factory\n");
        return false;
    }

    _worker = REDAWorkerFactory_createWorker( _workerFactory, "RTISockerImpl");

    if (_worker == NULL)
    {
        fprintf(stderr, "Error creating Worker\n");
        return false;
    }

    _exclusiveArea = REDAWorkerFactory_createExclusiveArea(_workerFactory, 1);
    if (_exclusiveArea == NULL){
        fprintf(stderr, "Error creating exclusive area\n");
        return false;
    }

    switch (_transport.transportConfig.kind) {

        case TRANSPORT_DEFAULT:
            /*Default transport is UDPv4*/

        case TRANSPORT_UDPv4:

            /*_Plugin configure for UDPv4*/

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
                    0, 0, 0);

            /* Compare the list of interfaces with the imput -nic*/
            for (int i = 0; i < outInterfaceCount; i++) {
                if (!strcmp(outInterfaceArray[i].name, interface)) {
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
                return false;
            }

            if (found) {
                NDDS_Transport_Address_to_string(
                        &transportAddress,
                        interface,
                        NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE);
            }

            updv4_prop.parent.allow_interfaces_list = &interface;
            updv4_prop.parent.allow_interfaces_list_length = 1;

            /*
             * This will reduce the package loss but affect on the performance.
             * updv4_prop.send_blocking = NDDS_TRANSPORT_UDPV4_BLOCKING_ALWAYS;
             */
            updv4_prop.no_zero_copy = false;
            updv4_prop.parent.message_size_max = MAX_SYNCHRONOUS_SIZE;
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
            * This could be necesary to modify for large data.
            */

            _plugin = NDDS_Transport_UDPv4_new(&updv4_prop);

            _transport.allowInterfaces = std::string(interface);

            break;

        case TRANSPORT_SHMEM:

            /*_Plugin configure for shared memory*/

            shmem_prop.parent.message_size_max = MAX_SYNCHRONOUS_SIZE;
            shmem_prop.receive_buffer_size = MAX_SYNCHRONOUS_SIZE * 10;
            shmem_prop.received_message_count_max =
                shmem_prop.receive_buffer_size / perftest_cpp::OVERHEAD_BYTES;
            /*
             * The total number of bytes that can be buffered in the receive
             * queue is:
             * size = receive_buffer_size + message_size_max +
             *          received_message_count_max * fixedOverhead
             */

            _plugin = NDDS_Transport_Shmem_new(&shmem_prop);

            break;

        case TRANSPORT_UDPv6:
        case TRANSPORT_TCPv4:
        case TRANSPORT_TLSv4:
        case TRANSPORT_DTLSv4:
        case TRANSPORT_WANv4:
        default:
            fprintf(stderr,
                    "Socket transport only support UDPv4 & SHMEM\n");
            return false;

    } // Switch

    if (_plugin == NULL) {
        fprintf(stderr, "Error creating transport plugin\n");
        return false;
    }

    return true;
}

#ifdef RTI_WIN32
#pragma warning(pop)
#endif
