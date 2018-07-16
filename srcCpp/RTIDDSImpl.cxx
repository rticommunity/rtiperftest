/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "perftest.h"
#include "perftestPlugin.h"
#include "perftestSupport.h"
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "RTIDDSImpl.h"
#include "ndds/ndds_cpp.h"
#include "qos_string.h"

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


template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;
template class RTIDDSImpl<TestDataKeyedLarge_t>;
template class RTIDDSImpl<TestDataLarge_t>;

#ifdef RTI_SECURE_PERFTEST
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PRIVATEKEY_FILE_PUB =
        "./resource/secure/pubkey.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PRIVATEKEY_FILE_SUB =
        "./resource/secure/subkey.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_CERTIFICATE_FILE_PUB =
        "./resource/secure/pub.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_CERTIFICATE_FILE_SUB =
        "./resource/secure/sub.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_CERTAUTHORITY_FILE =
        "./resource/secure/cacert.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PERMISION_FILE_PUB =
        "./resource/secure/signed_PerftestPermissionsPub.xml";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PERMISION_FILE_SUB =
        "./resource/secure/signed_PerftestPermissionsSub.xml";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_LIBRARY_NAME =
        "nddssecurity";
#endif

std::string valid_flow_controller[] = {"default", "1Gbps", "10Gbps"};

/* Perftest DynamicDataMembersId class */
DynamicDataMembersId::DynamicDataMembersId()
{
    membersId["key"] = 1;
    membersId["entity_id"] = 2;
    membersId["seq_num"] = 3;
    membersId["timestamp_sec"] = 4;
    membersId["timestamp_usec"] = 5;
    membersId["latency_ping"] = 6;
    membersId["bin_data"] = 7;
#ifdef RTI_CUSTOM_TYPE
    membersId["custom_type"] = 8;
    membersId["custom_type_size"] = 9;
#endif
}

DynamicDataMembersId::~DynamicDataMembersId()
{
    membersId.clear();
}

DynamicDataMembersId &DynamicDataMembersId::GetInstance()
{
    static DynamicDataMembersId instance;
    return instance;
}

int DynamicDataMembersId::at(std::string key)
{
   return membersId.at(key);
}

/*********************************************************
 * Shutdown
 */
template <typename T>
void RTIDDSImpl<T>::Shutdown()
{

    if (_participant != NULL) {
        perftest_cpp::MilliSleep(2000);

        if (_reader != NULL) {
            DDSDataReaderListener* reader_listener = _reader->get_listener();
            if (reader_listener != NULL) {
                delete(reader_listener);
            }
            _subscriber->delete_datareader(_reader);
        }

        DDSDomainParticipantListener* participant_listener = _participant->get_listener();
        if (participant_listener != NULL) {
            delete(participant_listener);
        }

        _participant->delete_contained_entities();
        DDSTheParticipantFactory->delete_participant(_participant);
    }
    if(_pongSemaphore != NULL) {
        RTIOsapiSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }
    // Unregister _loggerDevice
    if (NDDSConfigLogger::get_instance()->get_output_device() != NULL) {
        NDDSConfigLogger::finalize_instance();
    }
    DDSDomainParticipantFactory::finalize_instance();
}

/*********************************************************
 * Validate and manage the parameter
 */
