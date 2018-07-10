/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_
#define PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_

#include <string>
#include <map>
#include <sstream>
#include "perftest.h"
//#include "ParameterManager.h" moved to .cxx
#include "ndds/ndds_cpp.h"

/******************************************************************************/
// Default location of the security related files
const std::string TRANSPORT_PRIVATEKEY_FILE_PUB = "./resource/secure/pubkey.pem";
const std::string TRANSPORT_PRIVATEKEY_FILE_SUB = "./resource/secure/subkey.pem";
const std::string TRANSPORT_CERTIFICATE_FILE_PUB = "./resource/secure/pub.pem";
const std::string TRANSPORT_CERTIFICATE_FILE_SUB = "./resource/secure/sub.pem";
const std::string TRANSPORT_CERTAUTHORITY_FILE = "./resource/secure/cacert.pem";

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
    bool wanNetwork;
    std::string publicAddress;

    TcpTransportOptions() :
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
    // TCP specific options
    TcpTransportOptions tcpOptions;
    // Security files
    SecureTransportOptions secureOptions;
    // Wan specific options
    WanTransportOptions wanOptions;

    unsigned long dataLen;

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

};

bool configureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos);

#endif /* PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_ */
