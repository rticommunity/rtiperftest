/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "Infrastructure_pro.h"

/* Perftest Clock class */

PerftestClock::PerftestClock()
{
    clock = RTIHighResolutionClock_new();
    RTINtpTime_setZero(&clockTime_aux);

    clock_sec = 0;
    clock_usec = 0;
}

PerftestClock::~PerftestClock() 
{
    if (clock != NULL) {
        RTIHighResolutionClock_delete(clock);
    }
}

unsigned long long PerftestClock::GetTimeUsec() 
{
    clock->getTime(clock, &clockTime_aux);
    RTINtpTime_unpackToMicrosec(
            clock_sec,
            clock_usec,
            clockTime_aux);
    return clock_usec + 1000000 * clock_sec;
}

void PerftestClock::MilliSleep(unsigned int millisec)
{
  #if defined(RTI_WIN32)
    Sleep(millisec);
  #elif defined(RTI_VXWORKS)
    DDS_Duration_t sleep_period = {0, millisec * 1000000};
    NDDSUtility::sleep(sleep_period);
  #else
    usleep(millisec * 1000);
  #endif
}

const std::string classLoggingString = "PerftestTransport:";

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

        /*
         * By default, if the transport is not set, it should be UDPv4, if it is not
         * It means that we have modified the QOS, so we won't use the -nic param.
         */
        if (transport.transportConfig.kind == TRANSPORT_DEFAULT
                && qos.transport_builtin.mask != DDS_TRANSPORTBUILTIN_UDPv4) {
            fprintf(
                    stderr,
                    "%s Ignoring -nic option, "
                    "Transport has been modified via QoS\n",
                    classLoggingString.c_str());
            return true;
        }

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

    return true;
}

bool setTransportVerbosity(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos)
{
    if (!transport.verbosity.empty()) {

        /*
         * By default, if the transport is not set, it should be UDPv4, if it is not
         * It means that we have modified the QOS, so we won't use the
         * -transportVerbosity param.
         */
        if (transport.transportConfig.kind == TRANSPORT_DEFAULT
                && qos.transport_builtin.mask != DDS_TRANSPORTBUILTIN_UDPv4) {
            fprintf(
                    stderr,
                    "%s Ignoring -transportVerbosity option, "
                    "Transport has been modified via QoS\n",
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
                || transport.transportConfig.kind == TRANSPORT_SHMEM) {
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

    qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_SHMEM;

    // SHMEM transport properties
    int received_message_count_max = 1024 * 1024 * 2
            / (int) transport.dataLen;
    if (received_message_count_max < 1) {
        received_message_count_max = 1;
    }

    std::ostringstream string_stream_object;
    string_stream_object << received_message_count_max;
    if (!assertPropertyToParticipantQos(
            qos,
            transport.transportConfig.prefixString
                    + ".received_message_count_max",
            string_stream_object.str())) {
        return false;
    }
    return true;
}

bool configureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos)
{

    switch (transport.transportConfig.kind) {

    case TRANSPORT_DEFAULT:
        /* fprintf(stderr,
                "%s Using default configuration (xml)\n",
                classLoggingString.c_str()); */
        break;

    case TRANSPORT_UDPv4:
        qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv4;
        break;

    case TRANSPORT_UDPv6:
        qos.transport_builtin.mask = DDS_TRANSPORTBUILTIN_UDPv6;
        break;

    case TRANSPORT_SHMEM:
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
        fprintf(stderr,
                "%s Transport is not supported\n",
                classLoggingString.c_str());
        return false;

    } // Switch

    if (transport.transportConfig.kind != TRANSPORT_SHMEM) {
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

void PerftestCreateThread(
        const char *name,
        RTIOsapiThreadOnSpawnedMethod method,
        void *threadParam)
{
    RTIOsapiThread_new(
            name,
            RTI_OSAPI_THREAD_PRIORITY_DEFAULT,
            RTI_OSAPI_THREAD_OPTION_DEFAULT,
            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
            NULL,
            method,
            threadParam);
}

void configureVerbosity(int verbosityLevel)
{
    
    NDDS_Config_LogVerbosity verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
    switch (verbosityLevel) {
        case 0: verbosity = NDDS_CONFIG_LOG_VERBOSITY_SILENT;
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
        case 1: verbosity = NDDS_CONFIG_LOG_VERBOSITY_ERROR;
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
        case 2: verbosity = NDDS_CONFIG_LOG_VERBOSITY_WARNING;
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
        case 3: verbosity = NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL;
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
        default: fprintf(stderr,
                    "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                break;
    }
    NDDSConfigLogger::get_instance()->set_verbosity(verbosity);
}