template <typename T>
bool RTIDDSImpl<T>::validate_input()
{
    // Manage parameter -instance
    if (PM::GetInstance().is_set("instance")){
        _InstanceMaxCountReader =
                PM::GetInstance().get<unsigned long>("instances");
    }
    // Manage parameter -peers
    if (PM::GetInstance().get_vector<std::string>("peer").size()
            >= RTIPERFTEST_MAX_PEERS) {
        fprintf(stderr,"The maximun of 'initial_peers' is %d\n", RTIPERFTEST_MAX_PEERS);
        return false;
    }

    // Check if we need to enable Large Data. This works also for -scan
    if (PM::GetInstance().get<unsigned long long>("dataLen")
            > (unsigned long) (std::min)(
                    MAX_SYNCHRONOUS_SIZE,
                    MAX_BOUNDED_SEQ_SIZE)) {
        _isLargeData = true;
    } else { /* No Large Data */
        _isLargeData = false;
    }

    // Manage parameter -batchSize
    if (PM::GetInstance().get<long>("batchSize") > 0) {
        /* We will not use batching for a latency test */
        if (PM::GetInstance().get<bool>("latencyTest")) {
            if (PM::GetInstance().is_set("batchSize")) {
                fprintf(stderr, "Batching cannot be used in a Latency test.\n");
                return false;
            } else {
                // Disable Batching
                PM::GetInstance().set<long>("batchSize", 0);
            }
        }

        /* Check if using asynchronous */
        if (PM::GetInstance().get<bool>("asynchronous")) {
            if (PM::GetInstance().is_set("batchSize")) {
                fprintf(stderr,
                        "Batching cannot be used with asynchronous writing.\n");
                return false;
            } else {
                // Disable Batching
                PM::GetInstance().set<long>("batchSize", 0);
            }
        }

        /*
         * Large Data + batching cannot be set. But batching is enabled by default,
         * so in that case, we just disabled batching, else, the customer set it up,
         * so we explitly fail
         */
        if (_isLargeData) {
            if (PM::GetInstance().is_set("batchSize")) {
                fprintf(stderr, "Batching cannot be used with Large Data.\n");
                return false;
            } else {
                PM::GetInstance().set<long>("batchSize", -2);
            }
        } else if ((unsigned long)PM::GetInstance().get<long>("batchSize")
                < PM::GetInstance().get<unsigned long long>("dataLen") * 2) {
            /*
             * We don't want to use batching if the batch size is not large
             * enough to contain at least two samples (in this case we avoid the
             * checking at the middleware level).
             */
            if (PM::GetInstance().is_set("batchSize") ||
                    PM::GetInstance().is_set("scan")) {
                /*
                 * Batchsize disabled. A message will be print if _batchSize < 0
                 * in perftest_cpp::PrintConfiguration()
                 */
                PM::GetInstance().set<long>("batchSize", -1);
            }
            else {
                // Disable Batching
                PM::GetInstance().set<long>("batchSize", 0);
            }
        }
    }

    // Manage parameter -enableTurboMode
    if (PM::GetInstance().get<bool>("enableTurboMode")) {
        if (PM::GetInstance().get<bool>("asynchronous")) {
            fprintf(stderr, "Turbo Mode cannot be used with asynchronous writing.\n");
            return false;
        }
        if (_isLargeData) {
            fprintf(stderr, "Turbo Mode disabled, using large data.\n");
            PM::GetInstance().set<bool>("enableTurboMode",false);
        }
    }

    // Manage parameter -writeInstance
    if (PM::GetInstance().is_set("writeInstance")) {
        if (PM::GetInstance().get<long>("instances") <
                PM::GetInstance().get<long>("writeInstance")) {
            fprintf(stderr,
                    "Specified '-WriteInstance' (%ld) invalid: Bigger than the number of instances (%lu).\n",
                    PM::GetInstance().get<long>("writeInstance"),
                    PM::GetInstance().get<unsigned long>("instances"));
            return false;
        }
    }

    // Manage transport parameter
    if(!_transport.validate_input()) {
        fprintf(stderr, "Failure validation the transport options.\n");
        return false;
    };

    /*
     * Manage parameter -verbosity.
     *  Setting verbosity
     */
    if (PM::GetInstance().is_set("verbosity")) {
        switch (PM::GetInstance().get<int>("verbosity")) {
            case 0: NDDSConfigLogger::get_instance()->
                        set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                    fprintf(stderr, "Setting verbosity to SILENT\n");
                    break;
            case 1: NDDSConfigLogger::get_instance()->
                        set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                    fprintf(stderr, "Setting verbosity to ERROR\n");
                    break;
            case 2: NDDSConfigLogger::get_instance()->
                        set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                    fprintf(stderr, "Setting verbosity to WARNING\n");
                    break;
            case 3: NDDSConfigLogger::get_instance()->
                        set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                    fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                    break;
            default:
                fprintf(stderr, "Invalid value for the verbosity parameter\n");
                    return false;
        }
    }

    // Manage parameter -secureGovernanceFile
    if (PM::GetInstance().is_set("secureGovernanceFile")) {
        fprintf(stdout,
                "Warning -- authentication, encryption, signing arguments "
                "will be ignored, and the values specified by the Governance "
                "file will be used instead\n");
    }

    // Manage parameter -secureEncryptBoth
    if (PM::GetInstance().is_set("secureEncryptBoth")) {
        PM::GetInstance().set("secureEncryptData", true);
        PM::GetInstance().set("secureEncryptSM", true);
    }

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
template <typename T>
std::string RTIDDSImpl<T>::PrintConfiguration()
{

    std::ostringstream stringStream;

    // Domain ID
    stringStream << "\tDomain: "
                 << PM::GetInstance().get<int>("domain")
                 << "\n";

    // Dynamic Data
    stringStream << "\tDynamic Data: ";
    if (PM::GetInstance().get<bool>("dynamicData")) {
        stringStream << "Yes\n";
    } else {
        stringStream << "No\n";
    }

    // Dynamic Data
    if (PM::GetInstance().get<bool>("pub")) {
        stringStream << "\tAsynchronous Publishing: ";
        if (_isLargeData
                || PM::GetInstance().get<bool>("asynchronous")) {
            stringStream << "Yes\n";
            stringStream << "\tFlow Controller: "
                         << PM::GetInstance().get<std::string>("flowController")
                         << "\n";
        } else {
            stringStream << "No\n";
        }
    }

    // Turbo Mode / AutoThrottle
    if (PM::GetInstance().get<bool>("enableTurboMode")) {
        stringStream << "\tTurbo Mode: Enabled\n";
    }
    if (PM::GetInstance().get<bool>("enableAutoThrottle")) {
        stringStream << "\tAutoThrottle: Enabled\n";
    }

    // XML File
    stringStream << "\tXML File: ";
    if (PM::GetInstance().get<bool>("noXmlQos")) {
        stringStream << "Disabled\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>("qosFile")
                     << "\n";
    }

    stringStream << "\n" << _transport.printTransportConfigurationSummary();


    // set initial peers and not use multicast
    std::vector<std::string> _peerList =
            PM::GetInstance().get_vector<std::string>("peer");
    if (!_peerList.empty()) {
        stringStream << "\tInitial peers: ";
        for (size_t i = 0; i < _peerList.size(); ++i) {
            stringStream << _peerList[i];
            if (i == _peerList.size() - 1) {
                stringStream << "\n";
            } else {
                stringStream << ", ";
            }
        }
    }

   #ifdef RTI_SECURE_PERFTEST
   if (PM::GetInstance().group_is_use(SECURE)) {
        stringStream << "\n" << printSecureArgs();
   }
   #endif

    return stringStream.str();
}

/*********************************************************
 * DomainListener
 */
class DomainListener : public DDSDomainParticipantListener
{
    virtual void on_inconsistent_topic(
        DDSTopic *topic,
        const DDS_InconsistentTopicStatus& /*status*/)
    {
        fprintf(stderr,"Found inconsistent topic. Expecting %s of type %s.\n",
               topic->get_name(), topic->get_type_name());
        fflush(stderr);
    }

    virtual void on_offered_incompatible_qos(
        DDSDataWriter *writer,
        const DDS_OfferedIncompatibleQosStatus &status)
    {
        fprintf(stderr,"Found incompatible reader for writer %s QoS is %d.\n",
               writer->get_topic()->get_name(), status.last_policy_id);
        fflush(stderr);
    }

    virtual void on_requested_incompatible_qos(
        DDSDataReader *reader,
        const DDS_RequestedIncompatibleQosStatus &status)
    {
        fprintf(stderr,"Found incompatible writer for reader %s QoS is %d.\n",
               reader->get_topicdescription()->get_name(), status.last_policy_id);
        fflush(stderr);
    }
};

/*********************************************************
 * RTIPublisher
 */
template<typename T>
class RTIPublisher : public IMessagingWriter
{
  private:
    typename T::DataWriter *_writer;
    T data;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    DDS_InstanceHandle_t *_instance_handles;
    RTIOsapiSemaphore *_pongSemaphore;
    long _instancesToBeWritten;
  #ifdef RTI_CUSTOM_TYPE
    unsigned int _lastMessageSize;
    unsigned int _minCustomTypeSerializeSize;
  #endif
    bool _isReliable;

 public:
    RTIPublisher(DDSDataWriter *writer, unsigned long num_instances, RTIOsapiSemaphore * pongSemaphore, int instancesToBeWritten)
    {
      #ifdef RTI_CUSTOM_TYPE
        _lastMessageSize = 0;
        // Calculate _minCustomTypeSerializeSize
        if (!get_serialize_size_custom_type_data(_minCustomTypeSerializeSize)) {
            throw std::runtime_error("get_serialize_size_custom_type_data failed");
        }
        // Initialize data
        DDS_ReturnCode_t retcode = RTI_CUSTOM_TYPE::TypeSupport::initialize_data(
                &data.custom_type);
        if (retcode != DDS_RETCODE_OK) {
            RTI_CUSTOM_TYPE::TypeSupport::finalize_data(&data.custom_type);
            throw std::runtime_error("TypeSupport::initialize_data failed");
        }
        if (!initialize_custom_type_data(data.custom_type)) {
            RTI_CUSTOM_TYPE::TypeSupport::finalize_data(&data.custom_type);
            throw std::runtime_error("initialize_custom_type_data failed");
        }
      #endif
        _writer = T::DataWriter::narrow(writer);
        if (!data.bin_data.maximum(0)) {
            Shutdown();
            throw std::runtime_error("bin_data.maximum failed");
        }
        _num_instances = num_instances;
        _instance_counter = 0;
        _instance_handles = (DDS_InstanceHandle_t *) malloc(
                sizeof(DDS_InstanceHandle_t)*(_num_instances + 1)); // One extra for MAX_CFT_VALUE
        if (_instance_handles == NULL) {
            Shutdown();
            fprintf(stderr, "_instance_handles malloc failed\n");
            throw std::bad_alloc();
        }
        _pongSemaphore = pongSemaphore;
        _instancesToBeWritten = instancesToBeWritten;

        for (unsigned long i = 0; i < _num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                data.key[c] = (unsigned char) (i >> c * 8);
            }
          #ifdef RTI_CUSTOM_TYPE
            register_custom_type_data(data.custom_type, i);
          #endif
            _instance_handles[i] = _writer->register_instance(data);
        }
        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            data.key[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
        }
      #ifdef RTI_CUSTOM_TYPE
        register_custom_type_data(data.custom_type, MAX_CFT_VALUE);
      #endif
        _instance_handles[_num_instances] = _writer->register_instance(data);

        DDS_DataWriterQos qos;
        _writer->get_qos(qos);
        _isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    }

    ~RTIPublisher() {
        try {
            Shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIPublisher::~RTIPublisher(): %s.\n", ex.what());
        }
    }

    void Shutdown() {
        if (_writer->get_listener() != NULL) {
            delete(_writer->get_listener());
            _writer->set_listener(NULL);
        }
        if (_instance_handles != NULL) {
            free(_instance_handles);
            _instance_handles = NULL;
        }
      #ifdef RTI_CUSTOM_TYPE
        if (!finalize_custom_type_data(data.custom_type)) {
            throw std::runtime_error("finalize_custom_type_data failed");
        }
        RTI_CUSTOM_TYPE::TypeSupport::finalize_data(&data.custom_type);
      #endif
    }

    void Flush()
    {
        _writer->flush();
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        bool success = true;
        long key = 0;

        if (!isCftWildCardKey) {
            if (_num_instances > 1) {
                if (_instancesToBeWritten == -1) {
                    key = _instance_counter++ % _num_instances;
                } else { // send sample to a specific subscriber
                    key = _instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }
        for (int c = 0; c < KEY_SIZE; c++) {
            data.key[c] = (unsigned char)(key >> c * 8);
        }

        data.entity_id = message.entity_id;
        data.seq_num = message.seq_num;
        data.timestamp_sec = message.timestamp_sec;
        data.timestamp_usec = message.timestamp_usec;
        data.latency_ping = message.latency_ping;
      #ifdef RTI_CUSTOM_TYPE
        /**
         * Using custom type the size of the data is set in data.custom_type_size:
         *      If the message.size is a sentinel size value used to handle the test:
         *          data.custom_type_size = message.size
         *      Else:
         *          If the message.size is different from the last iteration:
         *              data.custom_type_size of the custom type (data.custom_type)
         *              is measured from the function serialize_data_to_cdr_buffer()
         *          Else:
         *              data.custom_type_size is the same as the last iteration
        */
        if (is_sentinel_size(message.size)) {
            data.custom_type_size = message.size;
        } else {
            if (!set_custom_type_data(
                    data.custom_type,
                    key,
                    message.size - _minCustomTypeSerializeSize)) {
                fprintf(stderr, "set_custom_type_data failed.\n");
                return false;
            }
            if ((unsigned int)message.size != _lastMessageSize) {
                success = get_serialize_size_custom_type_data(
                        (unsigned int &)data.custom_type_size);
                if (!success) {
                    return false;
                }
                _lastMessageSize = message.size;
            }
        }
      #else
        success = data.bin_data.loan_contiguous(
                (DDS_Octet*)message.data,
                message.size,
                message.size);
        if (!success) {
            fprintf(stderr, "bin_data.loan_contiguous() failed.\n");
            return false;
        }
      #endif
        if (!isCftWildCardKey) {
            retcode = _writer->write(data, _instance_handles[key]);
        } else { // send CFT_MAX sample
            retcode = _writer->write(data, _instance_handles[_num_instances]);
        }

      #ifndef RTI_CUSTOM_TYPE
        success = data.bin_data.unloan();
        if (!success) {
            fprintf(stderr, "bin_data.unloan() failed.\n");
            return false;
        }
      #endif

        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr,"Write error %d.\n", retcode);
            return false;
        }

        return true;
    }

    void WaitForReaders(int numSubscribers)
    {
        DDS_PublicationMatchedStatus status;

        while (true) {
            DDS_ReturnCode_t retcode = _writer->get_publication_matched_status(
                    status);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "WaitForReaders _writer->get_publication_matched_status "
                        "failed: %d.\n",
                        retcode);
            }
            if (status.current_count >= numSubscribers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }

    bool waitForPingResponse()
    {
        if(_pongSemaphore != NULL) {
            if(RTIOsapiSemaphore_take(_pongSemaphore, NULL)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout)
    {
        struct RTINtpTime blockDurationIn;
        RTINtpTime_packFromMillisec(blockDurationIn, 0, timeout);

        if(_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_take(_pongSemaphore, &blockDurationIn)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse()
    {
        if(_pongSemaphore != NULL) {
            if(RTIOsapiSemaphore_give(_pongSemaphore)
                    != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                fprintf(stderr,"Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount() {
        DDS_DataWriterProtocolStatus status;
        _writer->get_datawriter_protocol_status(status);
        return (unsigned int)status.pulled_sample_count;
    };

    void waitForAck(int sec, unsigned int nsec) {
        if (_isReliable) {
            DDS_Duration_t timeout = {sec, nsec};
            _writer->wait_for_acknowledgments(timeout);
        } else {
            perftest_cpp::MilliSleep(nsec / 1000000);
        }
    }

#ifdef RTI_CUSTOM_TYPE
  private:
    bool is_sentinel_size(int size) {
        return size == perftest_cpp::INITIALIZE_SIZE
                || size == perftest_cpp::FINISHED_SIZE
                || size == perftest_cpp::LENGTH_CHANGED_SIZE
                || size == 0;
    }

    bool get_serialize_size_custom_type_data(unsigned int &size) {
        DDS_ReturnCode_t retcode = RTI_CUSTOM_TYPE::TypeSupport::serialize_data_to_cdr_buffer(
                NULL,
                (unsigned int &)size,
                &data.custom_type);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "serialize_data_to_cdr_buffer failed: %d.\n", retcode);
            return false;
        }
        size -= RTI_CDR_ENCAPSULATION_HEADER_SIZE;
        return true;
    }
#endif
};

/* Dynamic Data equivalent function from RTIPublisher */
class RTIDynamicDataPublisher : public IMessagingWriter
{
  private:
    DDSDynamicDataWriter *_writer;
    DDS_DynamicData data;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    DDS_InstanceHandle_t *_instance_handles;
    RTIOsapiSemaphore *_pongSemaphore;
    long _instancesToBeWritten;
  #ifdef RTI_CUSTOM_TYPE
    unsigned int _minCustomTypeSerializeSize;
    unsigned int _customTypeSize;
  #endif
    int _lastMessageSize;
    bool _isReliable;

  public:
    RTIDynamicDataPublisher(
            DDSDataWriter *writer,
            unsigned long num_instances,
            RTIOsapiSemaphore *pongSemaphore,
            DDS_TypeCode *typeCode,
            int instancesToBeWritten) :
            data(typeCode, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT)

    {
        DDS_ReturnCode_t retcode;

      #ifdef RTI_CUSTOM_TYPE
        // Calculate size_alignment_type of DDS_DynamicData object
        if (!get_serialize_size_custom_type_data(_minCustomTypeSerializeSize)) {
            throw std::runtime_error("get_serialize_size_custom_type_data failed");
        }
        // Initialize data
        if (!initialize_custom_type_dynamic_data(data)) {
            // ~DDS_DynamicData() will be called and data will be free
            throw std::runtime_error("initialize_custom_type_dynamic_data failed");
        }
      #endif
        _writer = DDSDynamicDataWriter::narrow(writer);
        DDS_Octet key_octets[KEY_SIZE];

        _lastMessageSize = 0;
        _num_instances = num_instances;
        _instance_counter = 0;
        _instancesToBeWritten = instancesToBeWritten;
        _instance_handles = (DDS_InstanceHandle_t *) malloc(
                sizeof(DDS_InstanceHandle_t) * (num_instances + 1));
        if (_instance_handles == NULL) {
            Shutdown();
            fprintf(stderr, "_instance_handles malloc failed\n");
            throw std::bad_alloc();
        }
        _pongSemaphore = pongSemaphore;

        for (unsigned long i = 0; i < _num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                key_octets[c] = (unsigned char) (i >> c * 8);
            }
            retcode = data.set_octet_array(
                    "key",
                    DynamicDataMembersId::GetInstance().at("key"),
                    KEY_SIZE,
                    key_octets);
            if (retcode != DDS_RETCODE_OK) {
                Shutdown();
                char errorMessage[21 + 21]; // enough to hold all numbers
                sprintf(errorMessage, "set_octet_array(key) failed: %d", retcode);
                throw std::runtime_error(errorMessage);
            }
          #ifdef RTI_CUSTOM_TYPE
            register_custom_type_dynamic_data(data, i);
          #endif
            _instance_handles[i] = _writer->register_instance(data);
        }
        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
        }
        retcode = data.set_octet_array(
                    "key",
                    DynamicDataMembersId::GetInstance().at("key"),
                    KEY_SIZE,
                    key_octets);
        if (retcode != DDS_RETCODE_OK) {
            Shutdown();
            char errorMessage[21 + 21]; // enough to hold all numbers
            sprintf(errorMessage, "set_octet_array(key) failed: %d", retcode);
            throw std::runtime_error(errorMessage);
        }
      #ifdef RTI_CUSTOM_TYPE
        register_custom_type_dynamic_data(data, MAX_CFT_VALUE);
      #endif
        _instance_handles[_num_instances] = _writer->register_instance(data);

        DDS_DataWriterQos qos;
        _writer->get_qos(qos);
        _isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    }

    ~RTIDynamicDataPublisher() {
        try {
            Shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIDynamicDataPublisher::~RTIDynamicDataPublisher(): %s.\n", ex.what());
        }
    }

    void Shutdown() {
        if (_writer->get_listener() != NULL) {
            delete(_writer->get_listener());
            _writer->set_listener(NULL);
        }
        if (_instance_handles != NULL) {
            free(_instance_handles);
            _instance_handles = NULL;
        }
      #ifdef RTI_CUSTOM_TYPE
        if (!finalize_custom_type_dynamic_data(data)) {
            throw std::runtime_error("finalize_custom_type_dynamic_data failed");
        }
      #endif
    }

    void Flush()
    {
        _writer->flush();
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        DDS_Octet key_octets[KEY_SIZE];
        long key = 0;
        if (!isCftWildCardKey) {
            if (_num_instances > 1) {
                if (_instancesToBeWritten == -1) {
                    key = _instance_counter++ % _num_instances;
                } else { // send sample to a specific subscriber
                    key = _instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }
        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (unsigned char) (key >> c * 8);
        }

        if (_lastMessageSize != message.size) {
            //Cannot use data.clear_member("bind_data") because:
            //DDS_DynamicData_clear_member:unsupported for non-sparse types
            data.clear_all_members();
        }
        retcode = data.set_octet_array(
                "key",
                DynamicDataMembersId::GetInstance().at("key"),
                KEY_SIZE,
                key_octets);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_octet_array(key) failed: %d.\n", retcode);
        }
        retcode = data.set_long(
                "entity_id",
                DynamicDataMembersId::GetInstance().at("entity_id"),
                message.entity_id);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(entity_id) failed: %d.\n", retcode);
        }
        retcode = data.set_ulong(
                "seq_num",
                DynamicDataMembersId::GetInstance().at("seq_num"),
                message.seq_num);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_ulong(seq_num) failed: %d.\n", retcode);
        }
        retcode = data.set_long(
                "timestamp_sec",
                DynamicDataMembersId::GetInstance().at("timestamp_sec"),
                message.timestamp_sec);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(timestamp_sec) failed: %d.\n", retcode);
        }
        retcode = data.set_ulong(
                "timestamp_usec",
                DynamicDataMembersId::GetInstance().at("timestamp_usec"),
                message.timestamp_usec);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_ulong(timestamp_usec) failed: %d.\n", retcode);
        }
        retcode = data.set_long(
                "latency_ping",
                DynamicDataMembersId::GetInstance().at("latency_ping"),
                message.latency_ping);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(latency_ping) failed: %d.\n", retcode);
        }
     #ifndef RTI_CUSTOM_TYPE
        if (_lastMessageSize != message.size) {
            DDS_OctetSeq octetSeq;
            bool succeeded = octetSeq.from_array(
                    (DDS_Octet *) message.data,
                    message.size);
            if (!succeeded) {
                fprintf(stderr, "from_array() failed.\n");
            }
            retcode = data.set_octet_seq(
                "bin_data",
                DynamicDataMembersId::GetInstance().at("bin_data"),
                octetSeq);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_octet_seq(bin_data) failed: %d.\n", retcode);
            }
        }
      #else
        /**
         * Using custom type the size of the data is set in data.custom_type_size:
         *    If the message.size is a sentinel size value used to handle the test:
         *          data.custom_type_size = message.size
         *      Else:
         *          If the message.size is different from the last iteration:
         *              data.custom_type_size of the custom type (data.custom_type)
         *              is measured from the function serialize_data_to_cdr_buffer()
         *          Else:
         *              data.custom_type_size is the same as the last iteration
        */
        if (is_sentinel_size(message.size)) {
            retcode = data.set_long(
                    "custom_type_size",
                    DynamicDataMembersId::GetInstance().at("custom_type_size"),
                    message.size);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(custom_type_size) failed: %d.\n", retcode);
                return false;
            }
        } else {
            if (!set_custom_type_dynamic_data(
                    data,
                    key,
                    message.size - _minCustomTypeSerializeSize)) {
                fprintf(stderr, "set_custom_type_dynamic_data failed.\n");
                return false;
            }
            if (message.size != _lastMessageSize) {
                if (!get_serialize_size_custom_type_data(_customTypeSize)) {
                    fprintf(stderr, "get_serialize_size_custom_type_data.\n");
                }
            }
            retcode = data.set_long(
                    "custom_type_size",
                    DynamicDataMembersId::GetInstance().at("custom_type_size"),
                    _customTypeSize);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(custom_type_size) failed: %d.\n", retcode);
                return false;
            }
        }
      #endif
        _lastMessageSize = message.size;

        if (!isCftWildCardKey) {
            retcode = _writer->write(data, _instance_handles[key]);
        } else {
            retcode = _writer->write(data, _instance_handles[_num_instances]);
        }

        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Write error %d.\n", retcode);
            return false;
        }
        return true;
    }

    void WaitForReaders(int numSubscribers)
    {
        DDS_PublicationMatchedStatus status;

        while (true) {
            DDS_ReturnCode_t retcode = _writer->get_publication_matched_status(
                    status);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "WaitForReaders _writer->get_publication_matched_status "
                        "failed: %d.\n",
                        retcode);
            }
            if (status.current_count >= numSubscribers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }

    bool waitForPingResponse() {
        if (_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_take(_pongSemaphore, NULL)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout) {
        struct RTINtpTime blockDuration;
        RTINtpTime_packFromMillisec(blockDuration, 0, timeout);

        if (_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_take(_pongSemaphore, &blockDuration)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse() {
        if (_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_give(_pongSemaphore)
                    != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount() {
        DDS_DataWriterProtocolStatus status;
        _writer->get_datawriter_protocol_status(status);
        return (unsigned int)status.pulled_sample_count;
    };

    void waitForAck(int sec, unsigned int nsec) {
        if (_isReliable) {
            DDS_Duration_t timeout = {sec, nsec};
            _writer->wait_for_acknowledgments(timeout);
        } else {
            perftest_cpp::MilliSleep(nsec / 1000000);
        }
    }
#ifdef RTI_CUSTOM_TYPE
  private:
    bool is_sentinel_size(int size) {
        return size == perftest_cpp::INITIALIZE_SIZE
                || size == perftest_cpp::FINISHED_SIZE
                || size == perftest_cpp::LENGTH_CHANGED_SIZE
                || size == 0;
    }

    bool get_serialize_size_custom_type_data(unsigned int &size) {
        char *buffer = NULL;
        DDS_ReturnCode_t retcode = data.to_cdr_buffer(
                NULL,
                (unsigned int &)size);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "to_cdr_buffer failed: %d.\n", retcode);
            return false;
        }
        RTIOsapiHeap_allocateBufferAligned(
                &buffer,
                size,
                RTIOsapiAlignment_getAlignmentOf(char *));
        if (buffer == NULL) {
            fprintf(stderr, "RTIOsapiHeap_allocateBufferAligned failed.\n");
            return false;
        }
        retcode = data.to_cdr_buffer(
                buffer,
                (unsigned int &)size);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "to_cdr_buffer failed: %d.\n", retcode);
            return false;
        }
        if (buffer != NULL) {
            RTIOsapiHeap_freeBufferAligned(buffer);
            buffer = NULL;
        }
        size -= RTI_CDR_ENCAPSULATION_HEADER_SIZE;
        size -= perftest_cpp::OVERHEAD_BYTES;
        return true;
    }
#endif
};

