/*
 * (c) 2023-2024  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "RTICertImpl.h"

#include <ifaddrs.h>
#include <arpa/inet.h>

/**********************************
 * Free standing helper functions *
 **********************************/

#define CHECK_PTR(ptr, msg) \
    if (ptr == NULL) { \
        fprintf(stderr, "%s:%d: Error %s.\n", \
        __FUNCTION__, __LINE__, msg); \
        fflush(stderr); \
        goto done; \
    }


#define CHECK_BOOL(b, msg) \
    if (b == RTI_FALSE) { \
        fprintf(stderr, "%s:%d: Error %s.\n", \
        __FUNCTION__, __LINE__, msg); \
        fflush(stderr); \
        goto done; \
    }

#define CHECK_RETCODE(rc, msg) \
    if (rc != DDS_RETCODE_OK) { \
        fprintf(stderr, "%s:%d: Error %s (%d).\n", \
        __FUNCTION__, __LINE__, msg, rc); \
        fflush(stderr); \
        goto done; \
    }

// THis definition is only needed for Platform Independent Cert
#if !defined(BUILD_CERT_WITH_REGULAR_MICRO) && defined(RTI_CERT_IS_PI)
const char* const NETIO_DEFAULT_UDP_NAME = "_udp";
#endif

// Using an anonymous namespace to make sure that these functions are not
// used outside of this file
namespace {
#ifdef RTI_CERT
    DDS_Octet* DDS_OctetSeq_get_contiguous_buffer(const struct DDS_OctetSeq *self);

    RTI_BOOL DDS_OctetSeq_loan_contiguous(struct DDS_OctetSeq *self, void *buffer,
                        RTI_INT32 new_length, RTI_INT32 new_max);

    RTI_BOOL DDS_OctetSeq_unloan(struct DDS_OctetSeq *self);
#endif

template<typename T, typename TSeq>
T* DDS_TypedSampleSeq_get_reference(TSeq* self, RTI_INT32 i);

RTI_UINT32 get_interface_address(const char *interface_name);

bool configureUDPv4Transport(
        RT_Registry_T *registry,
        PerftestTransport &transport,
        ParameterManager *_PM);

void configureDPResourceLimits(
    DDS_DomainParticipantQos& dp_qos);
}

const std::string GetMiddlewareVersionString()
{
    return "RTI Connext DDS CERT "
        + perftest::to_string((int) RTIME_DDS_VERSION_MAJOR) + "."
        + perftest::to_string((int) RTIME_DDS_VERSION_MINOR) + "."
        + perftest::to_string((int) RTIME_DDS_VERSION_REVISION) + "."
        + perftest::to_string((int) RTIME_DDS_VERSION_RELEASE);
}

/*****************************************
 * End of free standing helper functions *
 *****************************************/

/****************************
 * Listeners for DataReader *
 ****************************/
/******************
 * ReaderListener *
 ******************/
template <typename T, typename TSeq>
static void on_data_available(void *listener_data, DDS_DataReader *reader)
{
    if (listener_data == NULL) {
        fprintf(stderr,"Error: listener_data is NULL\n");
        return;
    }

    TSeq samples = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq sampleInfo = DDS_SEQUENCE_INITIALIZER;
    TestMessage message;
    IMessagingCB *callback = (IMessagingCB *) listener_data;

    DDS_ReturnCode_t retcode = DDS_DataReader_take(
            reader,
            (struct DDS_UntypedSampleSeq *) &samples,
            &sampleInfo,
            DDS_LENGTH_UNLIMITED,
            DDS_ANY_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE,
            DDS_ANY_INSTANCE_STATE);
    if (retcode == DDS_RETCODE_NO_DATA)
    {
        return;
    }
    else if (retcode != DDS_RETCODE_OK)
    {
        fprintf(stderr,"Error during taking data in listener %d\n", retcode);
        return;
    }

    int seq_length = DDS_SampleInfoSeq_get_length(&sampleInfo);
    for (int i = 0; i < seq_length; ++i)
    {
        if (DDS_SampleInfoSeq_get_reference(&sampleInfo, i)->valid_data == DDS_BOOLEAN_TRUE)
        {
            T *sample = DDS_TypedSampleSeq_get_reference<T, TSeq>(&samples,i);

            message.entity_id = sample->entity_id;
            message.seq_num = sample->seq_num;
            message.timestamp_sec = sample->timestamp_sec;
            message.timestamp_usec = sample->timestamp_usec;
            message.latency_ping = sample->latency_ping;

            message.size = DDS_OctetSeq_get_length(&sample->bin_data);
            message.data = (char *) DDS_OctetSeq_get_contiguous_buffer(&sample->bin_data);

            callback->process_message(message);
        }
    }
    retcode = DDS_DataReader_return_loan(
            reader,
            (struct DDS_UntypedSampleSeq *) &samples,
            &sampleInfo);
    if (retcode != DDS_RETCODE_OK)
    {
        fprintf(stderr,"Error during return loan in listener %d.\n", retcode);
        fflush(stderr);
    }
}

template <typename T, typename TSeq>
class ReaderListenerBase {
public:
    DDS_DataReaderListener *get_listener()
    {
        return &this->dr_listener;
    }

protected:
    struct DDS_DataReaderListener dr_listener;
};

template <typename T, typename TSeq>
class ReaderListener : public ReaderListenerBase<T, TSeq> {
public:
    ReaderListener(IMessagingCB *callback)
    {
        this->dr_listener = DDS_DataReaderListener_INITIALIZER;
        this->dr_listener.as_listener.listener_data = (void *) callback;
        this->dr_listener.on_data_available = &on_data_available<T, TSeq>;
    }
};
/*************************
 * End of ReaderListener *
 *************************/

/***********************
 * ZCopyReaderListener *
 ***********************/
#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
static void on_data_available_zcopy(void *listener_data, DDS_DataReader *reader)
{
    if (listener_data == NULL) {
        fprintf(stderr,"Error: listener_data is NULL\n");
        return;
    }

    TSeq samples = DDS_SEQUENCE_INITIALIZER;
    struct DDS_SampleInfoSeq sampleInfo = DDS_SEQUENCE_INITIALIZER;
    TestMessage message;
    IMessagingCB *callback = (IMessagingCB *) listener_data;

    DDS_ReturnCode_t retcode = DDS_DataReader_take(
            reader,
            (struct DDS_UntypedSampleSeq *) &samples,
            &sampleInfo,
            DDS_LENGTH_UNLIMITED,
            DDS_ANY_SAMPLE_STATE,
            DDS_ANY_VIEW_STATE,
            DDS_ANY_INSTANCE_STATE);
    if (retcode == DDS_RETCODE_NO_DATA)
    {
        return;
    }
    else if (retcode != DDS_RETCODE_OK)
    {
        fprintf(stderr,"Error during taking data in listener %d\n", retcode);
        return;
    }

    int seq_length = DDS_SampleInfoSeq_get_length(&sampleInfo);
    for (int i = 0; i < seq_length; ++i)
    {
        if (DDS_SampleInfoSeq_get_reference(&sampleInfo, i)->valid_data == DDS_BOOLEAN_TRUE)
        {
            T *sample = DDS_TypedSampleSeq_get_reference<T, TSeq>(&samples, i);

            message.entity_id = sample->entity_id;
            message.seq_num = sample->seq_num;
            message.timestamp_sec = sample->timestamp_sec;
            message.timestamp_usec = sample->timestamp_usec;
            message.latency_ping = sample->latency_ping;
            /* DDS_Octet[] is fixed-size, and the content is ignored
             * So only the size of message matters */
            message.size = sample->size;

            if (message.size != CERT_ZC_ARRAY_SIZE
                && message.size != perftest_cpp::INITIALIZE_SIZE
                && message.size != perftest_cpp::FINISHED_SIZE)
            {
                fprintf(stderr, "Unexpected size of received sample (%d).\n",
                    message.size);
                continue;
            }

            callback->process_message(message);
        }
    }
    retcode = DDS_DataReader_return_loan(
            reader,
            (struct DDS_UntypedSampleSeq *) &samples,
            &sampleInfo);
    if (retcode != DDS_RETCODE_OK)
    {
        fprintf(stderr,"Error during return loan in listener %d.\n", retcode);
        fflush(stderr);
    }
}

template <typename T, typename TSeq>
class ZCopyReaderListener : public ReaderListenerBase<T, TSeq> {
public:
    ZCopyReaderListener(IMessagingCB *callback)
    {
        this->dr_listener = DDS_DataReaderListener_INITIALIZER;
        this->dr_listener.as_listener.listener_data = (void *) callback;
        this->dr_listener.on_data_available = &on_data_available_zcopy<T, TSeq>;
    }
};
#endif
/******************************
 * End of ZCopyReaderListener *
 ******************************/

/*******************
 * End of Listener *
 *******************/

/**********************************************************************
 *********************** RTICertImpl methods **************************
 **********************************************************************/

/*******************************
 * Constructor for RTICertImpl
 */
template <typename T, typename TSeq>
RTICertImplBase<T, TSeq>::RTICertImplBase(
    const char *type_name,
    NDDS_Type_Plugin *plugin)
    : _parent(nullptr),
      _PM(nullptr),
      _transport(),
      _pongSemaphore(nullptr)
{
    _instanceMaxCountReader = 1;
    _factory = NULL;
    _participant = NULL;
    _subscriber = NULL;
    _publisher = NULL;
    _reader = NULL;
    _typename = type_name;
    _plugin = plugin;
    _useZeroCopy = false;

    _qoSProfileNameMap[LATENCY_TOPIC_NAME] = std::string("LatencyQos");
    _qoSProfileNameMap[ANNOUNCEMENT_TOPIC_NAME] = std::string("AnnouncementQos");
    _qoSProfileNameMap[THROUGHPUT_TOPIC_NAME] = std::string("ThroughputQos");
}

/*********************************************************
 * shutdown
 */
template <typename T, typename TSeq>
void RTICertImplBase<T, TSeq>::shutdown()
{
    if(_pongSemaphore != NULL) {
        _pongSemaphore = NULL;
    }
    DDS_DomainParticipantFactory *factory = NULL;
    RT_Registry_T *registry = NULL;
    factory = DDS_DomainParticipantFactory_get_instance();
    CHECK_PTR(factory, "DDS_DomainParticipantFactory_get_instance");
    registry = DDS_DomainParticipantFactory_get_registry(factory);
    CHECK_PTR(registry, "DDS_DomainParticipantFactory_get_registry");
    RT_Registry_unregister(registry, DISC_FACTORY_DEFAULT_NAME, NULL, NULL);
    RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME, NULL, NULL);
    RT_Registry_unregister(registry, DDSHST_READER_DEFAULT_HISTORY_NAME, NULL, NULL);
    RT_Registry_unregister(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME, NULL, NULL);
done:
    return;
}

