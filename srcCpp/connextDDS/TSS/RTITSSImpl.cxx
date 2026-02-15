/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "RTITSSImpl.h"
#include "CustomConfiguration.hpp"

#include "rti_tss_impl.h"

#ifdef RTI_PERF_TSS_PRO
  #include "ndds/ndds_cpp.h"
#endif

const std::string GetMiddlewareVersionString()
{
    return std::string(RTI_TSS_VER_INFO_STR);
}

template <class Type, class TypedTS, class TypedCB>
RTITSSImpl<Type, TypedTS, TypedCB>::RTITSSImpl(const char*type_name) :
            _tss(new RTI::TSS::Base),
            _pong_semaphore(NULL),
            _pm(NULL)
{
    _type_name = type_name;

    _qoSProfileNameMap[LATENCY_TOPIC_NAME] = std::string("LatencyQos");
    _qoSProfileNameMap[ANNOUNCEMENT_TOPIC_NAME] = std::string("AnnouncementQos");
    _qoSProfileNameMap[THROUGHPUT_TOPIC_NAME] = std::string("ThroughputQos");

#ifdef PERFTEST_RTI_PRO
    _instanceMaxCountReader = DDS_LENGTH_UNLIMITED;
#else
    /*
     * For micro we want to restrict the use of memory, and since we need
     * to set a maximum (other than DDS_LENGTH_UNLIMITED), we decided to use
     * a default of 1. This means that for Micro, we need to specify the
     * number of instances that will be received in the reader side.
     */
    _instanceMaxCountReader = 1;
#endif

    _maxUnfragmentedRTPSPayloadSize = MESSAGE_SIZE_MAX_NOT_SET;

    _isLargeData = false;
}

template <class Type, class TypedTS, class TypedCB>
const std::string RTITSSImpl<Type, TypedTS, TypedCB>::get_qos_profile_name(const char *topicName)
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