/*********************************************************
 * ReceiverListener
 */
template <typename T>
class ReceiverListener : public DDSDataReaderListener
{
  private:

    typename T::Seq     _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage       _message;
    IMessagingCB     *_callback;

  public:

    ReceiverListener(IMessagingCB *callback): _message()
    {
        _callback = callback;
    }

    void on_data_available(DDSDataReader *reader)
    {

        typename T::DataReader *datareader;

        datareader = T::DataReader::narrow(reader);
        if (datareader == NULL)
        {
            fprintf(stderr,"DataReader narrow error.\n");
            return;
        }

        DDS_ReturnCode_t retcode = datareader->take(
                _data_seq, _info_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE);

        if (retcode == DDS_RETCODE_NO_DATA)
        {
            fprintf(stderr,"called back no data\n");
            return;
        }
        else if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Error during taking data %d\n", retcode);
            return;
        }

        int seq_length = _data_seq.length();
        for (int i = 0; i < seq_length; ++i)
        {
            if (_info_seq[i].valid_data)
            {
                _message.entity_id = _data_seq[i].entity_id;
                _message.seq_num = _data_seq[i].seq_num;
                _message.timestamp_sec = _data_seq[i].timestamp_sec;
                _message.timestamp_usec = _data_seq[i].timestamp_usec;
                _message.latency_ping = _data_seq[i].latency_ping;
              #ifdef RTI_CUSTOM_TYPE
                _message.size = _data_seq[i].custom_type_size;
              #else
                _message.size = _data_seq[i].bin_data.length();
              #endif
                _message.data = (char *)_data_seq[i].bin_data.get_contiguous_bufferI();

                _callback->ProcessMessage(_message);

            }
        }

