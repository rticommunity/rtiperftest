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

template <typename T>
RTIDDSImpl<T>::RTIDDSImpl():
        _SendQueueSize(50),
        _DataLen(100),
        _DomainID(1),
        _Nic(""),
        _ProfileFile("perftest_qos_profiles.xml"),
        _TurboMode(false),
        _UseXmlQos(true),
        _AutoThrottle(false),
        _IsReliable(true),
        _IsMulticast(false),
        _BatchSize(0),
        _InstanceCount(1),
        _InstanceMaxCountReader(dds::core::LENGTH_UNLIMITED), //(-1)
        _InstanceHashBuckets(dds::core::LENGTH_UNLIMITED), //(-1)
        _Durability(0), // DDS_VOLATILE_DURABILITY_QOS;
        _DirectCommunication(true),
        _KeepDurationUsec(1000),
        _UsePositiveAcks(true),
        _UseSharedMemory(false),
        _UseUdpv6(false),
        _LatencyTest(false),
        _UseTcpOnly(false),
        _IsDebug(false),
        _isLargeData(false),
        _isScan(false),
        _isPublisher(false),
        _isDynamicData(false),
        _IsAsynchronous(false),
        _FlowControllerCustom("default"),
        _useUnbounded(0),
        _peer_host_count(0),
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
        THROUGHPUT_MULTICAST_ADDR("239.255.1.1"),
        LATENCY_MULTICAST_ADDR("239.255.1.2"),
        ANNOUNCEMENT_MULTICAST_ADDR("239.255.1.100"),
        _ProfileLibraryName("PerftestQosLibrary"),
        _participant(dds::core::null),
        _subscriber(dds::core::null),
        _publisher(dds::core::null),
        _pongSemaphore(RTI_OSAPI_SEMAPHORE_KIND_BINARY,NULL)
    {}

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
    std::string usage_string =
            "\t-sendQueueSize <number> - Sets number of samples (or batches) in send\n"
            "\t                          queue, default 50\n"
            "\t-domain <ID>            - RTI DDS Domain, default 1\n"
            "\t-qosprofile <filename>  - Name of XML file for DDS Qos profiles, \n"
            "\t                          default perftest_qos_profiles.xml\n"
            "\t-nic <ipaddr>           - Use only the nic specified by <ipaddr>.\n"
            "\t                          If unspecificed, use all available interfaces\n"
            "\t-multicast              - Use multicast to send data, default not to\n"
            "\t                          use multicast\n"
            "\t-nomulticast            - Do not use multicast to send data (default)\n"
            "\t-multicastAddress <ipaddr>   - Multicast address to use for receiving \n"
            "\t                          latency/announcement (pub) or \n"
            "\t                          throughtput(sub) data.\n"
            "\t                          If unspecified: latency 239.255.1.2,\n"
            "\t                                          announcement 239.255.1.100,\n"
            "\t                                          throughput 239.255.1.1\n"
            "\t-bestEffort             - Run test in best effort mode, default reliable\n"
            "\t-batchSize <bytes>      - Size in bytes of batched message, default 0\n"
            "\t                          (no batching)\n"
            "\t-noPositiveAcks         - Disable use of positive acks in reliable \n"
            "\t                          protocol, default use positive acks\n"
            "\t-keepDurationUsec <usec> - Minimum time (us) to keep samples when\n"
            "\t                          positive acks are disabled, default 1000 us\n"
            "\t-enableSharedMemory     - Enable use of shared memory transport and \n"
            "\t                          disable all the other transports, default\n"
            "\t                          shared memory not enabled\n"
            "\t-enableUdpv6            - Enable use of the Udpv6 transport and \n"
            "\t                          disable all the other transports, default\n"
            "\t                          udpv6 not enabled\n"
            "\t-enableTcp              - Enable use of TCP transport and disable all\n"
            "\t                          the other transports, default do not use\n"
            "\t                          tcp transport\n"
            "\t-heartbeatPeriod <sec>:<nanosec>     - Sets the regular heartbeat period\n"
            "\t                          for throughput DataWriter, default 0:0\n"
            "\t                          (use XML QoS Profile value)\n"
            "\t-fastHeartbeatPeriod <sec>:<nanosec> - Sets the fast heartbeat period\n"
            "\t                          for the throughput DataWriter, default 0:0\n"
            "\t                          (use XML QoS Profile value)\n"
            "\t-durability <0|1|2|3>   - Set durability QOS, 0 - volatile,\n"
            "\t                          1 - transient local, 2 - transient, \n"
            "\t                          3 - persistent, default 0\n"
            "\t-dynamicData            - Makes use of the Dynamic Data APIs instead\n"
            "\t                          of using the generated types.\n"
            "\t-noDirectCommunication  - Use brokered mode for persistent durability\n"
            "\t-instanceHashBuckets <#count> - Number of hash buckets for instances.\n"
            "\t                          If unspecified, same as number of\n"
            "\t                          instances.\n"
            "\t-waitsetDelayUsec <usec>  - UseReadThread related. Allows you to\n"
            "\t                          process incoming data in groups, based on the\n"
            "\t                          time rather than individually. It can be used\n"
            "\t                          combined with -waitsetEventCount,\n"
            "\t                          default 100 usec\n"
            "\t-waitsetEventCount <count> - UseReadThread related. Allows you to\n"
            "\t                          process incoming data in groups, based on the\n"
            "\t                          number of samples rather than individually. It\n"
            "\t                          can be used combined with -waitsetDelayUsec,\n"
            "\t                          default 5\n"
            "\t-enableAutoThrottle     - Enables the AutoThrottling feature in the\n"
            "\t                          throughput DataWriter (pub)\n"
            "\t-enableTurboMode        - Enables the TurboMode feature in the\n"
            "\t                          throughput DataWriter (pub)\n"
            "\t-noXmlQos               - Skip loading the qos profiles from the xml\n"
            "\t                          profile\n"
            "\t-asynchronous           - Use asynchronous writer\n"
            "\t                          Default: Not set\n"
            "\t-flowController <flow>  - In the case asynchronous writer use a specific flow controller.\n"
            "\t                          There are several flow controller predefined:\n"
            "\t                          ";
    for(unsigned int i=0; i < sizeof(valid_flow_controller)/sizeof(valid_flow_controller[0]); i++) {
        usage_string+=valid_flow_controller[i] + " ";
    }
    usage_string +=
            "\n"
            "\t                          Default: set default\n"
            "\t-peer <address>          - Adds a peer to the peer host address list.\n"
            "\t                          This argument may be repeated to indicate multiple peers\n";
