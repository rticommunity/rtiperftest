#ifndef __RTISOCKETIMPL_H__
#define __RTISOCKETIMPL_H__

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
#include "RTIDDSImpl.h"
#include "perftest.h"
#include "perftest_cpp.h"

#define RTIPERFTEST_MAX_PEERS 1024

class RTISocketImpl : public IMessaging
{
  public:
    RTISocketImpl() : _transport()
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

        _pongSemaphore = NULL;

    /**************************************************************************/
    /*****************************sockets stuff********************************/

        /*TODO retrieve shmem, sendNic and receiveNic from the tramsport parameters*/
        /*TODO Shared memory does not work right now*/
        _useShmem = 0; /* 0: UDPv4 1: SHMEM */

        _nic = std::string("127.0.0.1");
        _plugin = NULL;
        _worker = NULL;
        std::string no_socket_params[] = {
            "-unbounded",
            "-sendQueueSize",
            "-heartbeatPeriod",
            "-fastHeartbeatPeriod",
            "-qosFile",
            "-qosLibrary",
            "-durability",
            "-dynamicData",
            "-noDirectCommunication",
            "-instances",
            "-instanceHashBuckets",
            "-batchSize",
            "-keepDurationUsec",
            "-noPositiveAcks",
            "-waitsetDelayUsec",
            "-waitsetEventCount",
            "-enableAutoThrottle",
            "-enableTurboMode",
            "-noXmlQos",
            "-asynchronous",
            "-flowController",
            "-cft",
            "-writeInstance",
            "-enableTCP",
            "-enableUDPv6",
            "-allowInterfaces",
            "-transportServerBindPort",
            "-transportWan",
            "-transportCertAuthority",
            "-transportCertFile",
            "-transportPrivateKey",
            "-transportWanServerAddress",
            "-transportWanServerPort",
            "-transportWanId",
            "-transportSecureWan"
        };

        /**************************************************************************/
        /**************************************************************************/
    }

    ~RTISocketImpl()
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
    int _SendQueueSize;
    unsigned long _DataLen;
    int _DomainID;
    const char *_ProfileFile;
    bool _TurboMode;
    bool _UseXmlQos;
    bool _AutoThrottle;
    bool _IsReliable;
    bool _IsMulticast;
    unsigned int _BatchSize;
    unsigned long _InstanceCount;
    unsigned long _InstanceMaxCountReader;
    int _InstanceHashBuckets;
    int _Durability;
    bool _DirectCommunication;
    int _KeepDurationUsec;
    bool _UsePositiveAcks;
    bool _LatencyTest;
    bool _IsDebug;
    bool _isLargeData;
    bool _isScan;
    bool _isPublisher;
    bool _isDynamicData;
    bool _IsAsynchronous;
    std::string _FlowControllerCustom;
    unsigned long _useUnbounded;
    int _peer_host_count;
    char *_peer_host[RTIPERFTEST_MAX_PEERS];
    bool _useCft;
    long _instancesToBeWritten;
    unsigned int _CFTRange[2];

    PerftestTransport _transport;

    DDS_Duration_t _HeartbeatPeriod;
    DDS_Duration_t _FastHeartbeatPeriod;

    const char *THROUGHPUT_MULTICAST_ADDR;
    const char *LATENCY_MULTICAST_ADDR;
    const char *ANNOUNCEMENT_MULTICAST_ADDR;
    const char *_ProfileLibraryName;

    DDSDomainParticipantFactory *_factory;
    DDSDomainParticipant *_participant;
    DDSSubscriber *_subscriber;
    DDSPublisher *_publisher;
    DDSDataReader *_reader;
    const char *_typename;

    RTIOsapiSemaphore *_pongSemaphore;

    /**************************************************************************/
    /*****************************sockets stuff********************************/

    int _useShmem;
    std::string _nic;
    /*TODO decide if is necesary only one Nic or one for send and another for recive*/
    // char *_sendNic;
    // char *_receiveNic;
    NDDS_Transport_Plugin *_plugin;
    struct REDAWorker *_worker;

    std::string no_socket_params[];

    /**************************************************************************/
    /**************************************************************************/

  public:
    static int _WaitsetEventCount;
    static unsigned int _WaitsetDelayUsec;
};

#endif // __RTISOCKETIMPL_H__