        retcode = datareader->return_loan(_data_seq, _info_seq);
        if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Error during return loan %d.\n", retcode);
            fflush(stderr);
        }

    }

};

/* Dynamic Data equivalent function from ReceiverListener */
class DynamicDataReceiverListener : public DDSDataReaderListener
{
  private:
    DDS_DynamicDataSeq _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage _message;
    IMessagingCB *_callback;

  public:
    DynamicDataReceiverListener(IMessagingCB *callback): _message()
    {
        _callback = callback;
    }

    void on_data_available(DDSDataReader *reader)
    {
        DDSDynamicDataReader *datareader = DDSDynamicDataReader::narrow(reader);
        if (datareader == NULL) {
            fprintf(stderr, "DataReader narrow error.\n");
            return;
        }

        DDS_ReturnCode_t retcode = datareader->take(
                _data_seq,
                _info_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE);

        if (retcode == DDS_RETCODE_NO_DATA) {
            fprintf(stderr, "No data received\n");
            return;
        } else if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Error while taking data %d\n", retcode);
            return;
        }

        DDS_OctetSeq octetSeq;
        for (int i = 0; i < _data_seq.length(); ++i) {
            if (_info_seq[i].valid_data) {
                retcode = _data_seq[i].get_long(
                        _message.entity_id,
                        "entity_id",
                        DynamicDataMembersId::GetInstance().at("entity_id"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(entity_id) failed: %d.\n",
                            retcode);
                    _message.entity_id = 0;
                }
                retcode = _data_seq[i].get_ulong(
                        _message.seq_num,
                        "seq_num",
                        DynamicDataMembersId::GetInstance().at("seq_num"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_ulong(seq_num) failed: %d.\n",
                            retcode);
                    _message.seq_num = 0;
                }
                retcode = _data_seq[i].get_long(
                        _message.timestamp_sec,
                        "timestamp_sec",
                        DynamicDataMembersId::GetInstance().at("timestamp_sec"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(timestamp_sec) failed: %d.\n",
                            retcode);
                    _message.timestamp_sec = 0;
                }
                retcode = _data_seq[i].get_ulong(
                        _message.timestamp_usec,
                        "timestamp_usec",
                        DynamicDataMembersId::GetInstance().at("timestamp_usec"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_ulong(timestamp_usec) failed: %d.\n",
                            retcode);
                    _message.timestamp_usec = 0;
                }
                retcode = _data_seq[i].get_long(
                        _message.latency_ping,
                        "latency_ping",
                        DynamicDataMembersId::GetInstance().at("latency_ping"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(latency_ping) failed: %d.\n",
                            retcode);
                    _message.latency_ping = 0;
                }
              #ifdef RTI_CUSTOM_TYPE
                retcode = _data_seq[i].get_long(
                        _message.size,
                        "custom_type_size",
                        DynamicDataMembersId::GetInstance().at("custom_type_size"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(size) failed: %d.\n",
                            retcode);
                    _message.size = 0;
                }
              #else
                retcode = _data_seq[i].get_octet_seq(
                        octetSeq,
                        "bin_data",
                        DynamicDataMembersId::GetInstance().at("bin_data"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_octet_seq(bin_data) failed: %d.\n",
                            retcode);
                }
                _message.size = octetSeq.length();
                _message.data = (char *)octetSeq.get_contiguous_buffer();
              #endif

                _callback->ProcessMessage(_message);
            }
        }

        retcode = datareader->return_loan(_data_seq, _info_seq);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Error during return loan %d.\n", retcode);
            fflush(stderr);
        }
    }
};

/*********************************************************
 * RTISubscriber
 */
template <typename T>
class RTISubscriber : public IMessagingReader
{
  private:
    typename T::DataReader *_reader;
    typename T::Seq         _data_seq;
    DDS_SampleInfoSeq     _info_seq;
    TestMessage           _message;
    DDSWaitSet           *_waitset;
    DDSConditionSeq       _active_conditions;

    int      _data_idx;
    bool      _no_data;

  public:

    RTISubscriber(DDSDataReader *reader): _message()
    {
        _reader = T::DataReader::narrow(reader);
        _data_idx = 0;
        _no_data = false;

        // null listener means using receive thread
        if (_reader->get_listener() == NULL) {
            DDS_WaitSetProperty_t property;
            property.max_event_count =
                    PM::GetInstance().get<unsigned long long>("waitsetEventCount");
            property.max_event_delay.sec =
                    PM::GetInstance().get<unsigned int>("waitsetDelayUsec") / 1000000;
            property.max_event_delay.nanosec =
                    (PM::GetInstance().get<unsigned int>("waitsetDelayUsec") % 1000000) * 1000;

            _waitset = new DDSWaitSet(property);

            DDSStatusCondition *reader_status;
            reader_status = reader->get_statuscondition();
            reader_status->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
            _waitset->attach_condition(reader_status);
        }
    }

