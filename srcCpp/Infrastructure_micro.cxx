/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef RTI_MICRO

#include "Infrastructure_micro.h"

/********************************************************************/
/* Perftest Clock class */

PerftestClock::PerftestClock()
{
    OSAPI_NtpTime_from_millisec(&clockTimeAux, 0, 0);
    clockSec = 0;
    clockUsec = 0;
}

PerftestClock::~PerftestClock()
{
}

PerftestClock &PerftestClock::getInstance()
{
    static PerftestClock instance;
    return instance;
}

unsigned long long PerftestClock::getTimeUsec()
{

    if (!OSAPI_System_get_time((OSAPI_NtpTime*)&clockTimeAux)) {
        return 0;
    }

    OSAPI_NtpTime_to_microsec(
            &clockSec,
            &clockUsec,
            (struct OSAPI_NtpTime*)&clockTimeAux);
    return clockUsec + 1000000 * clockSec;
}

void PerftestClock::milliSleep(unsigned int millisec)
{
    OSAPI_Thread_sleep(millisec);
}

/********************************************************************/
/* Get Connext Micro functions */

const std::string GetDDSVersionString()
{
    return "RTI Connext DDS Micro "
            + std::to_string(RTIME_DDS_VERSION_MAJOR) + "."
            + std::to_string(RTIME_DDS_VERSION_MINOR) + "."
            + std::to_string(RTIME_DDS_VERSION_REVISION);
}