#ifdef RTI_SECURE_PERFTEST
    usage_string +=
            "\t-secureEncryptDiscovery       - Encrypt discovery traffic\n"
            "\t-secureSign                   - Sign (HMAC) discovery and user data\n"
            "\t-secureEncryptData            - Encrypt topic (user) data\n"
            "\t-secureEncryptSM              - Encrypt RTPS submessages\n"
            "\t-secureGovernanceFile <file>  - Governance file. If specified, the authentication,\n"
            "\t                                signing, and encryption arguments are ignored. The\n"
            "\t                                governance document configuration will be used instead\n"
            "\t                                Default: built using the secure options.\n"
            "\t-securePermissionsFile <file> - Permissions file <optional>\n"
            "\t                                Default: \"./resource/secure/signed_PerftestPermissionsSub.xml\"\n"
            "\t-secureCertAuthority <file>   - Certificate authority file <optional>\n"
            "\t                                Default: \"./resource/secure/cacert.pem\"\n"
            "\t-secureCertFile <file>        - Certificate file <optional>\n"
            "\t                                Default: \"./resource/secure/sub.pem\"\n"
            "\t-securePrivateKey <file>      - Private key file <optional>\n"
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
    int i;
    int sec = 0;
    unsigned int nanosec = 0;

    // now load everything else, command line params override config file
    for (i = 0; i < argc; ++i) {
        if (IS_OPTION(argv[i], "-pub")) {
            _isPublisher = true;
        } else if (IS_OPTION(argv[i], "-scan")) {
            _isScan = true;
        } else if (IS_OPTION(argv[i], "-dataLen")) {

            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <length> after -dataLen\n"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < perftest_cpp::OVERHEAD_BYTES) {
                std::cerr << "[Error] -dataLen must be >= "
                        << perftest_cpp::OVERHEAD_BYTES << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }

            if (_DataLen > MAX_PERFTEST_SAMPLE_SIZE) {
                std::cerr << "[Error] -dataLen must be <= "
                        << MAX_PERFTEST_SAMPLE_SIZE << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            if (_useUnbounded == 0 && _DataLen > MAX_BOUNDED_SEQ_SIZE) {
                _useUnbounded = MAX_BOUNDED_SEQ_SIZE;
            }
        }
        else if (IS_OPTION(argv[i], "-unbounded")) {
            if ((i == (argc-1)) || *argv[i+1] == '-')
            {
                _useUnbounded = MAX_BOUNDED_SEQ_SIZE;
            } else {
                ++i;
                _useUnbounded = strtol(argv[i], NULL, 10);

                if (_useUnbounded <  perftest_cpp::OVERHEAD_BYTES)
                {
                    std::cerr << "[Error] -unbounded <managerMemory> must be >="
                        << perftest_cpp::OVERHEAD_BYTES << std::endl;
                    throw std::logic_error("[Error] Error parsing commands");
                }
                if (_useUnbounded > MAX_PERFTEST_SAMPLE_SIZE)
                {
                    std::cerr << "[Error] -unbounded <managerMemory> must be <="
                        << MAX_PERFTEST_SAMPLE_SIZE << std::endl;
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
        } else if (IS_OPTION(argv[i], "-qosprofile")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <filename> after -qosprofile"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _ProfileFile = argv[i];
        } else if (IS_OPTION(argv[i], "-multicast")) {
            _IsMulticast = true;
        } else if (IS_OPTION(argv[i], "-nomulticast")) {
            _IsMulticast = false;
        } else if (IS_OPTION(argv[i], "-multicastAddress")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr
                        << "[Error] Missing <multicast address> after -multicastAddress"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            THROUGHPUT_MULTICAST_ADDR = argv[i];
            LATENCY_MULTICAST_ADDR = argv[i];
            ANNOUNCEMENT_MULTICAST_ADDR = argv[i];
        } else if (IS_OPTION(argv[i], "-nic")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <address> after -nic"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _Nic = argv[i];
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
            std::cerr << "[Info] Using Dynamic Data." << std::endl;
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

            if (_BatchSize < 0) {
                std::cerr << "[Error] Batch size cannot be negative"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        } else if (IS_OPTION(argv[i], "-keepDurationUsec")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                std::cerr << "[Error] Missing <usec> after -keepDurationUsec"
                        << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
            _KeepDurationUsec = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-noPositiveAcks")) {
            _UsePositiveAcks = false;
        } else if (IS_OPTION(argv[i], "-enableSharedMemory")) {
            if (_UseUdpv6) {
                std::cerr << "[Info] -useUdpv6 was already set, "
                          << "ignoring -enableSharedMemory"
                          << std::endl;
            } else if (_UseTcpOnly) {
                std::cerr << "[Info] -enableTcp was already set, "
                          << "ignoring -enableSharedMemory"
                          << std::endl;
            } else {
                _UseSharedMemory = true;
            }
        } else if (IS_OPTION(argv[i], "-enableUdpv6")){
            if (_UseSharedMemory) {
                std::cerr << "[Info] -enableSharedMemory was already set, "
                          << "ignoring -enableUdpv6"
                          << std::endl;
            } else if (_UseTcpOnly) {
                std::cerr << "[Info] -enableTcp was already set, "
                          << "ignoring -enableUdpv6"
                          << std::endl;
            } else {
                _UseUdpv6 = true;
            }
        } else if (IS_OPTION(argv[i], "-enableTcp")) {
            if (_UseSharedMemory) {
                std::cerr << "[Info] -enableSharedMemory was already set, "
                          << "ignoring -enableTcp"
                          << std::endl;
            } else if (_UseUdpv6) {
                std::cerr << "[Info] -useUdpv6 was already set, "
                          << "ignoring -enableTcp"
                          << std::endl;
            } else {
                _UseTcpOnly = true;
            }
        }
        else if (IS_OPTION(argv[i], "-verbosity")) {
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
            std::cerr << "[Info] Auto Throttling enabled. Automatically "
                    "adjusting the DataWriter\'s writing rate" << std::endl;
            _AutoThrottle = true;
        } else if (IS_OPTION(argv[i], "-enableTurboMode")) {
            _TurboMode = true;
        } else if (IS_OPTION(argv[i], "-noXmlQos") ) {
            _UseXmlQos = false;
            std::cerr << "[Info] Not using xml file for QoS." << std::endl;
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
                std::cerr <<"[Error] The maximun of -initial peers is "<< RTIPERFTEST_MAX_PEERS << std::endl;
                return false;
            }
      #ifdef RTI_SECURE_PERFTEST
        } else if (IS_OPTION(argv[i], "-secureSign")) {
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
      #endif
        } else {
            if (i > 0) {
                std::cerr << argv[i] << " not recognized" << std::endl;
                throw std::logic_error("[Error] Error parsing commands");
            }
        }
    }

    if (_DataLen > MAX_SYNCHRONOUS_SIZE) {
        if (_isScan) {
            std::cerr << "[Info] DataLen will be ignored since -scan is present."
                    << std::endl;
        } else {
            std::cerr << "[Info] Large data settings enabled (-dataLen > "
                    << MAX_SYNCHRONOUS_SIZE << ")." << std::endl;
            _isLargeData = true;
        }
    }
    if (_isLargeData && _BatchSize > 0) {
        std::cerr << "[Error] Batch cannot be enabled if using large data"
                << std::endl;
        throw std::logic_error("[Error] Error parsing commands");
    }

    if (_isLargeData && _TurboMode) {
        std::cerr << "[Error] Turbo Mode cannot be used with asynchronous writing. "
                "It will be ignored."  << std::endl;
        throw std::logic_error("[Error] Error parsing commands");
    }

    /*
     * We don't want to use batching if the sample is the same size as the batch
     * nor if the sample is bigger (in this case we avoid the checking in the
     * middleware).
     */
     if (_BatchSize > 0 && (unsigned)_BatchSize <= _DataLen) {
         std::cerr << "[Info] Batching dissabled: BatchSize (" << _BatchSize
                   << ") is equal or smaller than the sample size (" << _DataLen
                   << ")."  << std::endl;
         _BatchSize = 0;
     }

    return true;
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
    int _num_instances;
    unsigned long _instance_counter;
    dds::core::InstanceHandleSeq _instance_handles;
    bool _useSemaphore;
    rti::core::Semaphore& _pongSemaphore;

public:
    RTIPublisherBase(
            dds::pub::DataWriter<T> writer,
            int num_instances,
            rti::core::Semaphore& pongSemaphore,
            bool useSemaphore)
          :
            _writer(writer),
            _num_instances(num_instances),
            _instance_counter(0),
            _useSemaphore(useSemaphore),
            _pongSemaphore(pongSemaphore) {}

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
            bool useSemaphore)
          :
            RTIPublisherBase<T>(
                    writer,
                    num_instances,
                    pongSemaphore,
                    useSemaphore)
    {

        for (int i = 0; i < this->_num_instances; ++i) {
            this->data.key()[0] = (char) (i);
            this->data.key()[1] = (char) (i >> 8);
            this->data.key()[2] = (char) (i >> 16);
            this->data.key()[3] = (char) (i >> 24);

            this->_instance_handles.push_back(
                    this->_writer.register_instance(this->data));
        }
    }

    inline bool send(TestMessage &message) {

        this->data.entity_id(message.entity_id);
        this->data.seq_num(message.seq_num);
        this->data.timestamp_sec(message.timestamp_sec);
        this->data.timestamp_usec(message.timestamp_usec);
        this->data.latency_ping(message.latency_ping);

        this->data.bin_data().resize(message.size);
        //data.bin_data(message.data);

        int key = 0;
        if (this->_num_instances > 1) {
            key = this->_instance_counter++ % this->_num_instances;
            this->data.key()[0] = (char) (key);
            this->data.key()[1] = (char) (key >> 8);
            this->data.key()[2] = (char) (key >> 16);
            this->data.key()[3] = (char) (key >> 24);
        }

        this->_writer.write(this->data, this->_instance_handles[key]);
        return true;
    }
};

