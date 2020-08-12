/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __CYCLONEDDSIMPL_H__
#define __CYCLONEDDSIMPL_H__

#ifdef PERFTEST_CYCLONEDDS

#include "Infrastructure_common.h"
#include "MessagingIF.h"
#include "PerftestTransport.h"
#include <algorithm>
#include <map>
#include <stdexcept>  // This header is part of the error handling library.
#include <string>

#include "perftest.h"
#include "dds/dds.h"
#include "dds/ddsc/dds_public_impl.h"
#include "dds/ddsc/dds_public_qosdefs.h"
#include "dds/ddsi/ddsi_xqos.h"
#include "dds/ddsrt/time.h"

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

template <typename T>
class CycloneDDSImpl : public IMessaging
{

public:

    CycloneDDSImpl(dds_topic_descriptor_t topicDescriptor);

    ~CycloneDDSImpl()
    {
        Shutdown();
    }

    const std::string get_middleware_version_string();

    void configure_middleware_verbosity(int verbosityLevel);

    bool validate_input();

    std::string PrintConfiguration();

    bool Initialize(ParameterManager &PM, perftest_cpp *parent);

    void Shutdown();

    unsigned long GetInitializationSampleCount();

    IMessagingWriter *CreateWriter(const char *topic_name);

    /*
     * Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
     * to get data
     */
    IMessagingReader *CreateReader(const char *topic_name, IMessagingCB *callback);

    bool supports_listener()
    {
        return true;
    };

    bool supports_discovery()
    {
        return true;
    };

    bool configure_participant_qos(dds_qos_t *qos);
    bool configure_writer_qos(dds_qos_t *qos, std::string qosProfile);
    bool configure_reader_qos(dds_qos_t *qos, std::string qosProfile);

    const std::string get_qos_profile_name(const char *topicName);

protected:

    // Perftest related entities
    perftest_cpp *_parent;
    ParameterManager *_PM;
    dds_topic_descriptor_t _topicDescriptor;

    std::map<std::string, std::string> _qoSProfileNameMap;

    PerftestTransport _transport;
    PerftestSemaphore *_pongSemaphore;

    // DDS Entities
    dds_entity_t _participant;
    dds_entity_t _publisher;
    dds_entity_t _subscriber;

};

#endif // PERFTEST_CYCLONEDDS
#endif // __CYCLONEDDSIMPL_H__
