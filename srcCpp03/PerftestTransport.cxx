/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestTransport.h"
#include "perftest_cpp.h"

/******************************************************************************/

#if defined(RTI_WIN32) || defined(RTI_INTIME)
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

/******************************************************************************/
/* DDS Related functions. These functions doesn't belong to the
 * PerftestTransport class. This way we decouple the Transport class from
 * the specific implementation (C++ Classic vs C++PSM).
 */

void setAllowInterfacesList(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{
    using namespace rti::core::policy;

    if (!_PM->get<std::string>("allowInterfaces").empty()) {

        if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
            fprintf(stderr,
                    "%s Ignoring -nic/-allowInterfaces option\n",
                    classLoggingString.c_str());
            return;
        }
        /*
         * In these 2 specific cases, we are forced to add 2 properties, one
         * for UDPv4 and another for UDPv6
         */
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6_SHMEM
                || transport.transportConfig.kind == TRANSPORT_UDPv4_UDPv6) {

            std::string propertyName =
                    "dds.transport.UDPv4.builtin.parent.allow_interfaces";
            qos_properties[propertyName] =
                    _PM->get<std::string>("allowInterfaces");

            propertyName =
                    "dds.transport.UDPv6.builtin.parent.allow_interfaces";
            qos_properties[propertyName] =
                    _PM->get<std::string>("allowInterfaces");

        } else {

            std::string propertyName = transport.transportConfig.prefixString;

            if (transport.transportConfig.kind == TRANSPORT_WANv4) {
                propertyName += ".parent";
            }

            propertyName += ".parent.allow_interfaces";

            qos_properties[propertyName] =
                    _PM->get<std::string>("allowInterfaces");
        }
    }
}

void setTransportVerbosity(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{
    using namespace rti::core::policy;

    if (!_PM->get<std::string>("transportVerbosity").empty()) {

        if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
            fprintf(stderr,
                    "%s Ignoring -transportVerbosity option\n",
                    classLoggingString.c_str());
            return;
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
            return;
        }

        qos_properties[propertyName] =
                _PM->get<std::string>("transportVerbosity");
    }
}

void configureSecurityFiles(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{

    if (!_PM->get<std::string>("transportCertAuthority").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".tls.verify.ca_file"] =
                        _PM->get<std::string>("transportCertAuthority");
    }

    if (!_PM->get<std::string>("transportCertFile").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".tls.identity.certificate_chain_file"] =
                        _PM->get<std::string>("transportCertFile");
    }

    if (!_PM->get<std::string>("transportPrivateKey").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".tls.identity.private_key_file"] =
                        _PM->get<std::string>("transportPrivateKey");
    }
}

