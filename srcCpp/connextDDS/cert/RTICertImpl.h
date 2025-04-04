/*
 * (c) 2023-2024  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __RTICERTIMPL_H__
#define __RTICERTIMPL_H__

#ifdef PERFTEST_CERT

#include "Infrastructure_common.h"
#include "MessagingIF.h"
#include "PerftestTransport.h"
#include <algorithm>
#include <map>
#include <stdexcept>  // This header is part of the error handling library.
#include <string>

#include "disc_dpse/disc_dpse_dpsediscovery.h"
#include "wh_sm/wh_sm_history.h"
#include "rh_sm/rh_sm_history.h"
#include "rti_me_c.h"

#ifdef RTI_CERT_IS_PI
#include "rti_me_psl/netio/netio_psl_udp.h"
#else
#include "netio/netio_udp.h"
#endif

#include "dds_c/dds_c_sequence.h"

// Import here the generated code and anything you need to use for Cert.
#include "perftest.h"
#include "perftestPlugin.h"
#include "perftestSupport.h"
#ifdef RTI_ZEROCOPY_AVAILABLE
#include "perftest_cert_zc.h"
#include "perftest_cert_zcPlugin.h"
#include "perftest_cert_zcSupport.h"
#include "netio_zcopy/netio_zcopy.h"
#ifdef RTI_CERT_IS_PI
#include "rti_me_psl/netio/netio_mynotif.h"
#else
#include "netio_zcopy/posixNotifMechanism.h"
#endif
#include "netio_zcopy/netio_zcopy_publication.h"
#endif

/* Maximum number of peer possible. -peer is used to provide peer addresses */
#define RTIPERFTEST_MAX_PEERS 1024
/* Maximum number of samples to optimize the performance result */
#define PERF_CERT_MAX_SAMPLES 5000
#define PARTICIPANT_NAME_PX  "Perftest_cert_publisher"
#define PARTICIPANT_NAME_SX  "Perftest_cert_subscriber"
#define PARTICIPANT_ID_PX    (1)
#define PARTICIPANT_ID_SX    (2)

#define DISC_FACTORY_DEFAULT_NAME "dpse"
#define UDP_TRANSPORT_NAME        "_udp"
#define UDP_TRANSPORT_LOCATOR     "_udp://"

#ifdef RTI_ZEROCOPY_AVAILABLE
    #define NOTIF_TRANSPORT_NAME      "notif"
    #define NOTIF_TRANSPORT_LOCATOR   "notif://"
#endif

/* Forward declaration of perftest_cpp to avoid circular dependencies */
class perftest_cpp;

const std::string GetMiddlewareVersionString();

template <typename T, typename TSeq>
class RTICertImplBase : public IMessaging
{

public:

    RTICertImplBase(const char *type_name, NDDS_Type_Plugin *plugin);

    ~RTICertImplBase()
    {
        shutdown();
    }

    const std::string get_middleware_version_string();

    bool validate_input();

    std::string print_configuration();

    bool initialize(ParameterManager &PM, perftest_cpp *parent);

    virtual void shutdown();

    unsigned long get_initial_burst_size();

    IMessagingWriter *create_writer(const char *topic_name);

    /*
     * Pass null for callback if using IMessagingSubscriber.receive_message()
     * to get data
     */
    IMessagingReader *create_reader(const char *topic_name, IMessagingCB *callback);

    bool supports_listeners()
    {
        return true;
    };

    virtual bool configure_participant_qos(DDS_DomainParticipantQos &qos);
    bool configure_writer_qos(DDS_DataWriterQos &dw_qos, std::string qos_profile, std::string topic_name);
    bool configure_reader_qos(DDS_DataReaderQos &dr_qos, std::string qos_profile, std::string topic_name);

    const std::string get_qos_profile_name(const char *topicName);

protected:
    virtual IMessagingWriter *create_CertPublisher(DDS_DataWriter *writer);
    virtual IMessagingReader *create_CertSubscriber(DDS_DataReader *reader,
        bool isCallbackNull);
    virtual void *create_CertReaderListener(
        IMessagingCB *callback);

    virtual bool create_domain_participant(
        DDS_DomainParticipantQos *dpQos);

    // Perftest related entities
    perftest_cpp *_parent;
    ParameterManager *_PM;
    std::map<std::string, std::string> _qoSProfileNameMap;

    PerftestTransport _transport;
    PerftestSemaphore *_pongSemaphore;

    long _instanceMaxCountReader;
    int _sendQueueSize;
    bool _useZeroCopy;

    // DDS Entities
    DDS_DomainParticipantFactory *_factory;
    DDS_DomainParticipant *_participant;
    DDS_Subscriber *_subscriber;
    DDS_Publisher *_publisher;
    DDS_DataReader *_reader;
    const char *_typename;
    NDDS_Type_Plugin *_plugin;

};

template <typename T, typename TSeq>
class RTICertImpl : public RTICertImplBase<T, TSeq>
{

public:

    RTICertImpl(const char *type_name, NDDS_Type_Plugin *plugin)
        : RTICertImplBase<T, TSeq>(type_name, plugin)
    { }

    virtual bool configure_participant_qos(
        DDS_DomainParticipantQos &qos) override;

protected:
    virtual IMessagingWriter *create_CertPublisher(
        DDS_DataWriter *writer) override;
    virtual IMessagingReader *create_CertSubscriber(
        DDS_DataReader *reader, bool isCallbackNull) override;
    virtual void *create_CertReaderListener(
        IMessagingCB *callback) override;
};

#ifdef RTI_ZEROCOPY_AVAILABLE
template <typename T, typename TSeq>
class RTICertImpl_ZCopy : public RTICertImplBase<T, TSeq>
{

public:
    /**
     * Constructor for CertImpl_ZCopy
     */
    RTICertImpl_ZCopy(const char *type_name, NDDS_Type_Plugin *plugin)
        : RTICertImplBase<T, TSeq>(type_name, plugin)
    { }

    virtual bool configure_participant_qos(
        DDS_DomainParticipantQos &qos) override;

        virtual void shutdown() override;

protected:
    virtual IMessagingWriter *create_CertPublisher(
        DDS_DataWriter *writer) override;
    virtual IMessagingReader *create_CertSubscriber(
        DDS_DataReader *reader, bool isCallbackNull) override;
    virtual void *create_CertReaderListener(
        IMessagingCB *callback) override;

    virtual bool create_domain_participant(
        DDS_DomainParticipantQos *dpQos) override;
};
#endif


#endif // PERFTEST_CERT
#endif // __RTICERTIMPL_H__
