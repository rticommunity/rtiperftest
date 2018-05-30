#ifndef __RTISOCKETIMPL_H__
#define __RTISOCKETIMPL_H__

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <algorithm>
#include <map>
#include <string>
#include "MessagingIF.h"
#include "PerftestTransport.h"
#include "RTIDDSImpl.h"
#include "perftest.h"
#include "perftestSupport.h"
#include "perftest_cpp.h"
#include "transport/transport_socketutil.h"
#include "transport/transport_udpv4.h"
#include "transport_tcp/transport_tcp_socketutil.h"
#include "transport_tcp/transport_tcp_tcpv4.h"

#include "perftestPlugin.h"

#define RTIPERFTEST_MAX_PEERS 1024
#define RTIPERFTEST_MAX_PORT_ATTEMPT 1024

class RTISocketImpl : public IMessaging {
  public:
    RTISocketImpl() : _transport() {
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

        _multicastAddrString = (char *)"239.255.1.1";

        _pongSemaphore = NULL;

        _plugin = NULL;
        _worker = NULL;
        _workerFactory = NULL;
        _exclusiveArea = NULL;

        // TODO: Decided how initialize this variables, not clear.
        //_nicAddress = NDDS_TRANSPORT_ADDRESS_INVALID;
        // _multicastAddrTransp = NDDS_TRANSPORT_ADDRESS_INVALID;

        _NumPublishers = 1;
        _NumSubscribers = 1;
    }

    ~RTISocketImpl() { Shutdown(); }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    unsigned int GetBatchSize() { return 0; }

    IMessagingWriter *CreateWriter(const char *topic_name);

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *
    CreateReader(const char *topic_name, IMessagingCB *callback);

    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic);

    bool ConfigureSocketsTransport();

    static double ObtainSerializeTimeCost(int iterations, unsigned int sampleSize);
    static double ObtainDeSerializeTimeCost(int iterations, unsigned int sampleSize);

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
    unsigned int _batchSize;
    int _peer_host_count;
    char *_peer_host[RTIPERFTEST_MAX_PEERS];

    /*TODO: Others solution to this:
     * 1 -> Use a unorderedMap (C++11) not possible
     * 2 -> Use a map with a custom comparator operator for the
     *      NDDS_Transport_Address_t type.
     *      std::map<NDDS_Transport_Address_t, int, dummyCmpStruct>
     */
    std::vector<std::pair<NDDS_Transport_Address_t, int> > _peersMap;

    PerftestTransport _transport;

    char *_multicastAddrString;

    RTIOsapiSemaphore *_pongSemaphore;

    NDDS_Transport_Plugin *_plugin;
    struct REDAWorker *_worker;
    struct REDAWorkerFactory *_workerFactory;
    struct REDAExclusiveArea *_exclusiveArea;
    NDDS_Transport_Address_t _nicAddress;
    NDDS_Transport_Address_t _multicastAddrTransp;

    int _NumPublishers;
    int _NumSubscribers;

  public:

    /*
     * Resources reserved by a participant
     * It's use to calculate the offset between the ports
     */
    static const int resources_per_participant;
};

char *InterfaceNameToAddress(const char *nicName);

int NDDS_Transport_UDPv4_get_num_multicast_interfaces(
        struct NDDS_Transport_UDP *plugin);

#endif // __RTISOCKETIMPL_H__
