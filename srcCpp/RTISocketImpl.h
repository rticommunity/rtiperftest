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
#include "perftest.h"
#include "perftestSupport.h"
#include "perftest_cpp.h"
#include "transport/transport_socketutil.h"
#include "transport/transport_udpv4.h"

#include "perftestPlugin.h"

#define RTIPERFTEST_MAX_PEERS 1024
#define RTIPERFTEST_MAX_PORT_ATTEMPT 1024

//TODO: typedef std::vector<NDDS_Transport_Resource_t> vectorTransportResources;

class RTISocketImpl : public IMessaging {
  public:
    RTISocketImpl();

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

    bool ConfigureSocketsTransport();

    /*
     * This function calculate the port depending of the resource we want to
     * create and the domain.
     */
    unsigned int GetUnicastPort(const char *topicName, int idProvided = -1);

    bool GetMulticastTransportAddr(
            const char *topicName,
            NDDS_Transport_Address_t &addr);

    bool IsMulticast()
    {
        return _transport.useMulticast && _transport.allowsMulticast();
    }

  private:
    unsigned long _DataLen;
    int _DomainID;
    bool _IsReliable;
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

    std::vector<std::pair<NDDS_Transport_Address_t, int> > _peersMap;

    PerftestTransport _transport;

    RTIOsapiSemaphore *_pongSemaphore;

    NDDS_Transport_Plugin *_plugin;
    struct REDAWorker *_worker;
    struct REDAWorkerFactory *_workerFactory;
    struct REDAExclusiveArea *_exclusiveArea;
    NDDS_Transport_Address_t _nicAddress;

  public:

    std::vector<NDDS_Transport_SendResource_t> _resourcesList;
    struct peerInfo {
        // Each resource it's pointing to one on resourceList
        NDDS_Transport_SendResource_t *resource;
        NDDS_Transport_Address_t transportAddr;
        unsigned int port;

        peerInfo()
        {
            resource = NULL;
            RTIOsapiMemory_zero(
                    &transportAddr, sizeof(NDDS_Transport_Address_t));
            port = 0;
        }
        peerInfo(
                NDDS_Transport_SendResource_t *res,
                NDDS_Transport_Address_t addr,
                unsigned int p)
        {
            resource = res;
            transportAddr = addr;
            port = p;
        }
    };
    std::vector<peerInfo> _peersInfoList;

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
