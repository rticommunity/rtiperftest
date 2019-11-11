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
#ifndef RTI_MICRO
  #include <algorithm> // std::max
  #include "ndds/ndds_cpp.h"
  #include "qos_string.h"
#endif

std::string valid_flow_controller[] = {"default", "1Gbps", "10Gbps"};

template <typename T>
PerftestMutex *RTIDDSImpl<T>::_finalizeFactoryMutex = NULL;

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
   return membersId[key];
}

/*********************************************************
 * Shutdown
 */
template <typename T>
void RTIDDSImpl<T>::Shutdown()
{
    // Get semaphore so other processes cannot dispose at the same time
    if (_finalizeFactoryMutex == NULL) {
        _finalizeFactoryMutex = PerftestMutex_new();
    }

    if (!PerftestMutex_take(_finalizeFactoryMutex)) {
        fprintf(stderr,"Unexpected error taking semaphore\n");
        return;
    }

    if (_participant != NULL) {
        PerftestClock::milliSleep(2000);

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
        PerftestSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }

  #ifdef RTI_MICRO
    if (_factory != NULL) {
        RTRegistry *registry = _factory->get_registry();

        /*
         * Some of these might not be registered, so we
         * won't show any errors if the unregister returns
         * that the module is not registerd.
         */
        if (!registry->unregister("dpde", NULL, NULL)) {
            //printf("failed to unregister dpde\n");
        }
        if (!registry->unregister("rh", NULL, NULL)) {
            //printf("failed to unregister rh\n");
        }
        if (!registry->unregister("wh", NULL, NULL)) {
            //printf("failed to unregister wh\n");
        }
        /*
         * Since Shared Memory is only aviable for Micro 3.0.0 the unregister
         * call would fail for previous versions. But it's ok if the unregister
         * call fails since would not affect the execution.
         */
        if (!registry->unregister("_shmem", NULL, NULL)) {
            //printf("failed to unregister _shmem\n");
        }
      #ifdef RTI_SECURE_PERFTEST
        if (!SECCORE_SecurePluginFactory::unregister_suite(
                    registry,
                    SECCORE_DEFAULT_SUITE_NAME)) {
            //printf("failed to unregister security plugins\n");
        }
      #endif
    }

    // There is no get_participants() API for Micro so just finalize it
    DDSDomainParticipantFactory::finalize_instance();

  #else
    DDSDomainParticipantSeq participants;
    DDS_ReturnCode_t retcode;

    // Unregister _loggerDevice
    if (NDDSConfigLogger::get_instance()->get_output_device() != NULL) {
        NDDSConfigLogger::finalize_instance();
    }

    retcode = DDSTheParticipantFactory->get_participants(participants);
    if (retcode != DDS_RETCODE_OK) {
        printf("Error getting participants. Retcode: %d", retcode);
    }

    if (participants.length() == 0) {
        DDSDomainParticipantFactory::finalize_instance();

        if (_finalizeFactoryMutex != NULL) {
            // Give semaphore so the mutex can be properly closed
            if (!PerftestMutex_give(_finalizeFactoryMutex)) {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return;
            }

            // Delete semaphore since no one else is going to need it
            PerftestMutex_delete(_finalizeFactoryMutex);
            _finalizeFactoryMutex = NULL;
            return;
        }
    } else {
        printf("[Warning] Cannot finalize Domain Factory since it is being in use by another thread(s)\n");
    }
  #endif

    if (!PerftestMutex_give(_finalizeFactoryMutex)) {
            fprintf(stderr, "Unexpected error giving semaphore\n");
            return;
    }
}

/*********************************************************

 * Validate and manage the parameters
 */
