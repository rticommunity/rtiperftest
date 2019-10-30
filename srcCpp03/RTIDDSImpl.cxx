/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "RTIDDSImpl.h"
#include "perftest_cpp.h"
#include "qos_string.h"

using dds::core::xtypes::DynamicData;

#if defined(RTI_WIN32) || defined(RTI_INTIME)
  #pragma warning(push)
  #pragma warning(disable : 4996)
  #define STRNCASECMP _strnicmp
#elif defined(RTI_VXWORKS)
  #define STRNCASECMP strncmp
#else
  #define STRNCASECMP strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

#ifdef RTI_SECURE_PERFTEST
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PRIVATEKEY_FILE_PUB =
        "./resource/secure/pubkey.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PRIVATEKEY_FILE_SUB =
        "./resource/secure/subkey.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_CERTIFICATE_FILE_PUB =
        "./resource/secure/pub.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_CERTIFICATE_FILE_SUB =
        "./resource/secure/sub.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_CERTAUTHORITY_FILE =
        "./resource/secure/cacert.pem";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PERMISION_FILE_PUB =
        "./resource/secure/signed_PerftestPermissionsPub.xml";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_PERMISION_FILE_SUB =
        "./resource/secure/signed_PerftestPermissionsSub.xml";
template <typename T>
const std::string RTIDDSImpl<T>::SECURE_LIBRARY_NAME = "nddssecurity";
#endif

/*
 * Since std::to_string is not defined until c++11
 * we will define it here.
 */
namespace std {
    template<typename T>
    std::string to_string(const T &n) {
        std::ostringstream s;
        s << n;
        return s.str();
    }
}

std::string valid_flow_controller[] = {"default", "1Gbps", "10Gbps"};

template <typename T>
rti::core::Semaphore RTIDDSImpl<T>::_finalizeFactorySemaphore(RTI_OSAPI_SEMAPHORE_KIND_MUTEX, NULL);

/* Perftest DynamicDataMembersId class */
DynamicDataMembersId::DynamicDataMembersId()
{
    membersId["key"] = 1;
    membersId["entity_id"] = 2;
    membersId["seq_num"] = 3;
    membersId["timestamp_sec"] = 4;
    membersId["timestamp_usec"] = 5;
    membersId["latency_ping"] = 6;
    membersId["bin_data"] = 7;
}

DynamicDataMembersId::~DynamicDataMembersId()
{
    membersId.clear();
}

DynamicDataMembersId &DynamicDataMembersId::GetInstance()
{
    static DynamicDataMembersId instance;
    return instance;
}

int DynamicDataMembersId::at(std::string key)
{
   return membersId[key];
}

template <typename T>
RTIDDSImpl<T>::RTIDDSImpl():
        _InstanceMaxCountReader(dds::core::LENGTH_UNLIMITED),
        _sendQueueSize(0),
        _InstanceHashBuckets(dds::core::LENGTH_UNLIMITED),
        _isLargeData(false),
        _isFlatData(false),
        _isZeroCopy(false),

        _participant(dds::core::null),
        _subscriber(dds::core::null),
        _publisher(dds::core::null),
        _pongSemaphore(RTI_OSAPI_SEMAPHORE_KIND_BINARY,NULL)
    {
        _qoSProfileNameMap[LATENCY_TOPIC_NAME] = std::string("LatencyQos");
        _qoSProfileNameMap[ANNOUNCEMENT_TOPIC_NAME]
                = std::string("AnnouncementQos");
        _qoSProfileNameMap[THROUGHPUT_TOPIC_NAME]
                = std::string("ThroughputQos");
    }

/*********************************************************
 * Shutdown
 */
template <typename T>
void RTIDDSImpl<T>::Shutdown()
{
    std::vector<dds::domain::DomainParticipant> participants;

    _finalizeFactorySemaphore.take();

    // We have to explicitly close the DomainParticipant to finalize the factory
    _participant.close();

    // Find outstanding participants. Finding 1 is enought
    rti::domain::find_participants(std::back_inserter(participants), 1);

    // Delete factory only I am the only participant
    if (participants.empty()){
        _participant.finalize_participant_factory();
    } else {
        std::cout << "[Warning] Cannot finalize Domain Factory since it is being in use by another thread(s)"
                  << std::endl;
    }

    _finalizeFactorySemaphore.give();
}

/*********************************************************
 * ParseConfig
 */
template <typename T>
bool RTIDDSImpl<T>::validate_input()
{
    // Manage parameter -instance
    if (_PM->is_set("instances")) {
        _InstanceMaxCountReader = _PM->get<long>("instances");
    }

    // Manage parameter -peer
    if (_PM->get_vector<std::string>("peer").size() >= RTIPERFTEST_MAX_PEERS) {
        std::cerr << "The maximun of 'initial_peers' is " << RTIPERFTEST_MAX_PEERS
                  << std::endl;
        return false;
    }

    // Check if we need to enable Large Data. This works also for -scan
    if (_PM->get<unsigned long long>("dataLen") > (unsigned long) (std::min)(
                    MAX_SYNCHRONOUS_SIZE,
                    MAX_BOUNDED_SEQ_SIZE)) {
        _isLargeData = true;
    } else { /* No Large Data */
        _isLargeData = false;
    }

    // Manage parameter -batchSize
    if (_PM->get<long>("batchSize") > 0) {
        // We will not use batching for a latency test
        if (_PM->get<bool>("latencyTest")) {
            if (_PM->is_set("batchSize")) {
                std::cerr << "Batching cannot be used in a Latency test."
                          << std::endl;
                return false;
            } else {
                _PM->set<long>("batchSize", 0);  // Disable Batching
            }
        }

        // Check if using asynchronous
        if (_PM->get<bool>("asynchronous")) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") != 0) {
                std::cerr << "Batching cannot be used with asynchronous writing."
                          << std::endl;
                return false;
            } else {
                _PM->set<long>("batchSize", 0);  // Disable Batching
            }
        }

        /*
         * Large Data + batching cannot be set. But batching is enabled by default,
         * so in that case, we just disabled batching, else, the customer set it up,
         * so we explitly fail
         */
        if (_isLargeData) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") != 0) {
                std::cerr << "Batching cannot be used with Large Data."
                          << std::endl;
                return false;
            } else {
                _PM->set<long>("batchSize", -2);
            }
        } else if (((unsigned long)_PM->get<long>("batchSize")
                        < _PM->get<unsigned long long>("dataLen") * 2)
                    && !_PM->is_set("scan")) {
            /*
            * We don't want to use batching if the batch size is not large
            * enough to contain at least two samples (in this case we avoid the
            * checking at the middleware level).
            */
            if (_PM->is_set("batchSize")) {
                /*
                * Batchsize disabled. A message will be print if batchSize < 0
                * in perftest_cpp::PrintConfiguration()
                */
                _PM->set<long>("batchSize", -1);
            } else {
                _PM->set<long>("batchSize", 0); // Disable Batching
            }
        }

        if (_isFlatData) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") > 0) {
                std::cerr << "Batching cannot be used with FlatData."
                          << std::endl;
                return false;
            } else {
                _PM->set<long>("batchSize", -3);
            }
        }
    }

    // Manage parameter -enableTurboMode
    if (_PM->get<bool>("enableTurboMode")) {
        if (_PM->get<bool>("asynchronous")) {
            std::cerr << "[Error] Turbo Mode cannot be used with asynchronous writing. "
                      << std::endl;
            return false;
        } if (_isLargeData) {
            std::cerr << "[Error] Turbo Mode disabled, using large data."
                      << std::endl;
            _PM->set<bool>("enableTurboMode", false);
        }
    }

    // Manage parameter -writeInstance
    if (_PM->is_set("writeInstance")) {
        if (_PM->get<long>("instances") < _PM->get<long>("writeInstance")) {
            std::cerr << "Specified '-WriteInstance' ("
                      << _PM->get<long>("writeInstance")
                      << ") invalid: Bigger than the number of instances ("
                      << _PM->get<long>("instances")
                      << ")."
                      << std::endl;
            return false;
        }
    }

    // Manage transport parameter
    if (!_transport.validate_input()) {
         std::cerr << "[Error] Failure validation the transport options."
                   << std::endl;
        return false;
    };

    /*
     * Manage parameter -verbosity.
     * Setting verbosity if the parameter is provided
     */
    if (_PM->is_set("verbosity")) {
        switch (_PM->get<int>("verbosity")) {
            case 0:
                rti::config::Logger::instance().verbosity(
                        rti::config::Verbosity::SILENT);
                std::cerr << "Setting verbosity to SILENT." << std::endl;
                break;
            case 1:
                rti::config::Logger::instance().verbosity(
                        rti::config::Verbosity::ERRORY);
                std::cerr << "Setting verbosity to ERROR." << std::endl;
                break;
            case 2:
                rti::config::Logger::instance().verbosity(
                        rti::config::Verbosity::WARNING);
                std::cerr << "Setting verbosity to WARNING." << std::endl;
                break;
            case 3: rti::config::Logger::instance().verbosity(
                        rti::config::Verbosity::STATUS_ALL);
                std::cerr << "Setting verbosity to STATUS_ALL." << std::endl;
                break;
            default:
                std::cerr << "[Info]: Invalid value for the verbosity"
                          << " parameter. Using default value (1)"
                          << std::endl
                          << "Invalid value for the '-verbosity' parameter."
                          << std::endl;
                return false;
        }
    }

    // Manage parameter -secureGovernanceFile
    if (_PM->is_set("secureGovernanceFile")) {
            std::cout << "[INFO] Authentication, encryption, signing arguments "
                      << "will be ignored, and the values specified by the "
                      << "Governance file will be used instead"
                      << std::endl;
    }

    // Manage parameter -secureEncryptBoth
    if (_PM->is_set("secureEncryptBoth")) {
        _PM->set("secureEncryptData", true);
        _PM->set("secureEncryptSM", true);
    }

    return true;
}


/*********************************************************
 * PrintConfiguration
 */
