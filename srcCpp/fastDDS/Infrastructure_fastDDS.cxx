/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef PERFTEST_EPROSIMA_FASTDDS

#include "Infrastructure_fastDDS.h"

using namespace eprosima::fastdds::rtps;
using namespace eprosima::fastdds::dds;

/********************************************************************/
/* Transport related functions */

bool configure_udpv4_transport(
        PerftestTransport &transport,
        DomainParticipantQos &qos,
        ParameterManager *_PM)
{
    qos.transport().use_builtin_transports = false;

    std::shared_ptr<UDPv4TransportDescriptor> udpTransport = std::make_shared<UDPv4TransportDescriptor>();
    /*
     * maxMessageSize:
     * When setting a value greater than 65500, the following error will show
     * [RTPS_MSG_OUT Error] maxMessageSize cannot be greater than 65000
     * So we will use the limit (which happens to be also its default value)
     * udpTransport->maxMessageSize = 65500;
     */
    udpTransport->sendBufferSize = 8912896; // 8.5Mb
    udpTransport->receiveBufferSize = 8912896; // 8.5Mb

    /* Use only interface supplied by command line */
    if (!_PM->get<std::string>("allowInterfaces").empty()) {
        udpTransport->interfaceWhiteList.clear();
        udpTransport->interfaceWhiteList.emplace_back(
                _PM->get<std::string>("allowInterfaces"));
    }

    qos.transport().user_transports.push_back(udpTransport);

    // Increase the sending buffer size
    qos.transport().send_socket_buffer_size = 8912896;
    // Increase the receiving buffer size
    qos.transport().listen_socket_buffer_size = 8912896;

    return true;
}

bool configure_shmem_transport(
        PerftestTransport &transport,
        DomainParticipantQos &qos,
        ParameterManager *_PM)
{
    qos.transport().use_builtin_transports = false;

    std::shared_ptr<SharedMemTransportDescriptor> shmTransport =
            std::make_shared<SharedMemTransportDescriptor>();

    // TODO: We need to calculate the size of the segment, I guess it should be
    // the size of the sample + X
    // shmTransport->segment_size(2 * 1024 * 1024);
    // shmTransport->maxMessageSize(2 * 1024 * 1024);
    // port_queue_capacity 1024
    // healthy_check_timeout_ms 250
    // rtps_dump_file /rtps_dump_file
    qos.transport().user_transports.push_back(shmTransport);

    // Increase the sending buffer size
    qos.transport().send_socket_buffer_size = 1048576;
    // Increase the receiving buffer size
    qos.transport().listen_socket_buffer_size = 4194304;

    return true;
}

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DomainParticipantQos &qos,
        ParameterManager *_PM)
{
    if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
        transport.transportConfig.kind = TRANSPORT_UDPv4;
        transport.transportConfig.nameString = "UDPv4 (Default)";
    }

    switch (transport.transportConfig.kind) {
    case TRANSPORT_UDPv4:
        return configure_udpv4_transport(transport, qos, _PM);
    case TRANSPORT_SHMEM:
        return configure_shmem_transport(transport, qos, _PM);
    default:
        fprintf(stderr,
                "%s Transport is not yet supported in Perftest for FastDDS\n",
                classLoggingString.c_str());
        return false;

    }  // Switch

    return true;
}

#endif // PERFTEST_EPROSIMA_FASTDDS
