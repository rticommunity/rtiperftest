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

class peerData;
class RTIRawTransportImpl : public IMessaging {
  public:
    RTIRawTransportImpl();

    ~RTIRawTransportImpl() { Shutdown(); }

    bool validate_input();

    std::string PrintConfiguration();

    bool Initialize(ParameterManager &PM);

    void Shutdown();
    int GetBatchSize();

    unsigned long GetInitializationSampleCount();

    NDDS_Transport_Plugin *getPlugin();

    std::vector<peerData> getPeersData();

    RTIOsapiSemaphore *getPongSemaphore();

    struct REDAWorkerFactory *getWorkerFactory();

    RTIOsapiThreadTssFactory *getTssFactory();

    /* Calculate the port depending of the Id of the subscriber.*/
    unsigned int
    getSendUnicastPort(const char *topicName, unsigned int subId = 0);

    /* Get the multicast address that match to the topic name */
    bool getMulticastTransportAddr(
            const char *topicName,
            NDDS_Transport_Address_t &addr);

    /* Calculate the ports thats it will be use for receive data */
    unsigned int getReceiveUnicastPort(const char *topicName);

    bool SupportsListener();

    bool SupportsDiscovery();

    IMessagingWriter *CreateWriter(const char *topic_name);

    // This implementation dont support listener so callback is ignore.
    IMessagingReader *
    CreateReader(const char *topic_name, IMessagingCB *callback);

    bool configureSocketsTransport();

    bool isMulticast()
    {
        return _PM->get<bool>("multicast") && _transport.allowsMulticast();
    }

  private:

    std::vector<std::pair<NDDS_Transport_Address_t, int> > _peersMap;
    std::vector<peerData> _peersDataList;
    PerftestTransport _transport;
    RTIOsapiSemaphore *_pongSemaphore;
    NDDS_Transport_Plugin *_plugin;
    struct REDAWorkerFactory *_workerFactory;
    struct REDAExclusiveArea *_exclusiveArea;
    RTIOsapiThreadTssFactory *_tssFactory;

    ParameterManager *_PM;
};

class peerData {
    public:
        // The resources created
        static std::vector<NDDS_Transport_SendResource_t> resourcesList;

        // Each resource it's pointing to one on resourceList
        NDDS_Transport_SendResource_t *resource;
        NDDS_Transport_Address_t transportAddr;
        unsigned int port;

        peerData() : resource(NULL), port(0)
        {
            RTIOsapiMemory_zero(&transportAddr, sizeof(NDDS_Transport_Address_t));
        }
        peerData(
                NDDS_Transport_SendResource_t *res,
                NDDS_Transport_Address_t addr,
                unsigned int p)
                : resource(res), transportAddr(addr), port(p)
        {}
};

int getNumMulticastInterfaces(struct NDDS_Transport_UDP *plugin);

/* Generate a different worker per thread. */
struct REDAWorker *RawTransportGetWorkerPerThread(
        REDAWorkerFactory *workerFactory,
        RTIOsapiThreadTssFactory *tssFactory,
        unsigned int *workerTssKey);

#endif // __RTIRawTransportImpl_H__
