/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_
#define PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_

#include <string>
#include <map>
#include <sstream>
#include "perftest.hpp"
#include <dds/dds.hpp>

/******************************************************************************/

enum Transport {
    TRANSPORT_NOT_SET,
    TRANSPORT_UDPv4,
    TRANSPORT_UDPv6,
    TRANSPORT_TCPv4,
    TRANSPORT_TLSv4,
    TRANSPORT_DTLSv4,
    TRANSPORT_WANv4,
    TRANSPORT_SHMEM,
    TRANSPORT_UDPv4_SHMEM,
    TRANSPORT_UDPv4_UDPv6,
    TRANSPORT_UDPv6_SHMEM,
    TRANSPORT_UDPv4_UDPv6_SHMEM
};

struct TransportConfig {
    Transport kind;
    std::string nameString;
    std::string prefixString;
    bool takenFromQoS;

    TransportConfig()
            : kind(TRANSPORT_NOT_SET),
              takenFromQoS(false)
    {
    }

    TransportConfig(
            Transport inputKind,
            std::string inputNameString,
            std::string inputPrefixString)
            :
            kind(inputKind),
            nameString(inputNameString),
            prefixString(inputPrefixString),
            takenFromQoS(false)
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
    bool customMulticastAddrSet;

    /**************************************************************************/
    /* CLASS CONSTRUCTOR AND DESTRUCTOR */

    PerftestTransport();

    virtual ~PerftestTransport();

    /**************************************************************************/
    /* PUBLIC METHODS */

    static std::map<std::string, unsigned int> getTransportCmdLineArgs();

    std::string helpMessageString();

    std::string printTransportConfigurationSummary();

    bool parseTransportOptions(int argc, char *argv[]);

    // Check if the transport allows the use of multicast.
    bool allowsMulticast();

    /*
     * Given the name of a Perftest-defined topic, returns its multicast
     * address.
     */
    const std::string getMulticastAddr(const char *topic);

private:

    static std::map<std::string, TransportConfig> transportConfigMap;

    std::map<std::string, std::string> multicastAddrMap;

    /**************************************************************************/

    static const std::map<std::string, TransportConfig>& getTransportConfigMap();
    bool setTransport(std::string transportString);
    void populateSecurityFiles(bool isPublisher);
    bool parse_multicast_addresses(char *arg);
    bool increase_address_by_one(const std::string addr, std::string &nextAddr);
};

bool configureTransport(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &properties);

#endif /* PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_ */