#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
void RTICertImpl_ZCopy<T, TSeq>::shutdown()
{
    RTICertImplBase<T, TSeq>::shutdown();
    DDS_DomainParticipantFactory *factory = NULL;
    RT_Registry_T *registry = NULL;
    factory = DDS_DomainParticipantFactory_get_instance();
    CHECK_PTR(factory, "DDS_DomainParticipantFactory_get_instance");
    registry = DDS_DomainParticipantFactory_get_registry(factory);
    CHECK_PTR(registry, "DDS_DomainParticipantFactory_get_registry");
    RT_Registry_unregister(registry, NETIO_DEFAULT_NOTIF_NAME, NULL, NULL);
done:
    return;
}
#endif

/********************************
 * get_middleware_version_string
 */
template <typename T, typename TSeq>
const std::string RTICertImplBase<T, TSeq>::get_middleware_version_string()
{
    return "RTI Connext DDS Micro "
        + perftest::to_string((int) RTIME_DDS_VERSION_MAJOR) + "."
        + perftest::to_string((int) RTIME_DDS_VERSION_MINOR) + "."
        + perftest::to_string((int) RTIME_DDS_VERSION_REVISION) + "."
        + perftest::to_string((int) RTIME_DDS_VERSION_RELEASE);
}

/*************************************
 * Validate and manage the parameters
 */
template <typename T, typename TSeq>
bool RTICertImplBase<T, TSeq>::validate_input()
{
    /* Manage parameter -instance */
    if (_PM->is_set("instances")) {
        _instanceMaxCountReader = _PM->get<long>("instances");
    }

#ifdef RTI_ZEROCOPY_AVAILABLE
    /* Manage parameter -dataLen */
    if (_PM->is_set("zerocopy")) {
        long dataLen = CERT_ZC_ARRAY_SIZE + perftest_cpp::OVERHEAD_BYTES;
        if (_PM->is_set("dataLen")) {
            fprintf(stderr,
                "Specified '-dataLen' (%ld) invalid and ignored:\n"
                "When using ZeroCopy transport, the Data length\n"
                "is configured at building stage with the\n"
                "build parameter --cert-zc-datalen <>.\n"
                "The current ZC data length is (%ld).\n",
                _PM->get<long>("dataLen"),
                dataLen);
        }
        _PM->set("dataLen", dataLen);
    }
#endif

    /* Manage parameter -writeInstance */
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

    /* Manage parameter -peer */
    if (_PM->get_vector<std::string>("peer").size() >= RTIPERFTEST_MAX_PEERS) {
        fprintf(stderr,
                "The maximum of 'initial_peers' is %d\n",
                RTIPERFTEST_MAX_PEERS);
        return false;
    }

    /* Manage transport parameter */
    if (!_transport.validate_input()) {
        fprintf(stderr, "Failure validating the transport options.\n");
        return false;
    };

#ifdef RTI_CERT
    if (_PM->get<std::string>("transport") == "SHMEM")
    {
        fprintf(stderr, "SHMEM is not supported for Connext Cert.\n");
        return false;
    }
#endif

    return true;
}

/*********************************************************
 * PrintConfiguration
 */
