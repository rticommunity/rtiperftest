/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "RTIDDSImpl.h"
#include "perftest_cpp.h"
#include "qos_string.h"

using dds::core::xtypes::DynamicData;

#if defined(RTI_WIN32)
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

std::string valid_flow_controller[] = {"default", "1Gbps", "10Gbps"};

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
   return membersId.at(key);
}

template <typename T>
RTIDDSImpl<T>::RTIDDSImpl():
        _SendQueueSize(50),
        _DataLen(100),
        _DomainID(1),
        _ProfileFile("perftest_qos_profiles.xml"),
        _TurboMode(false),
        _UseXmlQos(true),
        _AutoThrottle(false),
        _IsReliable(true),
        _IsMulticast(false),
        _BatchSize(DEFAULT_THROUGHPUT_BATCH_SIZE),
        _InstanceCount(1),
        _InstanceMaxCountReader(dds::core::LENGTH_UNLIMITED), //(-1)
        _InstanceHashBuckets(dds::core::LENGTH_UNLIMITED), //(-1)
        _Durability(0), // DDS_VOLATILE_DURABILITY_QOS;
        _DirectCommunication(true),
        _KeepDurationUsec(-1),
        _UsePositiveAcks(true),
        _LatencyTest(false),
        _IsDebug(false),
        _isLargeData(false),
        _isScan(false),
        _isDynamicData(false),
        _IsAsynchronous(false),
        _FlowControllerCustom("default"),
        _useUnbounded(0),
        _peer_host_count(0),
        _peer_host(dds::core::StringSeq(RTIPERFTEST_MAX_PEERS)),
        _useCft(false),
        _instancesToBeWritten(-1), // By default use round-robin (-1)
        _CFTRange(2),
      #ifdef RTI_SECURE_PERFTEST
        _secureUseSecure(false),
        _secureIsSigned(false),
        _secureIsDataEncrypted(false),
        _secureIsSMEncrypted(false),
        _secureIsDiscoveryEncrypted(false),
        _secureDebugLevel(-1),
      #endif
        _WaitsetEventCount(5),
        _WaitsetDelayUsec(100),
        _HeartbeatPeriod(dds::core::Duration::zero()),
        _FastHeartbeatPeriod(dds::core::Duration::zero()),
        _ProfileLibraryName("PerftestQosLibrary"),
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
    /* RTI Connext provides finalize_instance() method on
     * domain participant factory for people who want to release memory used
     * by the participant factory. Uncomment the following block of code for
     * clean destruction of the singleton. */
    /* _participant.finalize_participant_factory(); */

}

/*********************************************************
 * PrintCmdLineHelp
 */
template <typename T>
void RTIDDSImpl<T>::PrintCmdLineHelp() {
    std::string usage_string = std::string(
            "\t-sendQueueSize <number>       - Sets number of samples (or batches) in send\n") +
            "\t                                queue, default 50\n" +
            "\t-domain <ID>                  - RTI DDS Domain, default 1\n" +
            "\t-qosFile <filename>           - Name of XML file for DDS Qos profiles, \n"
            "\t                                default: perftest_qos_profiles.xml\n"
            "\t-qosLibrary <lib name>        - Name of QoS Library for DDS Qos profiles, \n"
            "\t                                default: PerftestQosLibrary\n"
            "\t-bestEffort                   - Run test in best effort mode, default reliable\n" +
            "\t-batchSize <bytes>            - Size in bytes of batched message, default 8kB\n" +
            "\t                                (Disabled for LatencyTest mode or if dataLen > 4kB)\n" +
            "\t-noPositiveAcks               - Disable use of positive acks in reliable \n" +
            "\t                                protocol, default use positive acks\n" +
            "\t-durability <0|1|2|3>         - Set durability QOS, 0 - volatile,\n" +
            "\t                                1 - transient local, 2 - transient, \n" +
            "\t                                3 - persistent, default 0\n" +
            "\t-dynamicData                  - Makes use of the Dynamic Data APIs instead\n" +
            "\t                                of using the generated types.\n" +
            "\t-noDirectCommunication        - Use brokered mode for persistent durability\n" +
            "\t-waitsetDelayUsec <usec>      - UseReadThread related. Allows you to\n" +
            "\t                                process incoming data in groups, based on the\n" +
            "\t                                time rather than individually. It can be used\n" +
            "\t                                combined with -waitsetEventCount,\n" +
            "\t                                default 100 usec\n" +
            "\t-waitsetEventCount <count>    - UseReadThread related. Allows you to\n" +
            "\t                                process incoming data in groups, based on the\n" +
            "\t                                number of samples rather than individually. It\n" +
            "\t                                can be used combined with -waitsetDelayUsec,\n" +
            "\t                                default 5\n" +
            "\t-enableAutoThrottle           - Enables the AutoThrottling feature in the\n" +
            "\t                                throughput DataWriter (pub)\n" +
            "\t-enableTurboMode              - Enables the TurboMode feature in the\n" +
            "\t                                throughput DataWriter (pub)\n" +
            "\t-noXmlQos                     - Skip loading the qos profiles from the xml\n" +
            "\t                                profile\n" +
            "\t-asynchronous                 - Use asynchronous writer\n" +
            "\t                                Default: Not set\n" +
            "\t-flowController <flow>        - In the case asynchronous writer use a specific flow controller.\n" +
            "\t                                There are several flow controller predefined:\n" +
            "\t                                ";
    for(unsigned int i=0; i < sizeof(valid_flow_controller)/sizeof(valid_flow_controller[0]); i++) {
        usage_string += "\"" + valid_flow_controller[i] + "\" ";
    }
    usage_string += std::string("\n") +
            "\t                                Default: \"default\" (If using asynchronous).\n" +
            "\t-peer <address>               - Adds a peer to the peer host address list.\n" +
            "\t                                This argument may be repeated to indicate multiple peers\n" +
            "\n";
  #ifdef RTI_SECURE_PERFTEST
    usage_string += std::string("\n") +
            "\t======================= SECURE Specific Options =======================\n\n" +
            "\t-secureEncryptDiscovery       - Encrypt discovery traffic\n" +
            "\t-secureSign                   - Sign (HMAC) discovery and user data\n" +
            "\t-secureEncryptData            - Encrypt topic (user) data\n" +
            "\t-secureEncryptSM              - Encrypt RTPS submessages\n" +
            "\t-secureGovernanceFile <file>  - Governance file. If specified, the authentication,\n" +
            "\t                                signing, and encryption arguments are ignored. The\n" +
            "\t                                governance document configuration will be used instead\n" +
            "\t                                Default: built using the secure options.\n" +
            "\t-securePermissionsFile <file> - Permissions file <optional>\n" +
            "\t                                Default: \"./resource/secure/signed_PerftestPermissionsSub.xml\"\n" +
            "\t-secureCertAuthority <file>   - Certificate authority file <optional>\n" +
            "\t                                Default: \"./resource/secure/cacert.pem\"\n" +
            "\t-secureCertFile <file>        - Certificate file <optional>\n" +
            "\t                                Default: \"./resource/secure/sub.pem\"\n" +
            "\t-securePrivateKey <file>      - Private key file <optional>\n" +
            "\t                                Default: \"./resource/secure/subkey.pem\"\n";
  #endif

    std::cerr << usage_string << std::endl;
}

