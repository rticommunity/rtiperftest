/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifdef RTI_MICRO

#include "Infrastructure_micro.h"

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

unsigned long long PerftestClock::getTimeUsec() {

    if (!OSAPI_System_get_time((OSAPI_NtpTime*)&clockTimeAux)) {
        return 0;
    }

    OSAPI_NtpTime_to_microsec(
            &clockSec,
            &clockUsec,
            (struct OSAPI_NtpTime*)&clockTimeAux);
    return clockUsec + 1000000 * clockSec;
}

void PerftestClock::milliSleep(unsigned int millisec) {
    OSAPI_Thread_sleep(millisec);
}

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

bool configureUDPv4Transport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos &qos)
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
    if (!transport.allowInterfaces.empty()) {
        udp_property->allow_interface.maximum(1);
        udp_property->allow_interface.length(1);
        *udp_property->allow_interface.get_reference(0) =
            DDS_String_dup(transport.allowInterfaces.c_str());
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

#if !RTI_MICRO_24x_COMPATIBILITY
bool configureShmemTransport(
        PerftestTransport &transport,
        DDS_DomainParticipantQos& qos)
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
        DDS_DomainParticipantQos &qos)
{

    switch (transport.transportConfig.kind) {

    case TRANSPORT_DEFAULT:
        /* fprintf(stderr,
                "%s Using default configuration (xml)\n",
                classLoggingString.c_str()); */
        return configureUDPv4Transport(transport, qos);

    case TRANSPORT_UDPv4:
        return configureUDPv4Transport(transport, qos);

#if !RTI_MICRO_24x_COMPATIBILITY
    case TRANSPORT_SHMEM:
        return configureShmemTransport(transport, qos);
#endif

    default:
        fprintf(stderr,
                "%s Transport is not supported in RTI Perftest for Micro\n",
                classLoggingString.c_str());
        return false;

    } // Switch

    return true;
}

bool PerftestCreateThread(
        const char *name,
        MicroThreadOnSpawnedMethod method,
        void *threadParam)
{
    struct OSAPI_ThreadProperty prio = OSAPI_ThreadProperty_INITIALIZER;
    PerftestMicroThreadOnSpawnedMethod *data = new PerftestMicroThreadOnSpawnedMethod();
    data->method = method;
    data->thread_param = threadParam;

    if (!OSAPI_Thread_start(
            OSAPI_Thread_create(
                name,
                &prio,
                perftestMicroThreadRoutine,
                data,
                NULL))) {
        return false;
    }
    return true;
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

#ifdef RTI_SECURE_PERFTEST
bool ConfigureSecurity(
        PerftestSecurity &security,
        DDS_DomainParticipantQos &qos)
{
    printf("Configuring security...\n");

    // configure use of security plugins, based on provided arguments
    RTRegistry *registry = DDSDomainParticipantFactory::get_instance()->get_registry();

    // Properties to register built-in security plugins
    SECCORE_SecurePluginFactoryProperty sec_plugin_prop;

    DDS_Boolean retval;

    // print arguments
    security.printSecurityConfigurationSummary();

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
    if (security.governanceFile.empty()) {
        // choose a pre-built governance file
        std::string file = "file:./resource/secure/signed_PerftestGovernance_";
        if (security.discoveryEncrypted) {
            file += "Discovery";
        }

        if (security.signPackages) {
            file += "Sign";
        }

        if (security.dataEncrypted && security.subMessageEncrypted) {
            file += "EncryptBoth";
        } else if (security.dataEncrypted) {
            file += "EncryptData";
        } else if (security.subMessageEncrypted) {
            file += "EncryptSubmessage";
        }

        file += ".xml";

        fprintf(stdout,
                "\tUsing pre-built governance file: \n\t%s\n",
                file.c_str());

        retval = qos.property.value.assert_property(
                        "dds.sec.access.governance",
                        file.c_str(),
                        false);
    } else {
        retval = qos.property.value.assert_property(
                        "dds.sec.access.governance",
                        security.governanceFile.c_str(),
                        false);
    }
    if (!retval) {
        printf("Failed to add property "
                "dds.sec.access.governance\n");
        return false;
    }

    // permissions file
    if (!qos.property.value.assert_property(
                "dds.sec.access.permissions",
                security.permissionsFile.c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.access.permissions\n");
        return false;
    }

    // permissions authority file
    if (!qos.property.value.assert_property(
                "dds.sec.access.permissions_ca",
                security.certAuthorityFile.c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.access.permissions_ca\n");
        return false;
    }

    // certificate authority
    if (!qos.property.value.assert_property(
                "dds.sec.auth.identity_ca",
                security.certAuthorityFile.c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.auth.identity_ca\n");
        return false;
    }

    // public key
    if (!qos.property.value.assert_property(
                "dds.sec.auth.identity_certificate",
                security.certificateFile.c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.auth.identity_certificate\n");
        return false;
    }

    // private key
    if (!qos.property.value.assert_property(
                "dds.sec.auth.private_key",
                security.privateKeyFile.c_str(),
                false))
    {
        printf("Failed to add property "
                "dds.sec.auth.private_key\n");
        return false;
    }

    if (security.debugLevel != -1) {
        char buf[16];
        sprintf(buf, "%d", security.debugLevel);
        if (!qos.property.value.assert_property(
                    "logging.log_level",
                    buf,
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


#endif // RTI_MICRO