template <typename T>
bool RTIDDSImpl<T>::validate_input()
{
    // Manage parameter -instance
    if (_PM->is_set("instances")) {
        _instanceMaxCountReader = _PM->get<long>("instances");
    }
    // Manage parameter -peer
    if (_PM->get_vector<std::string>("peer").size() >= RTIPERFTEST_MAX_PEERS) {
        fprintf(stderr,
                "The maximun of 'initial_peers' is %d\n",
                RTIPERFTEST_MAX_PEERS);
        return false;
    }

    // Check if we need to enable Large Data. This works also for -scan
    if (_PM->get<unsigned long long>("dataLen") > (unsigned long) (std::min)(
            MAX_SYNCHRONOUS_SIZE,
            MAX_BOUNDED_SEQ_SIZE)) {
        _isLargeData = true;
    } else { // No Large Data
        _isLargeData = false;
    }

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

        // Check if using asynchronous
        if (_PM->get<bool>("asynchronous")) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") != 0) {
                fprintf(stderr,
                        "Batching cannot be used with asynchronous writing.\n");
                return false;
            } else {
                _PM->set<long>("batchSize", 0); // Disable Batching
            }
        }

        /*
         * Large Data + batching cannot be set. But batching is enabled by default,
         * so in that case, we just disabled batching, else, the customer set it up,
         * so we explitly fail
         */
        if (_isLargeData) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") != 0) {
                fprintf(stderr, "Batching cannot be used with Large Data.\n");
                return false;
            } else {
                _PM->set<long>("batchSize", -2);
            }
        } else if (((unsigned long)_PM->get<long>("batchSize")
                        < _PM->get<unsigned long long>("dataLen") * 2)
                    && !_PM->is_set("scan")) {
            /*
            * We don't want to use batching if the batch size is not large
            * enough to contain at least two samples (in this case we avoid the
            * checking at the middleware level).
            */
            if (_PM->is_set("batchSize")) {
                /*
                * Batchsize disabled. A message will be print if batchSize < 0
                * in perftest_cpp::PrintConfiguration()
                */
                _PM->set<long>("batchSize", -1);
            } else {
                _PM->set<long>("batchSize", 0); // Disable Batching
            }
        }

        if (_isFlatData) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") > 0) {
                fprintf(stderr, "Batching cannot be used with FlatData.\n");
                return false;
            } else {
                _PM->set<long>("batchSize", -3);
            }
        }
    }

    // Manage parameter -enableTurboMode
    if (_PM->get<bool>("enableTurboMode")) {
        if (_PM->get<bool>("asynchronous")) {
            fprintf(stderr,
                    "Turbo Mode cannot be used with asynchronous writing.\n");
            return false;
        } if (_isLargeData) {
            fprintf(stderr, "Turbo Mode disabled, using large data.\n");
            _PM->set<bool>("enableTurboMode", false);
        }
    }

    // Manage parameter -writeInstance
    if (_PM->is_set("writeInstance")) {
        if (_PM->get<long>("instances") < _PM->get<long>("writeInstance")) {
            fprintf(stderr,
                    "Specified '-WriteInstance' (%ld) invalid: "
                    "Bigger than the number of instances (%ld).\n",
                    _PM->get<long>("writeInstance"),
                    _PM->get<long>("instances"));
            return false;
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
    if (_PM->is_set("verbosity")) {
        PerftestConfigureVerbosity(_PM->get<int>("verbosity"));
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
    stringStream << "\tDomain: " << _PM->get<int>("domain") << "\n";

  #ifndef RTI_MICRO
    // Dynamic Data
    stringStream << "\tDynamic Data: ";
    if (_PM->get<bool>("dynamicData")) {
        stringStream << "Yes";
      #ifdef RTI_LEGACY_DD_IMPL
        if (_PM->get<bool>("useLegacyDynamicData")) {
            stringStream << " (Legacy Implementation)";
        }
      #endif
        stringStream << "\n";
    } else {
        stringStream << "No\n";
    }
  #endif

  #ifdef RTI_FLATDATA_AVAILABLE
    // FlatData
    stringStream << "\tFlatData: "
                 << (_PM->get<bool>("flatdata") ? "Yes" : "No")
                 << std::endl;

    // Zero Copy
    stringStream << "\tZero Copy: "
                 << (_PM->get<bool>("zerocopy") ? "Yes" : "No");

    if (_PM->get<bool>("checkconsistency")) {
            stringStream << " (Check Consistency)";
    }

    stringStream << std::endl;
  #endif

  #ifndef RTI_MICRO
    // Asynchronous Publishing
    if (_PM->get<bool>("pub")) {
        stringStream << "\tAsynchronous Publishing: ";
        if ((_isLargeData || _PM->get<bool>("asynchronous"))
                && !_PM->get<bool>("zerocopy")) {
            stringStream << "Yes\n";
            stringStream << "\tFlow Controller: "
                         << _PM->get<std::string>("flowController")
                         << "\n";
        } else {
            stringStream << "No\n";
        }
    }
  #endif

    // Turbo Mode / AutoThrottle
    if (_PM->get<bool>("enableTurboMode")) {
        stringStream << "\tTurbo Mode: Enabled\n";
    }
    if (_PM->get<bool>("enableAutoThrottle")) {
        stringStream << "\tAutoThrottle: Enabled\n";
    }

  #ifndef RTI_MICRO
    // XML File
    stringStream << "\tXML File: ";
    if (_PM->get<bool>("noXmlQos")) {
        stringStream << "Disabled\n";
    } else {
        stringStream << _PM->get<std::string>("qosFile") << "\n";
    }
  #endif

    stringStream << "\n" << _transport.printTransportConfigurationSummary();

    const std::vector<std::string> peerList = _PM->get_vector<std::string>("peer");
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

   #ifdef RTI_SECURE_PERFTEST
    if (_PM->group_is_used(SECURE)) {
        stringStream << "\n" << _security.printSecurityConfigurationSummary();
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
class RTIPublisherBase : public IMessagingWriter
{
  protected:
    typename T::DataWriter *_writer;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    DDS_InstanceHandle_t *_instance_handles;
    PerftestSemaphore *_pongSemaphore;
    long _instancesToBeWritten;
  #ifdef RTI_CUSTOM_TYPE
    unsigned int _lastMessageSize;
    unsigned int _minCustomTypeSerializeSize;
  #endif
    bool _isReliable;
    ParameterManager *_PM;

    DDS_InstanceHandle_t &getCftInstanceHandle() {
        return _instance_handles[_num_instances];
    }

  #ifdef RTI_CUSTOM_TYPE
    bool is_sentinel_size(int size) {
        return size == perftest_cpp::INITIALIZE_SIZE
                || size == perftest_cpp::FINISHED_SIZE
                || size == perftest_cpp::LENGTH_CHANGED_SIZE
                || size == 0;
    }
  #endif

 public:
    RTIPublisherBase(
            unsigned long num_instances,
            PerftestSemaphore * pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
    {
        _PM = PM;
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
    }

    ~RTIPublisherBase() {
        try {
            Shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIPublisherBase::~RTIPublisherBase(): %s.\n", ex.what());
        }
    }

    void Shutdown() {
        if (_writer->get_listener() != NULL) {
            delete(_writer->get_listener());
          #ifndef RTI_MICRO
            _writer->set_listener(NULL);
          #else
            _writer->set_listener(NULL, 0);
          #endif
        }
        if (_instance_handles != NULL) {
            free(_instance_handles);
            _instance_handles = NULL;
        }
    }

    void Flush()
    {
      #ifndef RTI_MICRO
        _writer->flush();
      #endif
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
      #ifndef RTI_MICRO
        DDS_DataWriterProtocolStatus status;
        _writer->get_datawriter_protocol_status(status);
        return (unsigned int)status.pulled_sample_count;
      #else
        // Not supported in Micro
        return 0;
      #endif
    }

    unsigned int getSampleCount()
    {
      #ifndef RTI_MICRO
        DDS_DataWriterCacheStatus status;
        DDS_ReturnCode_t retcode = _writer->get_datawriter_cache_status(status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "get_datawriter_cache_status failed: %d.\n", retcode);
            return 0;
        }
        return (unsigned int)status.sample_count;
      #else
        // Not supported in Micro
        return 0;
      #endif
    }

    unsigned int getSampleCountPeak()
    {
      #ifndef RTI_MICRO
        DDS_DataWriterCacheStatus status;
        DDS_ReturnCode_t retcode = _writer->get_datawriter_cache_status(status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "get_datawriter_cache_status failed: %d.\n", retcode);
            return 0;
        }
        return (unsigned int)status.sample_count_peak;
      #else
        // Not supported in Micro
        return 0;
      #endif
    }

    void waitForAck(int sec, unsigned int nsec) {
      #ifndef RTI_MICRO
        if (_isReliable) {
            DDS_Duration_t timeout = {sec, nsec};
            _writer->wait_for_acknowledgments(timeout);
        } else {
            PerftestClock::milliSleep(nsec / 1000000);
        }
      #endif
    }
};

template<typename T>
class RTIPublisher : public RTIPublisherBase<T>
{
    T data;

  #ifdef RTI_CUSTOM_TYPE
    bool get_serialize_size_custom_type_data(unsigned int &size) {
        DDS_ReturnCode_t retcode = RTI_CUSTOM_TYPE::TypeSupport::serialize_data_to_cdr_buffer(
                NULL,
                (unsigned int &)size,
                &this->data.custom_type);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "serialize_data_to_cdr_buffer failed: %d.\n", retcode);
            return false;
        }
        size -= RTI_CDR_ENCAPSULATION_HEADER_SIZE;
        return true;
    }
  #endif

 public:
    RTIPublisher(
            DDSDataWriter *writer,
            unsigned long num_instances,
            PerftestSemaphore * pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
            : RTIPublisherBase<T>(
                    num_instances,
                    pongSemaphore,
                    instancesToBeWritten,
                    PM)
    {
      #ifdef RTI_CUSTOM_TYPE
        this->_lastMessageSize = 0;
        // Calculate _minCustomTypeSerializeSize
        if (!this->get_serialize_size_custom_type_data(this->_minCustomTypeSerializeSize)) {
            throw std::runtime_error("get_serialize_size_custom_type_data failed");
        }
        // Initialize data
        DDS_ReturnCode_t retcode = RTI_CUSTOM_TYPE::TypeSupport::initialize_data(
                &this->data.custom_type);
        if (retcode != DDS_RETCODE_OK) {
            RTI_CUSTOM_TYPE::TypeSupport::finalize_data(&this->data.custom_type);
            throw std::runtime_error("TypeSupport::initialize_data failed");
        }
        if (!initialize_custom_type_data(this->data.custom_type)) {
            RTI_CUSTOM_TYPE::TypeSupport::finalize_data(&this->data.custom_type);
            throw std::runtime_error("initialize_custom_type_data failed");
        }
      #endif

        this->_writer = T::DataWriter::narrow(writer);

        DDS_DataWriterQos qos;
        this->_writer->get_qos(qos); // Gota fix the writer narrow to fix seg fault here
        this->_isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

        for (unsigned long i = 0; i < this->_num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                data.key[c] = (unsigned char) (i >> c * 8);
            }
          #ifdef RTI_CUSTOM_TYPE
            register_custom_type_data(data.custom_type, i);
          #endif
            this->_instance_handles[i] = this->_writer->register_instance(data);
        }

        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            data.key[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
        }

      #ifdef RTI_CUSTOM_TYPE
        register_custom_type_data(data.custom_type, MAX_CFT_VALUE);
      #endif

        this->_instance_handles[this->_num_instances] = this->_writer->register_instance(data);
    }

    ~RTIPublisher() {
        try {
          #ifdef RTI_CUSTOM_TYPE
            if (!finalize_custom_type_data(this->data.custom_type)) {
                throw std::runtime_error("finalize_custom_type_data failed");
            }
            RTI_CUSTOM_TYPE::TypeSupport::finalize_data(&this->data.custom_type);
          #endif

            this->Shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIPublisher::~RTIPublisher(): %s.\n", ex.what());
        }
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        bool success = true;
        long key = 0;

        // Calculate key and add it if using more than one instance
        if (!isCftWildCardKey) {
            if (this->_num_instances > 1) {
                if (this->_instancesToBeWritten == -1) {
                    key = this->_instance_counter++ % this->_num_instances;
                } else { // send sample to a specific subscriber
                    key = this->_instancesToBeWritten;
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
        if (this->is_sentinel_size(message.size)) {
            data.custom_type_size = message.size;
        } else {
            if (!set_custom_type_data(
                    data.custom_type,
                    key,
                    message.size - this->_minCustomTypeSerializeSize)) {
                fprintf(stderr, "set_custom_type_data failed.\n");
                return false;
            }
            if ((unsigned int)message.size != this->_lastMessageSize) {
                success = get_serialize_size_custom_type_data(
                        (unsigned int &)data.custom_type_size);
                if (!success) {
                    return false;
                }
                this->_lastMessageSize = message.size;
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
            retcode = this->_writer->write(data, this->_instance_handles[key]);
        } else { // send CFT_MAX sample
            retcode = this->_writer->write(data, this->getCftInstanceHandle());
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
};

#ifndef RTI_MICRO //Dynamic Data and FlatData are not supported for micro

#ifdef RTI_FLATDATA_AVAILABLE

/**
 * Implementation of RTIPublisherBase for FlatData types.
 *
 * Since building a FlatData sample differs from
 * a classic type, we need to reimplement the Send() method with the
 * FlatData API.
 */
template<typename T>
class RTIFlatDataPublisher: public RTIPublisherBase<T> {
protected:
    typedef typename rti::flat::flat_type_traits<T>::builder Builder;
    typedef typename rti::flat::PrimitiveArrayOffset<unsigned char, 4> KeyBuilder;
    typedef typename rti::flat::PrimitiveSequenceBuilder<unsigned char> BinDataBuilder;
  #ifdef RTI_CUSTOM_TYPE_FLATDATA
    typedef typename rti::flat::flat_type_traits<RTI_CUSTOM_TYPE_FLATDATA>::builder BuilderCT;

    unsigned int _lastBufferSize;
  #endif

    void add_key(Builder &builder, unsigned long int i) {
        KeyBuilder key_offset = builder.add_key();

        for (int j = 0; j < KEY_SIZE; ++j) {
            // The key will be i but splitted in bytes
            key_offset.set_element(j, (unsigned char) (i >> j * 8));
        }
    }

public:
    RTIFlatDataPublisher(
            DDSDataWriter *writer,
            unsigned long num_instances,
            PerftestSemaphore *pongSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
            : RTIPublisherBase<T>(
                    num_instances,
                    pongSemaphore,
                    instancesToBeWritten,
                    PM)
    {
        this->_writer = T::DataWriter::narrow(writer);

        DDS_DataWriterQos qos;
        this->_writer->get_qos(qos); // Gota fix the writer narrow to fix seg fault here
        this->_isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

      #ifdef RTI_CUSTOM_TYPE_FLATDATA
        {
            this->_lastMessageSize = 0;
            _lastBufferSize = 0;

            Builder builder = rti::flat::build_data<T>(this->_writer);
            BuilderCT builderCT = builder.build_custom_type();

            // Create a sample without the sequence
            if (!set_custom_type_data_flatdata(builderCT, 0, 0)) {
                throw std::runtime_error("set_custom_type_data_flatdata failed");
            }

            rti::flat::OffsetBase ct = builderCT.finish();
            this->_minCustomTypeSerializeSize = ct.get_buffer_size();

            T *sample = builder.finish_sample();
            this->_writer->discard_loan(*sample);
        }
      #endif

        for (unsigned long i = 0; i < this->_num_instances; ++i) {
            Builder builder = rti::flat::build_data<T>(this->_writer);
            add_key(builder, i);
          #ifdef RTI_CUSTOM_TYPE_FLATDATA
            BuilderCT builderCT = builder.build_custom_type();
            register_custom_type_data_flatdata(builderCT, i);
            builderCT.finish();
          #endif

            T *sample = builder.finish_sample();

            this->_instance_handles[i] = this->_writer->register_instance(*sample);
        }

        Builder builder = rti::flat::build_data<T>(this->_writer);
        add_key(builder, MAX_CFT_VALUE);

        #ifdef RTI_CUSTOM_TYPE_FLATDATA
          BuilderCT builderCT = builder.build_custom_type();
          register_custom_type_data_flatdata(builderCT, MAX_CFT_VALUE);
          builderCT.finish();
        #endif

        T *sample = builder.finish_sample();


        this->_instance_handles[this->_num_instances] = this->_writer->register_instance(*sample);
        this->_writer->discard_loan(*sample);
    }

    ~RTIFlatDataPublisher() {
        try {
            this->Shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIPublisher::~RTIPublisher(): %s.\n", ex.what());
        }
    }

    /**
     * Build and send a sample from a given message using FlatData API.
     *
     * @param message the message that contains the information to build the sample
     * @param isCftWildcardKey states if CFT is being used
     */
    bool Send(const TestMessage &message, bool isCftWildCardKey) {
        long key = 0;
        Builder builder = rti::flat::build_data<T>(this->_writer);
        if (!builder.is_valid()) {
            return false;
        }

        // Calculate key if using more than one instance
        if (!isCftWildCardKey) {
            if (this->_num_instances > 1) {
                key = (this->_instancesToBeWritten == -1)
                        ? this->_instance_counter++ % this->_num_instances
                        : this->_instancesToBeWritten;
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        add_key(builder, key);

        // Initialize Information data
        builder.add_entity_id(message.entity_id);
        builder.add_seq_num(message.seq_num);
        builder.add_timestamp_sec(message.timestamp_sec);
        builder.add_timestamp_usec(message.timestamp_usec);
        builder.add_latency_ping(message.latency_ping);

        // Add payload
      #ifdef RTI_CUSTOM_TYPE_FLATDATA
        /**
         * Using custom type the size of the data is set in data.custom_type_size:
         *      If the message.size is a sentinel size value used to handle the test:
         *          data.custom_type_size = message.size
         *      Else:
         *          If the message.size is different from the last iteration:
         *              data.custom_type_size of the custom type (data.custom_type)
         *              is measured from this member in the buffer
         *          Else:
         *              data.custom_type_size is the same as the last iteration
        */
        if (this->is_sentinel_size(message.size)) {
            builder.add_custom_type_size(message.size);
        } else {
            BuilderCT customTypeBuilder = builder.build_custom_type();

            if (!set_custom_type_data_flatdata(
                    customTypeBuilder,
                    key,
                    message.size - this->_minCustomTypeSerializeSize)) {
                fprintf(stderr, "set_custom_type_data failed.\n");
                return false;
            }

            rti::flat::OffsetBase ct = customTypeBuilder.finish();

            if ((unsigned int)message.size != this->_lastMessageSize) {
                _lastBufferSize = ct.get_buffer_size();
                this->_lastMessageSize = message.size;
            }

            builder.add_custom_type_size(_lastBufferSize);
        }
      #else
        BinDataBuilder bin_data_builder = builder.build_bin_data();
        bin_data_builder.add_n(message.size);
        bin_data_builder.finish();
      #endif

        // Build the data to be sent
        T *sample = builder.finish_sample();

        // Send data through the writer
        if (!isCftWildCardKey) {
            this->_writer->write(*sample, this->_instance_handles[key]);
        } else {
            this->_writer->write(*sample, this->getCftInstanceHandle());
        }

        return true;
    }
};
#endif

/* Dynamic Data equivalent function from RTIPublisher */
class RTIDynamicDataPublisher: public RTIPublisherBase<DDS_DynamicData>
{
  private:
    DDS_DynamicData data;
    int _lastMessageSize;

  #ifdef RTI_CUSTOM_TYPE
    unsigned int _customTypeSize;

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
                RTI_OSAPI_ALIGNMENT_DEFAULT);
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

  public:
    RTIDynamicDataPublisher(
            DDSDataWriter *writer,
            unsigned long num_instances,
            PerftestSemaphore *pongSemaphore,
            DDS_TypeCode *typeCode,
            int instancesToBeWritten,
            ParameterManager *PM)
            : RTIPublisherBase<DDS_DynamicData>(
                    num_instances,
                    pongSemaphore,
                    instancesToBeWritten,
                    PM),
            data(typeCode, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT),
            _lastMessageSize(0)
    {
      #ifdef RTI_CUSTOM_TYPE

        // Calculate size_alignment_type of DDS_DynamicData object
        if (!get_serialize_size_custom_type_data(this->_minCustomTypeSerializeSize)) {
            throw std::runtime_error("get_serialize_size_custom_type_data failed");
        }
        // Initialize data
        if (!initialize_custom_type_dynamic_data(data)) {
            // ~DDS_DynamicData() will be called and data will be free
            throw std::runtime_error("initialize_custom_type_dynamic_data failed");
        }
      #endif

        DDS_ReturnCode_t retcode;
        DDS_Octet key_octets[KEY_SIZE];

        this->_writer = DDSDynamicDataWriter::narrow(writer);

        DDS_DataWriterQos qos;
        this->_writer->get_qos(qos); // Gota fix the writer narrow to fix seg fault here
        this->_isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);

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

            this->_instance_handles[i] = this->_writer->register_instance(data);
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

        this->_instance_handles[_num_instances] = this->_writer->register_instance(data);
    }

    ~RTIDynamicDataPublisher() {
        try {
            this->Shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIDynamicDataPublisher::~RTIDynamicDataPublisher(): %s.\n", ex.what());
        }
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        DDS_Octet key_octets[KEY_SIZE];
        long key = 0;

        if (!isCftWildCardKey) {
            if (this->_num_instances > 1) {
                if (this->_instancesToBeWritten == -1) {
                    key = this->_instance_counter++ % this->_num_instances;
                } else { // send sample to a specific subscriber
                    key = this->_instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (unsigned char) (key >> c * 8);
        }

        if (this->_lastMessageSize != message.size) {
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
        if (this->_lastMessageSize != message.size) {
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
        if (this->is_sentinel_size(message.size)) {
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
                    message.size - this->_minCustomTypeSerializeSize)) {
                fprintf(stderr, "set_custom_type_dynamic_data failed.\n");
                return false;
            }
            if (message.size != this->_lastMessageSize) {
                if (!get_serialize_size_custom_type_data(this->_customTypeSize)) {
                    fprintf(stderr, "get_serialize_size_custom_type_data.\n");
                }
            }
            retcode = data.set_long(
                    "custom_type_size",
                    DynamicDataMembersId::GetInstance().at("custom_type_size"),
                    this->_customTypeSize);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_long(custom_type_size) failed: %d.\n", retcode);
                return false;
            }
        }
      #endif
        this->_lastMessageSize = message.size;

        if (!isCftWildCardKey) {
            retcode = this->_writer->write(data, this->_instance_handles[key]);
        } else {
            retcode = this->_writer->write(data, this->getCftInstanceHandle());
        }

        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Write error %d.\n", retcode);
            return false;
        }
        return true;
    }


};

#endif //!RTI_MICRO: Dynamic Data not supported for micro

/*********************************************************
 * ReceiverListener
 */
template <typename T>
class ReceiverListenerBase : public DDSDataReaderListener
{
  protected:
    typename T::Seq     _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage       _message;
    IMessagingCB     *_callback;

  public:

    ReceiverListenerBase(IMessagingCB *callback): _message()
    {
        _callback = callback;
    }
};


template <typename T>
class ReceiverListener : public ReceiverListenerBase<T>
{
  public:
    ReceiverListener(IMessagingCB *callback)
            : ReceiverListenerBase<T>(callback) {
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
                this->_data_seq, this->_info_seq,
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

        int seq_length = this->_data_seq.length();
        for (int i = 0; i < seq_length; ++i)
        {
            if (this->_info_seq[i].valid_data)
            {
                this->_message.entity_id = this->_data_seq[i].entity_id;
                this->_message.seq_num = this->_data_seq[i].seq_num;
                this->_message.timestamp_sec = this->_data_seq[i].timestamp_sec;
                this->_message.timestamp_usec = this->_data_seq[i].timestamp_usec;
                this->_message.latency_ping = this->_data_seq[i].latency_ping;
              #ifdef RTI_CUSTOM_TYPE
                this->_message.size = this->_data_seq[i].custom_type_size;
              #else
                this->_message.size = this->_data_seq[i].bin_data.length();
              #endif
                this->_message.data = (char *)this->_data_seq[i].bin_data.get_contiguous_buffer();

                this->_callback->ProcessMessage(this->_message);
            }
        }

        retcode = datareader->return_loan(this->_data_seq, this->_info_seq);
        if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Error during return loan %d.\n", retcode);
            fflush(stderr);
        }

    }

};


#ifndef RTI_MICRO //Dynamic Data and FlatData are not supported for micro

#ifdef RTI_FLATDATA_AVAILABLE
/**
 * Implements ReceiverListenerBase with FlatData API.
 *
 * Since reading a FlatData sample differs from a classic type we need
 * to reimplement on_data_available method.
 */
template <typename T>
class FlatDataReceiverListener : public ReceiverListenerBase<T>
{
protected:
    bool _isZeroCopy;
    bool _checkConsistency;

public:
    typedef typename rti::flat::flat_type_traits<T>::offset Offset;

    /**
     * Contructor of FlatDataReceiverListener
     *
     * @param callback callback that will process received messages
     *
     * @param isZeroCopy states if Zero Copy will be used
     */
    FlatDataReceiverListener(IMessagingCB *callback, bool isZeroCopy, bool checkConsistency)
            : ReceiverListenerBase<T>(callback),
            _isZeroCopy(isZeroCopy),
            _checkConsistency(checkConsistency) {
    }

    /**
     * Take a new sample and process it using FlatData API.
     *
     * @param reader is the reader to take samples from
     */
    void on_data_available(DDSDataReader *reader)
    {
        typename T::DataReader *datareader;
        DDS_Boolean isConsistent;

        datareader = T::DataReader::narrow(reader);
        if (datareader == NULL)
        {
            fprintf(stderr,"DataReader narrow error.\n");
            return;
        }

        DDS_ReturnCode_t retcode = datareader->take(
                this->_data_seq, this->_info_seq,
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

        int seq_length = this->_data_seq.length();
        for (int i = 0; i < seq_length; ++i)
        {
            if (this->_info_seq[i].valid_data)
            {
                Offset message = this->_data_seq[i].root();

                this->_message.entity_id = message.entity_id();
                this->_message.seq_num = message.seq_num();
                this->_message.timestamp_sec = message.timestamp_sec();
                this->_message.timestamp_usec = message.timestamp_usec();
                this->_message.latency_ping = message.latency_ping();
              #ifdef RTI_CUSTOM_TYPE_FLATDATA
                this->_message.size = message.custom_type_size();
              #else
                this->_message.size = message.bin_data().element_count();
              #endif
                /*
                 * For regular data in C++ Classic we just use the pointer like
                 * this:
                 *
                 * _message.data = (char *)_data_seq[i].bin_data.get_contiguous_buffer();
                 *
                 * In Flat Data we cannot do this though, therefore what we do is
                 * just assume that the buffer we have created already is what we
                 * want to send. This should be modified in case we wanted to really
                 * send some specific Data.
                 */

                // Check that the sample was not modified on the publisher side when using Zero Copy.
                if (_isZeroCopy && _checkConsistency) {
                    if (datareader->is_data_consistent(
                            isConsistent,
                            this->_data_seq[i],
                            this->_info_seq[i]) != DDS_RETCODE_OK) {
                        fprintf(stderr, "Error checking sample consistency\n");
                    }

                    if (!isConsistent) {
                        continue;
                    }
                }

                this->_callback->ProcessMessage(this->_message);
            }
        }

        retcode = datareader->return_loan(this->_data_seq, this->_info_seq);
        if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Error during return loan %d.\n", retcode);
            fflush(stderr);
        }

    }
};
#endif //RTI_FLATDATA_AVAILABLE



/* Dynamic Data equivalent function from ReceiverListener */
class DynamicDataReceiverListener : public ReceiverListenerBase<DDS_DynamicData>
{
  private:
    DDS_DynamicDataSeq _data_seq;

  public:
    DynamicDataReceiverListener(IMessagingCB *callback)
            : ReceiverListenerBase<DDS_DynamicData>(callback) {
    }

    void on_data_available(DDSDataReader *reader)
    {
        DDSDynamicDataReader *datareader = DDSDynamicDataReader::narrow(reader);
        if (datareader == NULL) {
            fprintf(stderr, "DataReader narrow error.\n");
            return;
        }

        DDS_ReturnCode_t retcode = datareader->take(
                this->_data_seq,
                this->_info_seq,
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
        for (int i = 0; i < this->_data_seq.length(); ++i) {
            if (this->_info_seq[i].valid_data) {
                retcode = this->_data_seq[i].get_long(
                        this->_message.entity_id,
                        "entity_id",
                        DynamicDataMembersId::GetInstance().at("entity_id"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(entity_id) failed: %d.\n",
                            retcode);
                    this->_message.entity_id = 0;
                }
                retcode = this->_data_seq[i].get_ulong(
                        this->_message.seq_num,
                        "seq_num",
                        DynamicDataMembersId::GetInstance().at("seq_num"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_ulong(seq_num) failed: %d.\n",
                            retcode);
                    this->_message.seq_num = 0;
                }
                retcode = this->_data_seq[i].get_long(
                        this->_message.timestamp_sec,
                        "timestamp_sec",
                        DynamicDataMembersId::GetInstance().at("timestamp_sec"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(timestamp_sec) failed: %d.\n",
                            retcode);
                    this->_message.timestamp_sec = 0;
                }
                retcode = _data_seq[i].get_ulong(
                        this->_message.timestamp_usec,
                        "timestamp_usec",
                        DynamicDataMembersId::GetInstance().at("timestamp_usec"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_ulong(timestamp_usec) failed: %d.\n",
                            retcode);
                    this->_message.timestamp_usec = 0;
                }
                retcode = this->_data_seq[i].get_long(
                        this->_message.latency_ping,
                        "latency_ping",
                        DynamicDataMembersId::GetInstance().at("latency_ping"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(latency_ping) failed: %d.\n",
                            retcode);
                    this->_message.latency_ping = 0;
                }
              #ifdef RTI_CUSTOM_TYPE
                retcode = _data_seq[i].get_long(
                        this->_message.size,
                        "custom_type_size",
                        DynamicDataMembersId::GetInstance().at("custom_type_size"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(size) failed: %d.\n",
                            retcode);
                    this->_message.size = 0;
                }
              #else
                retcode = this->_data_seq[i].get_octet_seq(
                        octetSeq,
                        "bin_data",
                        DynamicDataMembersId::GetInstance().at("bin_data"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_octet_seq(bin_data) failed: %d.\n",
                            retcode);
                }
                this->_message.size = octetSeq.length();
                this->_message.data = (char *)octetSeq.get_contiguous_buffer();
              #endif

                this->_callback->ProcessMessage(this->_message);
            }
        }

        retcode = datareader->return_loan(this->_data_seq, this->_info_seq);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Error during return loan %d.\n", retcode);
            fflush(stderr);
        }
    }
};
#endif // !RTI_MICRO: Dynamic Data not supported for micro

/*********************************************************
 * RTISubscriber
 */

template <typename T>
class RTISubscriberBase : public IMessagingReader
{
  protected:
    typename T::DataReader *_reader;
    typename T::Seq         _data_seq;
    DDS_SampleInfoSeq       _info_seq;
    TestMessage             _message;
    DDSWaitSet             *_waitset;
    DDSConditionSeq         _active_conditions;
    DDSGuardCondition       _endTestCondition;
    int                     _data_idx;
    bool                    _no_data;
    bool                    _endTest;
    ParameterManager       *_PM;

  public:

    RTISubscriberBase(DDSDataReader *reader, ParameterManager *PM): _message()
    {
        _reader = T::DataReader::narrow(reader);
        _data_idx = 0;
        _no_data = false;
        _endTest = false;
        _PM = PM;
        _waitset = NULL;

        // null listener means using receive thread
        if (_reader->get_listener() == NULL) {
          #ifndef RTI_MICRO
            DDS_WaitSetProperty_t property;
            property.max_event_count =
                    _PM->get<long>("waitsetEventCount");
            property.max_event_delay =
                    DDS_Duration_t::from_micros(
                        (long)_PM->get<unsigned long long>("waitsetDelayUsec"));

            _waitset = new DDSWaitSet(property);
          #else
            _active_conditions.ensure_length(1,1);
            _waitset = new DDSWaitSet();
          #endif

            DDSStatusCondition *reader_status;
            reader_status = reader->get_statuscondition();
            reader_status->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
            _waitset->attach_condition(reader_status);
            _waitset->attach_condition((DDSCondition *) &_endTestCondition);
        }
    }

    ~RTISubscriberBase()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_reader->get_listener() != NULL) {
            delete(_reader->get_listener());
          #ifndef RTI_MICRO
            _reader->set_listener(NULL);
          #else
            _reader->set_listener(NULL, 0);
          #endif
        }

        if (_waitset != NULL) {
            delete _waitset;
        }

        // loan may be outstanding during shutdown
        _reader->return_loan(_data_seq, _info_seq);
    }

    void WaitForWriters(int numPublishers)
    {
        DDS_SubscriptionMatchedStatus status;

        while (true) {
            _reader->get_subscription_matched_status(status);
            if (status.current_count >= numPublishers) {
                break;
            }
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    bool unblock()
    {
        _endTest = true;
        if (_endTestCondition.set_trigger_value(DDS_BOOLEAN_TRUE)
                != DDS_RETCODE_OK) {
            fprintf(stderr,
                    "Error setting a GuardCondition on unblock.\n");
            return false;
        }
        return true;
    }

    unsigned int getSampleCount()
    {
      #ifndef RTI_MICRO
        DDS_DataReaderCacheStatus status;
        DDS_ReturnCode_t retcode = _reader->get_datareader_cache_status(status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "get_datareader_cache_status failed: %d.\n", retcode);
            return 0;
        }
        return (unsigned int)status.sample_count;
      #else
        // Not supported in Micro
        return 0;
      #endif
    }

    unsigned int getSampleCountPeak()
    {
      #ifndef RTI_MICRO
        DDS_DataReaderCacheStatus status;
        DDS_ReturnCode_t retcode = _reader->get_datareader_cache_status(status);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "get_datareader_cache_status failed: %d.\n", retcode);
            return 0;
        }
        return (unsigned int)status.sample_count_peak;
      #else
        // Not supported in Micro
        return 0;
      #endif
    }
};

template <typename T>
class RTISubscriber : public RTISubscriberBase<T>
{
  public:
    RTISubscriber(DDSDataReader *reader, ParameterManager *PM)
            : RTISubscriberBase<T>(reader, PM) {
    }

    ~RTISubscriber()
    {
        this->Shutdown();
    }

    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        int seq_length;

        while (!this->_endTest) {

            // no outstanding reads
            if (this->_no_data)
            {
                this->_waitset->wait(this->_active_conditions, DDS_DURATION_INFINITE);

                if (this->_active_conditions.length() == 0)
                {
                    //printf("Read thread woke up but no data\n.");
                    //return NULL;
                    continue;
                }

                retcode = this->_reader->take(
                    this->_data_seq, this->_info_seq,
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

                this->_data_idx = 0;
                this->_no_data = false;
            }

            seq_length = this->_data_seq.length();
            // check to see if hit end condition
            if (this->_data_idx == seq_length)
            {
                this->_reader->return_loan(this->_data_seq, this->_info_seq);
                this->_no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (this->_info_seq[this->_data_idx].valid_data == false)
                        && (++this->_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (this->_data_idx == seq_length) {
                continue;
            }

            this->_message.entity_id = this->_data_seq[this->_data_idx].entity_id;
            this->_message.seq_num = this->_data_seq[this->_data_idx].seq_num;
            this->_message.timestamp_sec = this->_data_seq[this->_data_idx].timestamp_sec;
            this->_message.timestamp_usec = this->_data_seq[this->_data_idx].timestamp_usec;
            this->_message.latency_ping = this->_data_seq[this->_data_idx].latency_ping;
          #ifdef RTI_CUSTOM_TYPE
            this->_message.size = this->_data_seq[this->_data_idx].custom_type_size;
          #else
            this->_message.size = this->_data_seq[this->_data_idx].bin_data.length();
          #endif
            this->_message.data = (char *)this->_data_seq[this->_data_idx].bin_data.get_contiguous_buffer();

            ++this->_data_idx;

            return &this->_message;
        }
        return NULL;
    }
};

#ifndef RTI_MICRO

#ifdef RTI_FLATDATA_AVAILABLE
/**
 * Implements RTISubscriberBase with FlatData API.
 *
 * Since reading a FlatData sample differs from a classic type we need
 * to reimplement ReceiveMessage method.
 */
template <typename T>
class RTIFlatDataSubscriber : public RTISubscriberBase<T>
{
protected:
    bool _isZeroCopy;
    bool _checkConsistency;

public:
    typedef typename rti::flat::flat_type_traits<T>::offset::ConstOffset ConstOffset;

    RTIFlatDataSubscriber(DDSDataReader *reader, ParameterManager *PM)
            : RTISubscriberBase<T>(reader, PM) {
        _isZeroCopy = PM->get<bool>("zerocopy");
        _checkConsistency = PM->get<bool>("checkconsistency");
    }

    ~RTIFlatDataSubscriber()
    {
        this->Shutdown();
    }

    /**
     * Receive a new sample when it is available. It uses a waitset
     *
     * @return a message with the information from the sample
     */
    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        DDS_Boolean isConsistent;
        int seq_length;

        while (!this->_endTest) {

            // no outstanding reads
            if (this->_no_data)
            {
                this->_waitset->wait(this->_active_conditions, DDS_DURATION_INFINITE);

                if (this->_active_conditions.length() == 0)
                {
                    //printf("Read thread woke up but no data\n.");
                    //return NULL;
                    continue;
                }

                retcode = this->_reader->take(
                    this->_data_seq, this->_info_seq,
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

                this->_data_idx = 0;
                this->_no_data = false;
            }

            seq_length = this->_data_seq.length();
            // check to see if hit end condition
            if (this->_data_idx == seq_length)
            {
                this->_reader->return_loan(this->_data_seq, this->_info_seq);
                this->_no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (this->_info_seq[this->_data_idx].valid_data == false)
                        && (++this->_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (this->_data_idx == seq_length) {
                continue;
            }

            const T &message_sample = this->_data_seq[this->_data_idx];
            ConstOffset message = message_sample.root();

            this->_message.entity_id = message.entity_id();
            this->_message.seq_num = message.seq_num();
            this->_message.timestamp_sec = message.timestamp_sec();
            this->_message.timestamp_usec = message.timestamp_usec();
            this->_message.latency_ping = message.latency_ping();
          #ifdef RTI_CUSTOM_TYPE_FLATDATA
            this->_message.size = message.custom_type_size();
          #else
            this->_message.size = message.bin_data().element_count();
          #endif
            /*
             * For regular data in C++ Classic we just use the pointer like
             * this:
             *
             * this->_message.data = (char *)_data_seq[_data_idx].bin_data.get_contiguous_buffer();
             *
             * In Flat Data we cannot do this though, therefore what we do is
             * just assume that the buffer we have created already is what we
             * want to send. This should be modified in case we wanted to really
             * send some specific Data.
             */

            ++this->_data_idx;

            // Check that the sample was not modified on the publisher side when using Zero Copy.
            if (_isZeroCopy && _checkConsistency) {
                if (this->_reader->is_data_consistent(
                        isConsistent,
                        this->_data_seq[this->_data_idx],
                        this->_info_seq[this->_data_idx]) != DDS_RETCODE_OK) {
                    fprintf(stderr, "Error checking sample consistency\n");
                }

                if (!isConsistent) continue;
            }


            return &this->_message;
        }
        return NULL;
    }
};
#endif

/* Dynamic Data equivalent function from RTISubscriber */
template <typename T>
class RTIDynamicDataSubscriber : public RTISubscriberBase<DDS_DynamicData>
{
  private:
    DDSDynamicDataReader *_reader;
    DDS_DynamicDataSeq _data_seq;

  public:
    RTIDynamicDataSubscriber(
            DDSDataReader *reader,
            ParameterManager *PM)
            : RTISubscriberBase<DDS_DynamicData>(reader, PM)
    {
        _reader = DDSDynamicDataReader::narrow(reader);
        if (_reader == NULL) {
            fprintf(stderr,"DDSDynamicDataReader::narrow(reader) error.\n");
        }

        // null listener means using receive thread
        if (_reader->get_listener() == NULL) {

            DDS_WaitSetProperty_t property;
            property.max_event_count =
                    _PM->get<long>("waitsetEventCount");
            property.max_event_delay.sec =
                    (long)_PM->get<unsigned long long>("waitsetDelayUsec")
                    / 1000000;
            property.max_event_delay.nanosec =
                    (_PM->get<unsigned long long>("waitsetDelayUsec") % 1000000)
                    * 1000;
            this->_waitset = new DDSWaitSet(property);
            DDSStatusCondition *reader_status;
            reader_status = reader->get_statuscondition();
            reader_status->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
            this->_waitset->attach_condition(reader_status);
        }
    }

    ~RTIDynamicDataSubscriber()
    {
        this->Shutdown();
    }

    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        int seq_length;
        DDS_OctetSeq octetSeq;

        while (true) {
            // no outstanding reads
            if (this->_no_data)
            {
                this->_waitset->wait(this->_active_conditions, DDS_DURATION_INFINITE);

                if (this->_active_conditions.length() == 0)
                {
                    continue;
                }

                retcode = this->_reader->take(
                        this->_data_seq,
                        this->_info_seq,
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

                this->_data_idx = 0;
                this->_no_data = false;
            }

            seq_length = this->_data_seq.length();
            // check to see if hit end condition
            if (this->_data_idx == seq_length)
            {
                this->_reader->return_loan(this->_data_seq, this->_info_seq);
                this->_no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (this->_info_seq[_data_idx].valid_data == false) &&
                    (++this->_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (this->_data_idx == seq_length) {
                continue;
            }

            retcode = this->_data_seq[this->_data_idx].get_long(
                this->_message.entity_id,
                "entity_id",
                DynamicDataMembersId::GetInstance().at("entity_id"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(entity_id) failed: %d.\n",
                        retcode);
                this->_message.entity_id = 0;
            }
            retcode = this->_data_seq[this->_data_idx].get_ulong(
                this->_message.seq_num,
                "seq_num",
                DynamicDataMembersId::GetInstance().at("seq_num"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_ulong(seq_num) failed: %d.\n",
                        retcode);
                this->_message.seq_num = 0;
            }
            retcode = this->_data_seq[this->_data_idx].get_long(
                this->_message.timestamp_sec,
                "timestamp_sec",
                DynamicDataMembersId::GetInstance().at("timestamp_sec"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(timestamp_sec) failed: %d.\n",
                        retcode);
                this->_message.timestamp_sec = 0;
            }
            retcode = this->_data_seq[this->_data_idx].get_ulong(
                this->_message.timestamp_usec,
                "timestamp_usec",
                DynamicDataMembersId::GetInstance().at("timestamp_usec"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_ulong(timestamp_usec) failed: %d.\n",
                        retcode);
                this->_message.timestamp_usec = 0;
            }
            retcode = this->_data_seq[this->_data_idx].get_long(
                this->_message.latency_ping,
                "latency_ping",
                DynamicDataMembersId::GetInstance().at("latency_ping"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(latency_ping) failed: %d.\n",
                        retcode);
                this->_message.latency_ping = 0;
            }
            #ifdef RTI_CUSTOM_TYPE
            retcode = this->_data_seq[this->_data_idx].get_long(
                    this->_message.size,
                    "custom_type_size",
                    DynamicDataMembersId::GetInstance().at("custom_type_size"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "on_data_available() get_long(size) failed: %d.\n",
                        retcode);
                this->_message.size = 0;
            }
          #else
            retcode = this->_data_seq[this->_data_idx].get_octet_seq(
                    octetSeq,
                    "bin_data",
                    DynamicDataMembersId::GetInstance().at("bin_data"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_octet_seq(bin_data) failed: %d.\n",
                        retcode);
            }
            this->_message.size = octetSeq.length();
            this->_message.data = (char *)octetSeq.get_contiguous_buffer();
          #endif

            ++this->_data_idx;

            return &this->_message;
        }
    }
};

#endif //RTI_MICRO

/*********************************************************
 * configureDomainParticipantQos
 */
template <typename T>
bool RTIDDSImpl<T>::configureDomainParticipantQos(DDS_DomainParticipantQos &qos)
{
  #ifndef RTI_MICRO
    DDS_DomainParticipantFactoryQos factory_qos;

    // Setup the QOS profile file to be loaded
    _factory->get_qos(factory_qos);
    if (!_PM->get<bool>("noXmlQos")) {
        factory_qos.profile.url_profile.ensure_length(1, 1);
        factory_qos.profile.url_profile[0] =
                DDS_String_dup(_PM->get<std::string>("qosFile").c_str());
    } else {
        factory_qos.profile.string_profile.from_array(
                PERFTEST_QOS_STRING,
                PERFTEST_QOS_STRING_SIZE);
    }
    _factory->set_qos(factory_qos);

    if (_factory->reload_profiles() != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem opening QOS profiles file %s.\n",
                _PM->get<std::string>("qosFile").c_str());
        return false;
    }

    if (_factory->set_default_library(_PM->get<std::string>("qosLibrary").c_str())
            != DDS_RETCODE_OK) {
        fprintf(stderr,
                "No QOS Library named \"%s\" found in %s.\n",
                _PM->get<std::string>("qosLibrary").c_str(),
                _PM->get<std::string>("qosFile").c_str());
        return false;
    }

    // Configure DDSDomainParticipant QOS
    if (_factory->get_participant_qos_from_profile(
            qos,
            _PM->get<std::string>("qosLibrary").c_str(),
            "BaseProfileQos")
            != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem setting QoS Library \"%s::BaseProfileQos\" "
                "for participant_qos.\n",
                _PM->get<std::string>("qosLibrary").c_str());
    }

    // Set initial peers and not use multicast
    const std::vector<std::string> peerList =
            _PM->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        std::vector<char*> cstrings;
        cstrings.reserve(peerList.size());
        for(unsigned int i = 0; i < peerList.size(); ++i) {
            cstrings.push_back(const_cast<char*>(peerList[i].c_str()));
        }
        qos.discovery.initial_peers.from_array(
                (const char **)&cstrings[0],
                (long)peerList.size());
        qos.discovery.multicast_receive_addresses.length(0);
    }

    /* Mask for threadPriorities when it's used */
    int mask = Perftest_THREAD_SETTINGS_REALTIME_PRIORITY;
    ThreadPriorities threadPriorities = _parent->get_thread_priorities();

    // set thread priorities.
    if (threadPriorities.isSet) {

        // Set real time schedule
        qos.receiver_pool.thread.mask = mask;
        qos.event.thread.mask = mask;
        qos.database.thread.mask = mask;

        // Set priority
        qos.receiver_pool.thread.priority = threadPriorities.receive;
        qos.event.thread.priority = threadPriorities.dbAndEvent;
        qos.database.thread.priority = threadPriorities.dbAndEvent;
    }

    if (_PM->get<bool>("enableAutoThrottle")) {
        DDSPropertyQosPolicyHelper::add_property(qos.property,
                "dds.domain_participant.auto_throttle.enable",
                "true",
                false);
    }

  #else // RTI_MICRO

    RTRegistry *registry = _factory->get_registry();

    if (!registry->register_component(
            "wh",
            WHSMHistoryFactory::get_interface(),
            NULL,
            NULL)) {
        printf("Micro: Failed to register wh\n");
        return false;
    }

    if (!registry->register_component(
            "rh",
            RHSMHistoryFactory::get_interface(),
            NULL,
            NULL)) {
        printf("Micro: Failed to register rh\n");
        return false;
    }

    /* If the user provides the list of addresses */
    const std::vector<std::string> peerList =
        _PM->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        qos.discovery.initial_peers.maximum((int) peerList.size());
        qos.discovery.initial_peers.length((int) peerList.size());
        for(unsigned int i = 0; i < peerList.size(); ++i) {
            *qos.discovery.initial_peers.get_reference(i) =
                DDS_String_dup(peerList[i].c_str());
        }
    } else { /* Default discovery peers (unicast and multicast) */
        qos.discovery.initial_peers.maximum(2);
        qos.discovery.initial_peers.length(2);
        *qos.discovery.initial_peers.get_reference(0) =
                DDS_String_dup("239.255.0.1");
        *qos.discovery.initial_peers.get_reference(1) =
                DDS_String_dup("127.0.0.1");
    }
    qos.discovery.accept_unknown_peers = DDS_BOOLEAN_TRUE;

  #endif // RTI_MICRO

    if (!PerftestConfigureTransport(_transport, qos, _PM)) {
        return false;
    }

  #ifdef RTI_SECURE_PERFTEST
    // Configure security
    if (_PM->group_is_used(SECURE)) {
        // validate arguments
        if (!_security.validateSecureArgs()) {
            fprintf(stderr, "Failed to configure security plugins\n");
            return false;
        }
        // configure
        if (!PerftestConfigureSecurity(_security, qos, _PM)) {
            fprintf(stderr, "Failed to configure security plugins\n");
            return false;
        }
    }
  #endif // RTI_SECURE_PERFTEST

    return true;
}

/*********************************************************
 * Initialize
 */
template <typename T>
bool RTIDDSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    // Assign ParameterManager
    _PM = &PM;
    _transport.initialize(_PM);
  #ifdef RTI_SECURE_PERFTEST
    _security.initialize(_PM);
  #endif

    DDS_DomainParticipantQos qos;
    DDS_DomainParticipantFactoryQos factory_qos;
    DDS_PublisherQos publisherQoS;

    DomainListener *listener = new DomainListener();

    /* Mask for _threadPriorities when it's used */
    int mask = Perftest_THREAD_SETTINGS_REALTIME_PRIORITY;

    if (parent == NULL) {
        return false;
    }
    _parent = parent;
    ThreadPriorities threadPriorities = _parent->get_thread_priorities();

  #ifndef RTI_MICRO
    // Register _loggerDevice
    if (!NDDSConfigLogger::get_instance()->set_output_device(&_loggerDevice)) {
        fprintf(stderr,"Failed set_output_device for Logger.\n");
        return false;
    }
  #endif

    _factory = DDSDomainParticipantFactory::get_instance();

    if (!validate_input()) {
        return false;
    }

    // only if we run the latency test we need to wait
    // for pongs after sending pings
    _pongSemaphore = _PM->get<bool>("latencyTest") ?
            PerftestSemaphore_new() :
            NULL;

    if (!configureDomainParticipantQos(qos)) {
        return false;
    }

    // Creates the participant
    _participant = _factory->create_participant(
            (DDS_DomainId_t) _PM->get<int>("domain"),
            qos,
            listener,
            DDS_INCONSISTENT_TOPIC_STATUS |
            DDS_OFFERED_INCOMPATIBLE_QOS_STATUS |
            DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);

  #ifndef RTI_MICRO
    if (_participant == NULL || _loggerDevice.checkShmemErrors()) {
        if (_loggerDevice.checkShmemErrors()) {
            fprintf(stderr,
                    "[Error]: The participant creation failed due to issues in the Shared Memory configuration of your OS.\n"
                    "For more information about how to configure Shared Memory see: http://community.rti.com/kb/osx510 \n"
                    "If you want to skip the use of Shared memory in RTI Perftest, "
                    "specify the transport using \"-transport <kind>\", e.g. \"-transport UDPv4\".\n");
        }
        fprintf(stderr,"Problem creating participant.\n");
        return false;
    }
  #else
    if (_participant == NULL) {
        fprintf(stderr,"Problem creating participant.\n");
        return false;
    }
  #endif

  #ifndef RTI_MICRO
  #ifdef RTI_LEGACY_DD_IMPL
    // If we are using Dynamic Data, check if we want to use the new or old impl
    if (_PM->get<bool>("dynamicData") && _PM->get<bool>("useLegacyDynamicData")) {
        DDS_DynamicData_enable_legacy_impl();
    }
  #endif
  #endif

    /* Register the types and create the topics except for FlatData types,
     * They will be registered in  RTIDDSImpl_FlatData
     */
    if (!_isFlatData) {
        if (!_PM->get<bool>("dynamicData")) {
            T::TypeSupport::register_type(_participant, _typename);
        } else {
        #ifndef RTI_MICRO
            DDSDynamicDataTypeSupport* dynamicDataTypeSupportObject =
                    new DDSDynamicDataTypeSupport(
                            T::TypeSupport::get_typecode(),
                            DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
            dynamicDataTypeSupportObject->register_type(_participant, _typename);
        #else
            fprintf(stderr, "Dynamic data not supported in Micro.\n");
            return false;
        #endif
        }
    }


  #ifndef RTI_MICRO
    _factory->get_publisher_qos_from_profile(
            publisherQoS,
            _PM->get<std::string>("qosLibrary").c_str(),
            "BaseProfileQos");

    if (threadPriorities.isSet) {
        // Asynchronous thread priority
        publisherQoS.asynchronous_publisher.disable_asynchronous_write = false;
        publisherQoS.asynchronous_publisher.thread.mask = mask;
        publisherQoS.asynchronous_publisher.thread.priority
                = threadPriorities.main;
        // Asynchronous thread for batching priority
        publisherQoS.asynchronous_publisher.disable_asynchronous_batch = false;
        publisherQoS.asynchronous_publisher.asynchronous_batch_thread.mask = mask;
        publisherQoS.asynchronous_publisher.asynchronous_batch_thread.priority
                = threadPriorities.main;
    }

    // Create the DDSPublisher and DDSSubscriber
    _publisher = _participant->create_publisher(
            publisherQoS,
            NULL,
            DDS_STATUS_MASK_NONE);
  #else
    _publisher = _participant->create_publisher(
            DDS_PUBLISHER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
  #endif
    if (_publisher == NULL) {
        fprintf(stderr,"Problem creating publisher.\n");
        return false;
    }

  #ifndef RTI_MICRO
    _subscriber = _participant->create_subscriber_with_profile(
            _PM->get<std::string>("qosLibrary").c_str(),
            "BaseProfileQos",
            NULL,
            DDS_STATUS_MASK_NONE);
  #else
     _subscriber = _participant->create_subscriber(
            DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
   #endif
    if (_subscriber == NULL) {
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
     * If we are using reliable, the maximum burst of that we can send is
     * limited by max_send_window_size (or max samples, but we will assume this
     * is not the case for this). In such case we should send
     * max_send_window_size samples.
     *
     * If we are not using reliability this should not matter.
     */
    unsigned long initializeSampleCount = this->_sendQueueSize;

    /*
     * If we are using batching we need to take into account tha the Send Queue
     * will be per-batch, therefore for the number of samples:
     */
    if (_PM->get<long>("batchSize") > 0) {
        initializeSampleCount = (std::max)(
                _PM->get<int>("sendQueueSize") *
                        (_PM->get<long>("batchSize") /
                        _PM->get<unsigned long long>("dataLen")),
                (unsigned long long) initializeSampleCount);
    }

    return initializeSampleCount;
}

#ifndef RTI_MICRO
template <typename T>
double RTIDDSImpl<T>::obtain_dds_serialize_time_cost(
        unsigned int sampleSize,
        unsigned int iters)
{
    T data;
    double serializeTime = 0;
    double timeInit = 0;
    double timeFinish = 0;
    bool success = true;
    unsigned int sequenceSize = sampleSize - perftest_cpp::OVERHEAD_BYTES;

    unsigned int maxSizeSerializedSample = 0;
    char *buffer = NULL;
    char *serializeBuffer = NULL;

    RTIOsapiHeap_allocateBuffer(
            &buffer,
            sampleSize,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

    if (buffer == NULL) {
        fprintf(stderr,
                "Error allocating memory for buffer on "
                "obtain_dds_serialize_time_cost\n");
        return 0;
    }

    /* --- Initialize data --- */
    data.entity_id = 0;
    data.seq_num = 0;
    data.timestamp_sec = 0;
    data.timestamp_usec = 0;
    data.latency_ping = 0;
    data.bin_data.ensure_length(sequenceSize, sequenceSize);

    if (DDS_RETCODE_OK != T::TypeSupport::serialize_data_to_cdr_buffer(
                NULL, maxSizeSerializedSample, &data)) {
        fprintf(stderr,
                "Fail to serialize sample on obtain_dds_serialize_time_cost\n");
        if (buffer != NULL) {
            RTIOsapiHeap_freeBuffer(buffer);
        }
        return 0;
    }

    RTIOsapiHeap_allocateBuffer(
            &serializeBuffer,
            maxSizeSerializedSample,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

    /* Serialize time calculating */
    timeInit = (unsigned int) PerftestClock::getInstance().getTime();

    for (unsigned int i = 0; i < iters; i++) {
        if (DDS_RETCODE_OK != T::TypeSupport::serialize_data_to_cdr_buffer(
                serializeBuffer,
                maxSizeSerializedSample,
                &data)){
            fprintf(stderr,
                    "Fail to serialize sample on obtain_dds_serialize_time_cost\n");
            success = false;
        }
    }

    timeFinish = (unsigned int) PerftestClock::getInstance().getTime();

    serializeTime = timeFinish - timeInit;

    if (buffer != NULL) {
        RTIOsapiHeap_freeBuffer(buffer);
    }
    if (serializeBuffer != NULL) {
        RTIOsapiHeap_freeBuffer(serializeBuffer);
    }

    if (!success) {
        return 0;
    }

    return serializeTime / (float) iters;
}

template <typename T>
double RTIDDSImpl<T>::obtain_dds_deserialize_time_cost(
        unsigned int sampleSize,
        unsigned int iters)
{
    T data;
    double timeInit = 0;
    double timeFinish = 0;
    double deSerializeTime = 0;
    bool success = true;
    unsigned int sequenceSize = sampleSize - perftest_cpp::OVERHEAD_BYTES;

    unsigned int maxSizeSerializedSample = 0;
    char *buffer = NULL;
    char *serializeBuffer = NULL;

    RTIOsapiHeap_allocateBuffer(
            &buffer,
            sampleSize,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

    if (buffer == NULL) {
        fprintf(stderr,
                "Error allocating memory for buffer on "
                "obtain_dds_deserialize_time_cost\n");
        return 0;
    }

    /* --- Initialize data --- */
    data.entity_id = 0;
    data.seq_num = 0;
    data.timestamp_sec = 0;
    data.timestamp_usec = 0;
    data.latency_ping = 0;
    data.bin_data.ensure_length(sequenceSize, sequenceSize);

    if (DDS_RETCODE_OK != T::TypeSupport::serialize_data_to_cdr_buffer(
            NULL,
            maxSizeSerializedSample,
            &data)){
        fprintf(stderr,
                "Fail to serialize sample on obtain_dds_serialize_time_cost\n");
        if (buffer != NULL) {
            RTIOsapiHeap_freeBuffer(buffer);
        }
        return 0;
    }

    RTIOsapiHeap_allocateBuffer(
            &serializeBuffer,
            maxSizeSerializedSample,
            RTI_OSAPI_ALIGNMENT_DEFAULT);

    if (serializeBuffer == NULL) {
        fprintf(stderr,
                "Error allocating memory for buffer on "
                "obtain_dds_deserialize_time_cost\n");
        return 0;
    }

    if (DDS_RETCODE_OK != T::TypeSupport::serialize_data_to_cdr_buffer(
            serializeBuffer,
            maxSizeSerializedSample,
            &data)) {
        fprintf(stderr,
                "Fail to serialize sample on obtain_dds_deserialize_time_cost\n");
        return 0;
    }

    /* Deserialize time calculating */
    timeInit = (unsigned int) PerftestClock::getInstance().getTime();

    for (unsigned int i = 0; i < iters; i++) {
        if (DDS_RETCODE_OK != T::TypeSupport::deserialize_data_from_cdr_buffer(
                    &data,
                    serializeBuffer,
                    maxSizeSerializedSample)) {
            fprintf(stderr,
                    "Fail to deserialize sample on "
                    "obtain_dds_deserialize_time_cost\n");
            success = false;
        }
    }

    timeFinish = (unsigned int) PerftestClock::getInstance().getTime();

    deSerializeTime = timeFinish - timeInit;

    if (buffer != NULL) {
        RTIOsapiHeap_freeBuffer(buffer);
    }
    if (serializeBuffer != NULL) {
        RTIOsapiHeap_freeBuffer(serializeBuffer);
    }

    if (!success) {
        return 0;
    }

    return deSerializeTime / (float) iters;
}
#endif //RTI_MICRO

#ifndef RTI_MICRO
template <typename T>
unsigned long int RTIDDSImpl<T>::getShmemSHMMAX() {
    unsigned long int shmmax = 0;

  #ifdef RTI_DARWIN
    shmmax = MAX_DARWIN_SHMEM_SIZE;
    const char *cmd = "sysctl kern.sysv.shmmax";
    int buffSize = 100;
    char buffer[buffSize];
    FILE *file = NULL;

    // Execute cmd and get file pointer
    if ((file = popen(cmd, "r")) == NULL) {
        fprintf(stderr,
                "Could not run cmd '%s'. Using default size: %lu bytes.\n",
                cmd, shmmax);
        return shmmax;
    }

    // Read cmd output from its file pointer
    if (fgets(buffer, buffSize, file) == NULL) {
       fprintf(stderr,
                "Could not read '%s' output. Using default size: %lu bytes.\n",
                cmd, shmmax);
        return shmmax;
    }

    // Split cmd output by blankspaces and get second position
    strtok(buffer, " ");
    char *size = strtok(NULL, " ");
    shmmax = atoi(size);

    // Close file and process
    pclose(file);

  #else
    // NOT IMPLEMENTED OR NEEDED (YET)
  #endif

    return shmmax;
}
#endif // !RTI_MICRO

template <typename T>
bool RTIDDSImpl<T>::setup_DW_QoS(
        DDS_DataWriterQos &dw_qos,
        std::string qos_profile,
        std::string topic_name)
{

    #ifndef RTI_MICRO
    if (_factory->get_datawriter_qos_from_profile(
            dw_qos,
            _PM->get<std::string>("qosLibrary").c_str(),
            qos_profile.c_str())
            != DDS_RETCODE_OK) {
        fprintf(stderr,
                "No QOS Profile named \"%s\" found in QOS Library \"%s\" in file %s.\n",
                qos_profile.c_str(),
                _PM->get<std::string>("qosLibrary").c_str(),
                _PM->get<std::string>("qosFile").c_str());
        return false;
    }

    if (_PM->get<bool>("noPositiveAcks")
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dw_qos.protocol.disable_positive_acks = true;
        if (_PM->is_set("keepDurationUsec")) {
            dw_qos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration =
                    DDS_Duration_t::from_micros(
                            _PM->get<unsigned long long>("keepDurationUsec"));
        }
    }

    if ((_isLargeData && !_isZeroCopy) || _PM->get<bool>("asynchronous")) {
        dw_qos.publish_mode.kind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
        if (_PM->get<std::string>("flowController") != "default") {
            dw_qos.publish_mode.flow_controller_name =
                    DDS_String_dup(("dds.flow_controller.token_bucket." +
                    _PM->get<std::string>("flowController")).c_str());
        }
    }
  #endif

    // Only force reliability on throughput/latency topics
    if (strcmp(topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (!_PM->get<bool>("bestEffort")) {
            // default: use the setting specified in the qos profile
            // dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
          #ifdef RTI_MICRO
            dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
          #endif
        }
        else {
            // override to best-effort
            dw_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

  #ifdef RTI_MICRO
    if (strcmp(topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) == 0) {
        dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        dw_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    }
  #endif

    // These QOS's are only set for the Throughput datawriter
    if (qos_profile == "ThroughputQos") {

      #ifndef RTI_MICRO
        if (_PM->get<bool>("multicast")) {
            dw_qos.protocol.rtps_reliable_writer.enable_multicast_periodic_heartbeat =
                    RTI_TRUE;
        }

        if (_PM->get<long>("batchSize") > 0) {
            dw_qos.batch.enable = true;
            dw_qos.batch.max_data_bytes = _PM->get<long>("batchSize");
            dw_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos.writer_resource_limits.max_batches =
                    _PM->get<int>("sendQueueSize");
        } else {
            dw_qos.resource_limits.max_samples = _PM->get<int>("sendQueueSize");
        }
      #else
        dw_qos.resource_limits.max_samples = _PM->get<int>("sendQueueSize");
        this->_sendQueueSize = dw_qos.resource_limits.max_samples;
      #endif

      #ifndef RTI_MICRO
        if (_PM->get<bool>("enableAutoThrottle")) {
            DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
                    "dds.data_writer.auto_throttle.enable", "true", false);
        }

        if (_PM->get<bool>("enableTurboMode")) {
            DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
                    "dds.data_writer.enable_turbo_mode",
                    "true",
                    false);
            dw_qos.batch.enable = false;
            dw_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos.writer_resource_limits.max_batches =
                    _PM->get<int>("sendQueueSize");
        }

        dw_qos.resource_limits.initial_samples = _PM->get<int>("sendQueueSize");
        this->_sendQueueSize = dw_qos.resource_limits.initial_samples;
      #endif
        dw_qos.resource_limits.max_samples_per_instance =
                dw_qos.resource_limits.max_samples;

        dw_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind)_PM->get<int>("durability");

      #ifndef RTI_MICRO
        dw_qos.durability.direct_communication =
                !_PM->get<bool>("noDirectCommunication");
      #endif


        dw_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                _PM->get<int>("sendQueueSize") / 10;
      #ifndef RTI_MICRO
        dw_qos.protocol.rtps_reliable_writer.low_watermark =
                _PM->get<int>("sendQueueSize") * 1 / 10;
        dw_qos.protocol.rtps_reliable_writer.high_watermark =
                _PM->get<int>("sendQueueSize") * 9 / 10;

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
                _PM->get<int>("sendQueueSize");
        dw_qos.protocol.rtps_reliable_writer.min_send_window_size =
                _PM->get<int>("sendQueueSize");
      #else
        #if RTI_MICRO_24x_COMPATIBILITY
          // Keep all not supported in Micro 2.4.x
          dw_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
        #else
          dw_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        #endif
        dw_qos.history.depth = _PM->get<int>("sendQueueSize");
        // Same values we use for Pro (See perftest_qos_profiles.xml).
        dw_qos.protocol.rtps_reliable_writer.heartbeat_period.sec = 0;
        dw_qos.protocol.rtps_reliable_writer.heartbeat_period.nanosec = 10000000;
      #endif
    }

    if (qos_profile == "LatencyQos"
            && _PM->get<bool>("noDirectCommunication")
            && (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
            || _PM->get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS)) {
        dw_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind)_PM->get<int>("durability");
      #ifndef RTI_MICRO
        dw_qos.durability.direct_communication =
                !_PM->get<bool>("noDirectCommunication");
      #endif
    }

    dw_qos.resource_limits.max_instances =
            _PM->get<long>("instances") + 1; // One extra for MAX_CFT_VALUE
  #ifndef RTI_MICRO
    dw_qos.resource_limits.initial_instances = _PM->get<long>("instances") + 1;

    // If is LargeData
    if (_PM->get<int>("unbounded") != 0) {
        char buf[10];
        sprintf(buf, "%d", (_isFlatData
                ? DDS_LENGTH_UNLIMITED // No dynamic alloc of serialize buffer
                : _PM->get<int>("unbounded")));
        DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
               "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
               buf,
               false);
    }

    if (_PM->get<long>("instances") > 1) {
        if (_PM->is_set("instanceHashBuckets")) {
            dw_qos.resource_limits.instance_hash_buckets =
                    _PM->get<long>("instanceHashBuckets");
        } else {
            dw_qos.resource_limits.instance_hash_buckets =
                    _PM->get<long>("instances");
        }
    }
  #endif

  #ifdef RTI_FLATDATA_AVAILABLE
    if (_isFlatData) {
         /**
         * If FlatData and LargeData, automatically estimate initial_samples
         * here in a range from 1 up to the initial samples specifies in the QoS
         * file.
         *
         * This is done to avoid using too much memory since DDS allocates
         * samples of the RTI_FLATDATA_MAX_SIZE size
         */
        if (_isLargeData) {
            unsigned long max_allocable_space = MAX_PERFTEST_SAMPLE_SIZE;

          #ifdef RTI_DARWIN
            /**
             * In OSX, we might not be able to allocate all the send queue
             * samples We only need this on the DW since it will allocate the
             * samples on Zero Copy
             */
            if (_isZeroCopy) {
                max_allocable_space = getShmemSHMMAX();

                /**
                 * Leave enought room for an sceneario of two participants:
                 *   - One Publisher with one DW (throughput topic)
                 *   - One Subscriber with two DW (Latency topic and
                 *     Announcement)
                 */
                max_allocable_space /= 3;

                /**
                 * If we wont be able to allocate as many samples as we
                 * originally want, Display a message letting know the user how
                 * to increase SHMEM operative system settings
                 */
                if (max_allocable_space < RTI_FLATDATA_MAX_SIZE *
                            dw_qos.resource_limits.initial_samples + 1) {
                    fprintf(stderr,
                            "[Warn] Performace Degradation: Not enought Shared "
                            "Memory space available to allocate intial samples. "
                            "Consider increasing SHMMAX parameter on your system"
                            " settings or select a different transport.\n"
                            "See https://community.rti.com/kb/what-are-possible-solutions-common-shared-memory-issues\n"
                            "If you still run into this issue, consider cleaning "
                            "your Shared Memory segments.\n");
                }
            }
          #endif

            // The writer_loaned_sample_allocation is initial_simples + 1
            unsigned long long initial_samples = (std::max)(
                    1ul, (max_allocable_space - RTI_FLATDATA_MAX_SIZE) / RTI_FLATDATA_MAX_SIZE);

            initial_samples = (std::min)(
                    initial_samples,
                    (unsigned long long) dw_qos.resource_limits.initial_samples);

            dw_qos.resource_limits.initial_samples = initial_samples;
            this->_sendQueueSize = initial_samples;

            if (_transport.transportConfig.kind == TRANSPORT_SHMEM
                    || _transport.transportConfig.kind == TRANSPORT_UDPv4_SHMEM) {
                /**
                 * Replace previously set resource limits by the new ones from the
                 * initial_samples size calculations
                 */
                dw_qos.resource_limits.max_samples = 2 * initial_samples;
                dw_qos.resource_limits.max_samples_per_instance =
                        dw_qos.resource_limits.max_samples;
                dw_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                        (std::max)(1.0, 0.1 * dw_qos.resource_limits.max_samples);
                dw_qos.protocol.rtps_reliable_writer.high_watermark =
                        0.9 * dw_qos.resource_limits.max_samples;
                dw_qos.protocol.rtps_reliable_writer.low_watermark =
                        0.1 * dw_qos.resource_limits.max_samples;

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
            }
        } else {
            /**
             * Avoid "DDS_DataWriter_get_loan_untypedI:ERROR: Out of resources
             * for writer loaned samples" error on small data due to not
             * having enought samples on the writer buffer where FlatData loans
             * samples from.
             */
            dw_qos.writer_resource_limits.writer_loaned_sample_allocation.
                initial_count = 2 * dw_qos.resource_limits.initial_samples;
            dw_qos.writer_resource_limits.writer_loaned_sample_allocation.
                max_count = 1 + dw_qos.writer_resource_limits.
                        writer_loaned_sample_allocation.initial_count;
        }

        /**
         * Enables a ZeroCopy DataWriter to send a special sequence number as a
         * part of its inline Qos. his sequence number is used by a ZeroCopy
         * DataReader to check for sample consistency.
         */
        if (_isZeroCopy) {
            dw_qos.transfer_mode.shmem_ref_settings.enable_data_consistency_check =
                    RTI_TRUE;
        }
    }
  #endif

    return true;
}

template <typename T>
bool RTIDDSImpl<T>::setup_DR_QoS(
        DDS_DataReaderQos &dr_qos,
        std::string qos_profile,
        std::string topic_name)
{

    #ifndef RTI_MICRO
    if (_factory->get_datareader_qos_from_profile(
            dr_qos,
            _PM->get<std::string>("qosLibrary").c_str(),
            qos_profile.c_str())
            != DDS_RETCODE_OK) {
        fprintf(stderr,
                "No QOS Profile named \"%s\" found in QOS Library "
                "\"%s\" in file %s.\n",
                qos_profile.c_str(),
                _PM->get<std::string>("qosLibrary").c_str(),
                _PM->get<std::string>("qosFile").c_str());
        return false;
    }
  #endif

    // Only force reliability on throughput/latency topics
    if (strcmp(topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (!_PM->get<bool>("bestEffort")) {
            dr_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        } else {
            dr_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

  #ifndef RTI_MICRO
    if (_PM->get<bool>("noPositiveAcks")
            && (qos_profile == "ThroughputQos"
            || qos_profile == "LatencyQos")) {
        dr_qos.protocol.disable_positive_acks = true;
    }
  #endif

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos"
            || (qos_profile == "LatencyQos"
            && _PM->get<bool>("noDirectCommunication")
            && (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
            || _PM->get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS))) {
        dr_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind) _PM->get<int>("durability");
      #ifndef RTI_MICRO
        dr_qos.durability.direct_communication =
                !_PM->get<bool>("noDirectCommunication");
      #endif

    }

  #ifdef RTI_MICRO

    if (qos_profile == "ThroughputQos") {
        /*
         * For Connext DDS Pro settings are set so initial samples are set to
         * a lower value than max_samples, so we can grow if needed. For micro
         * however we do not have the initial_samples parameter, therefore we
         * must choose a value for max_samples since the beginning. We chose to
         * use 10000. This value should be large enough to handle most of the
         * communications.
         *
         * We could potentially modify this with a new command line parameter
         */
        if (_PM->get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
            dr_qos.resource_limits.max_samples = 50;
            dr_qos.resource_limits.max_samples_per_instance = 50;
            dr_qos.history.depth = 50;
        }
        else {
            dr_qos.resource_limits.max_samples = 5000;
            dr_qos.resource_limits.max_samples_per_instance = 5000;
            dr_qos.history.depth = 5000;
        }
        /*
         * In micro 2.4.x we don't have keep all, this means we need to set the
         * history to keep last and chose a history depth. For the depth value
         * we can we same value as max_samples
         */
        #if RTI_MICRO_24x_COMPATIBILITY
          // Keep all not supported in Micro 2.4.x
          dr_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
        #else
          dr_qos.history.kind = DDS_KEEP_ALL_HISTORY_QOS;
        #endif

    } else { // "LatencyQos" or "AnnouncementQos"

        /*
         * By default Micro will use a max_samples = 1. This is too low for the
         * initial burst of data. In pro we set this value via QoS to
         * LENGTH_UNLIMITED. In Micro we will use a lower number due to
         * memory restrictions.
         */
        if (_PM->get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
            dr_qos.resource_limits.max_samples = 50;
        }
        else {
            dr_qos.resource_limits.max_samples = 1000;
        }
    }

    /*
     * We could potentially use here the number of subscriber, right now this
     * class does not have access to the number of subscriber though.
     */
    dr_qos.reader_resource_limits.max_remote_writers = 50;
    dr_qos.reader_resource_limits.max_remote_writers_per_instance = 50;
  #endif

  #ifndef RTI_MICRO
    dr_qos.resource_limits.initial_instances = _PM->get<long>("instances") + 1;
    if (_instanceMaxCountReader != DDS_LENGTH_UNLIMITED) {
        _instanceMaxCountReader++;
    }
  #else
    /*
     * In micro we cannot have UNLIMITED instances, this means that we need to
     * increase the InstanceMaxCountReader (max instances for the dr) in all
     * cases
     */
    _instanceMaxCountReader++;
  #endif
    dr_qos.resource_limits.max_instances = _instanceMaxCountReader;

  #ifndef RTI_MICRO
    if (_PM->get<long>("instances") > 1) {
        if (_PM->is_set("instanceHashBuckets")) {
            dr_qos.resource_limits.instance_hash_buckets =
                    _PM->get<long>("instanceHashBuckets");
        } else {
            dr_qos.resource_limits.instance_hash_buckets =
                    _PM->get<long>("instances");
        }
    }

    if (_PM->get<bool>("multicast") && _transport.allowsMulticast()) {
        dr_qos.multicast.value.ensure_length(1, 1);
        dr_qos.multicast.value[0].receive_address = DDS_String_dup(
                _transport.getMulticastAddr(topic_name.c_str()).c_str());

        if (dr_qos.multicast.value[0].receive_address == NULL) {
            fprintf(stderr,
                    "topic name must either be %s or %s or %s.\n",
                    THROUGHPUT_TOPIC_NAME,
                    LATENCY_TOPIC_NAME,
                    ANNOUNCEMENT_TOPIC_NAME);
            return false;
        }

        dr_qos.multicast.value[0].receive_port = 0;
        dr_qos.multicast.value[0].transports.length(0);
    }

    #ifdef RTI_FLATDATA_AVAILABLE
    if (_isFlatData) {
        char buf[10];
        sprintf(buf, "%d", DDS_LENGTH_UNLIMITED);
        DDSPropertyQosPolicyHelper::add_property(dr_qos.property,
                "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                buf, false);

        if (_isLargeData) {
            int max_allocable_space = MAX_PERFTEST_SAMPLE_SIZE;

            // The writer_loaned_sample_allocation is initial_simples + 1
            unsigned long long initial_samples = (std::max)(
                    1,
                    (max_allocable_space -
                            RTI_FLATDATA_MAX_SIZE) / RTI_FLATDATA_MAX_SIZE);

            initial_samples = (std::min)(
                    initial_samples,
                    (unsigned long long) dr_qos.resource_limits.initial_samples);

            dr_qos.resource_limits.initial_samples = initial_samples;

            /**
             * Since for ZeroCopy we are sending small data (16B reference),
             * we do not need these settings
             */
            if (!_isZeroCopy) {
                dr_qos.resource_limits.max_samples = initial_samples;
                dr_qos.resource_limits.max_samples_per_instance = initial_samples;
                dr_qos.reader_resource_limits.max_samples_per_remote_writer = initial_samples;
            }
        }
    }
    #endif

    /**
     * Configure DataReader to prevent dynamic allocation of
     * buffer used for storing received fragments
     */
    if (_PM->get<bool>("preallocateFragmentedSamples")) {
        dr_qos.reader_resource_limits.initial_fragmented_samples = 1;
        dr_qos.reader_resource_limits.dynamically_allocate_fragmented_samples =
            DDS_BOOLEAN_FALSE;
    }
  #endif


    if (_PM->get<int>("unbounded") != 0 && !_isFlatData) {
      #ifndef RTI_MICRO
        char buf[10];
        sprintf(buf, "%d", _PM->get<int>("unbounded"));
        DDSPropertyQosPolicyHelper::add_property(dr_qos.property,
                "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                buf, false);
      #else
        /* This is only needed for Micro 2.4.x. False unbounded sequences are
         * available in Micro 3.0 */
        #if RTI_MICRO_24x_COMPATIBILITY
          fprintf(stderr,
                  "Unbounded sequences not supported on Micro.\n");
          return false;
        #endif
      #endif
    }

    return true;
}


/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *RTIDDSImpl<T>::CreateWriter(const char *topic_name)
{
    DDS_DataWriterQos dw_qos;
    DDSDataWriter *writer = NULL;
    std::string qos_profile = "";
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

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return NULL;
    }

    if (!setup_DW_QoS(dw_qos, qos_profile, topic_name)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
        return NULL;
    }

    writer = _publisher->create_datawriter(
        topic, dw_qos, NULL,
        DDS_STATUS_MASK_NONE);

    if (writer == NULL)
    {
        fprintf(stderr,"Problem creating writer.\n");
        return NULL;
    }

    if (!_PM->get<bool>("dynamicData")) {
        try {
            return new RTIPublisher<T>(
                    writer,
                    _PM->get<long>("instances"),
                    _pongSemaphore,
                    _PM->get<long>("writeInstance"),
                    _PM);
        } catch (const std::exception &ex) {
            fprintf(stderr,
                    "Exception in RTIDDSImpl<T>::CreateWriter(): %s.\n", ex.what());
            return NULL;
        }
    } else {
      #ifndef RTI_MICRO
        try{
            return new RTIDynamicDataPublisher(
                    writer,
                    _PM->get<long>("instances"),
                    _pongSemaphore,
                    T::TypeSupport::get_typecode(),
                    _PM->get<long>("writeInstance"),
                    _PM);
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in RTIDDSImpl<T>::CreateWriter(): %s.\n", ex.what());
            return NULL;
        }
      #else
        fprintf(stderr,"Dynamic data not supported on Micro.\n");
        return NULL;
      #endif
    }

}

#ifdef RTI_FLATDATA_AVAILABLE
template <typename T>
IMessagingWriter *RTIDDSImpl_FlatData<T>::CreateWriter(const char *topic_name)
{
    DDS_DataWriterQos dw_qos;
    DDSDataWriter *writer = NULL;
    std::string qos_profile = "";

    /* Since we have to instantiate RTIDDSImpl<T> class
     * with T=TestData_t, we have to register the FlatData
     * type here.
     */
    T::TypeSupport::register_type(_participant, _typename);

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

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return NULL;
    }

    if (!setup_DW_QoS(dw_qos, qos_profile, topic_name)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
        return NULL;
    }

    writer = _publisher->create_datawriter(topic, dw_qos, NULL,
            DDS_STATUS_MASK_NONE);

    if (writer == NULL) {
        fprintf(stderr,"Problem creating writer.\n");
        return NULL;
    }

    try {
        return new RTIFlatDataPublisher<T>(
                writer,
                _PM->get<long>("instances"),
                _pongSemaphore,
                _PM->get<long>("writeInstance"),
                _PM);
    } catch (const std::exception &ex) {
        fprintf(stderr,
                "Exception in RTIDDSImpl_FlatData<T>::CreateWriter(): %s.\n", ex.what());
        return NULL;
    }
}
#endif

#ifndef RTI_MICRO
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
    /*
     * The Key 255,255,0,0 match the internal messages, we do not want
     * to block those messages, so the instance key will be added to the
     * condition.
     */
    std::string condition;
    DDS_StringSeq parameters(2 * KEY_SIZE);
    const std::vector<unsigned long long> cftRange =
            _PM->get_vector<unsigned long long>("cft");

    /* Reserve memory for the range case, the bigger one. */
    char cft_param[2 * KEY_SIZE][128];
    const char* param_list[] = { cft_param[0], cft_param[1],
            cft_param[2], cft_param[3],cft_param[4],
            cft_param[5], cft_param[6], cft_param[7]
    };

    /* Only one element, no range */
    if (cftRange.size() == 1) {
        printf("CFT enabled for instance: '%llu' \n", cftRange[0]);

        for (int i = 0; i < KEY_SIZE ; i++) {
            sprintf(cft_param[i], "%d", (unsigned char)(cftRange[0] >> i * 8));
        }

        parameters.from_array(param_list, KEY_SIZE);
        condition = "(%0 = key[0] AND %1 = key[1] AND %2 = key[2] AND %3 = key[3]) OR"
                "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";

    } else { /* More than one element, apply a range filter */
        printf("CFT enabled for instance range: [%llu,%llu] \n",
                cftRange[0],
                cftRange[1]);

        for (unsigned int i = 0; i < 2 * KEY_SIZE ; i++ ) {
            sprintf(cft_param[i], "%d", (unsigned char)
                    (cftRange[ i < KEY_SIZE? 0 : 1] >> (i % KEY_SIZE) * 8));
        }

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
#endif //RTI_MICRO

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
    std::string qos_profile = "";
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

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return NULL;
    }

    if (!setup_DR_QoS(dr_qos, qos_profile, topic_name)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
        return NULL;
    }

  #ifndef RTI_MICRO
    /* Create CFT Topic */
    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0 && _PM->is_set("cft")) {
        topic_desc = CreateCft(topic_name, topic);
        if (topic_desc == NULL) {
            printf("Create_contentfilteredtopic error\n");
            return NULL;
        }
    }
  #endif

    if (callback != NULL) {
        if (!_PM->get<bool>("dynamicData")) {
            reader = _subscriber->create_datareader(
                    topic_desc,
                    dr_qos,
                    new ReceiverListener<T>(callback),
                    DDS_DATA_AVAILABLE_STATUS);
        } else {
          #ifndef RTI_MICRO
            reader = _subscriber->create_datareader(
                    topic_desc,
                    dr_qos,
                    new DynamicDataReceiverListener(callback),
                    DDS_DATA_AVAILABLE_STATUS);
          #else
            fprintf(stderr,"Dynamic data not supported on Micro.\n");
            return NULL;
          #endif
        }

    } else {
        reader = _subscriber->create_datareader(
                topic_desc,
                dr_qos,
                NULL,
                DDS_STATUS_MASK_NONE);
    }

    if (reader == NULL) {
        fprintf(stderr, "Problem creating reader.\n");
        return NULL;
    }

    if (!strcmp(topic_name, THROUGHPUT_TOPIC_NAME) ||
        !strcmp(topic_name, LATENCY_TOPIC_NAME)) {
        _reader = reader;
    }

    if (!_PM->get<bool>("dynamicData")) {
        return new RTISubscriber<T>(reader, _PM);
    } else {
      #ifndef RTI_MICRO
        return new RTIDynamicDataSubscriber<T>(reader, _PM);
      #else
        fprintf(stderr,"Dynamic data not supported on Micro.\n");
        return NULL;
      #endif
    }
}

template <typename T>
const std::string RTIDDSImpl<T>::get_qos_profile_name(const char *topicName)
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

#ifdef RTI_FLATDATA_AVAILABLE
template <typename T>
IMessagingReader *RTIDDSImpl_FlatData<T>::CreateReader(
        const char *topic_name,
        IMessagingCB *callback)
{
    DDS_DataReaderQos dr_qos;
    DDSDataReader *reader = NULL;
    std::string qos_profile = "";
    DDSTopicDescription* topic_desc = NULL; // Used to create the DDS DataReader

    /* Since we have to instantiate RTIDDSImpl<T> class
     * with T=TestData_t, we have to register the FlatData
     * type here.
     */
    T::TypeSupport::register_type(_participant, _typename);

    DDSTopic *topic = _participant->create_topic(
                       topic_name, _typename,
                       DDS_TOPIC_QOS_DEFAULT, NULL,
                       DDS_STATUS_MASK_NONE);

    if (topic == NULL) {
        fprintf(stderr,"Problem creating topic %s.\n", topic_name);
        return NULL;
    }
    topic_desc = topic;

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return NULL;
    }

    if (!setup_DR_QoS(dr_qos, qos_profile, topic_name)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
        return NULL;
    }

  #ifndef RTI_MICRO
    /* Create CFT Topic */
    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0 && _PM->is_set("cft")) {
        topic_desc = CreateCft(topic_name, topic);
        if (topic_desc == NULL) {
            printf("Create_contentfilteredtopic error\n");
            return NULL;
        }
    }
  #endif

    if (callback != NULL) {
        reader = _subscriber->create_datareader(
                topic_desc,
                dr_qos,
                new FlatDataReceiverListener<T>(
                        callback,
                        _PM->get<bool>("zerocopy"),
                        _PM->get<bool>("checkconsistency")),
                DDS_DATA_AVAILABLE_STATUS);
    } else {
        reader = _subscriber->create_datareader(
                topic_desc,
                dr_qos,
                NULL,
                DDS_STATUS_MASK_NONE);
    }

    if (reader == NULL) {
        fprintf(stderr, "Problem creating reader.\n");
        return NULL;
    }

    if (!strcmp(topic_name, THROUGHPUT_TOPIC_NAME) ||
        !strcmp(topic_name, LATENCY_TOPIC_NAME)) {
        _reader = reader;
    }

    return new RTIFlatDataSubscriber<T>(reader, _PM);
}

template <typename T>
double RTIDDSImpl_FlatData<T>::obtain_dds_serialize_time_cost_override(
        unsigned int sampleSize,
        unsigned int iters)
{
    typedef typename rti::flat::flat_type_traits<T>::builder Builder;
    typedef typename rti::flat::PrimitiveSequenceBuilder<unsigned char> BinDataBuilder;

    unsigned long int serializedSize = 68 + RTI_FLATDATA_MAX_SIZE;
    unsigned char *buffer = new unsigned char[serializedSize];
    double total_time = 0.0;

    for (unsigned int i = 0; i < iters; i++) {
        Builder builder(buffer, serializedSize);

        // leave uninitialized
        builder.add_key();
        builder.add_entity_id(0);
        builder.add_seq_num(0);
        builder.add_timestamp_sec(0);
        builder.add_timestamp_usec(0);
        builder.add_latency_ping(0);

        // Add payload
        BinDataBuilder bin_data = builder.build_bin_data();
        bin_data.add_n(sampleSize);
        bin_data.finish();

        double start = (unsigned int) PerftestClock::getInstance().getTime();
        builder.finish_sample();
        double end = (unsigned int) PerftestClock::getInstance().getTime();
        total_time += end - start;
    }

    delete[] buffer;

    return total_time / (float) iters;

    return 0.0;
}

template <typename T>
double RTIDDSImpl_FlatData<T>::obtain_dds_deserialize_time_cost_override(
        unsigned int sampleSize,
        unsigned int iters)
{
    typedef typename rti::flat::flat_type_traits<T>::builder Builder;
    typedef typename rti::flat::PrimitiveSequenceBuilder<unsigned char> BinDataBuilder;

    unsigned long int serializedSize = 68 + RTI_FLATDATA_MAX_SIZE;
    unsigned char *buffer = new unsigned char[serializedSize];

    Builder builder(buffer, serializedSize);

    // Leave uninitialized
    builder.add_key();
    builder.add_entity_id(0);
    builder.add_seq_num(0);
    builder.add_timestamp_sec(0);
    builder.add_timestamp_usec(0);
    builder.add_latency_ping(0);

    // Add payload
    BinDataBuilder bin_data = builder.build_bin_data();
    bin_data.add_n(sampleSize);
    bin_data.finish();

    builder.finish_sample();

    double start = (unsigned int) PerftestClock::getInstance().getTime();
    for (unsigned int i = 0; i < iters; i++) {
        T::from_buffer(buffer);
    }
    double end = (unsigned int) PerftestClock::getInstance().getTime();

    delete[] buffer;

    return (end-start) / (float) iters;
}
#endif

/*
 * This instantiation is to avoid a undefined reference of a templated static
 * function of obtain_dds_de/serialize_time_costs.
 */
template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;
template class RTIDDSImpl<TestDataKeyedLarge_t>;
template class RTIDDSImpl<TestDataLarge_t>;

#ifdef RTI_FLATDATA_AVAILABLE
  template class RTIDDSImpl_FlatData<TestDataKeyed_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestData_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataKeyedLarge_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataLarge_FlatData_t>;
  #ifdef RTI_ZEROCOPY_AVAILABLE
  template class RTIDDSImpl_FlatData<TestDataKeyed_ZeroCopy_w_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestData_ZeroCopy_w_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataKeyedLarge_ZeroCopy_w_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataLarge_ZeroCopy_w_FlatData_t>;
  #endif
#endif // RTI_FLATDATA_AVAILABLE

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(pop)
#endif
