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
#include <limits.h>


#define DEFAULT_MESSAGE_SIZE_MAX 65536
#define MESSAGE_SIZE_MAX_NOT_SET LONG_MAX

/*
 * This const is used to calculate the maximum size that a packet can have. This
 * number has to be substracted to the message_size_max.
 * We calculate this as:
 * - COMMEND_WRITER_MAX_RTPS_OVERHEAD <- Size of the overhead for RTPS in the
 *                                       worst case
 * - 48 <- Max transport overhead (this would be for ipv6).
 * - Encapsulation (4 bytes) + aligment in the worst case (3)
 *
 * TODO: that Encapsulation should be taken out from the sample instead of here.
 */

#ifndef RTI_MICRO
  #define MESSAGE_OVERHEAD_BYTES (COMMEND_WRITER_MAX_RTPS_OVERHEAD + 48 + 4 + 3)
#else
  #define MESSAGE_OVERHEAD_BYTES 700
#endif

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
    std::map<std::string, TransportConfig> transportConfigMap;

    /*
     * This is the minimum size across all the active transports
     * message_size_max
     */
    long minimumMessageSizeMax;

    /*
     * When configuring the transport we might need to share information so it
     * is displayed in the summary, we will save it here.
     */
    std::string loggingString;

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
