/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestTransport.h"
#include "Infrastructure_common.h"

/******************************************************************************/
/* CLASS CONSTRUCTOR AND DESTRUCTOR */

PerftestTransport::PerftestTransport()
{
    multicastAddrMap[LATENCY_TOPIC_NAME] = TRANSPORT_MULTICAST_ADDR_LATENCY;
    multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] =
            TRANSPORT_MULTICAST_ADDR_ANNOUNCEMENT;
    multicastAddrMap[THROUGHPUT_TOPIC_NAME] =
            TRANSPORT_MULTICAST_ADDR_THROUGHPUT;

    transportConfigMap["Use XML"] = TransportConfig(
        TRANSPORT_NOT_SET,
        "--",
        "--");
    transportConfigMap["UDPv4"] = TransportConfig(
            TRANSPORT_UDPv4,
            "UDPv4",
            "dds.transport.UDPv4.builtin");
    transportConfigMap["UDPv6"] = TransportConfig(
            TRANSPORT_UDPv6,
            "UDPv6",
            "dds.transport.UDPv6.builtin");
    transportConfigMap["TCP"] = TransportConfig(
            TRANSPORT_TCPv4,
            "TCP",
            "dds.transport.TCPv4.tcp1");
    transportConfigMap["TLS"] = TransportConfig(
            TRANSPORT_TLSv4,
            "TLS",
            "dds.transport.TCPv4.tcp1");
    transportConfigMap["DTLS"] = TransportConfig(
            TRANSPORT_DTLSv4,
            "DTLS",
            "dds.transport.DTLS.dtls1");
    transportConfigMap["WAN"] = TransportConfig(
            TRANSPORT_WANv4,
            "WAN",
            "dds.transport.WAN.wan1");
    transportConfigMap["SHMEM"] = TransportConfig(
            TRANSPORT_SHMEM,
            "SHMEM",
            "dds.transport.shmem.builtin");
}

PerftestTransport::~PerftestTransport()
{
}

void PerftestTransport::initialize(ParameterManager *PM)
{
    _PM = PM;
}

/******************************************************************************/
/* PRIVATE METHODS */

bool PerftestTransport::setTransport(std::string transportString)
{

    std::map<std::string, TransportConfig> configMap = transportConfigMap;
    std::map<std::string, TransportConfig>::iterator it = configMap.find(
            transportString);
    if (it != configMap.end()) {
        transportConfig = it->second;
    } else {
        fprintf(stderr,
                "%s \"%s\" is not a valid transport. "
                "List of supported transport:\n",
                classLoggingString.c_str(),
                transportString.c_str());
        for (std::map<std::string, TransportConfig>::iterator it = configMap
                .begin(); it != configMap.end(); ++it) {
            fprintf(stderr, "\t\"%s\"\n", it->first.c_str());
        }
        return false;
    }
    return true;
}

void PerftestTransport::populateSecurityFiles() {

    if (_PM->get<std::string>("transportCertFile").empty()) {
        if (_PM->get<bool>("pub")) {
            _PM->set("transportCertFile", TRANSPORT_CERTIFICATE_FILE_PUB);
        } else {
            _PM->set("transportCertFile", TRANSPORT_CERTIFICATE_FILE_SUB);
        }
    }

    if (_PM->get<std::string>("transportPrivateKey").empty()) {
        if (_PM->get<bool>("pub")) {
            _PM->set("transportPrivateKey", TRANSPORT_PRIVATEKEY_FILE_PUB);
        } else {
            _PM->set("transportPrivateKey", TRANSPORT_PRIVATEKEY_FILE_SUB);
        }
    }

    if (_PM->get<std::string>("transportCertAuthority").empty()) {
        _PM->set("transportCertAuthority", TRANSPORT_CERTAUTHORITY_FILE);
    }
}

/******************************************************************************/
/* PUBLIC METHODS */