class RTIDynamicDataPublisher: public RTIPublisherBase<DynamicData> {

protected:
    DynamicData data;

public:
    RTIDynamicDataPublisher(
            dds::pub::DataWriter<DynamicData> writer,
            int num_instances,
            rti::core::Semaphore& pongSemaphore,
            bool useSemaphore,
            const dds::core::xtypes::StructType& typeCode)
          :
            RTIPublisherBase<DynamicData>(
                    writer,
                    num_instances,
                    pongSemaphore,
                    useSemaphore),
            data(typeCode)
    {

        for (int i = 0; i < this->_num_instances; ++i) {
            std::vector<uint8_t> key_octets;
            key_octets.push_back((uint8_t) (i));
            key_octets.push_back((uint8_t) (i >> 8));
            key_octets.push_back((uint8_t) (i >> 16));
            key_octets.push_back((uint8_t) (i >> 24));
            this->data.set_values("key",key_octets);

            this->_instance_handles.push_back(this->_writer.register_instance(this->data));
        }
    }

    inline bool send(TestMessage &message) {

        this->data.value("entity_id", message.entity_id);
        this->data.value("seq_num", message.seq_num);
        this->data.value("timestamp_sec", message.timestamp_sec);
        this->data.value("timestamp_usec", message.timestamp_usec);
        this->data.value("latency_ping", message.latency_ping);

        std::vector<uint8_t> octec_seq;
        octec_seq.resize(message.size);
        this->data.set_values("bin_data", octec_seq);

        int key = 0;
        if (this->_num_instances > 1) {
            std::vector<uint8_t> key_octets;
            key = this->_instance_counter++ % this->_num_instances;
            key_octets.push_back((uint8_t) (key));
            key_octets.push_back((uint8_t) (key >> 8));
            key_octets.push_back((uint8_t) (key >> 16));
            key_octets.push_back((uint8_t) (key >> 24));
            this->data.set_values("key", key_octets);
        }
        this->_writer.write(this->data, this->_instance_handles[key]);
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
                this->_message.entity_id =
                        sample.value<int32_t>("entity_id");
                this->_message.seq_num =
                        sample.value<uint32_t>("seq_num");
                this->_message.timestamp_sec =
                        sample.value<int32_t>("timestamp_sec");
                this->_message.timestamp_usec =
                        sample.value<uint32_t>("timestamp_usec");
                this->_message.latency_ping =
                        sample.value<int32_t>("latency_ping");
                this->_message.size =
                        (int)(sample.get_values<uint8_t>("bin_data")).size();

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

public:
    RTISubscriberBase(
            dds::sub::DataReader<T> reader,
            ReceiverListenerBase<T> *readerListener,
            int _WaitsetEventCount,
            unsigned int _WaitsetDelayUsec) :
            _reader(reader),
            _readerListener(readerListener),
            _waitset(rti::core::cond::WaitSetProperty(_WaitsetEventCount,
                    dds::core::Duration::from_microsecs(_WaitsetDelayUsec))) {
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
            unsigned int _WaitsetDelayUsec)
          :
            RTISubscriberBase<T>(
                    reader,
                    readerListener,
                    _WaitsetEventCount,
                    _WaitsetDelayUsec)
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
            unsigned int _WaitsetDelayUsec)
          :
            RTISubscriberBase<DynamicData>(reader,readerListener,_WaitsetEventCount,_WaitsetDelayUsec)
    {}

