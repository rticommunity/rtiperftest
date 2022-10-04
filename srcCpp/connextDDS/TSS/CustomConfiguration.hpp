
/*******************************************************************************
 *  Copyright (c) 2018-2021 Real-Time Innovations, Inc.  All rights reserved.
 *
 *  No duplications, whole or partial, manual or electronic, may be made
 *  without express written permission.  Any such copies, or revisions thereof,
 *  must display this notice unaltered.
 *  This code contains trade secrets of Real-Time Innovations, Inc.
 *******************************************************************************/

#ifndef CUSTOM_CONFIGURATION_HPP
#define CUSTOM_CONFIGURATION_HPP

#include <vector>

#include "gen/perftest_TSSConfigInterface.hpp"

#include "Infrastructure_common.h"
#include "ParameterManager.h"
#include "gen/perftest.h"

/** CustomConfiguration is an extension of the generated Configuration.
 * This allows customization of the configuration without creating an entirely
 * new config.
 * */
class CustomConfiguration : public RTI::perftest::Configuration
{
  public:
    CustomConfiguration()
    {}

    CustomConfiguration(const char* type_name)
    {
        _type_name = type_name;
    }

    void Initialize(
            const FACE::Configuration::INITIALIZATION_TYPE
                    &initialization_information,
            FACE::RETURN_CODE_TYPE::Value &return_code)
    {
        UNUSED_ARG(initialization_information);

        if (_initialized)
        {
            return_code = FACE::RETURN_CODE_TYPE::NO_ERROR;
            return;
        }

        if (!_populate_type_support_config())
        {
            return_code = FACE::RETURN_CODE_TYPE::INVALID_CONFIG;
            return;
        }

        if (!_populate_system_config() ||
            !_populate_custom_connection_config() ||
            !_populate_custom_domain_config())
        {
            return_code = FACE::RETURN_CODE_TYPE::INVALID_CONFIG;
            return;
        }

        _initialized = DDS_BOOLEAN_TRUE;
        return_code = FACE::RETURN_CODE_TYPE::NO_ERROR;
    }

    void Read(FACE::Configuration::HANDLE_TYPE handle,
            const FACE::Configuration::SET_NAME_TYPE &set_name,
            FACE::SYSTEM_ADDRESS_TYPE buffer,
            FACE::Configuration::BUFFER_SIZE_TYPE buffer_size,
            FACE::Configuration::BUFFER_SIZE_TYPE &bytes_read,
            FACE::RETURN_CODE_TYPE::Value &return_code)
    {
        const char *set_name_ptr = NULL;

        UNUSED_ARG(buffer_size);

        if (buffer == NULL)
        {
            return_code = FACE::RETURN_CODE_TYPE::INVALID_PARAM;
            return;
        }

        if (!_initialized)
        {
            return_code = FACE::RETURN_CODE_TYPE::NOT_AVAILABLE;
            return;
        }

        #ifdef FACE_SEQUENCE_AND_STRING_IMPLEMENTED
        set_name_ptr = set_name.buffer();
        #else
        set_name_ptr = set_name;
        #endif

        if (handle == system_configurations_handle)
        {
            RTI_TSS_System_Configuration_T **sys_config =
            (RTI_TSS_System_Configuration_T**) buffer;

            if (!_get_system_config(sys_config, set_name_ptr))
            {
                return_code = FACE::RETURN_CODE_TYPE::INVALID_CONFIG;
                return;
            }

            bytes_read = sizeof(void*);
        }
        else if (handle == domain_configurations_handle)
        {
            RTI_TSS_DDS_DomainParticipant_Configuration_T **domain_config =
            (RTI_TSS_DDS_DomainParticipant_Configuration_T**) buffer;

            if (!_get_domain_config(domain_config, set_name_ptr))
            {
                return_code = FACE::RETURN_CODE_TYPE::INVALID_CONFIG;
                return;
            }

            bytes_read = sizeof(void*);
        }
        else if (handle == type_support_configurations_handle)
        {
            RTI_TSS_TypeSupport_Configuration_T **type_config =
            (RTI_TSS_TypeSupport_Configuration_T**) buffer;

            if (!_get_type_config(type_config, set_name_ptr))
            {
                return_code = FACE::RETURN_CODE_TYPE::INVALID_CONFIG;
                return;
            }

            bytes_read = sizeof(void*);
        }
        else if (handle == connection_configurations_handle)
        {
            RTI_TSS_Connection_Configuration_T **connection_config =
            (RTI_TSS_Connection_Configuration_T**) buffer;

            if (!_custom_get_connection_config(connection_config, set_name_ptr))
            {
                return_code = FACE::RETURN_CODE_TYPE::INVALID_CONFIG;
                return;
            }

            bytes_read = sizeof(void*);
        }
        else
        {
            return_code = FACE::RETURN_CODE_TYPE::INVALID_PARAM;
            return;
        }

        return_code = FACE::RETURN_CODE_TYPE::NO_ERROR;
    }

