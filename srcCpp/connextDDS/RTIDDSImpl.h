/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

#if defined(PERFTEST_RTI_PRO) || defined (PERFTEST_RTI_MICRO)

#include <stdexcept> // This header is part of the error handling library.
#include <string>
#include <algorithm>
#include <map>
#if defined(RTI_DARWIN) && defined(PERFTEST_RTI_PRO)
  #include <sys/types.h>
  #include <sys/sysctl.h>
#endif


#include "MessagingIF.h"
#include "perftestSupport.h"
#include "PerftestTransport.h"
#include "Infrastructure_common.h"
#ifdef PERFTEST_RTI_PRO
  #include "RTIDDSLoggerDevice.h"
#endif
#ifdef RTI_CUSTOM_TYPE
  #include "CustomType.h"
#endif

#define RTIPERFTEST_MAX_PEERS 1024

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

const std::string GetMiddlewareVersionString();

/* Class for the DDS_DynamicDataMemberId of the type of RTI Perftest*/
class DynamicDataMembersId
{
private:
    std::map<std::string, int> membersId;
    DynamicDataMembersId();

public:
    ~DynamicDataMembersId();
    static DynamicDataMembersId &GetInstance();
    int at(std::string key);
};

template <typename T>
class RTIDDSImpl : public IMessaging
{
public:

    RTIDDSImpl();

    void shutdown();

    ~RTIDDSImpl()
    {
        shutdown();
    }

    bool initialize(ParameterManager &PM, perftest_cpp *parent);

    bool validate_input();
    unsigned long get_initial_burst_size();
    IMessagingWriter *create_writer(const char *topic_name);
    /*
     * Pass null for callback if using IMessagingSubscriber.receive_message()
     * to get data
     */
    IMessagingReader *create_reader(const char *topic_name, IMessagingCB *callback);
    std::string print_configuration();
    bool supports_listeners()
    {
        return true;
    };

  #ifdef PERFTEST_RTI_PRO

    /**
     * @brief This function calculates the overhead bytes that all the
     * members on TestData_* type add excluding the content of the sequence.
     *
     * @param size \b InOut. The size of the overhead of the data type.
     *
     * @return true if the operation was successful, otherwise false.
     */
    virtual bool get_serialized_overhead_size(unsigned int &overhead_size);

    /*
     * These two functions calculate the serialization/deserialization time cost
     * with a precision of microseconds.
     */
    static double obtain_dds_serialize_time_cost(
            unsigned int sampleSize,
            unsigned int iters = 1000);

    static double obtain_dds_deserialize_time_cost(
            unsigned int sampleSize,
            unsigned int iters = 1000);
  #endif

protected:

    void configure_middleware_verbosity(int verbosity_level);
    bool data_size_related_calculations();

  #ifdef PERFTEST_RTI_PRO
    DDSTopicDescription *create_cft(const char *topic_name, DDSTopic *topic);
    unsigned long int getShmemSHMMAX();
  #endif

    bool configure_participant_qos(DDS_DomainParticipantQos &qos);
    bool configure_reader_qos(
            DDS_DataReaderQos &dr_qos,
            std::string qos_profile,
            std::string topic_name);
    bool configure_writer_qos(
            DDS_DataWriterQos &dw_qos,
            std::string qos_profile,
            std::string topic_name);

    const std::string get_qos_profile_name(const char *topicName);

protected:

    // This Mutex is used in VxWorks to synchronize when finalizing the factory
    static PerftestMutex        *_finalizeFactoryMutex;

    long                         _instanceMaxCountReader;
    unsigned long                _sendQueueSize;
    bool                         _isLargeData;
    unsigned long long           _maxSynchronousSize;
    bool                         _isFlatData;
    bool                         _isZeroCopy;
    PerftestTransport            _transport;
  #ifdef RTI_SECURE_PERFTEST
    PerftestSecurity             _security;
  #endif
    DDSDomainParticipantFactory *_factory;
    DDSDomainParticipant        *_participant;
    DDSSubscriber               *_subscriber;
    DDSPublisher                *_publisher;
    DDSDataReader               *_reader;
    const char                  *_typename;
    PerftestSemaphore           *_pongSemaphore;
  #ifdef PERFTEST_RTI_PRO
    RTIDDSLoggerDevice           _loggerDevice;
    #ifdef PERFTEST_CONNEXT_PRO_610
    bool                         _isNetworkCapture;
    std::string                  _networkCaptureOutputFile;
    #endif
  #endif
    ParameterManager            *_PM;
    perftest_cpp                *_parent;
    std::map<std::string, std::string> _qoSProfileNameMap;

  #if defined(PERFTEST_RTI_PRO) && defined(PERFTEST_CONNEXT_PRO_610)
    // Parameters that configure the network capture
    struct NDDS_Utility_NetworkCaptureParams_t _networkCaptureParams;
  #endif

public:


};

#ifdef RTI_FLATDATA_AVAILABLE
/**
 * Overwrites CreateWriter and CreateReader from RTIDDSImpl
 * to return Writers and Readers that make use of FlatData API
 */
template <typename T>
class RTIDDSImpl_FlatData: public RTIDDSImpl<TestData_t> {
public:
    /**
     * Constructor for RTIDDSImpl_FlatData
     *
     * @param isZeroCopy states if the type is also ZeroCopy
     */
    RTIDDSImpl_FlatData(bool isZeroCopy=false);

    /**
     * Creates a Publisher that uses the FlatData API
     *
     * @param topic_name is the name of the topic where
     *      the created writer will write new samples to
     *
     * @return a RTIFlatDataPublisher
     */
    IMessagingWriter *create_writer(const char *topic_name);

    /**
     * Creates a Subscriber that uses the FlatData API
     *
     * @param topic_name is the name of the topic where the created reader
     *      will read new samples from
     *
     * @param callback is the callback that will process the receibed
     *      message once it has been taken by the reader. Pass null for
     *      callback if using IMessagingSubscriber.receive_message() to get
     *      data
     *
     * @return a RTIFlatDataSubscriber
     */
    IMessagingReader *create_reader(const char *topic_name, IMessagingCB *callback);

    /**
     * Obtain average serialization time
     *
     * @param sampleSize size of the payload to serialize
     * @param iters number of times to serialize the payload
     *
     * @return average serialization time in microseconds
     */
    static double obtain_dds_serialize_time_cost_override(
        unsigned int sampleSize,
        unsigned int iters = 1000);

    /**
     * Obtain average deserialization time
     *
     * @param sampleSize size of the payload to deserialize
     * @param iters number of times to deserialize the payload
     *
     * @return average serialization time in microseconds
     */
    static double obtain_dds_deserialize_time_cost_override(
        unsigned int sampleSize,
        unsigned int iters = 1000);

    /**
     * @brief This function calculates the overhead bytes added by all the
     * members on the TestData_* type, excluding the content of the sequence.
     *
     * @param size \b InOut. The size of the overhead of the data type.
     *
     * @return true if the operation was successful, otherwise false.
     */
    bool get_serialized_overhead_size(unsigned int &overhead_size);
};
#endif // RTI_FLATDATA_AVAILABLE

#endif // PERFTEST_RTI_PRO || PERFTEST_RTI_MICRO
#endif // __RTIDDSIMPL_H__
