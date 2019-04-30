/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestTransport.h"

/******************************************************************************/

#if defined(RTI_WIN32)
  #pragma warning(push)
  #pragma warning(disable : 4996)
  #define STRNCASECMP _strnicmp
#elif defined(RTI_VXWORKS)
  #define STRNCASECMP strncmp
#else
  #define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

/******************************************************************************/

// Tag used when adding logging output.
const std::string classLoggingString = "PerftestTransport:";

// Default location of the security related files
const std::string TRANSPORT_PRIVATEKEY_FILE_PUB = "./resource/secure/pubkey.pem";
const std::string TRANSPORT_PRIVATEKEY_FILE_SUB = "./resource/secure/subkey.pem";
const std::string TRANSPORT_CERTIFICATE_FILE_PUB = "./resource/secure/pub.pem";
const std::string TRANSPORT_CERTIFICATE_FILE_SUB = "./resource/secure/sub.pem";
const std::string TRANSPORT_CERTAUTHORITY_FILE = "./resource/secure/cacert.pem";

/******************************************************************************/
/* DDS Related functions. These functions doesn't belong to the
 * PerftestTransport class. This way we decouple the Transport class from
 * the specific implementation (C++ Classic vs C++PSM).
 */

bool addPropertyToParticipantQos(
        DDS_DomainParticipantQos &qos,
        std::string propertyName,
        std::string propertyValue)
{

    DDS_ReturnCode_t retcode = DDSPropertyQosPolicyHelper::add_property(
            qos.property,
            propertyName.c_str(),
            propertyValue.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "%s Failed to add property \"%s\"\n",
                classLoggingString.c_str(),
                propertyName.c_str());
        return false;
    }

    return true;
}

bool assertPropertyToParticipantQos(
        DDS_DomainParticipantQos &qos,
        std::string propertyName,
        std::string propertyValue)
{
    DDS_ReturnCode_t retcode = DDSPropertyQosPolicyHelper::assert_property(
            qos.property,
            propertyName.c_str(),
            propertyValue.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "%s Failed to add property \"%s\"\n",
                classLoggingString.c_str(),
                propertyName.c_str());
        return false;
    }

    return true;
}

bool setAllowInterfacesList(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos)
{
    if (!transport.allowInterfaces.empty()) {

        if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
            fprintf(stderr,
                    "%s Ignoring -nic/-allowInterfaces option\n",
                    classLoggingString.c_str());
            return true;
        }

        /*
         * In these 2 specific cases, we are forced to add 2 properties, one
         * for UDPv4 and another for UDPv6
         */
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6) {

            std::string propertyName =
                    "dds.transport.UDPv4.builtin.parent.allow_interfaces";

            if (!addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    transport.allowInterfaces)) {
                return false;
            }

            propertyName =
                    "dds.transport.UDPv6.builtin.parent.allow_interfaces";

            if (!addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    transport.allowInterfaces)) {
                return false;
            }

        } else {

            std::string propertyName = transport.transportConfig.prefixString;
            if (transport.transportConfig.kind == TRANSPORT_WANv4) {
                propertyName += ".parent";
            }

            propertyName += ".parent.allow_interfaces";

            return addPropertyToParticipantQos(
                    qos,
                    propertyName,
                    transport.allowInterfaces);
        }
    }

    return true;
}

bool setTransportVerbosity(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos)
{
    if (!transport.verbosity.empty()) {

        if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
            fprintf(stderr,
                    "%s Ignoring -transportVerbosity option\n",
                    classLoggingString.c_str());
            return true;
        }

        std::string propertyName = transport.transportConfig.prefixString
                + ".verbosity";

        // The name of the property in TCPv4 is different
        if (transport.transportConfig.kind == TRANSPORT_TCPv4) {
            propertyName = transport.transportConfig.prefixString
                    + ".logging_verbosity_bitmap";
        } else if (transport.transportConfig.kind == TRANSPORT_UDPv4
                || transport.transportConfig.kind == TRANSPORT_UDPv6
                || transport.transportConfig.kind == TRANSPORT_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv6_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6_SHMEM) {
            // We do not change logging for the builtin transports.
            return true;
        }

        return addPropertyToParticipantQos(
                qos,
                propertyName,
                transport.verbosity);
    }
    return true;
}