    ~RTISubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_reader->get_listener() != NULL) {
            delete(_reader->get_listener());
            _reader->set_listener(NULL);
        }
        // loan may be outstanding during shutdown
        _reader->return_loan(_data_seq, _info_seq);
    }

    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        int seq_length;

        while (true) {

            // no outstanding reads
            if (_no_data)
            {
                _waitset->wait(_active_conditions, DDS_DURATION_INFINITE);

                if (_active_conditions.length() == 0)
                {
                    //printf("Read thread woke up but no data\n.");
                    //return NULL;
                    continue;
                }

                retcode = _reader->take(
                    _data_seq, _info_seq,
                    DDS_LENGTH_UNLIMITED,
                    DDS_ANY_SAMPLE_STATE,
                    DDS_ANY_VIEW_STATE,
                    DDS_ANY_INSTANCE_STATE);
                if (retcode == DDS_RETCODE_NO_DATA)
                {
                    //printf("Called back no data.\n");
                    //return NULL;
                    continue;
                }
                else if (retcode != DDS_RETCODE_OK)
                {
                    fprintf(stderr,"Error during taking data %d.\n", retcode);
                    return NULL;
                }

                _data_idx = 0;
                _no_data = false;
            }

            seq_length = _data_seq.length();
            // check to see if hit end condition
            if (_data_idx == seq_length)
            {
                _reader->return_loan(_data_seq, _info_seq);
                _no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (_info_seq[_data_idx].valid_data == false)
                        && (++_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (_data_idx == seq_length) {
                continue;
            }

            _message.entity_id = _data_seq[_data_idx].entity_id;
            _message.seq_num = _data_seq[_data_idx].seq_num;
            _message.timestamp_sec = _data_seq[_data_idx].timestamp_sec;
            _message.timestamp_usec = _data_seq[_data_idx].timestamp_usec;
            _message.latency_ping = _data_seq[_data_idx].latency_ping;
          #ifdef RTI_CUSTOM_TYPE
            _message.size = _data_seq[_data_idx].custom_type_size;
          #else
            _message.size = _data_seq[_data_idx].bin_data.length();
          #endif
            _message.data = (char *)_data_seq[_data_idx].bin_data.get_contiguous_bufferI();

            ++_data_idx;

            return &_message;
        }
    }

    void WaitForWriters(int numPublishers)
    {
        DDS_SubscriptionMatchedStatus status;

        while (true) {
            _reader->get_subscription_matched_status(status);
            if (status.current_count >= numPublishers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }
};

/* Dynamic Data equivalent function from RTISubscriber */
template <typename T>
class RTIDynamicDataSubscriber : public IMessagingReader
{
  private:
    DDSDynamicDataReader *_reader;
    DDS_DynamicDataSeq _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage _message;
    DDSWaitSet *_waitset;
    DDSConditionSeq _active_conditions;

    int _data_idx;
    bool _no_data;

  public:

    RTIDynamicDataSubscriber(DDSDataReader *reader): _message()
    {
        _reader = DDSDynamicDataReader::narrow(reader);
        if (_reader == NULL) {
            fprintf(stderr,"DDSDynamicDataReader::narrow(reader) error.\n");
        }
        _data_idx = 0;
        _no_data = false;

        // null listener means using receive thread
        if (_reader->get_listener() == NULL) {

            DDS_WaitSetProperty_t property;
            property.max_event_count =
                    PM::GetInstance().get<int>("waitsetEventCount");
            property.max_event_delay.sec =
                    PM::GetInstance().get<unsigned int>("waitsetDelayUsec") / 1000000;
            property.max_event_delay.nanosec =
                    (PM::GetInstance().get<unsigned int>("waitsetDelayUsec") % 1000000) * 1000;
            _waitset = new DDSWaitSet(property);
            DDSStatusCondition *reader_status;
            reader_status = reader->get_statuscondition();
            reader_status->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
            _waitset->attach_condition(reader_status);
        }
    }

    ~RTIDynamicDataSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_reader->get_listener() != NULL) {
            delete(_reader->get_listener());
            _reader->set_listener(NULL);
        }
        // loan may be outstanding during shutdown
        _reader->return_loan(_data_seq, _info_seq);
    }

    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        int seq_length;
        DDS_OctetSeq octetSeq;

        while (true) {
            // no outstanding reads
            if (_no_data)
            {
                _waitset->wait(_active_conditions, DDS_DURATION_INFINITE);

                if (_active_conditions.length() == 0)
                {
                    continue;
                }

                retcode = _reader->take(
                        _data_seq,
                        _info_seq,
                        DDS_LENGTH_UNLIMITED,
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
                if (retcode == DDS_RETCODE_NO_DATA)
                {
                    //printf("Called back no data.\n");
                    //return NULL;
                    continue;
                }
                else if (retcode != DDS_RETCODE_OK)
                {
                    fprintf(stderr,"Error during taking data %d.\n", retcode);
                    return NULL;
                }

                _data_idx = 0;
                _no_data = false;
            }

            seq_length = _data_seq.length();
            // check to see if hit end condition
            if (_data_idx == seq_length)
            {
                _reader->return_loan(_data_seq, _info_seq);
                _no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (_info_seq[_data_idx].valid_data == false) &&
                    (++_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (_data_idx == seq_length) {
                continue;
            }

            retcode = _data_seq[_data_idx].get_long(
                _message.entity_id,
                "entity_id",
                DynamicDataMembersId::GetInstance().at("entity_id"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(entity_id) failed: %d.\n",
                        retcode);
                _message.entity_id = 0;
            }
            retcode = _data_seq[_data_idx].get_ulong(
                _message.seq_num,
                "seq_num",
                DynamicDataMembersId::GetInstance().at("seq_num"));
            if (retcode != DDS_RETCODE_OK){
                fprintf(stderr,
                        "ReceiveMessage() get_ulong(seq_num) failed: %d.\n",
                        retcode);
                _message.seq_num = 0;
            }
            retcode = _data_seq[_data_idx].get_long(
                _message.timestamp_sec,
                "timestamp_sec",
                DynamicDataMembersId::GetInstance().at("timestamp_sec"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(timestamp_sec) failed: %d.\n",
                        retcode);
                _message.timestamp_sec = 0;
            }
            retcode = _data_seq[_data_idx].get_ulong(
                _message.timestamp_usec,
                "timestamp_usec",
                DynamicDataMembersId::GetInstance().at("timestamp_usec"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_ulong(timestamp_usec) failed: %d.\n",
                        retcode);
                _message.timestamp_usec = 0;
            }
            retcode = _data_seq[_data_idx].get_long(
                _message.latency_ping,
                "latency_ping",
                DynamicDataMembersId::GetInstance().at("latency_ping"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(latency_ping) failed: %d.\n",
                        retcode);
                _message.latency_ping = 0;
            }
            #ifdef RTI_CUSTOM_TYPE
            retcode = _data_seq[_data_idx].get_long(
                    _message.size,
                    "custom_type_size",
                    DynamicDataMembersId::GetInstance().at("custom_type_size"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "on_data_available() get_long(size) failed: %d.\n",
                        retcode);
                _message.size = 0;
            }
          #else
            retcode = _data_seq[_data_idx].get_octet_seq(
                    octetSeq,
                    "bin_data",
                    DynamicDataMembersId::GetInstance().at("bin_data"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_octet_seq(bin_data) failed: %d.\n",
                        retcode);
            }
            _message.size = octetSeq.length();
            _message.data = (char *)octetSeq.get_contiguous_buffer();
          #endif

            ++_data_idx;

            return &_message;
        }
    }

    void WaitForWriters(int numPublishers)
    {
        DDS_SubscriptionMatchedStatus status;

        while (true) {
            _reader->get_subscription_matched_status(status);
            if (status.current_count >= numPublishers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }
};

/*******************************************************************************
 * SECURITY PLUGIN
 */
#ifdef RTI_SECURE_PERFTEST

template<typename T>
bool RTIDDSImpl<T>::configureSecurePlugin(DDS_DomainParticipantQos& dpQos) {
    // configure use of security plugins, based on provided arguments

    DDS_ReturnCode_t retcode;
    std::string governanceFilePath;

    // load plugin
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.load_plugin",
            "com.rti.serv.secure",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.load_plugin\n");
        return false;
    }

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    retcode = DDSPropertyQosPolicyHelper::assert_property(
            dpQos.property,
            "com.rti.serv.secure.create_function",
            "RTI_Security_PluginSuite_create",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.create_function\n");
        return false;
    }


    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.library",
            PM::GetInstance().get<std::string>("secureLibrary").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.library\n");
        return false;
    }

  #else // Static library linking

    retcode = DDSPropertyQosPolicyHelper::assert_pointer_property(
            dpQos.property,
            "com.rti.serv.secure.create_function_ptr",
            (void *) RTI_Security_PluginSuite_create);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add pointer_property "
                "com.rti.serv.secure.create_function_ptr\n");
        return false;
    }

  #endif

    /*
     * Below, we are using com.rti.serv.secure properties in order to be
     * backward compatible with RTI Connext DDS 5.3.0 and below. Later versions
     * use the properties that are specified in the DDS Security specification
     * (see also the RTI Security Plugins Getting Started Guide). However,
     * later versions still support the legacy properties as an alternative.
     */

    // check if governance file provided
    if (PM::GetInstance().get<std::string>("secureGovernanceFile").empty()) {
        // choose a pre-built governance file
        governanceFilePath = "./resource/secure/signed_PerftestGovernance_";
        if (PM::GetInstance().get<bool>("secureEncryptDiscovery")) {
            governanceFilePath += "Discovery";
        }

        if (PM::GetInstance().get<bool>("secureSign")) {
            governanceFilePath += "Sign";
        }

        if (PM::GetInstance().get<bool>("secureEncryptData")
                && PM::GetInstance().get<bool>("secureEncryptSM")) {
            governanceFilePath += "EncryptBoth";
        } else if (PM::GetInstance().get<bool>("secureEncryptData")) {
            governanceFilePath += "EncryptData";
        } else if (PM::GetInstance().get<bool>("secureEncryptSM")) {
            governanceFilePath += "EncryptSubmessage";
        }

        governanceFilePath += ".xml";

        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                governanceFilePath.c_str(),
                false);
    } else {
        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                governanceFilePath.c_str(),
                false);
    }
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.governance_file\n");
        return false;
    }

    PM::GetInstance().set("secureGovernanceFile", governanceFilePath);

    // permissions file
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.access_control.permissions_file",
            PM::GetInstance().get<std::string>("securePermissionsFile").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.permissions_file\n");
        return false;
    }

    // permissions authority file
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.access_control.permissions_authority_file",
            PM::GetInstance().get<std::string>("secureCertAuthority").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.permissions_authority_file\n");
        return false;
    }

    // certificate authority
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.ca_file",
            PM::GetInstance().get<std::string>("secureCertAuthority").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.ca_file\n");
        return false;
    }

    // public key
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.certificate_file",
            PM::GetInstance().get<std::string>("secureCertFile").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.certificate_file\n");
        return false;
    }

    // private key
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.private_key_file",
            PM::GetInstance().get<std::string>("securePrivateKey").c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.private_key_file\n");
        return false;
    }

    if (PM::GetInstance().is_set("secureDebug")) {
        char buf[16];
        sprintf(buf, "%llu",
                PM::GetInstance().get<unsigned long long>("secureDebug"));
        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.logging.log_level",
                buf,
                false);
        if (retcode != DDS_RETCODE_OK) {
            printf("Failed to add property "
                    "com.rti.serv.secure.logging.log_level\n");
            return false;
        }
    }

    return true;
}

