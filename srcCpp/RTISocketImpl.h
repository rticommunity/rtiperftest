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
        _typename = T::TypeSupport::get_type_name();

        _pongSemaphore = NULL;

    /**************************************************************************/
    /*****************************sockets stuff********************************/

        fast_queue = 0;
        domainId = 56;
        sample_count = -1; /* -1: Infinite */
        skip_dds = 0;
        use_shmem = 0; /* 0: UDPv4 1: SHMEM */
        payload_size = 256;
        debug = 0;
        execution_time = 30; /* 30 seconds */
        *send_nic = "127.0.0.1";
        *receive_nic = "127.0.0.1";
        plugin = NULL;

    /**************************************************************************/
    /**************************************************************************/
    }

    ~RTIDDSImpl()
    {
        Shutdown();
    }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    //unsigned int GetBatchSize() { return _BatchSize; }

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
    int fast_queue;
    int domainId;
    int sample_count;
    int skip_dds;
    int use_shmem;
    int payload_size;
    int debug;
    int execution_time;
    char *send_nic;
    char *receive_nic;
    NDDS_Transport_Plugin *plugin;

    /**************************************************************************/
    /**************************************************************************/

  public:
    static int _WaitsetEventCount;
    static unsigned int _WaitsetDelayUsec;
};

#endif // __RTISOCKETIMPL_H__