/*********************************************************
 * ParseConfig
 */
template <typename T>
bool RTIDDSImpl<T>::ParseConfig(int argc, char *argv[])
{
    unsigned long minScanSize = MAX_PERFTEST_SAMPLE_SIZE;
    bool isBatchSizeProvided = false;
    int i;
    int sec = 0;
    unsigned int nanosec = 0;

    // now load everything else, command-line params override config file
    for (i = 0; i < argc; ++i) {
        if (IS_OPTION(argv[i], "-pub")) {
        } else if (IS_OPTION(argv[i], "-scan")) {
            _isScan = true;

            /*
             * Check if we have custom scan values. In such case we are just
             * interested in the minimum one.
             */
            if ((i != (argc-1)) && *argv[1+i] != '-') {
                ++i;
                unsigned long auxScan = 0;
                char *pch = NULL;
                pch = strtok (argv[i], ":");
                while (pch != NULL) {
                    if (sscanf(pch, "%lu", &auxScan) != 1) {
                        return false;
                    }
                    pch = strtok (NULL, ":");
                    if (auxScan < minScanSize) {
                        minScanSize = auxScan;
                    }
                }
            /*
             * If we do not specify any custom value for the -scan, we would
             * set minScanSize to the minimum size in the default set for -scan.
             */
            } else {
                minScanSize = 32;
            }
        } else if (IS_OPTION(argv[i], "-dataLen")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <length> after -dataLen\n"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < (unsigned long)perftest_cpp::OVERHEAD_BYTES) {
                std::cerr << "[Error] -dataLen must be >= "
                        << perftest_cpp::OVERHEAD_BYTES << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (_DataLen > (unsigned long)MAX_PERFTEST_SAMPLE_SIZE) {
                std::cerr << "[Error] -dataLen must be <= "
                        << MAX_PERFTEST_SAMPLE_SIZE << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            if (_useUnbounded == 0 && _DataLen > (unsigned long)MAX_BOUNDED_SEQ_SIZE) {
                _useUnbounded = (std::min)(
                        2 * _DataLen, (unsigned long)MAX_BOUNDED_SEQ_SIZE);
            }
        }
        else if (IS_OPTION(argv[i], "-unbounded")) {
            if ((i == (argc-1)) || *argv[i+1] == '-')
            {
                _useUnbounded = (std::min)(
                        2 * _DataLen, (unsigned long)MAX_BOUNDED_SEQ_SIZE);
            } else {
                ++i;
                _useUnbounded = strtol(argv[i], NULL, 10);

                if (_useUnbounded <  (unsigned long)perftest_cpp::OVERHEAD_BYTES)
                {
                    std::cerr << "[Error] -unbounded <allocation_threshold> must be >= "
                        << perftest_cpp::OVERHEAD_BYTES << std::endl;
                    throw std::logic_error("[Error] Error parsing commands");
                }
                if (_useUnbounded > (unsigned long)MAX_BOUNDED_SEQ_SIZE)
                {
                    std::cerr << "[Error] -unbounded <allocation_threshold> must be <= "
                        << MAX_BOUNDED_SEQ_SIZE << std::endl;
                    throw std::logic_error("[Error] Error parsing commands");
                }
            }
        } else if (IS_OPTION(argv[i], "-sendQueueSize")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <count> after -sendQueueSize"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _SendQueueSize = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-heartbeatPeriod")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <period> after -heartbeatPeriod"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i], "%d:%u", &sec, &nanosec) != 2) {
                std::cerr
                        << "[Error] -heartbeatPeriod value must have the format <sec>:<nanosec>"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (sec > 0 || nanosec > 0) {
                _HeartbeatPeriod = dds::core::Duration(sec, nanosec);
            }
        } else if (IS_OPTION(argv[i], "-fastHeartbeatPeriod")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr
                        << "[Error] Missing <period> after -fastHeartbeatPeriod"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i], "%d:%u", &sec, &nanosec) != 2) {
                std::cerr
                        << "[Error] -fastHeartbeatPeriod value must have the format <sec>:<nanosec>"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (sec > 0 || nanosec > 0) {
                _FastHeartbeatPeriod = dds::core::Duration(sec, nanosec);
            }
        } else if (IS_OPTION(argv[i], "-domain")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <id> after -domain" << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _DomainID = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-qosFile")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <filename> after -qosFile"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _ProfileFile = argv[i];
        } else if (IS_OPTION(argv[i], "-qosLibrary")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <library name> after -qosLibrary"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _ProfileLibraryName = argv[i];
        } else if (IS_OPTION(argv[i], "-bestEffort")) {
            _IsReliable = false;
        } else if (IS_OPTION(argv[i], "-durability")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <kind> after -durability"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _Durability = strtol(argv[i], NULL, 10);

            if ((_Durability < 0) || (_Durability > 3)) {
                std::cerr << "[Error] durability kind must be 0(volatile), "
                        "1(transient local), 2(transient), or 3(persistent)."
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-dynamicData")) {
            _isDynamicData = true;
        } else if (IS_OPTION(argv[i], "-noDirectCommunication")) {
            _DirectCommunication = false;
        } else if (IS_OPTION(argv[i], "-instances")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <count> after -instances"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _InstanceCount = strtol(argv[i], NULL, 10);
            _InstanceMaxCountReader = _InstanceCount;

            if (_InstanceCount <= 0) {
                std::cerr << "[Error] Instance count cannot be negative or zero"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-instanceHashBuckets")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr
                        << "[Error] Missing <count> after -instanceHashBuckets"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _InstanceHashBuckets = strtol(argv[i], NULL, 10);

            if (_InstanceHashBuckets <= 0 && _InstanceHashBuckets != -1) {
                std::cerr
                        << "[Error] Instance hash buckets cannot be negative or zero"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-batchSize")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <#bytes> after -batchSize"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _BatchSize = strtol(argv[i], NULL, 10);

            if (_BatchSize < 0 || _BatchSize > (int)MAX_SYNCHRONOUS_SIZE) {
                std::cerr << "[Error] Batch size '" << _BatchSize
                          << "' should be between [0," << MAX_SYNCHRONOUS_SIZE
                          << "]" << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            isBatchSizeProvided = true;
        } else if (IS_OPTION(argv[i], "-keepDurationUsec")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <usec> after -keepDurationUsec"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _KeepDurationUsec = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-noPositiveAcks")) {
            _UsePositiveAcks = false;
        } else if (IS_OPTION(argv[i], "-verbosity")) {
            errno = 0;
            int verbosityLevel = strtol(argv[++i], NULL, 10);

            if (errno) {
                fprintf(stderr, "Unexpected value after -verbosity\n");
                return false;
            }

            switch (verbosityLevel) {
                case 0: rti::config::Logger::instance().verbosity(
                            rti::config::Verbosity::SILENT);
                        std::cerr << "[Info]: Setting verbosity to SILENT"
                                  << std::endl;
                        break;
                case 1: rti::config::Logger::instance().verbosity(
                            rti::config::Verbosity::ERRORY);
                        std::cerr << "[Info]: Setting verbosity to EXCEPTION"
                                  << std::endl;
                        break;
                case 2: rti::config::Logger::instance().verbosity(
                            rti::config::Verbosity::WARNING);
                        std::cerr << "[Info]: Setting verbosity to WARNING"
                                  << std::endl;
                        break;
                case 3: rti::config::Logger::instance().verbosity(
                            rti::config::Verbosity::STATUS_ALL);
                        std::cerr << "[Info]: Setting verbosity to STATUS_ALL"
                                  << std::endl;
                        break;
                default: std::cerr << "[Info]: Invalid value for the verbosity"
                                   << " parameter. Using default value (1)"
                                   << std::endl;
                        break;
            }
        } else if (IS_OPTION(argv[i], "-waitsetDelayUsec")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "Missing <usec> after -waitsetDelayUsec"
                        << std::endl;
                ;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _WaitsetDelayUsec = (unsigned int) strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-waitsetEventCount")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "Missing <count> after -waitsetEventCount"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _WaitsetEventCount = strtol(argv[i], NULL, 10);
            if (_WaitsetEventCount < 0) {
                std::cerr << "waitset event count cannot be negative"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-latencyTest")) {
            _LatencyTest = true;
        } else if (IS_OPTION(argv[i], "-enableAutoThrottle")) {
            _AutoThrottle = true;
        } else if (IS_OPTION(argv[i], "-enableTurboMode")) {
            _TurboMode = true;
        } else if (IS_OPTION(argv[i], "-noXmlQos") ) {
            _UseXmlQos = false;
        }
        else if (IS_OPTION(argv[i], "-asynchronous") )
        {
            _IsAsynchronous = true;
        }
        else if (IS_OPTION(argv[i], "-flowController"))
        {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "Missing <flow Controller Name> after -flowController"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _FlowControllerCustom = argv[i];

            // verify if the flow controller name is correct, else use "default"
            bool valid_flow_control = false;
            for(unsigned int i=0; i < sizeof(valid_flow_controller)/sizeof(valid_flow_controller[0]); i++) {
                if (_FlowControllerCustom == valid_flow_controller[i]) {
                    valid_flow_control = true;
                }
            }

            if (!valid_flow_control)
            {
                std::cerr <<"Bad <flow> "<<_FlowControllerCustom <<" for custom flow controller"<< std::endl;
                _FlowControllerCustom = "default";// used default
            }
        }
        else if (IS_OPTION(argv[i], "-peer")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                std::cerr << "[Error] Missing <address> after -peer"<< std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            if (_peer_host_count +1 < RTIPERFTEST_MAX_PEERS) {
                _peer_host[_peer_host_count++] = argv[i];
            } else {
                std::cerr << "[Error] The maximum of -initial peers is " << RTIPERFTEST_MAX_PEERS << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-cft")) {
            _useCft = true;
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <start>:<end> after -cft" << std::endl;
                throw std::logic_error("Missing <start>:<end> after -cft");
            }

            if (strchr(argv[i],':') != NULL) { // In the case that there are 2 parameter
                unsigned int cftStart = 0;
                unsigned int cftEnd = 0;
                if (sscanf(argv[i],"%u:%u",&cftStart,&cftEnd) != 2) {
                    std::cerr << "[Error] -cft value must have the format <start>:<end>" << std::endl;
                    throw std::logic_error("[Error] Error parsing commands");
                }
                _CFTRange[0] = cftStart;
                _CFTRange[1] = cftEnd;
            } else {
                _CFTRange[0] = strtol(argv[i], NULL, 10);
                _CFTRange[1] = _CFTRange[0];
            }

            if (_CFTRange[0] > _CFTRange[1]) {
                std::cerr << "[Error]  -cft <start> value cannot be bigger than <end>" << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            if (_CFTRange[0] < 0 ||
                    _CFTRange[0] >= (unsigned int)MAX_CFT_VALUE ||
                    _CFTRange[1] < 0 ||
                    _CFTRange[1] >= (unsigned int)MAX_CFT_VALUE) {
                std::cerr << "[Error] -cft <start>:<end> values should be between [0,"
                        << MAX_CFT_VALUE << "]" << std::endl;
                return false;
            }
        } else if (IS_OPTION(argv[i], "-writeInstance")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                std::cerr << "Missing <number> after -writeInstance"<< std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _instancesToBeWritten = strtol(argv[i], NULL, 10);
        }
      #ifdef RTI_SECURE_PERFTEST
        else if (IS_OPTION(argv[i], "-secureSign")) {
            _secureIsSigned = true;
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureEncryptBoth")) {
            _secureIsDataEncrypted = true;
            _secureIsSMEncrypted = true;
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureEncryptData")) {
            _secureIsDataEncrypted = true;
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureEncryptSM")) {
            _secureIsSMEncrypted = true;
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureEncryptDiscovery")) {
            _secureIsDiscoveryEncrypted = true;
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureGovernanceFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <file> after -secureGovernanceFile"
                          << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _secureGovernanceFile = argv[i];
            std::cout << "[INFO] Authentication, encryption, signing arguments "
                         "will be ignored, and the values specified by the "
                         "Governance file will be used instead"
                      << std::endl;
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-securePermissionsFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <file> after -securePermissionsFile"
                          << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _securePermissionsFile = argv[i];
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureCertAuthority")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "Missing <file> after -secureCertAuthority"
                          << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _secureCertAuthorityFile = argv[i];
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureCertFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "Missing <file> after -secureCertFile"
                          << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _secureCertificateFile = argv[i];
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-securePrivateKey")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "Missing <file> after -securePrivateKey"
                          << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _securePrivateKeyFile = argv[i];
            _secureUseSecure = true;
        } else if (IS_OPTION(argv[i], "-secureLibrary")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "Missing <file> after -secureLibrary"
                          << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _secureLibrary = argv[i];
        } else if (IS_OPTION(argv[i], "-secureDebug")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                std::cerr << "Missing <level> after -secureDebug"
                          << std::endl;

                throw std::logic_error("[Error] Error parsing commands");
            }
            _secureDebugLevel = strtol(argv[i], NULL, 10);
        }
      #endif
    }

    /* Check if we need to enable Large Data. This works also for -scan */
    if (_PM->get<unsigned long long>("dataLen")
            > (unsigned long) (std::min)(
                    MAX_SYNCHRONOUS_SIZE,
                    MAX_BOUNDED_SEQ_SIZE)) {
        _isLargeData = true;
        if (_PM->get<int>("unbounded") == 0) {
            _PM->set("unbounded", MAX_BOUNDED_SEQ_SIZE);
        }
    } else { /* No Large Data */
        _PM->set("unbounded", 0);
        _isLargeData = false;
    }

    /* If we are using batching */
    if (_PM->get<long>("batchSize") > 0) {
        /* We will not use batching for a latency test */
        if (_PM->get<bool>("latencyTest")) {
            if (_PM->is_set("batchSize")) {
                fprintf(stderr, "Batching cannot be used in a Latency test.\n");
                return false;
            } else {
                _PM->set<long>("batchSize", 0);  // Disable Batching
            }
        }

        /* Check if using asynchronous */
        if (_PM->get<bool>("asynchronous")) {
            if (_PM->is_set("batchSize") && _PM->get<long>("batchSize") != 0) {
                fprintf(stderr,
                        "Batching cannot be used with asynchronous writing.\n");
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
                fprintf(stderr, "Batching cannot be used with Large Data.\n");
                return false;
            } else {
                _PM->set<long>("batchSize", -2);
            }
        } else if ((unsigned long) _BatchSize
                < _PM->get<unsigned long long>("dataLen") * 2) {
            /*
             * We don't want to use batching if the batch size is not large
             * enough to contain at least two samples (in this case we avoid the
             * checking at the middleware level).
             */
            if (_PM->is_set("batchSize") || _PM->is_set("scan")) {
                /*
                 * Batchsize disabled. A message will be print if _batchsize < 0 in
                 * perftest_cpp::PrintConfiguration()
                 */
                _PM->set<long>("batchSize", -1);
            }
            else {
                _PM->set<long>("batchSize", 0); // Disable Batching
            }
        }
    }

    if (_PM->get<bool>("enableTurboMode")) {
        if (_PM->get<bool>("asynchronous")) {
            std::cerr << "[Error] Turbo Mode cannot be used with asynchronous writing. "
                    << std::endl;
            return false;
        }
        if (_isLargeData) {
            std::cerr << "Turbo Mode disabled, using large data." << std::endl;
            _PM->set<bool>("enableTurboMode", false);
        }
    }

    // Manage _instancesToBeWritten
    if (_PM->is_set("writeInstance")) {
        if (_PM->get<long>("instances") < _PM->get<long>("writeInstance")) {
            std::cerr << "Specified '-WriteInstance' (" <<
                    _PM->get<long>("writeInstance") <<
                    ") invalid: Bigger than the number of instances (" <<
                    _PM->get<long>("instances") << ")." << std::endl;
            throw std::logic_error("[Error] Error parsing commands");
        }
    }

    if(!_transport.validate_input()) {
        throw std::logic_error("Failure validation the transport options.");
        return false;
    };

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
   if (_secureUseSecure) {
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
            perftest_cpp::MilliSleep(1000);
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

    inline bool send(TestMessage &message, bool isCftWildCardKey) {

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
            this->_writer.write(this->data,
                    this->_instance_handles[this->_num_instances]);
        }
        return true;
    }
};

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
          :
            RTIPublisherBase<DynamicData>(
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

    inline bool send(TestMessage &message, bool isCftWildCardKey) {
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
            this->_writer.write(this->data,
                    this->_instance_handles[this->_num_instances]);
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
            int _WaitsetEventCount,
            unsigned int _WaitsetDelayUsec,
            ParameterManager *PM) :
            _reader(reader),
            _readerListener(readerListener),
            _waitset(rti::core::cond::WaitSetProperty(
                    _PM->get<long>("waitsetEventCount"),
                    dds::core::Duration::from_microsecs((long)
                            _PM->get<unsigned long long>("waitsetDelayUsec")))),
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
            perftest_cpp::MilliSleep(1000);
        }
    }
};

