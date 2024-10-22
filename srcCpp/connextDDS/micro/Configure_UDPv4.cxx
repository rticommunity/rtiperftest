#include <stdbool.h>

#include "rti_me_c.h"

#include "PerftestTransport.h"
#include "ParameterManager.h"

bool is_ip_address(std::string ip_string);

bool configureUDPv4Transport(
        PerftestTransport& transport,
        struct DDS_DomainParticipantQos& qos,
        ParameterManager *_PM)
{

    RTRegistry *registry = DDSDomainParticipantFactory::get_instance()->get_registry();
    DPDE_DiscoveryPluginProperty dpde_properties;

    UDP_InterfaceFactoryProperty* udp_property = (struct UDP_InterfaceFactoryProperty *)
            malloc(sizeof(struct UDP_InterfaceFactoryProperty));
    *udp_property = UDP_INTERFACE_FACTORY_PROPERTY_DEFAULT;

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
    }

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

    if (!registry->register_component(
                NETIO_DEFAULT_UDP_NAME,
                UDP_InterfaceFactory_get_interface(),
                &udp_property->_parent._parent,
                NULL)) {
        printf("Micro: Failed to register udp\n");
        if (udp_property != NULL) {
            free(udp_property);
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
            free(udp_property);
        }
        return false;
    }
#endif

    if (!RT_ComponentFactoryId_set_name(&qos.discovery.discovery.name, "dpde")) {
        printf("Micro: Failed to set discovery plugin name\n");
        if (udp_property != NULL) {
            free(udp_property);
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

/*
 * Function to check if a given string is an ip (it does not check for ranges)
 */
bool is_ip_address(std::string ip_string)
{
    int octect1, octect2, octect3, octect4;
    if (sscanf(ip_string.c_str(),
               "%d.%d.%d.%d",
               &octect1,
               &octect2,
               &octect3,
               &octect4)
        != 4) {
        return false;
    }
    return true;
}
