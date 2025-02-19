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
        shutdown();
    }

    bool data_size_related_calculations();

    bool validate_input();

    std::string print_configuration();

    bool initialize(ParameterManager &PM, perftest_cpp *parent);

    void shutdown();

    unsigned long get_initial_burst_size();

    IMessagingWriter *create_writer(const std::string &topic_name);
    // Pass null for callback if using IMessagingSubscriber.receive_message()
    // to get data
    IMessagingReader *create_reader(const std::string &topic_name, IMessagingCB *callback);

    /**
     * @brief This function calculates the overhead bytes that all the
     * members on TestData_* type add excluding the content of the sequence.
     *
     * @param size \b InOut. The size of the overhead of the data type.
     *
     * @return true if the operation was successful, otherwise false.
     */
    virtual bool get_serialized_overhead_size(unsigned int &overhead_size);

    dds::core::QosProvider getQosProviderForProfile(
            const std::string &library_name,
            const std::string &profile_name);

    template <typename U>
    dds::topic::ContentFilteredTopic<U> create_cft(
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
    unsigned long long _maxUnfragmentedRTPSPayloadSize;
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
    static const std::string LW_SECURE_LIBRARY_NAME;
  #endif

    unsigned long int getShmemSHMMAX();
    dds::sub::qos::DataReaderQos configure_writer_qos(
            std::string qos_profile,
            std::string topic_name);
    dds::pub::qos::DataWriterQos configure_reader_qos(
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
      IMessagingWriter *create_writer(const std::string &topic_name);

      /**
       * Creates a Subscriber that uses the FlatData API
       *
       * @param topic_name is the name of the topic where the created reader
       *      will read new samples from
       *
       * @param callback is the callback that will process the receibed message
       *      once it has been taken by the reader. Pass null for callback if
       *      using IMessagingSubscriber.receive_message() to get data
       *
       * @return a RTIFlatDataSubscriber
       */
      IMessagingReader *create_reader(
              const std::string &topic_name, IMessagingCB *callback);

      /**
       * @brief This function calculates the overhead bytes added by all the
       * members on the TestData_* type, excluding the content of the sequence.
       *
       * @param size \b InOut. The size of the overhead of the data type.
       *
       * @return true if the operation was successful, otherwise false.
       */
      bool get_serialized_overhead_size(unsigned int &overhead_size);
  };
#endif // RTI_FLATDATA_AVAILABLE

#endif // __RTIDDSIMPL_H__
