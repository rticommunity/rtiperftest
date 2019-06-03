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

#ifndef RTI_MICRO
#include "RTIDDSLoggerDevice.h"
#endif

#ifdef RTI_CUSTOM_TYPE
#include "CustomType.h"
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

private:
    // This Mutex is used in VxWorks to synchronize when finalizing the factory
    static PerftestMutex     *_finalizeFactoryMutex;

    long                         _instanceMaxCountReader;
    bool                         _isLargeData;
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
};

#endif // __RTIDDSIMPL_H__