bool configureSecurityFiles(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{

    if (!transport.secureOptions.certAuthorityFile.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".tls.verify.ca_file",
                transport.secureOptions.certAuthorityFile)) {
            return false;
        }
    }

    if (!transport.secureOptions.certificateFile.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".tls.identity.certificate_chain_file",
                transport.secureOptions.certificateFile)) {
            return false;
        }
    }

    if (!transport.secureOptions.privateKeyFile.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".tls.identity.private_key_file",
                transport.secureOptions.privateKeyFile)) {
            return false;
        }
    }

    return true;
}

bool configureTcpTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!transport.tcpOptions.serverBindPort.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server_bind_port",
                transport.tcpOptions.serverBindPort)) {
            return false;
        }
    }

    if (transport.tcpOptions.wanNetwork) {

        if (!assertPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".parent.classid",
                "NDDS_TRANSPORT_CLASSID_TCPV4_WAN")) {
            return false;
        }

        if (transport.tcpOptions.serverBindPort != "0") {
            if (!transport.tcpOptions.publicAddress.empty()) {
                if (!addPropertyToParticipantQos(
                        qos,
                        transport.transportConfig.prefixString
                                + ".public_address",
                        transport.tcpOptions.publicAddress)) {
                    return false;
                }
            } else {
                fprintf(stderr,
                        "%s Public Address is required if "
                        "Server Bind Port != 0\n",
                        classLoggingString.c_str());
                return false;
            }
        }
    }

    if (transport.transportConfig.kind == TRANSPORT_TLSv4) {

        if (transport.tcpOptions.wanNetwork) {
            if (!assertPropertyToParticipantQos(
                    qos,
                    transport.transportConfig.prefixString
                            + ".parent.classid",
                    "NDDS_TRANSPORT_CLASSID_TLSV4_WAN")) {
                return false;
            }
        } else {
            if (!assertPropertyToParticipantQos(
                    qos,
                    transport.transportConfig.prefixString
                            + ".parent.classid",
                    "NDDS_TRANSPORT_CLASSID_TLSV4_LAN")) {
                return false;
            }
        }

        if (!configureSecurityFiles(transport, qos)) {
            return false;
        }
    }

    return true;
}

bool configureDtlsTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".library",
            "nddstransporttls")) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".create_function",
            "NDDS_Transport_DTLS_create")) {
        return false;
    }

    if (!configureSecurityFiles(transport, qos)) {
        return false;
    }

    return true;
}

bool configureWanTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_MASK_NONE;

    if (!addPropertyToParticipantQos(
            qos,
            std::string("dds.transport.load_plugins"),
            transport.transportConfig.prefixString)) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".library",
            "nddstransportwan")) {
        return false;
    }

    if (!addPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString + ".create_function",
            "NDDS_Transport_WAN_create")) {
        return false;
    }

    if (!transport.wanOptions.wanServerAddress.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server",
                transport.wanOptions.wanServerAddress)) {
            return false;
        }
    } else {
        fprintf(
                stderr,
                "%s Wan Server Address is required\n",
                classLoggingString.c_str());
        return false;
    }

    if (!transport.wanOptions.wanServerPort.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".server_port",
                transport.wanOptions.wanServerPort)) {
            return false;
        }
    }

    if (!transport.wanOptions.wanId.empty()) {
        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString
                        + ".transport_instance_id",
                transport.wanOptions.wanId)) {
            return false;
        }
    } else {
        fprintf(stderr, "%s Wan ID is required\n", classLoggingString.c_str());
        return false;
    }

    if (transport.wanOptions.secureWan) {

        if (!addPropertyToParticipantQos(
                qos,
                transport.transportConfig.prefixString + ".enable_security",
                "1")) {
            return false;
        }

        if (!configureSecurityFiles(transport, qos)) {
            return false;
        }
    }

    return true;
}

bool configureShmemTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
{
    // Number of messages that can be buffered in the receive queue.
    int received_message_count_max = 1024 * 1024 * 2
            / (int) transport.dataLen;
    if (received_message_count_max < 1) {
        received_message_count_max = 1;
    }

    std::ostringstream string_stream_object;
    string_stream_object << received_message_count_max;
    if (!assertPropertyToParticipantQos(
            qos,
            "dds.transport.shmem.builtin.received_message_count_max",
            string_stream_object.str())) {
        return false;
    }
    return true;
}

