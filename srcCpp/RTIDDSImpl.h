#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <string>
#include <algorithm>
#include <map>
#include "MessagingIF.h"
#include "perftestSupport.h"
#include "PerftestTransport.h"

#define RTIPERFTEST_MAX_PEERS 1024

template <typename T>
class RTIDDSImpl : public IMessaging
{
  public:

    RTIDDSImpl() :
        _transport()
    {
        _SendQueueSize = 50;
        _DataLen = 100;
        _DomainID = 1;
        _ProfileFile = "perftest_qos_profiles.xml";
        _AutoThrottle = false;
        _TurboMode = false;
        _UseXmlQos = true;
        _IsReliable = true;
        _IsMulticast = false;
        _BatchSize = 0;
        _InstanceCount = 1;
        _InstanceMaxCountReader = DDS_LENGTH_UNLIMITED;
        _InstanceHashBuckets = -1;
        _Durability = DDS_VOLATILE_DURABILITY_QOS;
        _DirectCommunication = true;
        _KeepDurationUsec = -1;
        _UsePositiveAcks = true;
        _LatencyTest = false;
        _IsDebug = false;
        _isLargeData = false;
        _isScan = false;
        _isPublisher = false;
        _isDynamicData = false;
        _IsAsynchronous = false;
        _FlowControllerCustom = "default";
        _useUnbounded = 0;
        _peer_host_count = 0;
        _useCft = false;
        _instancesToBeWritten = -1; // By default use round-robin (-1)
        _CFTRange[0] = 0;
        _CFTRange[1] = 0;

      #ifdef RTI_SECURE_PERFTEST
        _secureUseSecure = false;
        _secureIsSigned = false;
        _secureIsDataEncrypted = false;
        _secureIsSMEncrypted = false;
        _secureIsDiscoveryEncrypted = false;
        _secureDebugLevel = -1;
      #endif

        _HeartbeatPeriod.sec = 0;
        _HeartbeatPeriod.nanosec = 0;
        _FastHeartbeatPeriod.sec = 0;
        _FastHeartbeatPeriod.nanosec = 0;

        THROUGHPUT_MULTICAST_ADDR = "239.255.1.1";
        LATENCY_MULTICAST_ADDR = "239.255.1.2";
        ANNOUNCEMENT_MULTICAST_ADDR = "239.255.1.100";
        _ProfileLibraryName = "PerftestQosLibrary";

        _factory = NULL;
        _participant = NULL;
        _subscriber = NULL;
        _publisher = NULL;
        _reader = NULL;
        _typename = T::TypeSupport::get_type_name();

        _pongSemaphore = NULL;
    }

    ~RTIDDSImpl()
    {
        Shutdown();
    }

    void PrintCmdLineHelp();


    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    unsigned int GetBatchSize() { return _BatchSize; }

    IMessagingWriter *CreateWriter(const char *topic_name);

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic);


  private:

    // Specific functions to configure the Security plugin
  #ifdef RTI_SECURE_PERFTEST
    bool configureSecurePlugin(DDS_DomainParticipantQos& dpQos);
    void printSecureArgs();
    bool validateSecureArgs();
  #endif


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
    unsigned long _InstanceMaxCountReader;
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
    char *       _peer_host[RTIPERFTEST_MAX_PEERS];
    bool         _useCft;
    long _instancesToBeWritten;
    unsigned int _CFTRange[2];

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

    DDS_Duration_t   _HeartbeatPeriod;
    DDS_Duration_t   _FastHeartbeatPeriod;

    const char          *THROUGHPUT_MULTICAST_ADDR;
    const char          *LATENCY_MULTICAST_ADDR;
    const char          *ANNOUNCEMENT_MULTICAST_ADDR;
    const char          *_ProfileLibraryName;

    DDSDomainParticipantFactory *_factory;
    DDSDomainParticipant        *_participant;
    DDSSubscriber               *_subscriber;
    DDSPublisher                *_publisher;
    DDSDataReader               *_reader;
    const char                  *_typename;

    RTIOsapiSemaphore *_pongSemaphore;

  public:

    static int          _WaitsetEventCount;
    static unsigned int _WaitsetDelayUsec;
};


#endif // __RTIDDSIMPL_H__

