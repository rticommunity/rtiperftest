/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __RTIDDSIMPL_H__
#define __RTIDDSIMPL_H__

#include <iostream>
#include <vector>
#include "perftest.hpp"
#include "MessagingIF.h"
#include <sstream>
#ifdef RTI_SECURE_PERFTEST
#include "security/security_default.h"
#endif
#include "rti/config/Logger.hpp"
#include "PerftestTransport.h"
#include <rti/domain/find.hpp>

#ifdef RTI_ZEROCOPY_AVAILABLE
#include "perftest_ZeroCopy.hpp"
#endif

#ifdef RTI_DARWIN
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#define RTIPERFTEST_MAX_PEERS 1024

/* Class for the DDS_DynamicDataMemberId of the type of RTI Perftest*/
class DynamicDataMembersId
{
  private:
    std::map<std::string, int> membersId;
    DynamicDataMembersId();

  public:
    ~DynamicDataMembersId();
    static DynamicDataMembersId &GetInstance();
    int at(std::string key);
};

template <typename T>
class RTIDDSImpl : public IMessaging
{
  public:

    RTIDDSImpl();

    ~RTIDDSImpl()
    {
        Shutdown();
    }

    bool validate_input();

    std::string PrintConfiguration();

    bool Initialize(ParameterManager &PM, perftest_cpp *parent);

    void Shutdown();

    unsigned long GetInitializationSampleCount();

    IMessagingWriter *CreateWriter(const std::string &topic_name);
    // Pass null for callback if using IMessagingSubscriber.ReceiveMessage()
    // to get data
    IMessagingReader *CreateReader(const std::string &topic_name, IMessagingCB *callback);

    dds::core::QosProvider getQosProviderForProfile(
            const std::string &library_name,
            const std::string &profile_name);

    template <typename U>
    dds::topic::ContentFilteredTopic<U> CreateCft(
        const std::string &topic_name,
        const dds::topic::Topic<U> &topic);

    const std::string get_qos_profile_name(std::string topicName);


  protected:
    // This semaphore is used in VxWorks to synchronize finalizing a factory
    static rti::core::Semaphore _finalizeFactorySemaphore;


    // Specific functions to configure the Security plugin
  #ifdef RTI_SECURE_PERFTEST
    void configureSecurePlugin(std::map<std::string, std::string> &dpQosProperties);
    std::string printSecureArgs();
    void validateSecureArgs();
  #endif


    long _InstanceMaxCountReader;
    unsigned long _sendQueueSize;
    int _InstanceHashBuckets;
    bool _isLargeData;
    bool _isFlatData;
    bool _isZeroCopy;
    PerftestTransport _transport;
    dds::domain::DomainParticipant _participant;
    dds::sub::Subscriber _subscriber;
    dds::pub::Publisher _publisher;

    rti::core::Semaphore _pongSemaphore;
    ParameterManager *_PM;
    perftest_cpp *_parent;
    std::map<std::string, std::string> _qoSProfileNameMap;

  #ifdef RTI_SECURE_PERFTEST
    static const std::string SECURE_PRIVATEKEY_FILE_PUB;
    static const std::string SECURE_PRIVATEKEY_FILE_SUB;
    static const std::string SECURE_CERTIFICATE_FILE_PUB;
    static const std::string SECURE_CERTIFICATE_FILE_SUB;
    static const std::string SECURE_CERTAUTHORITY_FILE;
    static const std::string SECURE_PERMISION_FILE_PUB;
    static const std::string SECURE_PERMISION_FILE_SUB;
    static const std::string SECURE_LIBRARY_NAME;
  #endif

    unsigned long int getShmemSHMMAX();
    dds::sub::qos::DataReaderQos setup_DR_QoS(
            std::string qos_profile,
            std::string topic_name);
    dds::pub::qos::DataWriterQos setup_DW_QoS(
            std::string qos_profile,
            std::string topic_name);
};

#ifdef RTI_FLATDATA_AVAILABLE
  /**
   * Overwrites CreateWriter and CreateReader from RTIDDSImpl
   * to return Writers and Readers that make use of FlatData API
   */
  template <typename T>
  class RTIDDSImpl_FlatData: public RTIDDSImpl<TestData_t> {
  public:
      /**
       * Constructor for RTIDDSImpl_FlatData
       *
       * @param isZeroCopy states if the type is also ZeroCopy
       */
      RTIDDSImpl_FlatData(bool isZeroCopy=false);

      /**
       * Creates a Publisher that uses the FlatData API
       *
       * @param topic_name is the name of the topic where
       *      the created writer will write new samples to
       *
       * @return a RTIFlatDataPublisher
       */
      IMessagingWriter *CreateWriter(const std::string &topic_name);

      /**
       * Creates a Subscriber that uses the FlatData API
       *
       * @param topic_name is the name of the topic where the created reader
       *      will read new samples from
       *
       * @param callback is the callback that will process the receibed message
       *      once it has been taken by the reader. Pass null for callback if
       *      using IMessagingSubscriber.ReceiveMessage() to get data
       *
       * @return a RTIFlatDataSubscriber
       */
      IMessagingReader *CreateReader(
              const std::string &topic_name, IMessagingCB *callback);
  };
#endif // RTI_FLATDATA_AVAILABLE

#endif // __RTIDDSIMPL_H__