void PerftestConfigureVerbosity(int verbosityLevel)
{

    OSAPI_LogVerbosity_T verbosity = OSAPI_LOG_VERBOSITY_ERROR;
    switch (verbosityLevel) {
        case 0: verbosity = OSAPI_LOG_VERBOSITY_SILENT;
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
        case 1: verbosity = OSAPI_LOG_VERBOSITY_ERROR;
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
        case 2: verbosity = OSAPI_LOG_VERBOSITY_WARNING;
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
        case 3: verbosity = OSAPI_LOG_VERBOSITY_DEBUG;
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
        default: fprintf(stderr,
                    "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                break;
    }
    OSAPI_Log_set_verbosity(verbosity);
}

/********************************************************************/
/* Micro OnSpawed Method */

struct PerftestMicroThreadOnSpawnedMethod
{
    MicroThreadOnSpawnedMethod method;
    void *thread_param;

};

static RTI_BOOL perftestMicroThreadRoutine(struct OSAPI_ThreadInfo *thread_info)
{
    PerftestMicroThreadOnSpawnedMethod *data = (PerftestMicroThreadOnSpawnedMethod *)thread_info->user_data;

    data->method(data->thread_param);

    delete(data);

    thread_info->stop_thread = RTI_TRUE;
    return RTI_TRUE;
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
    //TODO Francis: Support priorities for threads in Micro
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

/********************************************************************/
/* Transport related functions */

bool configureUDPv4Transport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM)
{

    RTRegistry *registry = DDSDomainParticipantFactory::get_instance()->get_registry();
    DPDE_DiscoveryPluginProperty dpde_properties;

    /*
     * We will use the same values we use for Pro (XML) for the buffers
     * these values can be reduced for a smaller memory footprint.
     */
    UDP_InterfaceFactoryProperty *udp_property =  new UDP_InterfaceFactoryProperty();
    udp_property->max_message_size = 65536;
    udp_property->max_receive_buffer_size = 2097152;
    udp_property->max_send_buffer_size = 524288;

    /* Configure UDP transport's allowed interfaces */
    if (!registry->unregister(NETIO_DEFAULT_UDP_NAME, NULL, NULL)) {
        printf("Micro: Failed to unregister udp\n");
        return false;
    }

    /* use only interface supplied by command line? */
    if (!_PM->get<std::string>("allowInterfaces").empty()) {
        udp_property->allow_interface.maximum(1);
        udp_property->allow_interface.length(1);
        *udp_property->allow_interface.get_reference(0) =
            DDS_String_dup(
                _PM->get<std::string>("allowInterfaces").c_str());
    }

    if (!registry->register_component(
                NETIO_DEFAULT_UDP_NAME,
                UDPInterfaceFactory::get_interface(),
                &udp_property->_parent._parent,
                NULL)) {
        printf("Micro: Failed to register udp\n");
        if (udp_property != NULL) {
            delete udp_property;
        }
        return false;
    }

    if (!registry->register_component("dpde",
                DPDEDiscoveryFactory::get_interface(),
                &dpde_properties._parent,
                NULL)) {
        printf("Micro: Failed to register dpde\n");
        if (udp_property != NULL) {
            delete udp_property;
        }
        return false;
    }

    if (!qos.discovery.discovery.name.set_name("dpde")) {
        printf("Micro: Failed to set discovery plugin name\n");
        if (udp_property != NULL) {
            delete udp_property;
        }
        return false;
    }

    if (_PM->get<bool>("multicast")) {
        DDS_StringSeq_set_maximum(&qos.user_traffic.enabled_transports, 1);
        DDS_StringSeq_set_length(&qos.user_traffic.enabled_transports, 1);
        *DDS_StringSeq_get_reference(&qos.user_traffic.enabled_transports, 0) = 
                DDS_String_dup("_udp://239.255.0.1");
    } 

    /* if there are more remote or local endpoints, you may need to increase these limits */
    qos.resource_limits.max_destination_ports = 32;
    qos.resource_limits.max_receive_ports = 32;
    qos.resource_limits.local_topic_allocation = 3;
    qos.resource_limits.local_type_allocation = 1;
    qos.resource_limits.local_reader_allocation = 2;
    qos.resource_limits.local_writer_allocation = 2;
    qos.resource_limits.remote_participant_allocation = 8;
    qos.resource_limits.remote_reader_allocation = 8;
    qos.resource_limits.remote_writer_allocation = 8;

    return true;
}

#ifndef RTI_MICRO_24x_COMPATIBILITY
bool configureShmemTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
{
    RTRegistry *registry = DDSDomainParticipantFactory::get_instance()->get_registry();

    DPDE_DiscoveryPluginProperty dpde_properties;
    NETIO_SHMEMInterfaceFactoryProperty shmem_property;
    
    /* We don't need UDP transport. Unregister it */
    if (!registry->unregister(NETIO_DEFAULT_UDP_NAME, NULL, NULL)) {
        printf("Micro: Failed to unregister udp\n");
        return false;
    }

    if (!registry->register_component("dpde",
                DPDEDiscoveryFactory::get_interface(),
                &dpde_properties._parent,
                NULL)) {
        printf("Micro: Failed to register dpde\n");
        return false;
    }

    if (!qos.discovery.discovery.name.set_name("dpde")) {
        printf("Micro: Failed to set discovery plugin name\n");
        return false;
    }

    if (!registry->register_component(
            NETIO_DEFAULT_SHMEM_NAME,
            SHMEMInterfaceFactory::get_interface(),
            (struct RT_ComponentFactoryProperty*) &shmem_property,
            NULL))
    {
        printf("failed to register shmem\n");
        return false;
    }

    qos.transports.enabled_transports.maximum(1);
    qos.transports.enabled_transports.length(1);
    qos.transports.enabled_transports[0] = DDS_String_dup("_shmem");

    qos.discovery.enabled_transports.maximum(1);
    qos.discovery.enabled_transports.length(1);
    qos.discovery.enabled_transports[0] = DDS_String_dup("_shmem://");

    qos.user_traffic.enabled_transports.maximum(1);
    qos.user_traffic.enabled_transports.length(1);
    qos.user_traffic.enabled_transports[0] = DDS_String_dup("_shmem://");

    qos.discovery.initial_peers.maximum(1);
    qos.discovery.initial_peers.length(1);
    qos.discovery.initial_peers[0] = DDS_String_dup("_shmem://");

    /* if there are more remote or local endpoints, you may need to increase these limits */
    qos.resource_limits.max_destination_ports = 32;
    qos.resource_limits.max_receive_ports = 32;
    qos.resource_limits.local_topic_allocation = 3;
    qos.resource_limits.local_type_allocation = 1;
    qos.resource_limits.local_reader_allocation = 2;
    qos.resource_limits.local_writer_allocation = 2;
    qos.resource_limits.remote_participant_allocation = 8;
    qos.resource_limits.remote_reader_allocation = 8;
    qos.resource_limits.remote_writer_allocation = 8;

    return true;
}
#endif

bool PerftestConfigureTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM)
{

    if (transport.transportConfig.kind == TRANSPORT_NOT_SET) {
        transport.transportConfig.kind = TRANSPORT_UDPv4;
        transport.transportConfig.nameString = "UDPv4 (Default)";
    }

    switch (transport.transportConfig.kind) {

    case TRANSPORT_UDPv4:
        return configureUDPv4Transport(transport, qos, _PM);

  #if !RTI_MICRO_24x_COMPATIBILITY
    case TRANSPORT_SHMEM:
        return configureShmemTransport(transport, qos, _PM);
  #endif

    default:
        fprintf(stderr,
                "%s Transport is not supported in RTI Perftest for Micro\n",
                classLoggingString.c_str());
        return false;

    } // Switch

    return true;
}

#ifdef RTI_SECURE_PERFTEST
bool PerftestConfigureSecurity(
        PerftestSecurity &security,
        DDS_DomainParticipantQos &qos,
        ParameterManager *_PM)
{
    return true;
}
#endif

/********************************************************************/
/* Security Related Functions */



#endif // RTI_MICRO
