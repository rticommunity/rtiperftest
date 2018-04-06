/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_
#define PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_

#include <string>
#include <map>
#include <sstream>
#include "ndds/ndds_cpp.h"
#include "MessagingIF.h"

/******************************************************************************/

enum Transport {
    TRANSPORT_DEFAULT,
    TRANSPORT_UDPv4,
    TRANSPORT_UDPv6,
    TRANSPORT_TCPv4,
    TRANSPORT_TLSv4,
    TRANSPORT_DTLSv4,
    TRANSPORT_WANv4,
    TRANSPORT_SHMEM
};

struct TransportConfig {
    Transport kind;
    std::string nameString;
    std::string prefixString;

    TransportConfig()
            : kind(TRANSPORT_DEFAULT)
    {
    }

    TransportConfig(
            Transport inputKind,
            std::string inputNameString,
            std::string inputPrefixString)
            :
            kind(inputKind),
            nameString(inputNameString),
            prefixString(inputPrefixString)
    {
    }
};

struct SecureTransportOptions {
    std::string certAuthorityFile;
    std::string certificateFile;
    std::string privateKeyFile;
};

struct TcpTransportOptions {
    std::string serverBindPort;
    bool wanNetwork;
    std::string publicAddress;

    TcpTransportOptions() :
        serverBindPort("7400"),
        wanNetwork(false)
    {}
};

struct WanTransportOptions {
    std::string wanServerAddress;
    std::string wanServerPort;
    std::string wanId;
    bool secureWan;

    WanTransportOptions() :
        wanServerPort("3478"),
        secureWan(false)
    {}
};

/******************************************************************************/

class PerftestTransport {

public:

    /**************************************************************************/
    /* PUBLIC CLASS MEMBERS */

    TransportConfig transportConfig;
    std::string allowInterfaces;
    std::string verbosity;
    // TCP specific options
    TcpTransportOptions tcpOptions;
    // Security files
    SecureTransportOptions secureOptions;
    // Wan specific options
    WanTransportOptions wanOptions;

    unsigned long dataLen;
    bool useMulticast;

    std::string throughputMulticastAddr;
    std::string latencyMulticastAddr;
    std::string announcementMulticastAddr;

    /**************************************************************************/
    /* CLASS CONSTRUCTOR AND DESTRUCTOR */

    PerftestTransport();

    virtual ~PerftestTransport();

    /**************************************************************************/
    /* PUBLIC METHODS */

    static std::map<std::string, unsigned int> getTransportCmdLineArgs();

    std::string helpMessageString();

    void printTransportConfigurationSummary();

    bool parseTransportOptions(int argc, char *argv[]);

    bool allowsMulticast();

    const char * getMulticastAddr(const char * topic);

private:

    static std::map<std::string, TransportConfig> transportConfigMap;

    std::map<std::string, std::string> multicastAddrMap;

    /**************************************************************************/

    static const std::map<std::string, TransportConfig>& getTransportConfigMap();
    bool setTransport(std::string transportString);
    void populateSecurityFiles(bool isPublisher);

};

bool configureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos);

#endif /* PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_ */
