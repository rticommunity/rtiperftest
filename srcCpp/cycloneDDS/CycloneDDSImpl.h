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

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

template <typename T>
class CycloneDDSImpl : public IMessaging
{

public:

    CycloneDDSImpl();

    ~CycloneDDSImpl()
    {
        Shutdown();
    }

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

    // bool configure_participant_qos(DomainParticipantQos &qos);
    // bool configure_writer_qos(/* DataWriterQos &qos, */std::string qosProfile);
    // bool configure_reader_qos(/* DataReaderQos &qos, */std::string qosProfile);

    const std::string get_qos_profile_name(const char *topicName);

protected:

    // Perftest related entities
    perftest_cpp *_parent;
    ParameterManager *_PM;

    std::map<std::string, std::string> _qoSProfileNameMap;

/*
    PerftestTransport _transport;
    PerftestSemaphore *_pongSemaphore;
    
    // DDS Entities
    eprosima::fastdds::dds::DomainParticipantFactory *_factory;
    eprosima::fastdds::dds::DomainParticipant *_participant;
    eprosima::fastdds::dds::Publisher *_publisher;
    eprosima::fastdds::dds::Subscriber *_subscriber;
    eprosima::fastdds::dds::TypeSupport _type; //This is a std::shared_ptr

*/
};

#endif // PERFTEST_CYCLONEDDS
#endif // __CYCLONEDDSIMPL_H__
