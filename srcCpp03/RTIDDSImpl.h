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

template <typename T>
class RTIDDSImpl : public IMessaging
{
  public:

    RTIDDSImpl();

    ~RTIDDSImpl() 
    {
        Shutdown();
    }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    unsigned int GetBatchSize()
    {
        return _BatchSize;
    }

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

  private:

    int          _SendQueueSize;
    unsigned long _DataLen;
    int          _DomainID;
    const char  *_ProfileFile;
    bool         _TurboMode;
    bool         _UseXmlQos;
    bool         _AutoThrottle;
    bool         _IsReliable;
    bool         _IsMulticast;
    unsigned int _BatchSize;
    unsigned long _InstanceCount;
    long _InstanceMaxCountReader;
    int          _InstanceHashBuckets;
    int          _Durability;
    bool         _DirectCommunication;
    int          _KeepDurationUsec;
    bool         _UsePositiveAcks;
    bool         _LatencyTest;
    bool         _IsDebug;
    bool         _isLargeData;
    bool         _isScan;
    bool         _isPublisher;
    bool         _isDynamicData;
    bool         _IsAsynchronous;
    std::string  _FlowControllerCustom;
    unsigned long _useUnbounded;
    int          _peer_host_count;
    dds::core::StringSeq  _peer_host;
    bool         _useCft;
    long         _instancesToBeWritten;
    std::vector<unsigned int> _CFTRange;

    PerftestTransport _transport;

  #ifdef RTI_SECURE_PERFTEST
    bool _secureUseSecure;
    bool _secureIsSigned;
    bool _secureIsDataEncrypted; // user data
    bool _secureIsSMEncrypted;   // submessage
    bool _secureIsDiscoveryEncrypted;
    std::string _secureCertAuthorityFile;
    std::string _secureCertificateFile;
    std::string _securePrivateKeyFile;
    std::string _secureGovernanceFile;
    std::string _securePermissionsFile;
    std::string _secureLibrary;
    int  _secureDebugLevel;

    static const std::string SECURE_PRIVATEKEY_FILE_PUB;
    static const std::string SECURE_PRIVATEKEY_FILE_SUB;
    static const std::string SECURE_CERTIFICATE_FILE_PUB;
    static const std::string SECURE_CERTIFICATE_FILE_SUB;
    static const std::string SECURE_CERTAUTHORITY_FILE;
    static const std::string SECURE_PERMISION_FILE_PUB;
    static const std::string SECURE_PERMISION_FILE_SUB;
    static const std::string SECURE_LIBRARY_NAME;
  #endif

    int          _WaitsetEventCount;
    unsigned int _WaitsetDelayUsec;

    dds::core::Duration   _HeartbeatPeriod;
    dds::core::Duration   _FastHeartbeatPeriod;

    const char          *THROUGHPUT_MULTICAST_ADDR;
    const char          *LATENCY_MULTICAST_ADDR;
    const char          *ANNOUNCEMENT_MULTICAST_ADDR;
    const char          *_ProfileLibraryName;

    dds::domain::DomainParticipant _participant;
    dds::sub::Subscriber _subscriber;
    dds::pub::Publisher _publisher;

    rti::core::Semaphore _pongSemaphore;

  #ifdef RTI_SECURE_PERFTEST
    void configureSecurePlugin(std::map<std::string, std::string> &dpQosProperties);
    void printSecureArgs();
    void validateSecureArgs();
  #endif

};


#endif // __RTIDDSIMPL_H__
