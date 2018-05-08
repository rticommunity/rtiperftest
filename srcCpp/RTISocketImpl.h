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
#include "transport/transport_socketutil.h"
#include "perftestPlugin.h"

#define RTIPERFTEST_MAX_PEERS 1024

class RTISocketImpl : public IMessaging
{
  public:
    RTISocketImpl() : _transport()
    {
        _DataLen = 100;
        _DomainID = 1;
        _IsReliable = false;
        _IsMulticast = false;
        _DirectCommunication = true;
        _LatencyTest = false;
        _IsDebug = false;
        _isLargeData = false;
        _isScan = false;
        _isPublisher = false;
        _peer_host_count = 0;
        _batchSize = 0;

        THROUGHPUT_MULTICAST_ADDR = "239.255.1.1";
        LATENCY_MULTICAST_ADDR = "239.255.1.2";
        ANNOUNCEMENT_MULTICAST_ADDR = "239.255.1.100";

        _pongSemaphore = NULL;

        _plugin = NULL;
        _worker = NULL;
        _workerFactory = NULL;
        _exclusiveArea = NULL;

        _NumPublishers = 1;
        _NumSubscribers = 1;

    }

    ~RTISocketImpl()
    {
        Shutdown();
    }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    unsigned int GetBatchSize() { return 0; }

    IMessagingWriter *CreateWriter(const char *topic_name);

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic);

    bool configureSocketsTransport();

  private:
    unsigned long _DataLen;
    int _DomainID;
    bool _IsMulticast;
    bool _IsReliable;
    int _Durability;
    bool _DirectCommunication;
    bool _LatencyTest;
    bool _IsDebug;
    bool _isLargeData;
    bool _isScan;
    bool _isPublisher;
    int _peer_host_count;
    unsigned int _batchSize;
    char *_peer_host[RTIPERFTEST_MAX_PEERS];

    PerftestTransport _transport;

    const char *THROUGHPUT_MULTICAST_ADDR;
    const char *LATENCY_MULTICAST_ADDR;
    const char *ANNOUNCEMENT_MULTICAST_ADDR;

    RTIOsapiSemaphore *_pongSemaphore;

    NDDS_Transport_Plugin *_plugin;
    struct REDAWorker *_worker;
    struct REDAWorkerFactory *_workerFactory;
    struct REDAExclusiveArea *_exclusiveArea;

    int _NumPublishers;
    int _NumSubscribers;

  public:
    static int _WaitsetEventCount;
    static unsigned int _WaitsetDelayUsec;

    /*
     * Resources reserved by a participant
     * It's use to calculate the offset between the ports
     */
    static const int resources_per_participant;
};

#endif // __RTISOCKETIMPL_H__
