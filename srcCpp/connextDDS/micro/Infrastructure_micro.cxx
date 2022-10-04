/*
 * (c) 2005-2019  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#if defined(PERFTEST_RTI_MICRO) || defined(RTI_PERF_TSS_MICRO)

#include "Infrastructure_common.h"

#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

/********************************************************************/
/* Perftest Clock class */

PerftestClock::PerftestClock()
{
  #ifndef RTI_PERFTEST_NANO_CLOCK
  #ifndef RTI_WIN32

    OSAPI_NtpTime_from_millisec(&clockTimeAux, 0, 0);
    clockSec = 0;
    clockUsec = 0;

  #else
    _frequency = 0.0;
    LARGE_INTEGER ticks;
    if(!QueryPerformanceFrequency(&ticks)){
        printf("QueryPerformanceFrequency failed!\n");
    }

    _frequency = double(ticks.QuadPart);

  #endif
  #endif
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
  #ifndef RTI_PERFTEST_NANO_CLOCK
    #ifndef RTI_WIN32

    if (!OSAPI_System_get_time((OSAPI_NtpTime*)&clockTimeAux)) {
        return 0;
    }

    OSAPI_NtpTime_to_microsec(
            &clockSec,
            &clockUsec,
            (struct OSAPI_NtpTime*)&clockTimeAux);
    return clockUsec + (unsigned long long) 1000000 * clockSec;

    #else
    /*
     * RTI Connext DDS Micro takes the timestamp by GetSystemTimeAsFileTime,
     * this function should have a resolution of 100 nanoseconds but
     * GetSystemTimeAsFileTime is a non-realtime time & busy loop (very fast)
     * in implementation. The system time is obtained from SharedUserData, which
     * is fill by system on every hardware clock interruption. MICRO-2099
     *
     * More info here:
     * https://docs.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps
     *
     * In order to obtain enough precission for a latencyTest we are going to
     * use the native API QueryPerformanceCounter function measured in
     * microseconds as RTI Connext DDS Pro does.
     */
    LARGE_INTEGER ticks;
    QueryPerformanceCounter(&ticks);
    return ticks.QuadPart / (unsigned long long) (_frequency /1000000.0);

    #endif /* RTI_WIN32 */
  #else
    clock_gettime(CLOCK_MONOTONIC, &timeStruct);
    return (timeStruct.tv_sec * ONE_BILLION) + timeStruct.tv_nsec;
  #endif /* RTI_PERFTEST_NANO_CLOCK */


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

#endif //#ifndef RTI_USE_CPP_11_INFRASTRUCTURE

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
#ifndef RTI_PERF_TSS_MICRO
    UDP_InterfaceFactoryProperty *udp_property =  new UDP_InterfaceFactoryProperty();
#else
    UDP_InterfaceFactoryProperty* udp_property = (struct UDP_InterfaceFactoryProperty *)
            malloc(sizeof(struct UDP_InterfaceFactoryProperty));
    *udp_property = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;
#endif
    udp_property->max_message_size = 65536;
    udp_property->max_receive_buffer_size = 2097152;
    udp_property->max_send_buffer_size = 524288;

    /* Configure UDP transport's allowed interfaces */
    if (!registry->unregister(NETIO_DEFAULT_UDP_NAME, NULL, NULL)) {
        fprintf(stderr, "[Error] Micro failed to unregister udp\n");
        return false;
    }

    /* use only interface supplied by command line? */
    if (!_PM->get<std::string>("allowInterfaces").empty()) {

        if (is_ip_address(_PM->get<std::string>("allowInterfaces"))) {
            fprintf(stderr,
                    "[Error]: Micro does not support providing the allowed interfaces\n"
                    "(-nic/-allowInterfaces) as an ip, provide the nic name instead\n"
                    "(value provided: %s)\n",
                    _PM->get<std::string>("allowInterfaces").c_str());
            return false;
        }
#ifndef RTI_PERF_TSS_MICRO
        udp_property->allow_interface.maximum(1);
        udp_property->allow_interface.length(1);
        *udp_property->allow_interface.get_reference(0) =
            DDS_String_dup(
                _PM->get<std::string>("allowInterfaces").c_str());
#else
        if (!DDS_StringSeq_set_maximum(&udp_property->allow_interface,1))
        {
            printf("failed to set allow_interface maximum\n");
            return false;
        }
        if (!DDS_StringSeq_set_length(&udp_property->allow_interface,1))
        {
            printf("failed to set allow_interface length\n");
            return false;
        }
        *DDS_StringSeq_get_reference(&udp_property->allow_interface,0) =
                DDS_String_dup(_PM->get<std::string>("allowInterfaces").c_str());
#endif
    }

#if defined(RTI_PERF_TSS_MICRO) && FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER

    /* Safety Base or stricter, with Micro, must manually configure available
    * interfaces.
    * First we disable reading out the interface list. Note that on some
    * platforms reading out the interface list has been compiled out, so
    * this property could have no effect.
    */
    udp_property->disable_auto_interface_config = RTI_TRUE;

    REDA_StringSeq_set_maximum(&udp_property->allow_interface,1);
    REDA_StringSeq_set_length(&udp_property->allow_interface,1);

    /* The name of the interface can be the anything, up to
    * UDP_INTERFACE_MAX_IFNAME characters including the '\0' character
    */
    *DDS_StringSeq_get_reference(&udp_property->allow_interface,0) =
    DDS_String_dup("loopback");

    /* This function takes the following arguments:
    * Param 1 is the iftable in the UDP property
    * Param 2 is the IP address of the interface in host order
    * Param 3 is the Netmask of the interface
    * Param 4 is the name of the interface
    * Param 5 are flags. The following flags are supported for Security and
    *    Safety Base (use OR for multiple):
        *        UDP_INTERFACE_INTERFACE_UP_FLAG - Interface is up
        *
        *    The following flags are supported for non-Security and non-Safety
        *    Base:
        *        UDP_INTERFACE_INTERFACE_MULTICAST_FLAG - Interface supports multicast
        */
        RTI_BOOL result;
        result = UDP_InterfaceTable_add_entry(&udp_property->if_table,
        0x7f000001,0xff000000,"loopback",
        UDP_INTERFACE_INTERFACE_UP_FLAG);

        if (!result)
        {
            printf("failed UDP table add entry!\n");
            return DDS_BOOLEAN_FALSE;
    }
#endif

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

    /* In order to avoid MICRO-2191 */
  #if RTIME_DDS_VERSION_MAJOR >= 3 && RTIME_DDS_VERSION_MINOR > 0
    dpde_properties.max_samples_per_remote_builtin_endpoint_writer = 1;
  #endif

// For TSS, this component is already registered
#ifndef RTI_PERF_TSS_MICRO
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
#endif

#ifndef RTI_PERF_TSS_MICRO
    if (!qos.discovery.discovery.name.set_name("dpde")) {
#else
    if (!RT_ComponentFactoryId_set_name(&qos.discovery.discovery.name, "dpde")) {
#endif
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
#ifdef RTI_PERF_TSS_MICRO
    qos.resource_limits.local_publisher_allocation = 3;
    qos.resource_limits.local_subscriber_allocation = 3;
#endif

    transport.minimumMessageSizeMax = udp_property->max_message_size;

    return true;
}

#ifndef PERFTEST_RTI_MICRO_24x_COMPATIBILITY
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

    /* In order to avoid MICRO-2191 */
  #if RTIME_DDS_VERSION_MAJOR >= 3 && RTIME_DDS_VERSION_MINOR > 0
    dpde_properties.max_samples_per_remote_builtin_endpoint_writer = 1;
  #endif

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

    transport.minimumMessageSizeMax = shmem_property.message_size_max;

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

  #if !PERFTEST_RTI_MICRO_24x_COMPATIBILITY
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
    // configure use of security plugins, based on provided arguments
    RTRegistry *registry =
            DDSDomainParticipantFactory::get_instance()->get_registry();

    // Properties to register built-in security plugins
    SECCORE_SecurePluginFactoryProperty sec_plugin_prop;

    DDS_Boolean retval;
    std::string governanceFilePath;

    // register plugin factory with registry
    if (!SECCORE_SecurePluginFactory::register_suite(
                    registry,SECCORE_DEFAULT_SUITE_NAME,sec_plugin_prop))
    {
        printf("Failed to register security plugins factory\n");
        return false;
    }

    // Enable security in DomainParticipantQos
    if (!qos.trust.suite.set_name(SECCORE_DEFAULT_SUITE_NAME))
    {
        printf("Failed to set security suite name: %s\n",
               SECCORE_DEFAULT_SUITE_NAME);
        return false;
    }

    // check if governance file provided
    if (_PM->get<std::string>("secureGovernanceFile").empty()) {
        // choose a pre-built governance file
        governanceFilePath = "file:./resource/secure/signed_PerftestGovernance_";
        if (_PM->get<bool>("secureEncryptDiscovery")) {
            governanceFilePath += "Discovery";
        }
        if (_PM->get<bool>("secureSign")) {
            governanceFilePath += "Sign";
        }
        if (_PM->get<bool>("secureEncryptData")
                && _PM->get<bool>("secureEncryptSM")) {
            governanceFilePath += "EncryptBoth";
        } else if (_PM->get<bool>("secureEncryptData")) {
            governanceFilePath += "EncryptData";
        } else if (_PM->get<bool>("secureEncryptSM")) {
            governanceFilePath += "EncryptSubmessage";
        }

        governanceFilePath += ".xml";

        retval = qos.property.value.assert_property(
                        "dds.sec.access.governance",
                        governanceFilePath.c_str(),
                        false);
    } else {
        governanceFilePath = _PM->get<std::string>("secureGovernanceFile");
        retval = qos.property.value.assert_property(
                        "dds.sec.access.governance",
                        governanceFilePath.c_str(),
                        false);
    }
    if (!retval) {
        printf("Failed to add property "
                "dds.sec.access.governance\n");
        return false;
    }

    /*
     * Save the local variable governanceFilePath into
     * the parameter "secureGovernanceFile"
     */
    _PM->set("secureGovernanceFile", governanceFilePath);

    // permissions file
    if (!qos.property.value.assert_property(
                "dds.sec.access.permissions",
                _PM->get<std::string>("securePermissionsFile").c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.access.permissions\n");
        return false;
    }

    // permissions authority file
    if (!qos.property.value.assert_property(
                "dds.sec.access.permissions_ca",
                _PM->get<std::string>("secureCertAuthority").c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.access.permissions_ca\n");
        return false;
    }

    // certificate authority
    if (!qos.property.value.assert_property(
                "dds.sec.auth.identity_ca",
                _PM->get<std::string>("secureCertAuthority").c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.auth.identity_ca\n");
        return false;
    }

    // public key
    if (!qos.property.value.assert_property(
                "dds.sec.auth.identity_certificate",
                _PM->get<std::string>("secureCertFile").c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.auth.identity_certificate\n");
        return false;
    }

    // private key
    if (!qos.property.value.assert_property(
                "dds.sec.auth.private_key",
                _PM->get<std::string>("securePrivateKey").c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.auth.private_key\n");
        return false;
    }

    if (_PM->is_set("secureDebug")) {
        if (!qos.property.value.assert_property(
                    "logging.log_level",
                    perftest::to_string(_PM->get<int>("secureDebug")).c_str(),
                    false))
        {
            printf("Failed to add property "
                    "logging.log_level\n");
            return false;
        }
    }
    return true;
}
#endif

/********************************************************************/
/* Security Related Functions */



#endif // PERFTEST_RTI_MICRO