  private:

    FACE::Boolean _populate_custom_system_config()
    {
        if (!RTI_TSS_System_Configuration_initialize(&_system_configurations[0]))
        {
            return false;
        }
        _system_configurations[0].config_name = "perftest";
        _system_configurations[0].configure_domain_participant_factory_qos_fn = FACE_DM_TestData_t_participant_factory_qos;

#ifdef RTI_PERF_TSS_MICRO
        _system_configurations[0].max_connections = 32;
        _system_configurations[0].max_topics = _connection_configurations_length / 2;

        _system_configurations[0].components_length = 4;
        int i = 3;

        _system_configurations[0].components =
        new RTI_TSS_Micro_Component_T[_system_configurations[0].components_length];

        // Writer History Component
        if (!RTI_TSS_Micro_Component_initialize(&_system_configurations[0].components[0]))
        {
            return false;
        }
        _system_configurations[0].components[0].name = DDSHST_WRITER_DEFAULT_HISTORY_NAME;
        _system_configurations[0].components[0].component = WHSM_HistoryFactory_get_interface();
        _system_configurations[0].components[0].listener = NULL;
        _system_configurations[0].components[0].property = NULL;

        // Reader History Component
        if (!RTI_TSS_Micro_Component_initialize(&_system_configurations[0].components[1]))
        {
            return false;
        }
        _system_configurations[0].components[1].name = DDSHST_READER_DEFAULT_HISTORY_NAME;
        _system_configurations[0].components[1].component = RHSM_HistoryFactory_get_interface();
        _system_configurations[0].components[1].listener = NULL;
        _system_configurations[0].components[1].property = NULL;

        #if !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER
        // DPDE Component
        struct DPDE_DiscoveryPluginProperty *dpde_properties =
        (struct DPDE_DiscoveryPluginProperty*) malloc(
            sizeof(struct DPDE_DiscoveryPluginProperty));

        if (DPDE_DiscoveryPluginProperty_initialize(dpde_properties) != DDS_RETCODE_OK)
        {
            printf("Failed to Initialize DPDE properties");
            return false;
        }

        if (!RTI_TSS_Micro_Component_initialize(&_system_configurations[0].components[i]))
        {
            return false;
        }
        _system_configurations[0].components[i].name = "dpde";
        _system_configurations[0].components[i].component = DPDE_DiscoveryFactory_get_interface();
        _system_configurations[0].components[i].listener = NULL;
        _system_configurations[0].components[i].property =
        (struct RT_ComponentFactoryProperty*) dpde_properties;
        i++;
#endif /* !FACE_COMPLIANCE_LEVEL_SAFETY_BASE_OR_STRICTER */

#if !EXCLUDE_UDP

        // UDP Component
        if (!RTI_TSS_Micro_Component_initialize(&_system_configurations[0].components[i]))
        {
            return false;
        }
        _system_configurations[0].components[i].name = NETIO_DEFAULT_UDP_NAME;
        _system_configurations[0].components[i].component = UDP_InterfaceFactory_get_interface();
        _system_configurations[0].components[i].listener = NULL;
        _system_configurations[0].components[i].property =
        (struct RT_ComponentFactoryProperty*) _new_udp_property();
#endif /* !EXCLUDE_UDP */
#endif /* RTI_CONNEXT_MICRO */

        return true;
    }

    FACE::Boolean _custom_get_connection_config(
            RTI_TSS_Connection_Configuration_T **cfg,
            const char *cfg_name)
    {
        if ((cfg == NULL) ||
        (*cfg != NULL) ||
        (cfg_name == NULL))
        {
            return false;
        }

        for (FACE::UnsignedLong i = 0; i < _connection_configs.size(); i++)
        {
            if (_strcmp_uppercase(_connection_configs[i].config_name, cfg_name))
            {
                *cfg = &_connection_configs[i];
                return true;
            }
        }

        return false;
    }

    FACE::Boolean _populate_custom_domain_config()
    {
        if (!RTI_TSS_DDS_DomainParticipant_Configuration_initialize(&_domain_configurations[0]))
        {
            return false;
        }
        _domain_configurations[0].config_name = "domain_0";
        _domain_configurations[0].domain_id = 0;
        _domain_configurations[0].configure_domain_participant_qos_fn = NULL;
        _domain_configurations[0].user_data = NULL;
        #ifdef RTI_CONNEXT_PRO
        // The binding with QoS library and profile will happen later on, so
        // no need to assign any QoS function here
        _domain_configurations[0].qos_library = NULL;
        _domain_configurations[0].qos_profile = NULL;
        #endif

        return true;
    }

