#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

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

#ifndef RTI_MICRO
#include "RTIDDSLoggerDevice.h"
#endif

#ifdef RTI_CUSTOM_TYPE
#include "CustomType.h"
#endif

#if defined(RTI_DARWIN) && !defined(RTI_MICRO)
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

    RTIDDSImpl() :
        _transport(),
      #ifdef RTI_SECURE_PERFTEST
        _security(),
      #endif
      #ifndef RTI_MICRO
        _loggerDevice(),
      #endif
        _parent(NULL)
    {
      #ifndef RTI_MICRO
        _instanceMaxCountReader = DDS_LENGTH_UNLIMITED;
        _sendQueueSize = 0;
      #else
        /*
         * For micro we want to restrict the use of memory, and since we need
         * to set a maximum (other than DDS_LENGTH_UNLIMITED), we decided to use
         * a default of 1. This means that for Micro, we need to specify the
         * number of instances that will be received in the reader side.
         */
        _instanceMaxCountReader = 1;
      #endif
        _isLargeData = false;
        _isFlatData = false;
        _isZeroCopy = false;
        _factory = NULL;
        _participant = NULL;
        _subscriber = NULL;
        _publisher = NULL;
        _reader = NULL;
        _typename = T::TypeSupport::get_type_name();
        _pongSemaphore = NULL;
        _PM = NULL;
        _qoSProfileNameMap[LATENCY_TOPIC_NAME] = std::string("LatencyQos");
        _qoSProfileNameMap[ANNOUNCEMENT_TOPIC_NAME]
                = std::string("AnnouncementQos");
        _qoSProfileNameMap[THROUGHPUT_TOPIC_NAME]
                = std::string("ThroughputQos");
    }

    ~RTIDDSImpl()
    {
        Shutdown();
    }

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

  #ifndef RTI_MICRO
    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic);
  #endif

    bool configureDomainParticipantQos(DDS_DomainParticipantQos &qos);

  #ifndef RTI_MICRO
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
  #ifndef RTI_MICRO
    RTIDDSLoggerDevice           _loggerDevice;
  #endif
    ParameterManager            *_PM;
    perftest_cpp                *_parent;
    std::map<std::string, std::string> _qoSProfileNameMap;

  #ifndef RTI_MICRO
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
    RTIDDSImpl_FlatData(bool isZeroCopy=false)
    {
    this->_isZeroCopy = isZeroCopy;
    this->_isFlatData = true;
    this->_typename = T::TypeSupport::get_type_name();
    };

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
  };
#endif // RTI_FLATDATA_AVAILABLE

#endif // __RTIDDSIMPL_H__