template<typename T>
class RTISubscriber: public RTISubscriberBase<T> {

public:
    RTISubscriber(
            dds::sub::DataReader<T> reader,
            ReceiverListener<T> *readerListener,
            int _WaitsetEventCount,
            unsigned int _WaitsetDelayUsec,
            ParameterManager *PM)
          :
            RTISubscriberBase<T>(
                    reader,
                    readerListener,
                    _WaitsetEventCount,
                    _WaitsetDelayUsec,
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

class RTIDynamicDataSubscriber: public RTISubscriberBase<DynamicData> {


public:
    RTIDynamicDataSubscriber(
            dds::sub::DataReader<DynamicData> reader,
            DynamicDataReceiverListener *readerListener,
            int _WaitsetEventCount,
            unsigned int _WaitsetDelayUsec,
            ParameterManager *PM)
          :
            RTISubscriberBase<DynamicData>(
                    reader,
                    readerListener,
                    _WaitsetEventCount,
                    _WaitsetDelayUsec,
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
        std::map<std::string, std::string> &dpQosProperties) {

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
        _secureGovernanceFile = "./resource/secure/signed_PerftestGovernance_";
        if (_PM->get<bool>("secureEncryptDiscovery")) {
            _secureGovernanceFile += "Discovery";
        }

        if (_PM->get<bool>("secureSign")) {
            _secureGovernanceFile += "Sign";
        }

        if (_PM->get<bool>("secureEncryptData")
                && _PM->get<bool>("secureEncryptSM")) {
            _secureGovernanceFile += "EncryptBoth";
        } else if (_PM->get<bool>("secureEncryptData")) {
            _secureGovernanceFile += "EncryptData";
        } else if (_PM->get<bool>("secureEncryptSM")) {
            _secureGovernanceFile += "EncryptSubmessage";
        }

        _secureGovernanceFile += ".xml";

        dpQosProperties["com.rti.serv.secure.access_control.governance_file"] =
                _secureGovernanceFile;
    } else {
        dpQosProperties["com.rti.serv.secure.access_control.governance_file"] =
                _secureGovernanceFile;
    }

    // permissions file
    dpQosProperties["com.rti.serv.secure.access_control.permissions_file"] =
            _securePermissionsFile;

    // permissions authority file
    dpQosProperties["com.rti.serv.secure.access_control.permissions_authority_file"] =
            _secureCertAuthorityFile;

    // certificate authority
    dpQosProperties["com.rti.serv.secure.authentication.ca_file"] =
            _secureCertAuthorityFile;

    // public key
    dpQosProperties["com.rti.serv.secure.authentication.certificate_file"] =
            _secureCertificateFile;

    // private key
    dpQosProperties["com.rti.serv.secure.authentication.private_key_file"] =
            _securePrivateKeyFile;

    if (_secureDebugLevel != -1) {
        std::ostringstream string_stream_object;
        string_stream_object << _secureDebugLevel;
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

        if (_secureCertificateFile.empty()) {
            if (_PM->get<bool>("pub")) {
                _PM->set("secureCertFile", SECURE_CERTIFICATE_FILE_PUB);
            } else {
                _PM->set("secureCertFile", SECURE_CERTIFICATE_FILE_SUB);
            }
        }

        if (_secureCertAuthorityFile.empty()) {
            _PM->set("secureCertAuthority", SECURE_CERTAUTHORITY_FILE);
        }

        if (_securePermissionsFile.empty()) {
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
        stringStream << _secureGovernanceFile << "\n";
    }

    stringStream << "\tPermissions file: ";
    if (_PM->get<std::string>("securePermissionsFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _securePermissionsFile << "\n";
    }

    stringStream << "\tPrivate key file: ";
    if (_PM->get<std::string>("securePrivateKey").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _securePrivateKeyFile << "\n";
    }

    stringStream << "\tCertificate file: ";
    if (_PM->get<std::string>("secureCertFile").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureCertificateFile << "\n";
    }

    stringStream << "\tCertificate authority file: ";
    if (_PM->get<std::string>("secureCertAuthority").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureCertAuthorityFile << "\n";
    }

    stringStream << "\tPlugin library: ";
    if (_PM->get<std::string>("secureLibrary").empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureLibrary << "\n";
    }

    if (_PM->is_set("secureDebug")) {
        stringStream << "\tDebug level: " <<  _secureDebugLevel << "\n";
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

    if (_UseXmlQos) {
        qosProvider = dds::core::QosProvider(
                _ProfileFile,
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
bool RTIDDSImpl<T>::Initialize(ParameterManager &PM)
{
    using namespace rti::core::policy;
    // Assigne the ParameterManager
    _PM = &PM;
    _transport.initialize(_PM);

     //TODO:
    // if (!validate_input()) {
    //     return false;
    // }

    // setup the QOS profile file to be loaded
    dds::core::QosProvider qos_provider =
        getQosProviderForProfile( _ProfileLibraryName,"BaseProfileQos");
    dds::domain::qos::DomainParticipantQos qos = qos_provider.participant_qos();

    std::map<std::string, std::string> properties =
            qos.policy<Property>().get_all();

  #ifdef RTI_SECURE_PERFTEST
    if (_secureUseSecure) {
        validateSecureArgs();
        configureSecurePlugin(properties);
    }
  #endif

    Discovery qos_discovery = qos.policy<Discovery>(); //get all the Discovery
    // set initial peers and not use multicast
    if (!_PM->get_vector<std::string>("peer").empty()) {
        _peer_host.resize(_PM->get_vector<std::string>("peer").size());
        qos_discovery.initial_peers(_peer_host);
        qos_discovery.multicast_receive_addresses(dds::core::StringSeq());
    }

    if (!configureTransport(_transport, qos, properties, _PM)){
        return false;
    };

    if (_PM->get<bool>("enableAutoThrottle")) {
        properties["dds.domain_participant.auto_throttle.enable"] = "true";
    }

    qos << qos_discovery;
    //We have to copy the properties to the participant_qos object
    qos << rti::core::policy::Property(
            properties.begin(),
            properties.end(),
            true);

    DomainListener *listener = new DomainListener;

    // Creates the participant
    _participant = dds::domain::DomainParticipant(
            _PM->get<int>("domain"),
            qos,
            listener,
            dds::core::status::StatusMask::inconsistent_topic() |
            dds::core::status::StatusMask::offered_incompatible_qos() |
            dds::core::status::StatusMask::requested_incompatible_qos() );

    // Create the _publisher and _subscriber
    _publisher = dds::pub::Publisher(_participant, qos_provider.publisher_qos());

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
     * There is a minimum number of samples that we want to send no matter what
     * the conditions are:
     */
    unsigned long initializeSampleCount = 50;

    /*
     * If we are using reliable, the maximum burst of that we can send is limited
     * by max_send_window_size (or max samples, but we will assume this is not
     * the case for this). In such case we should send max_send_window_size
     * samples.
     *
     * If we are not using reliability this should not matter.
     */
    initializeSampleCount = (std::max)(
            initializeSampleCount,
            (unsigned long) _PM->get<int>("sendQueueSize"));

    /*
     * If we are using batching we need to take into account tha the Send Queue
     * will be per-batch, therefore for the number of samples:
     */
    if (_PM->get<long>("batchSize") > 0) {
        initializeSampleCount = (std::max)(
                _PM->get<int>("sendQueueSize") *
                        (_PM->get<long>("batchSize") /
                        _PM->get<unsigned long>("dataLen")),
                initializeSampleCount);
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

    dds::core::QosProvider qos_provider =
        getQosProviderForProfile(
                _PM->get<std::string>("qosLibrary"),
                _PM->get<std::string>("qosFile"));
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

    if (_isLargeData || _PM->get<bool>("asynchronous")) {
        if (_PM->get<std::string>("flowController") != "default") {
            dwPublishMode = PublishMode::Asynchronous(
                    "dds.flow_controller.token_bucket."
                    + _PM->get<std::string>("flowController"));
       } else{
           dwPublishMode = PublishMode::Asynchronous();
       }
   }

    // only force reliability on throughput/latency topics
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

        if (_HeartbeatPeriod.sec() > 0 || _HeartbeatPeriod.nanosec() > 0) {
            // set the heartbeat_period
            dw_reliableWriterProtocol.heartbeat_period(_HeartbeatPeriod);
            // make the late joiner heartbeat compatible
            dw_reliableWriterProtocol.late_joiner_heartbeat_period(_HeartbeatPeriod);
        }

        if (_FastHeartbeatPeriod.sec() > 0 || _FastHeartbeatPeriod.nanosec() > 0) {
            // set the fast_heartbeat_period
            dw_reliableWriterProtocol.fast_heartbeat_period(_FastHeartbeatPeriod);
        }

        if (_PM->get<bool>("enableAutoThrottle")) {
            properties["dds.data_writer.auto_throttle.enable"] = "true";
        }

        if (_TurboMode) {
            properties["dds.data_writer.enable_turbo_mode.enable"] = "true";
            dwBatch.enable(false);
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_PM->get<int>("sendQueueSize"));
        }

        qos_resource_limits->initial_samples(_PM->get<int>("sendQueueSize"));
        qos_resource_limits.max_samples_per_instance(qos_resource_limits.max_samples());

        if (_Durability == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);

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
            && !_DirectCommunication
            && (_Durability == DDS_TRANSIENT_DURABILITY_QOS
                    || _Durability == DDS_PERSISTENT_DURABILITY_QOS)) {
        if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    qos_resource_limits.max_instances(_InstanceCount + 1); // One extra for MAX_CFT_VALUE
    qos_resource_limits->initial_instances(_InstanceCount + 1);

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            qos_resource_limits->instance_hash_buckets(_InstanceHashBuckets);
        } else {
            qos_resource_limits->instance_hash_buckets(_InstanceCount);
        }
    }

    if (_useUnbounded > 0) {
        char buf[10];
        sprintf(buf, "%lu", _useUnbounded);
        properties["dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size"] = buf;
    }

    dw_qos << qos_reliability;
    dw_qos << qos_resource_limits;
    dw_qos << qos_dw_resource_limits;
    dw_qos << qos_durability;
    dw_qos << dwPublishMode;
    dw_qos << dwBatch;
    dw_dataWriterProtocol.rtps_reliable_writer(dw_reliableWriterProtocol);
    dw_qos << dw_dataWriterProtocol;
    dw_qos << Property(properties.begin(), properties.end(), true);

    if (!_isDynamicData) {
        dds::topic::Topic<T> topic(_participant, topic_name);
        dds::pub::DataWriter<T> writer(_publisher, topic, dw_qos);
        return new RTIPublisher<T>(
                writer,
                _InstanceCount,
                _pongSemaphore,
                _LatencyTest,
                _instancesToBeWritten,
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
                _InstanceCount,
                _pongSemaphore,
                _LatencyTest,
                _instancesToBeWritten,
                type,
                _PM);
    }
}

/*********************************************************
 * CreateCFT
 * The CFT allows to the subscriber to receive a specific instance or a range of them.
 * In order generate the CFT it is necesary to create a condition:
 *      - In the case of a specific instance, it is necesary to convert to _CFTRange[0] into a key notation.
 *        Then it is enought with check that every element of key is equal to the instance.
 *        Exmaple: _CFTRange[0] = 300. condition ="(0 = key[0] AND 0 = key[1] AND 1 = key[2] AND  44 = key[3])"
 *          So, in the case that the key = { 0, 0, 1, 44}, it will be received.
 *      - In the case of a range of instances, it is necesary to convert to _CFTRange[0] and _CFTRange[1] into a key notation.
 *        Then it is enought with check that the key is in the range of instances.
 *        Exmaple: _CFTRange[1] = 300 and _CFTRange[1] = 1.
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
    if (_CFTRange[0] == _CFTRange[1]) { // If same elements, no range
        std::cerr << "[Info] CFT enabled for instance: '" << _CFTRange[0] << "'" <<std::endl;
        for (int i = 0; i < KEY_SIZE; i++) {
            std::ostringstream string_stream_object;
            string_stream_object << (int)((unsigned char)(_CFTRange[0] >> i * 8));
            parameters[i] = string_stream_object.str();
        }
        condition = "(%0 = key[0] AND  %1 = key[1] AND %2 = key[2] AND  %3 = key[3]) OR "
                    "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";
    } else { // If range
        std::cerr << "[Info] CFT enabled for instance range:[" << _CFTRange[0] << ","  << _CFTRange[1] << "]" << std::endl;
        for (int i = 0; i < 2 * KEY_SIZE; i++) {
            std::ostringstream string_stream_object;
            if (i < KEY_SIZE) {
                string_stream_object << (int)((unsigned char)(_CFTRange[0] >> i * 8));
                parameters[i]= string_stream_object.str();
            } else { // KEY_SIZE < i < KEY_SIZE * 2
                string_stream_object << (int)((unsigned char)(_CFTRange[1] >> i * 8));
                parameters[i] = string_stream_object.str();
            }
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
    using namespace dds::core::policy;
    using namespace rti::core::policy;

    std::string qos_profile;
    qos_profile = get_qos_profile_name(topic_name);
    if (qos_profile.empty()) {
        throw std::logic_error("[Error] Topic name");
    }

    std::string lib_name(_ProfileLibraryName);
    dds::core::QosProvider qos_provider =
        getQosProviderForProfile(lib_name, qos_profile);
    dds::sub::qos::DataReaderQos dr_qos = qos_provider.datareader_qos();


    Reliability qos_reliability = dr_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dr_qos.policy<ResourceLimits>();
    Durability qos_durability = dr_qos.policy<Durability>();
    rti::core::policy::DataReaderProtocol dr_DataReaderProtocol =
            dr_qos.policy<rti::core::policy::DataReaderProtocol>();

    // This will allow us to load some properties.
    std::map<std::string, std::string> properties =
            dr_qos.policy<Property>().get_all();

    // only force reliability on throughput/latency topics
    if (topic_name != ANNOUNCEMENT_TOPIC_NAME.c_str()) {
        if (_IsReliable) {
            qos_reliability = dds::core::policy::Reliability::Reliable();
        } else {
            qos_reliability = dds::core::policy::Reliability::BestEffort();
        }
    }

    if (!_UsePositiveAcks
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dr_DataReaderProtocol.disable_positive_acks(true);
    }

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos") {

        if (_Durability == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    if ((qos_profile == "LatencyQos")
            && !_DirectCommunication
            && (_Durability == DDS_TRANSIENT_DURABILITY_QOS
                    || _Durability == DDS_PERSISTENT_DURABILITY_QOS)) {

        if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        }
        else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    qos_resource_limits->initial_instances(_InstanceCount + 1);
    if (_InstanceMaxCountReader != dds::core::LENGTH_UNLIMITED) {
        _InstanceMaxCountReader++;
    }
    qos_resource_limits->max_instances(_InstanceMaxCountReader);

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            qos_resource_limits->instance_hash_buckets(_InstanceHashBuckets);
        } else {
            qos_resource_limits->instance_hash_buckets(_InstanceCount);
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
            return NULL;
        }
        rti::core::TransportMulticastSettings multicast_settings(
                transports,
                _transport.getMulticastAddr(topic_name.c_str()), 0);
        rti::core::TransportMulticastSettingsSeq multicast_seq;
        multicast_seq.push_back(multicast_settings);

        dr_qos << rti::core::policy::TransportMulticast(multicast_seq,
                rti::core::policy::TransportMulticastKind::AUTOMATIC);
    }

    if (_useUnbounded > 0) {
        char buf[10];
        sprintf(buf, "%lu", _useUnbounded);
        properties["dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size"] = buf;
    }

    dr_qos << qos_reliability;
    dr_qos << qos_resource_limits;
    dr_qos << qos_durability;
    dr_qos << dr_DataReaderProtocol;
    dr_qos << Property(properties.begin(), properties.end(), true);

    if (!_isDynamicData) {
        dds::topic::Topic<T> topic(_participant, topic_name);
        dds::sub::DataReader<T> reader(dds::core::null);
        ReceiverListener<T> *reader_listener = NULL;

        if (topic_name == THROUGHPUT_TOPIC_NAME.c_str() && _useCft) {
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
                _WaitsetEventCount,
                _WaitsetDelayUsec,
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
        if (topic_name == THROUGHPUT_TOPIC_NAME.c_str() && _useCft) {
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
                _WaitsetEventCount,
                _WaitsetDelayUsec,
                _PM);
    }
}

template <typename T>
const std::string RTIDDSImpl<T>::get_qos_profile_name(std::string topicName)
{
    if (_qoSProfileNameMap[topicName].empty()) {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME.c_str(),
                LATENCY_TOPIC_NAME.c_str(),
                ANNOUNCEMENT_TOPIC_NAME.c_str());
    }

    /* If the topic name dont match any key return a empty string */
    return _qoSProfileNameMap[topicName];
}

template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;
template class RTIDDSImpl<TestDataKeyedLarge_t>;
template class RTIDDSImpl<TestDataLarge_t>;


#ifdef RTI_WIN32
  #pragma warning(pop)
#endif