template <typename T>
std::string RTIDDSImpl<T>::PrintConfiguration()
{

    std::ostringstream stringStream;

    // Domain ID
    stringStream << "\tDomain: " << _PM->get<int>("domain") << "\n";

    // Dynamic Data
    stringStream << "\tDynamic Data: ";
    if (_PM->get<bool>("dynamicData")) {
        stringStream << "Yes\n";
    } else {
        stringStream << "No\n";
    }

  #ifdef RTI_FLATDATA_AVAILABLE
    // FlatData
    stringStream << "\tFlatData: "
                << (_PM->get<bool>("flatdata") ? "Yes" : "No")
                << std::endl;

    // Zero Copy
    stringStream << "\tZero Copy: "
                << (_PM->get<bool>("zerocopy") ? "Yes" : "No");

    if (_PM->get<bool>("checkconsistency")) {
            stringStream << " (Check Consistency)";
    }

    stringStream << std::endl;
  #endif

    // Dynamic Data
    if (_PM->get<bool>("pub")) {
        stringStream << "\tAsynchronous Publishing: ";
        if (_isLargeData || _PM->get<bool>("asynchronous")) {
            stringStream << "Yes\n";
            stringStream << "\tFlow Controller: "
                         << _PM->get<std::string>("flowController")
                         << "\n";
        } else {
            stringStream << "No\n";
        }
    }

    // Turbo Mode / AutoThrottle
    if (_PM->get<bool>("enableTurboMode")) {
        stringStream << "\tTurbo Mode: Enabled\n";
    }
    if (_PM->get<bool>("enableAutoThrottle")) {
        stringStream << "\tAutoThrottle: Enabled\n";
    }

    // XML File
    stringStream << "\tXML File: ";
    if (_PM->get<bool>("noXmlQos")) {
        stringStream << "Disabled\n";
    } else {
        stringStream << _PM->get<std::string>("qosFile") << "\n";
    }

    stringStream << "\n" << _transport.printTransportConfigurationSummary();

    // set initial peers and not use multicast
    const std::vector<std::string> peerList = _PM->get_vector<std::string>("peer");
    if (!peerList.empty()) {
        stringStream << "\tInitial peers: ";
        for (unsigned int i = 0; i < peerList.size(); ++i) {
            stringStream << peerList[i];
            if (i == peerList.size() - 1) {
                stringStream << "\n";
            } else {
                stringStream << ", ";
            }
        }
    }

   #ifdef RTI_SECURE_PERFTEST
    if (_PM->group_is_used(SECURE)) {
        stringStream << "\n" << printSecureArgs();
    }
   #endif

    return stringStream.str();
}

/*********************************************************
 * DomainListener
 */
class DomainListener : public dds::domain::NoOpDomainParticipantListener
{

public:
    void on_inconsistent_topic(dds::topic::AnyTopic& topic,
            const dds::core::status::InconsistentTopicStatus& /*status*/) {
        std::cerr << "Found inconsistent topic. Expecting " << topic.name()
                << " of type " << topic.type_name() << std::endl;
    }

    void on_offered_incompatible_qos(dds::pub::AnyDataWriter& writer,
            const ::dds::core::status::OfferedIncompatibleQosStatus& status) {
        std::cerr << "Found incompatible reader for writer "
                << writer.topic_name() << " QoS is " << status.last_policy_id()
                << std::endl;
    }

    void on_requested_incompatible_qos(dds::sub::AnyDataReader& reader,
            const dds::core::status::RequestedIncompatibleQosStatus& status) {
        std::cerr << "Found incompatible writer for reader "
                << reader.topic_name() << " QoS is " << status.last_policy_id()
                << std::endl;
    }
};

/******************************************************************************/

/* RTIPublisher */

template<typename T>
class RTIPublisherBase: public IMessagingWriter {

protected:
    dds::pub::DataWriter<T> _writer;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    dds::core::InstanceHandleSeq _instance_handles;
    bool _useSemaphore;
    rti::core::Semaphore& _pongSemaphore;
    long _instancesToBeWritten;
    bool _isReliable;
    ParameterManager *_PM;

