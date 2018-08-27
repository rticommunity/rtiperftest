/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

#include <iostream>
#include <vector>
#include "perftest.hpp"
#include "MessagingIF.h"
#include <sstream>
#ifdef RTI_SECURE_PERFTEST
#include "security/security_default.h"
#endif
#include "rti/config/Logger.hpp"
#include "PerftestTransport.h"

#define RTIPERFTEST_MAX_PEERS 1024

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

    bool validate_input();

    std::string PrintConfiguration();

    bool Initialize(ParameterManager &PM);

    void Shutdown();

    unsigned long GetInitializationSampleCount();

    IMessagingWriter *CreateWriter(const std::string &topic_name);
    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *CreateReader(const std::string &topic_name, IMessagingCB *callback);

    dds::core::QosProvider getQosProviderForProfile(
            const std::string &library_name,
            const std::string &profile_name);

    template <typename U>
    dds::topic::ContentFilteredTopic<U> CreateCft(
        const std::string &topic_name,
        const dds::topic::Topic<U> &topic);

    const std::string get_qos_profile_name(std::string topicName);


  private:

    long _InstanceMaxCountReader;
    int _InstanceHashBuckets;
    bool _isLargeData;
    PerftestTransport _transport;

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


    dds::domain::DomainParticipant _participant;
    dds::sub::Subscriber _subscriber;
    dds::pub::Publisher _publisher;

    rti::core::Semaphore _pongSemaphore;
    ParameterManager *_PM;
    std::map<std::string, std::string> _qoSProfileNameMap;

  #ifdef RTI_SECURE_PERFTEST
    void configureSecurePlugin(std::map<std::string, std::string> &dpQosProperties);
    std::string printSecureArgs();
    void validateSecureArgs();
  #endif

};


#endif // __RTIDDSIMPL_H__
