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
        ParameterManager *parameterManager)
{
    using namespace rti::core::policy;

    if (!parameterManager->get<std::string>("allowInterfaces").empty()) {

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
                    parameterManager->get<std::string>("allowInterfaces");

            propertyName =
                    "dds.transport.UDPv6.builtin.parent.allow_interfaces";
            qos_properties[propertyName] =
                    parameterManager->get<std::string>("allowInterfaces");

        } else {

            std::string propertyName = transport.transportConfig.prefixString;

            if (transport.transportConfig.kind == TRANSPORT_WANv4) {
                propertyName += ".parent";
            }

            propertyName += ".parent.allow_interfaces";

            qos_properties[propertyName] =
                    parameterManager->get<std::string>("allowInterfaces");
        }
    }
}

void setTransportVerbosity(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{
    using namespace rti::core::policy;

    if (!parameterManager->get<std::string>("transportVerbosity").empty()) {

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
                parameterManager->get<std::string>("transportVerbosity");
    }
}

void configureSecurityFiles(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{

    if (!parameterManager->get<std::string>("transportCertAuthority").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".tls.verify.ca_file"] =
                        parameterManager->get<std::string>("transportCertAuthority");
    }

    if (!parameterManager->get<std::string>("transportCertFile").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".tls.identity.certificate_chain_file"] =
                        parameterManager->get<std::string>("transportCertFile");
    }

    if (!parameterManager->get<std::string>("transportPrivateKey").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".tls.identity.private_key_file"] =
                        parameterManager->get<std::string>("transportPrivateKey");
    }
}

bool configureTcpTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{
    qos_properties["dds.transport.load_plugins"] =
            transport.transportConfig.prefixString;

    if (!parameterManager->get<std::string>("transportServerBindPort").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".server_bind_port"] =
                        parameterManager->get<std::string>("transportServerBindPort");
    }

    if (parameterManager->get<bool>("transportWan")) {

        qos_properties[transport.transportConfig.prefixString
                + ".parent.classid"] = "NDDS_TRANSPORT_CLASSID_TCPV4_WAN";

        if (parameterManager->get<std::string>("transportServerBindPort") != "0") {
            if (!parameterManager->get<std::string>("transportPublicAddress").empty()) {

                qos_properties[transport.transportConfig.prefixString
                        + ".public_address"] =
                                parameterManager->get<std::string>("transportPublicAddress");
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

        if (parameterManager->get<bool>("transportWan")) {
            qos_properties[transport.transportConfig.prefixString
                    + ".parent.classid"] = "NDDS_TRANSPORT_CLASSID_TLSV4_WAN";
        } else {
            qos_properties[transport.transportConfig.prefixString
                    + ".parent.classid"] = "NDDS_TRANSPORT_CLASSID_TLSV4_LAN";
        }

        configureSecurityFiles(transport, qos_properties, parameterManager);
    }

    return true;
}

void configureDtlsTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{
    qos_properties["dds.transport.load_plugins"] = transport.transportConfig
            .prefixString;

    qos_properties[transport.transportConfig.prefixString + ".library"] =
            "nddstransporttls";

    qos_properties[transport.transportConfig.prefixString + ".create_function"] =
            "NDDS_Transport_DTLS_create";

    configureSecurityFiles(transport, qos_properties, parameterManager);
}

bool configureWanTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{

    qos_properties["dds.transport.load_plugins"] =
            transport.transportConfig.prefixString;

    qos_properties[transport.transportConfig.prefixString
                   + ".library"] = "nddstransportwan";

    qos_properties[transport.transportConfig.prefixString
                   + ".create_function"] = "NDDS_Transport_WAN_create";

    if (!parameterManager->get<std::string>("transportWanServerAddress").empty()) {
        qos_properties[transport.transportConfig.prefixString
                        + ".server"] =
                                parameterManager->get<std::string>("transportWanServerAddress");
    } else {
        fprintf(stderr,
                "%s Wan Server Address is required\n",
                classLoggingString.c_str());
        return false;
    }

    if (!parameterManager->get<std::string>("transportWanServerPort").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".server_port"] =
                        parameterManager->get<std::string>("transportWanServerPort");
    }

    if (!parameterManager->get<std::string>("transportWanId").empty()) {
        qos_properties[transport.transportConfig.prefixString
                + ".transport_instance_id"] =
                        parameterManager->get<std::string>("transportWanId");
    } else {
        fprintf(stderr, "%s Wan ID is required\n", classLoggingString.c_str());
        return false;
    }

    if (parameterManager->get<bool>("transportSecureWan")) {
        qos_properties[transport.transportConfig.prefixString
                       + ".enable_security"] = "1";
        configureSecurityFiles(transport, qos_properties, parameterManager);
    }

    return true;
}