     /*
      * Returns the instance handler for the content filtered topic.
      * It must the last element pushed to _instance_handles
      * when appending instances to that sequence.
      */
    dds::core::InstanceHandle &getCftInstanceHandle() {
        return _instance_handles.back();
    }

public:
    RTIPublisherBase(
            dds::pub::DataWriter<T> writer,
            unsigned long num_instances,
            rti::core::Semaphore& pongSemaphore,
            bool useSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
          :
            _writer(writer),
            _num_instances(num_instances),
            _instance_counter(0),
            _useSemaphore(useSemaphore),
            _pongSemaphore(pongSemaphore),
            _instancesToBeWritten(instancesToBeWritten),
            _PM(PM)
    {
        using namespace dds::core::policy;

        _isReliable = (_writer.qos().template policy<Reliability>().kind()
                            == ReliabilityKind::RELIABLE);
    }

    void flush() {
        _writer->flush();
    }

    void waitForReaders(int numSubscribers) {
        while (_writer.publication_matched_status().current_count()
                < numSubscribers) {
            perftest_cpp::MilliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }

    void waitForPingResponse() {
        if (_useSemaphore) {
            _pongSemaphore.take();
        }
    }

    /* time out in milliseconds */
    void waitForPingResponse(int timeout) {
        RTINtpTime blockDurationIn;
        blockDurationIn.sec = timeout;
        blockDurationIn.frac = 0;

        if (_useSemaphore) {
            _pongSemaphore.take(&blockDurationIn);
        }
    }

    void notifyPingResponse() {
        if (_useSemaphore) {
            _pongSemaphore.give();
        }
    }

    unsigned int getPulledSampleCount() {
        return (unsigned int)_writer->datawriter_protocol_status().pulled_sample_count().total();
    }

    void waitForAck(long sec, unsigned long nsec) {

        if (_isReliable) {
            try {
                _writer->wait_for_acknowledgments(dds::core::Duration(sec, nsec));
            } catch (const dds::core::TimeoutError) {} // Expected exception
        } else {
            perftest_cpp::MilliSleep(nsec / 1000000);
        }

    }
};

template<typename T>
class RTIPublisher: public RTIPublisherBase<T> {

protected:
    T data;

public:
    RTIPublisher(
            dds::pub::DataWriter<T> writer,
            int num_instances,
            rti::core::Semaphore& pongSemaphore,
            bool useSemaphore,
            int instancesToBeWritten,
            ParameterManager *PM)
          :
            RTIPublisherBase<T>(
                    writer,
                    num_instances,
                    pongSemaphore,
                    useSemaphore,
                    instancesToBeWritten,
                    PM)
    {

        for (unsigned long i = 0; i < this->_num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                this->data.key()[c] = (unsigned char) (i >> c * 8);
            }
            this->_instance_handles.push_back(
                    this->_writer.register_instance(this->data));
        }
        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            this->data.key()[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
        }
        this->_instance_handles.push_back(
                this->_writer.register_instance(this->data));
    }

    bool send(TestMessage &message, bool isCftWildCardKey) {

        this->data.entity_id(message.entity_id);
        this->data.seq_num(message.seq_num);
        this->data.timestamp_sec(message.timestamp_sec);
        this->data.timestamp_usec(message.timestamp_usec);
        this->data.latency_ping(message.latency_ping);

        this->data.bin_data().resize(message.size);
        //data.bin_data(message.data);

        long key = 0;
        if (!isCftWildCardKey) {
            if (this->_num_instances > 1) {
                if (this->_instancesToBeWritten == -1) {
                    key = this->_instance_counter++ % this->_num_instances;
                } else { // send sample to a specific subscriber
                    key = this->_instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        for (int c = 0; c < KEY_SIZE; c++) {
            this->data.key()[c] = (unsigned char) (key >> c * 8);
        }
        if (!isCftWildCardKey) {
            this->_writer.write(this->data, this->_instance_handles[key]);
        } else {
            this->_writer.write(this->data, this->getCftInstanceHandle());
        }
        return true;
    }
};

#ifdef RTI_FLATDATA_AVAILABLE
  /**
   * Implementation of RTIPublisherBase for FlatData types.
   *
   * Since building a FlatData sample differs from
   * a classic type, we need to reimplement the Send() method with the
   * FlatData API.
   */
  template<typename T>
  class RTIFlatDataPublisher: public RTIPublisherBase<T> {
  protected:
      typedef typename rti::flat::flat_type_traits<T>::builder Builder;
      typedef typename rti::flat::PrimitiveArrayOffset<unsigned char, 4> KeyBuilder;
      typedef typename rti::flat::PrimitiveSequenceBuilder<unsigned char> BinDataBuilder;

      void add_key(Builder &builder, unsigned long int i) {
          KeyBuilder key_offset = builder.add_key();

          for (int j = 0; j < KEY_SIZE; ++j) {
              // The key will be i but splitted in bytes
              key_offset.set_element(j, (unsigned char) (i >> j * 8));
          }
      }

  public:
      RTIFlatDataPublisher(
              dds::pub::DataWriter<T> writer,
              int num_instances,
              rti::core::Semaphore& pongSemaphore,
              bool useSemaphore,
              int instancesToBeWritten,
              ParameterManager *PM)
              : RTIPublisherBase<T>(
                      writer,
                      num_instances,
                      pongSemaphore,
                      useSemaphore,
                      instancesToBeWritten,
                      PM)
      {
          for (unsigned long int i = 0; i < this->_num_instances; ++i) {
              Builder builder = rti::flat::build_data(writer);
              add_key(builder, i);

              T *sample = builder.finish_sample();

              this->_instance_handles.push_back(
                  this->_writer.register_instance(*sample));

              this->_writer.extensions().discard_loan(*sample);
          }

          // Register the key of MAX_CFT_VALUE
          Builder builder = rti::flat::build_data(writer);
          add_key(builder, MAX_CFT_VALUE);
          T *sample = builder.finish_sample();

          this->_instance_handles.push_back(
              this->_writer.register_instance(*sample));

          this->_writer.extensions().discard_loan(*sample);
      }

      /**
       * Build and send a sample from a given message using FlatData API.
       *
       * @param message the message that contains the information to build the sample
       * @param isCftWildcardKey states if CFT is being used
       */
      bool send(TestMessage &message, bool isCftWildcardKey) {
          Builder builder;
          long key = 0;

          try {
              builder = rti::flat::build_data(this->_writer);
          } catch (const std::exception &ex) {
              return false;
          }

          // Initialize Information data
          builder.add_entity_id(message.entity_id);
          builder.add_seq_num(message.seq_num);
          builder.add_timestamp_sec(message.timestamp_sec);
          builder.add_timestamp_usec(message.timestamp_usec);
          builder.add_latency_ping(message.latency_ping);

          // Add payload
          BinDataBuilder bin_data_builder = builder.build_bin_data();
          bin_data_builder.add_n(message.size);
          bin_data_builder.finish();

          // calculate key and add it
          if (!isCftWildcardKey && this->_num_instances > 1) {
              key = (this->_instancesToBeWritten == -1)
                      ? this->_instance_counter++ % this->_num_instances
                      : this->_instancesToBeWritten;
          } else {
              key = MAX_CFT_VALUE;
          }

          add_key(builder, key);

          // Build the data to be sent
          T *sample = builder.finish_sample();

          // Send data through the writer
          if (!isCftWildcardKey) {
              this->_writer.write(*sample, this->_instance_handles[key]);
          } else {
              this->_writer.write(*sample, this->getCftInstanceHandle());
          }

          return true;
      }
  };
#endif


class RTIDynamicDataPublisher: public RTIPublisherBase<DynamicData> {

protected:
    DynamicData data;
    int _last_message_size;

public:
    RTIDynamicDataPublisher(
            dds::pub::DataWriter<DynamicData> writer,
            int num_instances,
            rti::core::Semaphore& pongSemaphore,
            bool useSemaphore,
            int instancesToBeWritten,
            const dds::core::xtypes::StructType& typeCode,
            ParameterManager *PM)
            : RTIPublisherBase<DynamicData>(
                    writer,
                    num_instances,
                    pongSemaphore,
                    useSemaphore,
                    instancesToBeWritten,
                    PM),
            data(typeCode),
            _last_message_size(0)
    {
        std::vector<uint8_t> key_octets(KEY_SIZE);
        for (unsigned long i = 0; i < this->_num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                key_octets[c] = (uint8_t) (i >> c * 8);
            }
            this->data.set_values(
                    DynamicDataMembersId::GetInstance().at("key"),
                    key_octets);
            this->_instance_handles.push_back(
                    this->_writer.register_instance(this->data));
        }
        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (uint8_t)(MAX_CFT_VALUE >> c * 8);
        }
        this->data.set_values(
                    DynamicDataMembersId::GetInstance().at("key"),
                    key_octets);
        this->_instance_handles.push_back(
                this->_writer.register_instance(this->data));
    }

    bool send(TestMessage &message, bool isCftWildCardKey) {
        if (_last_message_size != message.size) {
            this->data.clear_all_members();
            std::vector<uint8_t> octec_seq(message.size);
            this->data.set_values(
                    DynamicDataMembersId::GetInstance().at("bin_data"),
                    octec_seq);
        }
        this->data.value(
                DynamicDataMembersId::GetInstance().at("entity_id"),
                message.entity_id);
        this->data.value(
                DynamicDataMembersId::GetInstance().at("seq_num"),
                message.seq_num);
        this->data.value(
                DynamicDataMembersId::GetInstance().at("timestamp_sec"),
                message.timestamp_sec);
        this->data.value(
                DynamicDataMembersId::GetInstance().at("timestamp_usec"),
                message.timestamp_usec);
        this->data.value(
                DynamicDataMembersId::GetInstance().at("latency_ping"),
                message.latency_ping);

        long key = 0;
        std::vector<uint8_t> key_octets(KEY_SIZE);
        if (!isCftWildCardKey) {
            if (this->_num_instances > 1) {
                if (this->_instancesToBeWritten == -1) {
                    key = this->_instance_counter++ % this->_num_instances;
                } else { // Send sample to a specific subscriber
                    key = this->_instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }

        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (uint8_t) (key >> c * 8);
        }

        this->data.set_values(
                DynamicDataMembersId::GetInstance().at("key"),
                key_octets);

        if (!isCftWildCardKey) {
            this->_writer.write(this->data, this->_instance_handles[key]);
        } else {
            this->_writer.write(this->data, this->getCftInstanceHandle());
        }

        return true;
    }
};

/******************************************************************************/

/* ReceiverListener */

template<typename T>
class ReceiverListenerBase: public dds::sub::NoOpDataReaderListener<T> {

protected:
    TestMessage _message;
    IMessagingCB *_callback;
    dds::sub::LoanedSamples<T> samples;

public:
    ReceiverListenerBase(IMessagingCB *callback) :
            _message(),
            _callback(callback) {
    }
};

template<typename T>
class ReceiverListener: public ReceiverListenerBase<T> {

public:
    ReceiverListener(IMessagingCB *callback) :
        ReceiverListenerBase<T>(callback) {
    }

    void on_data_available(dds::sub::DataReader<T> &reader) {

        this->samples = reader.take();

        for (unsigned int i = 0; i < this->samples.length(); ++i) {
            if (this->samples[i].info().valid()) {
                const T & sample = this->samples[i].data();
                this->_message.entity_id = sample.entity_id();
                this->_message.seq_num = sample.seq_num();
                this->_message.timestamp_sec = sample.timestamp_sec();
                this->_message.timestamp_usec = sample.timestamp_usec();
                this->_message.latency_ping = sample.latency_ping();
                this->_message.size = (int) sample.bin_data().size();
                //this->_message.data = sample.bin_data();
                this->_callback->ProcessMessage(this->_message);
            }
        }
    }
};

#ifdef RTI_FLATDATA_AVAILABLE
  /**
   * Implements ReceiverListenerBase with FlatData API.
   *
   * Since reading a FlatData sample differs from a classic type we need
   * to reimplement on_data_available method.
   */
  template<typename T>
  class FlatDataReceiverListener : public ReceiverListenerBase<T> {
  protected:
      bool _isZeroCopy;
      bool _checkConsistency;

  public:
      typedef typename rti::flat::flat_type_traits<T>::offset::ConstOffset ConstOffset;

      /**
       * Contructor of FlatDataReceiverListener
       *
       * @param callback callback that will process received messages
       *
       * @param isZeroCopy states if Zero Copy will be used
       */
      FlatDataReceiverListener(IMessagingCB *callback, bool isZeroCopy, bool checkConsistency) :
          ReceiverListenerBase<T>(callback),
          _isZeroCopy(isZeroCopy),
          _checkConsistency(checkConsistency){
      }

      /**
       * Take a new sample and process it using FlatData API.
       *
       * @param reader is the reader to take samples from
       */
      void on_data_available(dds::sub::DataReader<T> &reader) {
          dds::sub::LoanedSamples<T> samples = reader.take();

          for (unsigned int i = 0; i < samples.length(); ++i) {
              if (samples[i].info().valid()) {
                  const T &sample = samples[i].data();
                  ConstOffset message = sample.root();

                  this->_message.entity_id = message.entity_id();
                  this->_message.seq_num = message.seq_num();
                  this->_message.timestamp_sec = message.timestamp_sec();
                  this->_message.timestamp_usec = message.timestamp_usec();
                  this->_message.latency_ping = message.latency_ping();
                  this->_message.size = message.bin_data().element_count();
                  // bin_data should be retrieved here

                  // Check that the sample was not modified on the publisher side when using Zero Copy.
                  if (_isZeroCopy && _checkConsistency) {
                      if (!reader->is_data_consistent(samples[i])) continue;
                  }

                  this->_callback->ProcessMessage(this->_message);
              }
          }
      }
  };
#endif

class DynamicDataReceiverListener: public ReceiverListenerBase<DynamicData> {
public:
    DynamicDataReceiverListener(IMessagingCB *callback) :
        ReceiverListenerBase<DynamicData>(callback) {
    }

    void on_data_available(dds::sub::DataReader<DynamicData> &reader) {

        this->samples = reader.take();

        for (unsigned int i = 0; i < this->samples.length(); ++i) {
            if (this->samples[i].info().valid()) {
                DynamicData& sample =
                        const_cast<DynamicData&>(this->samples[i].data());
                this->_message.entity_id = sample.value<int32_t>(
                        DynamicDataMembersId::GetInstance().at("entity_id"));
                this->_message.seq_num = sample.value<uint32_t>(
                        DynamicDataMembersId::GetInstance().at("seq_num"));
                this->_message.timestamp_sec = sample.value<int32_t>(
                        DynamicDataMembersId::GetInstance().at("timestamp_sec"));
                this->_message.timestamp_usec = sample.value<uint32_t>(
                        DynamicDataMembersId::GetInstance().at("timestamp_usec"));
                this->_message.latency_ping = sample.value<int32_t>(
                        DynamicDataMembersId::GetInstance().at("latency_ping"));
                this->_message.size = (int)(sample.get_values<uint8_t>(
                        DynamicDataMembersId::GetInstance().at("bin_data")).size());

                //_message.data = sample.bin_data();
                _callback->ProcessMessage(this->_message);
            }
        }
    }
};

/******************************************************************************/

/* RTISubscriber */

template<typename T>
class RTISubscriberBase: public IMessagingReader {

protected:
    dds::sub::DataReader<T> _reader;
    ReceiverListenerBase<T> *_readerListener;
    TestMessage _message;
    dds::core::cond::WaitSet _waitset;
    int _data_idx;
    bool _no_data;
    ParameterManager *_PM;

public:
    RTISubscriberBase(
            dds::sub::DataReader<T> reader,
            ReceiverListenerBase<T> *readerListener,
            ParameterManager *PM) :
                    _reader(reader),
                    _readerListener(readerListener),
                    _waitset(rti::core::cond::WaitSetProperty(
                            PM->get<long>("waitsetEventCount"),
                            dds::core::Duration::from_microsecs((long)
                                    PM->get<unsigned long long>("waitsetDelayUsec")))),
                    _PM(PM)
    {
        // null listener means using receive thread
        if (_reader.listener() == NULL) {

            // Using status conditions:
            dds::core::cond::StatusCondition reader_status(_reader);
            reader_status.enabled_statuses(
                    dds::core::status::StatusMask::data_available());
            _waitset += reader_status;

            /* Uncomment these lines and comment previous ones to use Read
             * conditions instead of status conditions
             *
             * dds::sub::cond::ReadCondition read_condition(_reader,
             * dds::sub::status::DataState::any_data());
             * _waitset += read_condition; */
        }

        _no_data = true;
        _data_idx = 0;
    }

    void Shutdown() {
        _reader.listener(NULL, dds::core::status::StatusMask::none());
        if (_readerListener != NULL) {
            delete(_readerListener);
        }
    }

    void waitForWriters(int numPublishers) {
        while (_reader.subscription_matched_status().current_count() < numPublishers) {
            perftest_cpp::MilliSleep(PERFTEST_DISCOVERY_TIME_MSEC);
        }
    }
};

template<typename T>
class RTISubscriber: public RTISubscriberBase<T> {

public:
    RTISubscriber(
            dds::sub::DataReader<T> reader,
            ReceiverListenerBase<T> *readerListener,
            ParameterManager *PM)
          :
            RTISubscriberBase<T>(
                    reader,
                    readerListener,
                    PM)
    {}

    TestMessage *ReceiveMessage() {
        int seq_length;

        while (true) {

            if (this->_no_data) {
                this->_waitset.wait(dds::core::Duration::infinite());

            }
            dds::sub::LoanedSamples<T> samples = this->_reader.take();

            this->_data_idx = 0;
            this->_no_data = false;

            seq_length = samples.length();
            if (this->_data_idx == seq_length) {
                this->_no_data = true;
                continue;
            }

            // skip non-valid data
            while ((!samples[this->_data_idx].info().valid())
                    && (++(this->_data_idx) < seq_length))
                ;

            // may have hit end condition
            if (this->_data_idx == seq_length) {
                continue;
            }

            const T& data = samples[this->_data_idx].data();
            this->_message.entity_id = data.entity_id();
            this->_message.seq_num = data.seq_num();
            this->_message.timestamp_sec = data.timestamp_sec();
            this->_message.timestamp_usec = data.timestamp_usec();
            this->_message.latency_ping = data.latency_ping();
            this->_message.size = (int) data.bin_data().size();
            //_message.data = samples[_data_idx].data().bin_data();

            ++(this->_data_idx);

            return &(this->_message);
        }
        return NULL;
    }

    void ReceiveAndProccess(IMessagingCB *listener) {
        while (!listener->end_test) {

            this->_waitset.dispatch(dds::core::Duration::infinite());
            dds::sub::LoanedSamples<T> samples = this->_reader.take();

            for (unsigned int i = 0; i < samples.length(); ++i) {
                if (samples[i].info().valid()) {
                    const T & sample = samples[i].data();
                    this->_message.entity_id = sample.entity_id();
                    this->_message.seq_num = sample.seq_num();
                    this->_message.timestamp_sec = sample.timestamp_sec();
                    this->_message.timestamp_usec = sample.timestamp_usec();
                    this->_message.latency_ping = sample.latency_ping();
                    this->_message.size = (int) sample.bin_data().size();
                    //_message.data = sample.bin_data();

                    listener->ProcessMessage(this->_message);
                }
            }
        }
    }
};

#ifdef RTI_FLATDATA_AVAILABLE
  /**
   * Implements RTISubscriberBase with FlatData API.
   *
   * Since reading a FlatData sample differs from a classic type we need
   * to reimplement ReceiveMessage method.
   */
  template<typename T>
  class RTIFlatDataSubscriber: public RTISubscriberBase<T> {
  protected:
      bool _isZeroCopy;
      bool _checkConsistency;

  public:
      typedef typename rti::flat::flat_type_traits<T>::offset::ConstOffset ConstOffset;

      RTIFlatDataSubscriber(
              dds::sub::DataReader<T> reader,
              ReceiverListenerBase<T> *readerListener,
              ParameterManager *PM)
          :RTISubscriberBase<T>(
                    reader,
                    readerListener,
                    PM) {
              _isZeroCopy = PM->get<bool>("zerocopy");
              _checkConsistency = PM->get<bool>("checkconsistency");
          }

      /**
       * Receive a new sample when it is available. It uses a waitset
       *
       * @return a message with the information from the sample
       */
      TestMessage *ReceiveMessage() {

          int seq_length;

          while (true) {

              if (this->_no_data) {
                  this->_waitset.wait(dds::core::Duration::infinite());

              }
              dds::sub::LoanedSamples<T> samples = this->_reader.take();

              this->_data_idx = 0;
              this->_no_data = false;

              seq_length = samples.length();
              if (this->_data_idx == seq_length) {
                  this->_no_data = true;
                  continue;
              }

              // skip non-valid data
              while ((!samples[this->_data_idx].info().valid())
                      && (++(this->_data_idx) < seq_length))
                  ;

              // may have hit end condition
              if (this->_data_idx == seq_length) {
                  continue;
              }

              const T &message_sample = samples[this->_data_idx].data();
              ConstOffset message = message_sample.root();

              this->_message.entity_id = message.entity_id();
              this->_message.seq_num = message.seq_num();
              this->_message.timestamp_sec = message.timestamp_sec();
              this->_message.timestamp_usec = message.timestamp_usec();
              this->_message.latency_ping = message.latency_ping();
              this->_message.size = message.bin_data().element_count();

              ++(this->_data_idx);

              // Check that the sample was not modified on the publisher side when using Zero Copy.
              if (_isZeroCopy && _checkConsistency) {
                  if (!this->_reader->is_data_consistent(samples[this->_data_idx])) continue;
              }

              return &(this->_message);
          }
          return NULL;
      }

      /**
       * Take arriving samples from the reader and process them.
       *
       * @param listener that implements the ProcessMessage method
       *    that will be used to process received samples.
       */
      void ReceiveAndProccess(IMessagingCB *listener) {
          while (!listener->end_test) {

              this->_waitset.dispatch(dds::core::Duration::infinite());
              dds::sub::LoanedSamples<T> samples = this->_reader.take();

              for (unsigned int i = 0; i < samples.length(); ++i) {
                  if (samples[i].info().valid()) {
                      const T &message_sample = samples[i].data();
                      ConstOffset message = message_sample.root();

                      this->_message.entity_id = message.entity_id();
                      this->_message.seq_num = message.seq_num();
                      this->_message.timestamp_sec = message.timestamp_sec();
                      this->_message.timestamp_usec = message.timestamp_usec();
                      this->_message.latency_ping = message.latency_ping();
                      this->_message.size = message.bin_data().element_count();
                      //_message.data = message.bin_data();

                      // Check that the sample was not modified on the publisher side when using Zero Copy.
                      if (_isZeroCopy && _checkConsistency) {
                          if (!this->_reader->is_data_consistent(samples[i])) continue;
                      }

                      listener->ProcessMessage(this->_message);
                  }
              }
          }
      }
  };
#endif

class RTIDynamicDataSubscriber: public RTISubscriberBase<DynamicData> {
public:
    RTIDynamicDataSubscriber(
            dds::sub::DataReader<DynamicData> reader,
            DynamicDataReceiverListener *readerListener,
            ParameterManager *PM)
          :
            RTISubscriberBase<DynamicData>(
                    reader,
                    readerListener,
                    PM)
    {}

    TestMessage *ReceiveMessage() {

        int seq_length;

        while (true) {

            if (this->_no_data) {
                this->_waitset.wait(dds::core::Duration::infinite());
            }

            dds::sub::LoanedSamples<DynamicData> samples = this->_reader.take();
            this->_data_idx = 0;
            this->_no_data = false;

            seq_length = samples.length();
            if (this->_data_idx == seq_length) {
                this->_no_data = true;
                continue;
            }

            // skip non-valid data
            while ((!samples[this->_data_idx].info().valid())
                    && (++(this->_data_idx) < seq_length));

            // may have hit end condition
            if (this->_data_idx == seq_length) {
                continue;
            }

            DynamicData& sample = const_cast<DynamicData&>(
                    samples[this->_data_idx].data());
            this->_message.entity_id = sample.value<int32_t>(
                    DynamicDataMembersId::GetInstance().at("entity_id"));
            this->_message.seq_num = sample.value<uint32_t>(
                    DynamicDataMembersId::GetInstance().at("seq_num"));
            this->_message.timestamp_sec = sample.value<int32_t>(
                    DynamicDataMembersId::GetInstance().at("timestamp_sec"));
            this->_message.timestamp_usec = sample.value<uint32_t>(
                    DynamicDataMembersId::GetInstance().at("timestamp_usec"));
            this->_message.latency_ping = sample.value<int32_t>(
                    DynamicDataMembersId::GetInstance().at("latency_ping"));
            this->_message.size = (int)(sample.get_values<uint8_t>(
                    DynamicDataMembersId::GetInstance().at("bin_data")).size());

            ++(this->_data_idx);
            return &_message;
        }
        return NULL;
    }

    void ReceiveAndProccess(IMessagingCB *listener) {
        while (!listener->end_test) {

            this->_waitset.dispatch(dds::core::Duration::infinite());
            dds::sub::LoanedSamples<DynamicData> samples =
                    this->_reader.take();

            for (unsigned int i = 0; i < samples.length(); ++i) {
                if (samples[i].info().valid()) {
                    DynamicData& sample =
                            const_cast<DynamicData&>(
                                    samples[i].data());
                    this->_message.entity_id = sample.value<int32_t>(
                            DynamicDataMembersId::GetInstance().at(
                                    "entity_id"));
                    this->_message.seq_num = sample.value<uint32_t>(
                            DynamicDataMembersId::GetInstance().at(
                                    "seq_num"));
                    this->_message.timestamp_sec = sample.value<int32_t>(
                            DynamicDataMembersId::GetInstance().at(
                                    "timestamp_sec"));
                    this->_message.timestamp_usec = sample.value<uint32_t>(
                            DynamicDataMembersId::GetInstance().at(
                                    "timestamp_usec"));
                    this->_message.latency_ping = sample.value<int32_t>(
                            DynamicDataMembersId::GetInstance().at("latency_ping"));
                    this->_message.size = (int)(sample.get_values<uint8_t>(
                            DynamicDataMembersId::GetInstance().at("bin_data")).size());
                    //_message.data = sample.bin_data();
                    listener->ProcessMessage(this->_message);
                }
            }
        }
    }
};

/******************************************************************************/

#ifdef RTI_SECURE_PERFTEST

template<typename T>
void RTIDDSImpl<T>::configureSecurePlugin(
        std::map<std::string,
        std::string> &dpQosProperties) {

    // load plugin
    dpQosProperties["com.rti.serv.load_plugin"] = "com.rti.serv.secure";

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    dpQosProperties["com.rti.serv.secure.create_function"] =
            "RTI_Security_PluginSuite_create";

    dpQosProperties["com.rti.serv.secure.library"]
            = _PM->get<std::string>("secureLibrary");

#else // Static library linking

    void *pPtr = (void *) RTI_Security_PluginSuite_create;
    dpQosProperties["com.rti.serv.secure.create_function_ptr"] =
            rti::util::ptr_to_str(pPtr);

  #endif

    /*
     * Below, we are using com.rti.serv.secure properties in order to be
     * backward compatible with RTI Connext DDS 5.3.0 and below. Later versions
     * use the properties that are specified in the DDS Security specification
     * (see also the RTI Security Plugins Getting Started Guide). However,
     * later versions still support the legacy properties as an alternative.
     */

    // check if governance file provided
    if (_PM->get<std::string>("secureGovernanceFile").empty()) {
        // choose a pre-built governance file
        std::string governanceFilePath = "./resource/secure/signed_PerftestGovernance_";
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

        dpQosProperties["com.rti.serv.secure.access_control.governance_file"] =
                governanceFilePath;

        /*
         * Save the local variable governanceFilePath into
         * the parameter "secureGovernanceFile"
         */
        _PM->set("secureGovernanceFile", governanceFilePath);

    } else {
        dpQosProperties["com.rti.serv.secure.access_control.governance_file"] =
                _PM->get<std::string>("secureGovernanceFile");
    }

    // permissions file
    dpQosProperties["com.rti.serv.secure.access_control.permissions_file"]
            = _PM->get<std::string>("securePermissionsFile");

    // permissions authority file
    dpQosProperties["com.rti.serv.secure.access_control.permissions_authority_file"]
            = _PM->get<std::string>("secureCertAuthority");

    // certificate authority
    dpQosProperties["com.rti.serv.secure.authentication.ca_file"]
            = _PM->get<std::string>("secureCertAuthority");

    // public key
    dpQosProperties["com.rti.serv.secure.authentication.certificate_file"]
            = _PM->get<std::string>("secureCertFile");

    // private key
    dpQosProperties["com.rti.serv.secure.authentication.private_key_file"]
            = _PM->get<std::string>("securePrivateKey");

    if (_PM->is_set("secureDebug")) {
        std::ostringstream string_stream_object;
        string_stream_object << _PM->get<int>("secureDebug");
        dpQosProperties["com.rti.serv.secure.logging.log_level"] =
                string_stream_object.str();
    }
}

template <typename T>
void RTIDDSImpl<T>::validateSecureArgs()
{
    if (_PM->group_is_used(SECURE)) {
        if (_PM->get<std::string>("securePrivateKey").empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("securePrivateKey", SECURE_PRIVATEKEY_FILE_PUB);
            } else {
                _PM->set("securePrivateKey", SECURE_PRIVATEKEY_FILE_SUB);
            }
        }

        if (_PM->get<std::string>("secureCertFile").empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("secureCertFile", SECURE_CERTIFICATE_FILE_PUB);
            } else {
                _PM->set("secureCertFile", SECURE_CERTIFICATE_FILE_SUB);
            }
        }

        if (_PM->get<std::string>("secureCertAuthority").empty()) {
            _PM->set("secureCertAuthority", SECURE_CERTAUTHORITY_FILE);
        }

        if (_PM->get<std::string>("securePermissionsFile").empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("securePermissionsFile", SECURE_PERMISION_FILE_PUB);
            } else {
                _PM->set("securePermissionsFile", SECURE_PERMISION_FILE_SUB);
            }
        }

      #ifdef RTI_PERFTEST_DYNAMIC_LINKING
        if (_PM->get<std::string>("secureLibrary").empty()) {
            _PM->set("secureLibrary", SECURE_LIBRARY_NAME);
        }
      #endif
    }
}