    RTI_TSS_Connection_Configuration_T GenerateConnectionConfiguration(
            const char* config_name,
            const char* domain_config_name,
            FACE_CONNECTION_DIRECTION_TYPE direction,
            const char* topic_name,
            const char* type_name,
            FACE::Boolean &success)
    {
        success = DDS_BOOLEAN_TRUE;

        RTI_TSS_Connection_Configuration_T connection_configuration;
        if (!RTI_TSS_Connection_Configuration_initialize(&connection_configuration))
        {
            success = DDS_BOOLEAN_FALSE;
            return connection_configuration;
        }
        connection_configuration.config_name = DDS_String_dup(config_name);
        connection_configuration.domain_config_name = domain_config_name;
        connection_configuration.direction = direction;
        connection_configuration.topic_name = topic_name;
        connection_configuration.type_name = type_name;
        // The binding with QoS functions will happen later on, so no need to
        // assign any QoS function here
        connection_configuration.configure_publisher_qos_fn = NULL;
        connection_configuration.configure_topic_qos_fn = NULL;
        connection_configuration.configure_datawriter_qos_fn = NULL;
        connection_configuration.configure_subscriber_qos_fn = NULL;
        connection_configuration.configure_datareader_qos_fn = NULL;
        connection_configuration.user_data = NULL;
#ifndef RTI_CONNEXT_MICRO
        connection_configuration.bi_dir_ignore_self = DDS_BOOLEAN_TRUE;
        // The binding with QoS library and profile will happen later on, so
        // no need to assign any QoS function here
        connection_configuration.qos_library = NULL;
        connection_configuration.qos_profile = NULL;
#endif

        return connection_configuration;
    }

    FACE::Boolean _populate_custom_connection_config()
    {
        char* conf_name = (char*)malloc(32);

        if (!conf_name)
        {
            return DDS_BOOLEAN_FALSE;
        }

        FACE::Boolean success = true;

        sprintf(conf_name, "writer ");
        strcat(conf_name, ANNOUNCEMENT_TOPIC_NAME);

        /***** Announcement *****/
        _connection_configs.push_back(GenerateConnectionConfiguration(
                                         conf_name,
                                         "domain_0",
                                         FACE_SOURCE,
                                         ANNOUNCEMENT_TOPIC_NAME,
                                         _type_name,
                                         success));

        if (!success)
        {
            free(conf_name);
            return DDS_BOOLEAN_FALSE;
        }

        sprintf(conf_name, "reader ");
        strcat(conf_name, ANNOUNCEMENT_TOPIC_NAME);
        _connection_configs.push_back(GenerateConnectionConfiguration(
                                         conf_name,
                                         "domain_0",
                                         FACE_DESTINATION,
                                         ANNOUNCEMENT_TOPIC_NAME,
                                         _type_name,
                                         success));

        if (!success)
        {
            free(conf_name);
            return DDS_BOOLEAN_FALSE;
        }

        sprintf(conf_name, "writer ");
        strcat(conf_name, LATENCY_TOPIC_NAME);
        /***** Latency *****/
        _connection_configs.push_back(GenerateConnectionConfiguration(
                                         conf_name,
                                         "domain_0",
                                         FACE_SOURCE,
                                         LATENCY_TOPIC_NAME,
                                         _type_name,
                                         success));

        if (!success)
        {
            free(conf_name);
            return DDS_BOOLEAN_FALSE;
        }

        sprintf(conf_name, "reader ");
        strcat(conf_name, LATENCY_TOPIC_NAME);
        _connection_configs.push_back(GenerateConnectionConfiguration(
                                         conf_name,
                                         "domain_0",
                                         FACE_DESTINATION,
                                         LATENCY_TOPIC_NAME,
                                         _type_name,
                                         success));

        if (!success)
        {
            free(conf_name);
            return DDS_BOOLEAN_FALSE;
        }

        sprintf(conf_name, "writer ");
        strcat(conf_name, THROUGHPUT_TOPIC_NAME);
        /*****Throughput *****/
        _connection_configs.push_back(GenerateConnectionConfiguration(
                                         conf_name,
                                         "domain_0",
                                         FACE_SOURCE,
                                         THROUGHPUT_TOPIC_NAME,
                                         _type_name,
                                         success));

        if (!success)
        {
            free(conf_name);
            return DDS_BOOLEAN_FALSE;
        }

        sprintf(conf_name, "reader ");
        strcat(conf_name, THROUGHPUT_TOPIC_NAME);
        _connection_configs.push_back(GenerateConnectionConfiguration(
                                         conf_name,
                                         "domain_0",
                                         FACE_DESTINATION,
                                         THROUGHPUT_TOPIC_NAME,
                                         _type_name,
                                         success));

        if (!success)
        {
            free(conf_name);
            return DDS_BOOLEAN_FALSE;
        }

        return success;
    }

    const char* _type_name;
    std::vector<RTI_TSS_Connection_Configuration_T> _connection_configs;
};

#endif // CUSTOM_CONFIGURATION_HPP
