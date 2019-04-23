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
#include "RTIDDSLoggerDevice.h"

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
        _parent(NULL),
        _loggerDevice()
    {
        _instanceMaxCountReader = DDS_LENGTH_UNLIMITED;
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

    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic);

    const std::string get_qos_profile_name(const char *topicName);

private:

    // Specific functions to configure the Security plugin
  #ifdef RTI_SECURE_PERFTEST
    bool configureSecurePlugin(DDS_DomainParticipantQos& dpQos);
    std::string printSecureArgs();
    bool validateSecureArgs();
  #endif

    long                         _instanceMaxCountReader;
    bool                         _isLargeData;
    PerftestTransport            _transport;
    DDSDomainParticipantFactory *_factory;
    DDSDomainParticipant        *_participant;
    DDSSubscriber               *_subscriber;
    DDSPublisher                *_publisher;
    DDSDataReader               *_reader;
    const char                  *_typename;
    RTIOsapiSemaphore           *_pongSemaphore;
    RTIDDSLoggerDevice           _loggerDevice;
    ParameterManager            *_PM;
    perftest_cpp                *_parent;
    std::map<std::string, std::string> _qoSProfileNameMap;

  #ifdef RTI_SECURE_PERFTEST
    static const std::string SECURE_PRIVATEKEY_FILE_PUB;
    static const std::string SECURE_PRIVATEKEY_FILE_SUB;
    static const std::string SECURE_CERTIFICATE_FILE_PUB;
    static const std::string SECURE_CERTIFICATE_FILE_SUB;
    static const std::string SECURE_CERTAUTHORITY_FILE;
    static const std::string SECURE_PERMISION_FILE_PUB;
    static const std::string SECURE_PERMISION_FILE_SUB;
    static const std::string SECURE_LIBRARY_NAME;
  #endif

};

#endif // __RTIDDSIMPL_H__