template <class Type, class TypedTS, class TypedCB>
void RTITSSImpl<Type, TypedTS, TypedCB>::configure_middleware_verbosity(
        int verbosity_level)
{
#ifdef RTI_PERF_TSS_PRO

    NDDS_Config_LogVerbosity verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
    switch (verbosity_level) {
        case 0: verbosity = NDDS_CONFIG_LOG_VERBOSITY_SILENT;
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
        case 1: verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
        case 2: verbosity = NDDS_CONFIG_LOG_VERBOSITY_WARNING;
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
        case 3: verbosity = NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL;
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
        default: fprintf(stderr,
                    "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                break;
    }
    NDDSConfigLogger::get_instance()->set_verbosity(verbosity);

#else // defined(RTI_PERF_TSS_MICRO)

    OSAPI_LogVerbosity_T verbosity = OSAPI_LOG_VERBOSITY_ERROR;
    switch (verbosity_level) {
        case 0: verbosity = OSAPI_LOG_VERBOSITY_SILENT;
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
        case 1: verbosity = OSAPI_LOG_VERBOSITY_ERROR;
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
        case 2: verbosity = OSAPI_LOG_VERBOSITY_WARNING;
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
        case 3: verbosity = OSAPI_LOG_VERBOSITY_DEBUG;
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
        default: fprintf(stderr,
                    "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                break;
    }
    OSAPI_Log_set_verbosity(verbosity);

#endif
}

/* RTITSSImpl implementation */
template <class Type, class TypedTS, class TypedCB>
bool RTITSSImpl<Type, TypedTS, TypedCB>::initialize(ParameterManager &PM, perftest_cpp *parent)
{
    UNUSED_ARG(parent);

    FACE::RETURN_CODE_TYPE::Value retcode;

    _pm = &PM;
    _transport.initialize(_pm);
    if (!validate_input()) {
        fprintf(stderr, "Failed to validate test input\n");
		return false;
    }

    _customConfig = new CustomConfiguration(_type_name);

    _tss->Set_Reference("ThisIsIgnored", &_customConfig, 0, retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        fprintf(stderr, "Failed Set_Reference (rc=%d)\n", retcode);
        return false;
    }

    if (!SetSystemConfig(_customConfig, "perftest"))
    {
        fprintf(stderr, "Failed to set SystemConfig\n");
        return false;
    }

    ((CustomConfiguration*)_tss)->Initialize("perftest", retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Initialize(rc=%d)\n", retcode);
		return false;
	}

    if (!SetDomainParticipantConfig("domain_0"))
    {
        fprintf(stderr, "Failed to set DomainParticipantConfig\n");
        return false;
    }

    if (_pm->get<bool>("latencyTest")) {
        _pong_semaphore = PerftestSemaphore_new();
        if (_pong_semaphore == NULL) {
            fprintf(stderr, "Failed create pong semaphore\n");
		    return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
DDS_Boolean
RTITSSImpl<Type, TypedTS, TypedCB>::SetDomainParticipantConfig(
                const char* domain_cfg_name)
{
    QoSBundle* participant_qos_bundle = new QoSBundle;

    participant_qos_bundle->pm = _pm;
    participant_qos_bundle->transport = &_transport;

    participant_qos_bundle->topic_name = "ThisIsIgnored";

    // instanceMaxCountReader is not used for configuring participant QoS
    participant_qos_bundle->instanceMaxCountReader = 0;

    participant_qos_bundle->isLargeData = &_isLargeData;

    participant_qos_bundle->maxUnfragmentedRTPSPayloadSize = &_maxUnfragmentedRTPSPayloadSize;

    FACE::RETURN_CODE_TYPE::Value return_code;
    FACE::Configuration::HANDLE_TYPE domain_config_handler;
    FACE::Configuration::BUFFER_SIZE_TYPE bytes_read = 0;
    RTI_TSS_DDS_DomainParticipant_Configuration_T* domain_config = NULL;

    _customConfig->Open(
            RTI_TSS_DOMAIN_PARTICIPANT_CONFIGURATION_CONTAINER_NAME,
            domain_config_handler,
            return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }

    _customConfig->Read(
            domain_config_handler,
            domain_cfg_name,
            (void *)&domain_config,
            sizeof(&domain_config),
            bytes_read,
            return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }
    if (domain_config == NULL)
    {
        return false;
    }

    domain_config->domain_id = _pm->get<int>("domain");

    domain_config->user_data = (void*)participant_qos_bundle;
    domain_config->configure_domain_participant_qos_fn =
            RTI_TSS_participant_qos;

    return true;
}

template <class Type, class TypedTS, class TypedCB>
DDS_Boolean
RTITSSImpl<Type, TypedTS, TypedCB>::GetSystemConfig(
        FACE::Configuration *configuration,
        const FACE::CONFIGURATION_RESOURCE &name,
        RTI_TSS_System_Configuration_T *&sys_cfg)
{
    FACE::RETURN_CODE_TYPE::Value return_code;
    FACE::Configuration::HANDLE_TYPE system_config_handler;
    FACE::Configuration::BUFFER_SIZE_TYPE bytes_read = 0;

    configuration->Initialize(name, return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }

    configuration->Open(
            RTI_TSS_SYSTEM_CONFIGURATION_CONTAINER_NAME,
            system_config_handler,
            return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }

    configuration->Read(
            system_config_handler,
            name,
            (void *)&sys_cfg,
            sizeof(&sys_cfg),
            bytes_read,
            return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }
    if (sys_cfg == NULL)
    {
        return false;
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
DDS_Boolean
RTITSSImpl<Type, TypedTS, TypedCB>::SetSystemConfig(
        FACE::Configuration *configuration,
        const FACE::CONFIGURATION_RESOURCE &name)
{
    RTI_TSS_System_Configuration_T *sys_config = NULL;

    if (!GetSystemConfig(configuration, name, sys_config))
    {
        printf("Failed to get system config\n");
        return DDS_BOOLEAN_FALSE;
    }

#ifdef RTI_PERF_TSS_MICRO
    sys_config->max_connections = 3;
    sys_config->max_topics = 3;
#endif

#ifdef RTI_PERF_TSS_PRO
    if (!_pm->get<bool>("noXmlQos"))
    {
        sys_config->xml_qos_file =
                DDS_String_dup(_pm->get<std::string>("qosFile").c_str());
        sys_config->qos_library =
                DDS_String_dup(_pm->get<std::string>("qosLibrary").c_str());
        sys_config->qos_profile = DDS_String_dup("BaseProfileQos");
    }
#endif

    return DDS_BOOLEAN_TRUE;
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSImpl<Type, TypedTS, TypedCB>::shutdown()
{
    FACE::RETURN_CODE_TYPE::Value retcode;

    for (unsigned long i = 0; i < _connections.size(); ++i) {
        _tss->Destroy_Connection(_connections[i], retcode);
        if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		    fprintf(stderr, "Failed destroy connection (rc=%d)\n", retcode);
	    }
    }

    if(_pong_semaphore != NULL) {
        PerftestSemaphore_delete(_pong_semaphore);
        _pong_semaphore = NULL;
    }
}

/*********************************************************

 * Validate and manage the parameters
 */
template <class Type, class TypedTS, class TypedCB>
bool RTITSSImpl<Type, TypedTS, TypedCB>::validate_input()
{
    // Manage parameter -instance
    if (_pm->is_set("instances")) {
        _instanceMaxCountReader = _pm->get<long>("instances");
    }

    // Manage parameter -writeInstance
    if (_pm->is_set("writeInstance")) {
        if (_pm->get<long>("instances") < _pm->get<long>("writeInstance")) {
            fprintf(stderr,
                    "Specified '-WriteInstance' (%ld) invalid: "
                    "Bigger than the number of instances (%ld).\n",
                    _pm->get<long>("writeInstance"),
                    _pm->get<long>("instances"));
            return false;
        }
    }

    // Manage parameter -peer
    if (_pm->get_vector<std::string>("peer").size() >= RTIPERFTEST_MAX_PEERS) {
        fprintf(stderr,
                "The maximum of 'initial_peers' is %d\n",
                RTIPERFTEST_MAX_PEERS);
        return false;
    }

    // Manage parameter -batchSize
    if (_pm->get<long>("batchSize") > 0) {
        // We will not use batching for a latency test
        if (_pm->get<bool>("latencyTest")) {
            if (_pm->is_set("batchSize")) {
                fprintf(stderr, "Batching cannot be used in a Latency test.\n");
                return false;
            } else {
                _pm->set<long>("batchSize", 0); // Disable Batching
            }
        }
    }

    // Manage transport parameter
    if (!_transport.validate_input()) {
        fprintf(stderr, "Failure validating the transport options.\n");
        return false;
    };

    /*
     * Manage parameter -verbosity.
     * Setting verbosity if the parameter is provided
     */
    if (_pm->is_set("verbosity")) {
        configure_middleware_verbosity(_pm->get<int>("verbosity"));
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
std::string RTITSSImpl<Type, TypedTS, TypedCB>::print_configuration()
{
    std::ostringstream stringStream;

    stringStream << std::endl << "TSS Configuration:" << std::endl;

    stringStream << "\tDomain: " << _pm->get<int>("domain") << std::endl;

    stringStream << "\tFACE Profile: ";
#if ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_NONE
    stringStream << "None";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_SECURITY
    stringStream << "Security";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_SAFETY_BASE
    stringStream << "Safety Base";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_SAFETY_EXTENDED
    stringStream << "Safety Extended";
#elif ENABLE_FACE_COMPLIANCE == FACE_COMPLIANCE_LEVEL_GENERAL
    stringStream << "General Purpose";
#else
    #warning "Unknown FACE profile";
    stringStream << "Unknown - " << ENABLE_FACE_COMPLIANCE;
#endif

    stringStream << std::endl;

    stringStream << "\tXML File: " << _pm->get<std::string>("qosFile") << std::endl;

    stringStream << std::endl << _transport.printTransportConfigurationSummary();

    return stringStream.str();
}

template <class Type, class TypedTS, class TypedCB>
FACE::TSS::CONNECTION_ID_TYPE RTITSSImpl<Type, TypedTS, TypedCB>::_createConnection(
        std::string connection_name,
        std::string topic_name,
        FACE::RETURN_CODE_TYPE::Value &retcode)
{
    FACE::TSS::MESSAGE_SIZE_TYPE max_message_size;
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::TIMEOUT_TYPE timeout(0);

    InstrumentConnection(connection_name.c_str(), topic_name.c_str());

    _tss->Create_Connection(connection_name.c_str(),
                            timeout,
                            connection_id,
                            max_message_size,
                            retcode);

    return connection_id;
}

template <class Type, class TypedTS, class TypedCB>
DDS_Boolean
RTITSSImpl<Type, TypedTS, TypedCB>::InstrumentConnection(
        const FACE::TSS::CONNECTION_NAME_TYPE &connection_name,
        const char* topic_name)
{
    RTI_TSS_Connection_Configuration_T *connection_config = NULL;
    QoSBundle* connection_qos_bundle = new QoSBundle;

    connection_qos_bundle->pm = _pm;
    connection_qos_bundle->transport = &_transport;

    connection_qos_bundle->topic_name = const_cast<char*>(topic_name);

    connection_qos_bundle->instanceMaxCountReader = _instanceMaxCountReader;

    connection_qos_bundle->isLargeData = &_isLargeData;

    connection_qos_bundle->maxUnfragmentedRTPSPayloadSize = &_maxUnfragmentedRTPSPayloadSize;

    if (!GetConnectionConfig(connection_name, connection_config))
    {
        printf("Failed to get connection config.\n");
        return DDS_BOOLEAN_FALSE;
    }

#ifdef RTI_PERF_TSS_PRO
    if (!_pm->get<bool>("noXmlQos"))
    {
        connection_config->qos_library =
            DDS_String_dup(_pm->get<std::string>("qosLibrary").c_str());
        connection_config->qos_profile =
            DDS_String_dup(get_qos_profile_name(topic_name).c_str());
    }
#endif

    connection_config->user_data = (void *)connection_qos_bundle;

    connection_config->configure_datawriter_qos_fn = RTI_TSS_datawriter_qos;
    connection_config->configure_datareader_qos_fn = RTI_TSS_datareader_qos;

    return DDS_BOOLEAN_TRUE;
}

template <class Type, class TypedTS, class TypedCB>
DDS_Boolean
RTITSSImpl<Type, TypedTS, TypedCB>::GetConnectionConfig(
        const FACE::TSS::CONNECTION_NAME_TYPE &connection_name,
        RTI_TSS_Connection_Configuration_T *&connection_config)
{
    FACE::RETURN_CODE_TYPE::Value return_code;
    FACE::Configuration::HANDLE_TYPE connection_config_handler;
    FACE::Configuration::BUFFER_SIZE_TYPE bytes_read = 0;

    _customConfig->Open(
            RTI_TSS_CONNECTION_CONFIGURATION_CONTAINER_NAME,
            connection_config_handler,
            return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }

    _customConfig->Read(
            connection_config_handler,
            connection_name,
            (void *)&connection_config,
            sizeof(&connection_config),
            bytes_read,
            return_code);
    if (return_code != FACE::RETURN_CODE_TYPE::NO_ERROR)
    {
        return false;
    }
    if (connection_config == NULL)
    {
        return false;
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
IMessagingWriter *RTITSSImpl<Type, TypedTS, TypedCB>::create_writer(const char *topic_name)
{
    std::string name = "writer " + std::string(topic_name);
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::RETURN_CODE_TYPE::Value retcode;

    connection_id = _createConnection(name, std::string(topic_name), retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Create_Connection %s (writer) (rc=%d)\n",
                name.c_str(), retcode);
        return NULL;
	}

    _connections.push_back(connection_id);

    return new RTITSSPublisher<Type, TypedTS, TypedCB>(
                connection_id,
                _pm->get<long>("instances"),
                _pong_semaphore,
                _pm->get<long>("writeInstance"),
                _pm->get<bool>("loaningSendReceive"));
}

template <class Type, class TypedTS, class TypedCB>
IMessagingReader *RTITSSImpl<Type, TypedTS, TypedCB>::create_reader(const char *topic_name,
                                              IMessagingCB *callback)
{
    std::string name = "reader " + std::string(topic_name);
    FACE::TSS::CONNECTION_ID_TYPE connection_id;
    FACE::RETURN_CODE_TYPE::Value retcode;

    connection_id = _createConnection(name, std::string(topic_name), retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Create_Connection %s (reader) (rc=%d)\n",
                name.c_str(), retcode);
        return NULL;
	}

    _connections.push_back(connection_id);

    return new RTITSSSubscriber<Type, TypedTS, TypedCB>(
                connection_id,
                _pm->get<long>("instances"),
                _pong_semaphore,
                _pm->get<long>("writeInstances"),
                callback,
                _pm->get<bool>("loaningSendReceive"));
}

template <class Type, class TypedTS, class TypedCB>
unsigned long RTITSSImpl<Type, TypedTS, TypedCB>::get_initial_burst_size()
{
    return _pm->get<unsigned long>("sendQueueSize");
}

#ifdef RTI_PERF_TSS_PRO

template <>
int RTITSSImpl<FACE::DM::TestData_t,
               TestData_t::TypedTS,
               TestData_t::Read_Callback>::_serializeTyped(
        FACE::DM::TestData_t *data, unsigned int &size)
{
    return FACE_DM_TestData_tPlugin_serialize_to_cdr_buffer(NULL, &size, (FACE_DM_TestData_t *)data);
}

template <>
int RTITSSImpl<FACE::DM::TestDataLarge_t,
               TestDataLarge_t::TypedTS,
               TestDataLarge_t::Read_Callback>::_serializeTyped(
        FACE::DM::TestDataLarge_t *data, unsigned int &size)
{
    return FACE_DM_TestDataLarge_tPlugin_serialize_to_cdr_buffer(NULL, &size, (FACE_DM_TestDataLarge_t *)data);
}


template <>
int RTITSSImpl<FACE::DM::TestDataKeyed_t,
               TestDataKeyed_t::TypedTS,
               TestDataKeyed_t::Read_Callback>::_serializeTyped(
        FACE::DM::TestDataKeyed_t *data, unsigned int &size)
{
    return FACE_DM_TestDataKeyed_tPlugin_serialize_to_cdr_buffer(NULL, &size, (FACE_DM_TestDataKeyed_t *)data);
}

template <>
int RTITSSImpl<FACE::DM::TestDataKeyedLarge_t,
               TestDataKeyedLarge_t::TypedTS,
               TestDataKeyedLarge_t::Read_Callback>::_serializeTyped(
        FACE::DM::TestDataKeyedLarge_t *data, unsigned int &size)
{
    return FACE_DM_TestDataKeyedLarge_tPlugin_serialize_to_cdr_buffer(NULL, &size, (FACE_DM_TestDataKeyedLarge_t *)data);
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSImpl<Type, TypedTS, TypedCB>::get_serialized_overhead_size(unsigned int &overhead_size)
{
    Type data;
    data.entity_id = 0;
    data.seq_num = 0;
    data.timestamp_sec = 0;
    data.timestamp_usec = 0;
    data.latency_ping = 0;

    if (!DDS_OctetSeq_ensure_length((DDS_OctetSeq*)&data.bin_data, 0, 0)) {
        fprintf(stderr, "Failed DDS_OctetSeq_ensure_length\n");
        return false;
    }

    if (_serializeTyped(&data, overhead_size) != DDS_RETCODE_OK) {
        fprintf(stderr, "Failed to serialize to CDR buffer\n");
        return false;
    }

    overhead_size -= RTI_CDR_ENCAPSULATION_HEADER_SIZE;

    return true;
}

#endif /* RTI_PERF_TSS_PRO */

/* RTITSSPublisher implementation */
template <class Type, class TypedTS, class TypedCB>
RTITSSPublisher<Type, TypedTS, TypedCB>::RTITSSPublisher(
            FACE::TSS::CONNECTION_ID_TYPE connection_id,
            unsigned long num_instances,
            PerftestSemaphore * pongSemaphore,
            long instancesToBeWritten,
            bool loaning)
        : TSSConnection<Type, TypedTS, TypedCB>(connection_id,
                                                num_instances,
                                                instancesToBeWritten,
                                                loaning),
          _pong_semaphore(pongSemaphore)
{
    DDS_DataWriterQos qos;
    DDS_DataWriterQos_initialize(&qos);
    DDS_DataWriter_get_qos(this->_writer, &qos);
    _is_reliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

    if (!loaning)
    {
        this->_send_function = &TSSConnection<Type, TypedTS, TypedCB>::_send;
    }
    else
    {
        this->_send_function = &TSSConnection<Type, TypedTS, TypedCB>::_send_loaning;
    }
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::send(const TestMessage &message,
                                          bool isCftWildCardKey)
{
    return (this->*_send_function)(message, isCftWildCardKey);
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSPublisher<Type, TypedTS, TypedCB>::wait_for_ack(int sec, unsigned int nsec)
{
#ifdef RTI_PERF_TSS_PRO
    if (_is_reliable) {
        DDS_Duration_t timeout = {sec, nsec};
        DDS_DataWriter_wait_for_acknowledgments(this->_writer, &timeout);
    } else {
        PerftestClock::milliSleep(nsec / 1000000);
    }
#else
    UNUSED_ARG(sec);
#endif

    // Since there is no implementation of wait_for_ack() in Micro 2.4.x,
    // an sleep is performed in both reliable and best-effort
    PerftestClock::milliSleep(nsec / 1000000);
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSPublisher<Type, TypedTS, TypedCB>::wait_for_readers(int numSubscribers)
{
    DDS_PublicationMatchedStatus status;
    DDS_ReturnCode_t retcode;

    while (true) {
        retcode = DDS_DataWriter_get_publication_matched_status(this->_writer, &status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "WaitForReaders failed: %d\n", retcode);
        }

        if (status.current_count >= numSubscribers) {
            break;
        }

        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
    }
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::wait_for_ping_response()
{
    if(_pong_semaphore != NULL) {
        if (!PerftestSemaphore_take(
                    _pong_semaphore, PERFTEST_SEMAPHORE_TIMEOUT_INFINITE)) {
            fprintf(stderr, "Error taking semaphore\n");
            return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::wait_for_ping_response(int timeout)
{
    if(_pong_semaphore != NULL) {
        if (!PerftestSemaphore_take(_pong_semaphore, timeout)) {
            fprintf(stderr, "Error taking semaphore\n");
            return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
bool RTITSSPublisher<Type, TypedTS, TypedCB>::notify_ping_response()
{
    if(_pong_semaphore != NULL) {
        if (!PerftestSemaphore_give(_pong_semaphore)) {
            fprintf(stderr, "Error giving semaphore\n");
            return false;
        }
    }

    return true;
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSPublisher<Type, TypedTS, TypedCB>::get_pulled_sample_count()
{
#ifdef RTI_PERF_TSS_PRO
    DDS_DataWriterProtocolStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataWriter_get_datawriter_protocol_status(this->_writer, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "Error getting protocol status\n");
        return -1;
    }

    return (unsigned int) status.pulled_sample_count;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSPublisher<Type, TypedTS, TypedCB>::get_sample_count()
{
#ifdef RTI_PERF_TSS_PRO
    DDS_DataWriterCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataWriter_get_datawriter_cache_status(this->_writer, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datawriter_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int) status.sample_count;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSPublisher<Type, TypedTS, TypedCB>::get_sample_count_peak()
{
#ifdef RTI_PERF_TSS_PRO
    DDS_DataWriterCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataWriter_get_datawriter_cache_status(this->_writer, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datawriter_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int)status.sample_count_peak;
#else
    // Not supported in Micro
    return 0;
#endif
}


/* RTITSSSubscriber Implementation */
template <class Type, class TypedTS, class TypedCB>
RTITSSSubscriber<Type, TypedTS, TypedCB>::RTITSSSubscriber(
                    FACE::TSS::CONNECTION_ID_TYPE connection_id,
                    unsigned long num_instances,
                    PerftestSemaphore * pongSemaphore,
                    long instancesToBeWritten,
                    IMessagingCB *callback,
                    bool loaning)
        : TSSConnection<Type, TypedTS, TypedCB>(connection_id,
                                                num_instances,
                                                instancesToBeWritten,
                                                loaning,
                                                callback),
          _pong_semaphore(pongSemaphore)
{
    if (!loaning)
    {
        this->_receive_function = &TSSConnection<Type, TypedTS, TypedCB>::_receive;
    }
    else
    {
        this->_receive_function = &TSSConnection<Type, TypedTS, TypedCB>::_receive_loaning;
    }
}

template <class Type, class TypedTS, class TypedCB>
TestMessage *RTITSSSubscriber<Type, TypedTS, TypedCB>::receive_message()
{
    return (this->*_receive_function)();
}

template <class Type, class TypedTS, class TypedCB>
void RTITSSSubscriber<Type, TypedTS, TypedCB>::wait_for_writers(int numPublishers)
{
    DDS_SubscriptionMatchedStatus status;
    DDS_ReturnCode_t retcode;

    while (true) {
        retcode = DDS_DataReader_get_subscription_matched_status(this->_reader, &status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "wait_for_writers failed: %d\n", retcode);
        }

        if (status.current_count >= numPublishers) {
            break;
        }

        PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
    }
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSSubscriber<Type, TypedTS, TypedCB>::get_sample_count()
{
#ifdef RTI_PERF_TSS_PRO
    DDS_DataReaderCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataReader_get_datareader_cache_status(this->_reader, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datareader_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int)status.sample_count;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
unsigned int RTITSSSubscriber<Type, TypedTS, TypedCB>::get_sample_count_peak()
{
#ifdef RTI_PERF_TSS_PRO
    DDS_DataReaderCacheStatus status;
    DDS_ReturnCode_t retcode;

    retcode = DDS_DataReader_get_datareader_cache_status(this->_reader, &status);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "get_datareader_cache_status failed: %d.\n", retcode);
        return -1;
    }

    return (unsigned int)status.sample_count_peak;
#else
    // Not supported in Micro
    return 0;
#endif
}

template <class Type, class TypedTS, class TypedCB>
TSSConnection<Type, TypedTS, TypedCB>::TSSConnection(FACE::TSS::CONNECTION_ID_TYPE connection_id,
                  unsigned long num_instances,
                  long instancesToBeWritten,
                  bool loaning,
                  IMessagingCB *callback)
        : _connection_id(connection_id), _typedTS(new TypedTS), _typedCB(NULL)
{
    _num_instances = num_instances;
    _instancesToBeWritten = instancesToBeWritten;
    _instance_counter = 0;

    RTI_TSS_Impl *rti_tss = NULL;
    DDS_DomainParticipant *participant = NULL;
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE_CONNECTION_DIRECTION_TYPE direction;
    struct RTI_TSS_ConnectionEntry *connection_entry = NULL;
    bool set_writer = false;
    bool set_reader = false;

    rti_tss = RTI_TSS_Impl_get_instance();
    if (rti_tss == NULL) {
        fprintf(stderr, "Failed to get TSS impl instance.\n");
        return;
    }

    connection_entry = RTI_TSS_ConnectionManager_lookup_connection_id(
            rti_tss->connection_mgr, _connection_id);
    if (connection_entry == NULL) {
        fprintf(stderr, "Failed to get connection entry.\n");
        return;
    }

    participant = connection_entry->participant;
    if (participant == NULL) {
        fprintf(stderr, "Failed to get participant from connection.\n");
        return;
    }

    direction = RTI_TSS_ConnectionEntry_get_connection_direction(connection_entry);
    if (direction == FACE_SOURCE) {
        set_writer = true;
    } else if (direction == FACE_DESTINATION) {
        set_reader = true;
    } else {
        set_writer = true;
        set_reader = true;
    }

    if (set_writer) {
        _writer = connection_entry->writer;
        if (_writer == NULL) {
            fprintf(stderr, "Failed to get writer from connection.\n");
            return;
        }
    }

    if (set_reader) {
        _reader = connection_entry->reader;
        if (_reader == NULL) {
            fprintf(stderr, "Failed to get reader from connection.\n");
            return;
        }
    }

    if (callback != NULL) {
        if (!loaning)
        {
            _typedCB = new TSSListener<Type, TypedCB>(callback);
        }
        else
        {
            _typedCB = new TSSListenerLoaning<Type, TypedCB>(callback);
        }
        _typedTS->Register_Callback(_connection_id, *_typedCB, retcode);
        if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
            printf("Failed Register_Callback (rc=%d)\n", retcode);
            return;
        }
    }
}

template <class Type, class TypedTS, class TypedCB>
TSSConnection<Type, TypedTS, TypedCB>::~TSSConnection()
{
    if (_typedTS != NULL) delete _typedTS;
    if (_typedCB != NULL) delete _typedCB;
}

template <class Type, class TypedTS, class TypedCB>
inline bool TSSConnection<Type, TypedTS, TypedCB>::_send(
            const TestMessage &message,
            bool isCftWildCardKey)
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TIMEOUT_TYPE timeout(0);
    unsigned long key = MAX_CFT_VALUE;
    int seq_length = 0;

    // Calculate key and add it if using more than one instance
    if (!isCftWildCardKey) {
        if (_num_instances > 1) {
            if (_instancesToBeWritten == -1) {
                key = _instance_counter++ % _num_instances;
            } else { // send sample to a specific subscriber
                key = _instancesToBeWritten;
            }
        }
    }

    for (int c = 0; c < KEY_SIZE; ++c) {
        _sample.key[c] = (unsigned char)(key >> c * 8);
    }

    _sample.entity_id = message.entity_id;
    _sample.seq_num = message.seq_num;
    _sample.timestamp_sec = message.timestamp_sec;
    _sample.timestamp_usec = message.timestamp_usec;
    _sample.latency_ping = message.latency_ping;

    seq_length = DDS_OctetSeq_get_length((const DDS_OctetSeq*)&_sample.bin_data);

    if (seq_length != message.size)
    {
        _sample.bin_data.clear();
        DDS_OctetSeq_set_length((DDS_OctetSeq*)&_sample.bin_data, message.size);
    }

    memcpy(_sample.bin_data.buffer(), message.data, message.size);

    _typedTS->Send_Message(_connection_id,
                           timeout,
                           transaction_id, /* not used */
                           _sample,
                           retcode);

    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Send_Message (rc=%d)\n", retcode);
        return false;
	}

    return true;
}

template <class Type, class TypedTS, class TypedCB>
inline bool TSSConnection<Type, TypedTS, TypedCB>::_send_loaning(
            const TestMessage &message,
            bool isCftWildCardKey)
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TIMEOUT_TYPE timeout(0);
    unsigned long key = MAX_CFT_VALUE;

    // Calculate key and add it if using more than one instance
    if (!isCftWildCardKey) {
        if (_num_instances > 1) {
            if (_instancesToBeWritten == -1) {
                key = _instance_counter++ % _num_instances;
            } else { // send sample to a specific subscriber
                key = _instancesToBeWritten;
            }
        }
    }

    for (int c = 0; c < KEY_SIZE; ++c) {
        _sample.key[c] = (unsigned char)(key >> c * 8);
    }

    _sample.entity_id = message.entity_id;
    _sample.seq_num = message.seq_num;
    _sample.timestamp_sec = message.timestamp_sec;
    _sample.timestamp_usec = message.timestamp_usec;
    _sample.latency_ping = message.latency_ping;

    DDS_OctetSeq_set_maximum((DDS_OctetSeq*)&_sample.bin_data, 0);

#ifdef RTI_PERF_TSS_PRO
    if (!DDS_OctetSeq_loan_contiguous((DDS_OctetSeq*)&_sample.bin_data,
                                      (DDS_Octet*) message.data,
                                      message.size, message.size)) {
        fprintf(stderr, "bin_data.loan_contiguous() failed.\n");
        return false;
    }
#else
    if (!CDR_OctetSeq_loan_contiguous((CDR_OctetSeq*)&_sample.bin_data,
                                       (DDS_Octet*) message.data,
                                       message.size, message.size)) {
        fprintf(stderr, "bin_data.loan_contiguous() failed.\n");
        return false;
    }
#endif // RTI_PERF_TSS_PRO

    _typedTS->Send_Message(_connection_id,
                           timeout,
                           transaction_id, /* not used */
                           _sample,
                           retcode);

#ifdef RTI_PERF_TSS_PRO
    if (!DDS_OctetSeq_unloan((DDS_OctetSeq*)&_sample.bin_data)) {
        fprintf(stderr, "bin_data.unloan() failed.\n");
        return false;
    }
#else
    if (!CDR_OctetSeq_unloan((CDR_OctetSeq*)&_sample.bin_data)) {
        fprintf(stderr, "bin_data.unloan() failed.\n");
        return false;
    }
#endif // RTI_PERF_TSS_PRO

    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
		fprintf(stderr, "Failed Send_Message (rc=%d)\n", retcode);
        return false;
	}

    return true;
}

template <class Type, class TypedTS, class TypedCB>
inline TestMessage* TSSConnection<Type, TypedTS, TypedCB>::_receive()
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TSS::QoS_EVENT_TYPE qos_parameters;
    FACE::TSS::HEADER_TYPE header;
    FACE::TIMEOUT_TYPE timeout(0);
    int seq_length;

    _typedTS->Receive_Message(_connection_id,
                              timeout,
                              transaction_id, /* not used */
                              _sample,
                              header, /* not used */
                              qos_parameters, /* not used */
                              retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
        if (retcode != FACE::RETURN_CODE_TYPE::NOT_AVAILABLE &&
                    retcode != FACE::RETURN_CODE_TYPE::TIMED_OUT)
            fprintf(stderr, "!Failed Receive_Message (rc=%d)\n", retcode);

        return NULL;
	}

    _message.entity_id = _sample.entity_id;
    _message.seq_num = _sample.seq_num;
    _message.timestamp_sec = _sample.timestamp_sec;
    _message.timestamp_usec = _sample.timestamp_usec;
    _message.latency_ping = _sample.latency_ping;

    seq_length = DDS_OctetSeq_get_length((const DDS_OctetSeq*)&_sample.bin_data);

    if (seq_length != _message.size)
    {
        free(_message.data);
        _message.data = (char*)malloc(seq_length);
    }

    memcpy(_message.data, _sample.bin_data.buffer(), seq_length);

    _message.size = seq_length;

    return &_message;
}

template <class Type, class TypedTS, class TypedCB>
inline TestMessage* TSSConnection<Type, TypedTS, TypedCB>::_receive_loaning()
{
    FACE::RETURN_CODE_TYPE::Value retcode;
    FACE::TSS::TRANSACTION_ID_TYPE transaction_id(0);
    FACE::TSS::QoS_EVENT_TYPE qos_parameters;
    FACE::TSS::HEADER_TYPE header;
    FACE::TIMEOUT_TYPE timeout(0);

    _typedTS->Receive_Message(_connection_id,
                              timeout,
                              transaction_id, /* not used */
                              _sample,
                              header, /* not used */
                              qos_parameters, /* not used */
                              retcode);
    if (retcode != FACE::RETURN_CODE_TYPE::NO_ERROR) {
        if (retcode != FACE::RETURN_CODE_TYPE::NOT_AVAILABLE &&
                    retcode != FACE::RETURN_CODE_TYPE::TIMED_OUT)
            fprintf(stderr, "!Failed Receive_Message (rc=%d)\n", retcode);

        return NULL;
	}

    _message.entity_id = _sample.entity_id;
    _message.seq_num = _sample.seq_num;
    _message.timestamp_sec = _sample.timestamp_sec;
    _message.timestamp_usec = _sample.timestamp_usec;
    _message.latency_ping = _sample.latency_ping;
    _message.size = DDS_OctetSeq_get_length((const DDS_OctetSeq*)&_sample.bin_data);
    _message.data = (char *) DDS_OctetSeq_get_contiguous_buffer((const DDS_OctetSeq*)&_sample.bin_data);

    return &_message;
}


template <class Type, class TypedCB>
void TSSListener<Type, TypedCB>::Callback_Handler(
        FACE::TSS::CONNECTION_ID_TYPE connection_id,
        FACE::TSS::TRANSACTION_ID_TYPE transaction_id,
        const Type& sample,
        const FACE::TSS::HEADER_TYPE& header,
        const FACE::TSS::QoS_EVENT_TYPE& qos_parameters,
        FACE::RETURN_CODE_TYPE::Value& return_code)
{
    UNUSED_ARG(connection_id);
    UNUSED_ARG(transaction_id);
    UNUSED_ARG(header);
    UNUSED_ARG(qos_parameters);
    UNUSED_ARG(return_code);

    int seq_length = 0;

    _message.entity_id = sample.entity_id;
    _message.seq_num = sample.seq_num;
    _message.timestamp_sec = sample.timestamp_sec;
    _message.timestamp_usec = sample.timestamp_usec;
    _message.latency_ping = sample.latency_ping;
    seq_length = DDS_OctetSeq_get_length((const DDS_OctetSeq*)&sample.bin_data);

    if (seq_length != _message.size)
    {
        free(_message.data);
        _message.data = (char*)malloc(seq_length);
    }

    memcpy(_message.data, sample.bin_data.buffer(), seq_length);

    _message.size = seq_length;

    _callback->process_message(_message);
}

template <class Type, class TypedCB>
void TSSListenerLoaning<Type, TypedCB>::Callback_Handler(
        FACE::TSS::CONNECTION_ID_TYPE connection_id,
        FACE::TSS::TRANSACTION_ID_TYPE transaction_id,
        const Type& sample,
        const FACE::TSS::HEADER_TYPE& header,
        const FACE::TSS::QoS_EVENT_TYPE& qos_parameters,
        FACE::RETURN_CODE_TYPE::Value& return_code)
{
    UNUSED_ARG(connection_id);
    UNUSED_ARG(transaction_id);
    UNUSED_ARG(header);
    UNUSED_ARG(qos_parameters);
    UNUSED_ARG(return_code);

    this->_message.entity_id = sample.entity_id;
    this->_message.seq_num = sample.seq_num;
    this->_message.timestamp_sec = sample.timestamp_sec;
    this->_message.timestamp_usec = sample.timestamp_usec;
    this->_message.latency_ping = sample.latency_ping;
    this->_message.size = DDS_OctetSeq_get_length((const DDS_OctetSeq*)&sample.bin_data);
    this->_message.data = (char *) DDS_OctetSeq_get_contiguous_buffer((const DDS_OctetSeq*)&sample.bin_data);

    this->_callback->process_message(this->_message);
}

template class RTITSSImpl<FACE::DM::TestData_t, TestData_t::TypedTS, TestData_t::Read_Callback>;
template class RTITSSImpl<FACE::DM::TestDataLarge_t, TestDataLarge_t::TypedTS, TestDataLarge_t::Read_Callback>;
template class RTITSSImpl<FACE::DM::TestDataKeyed_t, TestDataKeyed_t::TypedTS, TestDataKeyed_t::Read_Callback>;
template class RTITSSImpl<FACE::DM::TestDataKeyedLarge_t, TestDataKeyedLarge_t::TypedTS, TestDataKeyedLarge_t::Read_Callback>;