std::string PerftestTransport::printTransportConfigurationSummary()
{
    std::ostringstream stringStream;
    stringStream << "Transport Configuration:\n";
    stringStream << "\tKind: " << transportConfig.nameString;
    if (transportConfig.takenFromQoS) {
        stringStream << " (taken from QoS XML file)";
    }
    stringStream << "\n";

    if (!_PM->get<std::string>("allowInterfaces").empty()) {
        stringStream << "\tNic: "
                     << _PM->get<std::string>("allowInterfaces")
                     << "\n";
    }

    stringStream << "\tUse Multicast: "
                 << ((allowsMulticast() && _PM->get<bool>("multicast"))
                        ? "True" : "False");
    if (!allowsMulticast() && _PM->get<bool>("multicast")) {
        stringStream << " (Multicast is not supported for "
                     << transportConfig.nameString << ")";
    }
    stringStream << "\n";

    if (_PM->is_set("multicastAddr")) {
        stringStream << "\tUsing custom Multicast Addresses:"
                << "\n\t\tThroughtput Address: "
                << multicastAddrMap[THROUGHPUT_TOPIC_NAME].c_str()
                << "\n\t\tLatency Address: "
                << multicastAddrMap[LATENCY_TOPIC_NAME].c_str()
                << "\n\t\tAnnouncement Address: "
                << multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME].c_str()
                << "\n";
    }

    if (transportConfig.kind == TRANSPORT_TCPv4
            || transportConfig.kind == TRANSPORT_TLSv4) {
        stringStream << "\tTCP Server Bind Port: "
                     << _PM->get<std::string>("transportServerBindPort")
                     << "\n";

        stringStream << "\tTCP LAN/WAN mode: "
                     << (_PM->get<bool>("transportWan") ? "WAN\n" : "LAN\n");
        if (_PM->get<bool>("transportWan")) {
            stringStream << "\tTCP Public Address: "
                         << _PM->get<std::string>("transportPublicAddress")
                         << "\n";
        }
    }

    if (transportConfig.kind == TRANSPORT_WANv4) {
        stringStream << "\tWAN Server Address: "
                     << _PM->get<std::string>("transportWanServerAddress")
                     << ":"
                     << _PM->get<std::string>("transportWanServerPort")
                     << "\n";
        stringStream << "\tWAN Id: "
                     << _PM->get<std::string>("transportWanId")
                     << "\n";
        stringStream << "\tWAN Secure: "
                     << _PM->get<bool>("transportSecureWan")
                     << "\n";
    }

    if (transportConfig.kind == TRANSPORT_TLSv4
            || transportConfig.kind == TRANSPORT_DTLSv4
            || (transportConfig.kind == TRANSPORT_WANv4
            && _PM->get<bool>("transportSecureWan"))) {
        stringStream << "\tCertificate authority file: "
                     << _PM->get<std::string>("transportCertAuthority")
                     << "\n";
        stringStream << "\tCertificate file: "
                     << _PM->get<std::string>("transportCertFile")
                     << "\n";
        stringStream << "\tPrivate key file: "
                     << _PM->get<std::string>("transportPrivateKey")
                     << "\n";
    }

    if (!_PM->get<std::string>("transportVerbosity").empty()) {
        stringStream << "\tVerbosity: "
                     << _PM->get<std::string>("transportVerbosity")
                     << "\n";
    }

    return stringStream.str();
}

/*********************************************************
 * Validate and manage the parameters
 */
bool PerftestTransport::validate_input()
{
    /*
     * "-allowInterfaces" and "-nic" are the same parameter,
     * so now use only one: "allowInterfaces"
     */
    if (_PM->get<std::string>("allowInterfaces").empty()) {
        _PM->set<std::string>("allowInterfaces", _PM->get<std::string>("nic"));
    }

    // Manage parameter -transport
    if (!setTransport(_PM->get<std::string>("transport"))) {
        fprintf(stderr,
                "%s Error Setting the transport\n",
                classLoggingString.c_str());
        return false;
    }

  #ifndef RTI_MICRO
    // Manage parameter -multicastAddr
    if(_PM->is_set("multicastAddr")) {
        if (!parse_multicast_addresses(
                _PM->get<std::string>("multicastAddr").c_str())) {
            fprintf(stderr, "Error parsing -multicastAddr\n");
            return false;
        }
        _PM->set<bool>("multicast", true);
    }
  #endif

    // We only need to set the secure properties for this
    if (transportConfig.kind == TRANSPORT_TLSv4
            || transportConfig.kind == TRANSPORT_DTLSv4
            || transportConfig.kind == TRANSPORT_WANv4) {
        populateSecurityFiles();
    }

    return true;
}

bool PerftestTransport::allowsMulticast()
{
    return (transportConfig.kind != TRANSPORT_TCPv4
            && transportConfig.kind != TRANSPORT_TLSv4
            && transportConfig.kind != TRANSPORT_WANv4
            && transportConfig.kind != TRANSPORT_SHMEM);
}

#ifndef RTI_MICRO

const std::string PerftestTransport::getMulticastAddr(const char *topicName)
{
    std::string address = multicastAddrMap[std::string(topicName)];

    if (address.length() == 0) {
        return NULL;
    }

    return address;
}

bool PerftestTransport::is_multicast(std::string addr)
{

    NDDS_Transport_Address_t transportAddress;

    if (!NDDS_Transport_Address_from_string(&transportAddress, addr.c_str())) {
        fprintf(stderr, "Fail to get a transport address from string\n");
        return false;
    }

    return NDDS_Transport_Address_is_multicast(&transportAddress);
}

