/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef PERFTEST_CYCLONEDDS

#include "Infrastructure_cycloneDDS.h"

/********************************************************************/
/* Perftest Clock class */

PerftestClock::PerftestClock()
{

}

PerftestClock::~PerftestClock()
{
}

PerftestClock &PerftestClock::getInstance()
{
    static PerftestClock instance;
    return instance;
}

unsigned long long PerftestClock::getTime()
{
    clock_gettime(CLOCK_MONOTONIC, &timeStruct);
    return (timeStruct.tv_sec * ONE_MILLION) + timeStruct.tv_nsec/1000;
}

void PerftestClock::milliSleep(unsigned int millisec)
{
    OSAPI_Thread_sleep(millisec);
}

void PerftestClock::sleep(const struct DDS_Duration_t& sleep_period)
{
    NDDSUtility::sleep(sleep_period);
}

/********************************************************************/
/* Get Connext Micro functions */

const std::string GetDDSVersionString()
{
    return "Eprosima FastDDS";
}

void PerftestConfigureVerbosity(int verbosityLevel)
{

    // OSAPI_LogVerbosity_T verbosity = OSAPI_LOG_VERBOSITY_ERROR;
    // switch (verbosityLevel) {
    //     case 0: fprintf(stderr, "[Error]: Cannot set verbosity to SILENT\n");
    //             break;
    //     case 1: Log::SetVerbosity(Log::Error);
    //             fprintf(stderr, "Setting verbosity to ERROR\n");
    //             break;
    //     case 2: Log::SetVerbosity(Log::Warning);
    //             fprintf(stderr, "Setting verbosity to WARNING\n");
    //             break;
    //     case 3: Log::SetVerbosity(Log::Info);
    //             fprintf(stderr, "Setting verbosity to INFO\n");
    //             break;
    //     default: fprintf(stderr,
    //                 "[Error]: Invalid value for the verbosity parameter. Using default\n");
    //             break;
    // }
}

/********************************************************************/
/* Micro OnSpawed Method */

struct PerftestMicroThreadOnSpawnedMethod
{
    MicroThreadOnSpawnedMethod method;
    void *thread_param;

};

static int perftestMicroThreadRoutine(struct OSAPI_ThreadInfo *thread_info)
{
    PerftestMicroThreadOnSpawnedMethod *data = (PerftestMicroThreadOnSpawnedMethod *)thread_info->user_data;

    data->method(data->thread_param);

    delete(data);

    thread_info->stop_thread = RTI_TRUE;
    return 1;
}

/********************************************************************/
/* Thread Related functions */

struct PerftestThread* PerftestThread_new(
        const char *name,
        int threadPriority,
        int threadOptions,
        MicroThreadOnSpawnedMethod method,
        void *threadParam)
{
    struct OSAPI_ThreadProperty prio = OSAPI_ThreadProperty_INITIALIZER;
    PerftestMicroThreadOnSpawnedMethod *data = new PerftestMicroThreadOnSpawnedMethod();
    data->method = method;
    data->thread_param = threadParam;

    struct OSAPI_Thread *thread = NULL;
    thread = OSAPI_Thread_create(
                name,
                &prio,
                perftestMicroThreadRoutine,
                data,
                NULL);
    if (!OSAPI_Thread_start(thread)) {
        return NULL;
    }
    return thread;
}

void PerftestThread_delete(struct PerftestThread* thread)
{
    if (!OSAPI_Thread_destroy(thread)) {
        printf("Error deleting thread");
    }
}

/********************************************************************/
/* Transport related functions */

// bool configure_udpv4_transport(
//         PerftestTransport &transport,
//         DomainParticipantQos &qos,
//         ParameterManager *_PM)
// {
//     qos.transport().use_builtin_transports = false;

//     std::shared_ptr<UDPv4TransportDescriptor> udpTransport = std::make_shared<UDPv4TransportDescriptor>();
//     /*
//      * maxMessageSize:
//      * When setting a value greater than 65500, the following error will show
//      * [RTPS_MSG_OUT Error] maxMessageSize cannot be greater than 65000
//      * So we will use the limit (which happens to be also its default value)
//      * udpTransport->maxMessageSize = 65500;
//      */
//     udpTransport->sendBufferSize = 8912896; // 8.5Mb
//     udpTransport->receiveBufferSize = 8912896; // 8.5Mb

//     /* Use only interface supplied by command line */
//     if (!_PM->get<std::string>("allowInterfaces").empty()) {
//         udpTransport->interfaceWhiteList.clear();
//         udpTransport->interfaceWhiteList.emplace_back(
//                 _PM->get<std::string>("allowInterfaces"));
//     }

//     qos.transport().user_transports.push_back(udpTransport);

//     // Increase the sending buffer size
//     qos.transport().send_socket_buffer_size = 8912896;
//     // Increase the receiving buffer size
//     qos.transport().listen_socket_buffer_size = 8912896;

//     return true;
// }

// bool configure_shmem_transport(
//         PerftestTransport &transport,
//         DomainParticipantQos &qos,
//         ParameterManager *_PM)
// {
//     qos.transport().use_builtin_transports = false;

//     std::shared_ptr<SharedMemTransportDescriptor> shmTransport =
//             std::make_shared<SharedMemTransportDescriptor>();

//     // TODO: We need to calculate the size of the segment, I guess it should be
//     // the size of the sample + X
//     // shmTransport->segment_size(2 * 1024 * 1024);
//     // shmTransport->maxMessageSize(2 * 1024 * 1024);
//     // port_queue_capacity 1024
//     // healthy_check_timeout_ms 250
//     // rtps_dump_file /rtps_dump_file
//     qos.transport().user_transports.push_back(shmTransport);

//     // Increase the sending buffer size
//     qos.transport().send_socket_buffer_size = 1048576;
//     // Increase the receiving buffer size
//     qos.transport().listen_socket_buffer_size = 4194304;

//     return true;
// }

// bool PerftestConfigureTransport(
//         PerftestTransport &transport,
//         DomainParticipantQos &qos,
//         ParameterManager *_PM)
// {
//     if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
//         transport.transportConfig.kind = TRANSPORT_UDPv4;
//         transport.transportConfig.nameString = "UDPv4 (Default)";
//     }

//     switch (transport.transportConfig.kind) {
//     case TRANSPORT_UDPv4:
//         return configure_udpv4_transport(transport, qos, _PM);
//     case TRANSPORT_SHMEM:
//         return configure_shmem_transport(transport, qos, _PM);
//     default:
//         fprintf(stderr,
//                 "%s Transport is not yet supported in Perftest for FastDDS\n",
//                 classLoggingString.c_str());
//         return false;

//     }  // Switch

//     return true;
// }

#endif // PERFTEST_CYCLONEDDS
