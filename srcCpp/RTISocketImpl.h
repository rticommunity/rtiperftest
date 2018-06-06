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
#include "RTIDDSImpl.h" //TODO: remove
#include "perftest.h"
#include "perftestSupport.h"
#include "perftest_cpp.h"
#include "transport/transport_socketutil.h"
#include "transport/transport_udpv4.h"
#include "transport_tcp/transport_tcp_socketutil.h"
#include "transport_tcp/transport_tcp_tcpv4.h"//TODO: remove

#include "perftestPlugin.h"

#define RTIPERFTEST_MAX_PEERS 1024
#define RTIPERFTEST_MAX_PORT_ATTEMPT 1024

class RTISocketPublisher;
class RTISocketSubscriber;
class RTISocketImpl : public IMessaging { //TODO: move to cxx
  friend RTISocketPublisher; //TODO: remove
  friend RTISocketSubscriber;
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
        _basePort = 7400;
        _useBlocking = true;

        _pongSemaphore = NULL;

        _plugin = NULL;
        _worker = NULL;
        _workerFactory = NULL;
        _exclusiveArea = NULL;

        // Similar to NDDS_TRANSPORT_ADDRESS_INVALID
        RTIOsapiMemory_zero(&_nicAddress, sizeof(NDDS_Transport_Address_t));
        RTIOsapiMemory_zero(
                &_multicastAddrTransp,
                sizeof(NDDS_Transport_Address_t));

        _NumPublishers = 1;
        _NumSubscribers = 1;
    }

    ~RTISocketImpl() { Shutdown(); }

    void PrintCmdLineHelp();

    bool ParseConfig(int argc, char *argv[]);

    std::string PrintConfiguration();

    bool Initialize(int argc, char *argv[]);

    void Shutdown();

    int GetBatchSize() {
        return _batchSize;
    }

    unsigned long GetInitializationSampleCount() {
        return 0;
    };

    bool SupportListener(){
        return false;
    };

    bool SupportDiscovery()
    {
        return false;
    };

    IMessagingWriter *CreateWriter(const char *topic_name);

    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *
    CreateReader(const char *topic_name, IMessagingCB *callback);

    DDSTopicDescription *CreateCft(const char *topic_name, DDSTopic *topic); //TODO: remove

    bool ConfigureSocketsTransport();

    /*
     * This function calculate the port depending of the resource we want to
     * create and the domain.
     */
    unsigned int GetUnicastPort(const char *topicName);

    bool GetMulticastTransportAddr(
            const char *topicName,
            NDDS_Transport_Address_t &addr);

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
    unsigned int _basePort;
    bool _useBlocking;

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
    static const int RESOURCES_PER_PARTICIPANT;
};

char *InterfaceNameToAddress(const char *nicName);

int NDDS_Transport_UDPv4_get_num_multicast_interfaces(
        struct NDDS_Transport_UDP *plugin);

#endif // __RTISOCKETIMPL_H__