template <typename T>
bool RTIDDSImpl<T>::validateSecureArgs()
{
    if (PM::GetInstance().group_is_use(SECURE)) {
        if (PM::GetInstance().get<std::string>("securePrivateKey").empty()) {
            if (PM::GetInstance().get<bool>("pub")) {
                PM::GetInstance().set(
                        "securePrivateKey",
                        SECURE_PRIVATEKEY_FILE_PUB);
            } else {
                PM::GetInstance().set(
                        "securePrivateKey",
                        SECURE_PRIVATEKEY_FILE_SUB);
            }
        }

        if (PM::GetInstance().get<std::string>("secureCertFile").empty()) {
            if (PM::GetInstance().get<bool>("pub")) {
                PM::GetInstance().set(
                        "secureCertFile",
                        SECURE_CERTIFICATE_FILE_PUB);
            } else {
                PM::GetInstance().set(
                        "secureCertFile",
                        SECURE_CERTIFICATE_FILE_SUB);
            }
        }

        if (PM::GetInstance().get<std::string>("secureCertAuthority").empty()) {
            PM::GetInstance().set(
                    "secureCertAuthority",
                    SECURE_CERTAUTHORITY_FILE);
        }

        if (PM::GetInstance().get<std::string>("securePermissionsFile").empty()) {
            if (PM::GetInstance().get<bool>("pub")) {
                PM::GetInstance().set(
                        "securePermissionsFile",
                        SECURE_PERMISION_FILE_PUB);
            } else {
                PM::GetInstance().set(
                        "securePermissionsFile",
                        SECURE_PERMISION_FILE_SUB);
            }
        }

      #ifdef RTI_PERFTEST_DYNAMIC_LINKING
        if (PM::GetInstance().get<std::string>("secureLibrary").empty()) {
            PM::GetInstance().set("secureLibrary", SECURE_LIBRARY_NAME);
        }
      #endif

    }

    return true;
}

