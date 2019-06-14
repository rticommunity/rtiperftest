/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_
#define PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_

#include <string>
#include <map>
#include <sstream>
#include "ParameterManager.h"

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

/******************************************************************************/

class PerftestTransport {

public:

    /**************************************************************************/
    /* PUBLIC CLASS MEMBERS */

    TransportConfig transportConfig;
    /**************************************************************************/
    /* CLASS CONSTRUCTOR AND DESTRUCTOR */

    PerftestTransport();

    virtual ~PerftestTransport();
    void initialize(ParameterManager *PM);

    /**************************************************************************/
    /* PUBLIC METHODS */
    std::string printTransportConfigurationSummary();

    bool validate_input();

    // Check if the transport allows the use of multicast.
    bool allowsMulticast();

  #ifndef RTI_MICRO
    /*
     * Given the name of a Perftest-defined topic, returns its multicast
     * address.
     */
    const std::string getMulticastAddr(const char *topic);

    /* Used to validate a multicast address */
    bool is_multicast(std::string addr);
  #endif

private:

    std::map<std::string, TransportConfig> transportConfigMap;
    std::map<std::string, std::string> multicastAddrMap;
    ParameterManager *_PM;
    /**************************************************************************/

    bool setTransport(std::string transportString);
    void populateSecurityFiles();
  #ifndef RTI_MICRO
    bool parse_multicast_addresses(const char *arg);
    bool increase_address_by_one(const std::string addr, std::string &nextAddr);
  #endif
};

// Tag used when adding logging output.
const std::string classLoggingString = "PerftestTransport:";

#endif /* PERFTEST_2_0_SRCCPP_PERFTESTTRANSPORT_H_ */
