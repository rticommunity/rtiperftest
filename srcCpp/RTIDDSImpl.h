#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

#if defined(PERTEST_RTI_PRO) || defined (PERTEST_RTI_MICRO)

/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <stdexcept> // This header is part of the error handling library.
#include <string>
#include <algorithm>
#include <map>
#include "MessagingIF.h"
#include "perftestSupport.h"
#include "PerftestTransport.h"
#include "Infrastructure_common.h"

#ifdef RTI_ZEROCOPY_AVAILABLE
#include "perftest_ZeroCopySupport.h"
#endif

#ifndef PERTEST_RTI_MICRO
#include "RTIDDSLoggerDevice.h"
#endif

#ifdef RTI_CUSTOM_TYPE
#include "CustomType.h"
#endif

#if defined(RTI_DARWIN) && !defined(PERTEST_RTI_MICRO)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#define RTIPERFTEST_MAX_PEERS 1024

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

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

    ~RTIDDSImpl()
    {
        Shutdown();
    }


    bool data_size_related_calculations();

    bool validate_input();

    std::string PrintConfiguration();

    bool Initialize(ParameterManager &PM, perftest_cpp *parent);

    void Shutdown();

    unsigned long GetInitializationSampleCount();

    IMessagingWriter *CreateWriter(const char *topic_name);

    /*
     * Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
     * to get data
     */
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

  #ifndef PERTEST_RTI_MICRO
    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic);
  #endif

    bool configureDomainParticipantQos(DDS_DomainParticipantQos &qos);

  #ifndef PERTEST_RTI_MICRO

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

    bool supports_listener()
    {
        return true;
    };

    bool supports_discovery()
    {
        return true;
    };

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
  #ifndef PERTEST_RTI_MICRO
    RTIDDSLoggerDevice           _loggerDevice;
  #endif
    ParameterManager            *_PM;
    perftest_cpp                *_parent;
    std::map<std::string, std::string> _qoSProfileNameMap;

  #ifndef PERTEST_RTI_MICRO
    unsigned long int getShmemSHMMAX();
  #endif
    bool setup_DR_QoS(
            DDS_DataReaderQos &dr_qos,
            std::string qos_profile,
            std::string topic_name);
    bool setup_DW_QoS(
            DDS_DataWriterQos &dw_qos,
            std::string qos_profile,
            std::string topic_name);
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
    IMessagingWriter *CreateWriter(const char *topic_name);

    /**
     * Creates a Subscriber that uses the FlatData API
     *
     * @param topic_name is the name of the topic where the created reader
     *      will read new samples from
     *
     * @param callback is the callback that will process the receibed
     *      message once it has been taken by the reader. Pass null for
     *      callback if using IMessagingSubscriber.ReceiveMessage() to get
     *      data
     *
     * @return a RTIFlatDataSubscriber
     */
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

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

#endif // PERTEST_RTI_PRO
#endif // __RTIDDSIMPL_H__
