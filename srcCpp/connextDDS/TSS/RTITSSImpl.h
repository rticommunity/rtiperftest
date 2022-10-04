#ifndef __RTITSSIMPL_H__
#define __RTITSSIMPL_H__

#ifdef RTI_PERF_TSS

#include "Infrastructure_common.h"
#include "MessagingIF.h"

#include "perftest_cpp.h"
#include "perftest.hpp"
#include "perftestSupport.h"
#include "perftestPlugin.h"
#include "FACE/DM/TestData_t/TypedTS_Impl.hpp"
#include "FACE/DM/TestDataLarge_t/TypedTS_Impl.hpp"
#include "FACE/DM/TestDataKeyed_t/TypedTS_Impl.hpp"
#include "FACE/DM/TestDataKeyedLarge_t/TypedTS_Impl.hpp"
#include "RTI/TSS/Base.hpp"

#include "CustomQosSupport.hpp"

#ifdef RTI_CONNEXT_MICRO
#include "dds_c/dds_c_config.h"
#define RTI_TSS_CONNEXT_VERSION "RTI Connext DDS Micro "            \
                                STR(RTIME_DDS_VERSION_MAJOR) "."    \
                                STR(RTIME_DDS_VERSION_MINOR) "."    \
                                STR(RTIME_DDS_VERSION_REVISION) "." \
                                STR(RTIME_DDS_VERSION_RELEASE)
#else
#include "ndds/ndds_version.h"
#define STR_HELPER(x_) #x_
#define STR(x_) STR_HELPER(x_)
#define RTI_TSS_CONNEXT_VERSION "RTI Connext DDS Professional "     \
                                STR(RTI_DDS_VERSION_MAJOR) "."      \
                                STR(RTI_DDS_VERSION_MINOR) "."      \
                                STR(RTI_DDS_VERSION_RELEASE) "."   \
                                STR(RTI_DDS_VERSION_REVISION)
#endif

#define RTI_TSS_VERSION         "RTI Connext TSS "            \
                                STR(RTI_TSS_VERSION_MAJOR) "."    \
                                STR(RTI_TSS_VERSION_MINOR) "."    \
                                STR(RTI_TSS_VERSION_REVISION) "." \
                                STR(RTI_TSS_VERSION_RELEASE)

#define RTI_TSS_VER_INFO_STR "RTI_TSS_VERSION: " \
    RTI_TSS_VERSION " using "         \
    RTI_TSS_CONNEXT_VERSION

#define RTIPERFTEST_MAX_PEERS 1024

const std::string GetMiddlewareVersionString();

template <class Type, class TypedCB>
class TSSListener : public TypedCB
{
protected:
    IMessagingCB *_callback;
    TestMessage _message;

public:
    TSSListener(IMessagingCB *callback)
        : _callback(callback), _message() {}

    virtual void Callback_Handler(
            FACE::TSS::CONNECTION_ID_TYPE connection_id,
            FACE::TSS::TRANSACTION_ID_TYPE transaction_id,
            const Type& sample,
            const FACE::TSS::HEADER_TYPE& header,
            const FACE::TSS::QoS_EVENT_TYPE& qos_parameters,
            FACE::RETURN_CODE_TYPE::Value& return_code);
};

template <class Type, class TypedCB>
class TSSListenerLoaning : public TSSListener<Type, TypedCB>
{
public:
    TSSListenerLoaning(IMessagingCB *callback)
        : TSSListener<Type, TypedCB>(callback) {};

    void Callback_Handler(
            FACE::TSS::CONNECTION_ID_TYPE connection_id,
            FACE::TSS::TRANSACTION_ID_TYPE transaction_id,
            const Type& sample,
            const FACE::TSS::HEADER_TYPE& header,
            const FACE::TSS::QoS_EVENT_TYPE& qos_parameters,
            FACE::RETURN_CODE_TYPE::Value& return_code) override;
};

template <class Type, class TypedTS, class TypedCB>
class TSSConnection
{
private:
    FACE::TSS::CONNECTION_ID_TYPE _connection_id;
    TypedTS *_typedTS;
    TSSListener<Type, TypedCB> *_typedCB;

    TestMessage _message;
    Type _sample;

    unsigned long _num_instances;
    long _instancesToBeWritten;
    unsigned long _instance_counter;

    void _registerCallBack(IMessagingCB *callback);

protected:
    DDS_DataWriter *_writer;
    DDS_DataReader *_reader;

public:
    TSSConnection(FACE::TSS::CONNECTION_ID_TYPE connection_id,
                  unsigned long num_instances,
                  long instancesToBeWritten,
                  bool loaning,
                  IMessagingCB *callback = NULL);
    ~TSSConnection();