template <typename T>
std::string RTIDDSImpl<T>::printSecureArgs()
{
    std::ostringstream stringStream;
    stringStream << "Secure Configuration:\n";

    stringStream << "\tEncrypt discovery: ";
    if (_PM->get<bool>("secureEncryptDiscovery")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt topic (user) data: ";
    if (_PM->get<bool>("secureEncryptData")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt submessage: ";
    if (_PM->get<bool>("secureEncryptData")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tSign data: ";
    if (_PM->get<bool>("secureSign")) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tGovernance file: ";
    if (_PM->get<std::string>("secureGovernanceFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureGovernanceFile")
                     << "\n";
    }

    stringStream << "\tPermissions file: ";
    if (_PM->get<std::string>("securePermissionsFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("securePermissionsFile")
                     << "\n";
    }

    stringStream << "\tPrivate key file: ";
    if (_PM->get<std::string>("securePrivateKey").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("securePrivateKey") << "\n";
    }

    stringStream << "\tCertificate file: ";
    if (_PM->get<std::string>("secureCertFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureCertFile") << "\n";
    }

    stringStream << "\tCertificate authority file: ";
    if (_PM->get<std::string>("secureCertAuthority").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureCertAuthority") << "\n";
    }

    stringStream << "\tPlugin library: ";
    if (_PM->get<std::string>("secureLibrary").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _PM->get<std::string>("secureLibrary") << "\n";
    }

    if (_PM->is_set("secureDebug")) {
        stringStream << "\tDebug level: "
                     << _PM->get<int>("secureDebug")
                     << "\n";
    }

    return stringStream.str();
}

#endif

template <typename T>
dds::core::QosProvider RTIDDSImpl<T>::getQosProviderForProfile(
        const std::string &library_name,
        const std::string &profile_name)
{
    using dds::core::QosProvider;

    QosProvider qosProvider(dds::core::null);

    if (!_PM->get<bool>("noXmlQos")) {
        qosProvider = dds::core::QosProvider(
                _PM->get<std::string>("qosFile"),
                library_name + "::" + profile_name);
    } else {
        rti::core::QosProviderParams perftestQosProviderParams;
        dds::core::StringSeq perftestStringProfile(
               PERFTEST_QOS_STRING,
               PERFTEST_QOS_STRING + PERFTEST_QOS_STRING_SIZE);
        perftestQosProviderParams.string_profile(perftestStringProfile);

        qosProvider = QosProvider::Default();
        qosProvider->provider_params(perftestQosProviderParams);
        qosProvider->default_library(library_name);
        qosProvider->default_profile(profile_name);
    }

    return qosProvider;
}

/*********************************************************
 * Initialize
 */
template <typename T>
bool RTIDDSImpl<T>::Initialize(ParameterManager &PM, perftest_cpp *parent)
{
    using namespace rti::core::policy;
    // Assigne the ParameterManager
    _PM = &PM;
    _transport.initialize(_PM);

    if (!validate_input()) {
        return false;
    }

    // setup the QOS profile file to be loaded
    dds::core::QosProvider qos_provider = getQosProviderForProfile(
            _PM->get<std::string>("qosLibrary"),
            "BaseProfileQos");
    dds::domain::qos::DomainParticipantQos qos = qos_provider.participant_qos();
    dds::pub::qos::PublisherQos publisherQoS = qos_provider.publisher_qos();

    /* Mask for threadPriorities when it's used */
    rti::core::ThreadSettingsKindMask mask(
            rti::core::ThreadSettingsKindMask::realtime_priority());

    if (parent == NULL) {
        return false;
    }
    _parent = parent;
    ThreadPriorities threadPriorities = parent->get_thread_priorities();

    std::map<std::string, std::string> properties =
            qos.policy<Property>().get_all();

  #ifdef RTI_SECURE_PERFTEST
    if (_PM->group_is_used(SECURE)) {
        validateSecureArgs();
        configureSecurePlugin(properties);
    }
  #endif

    Discovery qos_discovery = qos.policy<Discovery>(); //get all the Discovery
    // set initial peers and not use multicast

    if (!_PM->get_vector<std::string>("peer").empty()) {
        qos_discovery.initial_peers(_PM->get_vector<std::string>("peer"));
        qos_discovery.multicast_receive_addresses(dds::core::StringSeq());
    }

    if (!configureTransport(_transport, qos, properties, _PM)){
        return false;
    };

    // set thread priorities.
    if (threadPriorities.isSet) {
        // Set real time schedule
        qos.policy<ReceiverPool>().thread().mask(mask);
        qos.policy<Event>().thread().mask(mask);
        qos.policy<Database>().thread().mask(mask);

        // Set priority
        qos.policy<ReceiverPool>().thread().priority(
                threadPriorities.receive);
        qos.policy<Event>().thread().priority(
                threadPriorities.dbAndEvent);
        qos.policy<Database>().thread().priority(
                threadPriorities.dbAndEvent);
    }

    if (_PM->get<bool>("enableAutoThrottle")) {
        properties["dds.domain_participant.auto_throttle.enable"] = "true";
    }

    qos << qos_discovery;
    //We have to copy the properties to the participant_qos object
    qos << rti::core::policy::Property(
            properties.begin(),
            properties.end());

    DomainListener *listener = new DomainListener;

    // Creates the participant
    _participant = dds::domain::DomainParticipant(
            _PM->get<int>("domain"),
            qos,
            listener,
            dds::core::status::StatusMask::inconsistent_topic() |
            dds::core::status::StatusMask::offered_incompatible_qos() |
            dds::core::status::StatusMask::requested_incompatible_qos() );

    // Set publisher QoS
    if (threadPriorities.isSet) {
        // Asynchronous thread priority
        publisherQoS.policy<AsynchronousPublisher>().disable_asynchronous_write(false);
        publisherQoS.policy<AsynchronousPublisher>().thread().mask(mask);
        publisherQoS.policy<AsynchronousPublisher>().thread().priority(
                threadPriorities.main);

        // Asynchronous thread for batching priority
        publisherQoS.policy<AsynchronousPublisher>().disable_asynchronous_batch(false);
        publisherQoS.policy<AsynchronousPublisher>().asynchronous_batch_thread()
                .mask(mask);
        publisherQoS.policy<AsynchronousPublisher>().asynchronous_batch_thread()
                .priority(threadPriorities.main);
    }

    // Create the _publisher and _subscriber
    _publisher = dds::pub::Publisher(_participant, publisherQoS);

    _subscriber = dds::sub::Subscriber(_participant, qos_provider.subscriber_qos());

    return true;

}

/*********************************************************
 * GetInitializationSampleCount
 */
template <typename T>
unsigned long RTIDDSImpl<T>::GetInitializationSampleCount()
{
    /*
     * If we are using reliable, the maximum burst of that we can send is limited
     * by max_send_window_size (or max samples, but we will assume this is not
     * the case for this). In such case we should send max_send_window_size
     * samples.
     *
     * If we are not using reliability this should not matter.
     */
    unsigned long initializeSampleCount = this->_sendQueueSize;

    /*
     * If we are using batching we need to take into account tha the Send Queue
     * will be per-batch, therefore for the number of samples:
     */
    if (_PM->get<long>("batchSize") > 0) {
        initializeSampleCount = (std::max)(
                _PM->get<int>("sendQueueSize") *
                        (_PM->get<long>("batchSize") /
                        _PM->get<unsigned long long>("dataLen")),
                (unsigned long long) initializeSampleCount);
    }

    return initializeSampleCount;
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *RTIDDSImpl<T>::CreateWriter(const std::string &topic_name)
{
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    std::string qos_profile = "";
    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        throw std::logic_error("[Error] Topic name");
    }

    dds::pub::qos::DataWriterQos dw_qos = setup_DW_QoS(qos_profile, topic_name);

    if (!_PM->get<bool>("dynamicData")) {
        dds::topic::Topic<T> topic(_participant, topic_name);
        dds::pub::DataWriter<T> writer(_publisher, topic, dw_qos);

        return new RTIPublisher<T>(
                writer,
                _PM->get<long>("instances"),
                _pongSemaphore,
                _PM->get<bool>("latencyTest"),
                _PM->get<long>("writeInstance"),
                _PM);

    } else {
        const dds::core::xtypes::StructType& type =
                rti::topic::dynamic_type<T>::get();
        dds::topic::Topic<DynamicData> topic(
                _participant,
                topic_name,
                type);
        dds::pub::DataWriter<DynamicData> writer(
                _publisher,
                topic,
                dw_qos);

        return new RTIDynamicDataPublisher(
                writer,
                _PM->get<long>("instances"),
                _pongSemaphore,
                _PM->get<bool>("latencyTest"),
                _PM->get<long>("writeInstance"),
                type,
                _PM);
    }
}
/*********************************************************
 * CreateCFT
 * The CFT allows to the subscriber to receive a specific instance or a range of them.
 * In order generate the CFT it is necessary to create a condition:
 *      - In the case of a specific instance, it is necessary to convert to cftRange[0] into a key notation.
 *        Then it is enough with check that every element of key is equal to the instance.
 *        Example: cftRange[0] = 300. condition ="(0 = key[0] AND 0 = key[1] AND 1 = key[2] AND  44 = key[3])"
 *          So, in the case that the key = { 0, 0, 1, 44}, it will be received.
 *      - In the case of a range of instances, it is necessary to convert to cftRange[0] and cftRange[1] into a key notation.
 *        Then it is enough with check that the key is in the range of instances.
 *        Example: cftRange[1] = 300 and cftRange[1] = 1.
 *          condition = ""
 *              "("
 *                  "("
 *                      "(44 < key[3]) OR"
 *                      "(44 <= key[3] AND 1 < key[2]) OR"
 *                      "(44 <= key[3] AND 1 <= key[2] AND 0 < key[1]) OR"
 *                      "(44 <= key[3] AND 1 <= key[2] AND 0 <= key[1] AND 0 <= key[0])"
 *                  ") AND ("
 *                      "(1 > key[3]) OR"
 *                      "(1 >= key[3] AND 0 > key[2]) OR"
 *                      "(1 >= key[3] AND 0 >= key[2] AND 0 > key[1]) OR"
 *                      "(1 >= key[3] AND 0 >= key[2] AND 0 >= key[1] AND 0 >= key[0])"
 *                  ")"
 *              ")"
 *          The main goal for comaparing a instances and a key is by analyze the elemetns by more significant to the lest significant.
 *          So, in the case that the key is between [ {0, 0, 0, 1} and { 0, 0, 1, 44} ], it will be received.
 *  Beside, there is a special case where all the subscribers will receive the samples, it is MAX_CFT_VALUE = 65535 = [255,255,0,0,]
 */
template <typename T>
template <typename U>
dds::topic::ContentFilteredTopic<U> RTIDDSImpl<T>::CreateCft(
        const std::string &topic_name,
        const dds::topic::Topic<U> &topic) {
    std::string condition;
    std::vector<std::string> parameters(2 * KEY_SIZE);
    const std::vector<unsigned long long> cftRange
            = _PM->get_vector<unsigned long long>("cft");

    /* Only one element, no range */
    if (cftRange.size() == 1) {  // If same elements, no range
        std::cerr << "[Info] CFT enabled for instance: '"
                  << cftRange[0]
                  << "'"
                  <<std::endl;

        for (int i = 0; i < KEY_SIZE; i++) {
            std::ostringstream string_stream_object;
            string_stream_object << (int)((unsigned char)(cftRange[0] >> i * 8));
            parameters[i] = string_stream_object.str();
        }

        condition = "(%0 = key[0] AND  %1 = key[1] AND %2 = key[2] AND  %3 = key[3]) OR "
                    "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";

    } else { /* More than one element, apply a range filter */
        std::cerr << "[Info] CFT enabled for instance range:["
                  << cftRange[0]
                  << ","
                  << cftRange[1]
                  << "]"
                  << std::endl;

        for (int i = 0; i < 2 * KEY_SIZE; i++) {
            std::ostringstream string_stream_object;
            string_stream_object << (int)((unsigned char)
                    (cftRange[ i < KEY_SIZE? 0 : 1] >> (i % KEY_SIZE) * 8));
            parameters[i]= string_stream_object.str();
        }

        condition = ""
                "("
                    "("
                        "(%3 < key[3]) OR"
                        "(%3 <= key[3] AND %2 < key[2]) OR"
                        "(%3 <= key[3] AND %2 <= key[2] AND %1 < key[1]) OR"
                        "(%3 <= key[3] AND %2 <= key[2] AND %1 <= key[1] AND %0 <= key[0])"
                    ") AND ("
                        "(%7 > key[3]) OR"
                        "(%7 >= key[3] AND %6 > key[2]) OR"
                        "(%7 >= key[3] AND %6 >= key[2] AND %5 > key[1]) OR"
                        "(%7 >= key[3] AND %6 >= key[2] AND %5 >= key[1] AND %4 >= key[0])"
                    ") OR ("
                        "255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3]"
                    ")"
                ")";
    }

    return dds::topic::ContentFilteredTopic<U> (
            topic, topic_name,
            dds::topic::Filter(
                (const std::string)condition, parameters)
    );
}

/*********************************************************
 * CreateReader
 */
template <typename T>
IMessagingReader *RTIDDSImpl<T>::CreateReader(
        const std::string &topic_name,
        IMessagingCB *callback)
{
    std::string qos_profile;
    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        throw std::logic_error("[Error] Topic name");
    }

    dds::sub::qos::DataReaderQos dr_qos = setup_DR_QoS(qos_profile, topic_name);

    if (!_PM->get<bool>("dynamicData")) {
        dds::topic::Topic<T> topic(_participant, topic_name);
        dds::sub::DataReader<T> reader(dds::core::null);
        ReceiverListener<T> *reader_listener = NULL;

        if (topic_name == THROUGHPUT_TOPIC_NAME.c_str() && _PM->is_set("cft")) {
            /* Create CFT Topic */
            dds::topic::ContentFilteredTopic<T> topicCft = CreateCft(
                    topic_name,
                    topic);
            if (callback != NULL) {
                reader_listener = new ReceiverListener<T>(callback);
                reader = dds::sub::DataReader<T>(
                        _subscriber,
                        topicCft,
                        dr_qos,
                        reader_listener,
                        dds::core::status::StatusMask::data_available());
            } else {
                reader = dds::sub::DataReader<T>(_subscriber, topicCft, dr_qos);
            }
        } else {
            if (callback != NULL) {
                reader_listener = new ReceiverListener<T>(callback);
                reader = dds::sub::DataReader<T>(
                        _subscriber,
                        topic,
                        dr_qos,
                        reader_listener,
                        dds::core::status::StatusMask::data_available());
            } else {
                reader = dds::sub::DataReader<T>(_subscriber, topic, dr_qos);
            }
        }

        return new RTISubscriber<T>(
                reader,
                reader_listener,
                _PM);

    } else {
        const dds::core::xtypes::StructType& type =
                rti::topic::dynamic_type<T>::get();
        dds::topic::Topic<DynamicData> topic(
                _participant,
                topic_name,
                type);
        dds::sub::DataReader<DynamicData> reader(dds::core::null);
        DynamicDataReceiverListener *dynamic_data_reader_listener = NULL;
        if (topic_name == THROUGHPUT_TOPIC_NAME.c_str() && _PM->is_set("cft")) {
            /* Create CFT Topic */
            dds::topic::ContentFilteredTopic<DynamicData> topicCft = CreateCft(
                    topic_name,
                    topic);
            if (callback != NULL) {
                dynamic_data_reader_listener =
                        new DynamicDataReceiverListener(callback);
                reader = dds::sub::DataReader<DynamicData>(
                        _subscriber,
                        topicCft,
                        dr_qos,
                        dynamic_data_reader_listener,
                        dds::core::status::StatusMask::data_available());
            } else {
                reader = dds::sub::DataReader<DynamicData>(
                        _subscriber,
                        topicCft,
                        dr_qos);
            }
        } else {
            if (callback != NULL) {
                dynamic_data_reader_listener =
                        new DynamicDataReceiverListener(callback);
                reader = dds::sub::DataReader<DynamicData>(
                        _subscriber,
                        topic,
                        dr_qos,
                        dynamic_data_reader_listener,
                        dds::core::status::StatusMask::data_available());
            } else {
                reader = dds::sub::DataReader<DynamicData>(
                        _subscriber,
                        topic,
                        dr_qos);
            }
        }

        return new RTIDynamicDataSubscriber(
                reader,
                dynamic_data_reader_listener,
                _PM);
    }
}

template <typename T>
const std::string RTIDDSImpl<T>::get_qos_profile_name(std::string topicName)
{
    if (_qoSProfileNameMap[topicName].empty()) {
        std::cerr << "topic name must either be %s or %s or %s.\n"
                  << THROUGHPUT_TOPIC_NAME << " or "
                  << LATENCY_TOPIC_NAME << " or "
                  << ANNOUNCEMENT_TOPIC_NAME << "."
                  << std::endl;
    }

    /* If the topic name dont match any key return a empty string */
    return _qoSProfileNameMap[topicName];
}

#ifndef RTI_MICRO
template <typename T>
unsigned long int RTIDDSImpl<T>::getShmemSHMMAX() {
    unsigned long int shmmax = 0;

  #ifdef RTI_DARWIN
    shmmax = MAX_DARWIN_SHMEM_SIZE;
    const char *cmd = "sysctl kern.sysv.shmmax";
    int buffSize = 100;
    char buffer[buffSize];
    FILE *file = NULL;

    // Execute cmd and get file pointer
    if ((file = popen(cmd, "r")) == NULL) {
        std::cerr << "Could not run cmd '" << cmd << "'. "
                  << "Using default size: " << shmmax << " bytes." << std::endl;
        return shmmax;
    }

    // Read cmd output from its file pointer
    if (fgets(buffer, buffSize, file) == NULL) {
        std::cerr << "Could not read '" << cmd << "' output. "
                  << "Using default size: " << shmmax << " bytes." << std::endl;
        return shmmax;
    }

    // Split cmd output by blankspaces and get second position
    strtok(buffer, " ");
    char *size = strtok(NULL, " ");
    shmmax = atoi(size);

    // Close file and process
    pclose(file);
  #else
    // NOT IMPLEMENTED OR NEEDED (YET)
  #endif

    return shmmax;
}
#endif // !RTI_MICRO

template <typename T>
dds::sub::qos::DataReaderQos RTIDDSImpl<T>::setup_DR_QoS(
        std::string qos_profile, std::string topic_name) {
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    dds::core::QosProvider qos_provider = getQosProviderForProfile(
       _PM->get<std::string>("qosLibrary"),
        qos_profile
    );

    dds::sub::qos::DataReaderQos dr_qos = qos_provider.datareader_qos();
    Reliability qos_reliability = dr_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dr_qos.policy<ResourceLimits>();
    DataReaderResourceLimits qos_dr_resource_limits = dr_qos.policy<DataReaderResourceLimits>();
    Durability qos_durability = dr_qos.policy<Durability>();
    rti::core::policy::DataReaderProtocol dr_DataReaderProtocol =
            dr_qos.policy<rti::core::policy::DataReaderProtocol>();

    // This will allow us to load some properties.
    std::map<std::string, std::string> properties =
            dr_qos.policy<Property>().get_all();

    // only force reliability on throughput/latency topics
    if (topic_name != ANNOUNCEMENT_TOPIC_NAME.c_str()) {
        if (!_PM->get<bool>("bestEffort")) {
            qos_reliability = dds::core::policy::Reliability::Reliable();
        } else {
            qos_reliability = dds::core::policy::Reliability::BestEffort();
        }
    }

    if (_PM->get<bool>("noPositiveAcks")
            && (qos_profile == "ThroughputQos"
            || qos_profile == "LatencyQos")) {
        dr_DataReaderProtocol.disable_positive_acks(true);
    }

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos") {

        if (_PM->get<int>("durability") == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }

        qos_durability->direct_communication(
                !_PM->get<bool>("noDirectCommunication"));
    }

    if ((qos_profile == "LatencyQos")
            && _PM->get<bool>("noDirectCommunication")
            && (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
            || _PM->get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS)) {

        if (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(
                !_PM->get<bool>("noDirectCommunication"));
    }

    qos_resource_limits->initial_instances(_PM->get<long>("instances") + 1);
    if (_InstanceMaxCountReader != dds::core::LENGTH_UNLIMITED) {
        _InstanceMaxCountReader++;
    }
    qos_resource_limits->max_instances(_InstanceMaxCountReader);

    if (_PM->get<long>("instances") > 1) {
        if (_PM->get<long>("instanceHashBuckets") > 0) {
            qos_resource_limits->instance_hash_buckets(
                    _PM->get<long>("instanceHashBuckets"));
        } else {
            qos_resource_limits->instance_hash_buckets(
                    _PM->get<long>("instances"));
        }
    }

    if (_PM->get<bool>("multicast") && _transport.allowsMulticast()) {
        dds::core::StringSeq transports;
        transports.push_back("udpv4");
        std::string multicastAddr =
                _transport.getMulticastAddr(topic_name.c_str());
        if (multicastAddr.length() == 0) {
            std::cerr << "[Error] Topic name must either be "
                      << THROUGHPUT_TOPIC_NAME << " or "
                      << LATENCY_TOPIC_NAME << " or "
                      << ANNOUNCEMENT_TOPIC_NAME << std::endl;
            throw std::logic_error("[Error] Topic name");
        }
        rti::core::TransportMulticastSettings multicast_settings(
                transports,
                _transport.getMulticastAddr(topic_name.c_str()),
                0);
        rti::core::TransportMulticastSettingsSeq multicast_seq;
        multicast_seq.push_back(multicast_settings);

        dr_qos << rti::core::policy::TransportMulticast(multicast_seq,
                rti::core::policy::TransportMulticastKind::AUTOMATIC);
    }

    if (_PM->get<int>("unbounded") > 0 && !_isFlatData) {
        char buf[10];
        sprintf(buf, "%d", _PM->get<int>("unbounded"));
        properties["dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size"] = buf;
    }

    #ifdef RTI_FLATDATA_AVAILABLE
    if (_isFlatData) {
        properties["dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size"] =
                std::to_string(dds::core::LENGTH_UNLIMITED);

        if (_isLargeData) {
            int max_allocable_space = MAX_PERFTEST_SAMPLE_SIZE;

            unsigned long long initial_samples = (std::max)(
                    1, max_allocable_space / RTI_FLATDATA_MAX_SIZE);

            initial_samples = (std::min)(
                    initial_samples,
                    (unsigned long long) qos_resource_limits->initial_samples());

            qos_resource_limits->initial_samples(initial_samples);

            /**
             * Since for ZeroCopy we are sending small data (16B reference),
             * we do not need these settings
             */
            if (!_isZeroCopy) {
                qos_resource_limits->max_samples(initial_samples);
                qos_resource_limits->max_samples_per_instance(
                        initial_samples);
                qos_dr_resource_limits.max_samples_per_remote_writer(
                        initial_samples);
            }
        }
    }
    #endif

    /**
     * Configure DataReader to prevent dynamic allocation of
     * buffer used for storing received fragments
     */
    if (_PM->get<bool>("preallocateFragmentedSamples")) {
        qos_dr_resource_limits.initial_fragmented_samples(1);
        qos_dr_resource_limits.dynamically_allocate_fragmented_samples(
                    DDS_BOOLEAN_FALSE);
    }

    dr_qos << qos_reliability;
    dr_qos << qos_resource_limits;
    dr_qos << qos_durability;
    dr_qos << dr_DataReaderProtocol;
    dr_qos << qos_dr_resource_limits;
    dr_qos << Property(properties.begin(), properties.end());

    return dr_qos;
}

template <typename T>
dds::pub::qos::DataWriterQos RTIDDSImpl<T>::setup_DW_QoS(
        std::string qos_profile, std::string topic_name) {
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    dds::core::QosProvider qos_provider = getQosProviderForProfile(
            _PM->get<std::string>("qosLibrary"),
            qos_profile);

    dds::pub::qos::DataWriterQos dw_qos = qos_provider.datawriter_qos();
    Reliability qos_reliability = dw_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dw_qos.policy<ResourceLimits>();
    DataWriterResourceLimits qos_dw_resource_limits =
            dw_qos.policy<DataWriterResourceLimits>();
    Durability qos_durability = dw_qos.policy<Durability>();
    PublishMode dwPublishMode= dw_qos.policy<PublishMode>();
    Batch dwBatch = dw_qos.policy<Batch>();
    rti::core::policy::DataWriterProtocol dw_dataWriterProtocol =
            dw_qos.policy<rti::core::policy::DataWriterProtocol>();
    RtpsReliableWriterProtocol dw_reliableWriterProtocol =
            dw_dataWriterProtocol.rtps_reliable_writer();

    // This will allow us to load some properties.
    std::map<std::string, std::string> properties =
            dw_qos.policy<Property>().get_all();

    if (_PM->get<bool>("noPositiveAcks")
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dw_dataWriterProtocol.disable_positive_acks(true);
        if (_PM->is_set("keepDurationUsec")) {
            dw_reliableWriterProtocol
                    .disable_positive_acks_min_sample_keep_duration(
                            dds::core::Duration::from_microsecs(
                                    _PM->get<unsigned long long>(
                                            "keepDurationUsec")));
        }
    }

    if ((_isLargeData && !_isZeroCopy) || _PM->get<bool>("asynchronous")) {
        if (_PM->get<std::string>("flowController") != "default") {
            dwPublishMode = PublishMode::Asynchronous(
                    "dds.flow_controller.token_bucket."
                    + _PM->get<std::string>("flowController"));
       } else{
           dwPublishMode = PublishMode::Asynchronous();
       }
   }

    // Only force reliability on throughput/latency topics
    if (topic_name != ANNOUNCEMENT_TOPIC_NAME.c_str()) {
        if (!_PM->get<bool>("bestEffort")) {
            // default: use the setting specified in the qos profile
            // qos_reliability = Reliability::Reliable(dds::core::Duration::infinite());
        } else {
            // override to best-effort
            qos_reliability = Reliability::BestEffort();
        }
    }

    // These QOS's are only set for the Throughput datawriter
    if (qos_profile == "ThroughputQos") {

        if (_PM->get<bool>("multicast")) {
            dw_reliableWriterProtocol.enable_multicast_periodic_heartbeat(true);
        }

        if (_PM->get<long>("batchSize") > 0) {
            dwBatch.enable(true);
            dwBatch.max_data_bytes(_PM->get<long>("batchSize"));
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_PM->get<int>("sendQueueSize"));
        } else {
            qos_resource_limits.max_samples(_PM->get<int>("sendQueueSize"));
        }

        if (_PM->get<bool>("enableAutoThrottle")) {
            properties["dds.data_writer.auto_throttle.enable"] = "true";
        }

        if (_PM->get<bool>("enableTurboMode")) {
            properties["dds.data_writer.enable_turbo_mode.enable"] = "true";
            dwBatch.enable(false);
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_PM->get<int>("sendQueueSize"));
        }

        qos_resource_limits->initial_samples(_PM->get<int>("sendQueueSize"));
        qos_resource_limits.max_samples_per_instance(qos_resource_limits.max_samples());
        this->_sendQueueSize = qos_resource_limits->initial_samples();

        if (_PM->get<int>("durability") == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(
                !_PM->get<bool>("noDirectCommunication"));

        dw_reliableWriterProtocol.heartbeats_per_max_samples(
                _PM->get<int>("sendQueueSize") / 10);
        dw_reliableWriterProtocol.low_watermark(
                _PM->get<int>("sendQueueSize") * 1 / 10);
        dw_reliableWriterProtocol.high_watermark(
                _PM->get<int>("sendQueueSize") * 9 / 10);

        /*
         * If _SendQueueSize is 1 low watermark and high watermark would both be
         * 0, which would cause the middleware to fail. So instead we set the
         * high watermark to the low watermark + 1 in such case.
         */
        if (dw_reliableWriterProtocol.high_watermark()
                == dw_reliableWriterProtocol.high_watermark()) {
            dw_reliableWriterProtocol.high_watermark(
                    dw_reliableWriterProtocol.high_watermark() + 1);
        }

        dw_reliableWriterProtocol.max_send_window_size(
                _PM->get<int>("sendQueueSize"));
        dw_reliableWriterProtocol.min_send_window_size(
                _PM->get<int>("sendQueueSize"));
    }

    if (qos_profile == "LatencyQos"
            && _PM->get<bool>("noDirectCommunication")
            && (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS
            || _PM->get<int>("durability") == DDS_PERSISTENT_DURABILITY_QOS)) {
        if (_PM->get<int>("durability") == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(
                !_PM->get<bool>("noDirectCommunication"));
    }

    qos_resource_limits.max_instances(_PM->get<long>("instances") + 1); // One extra for MAX_CFT_VALUE
    qos_resource_limits->initial_instances(_PM->get<long>("instances") + 1);

    if (_PM->get<long>("instances") > 1) {
        if (_PM->is_set("instanceHashBuckets")) {
            qos_resource_limits->instance_hash_buckets(
                    _PM->get<long>("instanceHashBuckets"));
        } else {
            qos_resource_limits->instance_hash_buckets(
                    _PM->get<long>("instances"));
        }
    }

    // If is LargeData.
    if (_PM->get<int>("unbounded") > 0) {
        char buf[10];
        sprintf(buf, "%d", (_isFlatData
                ? DDS_LENGTH_UNLIMITED // No dynamic alloc of serialize buffer
                : _PM->get<int>("unbounded")));
        properties["dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size"] =
                buf;
    }

    #ifdef RTI_FLATDATA_AVAILABLE
        if (_isFlatData) {
            /**
             *  If FlatData and LargeData, automatically estimate
             *  initial_samples here in a range from 1 up to the initial samples
             *  specifies in the QoS file
             *
             *  This is done to avoid using too much memory since DDS allocates
             *  samples of the RTI_FLATDATA_MAX_SIZE size
             */
            if (_isLargeData) {
                unsigned long max_allocable_space = MAX_PERFTEST_SAMPLE_SIZE;

              #ifdef RTI_DARWIN
                /**
                 * In OSX, we might not be able to allocate all the send queue
                 * samples We only need this on the DW since it will allocate
                 * the samples on Zero Copy
                 */
                if (_isZeroCopy) {
                    max_allocable_space = getShmemSHMMAX();

                    /**
                     * Leave enought room for an sceneario of two participants:
                     *   - One Publisher with one DW (throughput topic)
                     *   - One Subscriber with two DW (Latency topic and Announcement)
                     */
                    max_allocable_space /= 3;

                    /**
                     * If we wont be able to allocate as many samples as we
                     * originally want, Display a message letting know the user
                     * how to increase SHMEM operative system settings
                     */
                    if (max_allocable_space < RTI_FLATDATA_MAX_SIZE *
                                qos_resource_limits->initial_samples() + 1) {

                        std::cout << "[Warn] Performace Degradation: Not enought "
                                  << "Shared Memory space available to allocate "
                                  << "intial samples. Consider increasing SHMMAX "
                                  << "parameter on your system settings or select "
                                  << "a different transport." << std::endl
                                  << "See https://community.rti.com/kb/what-are-possible-solutions-common-shared-memory-issues"
                                  << std::endl
                                  << "If you still run into this issue, consider"
                                  <<" cleaning your Shared Memory segments."
                                  << std::endl;
                    }
                }
              #endif

                // The writer_loaned_sample_allocation is initial_simples + 1
                unsigned long long initial_samples = (std::max)(
                        1ul,
                        (max_allocable_space -
                                RTI_FLATDATA_MAX_SIZE) / RTI_FLATDATA_MAX_SIZE);

                initial_samples = (std::min)(
                        initial_samples,
                        (unsigned long long) qos_resource_limits->initial_samples());

                qos_resource_limits->initial_samples(initial_samples);
                this->_sendQueueSize = initial_samples;

                if (_transport.transportConfig.kind == TRANSPORT_SHMEM
                        || _transport.transportConfig.kind == TRANSPORT_UDPv4_SHMEM) {
                    /**
                     * Replace previously set reduce limits by the new ones
                     * from the initial_samples size calculations
                     */
                    qos_resource_limits->max_samples(
                            qos_resource_limits->initial_samples());
                    qos_resource_limits->max_samples_per_instance(
                            qos_resource_limits->max_samples());
                    dw_reliableWriterProtocol.heartbeats_per_max_samples(
                            (std::max)(1.0, 0.1 * qos_resource_limits->max_samples()));
                    dw_reliableWriterProtocol.high_watermark(
                            0.9 * qos_resource_limits->max_samples());
                    dw_reliableWriterProtocol.low_watermark(
                            0.1 * qos_resource_limits->max_samples());
                }
            } else {
                /**
                 * Avoid losing samples on
                 * "DDS_DataWriter_get_loan_untypedI:ERROR: Out of resources for
                 * writer loaned samples" error on small data due to not having
                 * enought samples on the writer buffer where FlatData loans
                 * samples from.
                 */
                qos_dw_resource_limits.writer_loaned_sample_allocation()
                        .initial_count(
                                2 * qos_resource_limits->initial_samples());
                qos_dw_resource_limits.
                    writer_loaned_sample_allocation().max_count(
                            1 + qos_dw_resource_limits.
                                    writer_loaned_sample_allocation().initial_count());
            }

            /**
             * Enables a ZeroCopy DataWriter to send a special sequence number
             * as a part of its inline Qos. his sequence number is used by a
             * ZeroCopy DataReader to check for sample consistency.
             */
            if (_isZeroCopy) {
                dw_qos << DataWriterTransferMode::ShmemRefSettings(true);
            }
        }
    #endif

    dw_qos << qos_reliability;
    dw_qos << qos_resource_limits;
    dw_qos << qos_dw_resource_limits;
    dw_qos << qos_durability;
    dw_qos << dwPublishMode;
    dw_qos << dwBatch;
    dw_dataWriterProtocol.rtps_reliable_writer(dw_reliableWriterProtocol);
    dw_qos << dw_dataWriterProtocol;
    dw_qos << Property(properties.begin(), properties.end());

    return dw_qos;
}

#ifdef RTI_FLATDATA_AVAILABLE
template <typename T>
RTIDDSImpl_FlatData<T>::RTIDDSImpl_FlatData(bool isZeroCopy) {
    this->_isFlatData = true;
    this->_isZeroCopy = isZeroCopy;
}

/*********************************************************
 * CreateReader FlatData
 */
template <typename T>
IMessagingReader *RTIDDSImpl_FlatData<T>::CreateReader(
        const std::string &topic_name,
        IMessagingCB *callback)
{
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    std::string qos_profile = "";

    qos_profile = this->get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        throw std::logic_error("[Error] Topic name");
    }

    dds::sub::qos::DataReaderQos dr_qos = setup_DR_QoS(qos_profile, topic_name);

    dds::topic::Topic<T> topic(this->_participant, topic_name);
    dds::sub::DataReader<T> reader(dds::core::null);
    ReceiverListenerBase<T> *reader_listener = NULL;

    if (topic_name == THROUGHPUT_TOPIC_NAME.c_str() && this->_PM->is_set("cft")) {
        /* Create CFT Topic */
        dds::topic::ContentFilteredTopic<T> topicCft = CreateCft(
                topic_name,
                topic);
        if (callback != NULL) {

            reader_listener = new FlatDataReceiverListener<T>(
                    callback,
                    _PM->get<bool>("zerocopy"),
                    _PM->get<bool>("checkconsistency"));

            reader = dds::sub::DataReader<T>(
                    this->_subscriber,
                    topicCft,
                    dr_qos,
                    reader_listener,
                    dds::core::status::StatusMask::data_available());
        } else {
            reader = dds::sub::DataReader<T>(this->_subscriber, topicCft, dr_qos);
        }
    } else {

        if (callback != NULL) {

            reader_listener = new FlatDataReceiverListener<T>(
                    callback,
                    _PM->get<bool>("zerocopy"),
                    _PM->get<bool>("checkconsistency"));

            reader = dds::sub::DataReader<T>(
                    this->_subscriber,
                    topic,
                    dr_qos,
                    reader_listener,
                    dds::core::status::StatusMask::data_available());
        } else {
            reader = dds::sub::DataReader<T>(this->_subscriber, topic, dr_qos);
        }
    }

    return new RTIFlatDataSubscriber<T>(
            reader,
            reader_listener,
            this->_PM);
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *RTIDDSImpl_FlatData<T>::CreateWriter(const std::string &topic_name)
{
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    std::string qos_profile = "";

    qos_profile = this->get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        throw std::logic_error("[Error] Topic name");
    }

    dds::pub::qos::DataWriterQos dw_qos = this->setup_DW_QoS(qos_profile, topic_name);

    dds::topic::Topic<T> topic(this->_participant, topic_name);
    dds::pub::DataWriter<T> writer(this->_publisher, topic, dw_qos);

    return new RTIFlatDataPublisher<T>(
            writer,
            this->_PM->template get<long>("instances"),
            this->_pongSemaphore,
            this->_PM->template get<bool>("latencyTest"),
            this->_PM->template get<long>("writeInstance"),
            this->_PM);
}
#endif

template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;
template class RTIDDSImpl<TestDataKeyedLarge_t>;
template class RTIDDSImpl<TestDataLarge_t>;

#ifdef RTI_FLATDATA_AVAILABLE
  template class RTIDDSImpl_FlatData<TestDataKeyed_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestData_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataKeyedLarge_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataLarge_FlatData_t>;
  #ifdef RTI_ZEROCOPY_AVAILABLE
  template class RTIDDSImpl_FlatData<TestDataKeyed_ZeroCopy_w_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestData_ZeroCopy_w_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataKeyedLarge_ZeroCopy_w_FlatData_t>;
  template class RTIDDSImpl_FlatData<TestDataLarge_ZeroCopy_w_FlatData_t>;
  #endif
#endif // RTI_FLATDATA_AVAILABLE

#if defined RTI_WIN32 || defined(RTI_INTIME)
  #pragma warning(pop)
#endif