template <typename T>
std::string RTIDDSImpl<T>::printSecureArgs()
{
    std::ostringstream stringStream;
    stringStream << "Secure Configuration:\n";

    stringStream << "\tEncrypt discovery: ";
    if (PM::GetInstance().get<bool>("secureEncryptDiscovery")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt topic (user) data: ";
    if (PM::GetInstance().get<bool>("secureEncryptData")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt submessage: ";
    if (PM::GetInstance().get<bool>("secureEncryptSM")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tSign data: ";
    if (PM::GetInstance().get<bool>("secureSign")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tGovernance file: ";
    if (PM::GetInstance().get<std::string>("secureGovernanceFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>(
                                "secureGovernanceFile")
                     << "\n";
    }

    stringStream << "\tPermissions file: ";
    if (PM::GetInstance().get<std::string>("securePermissionsFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>(
                                "securePermissionsFile")
                     << "\n";
    }

    stringStream << "\tPrivate key file: ";
    if (PM::GetInstance().get<std::string>("securePrivateKey").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>("securePrivateKey")
                     << "\n";
    }

    stringStream << "\tCertificate file: ";
    if (PM::GetInstance().get<std::string>("secureCertFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>("secureCertFile")
                     << "\n";
    }

    stringStream << "\tCertificate authority file: ";
    if (PM::GetInstance().get<std::string>("secureCertAuthority").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>(
                                "secureCertAuthority")
                     << "\n";
    }

    stringStream << "\tPlugin library: ";
    if (PM::GetInstance().get<std::string>("secureLibrary").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << PM::GetInstance().get<std::string>("secureLibrary")
                     << "\n";
    }

    if (PM::GetInstance().is_set("secureDebug")) {
        stringStream << "\tDebug level: "
                     << PM::GetInstance().get<unsigned long long>("secureDebug")
                     << "\n";
    }

    return stringStream.str();
}

#endif

/*********************************************************
 * Initialize
 */
template <typename T>
bool RTIDDSImpl<T>::Initialize(int argc, char *argv[])
{
    DDS_DomainParticipantQos qos;
    DDS_DomainParticipantFactoryQos factory_qos;
    DomainListener *listener = new DomainListener();

    // Register _loggerDevice
    if (!NDDSConfigLogger::get_instance()->set_output_device(&_loggerDevice)) {
        fprintf(stderr,"Failed set_output_device for Logger.\n");
        return false;
    }

    _factory = DDSDomainParticipantFactory::get_instance();

    if (!validate_input()) {
        return false;
    }

    // only if we run the latency test we need to wait
    // for pongs after sending pings
    _pongSemaphore = PM::GetInstance().get<bool>("latencyTest") ?
            RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL) :
            NULL;

    // setup the QOS profile file to be loaded
    _factory->get_qos(factory_qos);
    if (!PM::GetInstance().get<bool>("noXmlQos")) {
        factory_qos.profile.url_profile.ensure_length(1, 1);
        factory_qos.profile.url_profile[0] =
                DDS_String_dup(PM::GetInstance().get<std::string>("qosFile").c_str());
    } else {
        factory_qos.profile.string_profile.from_array(
                PERFTEST_QOS_STRING,
                PERFTEST_QOS_STRING_SIZE);
    }
    _factory->set_qos(factory_qos);

    if (_factory->reload_profiles() != DDS_RETCODE_OK)
    {
        fprintf(stderr,"Problem opening QOS profiles file %s.\n",
                PM::GetInstance().get<std::string>("qosFile").c_str());
        return false;
    }

    if (_factory->set_default_library(PM::GetInstance().get<std::string>("qosLibrary").c_str())
             != DDS_RETCODE_OK) {
        fprintf(stderr,"No QOS Library named \"%s\" found in %s.\n",
               PM::GetInstance().get<std::string>("qosLibrary").c_str(),
               PM::GetInstance().get<std::string>("qosFile").c_str());
        return false;
    }

    // Configure DDSDomainParticipant QOS
    if (_factory->get_participant_qos_from_profile(
            qos,
            PM::GetInstance().get<std::string>("qosLibrary").c_str(),
            "BaseProfileQos") != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem setting QoS Library \"%s::BaseProfileQos\" for participant_qos.\n",
                PM::GetInstance().get<std::string>("qosLibrary").c_str());
    }

  #ifdef RTI_SECURE_PERFTEST
    if (PM::GetInstance().group_is_use(SECURE)) {
        // validate arguments
        if (!validateSecureArgs()) {
            fprintf(stderr, "Failed to configure security plugins\n");
            return false;
        }
        // configure
        if (!configureSecurePlugin(qos)) {
            fprintf(stderr, "Failed to configure security plugins\n");
            return false;
        }
    }
  #endif

    // set initial peers and not use multicast
    std::vector<std::string> _peerList =
            PM::GetInstance().get_vector<std::string>("peer");
    if (!_peerList.empty()) {
        std::vector<char*> cstrings;
        cstrings.reserve(_peerList.size());
        for(size_t i = 0; i < _peerList.size(); ++i)
            cstrings.push_back(const_cast<char*>(_peerList[i].c_str()));
        qos.discovery.initial_peers.from_array(
            (const char **)&cstrings[0],
            _peerList.size());
        qos.discovery.multicast_receive_addresses.length(0);
    }

    if (!configureTransport(_transport, qos)){
        return false;
    };

    if (PM::GetInstance().get<bool>("enableAutoThrottle")) {
        DDSPropertyQosPolicyHelper::add_property(qos.property,
                "dds.domain_participant.auto_throttle.enable", "true", false);
    }

    // Creates the participant
    _participant = _factory->create_participant(
            PM::GetInstance().get<int>("domain"), qos, listener,
            DDS_INCONSISTENT_TOPIC_STATUS |
            DDS_OFFERED_INCOMPATIBLE_QOS_STATUS |
            DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);

    if (_participant == NULL || _loggerDevice.checkShmemErrors()) {
        if (_loggerDevice.checkShmemErrors()) {
            fprintf(
                    stderr,
                    "The participant creation failed due to issues in the Shared Memory configuration of your OS.\n"
                    "For more information about how to configure Shared Memory see: http://community.rti.com/kb/osx510 \n"
                    "If you want to skip the use of Shared memory in RTI Perftest, "
                    "specify the transport using \"-transport <kind>\", e.g. \"-transport UDPv4\".\n");
        }
        fprintf(stderr,"Problem creating participant.\n");
        return false;
    }

    // Register the types and create the topics
    if (!PM::GetInstance().get<bool>("dynamicData")) {
        T::TypeSupport::register_type(_participant, _typename);
    } else {
        DDSDynamicDataTypeSupport* dynamicDataTypeSupportObject =
                new DDSDynamicDataTypeSupport(
                        T::TypeSupport::get_typecode(),
                        DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
        dynamicDataTypeSupportObject->register_type(_participant, _typename);
    }

    // Create the DDSPublisher and DDSSubscriber
    _publisher = _participant->create_publisher_with_profile(
            PM::GetInstance().get<std::string>("qosLibrary").c_str(),
            "BaseProfileQos",
            NULL,
            DDS_STATUS_MASK_NONE);
    if (_publisher == NULL)
    {
        fprintf(stderr,"Problem creating publisher.\n");
        return false;
    }

    _subscriber = _participant->create_subscriber_with_profile(
            PM::GetInstance().get<std::string>("qosLibrary").c_str(),
            "BaseProfileQos",
            NULL,
            DDS_STATUS_MASK_NONE);
    if (_subscriber == NULL)
    {
        fprintf(stderr,"Problem creating subscriber.\n");
        return false;
    }

    return true;
}

/*********************************************************
 * GetInitializationSampleCount
 */
template <typename T>
unsigned long RTIDDSImpl<T>::GetInitializationSampleCount()
{
    /*
     * There is a minimum number of samples that we want to send no matter what
     * the conditions are:
     */
    unsigned long initializeSampleCount = 50;

    /*
     * If we are using reliable, the maximum burst of that we can send is limited
     * by max_send_window_size (or max samples, but we will assume this is not
     * the case for this). In such case we should send max_send_window_size
     * samples.
     *
     * If we are not using reliability this should not matter.
     */
    initializeSampleCount = (std::max)(
            initializeSampleCount,
            (unsigned long) PM::GetInstance().get<int>("sendQueueSize"));

    /*
     * If we are using batching we need to take into account tha the Send Queue
     * will be per-batch, therefore for the number of samples:
     */
    if (PM::GetInstance().get<long>("batchSize") > 0) {
        initializeSampleCount = (std::max)(
                PM::GetInstance().get<int>("sendQueueSize") *
                        (PM::GetInstance().get<long>("batchSize") /
                        PM::GetInstance().get<unsigned long>("dataLen")),
                initializeSampleCount);
    }

    return initializeSampleCount;
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *RTIDDSImpl<T>::CreateWriter(const char *topic_name)
{
    DDSDataWriter *writer = NULL;
    DDS_DataWriterQos dw_qos;
    std::string qos_profile;

    DDSTopic *topic = _participant->create_topic(
                       topic_name,
                       _typename,
                       DDS_TOPIC_QOS_DEFAULT,
                       NULL,
                       DDS_STATUS_MASK_NONE);

    if (topic == NULL) {
        fprintf(stderr,"Problem creating topic %s.\n", topic_name);
        return NULL;
    }

    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0) {
        qos_profile = "ThroughputQos";
    } else if (strcmp(topic_name, LATENCY_TOPIC_NAME) == 0) {
        qos_profile = "LatencyQos";
    } else if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) == 0) {
        qos_profile = "AnnouncementQos";
    } else {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME,
                LATENCY_TOPIC_NAME,
                ANNOUNCEMENT_TOPIC_NAME);
        return NULL;
    }

    if (_factory->get_datawriter_qos_from_profile(dw_qos,
            PM::GetInstance().get<std::string>("qosLibrary").c_str(),
            qos_profile.c_str())
        != DDS_RETCODE_OK) {
        fprintf(stderr,
                "No QOS Profile named \"%s\" found in QOS Library \"%s\" in file %s.\n",
                qos_profile.c_str(),
                PM::GetInstance().get<std::string>("qosLibrary").c_str(),
                PM::GetInstance().get<std::string>("qosFile").c_str());
        return NULL;
    }

    if (PM::GetInstance().get<bool>("noPositiveAcks")
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dw_qos.protocol.disable_positive_acks = true;
        if (PM::GetInstance().is_set("keepDurationUsec")) {
            dw_qos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration =
                DDS_Duration_t::from_micros(
                            PM::GetInstance().get<unsigned long long>("keepDurationUsec"));
        }
    }

    if (_isLargeData
            || PM::GetInstance().get<bool>("asynchronous")) {
        dw_qos.publish_mode.kind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
        if (PM::GetInstance().get<std::string>("flowController")
                != "default") {
            dw_qos.publish_mode.flow_controller_name =
                    DDS_String_dup(("dds.flow_controller.token_bucket." +
                    PM::GetInstance().get<std::string>("flowController")).c_str());
        }
    }

    // only force reliability on throughput/latency topics
    if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (!PM::GetInstance().get<bool>("bestEffort")) {
            // default: use the setting specified in the qos profile
            // dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }
        else {
            // override to best-effort
            dw_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

    // These QOS's are only set for the Throughput datawriter
    if (qos_profile == "ThroughputQos") {

        if (PM::GetInstance().get<bool>("multicast")) {
            dw_qos.protocol.rtps_reliable_writer.enable_multicast_periodic_heartbeat =
                    RTI_TRUE;
        }

        if (PM::GetInstance().get<long>("batchSize") > 0) {
            dw_qos.batch.enable = true;
            dw_qos.batch.max_data_bytes =
                    PM::GetInstance().get<long>("batchSize");
            dw_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos.writer_resource_limits.max_batches =
                    PM::GetInstance().get<int>("sendQueueSize");
        } else {
            dw_qos.resource_limits.max_samples =
                    PM::GetInstance().get<int>("sendQueueSize");
        }

        if (PM::GetInstance().get<bool>("enableAutoThrottle")) {
            DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
                    "dds.data_writer.auto_throttle.enable", "true", false);
        }

        if (PM::GetInstance().get<bool>("enableTurboMode")) {
            DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
                    "dds.data_writer.enable_turbo_mode", "true", false);
            dw_qos.batch.enable = false;
            dw_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos.writer_resource_limits.max_batches =
                    PM::GetInstance().get<int>("sendQueueSize");
        }

        dw_qos.resource_limits.initial_samples =
                PM::GetInstance().get<int>("sendQueueSize");
        dw_qos.resource_limits.max_samples_per_instance
            = dw_qos.resource_limits.max_samples;

        dw_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind)PM::GetInstance().get<int>("durability");
        dw_qos.durability.direct_communication =
                !PM::GetInstance().get<bool>("noDirectCommunication");

        dw_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                PM::GetInstance().get<int>("sendQueueSize") / 10;

        dw_qos.protocol.rtps_reliable_writer.low_watermark =
                PM::GetInstance().get<int>("sendQueueSize") * 1 / 10;
        dw_qos.protocol.rtps_reliable_writer.high_watermark =
                PM::GetInstance().get<int>("sendQueueSize") * 9 / 10;

        /*
         * If _SendQueueSize is 1 low watermark and high watermark would both be
         * 0, which would cause the middleware to fail. So instead we set the
         * high watermark to the low watermark + 1 in such case.
         */
        if (dw_qos.protocol.rtps_reliable_writer.high_watermark
            == dw_qos.protocol.rtps_reliable_writer.low_watermark) {
            dw_qos.protocol.rtps_reliable_writer.high_watermark =
                    dw_qos.protocol.rtps_reliable_writer.low_watermark + 1;
        }

        dw_qos.protocol.rtps_reliable_writer.max_send_window_size =
                PM::GetInstance().get<int>("sendQueueSize");
        dw_qos.protocol.rtps_reliable_writer.min_send_window_size =
                PM::GetInstance().get<int>("sendQueueSize");
    }

    if (qos_profile == "LatencyQos"
            && PM::GetInstance().get<bool>("noDirectCommunication")
            && (PM::GetInstance().get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
                    || PM::GetInstance().get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS)) {
        dw_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind)PM::GetInstance().get<int>("durability");
        dw_qos.durability.direct_communication =
                !PM::GetInstance().get<bool>("noDirectCommunication");
    }

    dw_qos.resource_limits.max_instances =
            PM::GetInstance().get<unsigned long>("instances") + 1; // One extra for MAX_CFT_VALUE
    dw_qos.resource_limits.initial_instances =
            PM::GetInstance().get<unsigned long>("instances") +1;

    if (PM::GetInstance().get<int>("unbounded") != 0) {
        char buf[10];
        sprintf(buf, "%d", PM::GetInstance().get<int>("unbounded"));
        DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
               "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
               buf, false);
    }

    if (PM::GetInstance().get<unsigned long>("instances") > 1) {
        if (PM::GetInstance().is_set("instanceHashBuckets")) {
            dw_qos.resource_limits.instance_hash_buckets =
                PM::GetInstance().get<unsigned long>("instanceHashBuckets");
        } else {
            dw_qos.resource_limits.instance_hash_buckets =
                    PM::GetInstance().get<unsigned long>("instances");
        }
    }

    writer = _publisher->create_datawriter(
        topic, dw_qos, NULL,
        DDS_STATUS_MASK_NONE);

    if (writer == NULL)
    {
        fprintf(stderr,"Problem creating writer.\n");
        return NULL;
    }

    if (!PM::GetInstance().get<bool>("dynamicData")) {
        try {
            return new RTIPublisher<T>(
                    writer,
                    PM::GetInstance().get<unsigned long>("instances"),
                    _pongSemaphore,
                    PM::GetInstance().get<long>("writeInstance"));
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIDDSImpl<T>::CreateWriter(): %s.\n", ex.what());
            return NULL;
        }
    } else {
        try{
            return new RTIDynamicDataPublisher(
                    writer,
                    PM::GetInstance().get<unsigned long>("instances"),
                    _pongSemaphore,
                    T::TypeSupport::get_typecode(),
                    PM::GetInstance().get<long>("writeInstance"));
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIDDSImpl<T>::CreateWriter(): %s.\n", ex.what());
            return NULL;
        }
    }

}