bool configureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos)
{

    /*
     * If transportConfig.kind is not set, then we want to use the value
     * provided by the Participant Qos, so first we read it from there and
     * update the value of transportConfig.kind with whatever was already set.
     */

    if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {

        DDS_TransportBuiltinKindMask mask = qos.transport_builtin.mask;
        switch (mask) {
            case DDS_TRANSPORTBUILTIN_UDPv4:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4,
                    "UDPv4",
                    "dds.transport.UDPv4.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv6:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv6,
                    "UDPv6",
                    "dds.transport.UDPv6.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_SHMEM:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_SHMEM,
                    "SHMEM",
                    "dds.transport.shmem.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv4|DDS_TRANSPORTBUILTIN_SHMEM:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_SHMEM,
                    "UDPv4 & SHMEM",
                    "dds.transport.UDPv4.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv4|DDS_TRANSPORTBUILTIN_UDPv6:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_UDPv6,
                    "UDPv4 & UDPv6",
                    "dds.transport.UDPv4.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_SHMEM|DDS_TRANSPORTBUILTIN_UDPv6:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv6_SHMEM,
                    "UDPv6 & SHMEM",
                    "dds.transport.UDPv6.builtin");
                break;
            case DDS_TRANSPORTBUILTIN_UDPv4|DDS_TRANSPORTBUILTIN_UDPv6|DDS_TRANSPORTBUILTIN_SHMEM:
                transport.transportConfig = TransportConfig(
                    TRANSPORT_UDPv4_UDPv6_SHMEM,
                    "UDPv4 & UDPv6 & SHMEM",
                    "dds.transport.UDPv4.builtin");
                break;
            default:
                /*
                 * This would mean that the mask is either empty or a
                 * different value that we do not support yet. So we keep
                 * the value as "TRANSPORT_NOT_SET"
                 */
                break;
        }
        transport.transportConfig.takenFromQoS = true;
    }

    switch (transport.transportConfig.kind) {

        case TRANSPORT_UDPv4:
            qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;
            break;

        case TRANSPORT_UDPv6:
            qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv6;
            break;

        case TRANSPORT_SHMEM:
            qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_SHMEM;
            if (!configureShmemTransport(transport, qos)) {
                fprintf(stderr,
                        "%s Failed to configure SHMEM plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_TCPv4:
            if (!configureTcpTransport(transport, qos)) {
                fprintf(stderr,
                        "%s Failed to configure TCP plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_TLSv4:
            if (!configureTcpTransport(transport, qos)) {
                fprintf(stderr,
                        "%s Failed to configure TCP + TLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_DTLSv4:
            if (!configureDtlsTransport(transport, qos)) {
                fprintf(stderr,
                        "%s Failed to configure DTLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;

        case TRANSPORT_WANv4:
            if (!configureWanTransport(transport, qos)) {
                fprintf(stderr,
                        "%s Failed to configure WAN plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            break;
        default:

            /*
             * If shared memory is enabled we want to set up its
             * specific configuration
             */
            if ((qos.transport_builtin.mask & DDS_TRANSPORTBUILTIN_SHMEM)
                    != 0) {
                if (!configureShmemTransport(transport, qos)) {
                    fprintf(stderr,
                            "%s Failed to configure SHMEM plugin\n",
                            classLoggingString.c_str());
                    return false;
                }
            }
            break;
    } // Switch

    /*
     * If the transport is empty or if it is shmem, it does not make sense
     * setting an interface, in those cases, if the allow interfaces is provided
     * we empty it.
     */
    if (transport.transportConfig.kind != TRANSPORT_NOT_SET
            && transport.transportConfig.kind != TRANSPORT_SHMEM) {
        if (!setAllowInterfacesList(transport, qos)) {
            return false;
        }
    } else {
        // We are not using the allow interface string, so we clear it
        transport.allowInterfaces.clear();
    }

    if (!setTransportVerbosity(transport, qos)) {
        return false;
    }

    return true;
}

/******************************************************************************/
/* CLASS CONSTRUCTOR AND DESTRUCTOR */

PerftestTransport::PerftestTransport() :
        dataLen(100),
        useMulticast(false)
{
    multicastAddrMap[LATENCY_TOPIC_NAME] = "239.255.1.2";
    multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = "239.255.1.100";
    multicastAddrMap[THROUGHPUT_TOPIC_NAME] = "239.255.1.1";

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
        fprintf(
                stderr,
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

void PerftestTransport::populateSecurityFiles(bool isPublisher) {

    if (secureOptions.certificateFile.empty()) {
        if (isPublisher) {
            secureOptions.certificateFile = TRANSPORT_CERTIFICATE_FILE_PUB;
        } else {
            secureOptions.certificateFile = TRANSPORT_CERTIFICATE_FILE_SUB;
        }
    }

    if (secureOptions.privateKeyFile.empty()) {
        if (isPublisher) {
            secureOptions.privateKeyFile = TRANSPORT_PRIVATEKEY_FILE_PUB;
        } else {
            secureOptions.privateKeyFile = TRANSPORT_PRIVATEKEY_FILE_SUB;
        }
    }

    if (secureOptions.certAuthorityFile.empty()) {
        secureOptions.certAuthorityFile = TRANSPORT_CERTAUTHORITY_FILE;
    }
}

/******************************************************************************/
/* PUBLIC METHODS */

std::map<std::string, unsigned int> PerftestTransport::getTransportCmdLineArgs()
{

    std::map<std::string, unsigned int> cmdLineArgsMap;

    cmdLineArgsMap["-transport"] = 1;
    cmdLineArgsMap["-enableTCP"] = 0;
    cmdLineArgsMap["-enableUDPv6"] = 0;
    cmdLineArgsMap["-enableSharedMemory"] = 0;
    cmdLineArgsMap["-nic"] = 1;
    cmdLineArgsMap["-allowInterfaces"] = 1;
    cmdLineArgsMap["-transportServerBindPort"] = 1;
    cmdLineArgsMap["-transportVerbosity"] = 1;
    cmdLineArgsMap["-transportWan"] = 0;
    cmdLineArgsMap["-transportPublicAddress"] = 1;
    cmdLineArgsMap["-transportCertAuthority"] = 1;
    cmdLineArgsMap["-transportCertFile"] = 1;
    cmdLineArgsMap["-transportPrivateKey"] = 1;
    cmdLineArgsMap["-transportWanServerAddress"] = 1;
    cmdLineArgsMap["-transportWanServerPort"] = 1;
    cmdLineArgsMap["-transportWanId"] = 1;
    cmdLineArgsMap["-transportSecureWan"] = 0;
    cmdLineArgsMap["-multicast"] = 0;
    cmdLineArgsMap["-nomulticast"] = 0;
    cmdLineArgsMap["-multicastAddr"] = 1;

    return cmdLineArgsMap;
}

std::string PerftestTransport::helpMessageString()
{

    std::ostringstream oss;
    oss
    << "\t===================== Transport Specific Options ======================\n"
    << "\n"
    << "\t-transport <kind>             - Set transport to be used. The rest of\n"
    << "\t                                the transports will be disabled.\n"
    << "\t                                Values:\n"
    << "\t                                    UDPv4\n"
    << "\t                                    UDPv6\n"
    << "\t                                    SHMEM\n"
    << "\t                                    TCP\n"
    << "\t                                    TLS\n"
    << "\t                                    DTLS\n"
    << "\t                                    WAN\n"
    << "\t                                    Use XML\n"
    << "\t                                Default: Use XML (UDPv4|SHMEM).\n"
    << "\t-nic <ipaddr>                 - Use only the nic specified by <ipaddr>.\n"
    << "\t                                If not specified, use all available\n"
    << "\t                                interfaces\n"
    << "\t-multicast                    - Use multicast to send data. Each topic\n"
    << "\t                                will use a different address:\n";
    for (std::map<std::string, std::string>::iterator it = multicastAddrMap.begin();
        it!=multicastAddrMap.end(); ++it) {
            oss << "                                            "
            << it->first << " " << it->second << "\n";
    }
    oss
    << "\t-multicastAddr <address>      - Use multicast to send data and set\n"
    << "\t                                the input <address> as the multicast\n"
    << "\t                                address for all the topics.\n"
    << "\t-transportVerbosity <level>   - Verbosity of the transport\n"
    << "\t                                Default: 0 (errors only)\n"
    << "\t-transportServerBindPort <p>  - Port used by the transport to accept\n"
    << "\t                                TCP/TLS connections <optional>\n"
    << "\t                                Default: 7400\n"
    << "\t-transportWan                   Use TCP/TLS across LANs and Firewalls.\n"
    << "\t                                Default: Not Set, LAN mode.\n"
    << "\t-transportPublicAddress <ip>  - Public IP address and port (WAN address\n"
    << "\t                                and port) (separated with ‘:’ ) related\n"
    << "\t                                to the transport instantiation. This is\n"
    << "\t                                required when using server mode.\n"
    << "\t                                Default: Not Set.\n"
    << "\t-transportWanServerAddress <a>- Address where to find the WAN Server\n"
    << "\t                                Default: Not Set (Required)\n"
    << "\t-transportWanServerPort <p>     Port where to find the WAN Server.\n"
    << "\t                                Default: 3478.\n"
    << "\t-transportWanId <id>          - Id to be used for the WAN transport.\n"
    << "\t                                Default: Not Set (Required).\n"
    << "\t-transportSecureWan           - Use WAN with security.\n"
    << "\t                                Default: False.\n"
    << "\t-transportCertAuthority <file>- Certificate authority file <optional>\n"
    << "\t                                Default: \""
    << TRANSPORT_CERTAUTHORITY_FILE << "\"\n"
    << "\t-transportCertFile <file>     - Certificate file <optional>\n"
    << "\t                                Default (Publisher): \""
    << TRANSPORT_CERTIFICATE_FILE_PUB << "\"\n"
    << "\t                                Default (Subscriber): \""
    << TRANSPORT_CERTIFICATE_FILE_SUB << "\"\n"
    << "\t-transportPrivateKey <file>   - Private key file <optional>\n"
    << "\t                                Default (Publisher): \""
    << TRANSPORT_PRIVATEKEY_FILE_PUB << "\"\n"
    << "\t                                Default (Subscriber): \""
    << TRANSPORT_PRIVATEKEY_FILE_SUB << "\"\n";

    return oss.str();
}

std::string PerftestTransport::printTransportConfigurationSummary()
{

    std::ostringstream stringStream;
    stringStream << "Transport Configuration:\n";
    stringStream << "\tKind: " << transportConfig.nameString;
    if (transportConfig.takenFromQoS) {
        stringStream << " (taken from QoS XML file)";
    }
    stringStream << "\n";

    if (!allowInterfaces.empty()) {
        stringStream << "\tNic: " << allowInterfaces << "\n";
    }

    stringStream << "\tUse Multicast: "
                 << ((allowsMulticast() && useMulticast) ? "True" : "False");
    if (!allowsMulticast() && useMulticast) {
        stringStream << " (Multicast is not supported for "
                     << transportConfig.nameString << ")";
    }
    stringStream << "\n";

    if (transportConfig.kind == TRANSPORT_TCPv4
            || transportConfig.kind == TRANSPORT_TLSv4) {

        stringStream << "\tTCP Server Bind Port: "
                     << tcpOptions.serverBindPort << "\n";

        stringStream << "\tTCP LAN/WAN mode: "
                     << (tcpOptions.wanNetwork ? "WAN\n" : "LAN\n");
        if (tcpOptions.wanNetwork) {
            stringStream << "\tTCP Public Address: "
                         << tcpOptions.publicAddress << "\n";
        }
    }

    if (transportConfig.kind == TRANSPORT_WANv4) {

        stringStream << "\tWAN Server Address: "
                     << wanOptions.wanServerAddress << ":"
                     << wanOptions.wanServerPort << "\n";
        stringStream << "\tWAN Id: "
                     << wanOptions.wanId << "\n";
        stringStream << "\tWAN Secure: " << wanOptions.secureWan << "\n";

    }

    if (transportConfig.kind == TRANSPORT_TLSv4
            || transportConfig.kind == TRANSPORT_DTLSv4
            || (transportConfig.kind == TRANSPORT_WANv4 && wanOptions.secureWan)) {
        stringStream << "\tCertificate authority file: "
                     << secureOptions.certAuthorityFile << "\n";
        stringStream << "\tCertificate file: "
                     << secureOptions.certificateFile << "\n";
        stringStream << "\tPrivate key file: "
                     << secureOptions.privateKeyFile << "\n";
    }

    if (!verbosity.empty()) {
        stringStream << "\tVerbosity: " << verbosity << "\n";
    }

    return stringStream.str();
}

bool PerftestTransport::parseTransportOptions(int argc, char *argv[])
{

    bool isPublisher = false;
    std::string transportString = "Use XML";

    // We will only parse the properties related with transports here.
    for (int i = 0; i < argc; ++i) {

        if (IS_OPTION(argv[i], "-pub")) {

            isPublisher = true;

        } else if (IS_OPTION(argv[i], "-dataLen")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <length> after -dataLen\n",
                        classLoggingString.c_str());
                return false;
            }

            dataLen = strtol(argv[i], NULL, 10);

        } else if (IS_OPTION(argv[i], "-transport")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <kind> after -transport\n",
                        classLoggingString.c_str());
                return false;
            }

            transportString = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-enableTCP")) {

            /* Legacy option */
            transportString = "TCP";

        } else if (IS_OPTION(argv[i], "-enableUDPv6")) {

            /* Legacy option */
            transportString = "UDPv6";

        } else if (IS_OPTION(argv[i], "-enableSharedMemory")) {

            /* Legacy option */
            transportString = "SHMEM";

        } else if (IS_OPTION(argv[i], "-nic")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <address> after -nic\n",
                        classLoggingString.c_str());
                return false;
            }

            allowInterfaces = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-allowInterfaces")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <address> after -allowInterfaces\n",
                        classLoggingString.c_str());
                return false;
            }

            allowInterfaces = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportVerbosity")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <level> after -transportVerbosity\n",
                        classLoggingString.c_str());
                return false;
            }

            verbosity = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportServerBindPort")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <number> after "
                        "-transportServerBindPort\n",
                        classLoggingString.c_str());
                return false;
            }

            tcpOptions.serverBindPort = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportWan")) {

            tcpOptions.wanNetwork = true;

        } else if (IS_OPTION(argv[i], "-transportPublicAddress")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <address> after "
                        "-transportPublicAddress\n",
                        classLoggingString.c_str());
                return false;
            }

            tcpOptions.publicAddress = std::string(argv[i]);

        }  else if (IS_OPTION(argv[i], "-transportCertAuthority")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <path> after -transportCertAuthority\n",
                        classLoggingString.c_str());
                return false;
            }

            secureOptions.certAuthorityFile = std::string(argv[i]);

        }  else if (IS_OPTION(argv[i], "-transportCertFile")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <path> after -transportCertFile\n",
                        classLoggingString.c_str());
                return false;
            }

            secureOptions.certificateFile = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportPrivateKey")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <path> after -transportPrivateKey\n",
                        classLoggingString.c_str());
                return false;
            }

            secureOptions.privateKeyFile = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportWanServerAddress")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <address> after "
                        "-transportWanServerAddress\n",
                        classLoggingString.c_str());
                return false;
            }

            wanOptions.wanServerAddress = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportWanServerPort")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <port> after "
                        "-transportWanServerPort\n",
                        classLoggingString.c_str());
                return false;
            }

            wanOptions.wanServerPort = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportWanId")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <id> after "
                        "-transportWanId\n",
                        classLoggingString.c_str());
                return false;
            }

            wanOptions.wanId = std::string(argv[i]);

        } else if (IS_OPTION(argv[i], "-transportSecureWan")) {

            wanOptions.secureWan = true;

        } else if (IS_OPTION(argv[i], "-multicast")) {
            useMulticast = true;
        } else if (IS_OPTION(argv[i], "-multicastAddr")) {
            useMulticast = true;
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr,
                        "%s Missing <address> after "
                        "-multicastAddr\n",
                        classLoggingString.c_str());
                return false;
            }
            multicastAddrMap[THROUGHPUT_TOPIC_NAME] = argv[i];
            multicastAddrMap[LATENCY_TOPIC_NAME] = argv[i];
            multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = argv[i];

        } else if (IS_OPTION(argv[i], "-nomulticast")) {
            useMulticast = false;
        }
    }

    if (!setTransport(transportString)) {
        fprintf(stderr,
                "%s Error Setting the transport\n",
                classLoggingString.c_str());
        return false;
    }

    // We only need to set the secure properties for this
    if (transportConfig.kind == TRANSPORT_TLSv4
            || transportConfig.kind == TRANSPORT_DTLSv4
            || transportConfig.kind == TRANSPORT_WANv4) {
        populateSecurityFiles(isPublisher);
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

const std::string PerftestTransport::getMulticastAddr(const char *topicName)
{
    std::string address = multicastAddrMap[std::string(topicName)];

    if (address.length() == 0) {
        return NULL;
    }

    return address;
}

/******************************************************************************/