bool PerftestTransport::increase_address_by_one(
        const std::string addr,
        std::string &nextAddr)
{
    bool success = false;
    bool isIPv4 = false; /* true = IPv4 // false = IPv6 */
    NDDS_Transport_Address_t transportAddress;
    char buffer[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];

    if (!NDDS_Transport_Address_from_string(&transportAddress, addr.c_str())) {
        fprintf(stderr, "Fail to get a transport address from string\n");
        return false;
    }

    isIPv4 = NDDS_Transport_Address_is_ipv4(&transportAddress);

    /*
     * Increase the address by one value.
     * If the Address is 255.255.255.255 (or the equivalent for IPv6) this
     * function will FAIL
     */
    for (int i = NDDS_TRANSPORT_ADDRESS_LENGTH - 1; i >= 0 && !success; i--) {
        if (isIPv4 && i < 9) {
            /* If the user set a IPv4 higher than 255.255.255.253 fail here*/
            break;
        }
        if (transportAddress.network_ordered_value[i] == 255) {
            transportAddress.network_ordered_value[i] = 0;
        }
        else {
            /* Increase the value and exit */
            transportAddress.network_ordered_value[i]++;
            success = true;
        }
    }

    if (!success) {
        fprintf(stderr,
                "IP value too high. Please use -help for more information "
                "about -multicastAddr command line\n");
        return false;
    }

    /* Try to get a IP string format */
    if (!NDDS_Transport_Address_to_string_with_protocol_family_format(
            &transportAddress,
            buffer,
            NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE,
            isIPv4 ? RTI_OSAPI_SOCKET_AF_INET : RTI_OSAPI_SOCKET_AF_INET6)) {
        fprintf(stderr, "Fail to retrieve a ip string format\n");
        return false;
    }

    nextAddr = buffer;

    return true;
}

bool PerftestTransport::parse_multicast_addresses(const char *arg)
{
    char throughput[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    char latency[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    char annonuncement[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    unsigned int numberOfAddressess = 0;

    /* Get number of addresses on the string */
    if (!NDDS_Transport_get_number_of_addresses_in_string(
            arg,
            &numberOfAddressess)) {
        fprintf(stderr, "Error parsing Address for -multicastAddr option\n");
        return false;
    }

    /* If three addresses are given */
    if (numberOfAddressess == 3) {
        if (!NDDS_Transport_get_address(arg, 0, throughput)
                || !NDDS_Transport_get_address(arg, 1, latency)
                || !NDDS_Transport_get_address(arg, 2, annonuncement)){
            fprintf(stderr,
                    "Error parsing Address for -multicastAddr option\n");
            return false;
        }

        multicastAddrMap[THROUGHPUT_TOPIC_NAME] = throughput;
        multicastAddrMap[LATENCY_TOPIC_NAME] = latency;
        multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = annonuncement;

    } else if (numberOfAddressess == 1) {
        /* If only one address is given */
        if (!NDDS_Transport_get_address(arg, 0, throughput)) {
            fprintf(stderr,
                    "Error parsing Address for -multicastAddr option\n");
            return false;
        }
        multicastAddrMap[THROUGHPUT_TOPIC_NAME] = throughput;

        /* Calculate the consecutive one */
        if (!increase_address_by_one(
                multicastAddrMap[THROUGHPUT_TOPIC_NAME],
                multicastAddrMap[LATENCY_TOPIC_NAME])) {
            fprintf(stderr,
                    "Fail to increase the value of IP address given\n");
            return false;
        }

        /* Calculate the consecutive one */
        if (!increase_address_by_one(
                multicastAddrMap[LATENCY_TOPIC_NAME],
                multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME])) {
            fprintf(stderr,
                    "Fail to increase the value of IP address given\n");
            return false;
        }

    } else {
        fprintf(stderr,
                "Error parsing Address/es '%s' for -multicastAddr option\n"
                "Use -help option to see the correct sintax\n",
                arg);
        return false;
    }

    /* Last check. All the IPs must be in IP format and multicast range */
    if (!is_multicast(multicastAddrMap[THROUGHPUT_TOPIC_NAME])
            || !is_multicast(multicastAddrMap[LATENCY_TOPIC_NAME])
            || !is_multicast(multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME])) {

        fprintf(stderr,
                "Error parsing the address/es '%s' for -multicastAddr option\n",
                arg);
        if (numberOfAddressess == 1) {
            fprintf(stderr,
                    "\tThe calculated addresses are outsite of multicast range.\n");
        } else {
            fprintf(stderr, "\tThere are outsite of multicast range.\n");
        }

        fprintf(stderr, "\tUse -help option to see the correct sintax\n");

        return false;
    }

    // We only need to set the secure properties for this
    if (transportConfig.kind == TRANSPORT_TLSv4
            || transportConfig.kind == TRANSPORT_DTLSv4
            || transportConfig.kind == TRANSPORT_WANv4) {
        populateSecurityFiles();
    }

    return true;
}

#endif
/******************************************************************************/