/*********************************************************
 * CreateCFT
 * The CFT allows to the subscriber to receive a specific instance or a range of them.
 * In order generate the CFT it is necesary to create a condition:
 *      - In the case of a specific instance, it is necesary to convert to cftRange[0] into a key notation.
 *        Then it is enought with check that every element of key is equal to the instance.
 *        Exmaple: cftRange[0] = 300. condition ="(0 = key[0] AND 0 = key[1] AND 1 = key[2] AND  44 = key[3])"
 *          So, in the case that the key = { 0, 0, 1, 44}, it will be received.
 *      - In the case of a range of instances, it is necesary to convert to cftRange[0] and cftRange[1] into a key notation.
 *        Then it is enought with check that the key is in the range of instances.
 *        Exmaple: cftRange[1] = 300 and cftRange[1] = 1.
 *          condition = ""
 *              "("
 *                  "("
 *                      "(44 < key[3]) OR"
 *                      "(44 <= key[3] AND 1 < key[2]) OR"
 *                      "(44 <= key[3] AND 1 <= key[2] AND 0 < key[1]) OR"
 *                      "(44 <= key[3] AND 1 <= key[2] AND 0 <= key[1] AND 0 <= key[0])"
 *                  ") AND ("
 *                      "(1 > key[3]) OR"
 *                      "(1 >= key[3] AND 0 > key[2]) OR"
 *                      "(1 >= key[3] AND 0 >= key[2] AND 0 > key[1]) OR"
 *                      "(1 >= key[3] AND 0 >= key[2] AND 0 >= key[1] AND 0 >= key[0])"
 *                  ")"
 *              ")"
 *          The main goal for comaparing a instances and a key is by analyze the elemetns by more significant to the lest significant.
 *          So, in the case that the key is between [ {0, 0, 0, 1} and { 0, 0, 1, 44} ], it will be received.
 *  Beside, there is a special case where all the subscribers will receive the samples, it is MAX_CFT_VALUE = 65535 = [255,255,0,0,]
 */
template <typename T>
DDSTopicDescription *RTIDDSImpl<T>::CreateCft(
        const char *topic_name,
        DDSTopic *topic)
{
    std::string condition;
    DDS_StringSeq parameters(2 * KEY_SIZE);
    std::vector<unsigned long long> cftRange =
            PM::GetInstance().get_vector<unsigned long long>("cft");
    if (cftRange.size() == 1) { // If same elements, no range
        printf("CFT enabled for instance: '%llu' \n",cftRange[0]);
        char cft_param[KEY_SIZE][128];
        for (int i = 0; i < KEY_SIZE ; i++) {
            sprintf(cft_param[i],"%d", (unsigned char)(cftRange[0] >> i * 8));
        }
        const char* param_list[] = { cft_param[0], cft_param[1], cft_param[2], cft_param[3]};
        parameters.from_array(param_list, KEY_SIZE);
        condition = "(%0 = key[0] AND %1 = key[1] AND %2 = key[2] AND %3 = key[3]) OR"
                "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";
    } else { // If cftRange.size() == 2 (RANGE)
        printf("CFT enabled for instance range: [%llu,%llu] \n",
            cftRange[0],
            cftRange[1]);
        char cft_param[2 * KEY_SIZE][128];
        for (int i = 0; i < 2 * KEY_SIZE ; i++ ) {
            if (i < KEY_SIZE) {
                sprintf(cft_param[i], "%d", (unsigned char)(cftRange[0] >> i * 8));
            } else { // KEY_SIZE < i < KEY_SIZE * 2
                sprintf(cft_param[i], "%d", (unsigned char)(cftRange[1] >> i * 8));
            }
        }
        const char* param_list[] = { cft_param[0], cft_param[1],
                cft_param[2], cft_param[3],cft_param[4],
                cft_param[5], cft_param[6], cft_param[7]
        };
        parameters.from_array(param_list, 2 * KEY_SIZE);
        condition = ""
                "("
                    "("
                        "(%3 < key[3]) OR"
                        "(%3 <= key[3] AND %2 < key[2]) OR"
                        "(%3 <= key[3] AND %2 <= key[2] AND %1 < key[1]) OR"
                        "(%3 <= key[3] AND %2 <= key[2] AND %1 <= key[1] AND %0 <= key[0])"
                    ") AND ("
                        "(%7 > key[3]) OR"
                        "(%7 >= key[3] AND %6 > key[2]) OR"
                        "(%7 >= key[3] AND %6 >= key[2] AND %5 > key[1]) OR"
                        "(%7 >= key[3] AND %6 >= key[2] AND %5 >= key[1] AND %4 >= key[0])"
                    ") OR ("
                        "255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3]"
                    ")"
                ")";
    }
    return _participant->create_contentfilteredtopic(
            topic_name,
            topic,
            condition.c_str(),
            parameters);
}

/*********************************************************
 * CreateReader
 */
template <typename T>
IMessagingReader *RTIDDSImpl<T>::CreateReader(
        const char *topic_name,
        IMessagingCB *callback)
{
    DDSDataReader *reader = NULL;
    DDS_DataReaderQos dr_qos;
    std::string qos_profile;
    DDSTopicDescription* topic_desc = NULL; // Used to create the DDS DataReader

    DDSTopic *topic = _participant->create_topic(
                       topic_name, _typename,
                       DDS_TOPIC_QOS_DEFAULT, NULL,
                       DDS_STATUS_MASK_NONE);

    if (topic == NULL) {
        fprintf(stderr,"Problem creating topic %s.\n", topic_name);
        return NULL;
    }
    topic_desc = topic;

    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0) {
        qos_profile = "ThroughputQos";
    } else if (strcmp(topic_name, LATENCY_TOPIC_NAME) == 0) {
        qos_profile = "LatencyQos";
    } else if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) == 0) {
        qos_profile = "AnnouncementQos";
    } else {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME,
                LATENCY_TOPIC_NAME,
                ANNOUNCEMENT_TOPIC_NAME);
        return NULL;
    }

    if (_factory->get_datareader_qos_from_profile(dr_qos,
            PM::GetInstance().get<std::string>("qosLibrary").c_str(),
            qos_profile.c_str())
            != DDS_RETCODE_OK) {
        fprintf(stderr,
                "No QOS Profile named \"%s\" found in QOS Library \"%s\" in file %s.\n",
                qos_profile.c_str(),
                PM::GetInstance().get<std::string>("qosLibrary").c_str(),
                PM::GetInstance().get<std::string>("qosFile").c_str());
        return NULL;
    }

    // only force reliability on throughput/latency topics
    if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) != 0)
    {
        if (!PM::GetInstance().get<bool>("bestEffort")) {
            dr_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        } else {
            dr_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

    if (PM::GetInstance().get<bool>("noPositiveAcks")
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dr_qos.protocol.disable_positive_acks = true;
    }

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos"
            || (qos_profile == "LatencyQos"
                    && PM::GetInstance().get<bool>("noDirectCommunication")
                    && (PM::GetInstance().get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
                            || PM::GetInstance().get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS)))
    {
        dr_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind) PM::GetInstance().get<int>("durability");
        dr_qos.durability.direct_communication =
            !PM::GetInstance().get<bool>("noDirectCommunication");
    }

    dr_qos.resource_limits.initial_instances =
            PM::GetInstance().get<unsigned long>("instances") + 1;
    if (_InstanceMaxCountReader != DDS_LENGTH_UNLIMITED) {
        _InstanceMaxCountReader++;
    }
    dr_qos.resource_limits.max_instances = _InstanceMaxCountReader;

    if (PM::GetInstance().get<unsigned long>("instances") > 1) {
        if (PM::GetInstance().is_set("instanceHashBuckets")) {
            dr_qos.resource_limits.instance_hash_buckets =
                PM::GetInstance().get<unsigned long>("instanceHashBuckets");
        } else {
            dr_qos.resource_limits.instance_hash_buckets =
                PM::GetInstance().get<unsigned long>("instances");
        }
    }

    if (PM::GetInstance().get<bool>("multicast")
            && _transport.allowsMulticast()) {
        dr_qos.multicast.value.ensure_length(1, 1);
        dr_qos.multicast.value[0].receive_address = DDS_String_dup(
                _transport.getMulticastAddr(topic_name).c_str());

        if (dr_qos.multicast.value[0].receive_address == NULL) {
            fprintf(stderr,
                    "topic name must either be %s or %s or %s.\n",
                    THROUGHPUT_TOPIC_NAME,
                    LATENCY_TOPIC_NAME,
                    ANNOUNCEMENT_TOPIC_NAME);
            return NULL;
        }

        dr_qos.multicast.value[0].receive_port = 0;
        dr_qos.multicast.value[0].transports.length(0);
    }

    if (PM::GetInstance().get<int>("unbounded") != 0) {
        char buf[10];
        sprintf(buf, "%d", PM::GetInstance().get<int>("unbounded"));
        DDSPropertyQosPolicyHelper::add_property(dr_qos.property,
                "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                buf, false);
    }

    /* Create CFT Topic */
    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0 &&
            PM::GetInstance().is_set("cft")) {
        topic_desc = CreateCft(topic_name, topic);
        if (topic_desc == NULL) {
            printf("Create_contentfilteredtopic error\n");
            return NULL;
        }
    }

    if (callback != NULL) {

        if (!PM::GetInstance().get<bool>("dynamicData")) {
            reader = _subscriber->create_datareader(
                    topic_desc,
                    dr_qos,
                    new ReceiverListener<T>(callback),
                    DDS_DATA_AVAILABLE_STATUS);
        } else {
            reader = _subscriber->create_datareader(
                    topic_desc,
                    dr_qos,
                    new DynamicDataReceiverListener(callback),
                    DDS_DATA_AVAILABLE_STATUS);
        }

    } else {
        reader = _subscriber->create_datareader(
                topic_desc,
                dr_qos,
                NULL,
                DDS_STATUS_MASK_NONE);
    }

    if (reader == NULL)
    {
        fprintf(stderr,"Problem creating reader.\n");
        return NULL;
    }

    if (!strcmp(topic_name, THROUGHPUT_TOPIC_NAME) ||
        !strcmp(topic_name, LATENCY_TOPIC_NAME)) {
        _reader = reader;
    }

    if (!PM::GetInstance().get<bool>("dynamicData")) {
        return new RTISubscriber<T>(reader);
    } else {
        return new RTIDynamicDataSubscriber<T>(reader);
    }
}

#ifdef RTI_WIN32
  #pragma warning(pop)
#endif
