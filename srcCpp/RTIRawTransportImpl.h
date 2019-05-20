#ifndef __RTIRawTransportImpl_H__
#define __RTIRawTransportImpl_H__

/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include <algorithm>
#include <map>
#include <string>
#include <stdexcept>
#include "MessagingIF.h"
#include "PerftestTransport.h"
#include "perftest.h"
#include "perftestSupport.h"
#include "perftest_cpp.h"
#include "transport/transport_socketutil.h"
#include "transport/transport_udpv4.h"

#include "perftestPlugin.h"

#define RTIPERFTEST_MAX_PEERS 1024

/* Forwaded declaration */
class PeerData;
class RTIRawTransportImpl : public IMessaging {
  public:
    RTIRawTransportImpl();
    ~RTIRawTransportImpl() { Shutdown(); }
    bool validate_input();
    std::string PrintConfiguration();
    bool Initialize(ParameterManager &PM, perftest_cpp *parent);
    void Shutdown();
    int GetBatchSize();

    bool supports_listener() {return false;}
    bool supports_discovery() {return false;}

    IMessagingWriter *CreateWriter(const char *topic_name);

    /* This implementation does not support listener so callback is ignored. */
    IMessagingReader *
    CreateReader(const char *topic_name, IMessagingCB *callback);

    bool configure_sockets_transport();

    /*------------------------------- Getters --------------------------------*/
    unsigned long GetInitializationSampleCount()    {return 0;}
    NDDS_Transport_Plugin *get_plugin()             {return _plugin;}
    std::vector<PeerData> get_peers_data()          {return _peersDataList;}
    RTIOsapiSemaphore *get_pong_semaphore()         {return _pongSemaphore;}
    struct REDAWorkerFactory *get_worker_factory()  {return _workerFactory;}
    ParameterManager *get_parameter_manager()       {return _PM;}

    /* Calculate the port depending of the Id of the subscriber.*/
    unsigned int
    get_peer_unicast_port(const char *topicName, unsigned int subId = 0);

    /* Get the multicast address that match to the topic name */
    bool get_multicast_transport_addr(
            const char *topicName,
            NDDS_Transport_Address_t &addr);

    /* Calculate the ports thats it will be use for receive data */
    unsigned int get_receive_unicast_port(const char *topicName);
    /*------------------------------------------------------------------------*/

    bool is_multicast()
    {
        return _PM->get<bool>("multicast") && _transport.allowsMulticast();
    }

  private:

    std::vector<std::pair<NDDS_Transport_Address_t, int> > _peersMap;
    std::vector<PeerData> _peersDataList;
    PerftestTransport _transport;
    RTIOsapiSemaphore *_pongSemaphore;
    NDDS_Transport_Plugin *_plugin;
    struct REDAWorkerFactory *_workerFactory;
    struct REDAExclusiveArea *_exclusiveArea;

    ParameterManager *_PM;

  public:
    static unsigned int pubWorkerCount;
    static unsigned int subWorkerCount;

};

/*
 * This class is to allocate all the addresses-ports and his correspond send
 * resources.
 */
class PeerData {
    public:
        // The resources created
        static std::vector<NDDS_Transport_SendResource_t> resourcesList;

        // Each resource it's pointing to one on resourceList
        NDDS_Transport_SendResource_t *resource;
        NDDS_Transport_Address_t transportAddr;
        unsigned int port;

        PeerData() : resource(NULL), port(0)
        {
            RTIOsapiMemory_zero(&transportAddr, sizeof(NDDS_Transport_Address_t));
        }
        PeerData(
                NDDS_Transport_SendResource_t *res,
                NDDS_Transport_Address_t addr,
                unsigned int p)
                        : resource(res), transportAddr(addr), port(p)
        {}
};

/* Allow to check if there is any interface with enabled multicast */
int get_num_multicast_interfaces(struct NDDS_Transport_UDP *plugin);

/* Generate a different worker per thread. */
struct REDAWorker *raw_transport_get_worker_per_thread(
        REDAWorkerFactory *workerFactory,
        RTIOsapiThreadTssFactory *tssFactory,
        unsigned int *workerTssKey);

#endif // __RTIRawTransportImpl_H__