template <typename T, typename TSeq>
std::string RTICertImplBase<T, TSeq>::print_configuration()
{

    std::ostringstream stringStream;

    stringStream << "\tDomain: " << _PM->get<int>("domain") << "\n";

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

/*********************************
 ******* Publisher classes *******
 *********************************/

/*********************************
 * CertPublisher class
 */
template<typename T, typename TSeq>
class CertPublisherBase : public IMessagingWriter
{
protected:
    ParameterManager *_PM;
    DDS_DataWriter *_writer;
    PerftestSemaphore *_pongSemaphore;
    long _numInstances;
    long _instanceCounter;
    long _instancesToBeWritten;
    DDS_InstanceHandle_t *_instanceHandles;
    bool _isReliable;
    DDS_ReturnCode_t retCode;

    long &getCftInstanceIndex() {
        return _numInstances;
    }

public:
    CertPublisherBase(
            DDS_DataWriter *writer,
            long num_instances,
            PerftestSemaphore *pongSemaphore,
            long instancesToBeWritten,
            ParameterManager *PM)
            : _PM(PM),
              _writer(writer),
              _pongSemaphore(pongSemaphore),
              _numInstances(num_instances),
              _instanceCounter(0),
              _instancesToBeWritten(instancesToBeWritten),
              _isReliable(!_PM->is_set("bestEffort"))
    {
        /* Adding one extra instance for MAX_CFT_VALUE */
        _instanceHandles = (DDS_InstanceHandle_t *) malloc(
                sizeof(DDS_InstanceHandle_t)*static_cast<unsigned long>(
                    _numInstances + 1));
        if (_instanceHandles == NULL) {
            shutdown();
            fprintf(stderr, "_instanceHandles malloc failed\n");
            throw std::bad_alloc();
        }
    }

    ~CertPublisherBase()
    {
        try {
            shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in CertPublisherBase::~CertPublisherBase(): %s.\n", ex.what());
        }
    }

    void shutdown()
    {
        if (_instanceHandles != NULL) {
            free(_instanceHandles);
            _instanceHandles = NULL;
        }
    }

    virtual void flush() override
    {
    }

    virtual bool wait_for_ping_response() override
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
    virtual bool wait_for_ping_response(int timeout) override
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

    virtual bool notify_ping_response() override
    {
        if(_pongSemaphore != nullptr) {
            if (!PerftestSemaphore_give(_pongSemaphore)) {
                fprintf(stderr,"Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    virtual unsigned int get_pulled_sample_count() override
    {
        /* Not supported in Cert */
        return 0;
    }

    virtual unsigned int get_sample_count() override
    {
        /* Not supported in Cert */
        return 0;
    }

    virtual unsigned int get_sample_count_peak() override
    {
        /* Not supported in Cert */
        return 0;
    }

    virtual void wait_for_ack(int sec, unsigned int nsec) override
    {
        UNUSED_ARG(sec);
        UNUSED_ARG(nsec);

        /* Since wait_for_ack is not supported in Cert, it is replaced by a
        sleep of 20 milliseconds */
        PerftestClock::milliSleep(20);

        return;
    }

};

template <typename T, typename TSeq>
class CertPublisher : public CertPublisherBase<T, TSeq>
{
protected:
    /* pointer to loned data in SharedQ */
    T *_data;
public:
    CertPublisher(
            DDS_DataWriter *writer,
            long num_instances,
            PerftestSemaphore *pongSemaphore,
            long instancesToBeWritten,
            ParameterManager *PM)
            : CertPublisherBase<T, TSeq>(
                    writer,
                    num_instances,
                    pongSemaphore,
                    instancesToBeWritten,
                    PM)
    {
        _data = (T *) malloc(sizeof(T));
        memset(_data, 0, sizeof(*_data));
    }

    ~CertPublisher()
    {
        try {
            this->shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in CertPublisher::~CertPublisher(): %s.\n", ex.what());
        }
    }

    void shutdown()
    {
        if (this->_instanceHandles != NULL) {
            free(this->_instanceHandles);
            this->_instanceHandles = NULL;
        }

        if (this->_data != NULL) {
            free(this->_data);
            this->_data = NULL;
        }
    }

    void wait_for_readers(int numSubscribers)
    {
        /* This is a workaround for registering before enabling Datawriter
         * The datawriter is enabled at this point
         * Possible Improvement: Create a function for this instead
         * Modify the perftest_cpp to call the function before calling wait_for_reader
         */
        for (long i = 0; i < this->_numInstances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                _data->key[c] = (unsigned char) (i >> (c * 8));
            }
            this->_instanceHandles[i] = DDS_DataWriter_register_instance(this->_writer, this->_data);
        }

        /* Register the key of MAX_CFT_VALUE */
        for (int c = 0; c < KEY_SIZE; c++) {
            _data->key[c] = (unsigned char)(MAX_CFT_VALUE >> (c * 8));
        }
        this->_instanceHandles[this->_numInstances] = DDS_DataWriter_register_instance(this->_writer, this->_data);

        DDS_PublicationMatchedStatus status;

        while (true) {
            DDS_ReturnCode_t retcode = DDS_DataWriter_get_publication_matched_status(
                    this->_writer,
                    &status);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "wait_for_readers DDS_DataWriter_get_publication_matched_status "
                        "failed: %d.\n",
                        retcode);
            }
            if (status.current_count >= numSubscribers) {
                break;
            }
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    bool send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        RTI_BOOL brc;
        long key = 0;
        long instanceHandlesIndex = 0;

        /* Calculate key and add it if using more than one instance */
        if (!isCftWildCardKey) {
            if (this->_numInstances > 1) {
                if (this->_instancesToBeWritten == -1) {
                    key = this->_instanceCounter++ % this->_numInstances;
                } else {
                    /* send sample to a specific subscriber */
                    key = this->_instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        for (int c = 0; c < KEY_SIZE; c++) {
            _data->key[c] = (unsigned char)(key >> (c * 8));
        }

        /* Populate sample */
        _data->entity_id = message.entity_id;
        _data->seq_num = message.seq_num;
        _data->timestamp_sec =message.timestamp_sec;
        _data->timestamp_usec = message.timestamp_usec;
        _data->latency_ping = message.latency_ping;
        brc = DDS_OctetSeq_loan_contiguous(&_data->bin_data,
                (DDS_Octet*)message.data,
                message.size,
                message.size);
        CHECK_BOOL(brc, "DDS_OctetSeq_loan_contiguous");

#ifdef DEBUG_PING_PONG
        std::cerr << " -- -- before sending it in the write\n";
#endif
        instanceHandlesIndex = this->getCftInstanceIndex();
        if (!isCftWildCardKey) {
            instanceHandlesIndex = key;
        }
        retcode = DDS_DataWriter_write(
            this->_writer,
            this->_data,
            &this->_instanceHandles[instanceHandlesIndex]);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr,"Write error %d.\n", retcode);
            return false;
        }
#ifdef DEBUG_PING_PONG
        else std::cerr << ">> wrote sample " << this->_data->seq_num << std::endl;
#endif

        brc = DDS_OctetSeq_unloan(&_data->bin_data);
        CHECK_BOOL(brc, "DDS_OctetSeq_unloan");

        return true;
    done:
        return false;
    }
};
/**********************************
 * End of CertPublisher class
 */


#ifdef RTI_ZEROCOPY_AVAILABLE
/*********************************
 * ZCopyCertPublisher class
 */
template <typename T, typename TSeq>
class ZCopyCertPublisher : public CertPublisherBase<T, TSeq>
{
public:
    ZCopyCertPublisher(
            DDS_DataWriter *writer,
            long num_instances,
            PerftestSemaphore *pongSemaphore,
            long instancesToBeWritten,
            ParameterManager *PM)
            : CertPublisherBase<T, TSeq>(
                    writer,
                    num_instances,
                    pongSemaphore,
                    instancesToBeWritten,
                    PM)
    { }

    ~ZCopyCertPublisher()
    {
        try {
            this->shutdown();
        } catch (const std::exception &ex) {
            fprintf(stderr, "Exception in ZCopyCertPublisher::~ZCopyCertPublisher(): %s.\n", ex.what());
        }
    }

    void shutdown()
    {
        if (this->_instanceHandles != NULL) {
            free(this->_instanceHandles);
            this->_instanceHandles = NULL;
        }
    }

    void wait_for_readers(int numSubscribers)
    {
        DDS_PublicationMatchedStatus status;

        while (true) {
            DDS_ReturnCode_t retcode = DDS_DataWriter_get_publication_matched_status(
                    this->_writer,
                    &status);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "wait_for_readers DDS_DataWriter_get_publication_matched_status "
                        "failed: %d.\n",
                        retcode);
            }
            if (status.current_count >= numSubscribers) {
                break;
            }
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    bool send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        long key = 0;
        long instanceHandlesIndex = 0;

        /* Must loan a new sample for every write
         * Cannot reuse writen-sample
         */
        T* data;
        retcode = DDS_DataWriter_get_loan(this->_writer, (void **) &data);
        if (retcode == DDS_RETCODE_OUT_OF_RESOURCES) {
            goto done;
        }
        CHECK_RETCODE(retcode, "DDS_DataWriter_get_loan");

        /* This is a workaround for registering before enabling Datawriter 
         * The datawriter is enabled at this point 
         * Possible Improvement: Create a function for this instead
         * Modify the perftest_cpp to call the function before calling wait_for_reader 
         */
        for (long i = 0; i < this->_numInstances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                data->key[c] = (unsigned char) (i >> (c * 8));
            }
            this->_instanceHandles[i] = DDS_DataWriter_register_instance(this->_writer, data);
        }

        /* Register the key of MAX_CFT_VALUE */
        for (int c = 0; c < KEY_SIZE; c++) {
            data->key[c] = (unsigned char)(MAX_CFT_VALUE >> (c * 8));
        }
        this->_instanceHandles[this->_numInstances] = DDS_DataWriter_register_instance(this->_writer, data);

        /* Calculate key and add it if using more than one instance */
        if (!isCftWildCardKey) {
            if (this->_numInstances > 1) {
                if (this->_instancesToBeWritten == -1) {
                    key = this->_instanceCounter++ % this->_numInstances;
                } else { /* send sample to a specific subscriber */
                    key = this->_instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        for (int c = 0; c < KEY_SIZE; c++) {
            data->key[c] = (unsigned char)(key >> (c * 8));
        }

        /* Populate sample */
        data->entity_id = message.entity_id;
        data->seq_num = message.seq_num;
        data->timestamp_sec = message.timestamp_sec;
        data->timestamp_usec = message.timestamp_usec;
        data->latency_ping = message.latency_ping;
        /* DDS_Octet[] is fixed size and content does not matter
         * Only the size of the message matters.
         */
        data->size = message.size;

#ifdef DEBUG_PING_PONG
        std::cerr << " -- -- before sending it in the write\n";
#endif
        instanceHandlesIndex = this->getCftInstanceIndex();
        if (!isCftWildCardKey) {
            instanceHandlesIndex = key;
        }
        retcode = DDS_DataWriter_write(
            this->_writer,
            data,
            &this->_instanceHandles[instanceHandlesIndex]);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr,"Write error %d.\n", retcode);
            return false;
        }
#ifdef DEBUG_PING_PONG
        else std::cerr << ">> wrote sample " << data->seq_num << std::endl;
#endif

        return true;
    done:
        return false;
    }
};
/***************************************
 * End of ZCopyCertPublisher class
 */
#endif /* RTI_ZEROCOPY_AVAILABLE */
/*********************************
 *** End of Publisher classes **
 *********************************/

/*********************************
 ****** Subscriber  classes ******
 *********************************/

/**********************
 * CertSubscriber class
 */
template <typename T, typename TSeq>
class CertSubscriberBase : public IMessagingReader
{
public:
    ParameterManager *_PM;
    TestMessage _message;
    int _data_idx;
    bool _no_data;
    bool _endTest;
    bool _useReceiveThread;
    DDS_WaitSet            *_waitset;
    DDS_ConditionSeq        _active_conditions;
    DDS_GuardCondition      *_endTestCondition;
    DDS_DataReader          *_reader;
    TSeq                    _data_seq;
    DDS_SampleInfoSeq       _info_seq;

    CertSubscriberBase(DDS_DataReader *reader, bool useReceiveThread, ParameterManager *PM)
            : _PM(PM),
              _message(),
              _data_idx(0),
              _no_data(true),
              _endTest(false),
              _useReceiveThread(useReceiveThread),
              _waitset(NULL),
              _reader(reader)
    {
        RTI_BOOL brc;
        DDS_ReturnCode_t retcode;

        _data_seq = DDS_SEQUENCE_INITIALIZER;
        _info_seq = DDS_SEQUENCE_INITIALIZER;

        if (_useReceiveThread) {
            _active_conditions = (struct DDS_ConditionSeq)DDS_SEQUENCE_INITIALIZER;
            brc = DDS_ConditionSeq_set_maximum(&this->_active_conditions, 1);
            CHECK_BOOL(brc, "DDS_ConditionSeq_set_maximum");
            _waitset = DDS_WaitSet_new();
            CHECK_PTR(_waitset, "DDS_WaitSet_new");

            DDS_StatusCondition *reader_status;
            DDS_Condition *condition;
            DDS_Entity *entity = DDS_DataReader_as_entity(reader);
            CHECK_PTR(entity, "DDS_DataReader_as_entity");
            reader_status = DDS_Entity_get_statuscondition(entity);
            CHECK_PTR(reader_status, "DDS_Entity_get_statuscondition");

            retcode = DDS_StatusCondition_set_enabled_statuses(reader_status, DDS_DATA_AVAILABLE_STATUS);
            CHECK_RETCODE(retcode, "DDS_StatusCondition_set_enabled_statuses");
            condition = DDS_StatusCondition_as_condition(reader_status);
            CHECK_PTR(condition, "DDS_StatusCondition_as_condition");
            retcode = DDS_WaitSet_attach_condition(_waitset, condition);
            CHECK_RETCODE(retcode, "DDS_WaitSet_attach_condition");

            _endTestCondition = DDS_GuardCondition_new();
            CHECK_PTR(_endTestCondition, "DDS_GuardCondition_new");
            retcode = DDS_WaitSet_attach_condition(_waitset, DDS_GuardCondition_as_condition(_endTestCondition));
            CHECK_RETCODE(retcode, "DDS_WaitSet_attach_condition");
        }
    done:
        return;
    }

    ~CertSubscriberBase()
    {
        try
        {
            shutdown();
        }
        catch(const std::exception& e)
        {
            fprintf(stderr, "Exception in CertSubscriberBase::~CertSubscriberBase(): %s.\n", e.what());
        }
    }

    virtual void shutdown() override
    {
        /* loan may be outstanding during shutdown */
        DDS_DataReader_return_loan(
                _reader,
                (struct DDS_UntypedSampleSeq *) &_data_seq,
                &_info_seq);
    }

    virtual void wait_for_writers(int numPublishers) override
    {
        DDS_SubscriptionMatchedStatus status;

        while (true) {
            DDS_DataReader_get_subscription_matched_status(_reader, &status);
            if (status.current_count >= numPublishers) {
                break;
            }
            PerftestClock::milliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    virtual bool unblock() override
    {
        _endTest = true;
        DDS_ReturnCode_t retCode = DDS_GuardCondition_set_trigger_value(_endTestCondition, DDS_BOOLEAN_TRUE);
        if (retCode != DDS_RETCODE_OK) {
            fprintf(stderr,"Error setting a GuardCondition on unblock %d.\n", retCode);
            fflush(stderr);
            return false;
        }
        return true;
    }

    virtual unsigned int get_sample_count() override
    {
        /* Not supported in Micro */
        return 0;
    }

    virtual unsigned int get_sample_count_peak() override
    {
        /* Not supported in Micro */
        return 0;
    }

};

template <typename T, typename TSeq>
class CertSubscriber : public CertSubscriberBase<T, TSeq>
{
public:
    CertSubscriber(DDS_DataReader *reader, bool useReceiveThread, ParameterManager *PM)
            : CertSubscriberBase<T, TSeq>(reader, useReceiveThread, PM)
    { }

    TestMessage *receive_message()
    {
        DDS_ReturnCode_t retCode;
        int seq_length;

        while (!this->_endTest) {
            /* no outstanding reads */
            if (this->_no_data) {
                retCode = DDS_WaitSet_wait(
                        this->_waitset,
                        &this->_active_conditions,
                        &DDS_DURATION_INFINITE);
                if (retCode != DDS_RETCODE_OK) {
                    printf("dds_waitset_wait RetCode: %d\n",
                           retCode);
                    continue;
                }

                if (DDS_ConditionSeq_get_length(&this->_active_conditions) == 0)
                {
                    continue;
                }

                if (this->_endTest)
                {
                    if (this->_data_idx != seq_length)
                    {
                        retCode = DDS_DataReader_return_loan(
                                this->_reader,
                                (struct DDS_UntypedSampleSeq *) &this->_data_seq,
                                &this->_info_seq);
                        if (retCode != DDS_RETCODE_OK) {
                            fprintf(stderr,"Error during return loan %d.\n", retCode);
                            fflush(stderr);
                        }
                    }
                    return NULL;
                }

                retCode = DDS_DataReader_take(
                    this->_reader,
                    (struct DDS_UntypedSampleSeq *) &this->_data_seq,
                    &this->_info_seq,
                    DDS_LENGTH_UNLIMITED,
                    DDS_ANY_SAMPLE_STATE,
                    DDS_ANY_VIEW_STATE,
                    DDS_ANY_INSTANCE_STATE);
                if (retCode == DDS_RETCODE_NO_DATA)
                {
                    continue;
                }
                else if (retCode != DDS_RETCODE_OK)
                {
                    fprintf(stderr,"Error during taking data %d.\n", retCode);
                    return NULL;
                }

                this->_data_idx = 0;
                this->_no_data = false;
            }

            seq_length = DDS_SampleInfoSeq_get_length(&this->_info_seq);
            /* check to see if hit end condition */
            if (this->_data_idx == seq_length)
            {
                retCode = DDS_DataReader_return_loan(
                        this->_reader,
                        (struct DDS_UntypedSampleSeq *) &this->_data_seq,
                        &this->_info_seq);
                if (retCode != DDS_RETCODE_OK) {
                    fprintf(stderr,"Error during return loan %d.\n", retCode);
                    fflush(stderr);
                }
                this->_no_data = true;
                continue;
            }

            /*skip non-valid data */
            DDS_SampleInfo *sample_info = DDS_SampleInfoSeq_get_reference(
                &this->_info_seq, this->_data_idx);
            while ((sample_info->valid_data == false)
                    && (this->_data_idx < seq_length))
            {
                this->_data_idx++;
                sample_info = DDS_SampleInfoSeq_get_reference(&this->_info_seq,
                    this->_data_idx);
            }

            T *valid_sample = DDS_TypedSampleSeq_get_reference<T, TSeq>(
                    &this->_data_seq, this->_data_idx);

            this->_message.entity_id = valid_sample->entity_id;
            this->_message.seq_num = valid_sample->seq_num;
            this->_message.timestamp_sec = valid_sample->timestamp_sec;
            this->_message.timestamp_usec = valid_sample->timestamp_usec;
            this->_message.latency_ping = valid_sample->latency_ping;
            this->_message.size = DDS_OctetSeq_get_length(&valid_sample->bin_data);
            this->_message.data = (char *) DDS_OctetSeq_get_contiguous_buffer(&valid_sample->bin_data);

            ++this->_data_idx;

            return &this->_message;
        }  /* end while */
        return NULL;
    }
};
/**********************************
 * End of CertSubscriber class
 */

#ifdef RTI_ZEROCOPY_AVAILABLE
/*********************************
 * ZCopyCertSubscriber class
 */
template <typename T, typename TSeq>
class ZCopyCertSubscriber : public CertSubscriberBase<T, TSeq>
{
public:
    ZCopyCertSubscriber(DDS_DataReader *reader, bool useReceiveThread, ParameterManager *PM)
            : CertSubscriberBase<T, TSeq>(reader, useReceiveThread, PM)
    { }

    TestMessage *receive_message()
    {
        DDS_ReturnCode_t retCode;
        int seq_length;

        while (!this->_endTest) {
            /* no outstanding reads */
            if (this->_no_data) {
                retCode = DDS_WaitSet_wait(
                        this->_waitset,
                        &this->_active_conditions,
                        &DDS_DURATION_INFINITE);
                if (retCode != DDS_RETCODE_OK) {
                    printf("dds_waitset_wait RetCode: %d\n",
                           retCode);
                    continue;
                }

                if ((int) DDS_ConditionSeq_get_length(&this->_active_conditions) == 0)
                {
                    continue;
                }

                if (this->_endTest)
                {
                    if (this->_data_idx != seq_length)
                    {
                        retCode = DDS_DataReader_return_loan(
                                this->_reader,
                                (struct DDS_UntypedSampleSeq *) &this->_data_seq,
                                &this->_info_seq);
                        if (retCode != DDS_RETCODE_OK) {
                            fprintf(stderr,"Error during return loan %d.\n", retCode);
                            fflush(stderr);
                        }
                    }
                    return NULL;
                }

                retCode = DDS_DataReader_take(
                    this->_reader,
                    (struct DDS_UntypedSampleSeq *) &this->_data_seq,
                    &this->_info_seq,
                    DDS_LENGTH_UNLIMITED,
                    DDS_ANY_SAMPLE_STATE,
                    DDS_ANY_VIEW_STATE,
                    DDS_ANY_INSTANCE_STATE);
                if (retCode == DDS_RETCODE_NO_DATA)
                {
                    continue;
                }
                else if (retCode != DDS_RETCODE_OK)
                {
                    fprintf(stderr,"Error during taking data %d.\n", retCode);
                    return NULL;
                }

                this->_data_idx = 0;
                this->_no_data = false;
            }

            seq_length = DDS_SampleInfoSeq_get_length(&this->_info_seq);
            /* check to see if hit end condition */
            if (this->_data_idx == seq_length)
            {
                retCode = DDS_DataReader_return_loan(
                        this->_reader,
                        (struct DDS_UntypedSampleSeq *) &this->_data_seq,
                        &this->_info_seq);
                if (retCode != DDS_RETCODE_OK) {
                    fprintf(stderr,"Error during return loan %d.\n", retCode);
                    fflush(stderr);
                }
                this->_no_data = true;
                continue;
            }

            /* skip non-valid data */
            DDS_SampleInfo *sample_info = DDS_SampleInfoSeq_get_reference(
                &this->_info_seq, this->_data_idx);
            while ((sample_info->valid_data == false)
                    && (this->_data_idx < seq_length))
            {
                this->_data_idx++;
                sample_info = DDS_SampleInfoSeq_get_reference(&this->_info_seq,
                    this->_data_idx);
            }

            T *valid_sample = DDS_TypedSampleSeq_get_reference<T, TSeq>(
                    &this->_data_seq, this->_data_idx);

            this->_message.entity_id = valid_sample->entity_id;
            this->_message.seq_num = valid_sample->seq_num;
            this->_message.timestamp_sec = valid_sample->timestamp_sec;
            this->_message.timestamp_usec = valid_sample->timestamp_usec;
            this->_message.latency_ping = valid_sample->latency_ping;
            /* DDS_Octet[] is fixed-size, and the content is ignored
             * So only the size of message matters */
            this->_message.size = valid_sample->size;

            if (this->_message.size != CERT_ZC_ARRAY_SIZE
                && this->_message.size != perftest_cpp::INITIALIZE_SIZE
                && this->_message.size != perftest_cpp::FINISHED_SIZE)
            {
                fprintf(stderr, "Unexpected size of received sample (%d).\n",
                    this->_message.size);
                continue;
            }

            ++this->_data_idx;

            return &this->_message;
        }  /* end while */
        return NULL;
    }
};
/***************************************
 * End of ZCopyCertSubscriber class
 */
#endif /* RTI_ZEROCOPY_AVAILABLE */
/*********************************
 *** End of Subscriber classes ***
 *********************************/

/******************************************************************
 ****************** RTICertImpl Configuration *********************
 ******************************************************************/

/*********************************
 * configure participant qos
 */
template <typename T, typename TSeq> bool
RTICertImplBase<T, TSeq>::configure_participant_qos(DDS_DomainParticipantQos &dpQos)
{
    // To be implemented by derived classes
    // Unused parameter
    UNUSED_ARG(dpQos);
    return false;
}

template <typename T, typename TSeq> bool
RTICertImpl<T, TSeq>::configure_participant_qos(DDS_DomainParticipantQos &dpQos)
{
    DDS_DomainParticipantFactory *factory;
    struct DDS_DomainParticipantFactoryQos dpfQos =
        DDS_DomainParticipantFactoryQos_INITIALIZER;
    dpQos = DDS_DomainParticipantQos_INITIALIZER;
    const std::vector<std::string> peerList =
        this->_PM->template get_vector<std::string>("peer");
    RT_Registry_T *registry;
    struct DPSE_DiscoveryPluginProperty discProp = DPSE_DiscoveryPluginProperty_INITIALIZER;
    RTI_BOOL brc;
    DDS_ReturnCode_t retCode;
    char **strRef;

    factory = DDS_DomainParticipantFactory_get_instance();
    CHECK_PTR(factory, "Domain ParticipantFactory get instance");

    registry = DDS_DomainParticipantFactory_get_registry(factory);
    CHECK_PTR(registry, "DDS DomainParticipantFactory get registry");

    /* Register default Data Writer and Reader history */
    brc = RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
            WHSM_HistoryFactory_get_interface(), NULL, NULL);
    CHECK_BOOL(brc, "RT_Registry_register wh");

    brc = RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
            RHSM_HistoryFactory_get_interface(), NULL, NULL);
    CHECK_BOOL(brc, "RT_Registry_register rh");

    /* Disable autoenable created entities */
    retCode = DDS_DomainParticipantFactory_get_qos(factory, &dpfQos);
    CHECK_RETCODE(retCode, "DDS_DomainParticipantFactory_get_qos");
    dpfQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    retCode = DDS_DomainParticipantFactory_set_qos(factory, &dpfQos);
    CHECK_RETCODE(retCode, "DDS_DomainParticipantFactory_set_qos");

    /* Discovery register */
    brc = RT_Registry_register(registry, DISC_FACTORY_DEFAULT_NAME,
            DPSE_DiscoveryFactory_get_interface(),
            &discProp._parent, NULL);
    CHECK_BOOL(brc, "RT_Registry_register DISC");

    brc = RT_ComponentFactoryId_set_name(&dpQos.discovery.discovery.name,
            DISC_FACTORY_DEFAULT_NAME);
    CHECK_BOOL(brc, "RT_ComponentFactoryId_set_name DISC");

    /* Configure UDP */
    if (!configureUDPv4Transport(registry, this->_transport, this->_PM))
    {
        return false;
    }

    /* Enabled transport(s) for transport and user_traffic */
    brc = DDS_StringSeq_set_maximum(&dpQos.transports.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
    brc = DDS_StringSeq_set_length(&dpQos.transports.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    strRef = DDS_StringSeq_get_reference(&dpQos.transports.enabled_transports, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(UDP_TRANSPORT_NAME);
    CHECK_PTR(*strRef, "DDS_String_dup");

    if (this->_PM->template get<bool>("multicast")) {
        brc = DDS_StringSeq_set_maximum(&dpQos.user_traffic.enabled_transports, 2);
        CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
        brc = DDS_StringSeq_set_length(&dpQos.user_traffic.enabled_transports, 2);
        CHECK_BOOL(brc, "DDS_StringSeq_set_length");
        strRef = DDS_StringSeq_get_reference(
            &dpQos.user_traffic.enabled_transports, 1);
        CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
        *strRef = DDS_String_dup("_udp://239.255.0.1");
    }
    else
    {
        brc = DDS_StringSeq_set_maximum(&dpQos.user_traffic.enabled_transports, 1);
        CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
        brc = DDS_StringSeq_set_length(&dpQos.user_traffic.enabled_transports, 1);
        CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    }
    strRef = DDS_StringSeq_get_reference(&dpQos.user_traffic.enabled_transports, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(UDP_TRANSPORT_LOCATOR);
    CHECK_PTR(*strRef, "DDS_String_dup");

    this->_transport.transportConfig.kind = TRANSPORT_UDPv4;
    this->_transport.transportConfig.nameString = "UDPv4";

    /* Enable transport for Discovery */
    brc = DDS_StringSeq_set_maximum(&dpQos.discovery.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
    brc = DDS_StringSeq_set_length(&dpQos.discovery.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    strRef = DDS_StringSeq_get_reference(&dpQos.discovery.enabled_transports, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(UDP_TRANSPORT_LOCATOR);
    CHECK_PTR(*strRef, "DDS_String_dup");

    /* If the user provides the list of addresses */
    if (!peerList.empty()) {
        brc = DDS_StringSeq_set_maximum(
            &dpQos.discovery.initial_peers, (int) peerList.size());
        CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
        brc = DDS_StringSeq_set_length(
            &dpQos.discovery.initial_peers, (int) peerList.size());
        CHECK_BOOL(brc, "DDS_StringSeq_set_length");
        for(unsigned int i = 0; i < peerList.size(); ++i) {
            strRef = DDS_StringSeq_get_reference(
                &dpQos.discovery.initial_peers, static_cast<RTI_INT32>(i));
            CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
            *strRef = DDS_String_dup(peerList[i].c_str());
            CHECK_PTR(*strRef, "DDS_String_dup");
        }
    }
    else
    {
        /* Default discovery peers (unicast and multicast) */
        brc = DDS_StringSeq_set_maximum(&dpQos.discovery.initial_peers, 2);
        CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
        brc = DDS_StringSeq_set_length(&dpQos.discovery.initial_peers, 2);
        CHECK_BOOL(brc, "DDS_StringSeq_set_length");
        strRef = DDS_StringSeq_get_reference(&dpQos.discovery.initial_peers, 0);
        CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
        *strRef = DDS_String_dup("239.255.0.1");
        CHECK_PTR(*strRef, "DDS_String_dup");
        strRef = DDS_StringSeq_get_reference(&dpQos.discovery.initial_peers, 1);
        CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
        *strRef = DDS_String_dup("127.0.0.1");
        CHECK_PTR(*strRef, "DDS_String_dup");
    }
    dpQos.discovery.accept_unknown_peers = DDS_BOOLEAN_TRUE;

    configureDPResourceLimits(dpQos);

    return true;
done:
    return false;
}

#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
bool RTICertImpl_ZCopy<T, TSeq>::configure_participant_qos(
    DDS_DomainParticipantQos &dpQos)
{
    DDS_DomainParticipantFactory *factory;
    struct DDS_DomainParticipantFactoryQos dpfQos = DDS_DomainParticipantFactoryQos_INITIALIZER;
    dpQos = DDS_DomainParticipantQos_INITIALIZER;
    RT_Registry_T *registry;
    struct DPSE_DiscoveryPluginProperty discProp = DPSE_DiscoveryPluginProperty_INITIALIZER;
    DDS_DomainParticipantListener participantlistener = DDS_DomainParticipantListener_INITIALIZER;
    struct ZCOPY_NotifInterfaceFactoryProperty *notifProp = NULL;
    struct ZCOPY_NotifMechanismProperty *notifMechProp = NULL;
    RTI_BOOL brc;
    DDS_ReturnCode_t retCode;
    char **strRef;

    factory = DDS_DomainParticipantFactory_get_instance();
    CHECK_PTR(factory, "Domain ParticipantFactory get instance");

    registry = DDS_DomainParticipantFactory_get_registry(factory);
    CHECK_PTR(registry, "DDS DomainParticipantFactory get registry");

    /* Register default Data Writer and Reader history */
    brc = RT_Registry_register(registry, DDSHST_WRITER_DEFAULT_HISTORY_NAME,
            WHSM_HistoryFactory_get_interface(), NULL, NULL);
    CHECK_BOOL(brc, "RT_Registry_register wh");

    brc = RT_Registry_register(registry, DDSHST_READER_DEFAULT_HISTORY_NAME,
            RHSM_HistoryFactory_get_interface(), NULL, NULL);
    CHECK_BOOL(brc, "RT_Registry_register rh");

    /* Disable autoenable created entities */
    retCode = DDS_DomainParticipantFactory_get_qos(factory, &dpfQos);
    CHECK_RETCODE(retCode, "DDS_DomainParticipantFactory_get_qos");
    dpfQos.entity_factory.autoenable_created_entities = DDS_BOOLEAN_FALSE;
    retCode = DDS_DomainParticipantFactory_set_qos(factory, &dpfQos);
    CHECK_RETCODE(retCode, "DDS_DomainParticipantFactory_set_qos");

    /* Discovery register */
    brc = RT_Registry_register(registry, DISC_FACTORY_DEFAULT_NAME,
            DPSE_DiscoveryFactory_get_interface(),
            &discProp._parent, NULL);
    CHECK_BOOL(brc, "RT_Registry_register DISC");

    brc = RT_ComponentFactoryId_set_name(&dpQos.discovery.discovery.name,
            DISC_FACTORY_DEFAULT_NAME);
    CHECK_BOOL(brc, "RT_ComponentFactoryId_set_name DISC");

    /* Configure UDP */
    if (!configureUDPv4Transport(registry, this->_transport, this->_PM)) {
        return false;
    }

    /* Initialize Zero Copy */
    brc = NDDS_Transport_ZeroCopy_initialize(registry, NULL, NULL);
    CHECK_BOOL(brc, "NDDS_Transport_ZeroCopy_initialize");

    /* Register Notification transport mechanism */
    notifMechProp = (struct ZCOPY_NotifMechanismProperty *)
        malloc(sizeof(struct ZCOPY_NotifMechanismProperty));
    CHECK_PTR(notifMechProp, "malloc");

    *notifMechProp = ZCOPY_NotifMechanismProperty_INITIALIZER;
    notifProp = (struct ZCOPY_NotifInterfaceFactoryProperty *)
        malloc(sizeof(struct ZCOPY_NotifInterfaceFactoryProperty));
    CHECK_PTR(notifProp, "malloc");
    *notifProp = ZCOPY_NotifInterfaceFactoryProperty_INITIALIZER;

    notifMechProp->intf_addr = 0;
    notifProp->user_property = notifMechProp;
    if (this->_PM->template get<bool>("latencyTest")) {
        notifProp->max_samples_per_notif = 1;
    } else {
        /* Since DW is writing as fast as possible
         * DR should read more samples per notification
         * to keep up with DW.
         */
        notifProp->max_samples_per_notif = 500;
    }
    brc = ZCOPY_NotifMechanism_register(registry, NETIO_DEFAULT_NOTIF_NAME, notifProp);
    CHECK_BOOL(brc, "ZCOPY_NotifMechanism_register");

    /* Enabled transport(s) for transport and user_traffic */
    brc = DDS_StringSeq_set_maximum(&dpQos.transports.enabled_transports, 2);
    CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
    brc = DDS_StringSeq_set_length(&dpQos.transports.enabled_transports, 2);
    CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    strRef = DDS_StringSeq_get_reference(&dpQos.transports.enabled_transports, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(NOTIF_TRANSPORT_NAME);
    CHECK_PTR(*strRef, "DDS_String_dup");
    strRef = DDS_StringSeq_get_reference(&dpQos.transports.enabled_transports, 1);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(UDP_TRANSPORT_NAME);
    CHECK_PTR(*strRef, "DDS_String_dup");

    brc = DDS_StringSeq_set_maximum(&dpQos.user_traffic.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
    brc = DDS_StringSeq_set_length(&dpQos.user_traffic.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    strRef = DDS_StringSeq_get_reference(&dpQos.user_traffic.enabled_transports, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(NOTIF_TRANSPORT_LOCATOR);
    CHECK_PTR(*strRef, "DDS_String_dup");

    this->_transport.transportConfig.kind = TRANSPORT_SHMEM;
    this->_transport.transportConfig.nameString = "ZeroCopy";

    /* Enable transport for Discovery */
    brc = DDS_StringSeq_set_maximum(&dpQos.discovery.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
    brc = DDS_StringSeq_set_length(&dpQos.discovery.enabled_transports, 1);
    CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    strRef = DDS_StringSeq_get_reference(&dpQos.discovery.enabled_transports, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup(UDP_TRANSPORT_LOCATOR);
    CHECK_PTR(*strRef, "DDS_String_dup");

    /* Default discovery peers (unicast and multicast) 
     * Custom peer list is not supported yet
     */
    brc = DDS_StringSeq_set_maximum(&dpQos.discovery.initial_peers, 2);
    CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
    brc = DDS_StringSeq_set_length(&dpQos.discovery.initial_peers, 2);
    CHECK_BOOL(brc, "DDS_StringSeq_set_length");
    strRef = DDS_StringSeq_get_reference(&dpQos.discovery.initial_peers, 0);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup("239.255.0.1");
    CHECK_PTR(*strRef, "DDS_String_dup");
    strRef = DDS_StringSeq_get_reference(&dpQos.discovery.initial_peers, 1);
    CHECK_PTR(strRef, "DDS_StringSeq_get_reference");
    *strRef = DDS_String_dup("127.0.0.1");
    CHECK_PTR(*strRef, "DDS_String_dup");

    dpQos.discovery.accept_unknown_peers = DDS_BOOLEAN_TRUE;

    configureDPResourceLimits(dpQos);

    return true;
done:
    return false;
}
#endif /* RTI_ZEROCOPY_AVAILABLE */
/*************************************
 * End of configuring participant qos
 */


/***************************
 * configure writer qos
 */
template <typename T, typename TSeq>
bool RTICertImplBase<T, TSeq>::configure_writer_qos(
        DDS_DataWriterQos &dw_qos,
        std::string qos_profile,
        std::string topic_name)
{
     /* Only force reliability on throughput/latency topics */
    if (strcmp(topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (!_PM->get<bool>("bestEffort")) {
            /* default: use the setting specified in the qos profile */
            dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }
        else {
            /* override to best-effort */
            dw_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    } else {
        dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        dw_qos.durability.kind = DDS_TRANSIENT_LOCAL_DURABILITY_QOS;
    }

    if (_PM->get<bool>("keyed"))
    {
        /* Adding one extra instance for MAX_CFT_VALUE */
        dw_qos.resource_limits.max_instances =
                static_cast<DDS_Long>(_PM->get<long>("instances") + 1);
        dw_qos.resource_limits.max_samples = 
                dw_qos.resource_limits.max_instances * dw_qos.resource_limits.max_samples_per_instance;
    }

    /* These QOS's are only set for the Throughput datawriter */
    if (qos_profile == "ThroughputQos") {
        dw_qos.resource_limits.max_samples = _PM->get<int>("sendQueueSize");
        this->_sendQueueSize = dw_qos.resource_limits.max_samples;

        if (_PM->get<bool>("keyed")) {
            dw_qos.resource_limits.max_samples_per_instance =
                    dw_qos.resource_limits.max_samples / dw_qos.resource_limits.max_instances;
        } else {
            dw_qos.resource_limits.max_samples_per_instance =
                    dw_qos.resource_limits.max_samples;
        }

        dw_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind)_PM->get<int>("durability");

        if (_PM->get<unsigned long long>("dataLen") > DEFAULT_MESSAGE_SIZE_MAX) {
            dw_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                _PM->get<int>("sendQueueSize");
        } else {
            dw_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples =
                _PM->get<int>("sendQueueSize") / 10;
        }
        dw_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;
        dw_qos.history.depth = dw_qos.resource_limits.max_samples_per_instance;
        /* Same values we use for Pro (See perftest_qos_profiles.xml). */
        dw_qos.protocol.rtps_reliable_writer.heartbeat_period.sec = 0;
        dw_qos.protocol.rtps_reliable_writer.heartbeat_period.nanosec = 10000000;
        /* DPSE object ID for discovery */
        dw_qos.protocol.rtps_object_id = 101;
    }
    else if (qos_profile == "LatencyQos") {
        /* DPSE object ID for discovery */
        dw_qos.protocol.rtps_object_id = 202;
    }
    else /* qos_profile == "AnnouncementQos" */
    {
        /* DPSE object ID for discovery */
        dw_qos.protocol.rtps_object_id = 203;
    }

    /* SUMMARY FOR THE RESOURCE LIMITS */
    if (_PM->get<bool>("showResourceLimits")) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DW ("
                    << topic_name
                    << " topic):\n"
                    << "\tSamples (Max): "
                    << dw_qos.resource_limits.max_samples
                    << "\n";

        if (_PM->get<bool>("keyed")) {
            stringStream << "\tInstances (Max): "
                        << dw_qos.resource_limits.max_instances
                        << "\n";
            stringStream << "\tMax Samples per Instance: "
                        << dw_qos.resource_limits.max_samples_per_instance
                        << "\n";

        }
        stringStream << "\tDurability is: "
                    << dw_qos.durability.kind
                    << "\n";
        stringStream << "\tHistory is: "
                    << dw_qos.history.kind
                    << "\n";
        stringStream << "\tReliability is: "
                    << dw_qos.reliability.kind
                    << "\n";

        /* Heartbeats per max samples */
        stringStream << "\tHeartbeat period (s/ns): "
                     << dw_qos.protocol.rtps_reliable_writer.heartbeat_period.sec 
                     << ", "
                     << dw_qos.protocol.rtps_reliable_writer.heartbeat_period.nanosec
                     << "\n";
        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }
    return true;
}

/***************************
 * configure reader qos
 */
template <typename T, typename TSeq>
bool RTICertImplBase<T, TSeq>::configure_reader_qos(
        DDS_DataReaderQos &dr_qos,
        std::string qos_profile,
        std::string topic_name)
{
     /* Only force reliability on throughput/latency topics */
    if (strcmp(topic_name.c_str(), ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (!_PM->get<bool>("bestEffort")) {
            dr_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        } else {
            dr_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

    /* only apply durability on Throughput datareader */
    if ((qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")
        && _PM->get<int>("durability") != DDS_PERSISTENT_DURABILITY_QOS) {
        dr_qos.durability.kind =
                (DDS_DurabilityQosPolicyKind) _PM->get<int>("durability");
    }

    /*
     * In micro we cannot have UNLIMITED instances, this means that we need to
     * increase the InstanceMaxCountReader (max instances for the dr) in all
     * cases
     */
    _instanceMaxCountReader++;
    dr_qos.resource_limits.max_instances = static_cast<DDS_Long>(
        _instanceMaxCountReader);

    if (qos_profile == "ThroughputQos") {
        /*
         * For Connext DDS Pro settings are set so initial samples are set to
         * a lower value than max_samples, so we can grow if needed. For micro
         * however we do not have the initial_samples parameter, therefore we
         * must choose a value for max_samples since the beginning. We chose to
         * use 5000. This value should be large enough to handle most of the
         * communications.
         *
         * We could potentially modify this with a new command line parameter
         */
        if (_PM->get<unsigned long long>("dataLen") > MAX_BOUNDED_SEQ_SIZE) {
            dr_qos.resource_limits.max_samples = 50;
            dr_qos.resource_limits.max_samples_per_instance =
                    50 / dr_qos.resource_limits.max_instances;
            dr_qos.history.depth = 50 / dr_qos.resource_limits.max_instances;
        }
        else {
            dr_qos.resource_limits.max_samples = 5000;
            dr_qos.resource_limits.max_samples_per_instance =
                    5000 / dr_qos.resource_limits.max_instances;
            dr_qos.history.depth = 5000 / dr_qos.resource_limits.max_instances;
        }
        /*
         * In micro 2.4.x we don't have keep all, this means we need to set the
         * history to keep last and chose a history depth. For the depth value
         * we can we same value as max_samples
         */

        /* Keep all not supported in Micro 2.4.x */
        dr_qos.history.kind = DDS_KEEP_LAST_HISTORY_QOS;

        /* DPSE object ID for discovery */
        dr_qos.protocol.rtps_object_id = 211;

    } else { /* "LatencyQos" or "AnnouncementQos" */

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

        if (qos_profile == "LatencyQos")
        {
            /* DPSE object ID for discovery */
            dr_qos.protocol.rtps_object_id = 112;
        }
        else /* qos_profile == "AnnouncementQos" */
        {
            /* DPSE object ID for discovery */
            dr_qos.protocol.rtps_object_id = 113;
        }
    }

     /*
     * We could potentially use here the number of subscriber, right now this
     * class does not have access to the number of subscriber though.
     */
    dr_qos.reader_resource_limits.max_remote_writers = 50;
    dr_qos.reader_resource_limits.max_remote_writers_per_instance = 50;

    /*
     * In micro we cannot have UNLIMITED instances, this means that we need to
     * increase the InstanceMaxCountReader (max instances for the dr) in all
     * cases
     */
    dr_qos.resource_limits.max_instances = _instanceMaxCountReader;

    _instanceMaxCountReader++;

    if (_PM->get<bool>("multicast")) {

        if (_transport.getMulticastAddr(topic_name.c_str()).empty()) {
            fprintf(stderr,
                    "topic name must either be %s or %s or %s.\n",
                    THROUGHPUT_TOPIC_NAME,
                    LATENCY_TOPIC_NAME,
                    ANNOUNCEMENT_TOPIC_NAME);
            return false;
        }

        std::string receive_address = "_udp://" + _transport.getMulticastAddr(topic_name.c_str());
        DDS_StringSeq_set_maximum(&dr_qos.transport.enabled_transports, 1);
        DDS_StringSeq_set_length(&dr_qos.transport.enabled_transports, 1);
        *DDS_StringSeq_get_reference(&dr_qos.transport.enabled_transports, 0) =
                DDS_String_dup(receive_address.c_str());

    }

    if (_PM->get<bool>("showResourceLimits")
            && topic_name.c_str() != ANNOUNCEMENT_TOPIC_NAME) {
        std::ostringstream stringStream;

        stringStream << "Resource Limits DR (" 
                    << topic_name
                    << " topic):\n"
                    << "\tSamples (Max): "
                    << dr_qos.resource_limits.max_samples
                    << "\n";

        if (_PM->get<bool>("keyed")){
            stringStream << "\tInstances (Max): "
                        << dr_qos.resource_limits.max_instances
                        << "\n";

            /* Samples per Instance */
            stringStream << "\tMax Samples per Instance: "
                        << dr_qos.resource_limits.max_samples_per_instance
                        << "\n";
        }
        fprintf(stderr, "%s\n", stringStream.str().c_str());
    }

    return true;
}

/*********************************************************
 * Initialize
 */
template <typename T, typename TSeq>
bool RTICertImplBase<T, TSeq>::initialize(
    ParameterManager &PM,
    perftest_cpp *parent)
{
    /* Assign ParameterManager */
    _PM = &PM;
    _transport.initialize(_PM);
    _useZeroCopy = _PM->get<bool>("zerocopy");

    DDS_DomainParticipantQos dpQos = DDS_DomainParticipantQos_INITIALIZER;
    DDS_ReturnCode_t retCode;

    if (parent == NULL) {
        return false;
    }
    _parent = parent;

    _factory = DDS_DomainParticipantFactory_get_instance();

    if (!validate_input()) {
        return false;
    }

    /*
     * Only if we run the latency test we need to wait
     * for pongs after sending pings
     */
    _pongSemaphore = _PM->get<bool>("latencyTest")
            ? PerftestSemaphore_new()
            : NULL;


    if (!configure_participant_qos(dpQos)) {
        return false;
    }

    if (!create_domain_participant(&dpQos))
    {
        return false;
    }

    if (!_PM->get<bool>("dynamicData")) {
        retCode = DDS_DomainParticipant_register_type(_participant, _typename, _plugin);
        CHECK_RETCODE(retCode, "DDS_DomainParticipant_register_type");
    } else {
        fprintf(stderr, "Dynamic data not supported in CERT.\n");
        return false;
    }

    /* Enabling DPSE discovery of an remote participant */
    if (_PM->get<bool>("pub")) {
        retCode = DPSE_RemoteParticipant_assert(_participant, PARTICIPANT_NAME_SX);
    } else {
        retCode = DPSE_RemoteParticipant_assert(_participant, PARTICIPANT_NAME_PX);
    }
    CHECK_RETCODE(retCode, "DPSE_RemoteParticipant_assert");

    _publisher = DDS_DomainParticipant_create_publisher(
            _participant,
            &DDS_PUBLISHER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
    CHECK_PTR(_publisher, "DDS_DomainParticipant_create_publisher");

    _subscriber = DDS_DomainParticipant_create_subscriber(
            _participant,
            &DDS_SUBSCRIBER_QOS_DEFAULT,
            NULL,
            DDS_STATUS_MASK_NONE);
    CHECK_PTR(_subscriber, "DDS_DomainParticipant_create_subscriber");

    return true;
done:
    return false;
}

template <typename T, typename TSeq>
bool RTICertImplBase<T, TSeq>::create_domain_participant(
    DDS_DomainParticipantQos *dpQos)
{

    DDS_DomainParticipantListener participantlistener = DDS_DomainParticipantListener_INITIALIZER;

    /* Configure the participant name for DPSE */
    if (this->_PM->template get<bool>("pub")) {
        DDS_EntityNameQosPolicy_set_name(&dpQos->participant_name, PARTICIPANT_NAME_PX);
        dpQos->protocol.participant_id = PARTICIPANT_ID_PX;
    } else {
        DDS_EntityNameQosPolicy_set_name(&dpQos->participant_name, PARTICIPANT_NAME_SX);
        dpQos->protocol.participant_id = PARTICIPANT_ID_SX;
    }

    _participant = DDS_DomainParticipantFactory_create_participant(
            _factory,
            _PM->get<int>("domain"),
            dpQos,
            &participantlistener,
            DDS_STATUS_MASK_NONE);

    CHECK_PTR(_participant, "DDS_DomainParticipantFactory_create_participant");

    return true;

done:

    return false;
}

#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
bool RTICertImpl_ZCopy<T, TSeq>::create_domain_participant(
    DDS_DomainParticipantQos *dpQos)
{
    DDS_DomainParticipantListener participantlistener = DDS_DomainParticipantListener_INITIALIZER;

    /* Configure the participant name for DPSE */
    if (this->_PM->template get<bool>("pub")) {
        DDS_EntityNameQosPolicy_set_name(&dpQos->participant_name, PARTICIPANT_NAME_PX);
        dpQos->protocol.participant_id = PARTICIPANT_ID_PX;
    } else {
        DDS_EntityNameQosPolicy_set_name(&dpQos->participant_name, PARTICIPANT_NAME_SX);
        dpQos->protocol.participant_id = PARTICIPANT_ID_SX;
    }

    this->_participant = DDS_DomainParticipantFactory_create_participant(
            this->_factory,
            this->_PM->template get<int>("domain"),
            dpQos,
            &participantlistener,
            DDS_STATUS_MASK_NONE);
    CHECK_PTR(this->_participant,
        "DDS_DomainParticipantFactory_create_participant");

    return true;

done:

    return false;
}
#endif

/*********************************************************
 * GetInitializationSampleCount
 */
template <typename T, typename TSeq>
unsigned long RTICertImplBase<T, TSeq>::get_initial_burst_size()
{
    unsigned long initial_burst_size = 50;
    if(_PM->is_set("initialBurstSize"))
    {
        initial_burst_size = _PM->get<unsigned long>("initialBurstSize");
    }
    return initial_burst_size;
}

/*********************************************************
 * CreateWriter
 */
template <typename T, typename TSeq>
IMessagingWriter *RTICertImplBase<T, TSeq>::create_writer(const char *topic_name)
{
    if (_participant == NULL)
    {
        fprintf(stderr,"Participant not initialized.\n");
        return NULL;
    }
    struct DDS_SubscriptionBuiltinTopicData rem_subscription_data =
            DDS_SubscriptionBuiltinTopicData_INITIALIZER;
    std::string rem_participant_name = "";
    struct DDS_DataWriterQos dw_qos = DDS_DataWriterQos_INITIALIZER;
    DDS_DataWriter *writer = NULL;
    std::string qos_profile = "";
    DDS_Topic *topic = DDS_DomainParticipant_create_topic(
                       _participant,
                       topic_name,
                       _typename,
                       &DDS_TOPIC_QOS_DEFAULT,
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

    if (!configure_writer_qos(dw_qos, qos_profile, topic_name)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
        return NULL;
    }

    writer = DDS_Publisher_create_datawriter(_publisher, topic, &dw_qos, NULL, DDS_STATUS_MASK_NONE);
    CHECK_PTR(writer, "Problem creating writer");

    /* DPSE assert remote subscription
     * The schematic used is the following:
     * Participant Publisher is 1 / Subscriber is 2
     * DatarWriter is 0 / DataReader is 1
     * topic name is Throughput is 1 / Latency is 2 / Announcement is 3

     * Publisher: Throughput DW is 101
     * Publisher: Latency DR is 112
     * Publisher: Announcement DR is 113

     * Subscriber: Throughput DR is 211
     * Subscriber: Latency DW is 202
     * Subscriber: Announcement DW is 203
     */

    /* Assert Remote Subscriptions to discover them */
    rem_subscription_data.topic_name = DDS_String_dup(topic_name);
    rem_subscription_data.type_name = DDS_String_dup(_typename);
    rem_subscription_data.reliability.kind = dw_qos.reliability.kind;
    rem_subscription_data.durability.kind = dw_qos.durability.kind;
    if (qos_profile == "ThroughputQos") {
        rem_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 211;
        rem_participant_name = PARTICIPANT_NAME_SX;
    } else if (qos_profile == "LatencyQos") {
        rem_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 112;
        rem_participant_name = PARTICIPANT_NAME_PX;
    } else {
        rem_subscription_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 113;
        rem_participant_name = PARTICIPANT_NAME_PX;
    }

    if (DDS_RETCODE_OK !=
        DPSE_RemoteSubscription_assert(
                this->_participant,
                rem_participant_name.c_str(),
                &rem_subscription_data,
                this->_plugin->get_key_kind(this->_plugin, NULL)))
    {
        printf("failed to assert remote subscription: %s\n", rem_participant_name.c_str());
        goto done;
    }

    /* Enable DDS entities since this is the last writer created (Announcement) 
     * for Perftest_cert_subscriber */
    if (qos_profile == "AnnouncementQos")
    {
        DDS_Entity *entity;
        DDS_ReturnCode_t retcode;

        entity = DDS_DomainParticipant_as_entity(this->_participant);

        retcode = DDS_Entity_enable(entity);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("failed to enable entity\n");
            goto done;
        }
    }

    if (!_PM->get<bool>("dynamicData")) {
        try {
            return create_CertPublisher(writer);
        } catch (const std::exception &ex) {
            fprintf(stderr,
                    "Exception in RTICertImpl<T>::create_writer(): %s.\n",
                    ex.what());
            return NULL;
        }
    } else {
        fprintf(stderr,"Dynamic data not supported on CERT.\n");
        return NULL;
    }
done:
    return NULL;
}

template <typename T, typename TSeq>
IMessagingWriter *RTICertImplBase<T, TSeq>::create_CertPublisher(
    DDS_DataWriter *writer)
{
    // To be implemented by derived classes
    UNUSED_ARG(writer);

    return nullptr;
}

template <typename T, typename TSeq>
IMessagingWriter *RTICertImpl<T, TSeq>::create_CertPublisher(
    DDS_DataWriter *writer)
{
    return new CertPublisher<T, TSeq>(
                    writer,
                    this->_PM->template get<long>("instances"),
                    this->_pongSemaphore,
                    this->_PM->template get<long>("writeInstance"),
                    this->_PM);
}

#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
IMessagingWriter *RTICertImpl_ZCopy<T, TSeq>::create_CertPublisher(
    DDS_DataWriter *writer)
{
    return new ZCopyCertPublisher<T, TSeq>(
                    writer,
                    this->_PM->template get<long>("instances"),
                    this->_pongSemaphore,
                    this->_PM->template get<long>("writeInstance"),
                    this->_PM);
}
#endif

/*********************************************************
 * CreateReader
 */
template <typename T, typename TSeq>
IMessagingReader *RTICertImplBase<T, TSeq>::create_reader(
        const char *topic_name,
        IMessagingCB *callback)
{
    std::string err_str = "";
    DDS_DataReader *reader = NULL;
    struct DDS_DataReaderQos dr_qos = DDS_DataReaderQos_INITIALIZER;
    std::string qos_profile = "";
    std::string rem_participant_name = "";
    /* Used to create the DDS DataReader */
    DDS_TopicDescription* topic_desc = NULL;
    struct DDS_PublicationBuiltinTopicData rem_publication_data =
            DDS_PublicationBuiltinTopicData_INITIALIZER;
    DDS_Topic *topic =  DDS_DomainParticipant_create_topic(
                       _participant,
                       topic_name,
                       _typename,
                       &DDS_TOPIC_QOS_DEFAULT,
                       NULL,
                       DDS_STATUS_MASK_NONE);
    CHECK_PTR(topic, "Problem creating topic");
    topic_desc = DDS_Topic_as_topicdescription(topic);

    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        fprintf(stderr, "Problem getting qos profile.\n");
        return NULL;
    }

    if (!configure_reader_qos(dr_qos, qos_profile, topic_name)) {
        fprintf(stderr, "Problem creating additional QoS settings with %s profile.\n", qos_profile.c_str());
        return NULL;
    }

    if (callback != NULL)
    {
        if (!_PM->get<bool>("dynamicData"))
        {
            ReaderListenerBase<T, TSeq> *listener
                = (ReaderListenerBase<T, TSeq> *)create_CertReaderListener(callback);
            reader = DDS_Subscriber_create_datareader(
                    _subscriber,
                    topic_desc,
                    &dr_qos,
                    listener->get_listener(),
                    DDS_DATA_AVAILABLE_STATUS);
        }
        else
        {
            fprintf(stderr,"Dynamic data not supported on CERT.\n");
            return NULL;
        }
    }
    else
    {
        reader = DDS_Subscriber_create_datareader(
                _subscriber,
                topic_desc,
                &dr_qos,
                NULL,
                DDS_STATUS_MASK_NONE);
    }
    CHECK_PTR(reader, "Problem creating reader");

    /* Assert Remote Publisher */
    rem_publication_data.topic_name = DDS_String_dup(topic_name);
    rem_publication_data.type_name = DDS_String_dup(_typename);
    rem_publication_data.reliability.kind = dr_qos.reliability.kind;
    rem_publication_data.durability.kind = dr_qos.durability.kind;
    if (qos_profile == "ThroughputQos") {
        rem_publication_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 101;
        rem_participant_name = PARTICIPANT_NAME_PX;
    } else if (qos_profile == "LatencyQos") {
        rem_publication_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 202;
        rem_participant_name = PARTICIPANT_NAME_SX;
    } else {
        rem_publication_data.key.value[DDS_BUILTIN_TOPIC_KEY_OBJECT_ID] = 203;
        rem_participant_name = PARTICIPANT_NAME_SX;
    }

    if (DDS_RETCODE_OK !=
        DPSE_RemotePublication_assert(
                this->_participant,
                rem_participant_name.c_str(),
                &rem_publication_data,
                this->_plugin->get_key_kind(this->_plugin, NULL)))
    {
        printf("failed to assert remote publisher: %s\n", rem_participant_name.c_str());
        goto done;
    }

    /* Enable DDS entities since this is the last writer created (Announcement) 
     * for Perftest Cert Publisher */
    if (qos_profile == "AnnouncementQos")
    {
        DDS_Entity *entity;
        DDS_ReturnCode_t retcode;

        entity = DDS_DomainParticipant_as_entity(this->_participant);

        retcode = DDS_Entity_enable(entity);
        if (retcode != DDS_RETCODE_OK)
        {
            printf("failed to enable entity\n");
            goto done;
        }
    }
    return create_CertSubscriber(reader, callback == NULL);
done:
    return NULL;
}

template <typename T, typename TSeq>
IMessagingReader *RTICertImplBase<T, TSeq>::create_CertSubscriber(
    DDS_DataReader *reader, bool isCallbackNull)
{
    // To be implemented by derived classes
    UNUSED_ARG(reader);
    UNUSED_ARG(isCallbackNull);

    return nullptr;
}

template <typename T, typename TSeq>
void *RTICertImplBase<T, TSeq>::create_CertReaderListener(
    IMessagingCB *callback)
{
    // To be implemented by derived classes
    UNUSED_ARG(callback);
    return nullptr;
}

template <typename T, typename TSeq>
IMessagingReader *RTICertImpl<T, TSeq>::create_CertSubscriber(
    DDS_DataReader *reader, bool isCallbackNull)
{
    return new CertSubscriber<T, TSeq>(reader, isCallbackNull, this->_PM);
}

template <typename T, typename TSeq>
void *RTICertImpl<T, TSeq>::create_CertReaderListener(
    IMessagingCB *callback)
{
    return new ReaderListener<T, TSeq>(callback);
}

#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
IMessagingReader *RTICertImpl_ZCopy<T, TSeq>::create_CertSubscriber(
    DDS_DataReader *reader, bool isCallbackNull)
{
    return new ZCopyCertSubscriber<T, TSeq>(reader, isCallbackNull, this->_PM);
}

template <typename T, typename TSeq>
void *RTICertImpl_ZCopy<T, TSeq>::create_CertReaderListener(
    IMessagingCB *callback)
{
    return new ZCopyReaderListener<T, TSeq>(callback);
}
#endif

template <typename T, typename TSeq>
const std::string RTICertImplBase<T, TSeq>::get_qos_profile_name(const char *topicName)
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
/*********************************************************
 *************** End of RTICertImpl methods **************
 *********************************************************/
namespace {
#ifdef RTI_CERT
    /**
     * WARNING: The REDA functions used here cannot be used in safety
     * applications.
     */
    DDS_Octet* DDS_OctetSeq_get_contiguous_buffer(const struct DDS_OctetSeq *self)
    {
        return (DDS_Octet*)REDA_Sequence_get_buffer((const struct REDA_Sequence *)self);
    }

    RTI_BOOL DDS_OctetSeq_loan_contiguous(struct DDS_OctetSeq *self, void *buffer,
                        RTI_INT32 new_length, RTI_INT32 new_max)
    {
        self->_element_size = sizeof(DDS_Octet);

        return REDA_Sequence_loan_contiguous(
        (struct REDA_Sequence *)self, buffer, new_length, new_max);
    }

    RTI_BOOL DDS_OctetSeq_unloan(struct DDS_OctetSeq *self)
    {
        return REDA_Sequence_unloan((struct REDA_Sequence *)self);
    }
#endif

    /**
     * Explicitly instatiating the DDS_ template function for each possible type,
     * as we need to use C functions in their implementations
     */
    template<>
    TestData_t* DDS_TypedSampleSeq_get_reference<TestData_t, TestData_tSeq>(
        TestData_tSeq* self, RTI_INT32 i)
    {
        return TestData_tSeq_get_reference(self, i);
    }

    template<>
    TestDataKeyed_t* DDS_TypedSampleSeq_get_reference<TestDataKeyed_t, TestDataKeyed_tSeq>(
        TestDataKeyed_tSeq* self, RTI_INT32 i)
    {
        return TestDataKeyed_tSeq_get_reference(self, i);
    }

#ifdef RTI_ZEROCOPY_AVAILABLE
    template<>
    TestData_Cert_ZCopy_t* DDS_TypedSampleSeq_get_reference<TestData_Cert_ZCopy_t, TestData_Cert_ZCopy_tSeq>(
        TestData_Cert_ZCopy_tSeq* self, RTI_INT32 i)
    {
        return TestData_Cert_ZCopy_tSeq_get_reference(self, i);
    }

    template<>
    TestDataKeyed_Cert_ZCopy_t* DDS_TypedSampleSeq_get_reference<TestDataKeyed_Cert_ZCopy_t, TestDataKeyed_Cert_ZCopy_tSeq>(
        TestDataKeyed_Cert_ZCopy_tSeq* self, RTI_INT32 i)
    {
        return TestDataKeyed_Cert_ZCopy_tSeq_get_reference(self, i);
    }
#endif

    RTI_UINT32 get_interface_address(const char *interface_name)
    {
        struct ifaddrs * ifAddrStruct = NULL;
        struct ifaddrs * ifa = NULL;
        void * tmpAddrPtr = NULL;
        RTI_UINT32 address_hex = 0;

        getifaddrs(&ifAddrStruct);

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr) {
                continue;
            }
            if (strcmp(ifa->ifa_name, interface_name) != 0) {
                continue;
            }
#ifndef RTI_QNX
            if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
#endif
                // is a valid IP4 Address
                tmpAddrPtr = &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                address_hex = ntohl(((struct in_addr *)tmpAddrPtr)->s_addr);
#ifndef RTI_QNX
            }
#endif
        }
        if (ifAddrStruct!=NULL) freeifaddrs(ifAddrStruct);
        return address_hex;
    }

#ifndef RTI_CERT_IS_PI
    bool configureUDPv4Transport(
            RT_Registry_T *registry,
            PerftestTransport &transport,
            ParameterManager *_PM)
    {
        RTI_BOOL brc;
        struct UDP_InterfaceFactoryProperty* udp_property = (struct UDP_InterfaceFactoryProperty *)
                malloc(sizeof(struct UDP_InterfaceFactoryProperty));
        CHECK_PTR(udp_property, "malloc");
        *udp_property = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

        udp_property->max_message_size = 65536;
        udp_property->max_receive_buffer_size = _PM->get<int>("receiveBufferSize");
        udp_property->max_send_buffer_size = _PM->get<int>("sendBufferSize");

        brc = RT_Registry_unregister(registry, NETIO_DEFAULT_UDP_NAME, NULL, NULL);
        CHECK_BOOL(brc, "RT_Registry_unregister");

        /* Set interface to use if provided */
        if (!_PM->get<std::string>("allowInterfaces").empty()) {

            if (is_ip_address(_PM->get<std::string>("allowInterfaces"))) {
                fprintf(stderr,
                        "[Error]: Micro does not support providing the allowed interfaces\n"
                        "(-nic/-allowInterfaces) as an ip, provide the nic name instead\n"
                        "(value provided: %s)\n",
                        _PM->get<std::string>("allowInterfaces").c_str());
                return false;
            }

            brc = DDS_StringSeq_set_maximum(&udp_property->allow_interface,1);
            CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
            brc = DDS_StringSeq_set_length(&udp_property->allow_interface,1);
            CHECK_BOOL(brc, "DDS_StringSeq_set_length");
            *DDS_StringSeq_get_reference(&udp_property->allow_interface,0) = 
                DDS_String_dup(_PM->get<std::string>("allowInterfaces").c_str());

            RTI_UINT32 address_hex = get_interface_address(
                _PM->get<std::string>("allowInterfaces").c_str());

            if (!UDP_InterfaceTable_add_entry(
                    &udp_property->if_table,
                    address_hex,
                    0xff000000,
                    _PM->get<std::string>("allowInterfaces").c_str(),
                    UDP_INTERFACE_INTERFACE_UP_FLAG | UDP_INTERFACE_INTERFACE_MULTICAST_FLAG))
            {
                printf("failed to add interface\n");
            }
        }
        else
        {
            brc = DDS_StringSeq_set_maximum(&udp_property->allow_interface,2);
            CHECK_BOOL(brc, "DDS_StringSeq_set_maximum");
            brc = DDS_StringSeq_set_length(&udp_property->allow_interface,2);
            CHECK_BOOL(brc, "DDS_StringSeq_set_length");
            *DDS_StringSeq_get_reference(&udp_property->allow_interface,0) =
                DDS_String_dup("lo");
            *DDS_StringSeq_get_reference(&udp_property->allow_interface,1) =
                DDS_String_dup("eth0");

            if (!UDP_InterfaceTable_add_entry(
                    &udp_property->if_table,
                    0x7f000001,
                    0xff000000,
                    "lo",
                    UDP_INTERFACE_INTERFACE_UP_FLAG | UDP_INTERFACE_INTERFACE_MULTICAST_FLAG))
            {
                printf("failed to add interface\n");
            }
        }

        /* Re-register UDP under its default name */
        brc = RT_Registry_register(registry, NETIO_DEFAULT_UDP_NAME,
                UDP_InterfaceFactory_get_interface(),
                (struct RT_ComponentFactoryProperty*)udp_property, NULL);

        CHECK_BOOL(brc, "RT_Registry_register");

        transport.minimumMessageSizeMax = udp_property->max_message_size;

        return true;

    done:
        return false;
    }
#else
    bool configureUDPv4Transport(
            RT_Registry_T *registry,
            PerftestTransport &transport,
            ParameterManager *_PM)
    {
        RTI_BOOL brc;
        Udpv4_TransportProperties_T *udp_property = UDPv4_TransportProperties_new();
        CHECK_PTR(udp_property, "Udpv4_TransportProperties_new");

        /* Set interface to use if provided */
        if (!_PM->get<std::string>("allowInterfaces").empty()) {

            if (is_ip_address(_PM->get<std::string>("allowInterfaces"))) {
                fprintf(stderr,
                        "[Error]: Micro does not support providing the allowed interfaces\n"
                        "(-nic/-allowInterfaces) as an ip, provide the nic name instead\n"
                        "(value provided: %s)\n",
                        _PM->get<std::string>("allowInterfaces").c_str());
                return false;
            }

            RTI_UINT32 address_hex = get_interface_address(
                _PM->get<std::string>("allowInterfaces").c_str());

            if (!UDPv4_InterfaceTable_add_entry(
                    udp_property,
                    address_hex,
                    0xff000000,
                    _PM->get<std::string>("allowInterfaces").c_str(),
                    UDP_INTERFACE_INTERFACE_UP_FLAG | UDP_INTERFACE_INTERFACE_MULTICAST_FLAG))
            {
                printf("failed to add interface\n");
            }
        }
        else
        {
            if (!UDPv4_InterfaceTable_add_entry(
                    udp_property,
                    0x7f000001,
                    0xff000000,
                    "lo",
                    UDP_INTERFACE_INTERFACE_UP_FLAG | UDP_INTERFACE_INTERFACE_MULTICAST_FLAG))
            {
                printf("failed to add interface\n");
            }
        }

        /* Re-register UDP under its default name */
        brc = Udpv4_Interface_register(registry, NETIO_DEFAULT_UDP_NAME,
                udp_property);

        CHECK_BOOL(brc, "RT_Registry_register");

        return true;

    done:
        return false;
    }
#endif

    void configureDPResourceLimits(
        DDS_DomainParticipantQos& dp_qos)
    {
        dp_qos.resource_limits.max_destination_ports = 32;
        dp_qos.resource_limits.max_receive_ports = 32;
        dp_qos.resource_limits.local_topic_allocation = 3;
        dp_qos.resource_limits.local_type_allocation = 1;
        dp_qos.resource_limits.local_reader_allocation = 2;
        dp_qos.resource_limits.local_writer_allocation = 2;
        dp_qos.resource_limits.remote_participant_allocation = 8;
        dp_qos.resource_limits.remote_reader_allocation = 8;
        dp_qos.resource_limits.remote_writer_allocation = 8;
        dp_qos.resource_limits.local_publisher_allocation = 3;
        dp_qos.resource_limits.local_subscriber_allocation = 3;
    }
}

template class RTICertImpl<TestData_t, TestData_tSeq>;
template class RTICertImpl<TestDataKeyed_t, TestDataKeyed_tSeq>;
#ifdef RTI_ZEROCOPY_AVAILABLE
template class RTICertImpl_ZCopy<TestData_Cert_ZCopy_t, TestData_Cert_ZCopy_tSeq>;
template class RTICertImpl_ZCopy<TestDataKeyed_Cert_ZCopy_t, TestDataKeyed_Cert_ZCopy_tSeq>;
#endif