    TestMessage *ReceiveMessage() {

        int seq_length;

        while (true) {

            if (this->_no_data) {
                this->_waitset.wait(dds::core::Duration::infinite());
            }

            dds::sub::LoanedSamples<DynamicData> samples =
                    this->_reader.take();
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

            DynamicData& sample =
                    const_cast<DynamicData&>(
                            samples[this->_data_idx].data());
            this->_message.entity_id = sample.value<int32_t>("entity_id");
            this->_message.seq_num = sample.value<uint32_t>("seq_num");
            this->_message.timestamp_sec = sample.value<int32_t>("timestamp_sec");
            this->_message.timestamp_usec = sample.value<uint32_t>("timestamp_usec");
            this->_message.latency_ping = sample.value<int32_t>("latency_ping");
            this->_message.size = (int)(sample.get_values<uint8_t>("bin_data")).size();

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
                    this->_message.entity_id = sample.value<int32_t>("entity_id");
                    this->_message.seq_num = sample.value<uint32_t>("seq_num");
                    this->_message.timestamp_sec =
                            sample.value<int32_t>("timestamp_sec");
                    this->_message.timestamp_usec =
                            sample.value<uint32_t>("timestamp_usec");
                    this->_message.latency_ping =
                            sample.value<int32_t>("latency_ping");
                    this->_message.size =
                            (int)(sample.get_values<uint8_t>("bin_data")).size();
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

    // print arguments
    printSecureArgs();

    // load plugin
    dpQosProperties["com.rti.serv.load_plugin"] = "com.rti.serv.secure";

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    dpQosProperties["com.rti.serv.secure.create_function"] =
            "RTI_Security_PluginSuite_create";

    dpQosProperties["com.rti.serv.secure.library"] = _secureLibrary;

  #else // Static library linking

    void *pPtr = (void *) RTI_Security_PluginSuite_create;
    dpQosProperties["com.rti.serv.secure.create_function_ptr"] =
            rti::util::ptr_to_str(pPtr);

  #endif

    // check if governance file provided
    if (_secureGovernanceFile.empty()) {
        // choose a pre-built governance file
        std::string governance_file = "./resource/secure/signed_PerftestGovernance_";
        if (_secureIsDiscoveryEncrypted) {
            governance_file += "Discovery";
        }

        if (_secureIsSigned) {
            governance_file += "Sign";
        }

        if (_secureIsDataEncrypted && _secureIsSMEncrypted) {
            governance_file += "EncryptBoth";
        } else if (_secureIsDataEncrypted) {
            governance_file += "EncryptData";
        } else if (_secureIsSMEncrypted) {
            governance_file += "EncryptSubmessage";
        }

        governance_file = governance_file + ".xml";

        std::cout << "[Info] Secure: using pre-built governance file: "
                  << governance_file
                  << std::endl;
        dpQosProperties["com.rti.serv.secure.access_control.governance_file"] =
                governance_file;
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
    if (_secureUseSecure) {
        if (_securePrivateKeyFile.empty()) {
            if (_isPublisher) {
                _securePrivateKeyFile = SECURE_PRIVATEKEY_FILE_PUB;
            } else {
                _securePrivateKeyFile = SECURE_PRIVATEKEY_FILE_SUB;
            }
        }

        if (_secureCertificateFile.empty()) {
            if (_isPublisher) {
                _secureCertificateFile = SECURE_CERTIFICATE_FILE_PUB;
            } else {
                _secureCertificateFile = SECURE_CERTIFICATE_FILE_SUB;
            }
        }

        if (_secureCertAuthorityFile.empty()) {
            _secureCertAuthorityFile = SECURE_CERTAUTHORITY_FILE;
        }

        if (_securePermissionsFile.empty()) {
            if (_isPublisher) {
                _securePermissionsFile = SECURE_PERMISION_FILE_PUB;
            } else {
                _securePermissionsFile = SECURE_PERMISION_FILE_SUB;
            }
        }

      #ifdef RTI_PERFTEST_DYNAMIC_LINKING
        if (_secureLibrary.empty()) {
            _secureLibrary = SECURE_LIBRARY_NAME;
        }
      #endif
    }
}

template <typename T>
void RTIDDSImpl<T>::printSecureArgs()
{
    std::cout << "[Info] Secure Arguments:\n"
              << "\t encrypt discovery: "
              << (_secureIsDiscoveryEncrypted ? "true\n" : "false\n")
              << "\t encrypt topic (user) data: "
              << (_secureIsDataEncrypted ? "true\n" : "false\n")
              << "\t encrypt submessage: "
              << (_secureIsSMEncrypted ? "true\n" : "false\n")
              << "\t sign data: "
              << (_secureIsSigned ? "true\n" : "false\n")
              << "\t governance file: "
              << (_secureGovernanceFile.empty() ? "not specified" : _secureGovernanceFile)
              << "\n\t permissions file: "
              << (_securePermissionsFile.empty() ? "not specified" : _securePermissionsFile)
              << "\n\t private key file: "
              << (_securePrivateKeyFile.empty() ? "not specified" : _securePrivateKeyFile)
              << "\n\t certificate file: "
              << (_secureCertificateFile.empty() ? "not specified" : _secureCertificateFile)
              << "\n\t certificate authority file: "
              << (_secureCertAuthorityFile.empty() ? "not specified" : _secureCertAuthorityFile)
              << "\n\t plugin library: "
              << (_secureLibrary.empty() ? "not specified" : _secureLibrary)
              << std::endl;
    if( _secureDebugLevel != -1 ){
        std::cout << "\t debug level: "
                  << _secureDebugLevel
                  << std::endl;
    }
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
bool RTIDDSImpl<T>::Initialize(int argc, char *argv[])
{
    using namespace rti::core::policy;
    ParseConfig(argc, argv);

    // setup the QOS profile file to be loaded
    dds::core::QosProvider qos_provider =
        getQosProviderForProfile("PerftestQosLibrary","BaseProfileQos");
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
    if ( _peer_host_count > 0 ) {
        std::cout << "[INFO]: Initial peers: ";
        for ( int i =0; i< _peer_host_count; ++i) {
            std::cout << _peer_host[i] << " ";
        }
        std::cout << std::endl;
        _peer_host.resize(_peer_host_count);
        qos_discovery.initial_peers(_peer_host);
        qos_discovery.multicast_receive_addresses(dds::core::StringSeq());
    }

    // set transports to use
    qos << rti::core::policy::TransportBuiltin(TransportBuiltinMask::udpv4());
    if (_UseTcpOnly) {
        qos << TransportBuiltin(TransportBuiltinMask::none());
        properties["dds.transport.load_plugins"] = "dds.transport.TCPv4.tcp1";
    } else if (_UseSharedMemory) {
        qos << TransportBuiltin(TransportBuiltinMask::shmem());
    } else if (_UseUdpv6) {
        qos << TransportBuiltin(TransportBuiltinMask::udpv6());
    }

    if (_AutoThrottle) {
        properties["dds.domain_participant.auto_throttle.enable"] = "true";
    }

    if (!_UseTcpOnly) {
        if ((_Nic != NULL) && (strlen(_Nic) > 0)) {
            properties["dds.transport.UDPv4.builtin.parent.allow_interfaces"] = _Nic;
        }

        // Shem transport properties
        int received_message_count_max = 1024 * 1024 * 2 / (int)_DataLen;

        if (received_message_count_max < 1) {
            received_message_count_max = 1;
        }

        char buf[64];
        sprintf(buf,"%d", received_message_count_max);
        properties["dds.transport.shmem.builtin.received_message_count_max"] = buf;
    }

    qos << qos_discovery;
    //We have to copy the properties to the participant_qos object
    qos << rti::core::policy::Property(
            properties.begin(),
            properties.end(),
            true);

    DomainListener *listener = new DomainListener;

    // Creates the participant
    _participant = dds::domain::DomainParticipant(_DomainID, qos, listener,
            dds::core::status::StatusMask::inconsistent_topic() |
            dds::core::status::StatusMask::offered_incompatible_qos() |
            dds::core::status::StatusMask::requested_incompatible_qos() );

    // Create the _publisher and _subscriber
    _publisher = dds::pub::Publisher(_participant, qos_provider.publisher_qos());

    _subscriber = dds::sub::Subscriber(_participant, qos_provider.subscriber_qos());

    return true;

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
    if (topic_name == perftest_cpp::_ThroughputTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "ThroughputQos";
        } else {
            qos_profile = "NoAckThroughputQos";
        }
    } else if (topic_name == perftest_cpp::_LatencyTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "LatencyQos";
        } else {
            qos_profile = "NoAckLatencyQos";
        }
    } else if (topic_name == perftest_cpp::_AnnouncementTopicName) {
        qos_profile = "AnnouncementQos";
    } else {
        std::cerr << "[Error] Topic name must either be "
                << perftest_cpp::_ThroughputTopicName << " or "
                << perftest_cpp::_LatencyTopicName << " or "
                << perftest_cpp::_AnnouncementTopicName << std::endl;
        throw std::logic_error("[Error] Topic name");
    }

    std::string lib_name(_ProfileLibraryName);
    dds::core::QosProvider qos_provider =
        getQosProviderForProfile(lib_name, qos_profile);
    dds::pub::qos::DataWriterQos dw_qos = qos_provider.datawriter_qos();

    Reliability qos_reliability = dw_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dw_qos.policy<ResourceLimits>();
    DataWriterResourceLimits qos_dw_resource_limits =
            dw_qos.policy<DataWriterResourceLimits>();
    Durability qos_durability = dw_qos.policy<Durability>();
    PublishMode dwPublishMode= dw_qos.policy<PublishMode>();
    Batch dwBatch = dw_qos.policy<Batch>();
    rti::core::policy::DataWriterProtocol dw_DataWriterProtocol =
            dw_qos.policy<rti::core::policy::DataWriterProtocol>();
    RtpsReliableWriterProtocol dw_reliableWriterProtocol =
            dw_DataWriterProtocol.rtps_reliable_writer();


    // This will allow us to load some properties.
    std::map<std::string, std::string> properties =
            dw_qos.policy<Property>().get_all();

    if (_UsePositiveAcks) {
        dw_reliableWriterProtocol.disable_positive_acks_min_sample_keep_duration(
                dds::core::Duration((int) _KeepDurationUsec / 1000000,
                        _KeepDurationUsec % 1000000));
    }

    if (((_isLargeData) && (!_isScan)) || _IsAsynchronous) {
        std::cerr << "[Info] Using asynchronous write for "
                  << topic_name << std::endl;

       if (_FlowControllerCustom!= "default") {
           dwPublishMode = PublishMode::Asynchronous(
               "dds.flow_controller.token_bucket."+_FlowControllerCustom);
       } else{
           dwPublishMode = PublishMode::Asynchronous();
       }
       std::cerr << "[Info] Using flow controller "
                 << _FlowControllerCustom << std::endl;
   }

    // only force reliability on throughput/latency topics
    if (topic_name != perftest_cpp::_AnnouncementTopicName) {
        if (_IsReliable) {
            qos_reliability = Reliability::Reliable(dds::core::Duration::infinite());
        } else {
            qos_reliability = Reliability::BestEffort();
        }
    }

    // These QOS's are only set for the Throughput datawriter
    if (qos_profile == "ThroughputQos" ||
        qos_profile == "NoAckThroughputQos")
    {

        if (_IsMulticast) {
            dw_reliableWriterProtocol.enable_multicast_periodic_heartbeat(true);
        }

        if (_BatchSize > 0) {
            dwBatch.enable(true);
            dwBatch.max_data_bytes(_BatchSize);
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_SendQueueSize);
        } else {
            qos_resource_limits.max_samples(_SendQueueSize);
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

        if (_AutoThrottle) {
            properties["dds.data_writer.auto_throttle.enable"] = "true";
        }

        if (_TurboMode) {
            properties["dds.data_writer.enable_turbo_mode.enable"] = "true";
            dwBatch.enable(false);
            qos_resource_limits.max_samples(dds::core::LENGTH_UNLIMITED);
            qos_dw_resource_limits.max_batches(_SendQueueSize);
        }

        qos_resource_limits->initial_samples(_SendQueueSize);
        qos_resource_limits.max_samples_per_instance(qos_resource_limits.max_samples());

        if (_Durability == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);

        dw_reliableWriterProtocol.heartbeats_per_max_samples(_SendQueueSize / 10);
        dw_reliableWriterProtocol.low_watermark(_SendQueueSize * 1 / 10);
        dw_reliableWriterProtocol.high_watermark(_SendQueueSize * 9 / 10);

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

        dw_reliableWriterProtocol.max_send_window_size(_SendQueueSize);
        dw_reliableWriterProtocol.min_send_window_size(_SendQueueSize);
    }

    if ( (qos_profile == "LatencyQos" || qos_profile =="NoAckLatencyQos")
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

    qos_resource_limits.max_instances(_InstanceCount);
    qos_resource_limits->initial_instances(_InstanceCount);

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
    dw_DataWriterProtocol.rtps_reliable_writer(dw_reliableWriterProtocol);
    dw_qos << dw_DataWriterProtocol;
    dw_qos << Property(properties.begin(), properties.end(), true);

    if (!_isDynamicData) {
        dds::topic::Topic<T> topic(_participant, topic_name);
        dds::pub::DataWriter<T> writer(_publisher, topic, dw_qos);
        return new RTIPublisher<T>(
                writer,
                _InstanceCount,
                _pongSemaphore,
                _LatencyTest);

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
                type);
    }
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
    if (topic_name == perftest_cpp::_ThroughputTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "ThroughputQos";
        } else {
            qos_profile = "NoAckThroughputQos";
        }
    } else if (topic_name == perftest_cpp::_LatencyTopicName) {
        if (_UsePositiveAcks) {
            qos_profile = "LatencyQos";
        } else {
            qos_profile = "NoAckLatencyQos";
        }
    } else if (topic_name == perftest_cpp::_AnnouncementTopicName) {
        qos_profile = "AnnouncementQos";
    } else {
        std::cerr << "[Error] Topic name must either be "
                << perftest_cpp::_ThroughputTopicName << " or "
                << perftest_cpp::_LatencyTopicName << " or "
                << perftest_cpp::_AnnouncementTopicName << std::endl;
        throw std::logic_error("[Error] Topic name");
    }

    std::string lib_name(_ProfileLibraryName);
    dds::core::QosProvider qos_provider =
        getQosProviderForProfile(lib_name, qos_profile);
    dds::sub::qos::DataReaderQos dr_qos = qos_provider.datareader_qos();


    Reliability qos_reliability = dr_qos.policy<Reliability>();
    ResourceLimits qos_resource_limits = dr_qos.policy<ResourceLimits>();
    Durability qos_durability = dr_qos.policy<Durability>();

    // This will allow us to load some properties.
    std::map<std::string, std::string> properties =
            dr_qos.policy<Property>().get_all();

    // only force reliability on throughput/latency topics
    if (topic_name != perftest_cpp::_AnnouncementTopicName) {
        if (_IsReliable) {
            qos_reliability = dds::core::policy::Reliability::Reliable();
        } else {
            qos_reliability = dds::core::policy::Reliability::BestEffort();
        }
    }

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos" ||
        qos_profile == "NoAckThroughputQos") {

        if (_Durability == DDS_VOLATILE_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::Volatile();
        } else if (_Durability == DDS_TRANSIENT_DURABILITY_QOS) {
            qos_durability = dds::core::policy::Durability::TransientLocal();
        } else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    if ((qos_profile == "LatencyQos" ||
        qos_profile == "NoAckLatencyQos") &&
        !_DirectCommunication &&
        (_Durability == DDS_TRANSIENT_DURABILITY_QOS ||
         _Durability == DDS_PERSISTENT_DURABILITY_QOS)) {
        if (_Durability == DDS_TRANSIENT_DURABILITY_QOS){
            qos_durability = dds::core::policy::Durability::TransientLocal();
        }
        else {
            qos_durability = dds::core::policy::Durability::Persistent();
        }
        qos_durability->direct_communication(_DirectCommunication);
    }

    qos_resource_limits->initial_instances(_InstanceCount);
    qos_resource_limits->max_instances(_InstanceMaxCountReader);

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            qos_resource_limits->instance_hash_buckets(_InstanceHashBuckets);
        } else {
            qos_resource_limits->instance_hash_buckets(_InstanceCount);
        }
    }

    if (!_UseTcpOnly && _IsMulticast) {

       const char *multicast_addr;

        if (topic_name == perftest_cpp::_ThroughputTopicName) {
            multicast_addr = THROUGHPUT_MULTICAST_ADDR;
        } else if (topic_name == perftest_cpp::_LatencyTopicName) {
            multicast_addr = LATENCY_MULTICAST_ADDR;
        } else {
            multicast_addr = ANNOUNCEMENT_MULTICAST_ADDR;
        }

        dds::core::StringSeq transports;
        transports.push_back("udpv4");
        rti::core::TransportMulticastSettings multicast_settings(
                transports, multicast_addr, 0);
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
    dr_qos << Property(properties.begin(), properties.end(), true);

    if (!_isDynamicData) {
        dds::topic::Topic<T> topic(_participant, topic_name);
        dds::sub::DataReader<T> reader(dds::core::null);
        ReceiverListener<T> *reader_listener = NULL;
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

        return new RTISubscriber<T>(
                reader,
                reader_listener,
                _WaitsetEventCount,
                _WaitsetDelayUsec);

    } else {

        const dds::core::xtypes::StructType& type =
                rti::topic::dynamic_type<T>::get();
        dds::topic::Topic<DynamicData> topic(
                _participant,
                topic_name,
                type);
        dds::sub::DataReader<DynamicData> reader(
                dds::core::null);
        DynamicDataReceiverListener *dynamic_data_reader_listener = NULL;

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

        return new RTIDynamicDataSubscriber(
                reader,
                dynamic_data_reader_listener,
                _WaitsetEventCount,
                _WaitsetDelayUsec);
    }
}

template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;
template class RTIDDSImpl<TestDataKeyedLarge_t>;
template class RTIDDSImpl<TestDataLarge_t>;

#ifdef RTI_WIN32
  #pragma warning(pop)
#endif