    // Declared in public section because these functions must be accessible
    // from the subclasses
    inline bool _send(const TestMessage &message,
                      bool isCftWildCardKey);
    inline bool _send_loaning(const TestMessage &message,
                              bool isCftWildCardKey);
    inline TestMessage* _receive();
    inline TestMessage* _receive_loaning();
};

template <class Type, class TypedTS, class TypedCB>
class RTITSSImpl : public IMessaging
{
private:
    std::vector<FACE::TSS::CONNECTION_ID_TYPE> _connections;
    RTI::TSS::Base *_tss;
    PerftestSemaphore *_pong_semaphore;
    PerftestTransport _transport;
    ParameterManager *_pm;

    FACE::TSS::CONNECTION_ID_TYPE _createConnection(
            std::string topic_name,
            std::string name,
            FACE::RETURN_CODE_TYPE::Value &retcode);

    /* NOTE: This method is type-specific. It has to be
     * defined for each type specifically.
     */
    int _serializeTyped(Type *data, unsigned int &size);

    const char* _type_name;

    FACE::Configuration* _customConfig;

    std::map<std::string, std::string> _qoSProfileNameMap;

    const std::string get_qos_profile_name(const char *topicName);

    void configure_middleware_verbosity(int verbosity_level);

    long _instanceMaxCountReader;

    bool _isLargeData;

    unsigned long long _maxSynchronousSize;

    DDS_Boolean
    GetSystemConfig(
        FACE::Configuration *configuration,
        const FACE::CONFIGURATION_RESOURCE &name,
        RTI_TSS_System_Configuration_T *&sys_cfg);

    DDS_Boolean
    SetSystemConfig(
        FACE::Configuration *configuration,
        const FACE::CONFIGURATION_RESOURCE &name);

    DDS_Boolean
    SetDomainParticipantConfig(const char* domain_cfg_name);

    DDS_Boolean
    InstrumentConnection(
            const FACE::TSS::CONNECTION_NAME_TYPE &connection_name,
            const char* topic_name);

    DDS_Boolean
    GetConnectionConfig(
            const FACE::TSS::CONNECTION_NAME_TYPE &connection_name,
            RTI_TSS_Connection_Configuration_T *&connection_config);

public:
    RTITSSImpl(const char* type_name);

    ~RTITSSImpl() { delete _tss; };

    bool initialize(ParameterManager &PM, perftest_cpp *parent);

    bool validate_input();

    void shutdown();

    std::string print_configuration();

    IMessagingWriter *create_writer(const char *topic_name);
    IMessagingReader *create_reader(const char *topic_name, IMessagingCB *callback);

    bool supports_listener() { return true; }
    bool supports_discovery() { return true; }

    unsigned long get_initial_burst_size();
#ifdef RTI_PERF_TSS_PRO
    bool get_serialized_overhead_size(unsigned int &overhead_size);
#endif

    bool supports_listeners() { return true; }
};

template <class Type, class TypedTS, class TypedCB>
class RTITSSPublisher : public IMessagingWriter,
                        public TSSConnection<Type, TypedTS, TypedCB>
{
private:
    PerftestSemaphore *_pong_semaphore;
    bool _is_reliable;

    bool (TSSConnection<Type, TypedTS, TypedCB>::*_send_function)(const TestMessage&, bool);

public:
    RTITSSPublisher(FACE::TSS::CONNECTION_ID_TYPE connection_id,
            unsigned long num_instances,
            PerftestSemaphore * pongSemaphore,
            long instancesToBeWritten,
            bool loaning);

    bool send(const TestMessage &message, bool isCftWildCardKey = false);
    void flush() { /* dummy */ }

    void wait_for_ack(int sec, unsigned int nsec);
    void wait_for_readers(int numSubscribers);
    bool wait_for_ping_response();
    bool wait_for_ping_response(int timeout);
    bool notify_ping_response();

    unsigned int get_pulled_sample_count();
    unsigned int get_sample_count();
    unsigned int get_sample_count_peak();
};

template <class Type, class TypedTS, class TypedCB>
class RTITSSSubscriber : public IMessagingReader,
                         public TSSConnection<Type, TypedTS, TypedCB>
{
private:
    PerftestSemaphore *_pong_semaphore;

    TestMessage* (TSSConnection<Type, TypedTS, TypedCB>::*_receive_function)();

public:
    RTITSSSubscriber(FACE::TSS::CONNECTION_ID_TYPE connection_id,
                    unsigned long num_instances,
                    PerftestSemaphore * pongSemaphore,
                    long instancesToBeWritten,
                    IMessagingCB *callback,
                    bool loaning);

    TestMessage *receive_message();

    void wait_for_writers(int numPublishers);

    unsigned int get_sample_count();
    unsigned int get_sample_count_peak();
};

#endif /* RTI_PERF_TSS */
#endif /* __RTITSSIMPL_H__ */