#ifdef PERFTEST_CONNEXT_PRO_610
bool configureUdpv4WanTransport(
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{

    if (!parameterManager->get<std::string>("transportPublicAddress").empty()) {

        std::string publicAddress
                = parameterManager->get<std::string>("transportPublicAddress");
        std::string delimiter = ":";

        size_t pos = publicAddress.find(delimiter);
        if (pos == std::string::npos) {
            fprintf(stderr,
                    "%s Wan Public Address format invalid. Use "
                    "<public_address>:<public_port>\n",
                    classLoggingString.c_str());
            return false;
        }

        std::string publicAddressIp
                = publicAddress.substr(0, pos);
        std::string publicAddressPort
                = publicAddress.substr(pos+1, std::string::npos);

        // If transportHostPort is defined use that port for the transport
        std::string publicAddressHostPort;
        if (!parameterManager->get<std::string>("transportHostPort").empty()) {
            std::string transportHostPort = parameterManager->get<std::string>("transportHostPort");
            publicAddressHostPort = transportHostPort;
        } else {  // Use Internal Port as external port
            publicAddressHostPort = publicAddressPort;
        }

        qos_properties[transport.transportConfig.prefixString + ".public_address"] = publicAddressIp;

        std::string json_string_comm_ports(
                "{ \"default\": { \"host\": "
                + publicAddressHostPort
                + ",  \"public\": "
                + publicAddressPort
                + " } }");

        qos_properties[transport.transportConfig.prefixString + ".comm_ports"] = json_string_comm_ports;
    }

    return true;
}
#endif // PERFTEST_CONNEXT_PRO_610

void configureShmemTransport(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{
    using namespace rti::core::policy;
    unsigned long long datalen = parameterManager->get<unsigned long long>("dataLen");

    long parentMsgSizeMax = transport.minimumMessageSizeMax;
    std::ostringstream ss;
    bool messageSizeMaxSet = false;

    /*
     * If the property defining the message_size_max for shared memory is not
     * set and we are using exclusively SHMEM, then we will calculate
     * automatically the message_size_max to accomodate the sample in a single
     * packet and avoid fragmentation.
     */
    TransportBuiltinMask mask = qos.policy<TransportBuiltin>().mask();
    if (qos_properties.count("dds.transport.shmem.builtin.parent.message_size_max")) {
        parentMsgSizeMax = atoi(qos_properties["dds.transport.shmem.builtin.parent.message_size_max"].c_str());
        messageSizeMaxSet = true;
    } else {
        if (mask == TransportBuiltinMask::shmem()) {
            if ((datalen + MESSAGE_OVERHEAD_BYTES)
                    > (unsigned long long) parentMsgSizeMax) {
                parentMsgSizeMax = datalen + MESSAGE_OVERHEAD_BYTES;

                if (parameterManager->get<bool>("flatdata")) {
                    /*
                     * TODO: This should not be needed after adding changes for #265
                     * Rework idls to handle better custom types and Flat Data
                     *
                     * This 17 is due to the incorrect setting of OVERHEAD_BYTES in
                     * perftest_cpp. which in the case of Flat Data is not right.
                     */
                    parentMsgSizeMax += 17;
                }

                transport.minimumMessageSizeMax = parentMsgSizeMax;
            }
        }

        ss.str("");
        ss.clear();
        ss << parentMsgSizeMax;
        qos_properties["dds.transport.shmem.builtin.parent.message_size_max"] = ss.str();
        transport.loggingString +=
                ("\tSHMEM message_size_max: "
                + ss.str()
                + "\n");
    }

    /*
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
    long minBufferSize = 1048576; //1MB
    int step = 1048576; // 1MB
    long maxBufferSize = (std::max)((long) 121634816 /* 116MB */, parentMsgSizeMax);

    do {
        // Reset handles to known state
        RTIOsapiMemory_zero(&handle,
                sizeof(struct RTIOsapiSharedMemorySegmentHandle));

        success = RTIOsapiSharedMemorySegment_create(
                &handle, &retcode, key, maxBufferSize, pid);

        RTIOsapiSharedMemorySegment_delete(&handle);

        if (success) {
            break;
        }

        maxBufferSize -= step;
    } while (maxBufferSize > minBufferSize);

    if (maxBufferSize < minBufferSize) {
        fprintf(stderr,
                "%s Failed to allocate SHMEM segment of 1MB.\n"
                "Change OS settings to test SHMEM.\n",
                classLoggingString.c_str());
    }

    if (!messageSizeMaxSet &&
            parentMsgSizeMax > maxBufferSize) {
        parentMsgSizeMax = maxBufferSize;

        transport.minimumMessageSizeMax = parentMsgSizeMax;
        ss.str("");
        ss.clear();
        ss << maxBufferSize;
        qos_properties["dds.transport.shmem.builtin.parent.message_size_max"] = ss.str();
        transport.loggingString +=
                ("\tSHMEM message_size_max: "
                + ss.str()
                + "\n");
    }

    /*
     * From user manual "Properties for Builtin Shared-Memory Transport":
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

    /*
     * This is the flow Controller default token size. Change this if you modify
     * the qos file to add a different "bytes_per_token" property
     */
    long flowControllerTokenSize = transport.minimumMessageSizeMax;

  #ifdef RTI_FLATDATA_AVAILABLE
    // Zero Copy sends 16-byte references
    if (parameterManager->get<bool>("zerocopy")) {
        datalen = 16;
    }
  #endif

    /*
     * We choose the minimum between the flow Controller max fragment size and
     * the message_size_max - RTPS headers.
     */
    int fragmentSize = (std::min)(
            parentMsgSizeMax - TMP_COMMEND_WRITER_MAX_RTPS_OVERHEAD,
            flowControllerTokenSize - TMP_COMMEND_WRITER_MAX_RTPS_OVERHEAD);

    unsigned long long rtpsMessagesPerSample = (std::max)(
            1ull, (datalen / fragmentSize) + 1);

    unsigned long long receivedMessageCountMax =
            2 * (parameterManager->get<int>("sendQueueSize") + 1) * rtpsMessagesPerSample;

    unsigned long long receiveBufferSize = (std::min)(
        (unsigned long long) maxBufferSize,
        receivedMessageCountMax *
                (TMP_COMMEND_WRITER_MAX_RTPS_OVERHEAD + fragmentSize));

    if (qos_properties["dds.transport.shmem.builtin.received_message_count_max"].empty()) {
        ss.str("");
        ss.clear();
        ss << receivedMessageCountMax;
        qos_properties["dds.transport.shmem.builtin.received_message_count_max"] = ss.str();
        transport.loggingString +=
                ("\tSHMEM received_message_count_max: "
                + ss.str()
                + "\n");
    }

    if (qos_properties["dds.transport.shmem.builtin.receive_buffer_size"].empty()) {
        ss.str("");
        ss.clear();
        ss << receiveBufferSize;
        qos_properties["dds.transport.shmem.builtin.receive_buffer_size"] = ss.str();
        transport.loggingString +=
                ("\tSHMEM receive_buffer_size: "
                + ss.str()
                + "\n");
    }
}

/*
 * Gets the MessageSizeMax given the name (String) of a transport from a
 * DDS_DomainParticipantQos object. If the value is not present, returns
 * DEFAULT_MESSAGE_SIZE_MAX.
 */
long getTransportMessageSizeMax(
        std::string targetTransportName,
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties)
{
    std::string propertyName =
            transport.transportConfigMap[targetTransportName].prefixString
            + ".parent.message_size_max";

    if (qos_properties.count(propertyName)) {
        return atoi(qos_properties[propertyName].c_str());
    } else {
        return DEFAULT_MESSAGE_SIZE_MAX;
    }
}

/*
 * Sets the MessageSizeMax given the name (String) of a transport in a
 * property object.
 */
void setTransportMessageSizeMax(
        std::string targetTransportName,
        PerftestTransport &transport,
        std::map<std::string, std::string> &qos_properties,
        long messageSizeMax)
{
    std::string propertyName =
            transport.transportConfigMap[targetTransportName].prefixString
            + ".parent.message_size_max";
    
    qos_properties[propertyName] = std::to_string(messageSizeMax);

}

/*
 * Configures the minimumMessageSizeMax value in the PerftestTransport object with
 * the minimum value for all the enabled transports in the XML configuration.
 */
bool setupTransportMinimumMessageSizeMax(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
{
    using namespace rti::core::policy;

    long qosConfigurationMessageSizeMax = MESSAGE_SIZE_MAX_NOT_SET;
    long transportMessageSizeMax = MESSAGE_SIZE_MAX_NOT_SET;
    TransportBuiltinMask mask = qos.policy<TransportBuiltin>().mask();

    if (parameterManager->is_set("messageSizeMax")) {

        transportMessageSizeMax = parameterManager->get<int>("messageSizeMax");
        qosConfigurationMessageSizeMax = transportMessageSizeMax;
        transport.loggingString += ("\tmessage_size_max: "
                + std::to_string(transportMessageSizeMax)
                + " (set via command line)\n");

        if ((mask & TransportBuiltinMask::shmem()) != 0) {
            setTransportMessageSizeMax("SHMEM", transport, qos_properties, transportMessageSizeMax);
        }
        if ((mask & TransportBuiltinMask::udpv4()) != 0) {
            setTransportMessageSizeMax("UDPv4", transport, qos_properties, transportMessageSizeMax);
        }
        if ((mask & TransportBuiltinMask::udpv6()) != 0) {
            setTransportMessageSizeMax("UDPv6", transport, qos_properties, transportMessageSizeMax);
        }
        if (transport.transportConfig.kind == TRANSPORT_TCPv4
                || transport.transportConfig.kind == TRANSPORT_TLSv4) {
            setTransportMessageSizeMax("TCP", transport, qos_properties, transportMessageSizeMax);
        }
        if (transport.transportConfig.kind == TRANSPORT_DTLSv4) {
            setTransportMessageSizeMax("DTLS", transport, qos_properties, transportMessageSizeMax);
        }
        if (transport.transportConfig.kind == TRANSPORT_WANv4) {
            setTransportMessageSizeMax("WAN", transport, qos_properties, transportMessageSizeMax);
        }
      #ifdef PERFTEST_CONNEXT_PRO_610
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_WAN) {
            setTransportMessageSizeMax("UDPv4_WAN", transport, qos_properties, transportMessageSizeMax);
        }
      #endif

    } else {

        if ((mask & TransportBuiltinMask::shmem()) != 0) {
            transportMessageSizeMax = getTransportMessageSizeMax("SHMEM", transport, qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if ((mask & TransportBuiltinMask::udpv4()) != 0) {
            transportMessageSizeMax = getTransportMessageSizeMax(
                    "UDPv4",
                    transport,
                    qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if ((mask & TransportBuiltinMask::udpv6()) != 0) {
            transportMessageSizeMax = getTransportMessageSizeMax(
                    "UDPv6",
                    transport,
                    qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_TCPv4
                || transport.transportConfig.kind == TRANSPORT_TLSv4) {
            transportMessageSizeMax = getTransportMessageSizeMax(
                    "TCP",
                    transport,
                    qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_DTLSv4) {
            transportMessageSizeMax = getTransportMessageSizeMax(
                    "DTLS",
                    transport,
                    qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
        if (transport.transportConfig.kind == TRANSPORT_WANv4) {
            transportMessageSizeMax = getTransportMessageSizeMax(
                    "WAN",
                    transport,
                    qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
      #ifdef PERFTEST_CONNEXT_PRO_610
        if (transport.transportConfig.kind == TRANSPORT_UDPv4_WAN) {
            transportMessageSizeMax = getTransportMessageSizeMax(
                    "UDPv4_WAN",
                    transport,
                    qos_properties);

            if (transportMessageSizeMax < qosConfigurationMessageSizeMax) {
                qosConfigurationMessageSizeMax = transportMessageSizeMax;
            }
        }
      #endif
        transport.loggingString += ("\tmessage_size_max: "
                + std::to_string(qosConfigurationMessageSizeMax)
                + " (minimum across all transports)\n");
    }

    transport.minimumMessageSizeMax = qosConfigurationMessageSizeMax;
    return true;
}

bool configureTransport(
        PerftestTransport &transport,
        dds::domain::qos::DomainParticipantQos &qos,
        std::map<std::string, std::string> &qos_properties,
        ParameterManager *parameterManager)
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
      #ifdef PERFTEST_CONNEXT_PRO_610
        else if (mask == TransportBuiltinMask::udpv4_wan()) {
            transport.transportConfig = TransportConfig(
                TRANSPORT_UDPv4_WAN,
                "UDPv4_WAN",
                "dds.transport.UDPv4_WAN.builtin");
        }
      #endif
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
            qos <<  rti::core::policy::TransportBuiltin(
                    TransportBuiltinMask::shmem());
            break;
            
      #ifdef PERFTEST_CONNEXT_PRO_610
        case TRANSPORT_UDPv4_WAN:
            qos <<  rti::core::policy::TransportBuiltin(
                    TransportBuiltinMask::udpv4_wan());
            break;
      #endif // PERFTEST_CONNEXT_PRO_610

        default:
            break;
    };

    /*
     * Once the configurations have been stablished, we can get the
     * MessageSizeMax for the Transport, which should be the minimum of
     * all the enabled transports
     */
    setupTransportMinimumMessageSizeMax(transport, qos, qos_properties, parameterManager);

    switch (transport.transportConfig.kind) {

        case TRANSPORT_UDPv4:
        case TRANSPORT_UDPv6:
            break;

        case TRANSPORT_SHMEM:
            configureShmemTransport(transport, qos, qos_properties, parameterManager);
            break;

      #ifdef PERFTEST_CONNEXT_PRO_610
        case TRANSPORT_UDPv4_WAN:
            configureUdpv4WanTransport(transport, qos_properties, parameterManager);
            break;
      #endif // PERFTEST_CONNEXT_PRO_610

        case TRANSPORT_TCPv4:
            if (!configureTcpTransport(transport, qos_properties, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure TCP plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        case TRANSPORT_TLSv4:
            if (!configureTcpTransport(transport, qos_properties, parameterManager)) {
                fprintf(stderr,
                        "%s Failed to configure TCP + TLS plugin\n",
                        classLoggingString.c_str());
                return false;
            }
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        case TRANSPORT_DTLSv4:
            configureDtlsTransport(transport, qos_properties, parameterManager);
            qos << TransportBuiltin(
                    TransportBuiltinMask::none());
            break;

        case TRANSPORT_WANv4:
            if (!configureWanTransport(transport, qos_properties, parameterManager)) {
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
                configureShmemTransport(transport, qos, qos_properties, parameterManager);
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
        setAllowInterfacesList(transport, qos, qos_properties, parameterManager);
    } else {
        // We are not using the allow interface string, so it is clean
        parameterManager->set<std::string>("allowInterfaces", std::string(""));
    }

    setTransportVerbosity(transport, qos, qos_properties, parameterManager);

    return true;
}

/******************************************************************************/
/* CLASS CONSTRUCTOR AND DESTRUCTOR */

PerftestTransport::PerftestTransport()
{

    multicastAddrMap[LATENCY_TOPIC_NAME] = TRANSPORT_MULTICAST_ADDR_LATENCY;
    multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = TRANSPORT_MULTICAST_ADDR_ANNOUNCEMENT;
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

    minimumMessageSizeMax = MESSAGE_SIZE_MAX_NOT_SET;
    loggingString = "";
}

PerftestTransport::~PerftestTransport()
{
}

void PerftestTransport::initialize(ParameterManager *PM)
{
    parameterManager = PM;
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

    if (parameterManager->get<std::string>("transportCertFile").empty()) {
        if (parameterManager->get<bool>("pub")) {
            parameterManager->set("transportCertFile", TRANSPORT_CERTIFICATE_FILE_PUB);
        } else {
            parameterManager->set("transportCertFile", TRANSPORT_CERTIFICATE_FILE_SUB);
        }
    }

    if (parameterManager->get<std::string>("transportPrivateKey").empty()) {
        if (parameterManager->get<bool>("pub")) {
            parameterManager->set("transportPrivateKey", TRANSPORT_PRIVATEKEY_FILE_PUB);
        } else {
            parameterManager->set("transportPrivateKey", TRANSPORT_PRIVATEKEY_FILE_SUB);
        }
    }

    if (parameterManager->get<std::string>("transportCertAuthority").empty()) {
        parameterManager->set("transportCertAuthority", TRANSPORT_CERTAUTHORITY_FILE);
    }
}

/******************************************************************************/
/* PUBLIC METHODS */

std::string PerftestTransport::printTransportConfigurationSummary()
{
    std::ostringstream stringStream;
    stringStream << "\nTransport Configuration:\n";
    stringStream << "\tKind: " << transportConfig.nameString;
    if (transportConfig.takenFromQoS) {
        stringStream << " (taken from QoS XML file)";
    }
    stringStream << std::endl;

    if (!parameterManager->get<std::string>("allowInterfaces").empty()) {
        stringStream << "\tNic: "
                     << parameterManager->get<std::string>("allowInterfaces")
                     << "\n";
    }

    stringStream << "\tUse Multicast: "
                 << ((allowsMulticast() && parameterManager->get<bool>("multicast"))
                        ? "True" : "False");
    if (!allowsMulticast() && parameterManager->get<bool>("multicast")) {
        stringStream << " (Multicast is not supported for "
                     << transportConfig.nameString << ")";
    }
    stringStream << "\n";

    if (parameterManager->is_set("multicastAddr")) {
        stringStream << "\tUsing custom Multicast Addresses:"
                     << "\n\t\tThroughtput Address: "
                     << multicastAddrMap[THROUGHPUT_TOPIC_NAME].c_str()
                     << "\n\t\tLatency Address: "
                     << multicastAddrMap[LATENCY_TOPIC_NAME].c_str()
                     << "\n\t\tAnnouncement Address: "
                     << multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME].c_str()
                     << "\n";
    }

    if (!loggingString.empty()) {
        stringStream << loggingString;
    }

    if (transportConfig.kind == TRANSPORT_TCPv4
            || transportConfig.kind == TRANSPORT_TLSv4) {
        stringStream << "\tTCP Server Bind Port: "
                     << parameterManager->get<std::string>("transportServerBindPort")
                     << "\n";

        stringStream << "\tTCP LAN/WAN mode: "
                     << (parameterManager->get<bool>("transportWan") ? "WAN\n" : "LAN\n");
        if (parameterManager->get<bool>("transportWan")) {
            stringStream << "\tTCP Public Address: "
                         << parameterManager->get<std::string>("transportPublicAddress")
                         << "\n";
        }
    }

    if (transportConfig.kind == TRANSPORT_WANv4) {
        stringStream << "\tWAN Server Address: "
                     << parameterManager->get<std::string>("transportWanServerAddress")
                     << ":"
                     << parameterManager->get<std::string>("transportWanServerPort")
                     << "\n";
        stringStream << "\tWAN Id: "
                     << parameterManager->get<std::string>("transportWanId")
                     << "\n";
        stringStream << "\tWAN Secure: "
                    << parameterManager->get<bool>("transportSecureWan")
                    << "\n";

    }

    if (transportConfig.kind == TRANSPORT_TLSv4
            || transportConfig.kind == TRANSPORT_DTLSv4
            || (transportConfig.kind == TRANSPORT_WANv4
            && parameterManager->get<bool>("transportSecureWan"))) {
        stringStream << "\tCertificate authority file: "
                     << parameterManager->get<std::string>("transportCertAuthority")
                     << "\n";
        stringStream << "\tCertificate file: "
                     << parameterManager->get<std::string>("transportCertFile")
                     << "\n";
        stringStream << "\tPrivate key file: "
                     << parameterManager->get<std::string>("transportPrivateKey")
                     << "\n";
    }

    if (!parameterManager->get<std::string>("transportVerbosity").empty()) {
        stringStream << "\tVerbosity: "
                     << parameterManager->get<std::string>("transportVerbosity")
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
    if (parameterManager->get<std::string>("allowInterfaces").empty()) {
        parameterManager->set<std::string>("allowInterfaces", parameterManager->get<std::string>("nic"));
    }

    // Manage parameter -transport
    if (!setTransport(parameterManager->get<std::string>("transport"))) {
        fprintf(stderr,
                "%s Error Setting the transport\n",
                classLoggingString.c_str());
        return false;
    }

    // Manage parameter -multicastAddr
    if(parameterManager->is_set("multicastAddr")) {
        if (!parse_multicast_addresses(
                    parameterManager->get<std::string>("multicastAddr").c_str())) {
            fprintf(stderr, "Error parsing -multicastAddr\n");
            return false;
        }
        parameterManager->set<bool>("multicast", true);
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

bool PerftestTransport::get_number_of_addresses_in_string(
        unsigned int *number,
        const char *input_string)
{
    #ifdef PERFTEST_CONNEXT_PRO_750
        return NDDS_Transport_get_number_of_addresses_in_string(
                number,
                input_string);
    #else
        return NDDS_Transport_get_number_of_addresses_in_string(
                input_string,
                number);
    #endif
}

bool PerftestTransport::get_address(
    char *output_address,
    size_t output_buffer_size,
    const char *input_string,
    int index)
{
  #ifdef PERFTEST_CONNEXT_PRO_750
    return NDDS_Transport_get_address(output_address, output_buffer_size, input_string, index);
  #else
    return NDDS_Transport_get_address(input_string, index, output_address);
  #endif
}

bool PerftestTransport::parse_multicast_addresses(const char *arg)
{
    char throughput[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    char latency[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    char announcement[NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE];
    unsigned int numberOfAddresses = 0;

    /* Get number of addresses on the string */
    if (!get_number_of_addresses_in_string(
            &numberOfAddresses,
            arg)) {
        fprintf(stderr, "Error parsing Address for -multicastAddr option\n");
        return false;
    }

    /* If three addresses are given */
    if (numberOfAddresses == 3) {
        if (!get_address(throughput, NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE, arg, 0)
                || !get_address(latency, NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE, arg, 1)
                || !get_address(announcement, NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE, arg, 2)) {
            fprintf(stderr,
                    "Error parsing Address for -multicastAddr option\n");
            return false;
        }

        multicastAddrMap[THROUGHPUT_TOPIC_NAME] = throughput;
        multicastAddrMap[LATENCY_TOPIC_NAME] = latency;
        multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = announcement;

    } else if (numberOfAddresses == 1) {
        /* If only one address are given */
        if (!get_address(throughput, NDDS_TRANSPORT_ADDRESS_STRING_BUFFER_SIZE, arg, 0)) {
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
        if (numberOfAddresses == 1) {
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