bool configureTcpTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{
    qos_properties["dds.transport.load_plugins"] =
            transport.transportConfig.prefixString;

    if (!_PM->get<std::string>("transportServerBindPort").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".server_bind_port"] =
                        _PM->get<std::string>("transportServerBindPort");
    }

    if (_PM->get<bool>("transportWan")) {

        qos_properties[transport.transportConfig.prefixString
                + ".parent.classid"] = "NDDS_TRANSPORT_CLASSID_TCPV4_WAN";

        if (_PM->get<std::string>("transportServerBindPort") != "0") {
            if (!_PM->get<std::string>("transportPublicAddress").empty()) {

                qos_properties[transport.transportConfig.prefixString
                        + ".public_address"] =
                                _PM->get<std::string>("transportPublicAddress");
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

        if (_PM->get<bool>("transportWan")) {
            qos_properties[transport.transportConfig.prefixString
                    + ".parent.classid"] = "NDDS_TRANSPORT_CLASSID_TLSV4_WAN";
        } else {
            qos_properties[transport.transportConfig.prefixString
                    + ".parent.classid"] = "NDDS_TRANSPORT_CLASSID_TLSV4_LAN";
        }

        configureSecurityFiles(transport, qos_properties, _PM);
    }

    return true;
}

void configureDtlsTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{
    qos_properties["dds.transport.load_plugins"] = transport.transportConfig
            .prefixString;

    qos_properties[transport.transportConfig.prefixString + ".library"] =
            "nddstransporttls";

    qos_properties[transport.transportConfig.prefixString + ".create_function"] =
            "NDDS_Transport_DTLS_create";

    configureSecurityFiles(transport, qos_properties, _PM);
}

bool configureWanTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{

    qos_properties["dds.transport.load_plugins"] =
            transport.transportConfig.prefixString;

    qos_properties[transport.transportConfig.prefixString
                   + ".library"] = "nddstransportwan";

    qos_properties[transport.transportConfig.prefixString
                   + ".create_function"] = "NDDS_Transport_WAN_create";

    if (!_PM->get<std::string>("transportWanServerAddress").empty()) {
        qos_properties[transport.transportConfig.prefixString
                        + ".server"] =
                                _PM->get<std::string>("transportWanServerAddress");
    } else {
        fprintf(stderr,
                "%s Wan Server Address is required\n",
                classLoggingString.c_str());
        return false;
    }

    if (!_PM->get<std::string>("transportWanServerPort").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".server_port"] =
                        _PM->get<std::string>("transportWanServerPort");
    }

    if (!_PM->get<std::string>("transportWanId").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".transport_instance_id"] =
                        _PM->get<std::string>("transportWanId");
    } else {
        fprintf(stderr, "%s Wan ID is required\n", classLoggingString.c_str());
        return false;
    }

    if (_PM->get<bool>("transportSecureWan")) {
        qos_properties[transport.transportConfig.prefixString
                       + ".enable_security"] = "1";
        configureSecurityFiles(transport, qos_properties, _PM);
    }

    return true;
}

void configureShmemTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{
    int parentMsgSizeMax = atoi(
            qos_properties["dds.transport.shmem.builtin.parent.message_size_max"]
            .c_str());

    /**
     * The maximum size of a SHMEM segment highly depends on the platform.
     * So that, we need to find out the maximum allocable space to avoid
     * runtime errors.
     *
     * minSize is the packet size of the builtin transport for SHMEM.
     * We need it as the minimum size for a SHMEM segment since otherwise,
     * DDS would not even be able to send a sample.
     *
     * The maxBufferSize is present in order to have a limit on the buffer size
     * for SHMEM so Perftest does not require unlimited resources.
     *
     * Since we are doing a decremental search, we use a step of 1MB to search
     * down a pseudo-optimal buffer size in the range
     * [minBufferSize, maxBufferSize].
     */
    RTIOsapiSharedMemorySegmentHandle handle;
    RTI_UINT64 pid = RTIOsapiProcess_getId();
    RTIBool success = RTI_FALSE;
    int retcode;
    int key = rand();
    int minBufferSize = parentMsgSizeMax;
    int step = 1048576 + parentMsgSizeMax; // 1MB + parent_msg_size_max
    int maxBufferSize = 60817408; // 58MB

    do {
        // Reset handles to known state
        RTIOsapiMemory_zero(&handle,
                sizeof(struct RTIOsapiSharedMemorySegmentHandle));

        success = RTIOsapiSharedMemorySegment_create(
                &handle, &retcode, key, maxBufferSize, pid);

        RTIOsapiSharedMemorySegment_delete(&handle);

        maxBufferSize -= step;
    } while (maxBufferSize > minBufferSize && success == RTI_FALSE);

    /** From user manual "Properties for Builtin Shared-Memory Transport":
     * To optimize memory usage, specify a receive queue size less than that
     * required to hold the maximum number of messages which are all of the
     * maximum size.
     *
     * In most situations, the average message size may be far less than the
     * maximum message size. So for example, if the maximum message size is 64K
     * bytes, and you configure the plugin to buffer at least 10 messages, then
     * 640K bytes of memory would be needed if all messages were 64K bytes.
     * Should this be desired, then receive_buffer_size should be set to 640K
     * bytes.
     *
     * However, if the average message size is only 10K bytes, then you could
     * set the receive_buffer_size to 100K bytes. This allows you to optimize
     * the memory usage of the plugin for the average case and yet allow the
     * plugin to handle the extreme case.
     *
     * The received_message_count_max should be set to a value that can hold
     * more than “-sendQueueSize" samples in perftest in order block in the
     * send window before starting to lose messages on the Shared Memory
     * transport
     */
    std::ostringstream ss;
    int flowControllerTokenSize = INT_MAX;
    unsigned long long datalen = _PM->get<unsigned long long>("dataLen");

  #ifdef RTI_FLATDATA_AVAILABLE
    // Zero Copy sends 16-byte references
    if (_PM->get<bool>("zerocopy")) datalen = 16;
  #endif

    int fragmentSize = (std::min)(
            parentMsgSizeMax - COMMEND_WRITER_MAX_RTPS_OVERHEAD,
            flowControllerTokenSize);

    // max(1, (sample_serialized_size / fragment_size))
    unsigned long long int rtpsMessagesPerSample = (std::max)(
            1ull, (perftest_cpp::OVERHEAD_BYTES + datalen) / fragmentSize);

    unsigned long long int receivedMessageCountMax =
            2 * (_PM->get<int>("sendQueueSize") + 1) * rtpsMessagesPerSample;

    // min(maxBufferSize, received_message_count_max * rtps_message_size)
    unsigned long long int receiveBufferSize = (std::min)(
        (unsigned long long) maxBufferSize,
        receivedMessageCountMax *
                (COMMEND_WRITER_MAX_RTPS_OVERHEAD + fragmentSize));

    // Avoid bottleneck due to SHMEM.
    ss << receivedMessageCountMax;
    qos_properties["dds.transport.shmem.builtin.received_message_count_max"] =
        ss.str();

    ss.str("");
    ss.clear();
    ss << receiveBufferSize;
    qos_properties["dds.transport.shmem.builtin.receive_buffer_size"] =
        ss.str();
}

bool configureTransport(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *_PM)
{
    using namespace rti::core::policy;

    /*
     * If transportConfig.kind is not set, then we want to use the value
     * provided by the Participant Qos, so first we read it from there and
     * update the value of transportConfig.kind with whatever was already set.
     */

    if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {

        TransportBuiltinMask mask = qos.policy<TransportBuiltin>().mask();

        if (mask == TransportBuiltinMask::udpv4()) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv4,
                "UDPv4",
                "dds.transport.UDPv4.builtin");
        } else if (mask == TransportBuiltinMask::udpv6()) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv6,
                "UDPv6",
                "dds.transport.UDPv6.builtin");
        } else if (mask == TransportBuiltinMask::shmem()) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_SHMEM,
                "SHMEM",
                "dds.transport.shmem.builtin");
        } else if (mask == (TransportBuiltinMask::udpv4()
                                | TransportBuiltinMask::shmem())) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv4_SHMEM,
                "UDPv4 & SHMEM",
                "dds.transport.UDPv4.builtin");
        } else if (mask == (TransportBuiltinMask::udpv4()
                                | TransportBuiltinMask::udpv6())) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv4_UDPv6,
                "UDPv4 & UDPv6",
                "dds.transport.UDPv4.builtin");
        } else if (mask == (TransportBuiltinMask::udpv6()
                                | TransportBuiltinMask::shmem())) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv6_SHMEM,
                "UDPv6 & SHMEM",
                "dds.transport.UDPv6.builtin");
        } else if (mask == (TransportBuiltinMask::udpv4()
                                | TransportBuiltinMask::udpv6()
                                | TransportBuiltinMask::shmem())) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv4_UDPv6_SHMEM,
                "UDPv4 & UDPv6 & SHMEM",
                "dds.transport.UDPv4.builtin");
        }

        /*
         * If we do not enter in any of these if statements, it
         * would mean that the mask is either empty or a
         * different value that we do not support yet. So we keep
         * the value as "TRANSPORT_NOT_SET"
         */

        transport.transportConfig.takenFromQoS = true;
    }

    switch (transport.transportConfig.kind) {

        case TRANSPORT_UDPv4:
            qos << rti::core::policy::TransportBuiltin(
                    TransportBuiltinMask::udpv4());
            break;

        case TRANSPORT_UDPv6:
            qos << rti::core::policy::TransportBuiltin(
                    TransportBuiltinMask::udpv6());
            break;

        case TRANSPORT_SHMEM:
            configureShmemTransport(transport, qos_properties, _PM);
            qos << TransportBuiltin(
                    TransportBuiltinMask::shmem());
            break;

        case TRANSPORT_TCPv4:
            if (!configureTcpTransport(transport, qos_properties, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure TCP plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        case TRANSPORT_TLSv4:
            if (!configureTcpTransport(transport, qos_properties, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure TCP + TLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        case TRANSPORT_DTLSv4:
            configureDtlsTransport(transport, qos_properties, _PM);
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        case TRANSPORT_WANv4:
            if (!configureWanTransport(transport, qos_properties, _PM)) {
                fprintf(stderr,
                        "%s Failed to configure Wan plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        default:

            /*
             * If shared memory is enabled we want to set up its
             * specific configuration
             */
            TransportBuiltinMask mask = qos.policy<TransportBuiltin>().mask();
            if ((mask & TransportBuiltinMask::shmem()) != 0) {
                configureShmemTransport(transport, qos_properties, _PM);
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
        setAllowInterfacesList(transport, qos, qos_properties, _PM);
    } else {
        // We are not using the allow interface string, so it is clean
        _PM->set<std::string>("allowInterfaces", std::string(""));
    }

    setTransportVerbosity(transport, qos, qos_properties, _PM);

    return true;
}

/******************************************************************************/
/* CLASS CONSTRUCTOR AND DESTRUCTOR */

PerftestTransport::PerftestTransport()
{

    multicastAddrMap[LATENCY_TOPIC_NAME] = TRANSPORT_MULTICAST_ADDR_LATENCY;
    multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] =
            TRANSPORT_MULTICAST_ADDR_ANNOUNCEMENT;
    multicastAddrMap[THROUGHPUT_TOPIC_NAME] = TRANSPORT_MULTICAST_ADDR_THROUGHPUT;

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
    stringStream << std::endl;

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
     * Manage parameter -allowInterfaces -nic
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

    // Manage parameter -multicastAddr
    if(_PM->is_set("multicastAddr")) {
        if (!parse_multicast_addresses(
                    _PM->get<std::string>("multicastAddr").c_str())) {
            fprintf(stderr, "Error parsing -multicastAddr\n");
            return false;
        }
        _PM->set<bool>("multicast", true);
    }

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
     * Increase the full address by one value.
     * if the Address is 255.255.255.255 (or the equivalent for IPv6) this
     * function will FAIL
     */
    for (int i = NDDS_TRANSPORT_ADDRESS_LENGTH - 1; i >= 0 && !success; i--) {
        if (isIPv4 && i < 9) {
            /* If the user set a IPv4 higher than 255.255.255.253 fail here*/
            break;
        }
        if (transportAddress.network_ordered_value[i] == 255) {
            transportAddress.network_ordered_value[i] = 0;
        } else {
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

    /* Try to get a IPv4 string format */
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
                || !NDDS_Transport_get_address(arg, 2, annonuncement)) {
            fprintf(stderr,
                    "Error parsing Address for -multicastAddr option\n");
            return false;
        }

        multicastAddrMap[THROUGHPUT_TOPIC_NAME] = throughput;
        multicastAddrMap[LATENCY_TOPIC_NAME] = latency;
        multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = annonuncement;

    } else if (numberOfAddressess == 1) {
        /* If only one address are given */
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
                    "Fail to increase the value of IP addres given\n");
            return false;
        }

        /* Calculate the consecutive one */
        if (!increase_address_by_one(
                multicastAddrMap[LATENCY_TOPIC_NAME],
                multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME])) {
            fprintf(stderr,
                    "Fail to increase the value of IP addres given\n");
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

    return true;
}
/******************************************************************************/
