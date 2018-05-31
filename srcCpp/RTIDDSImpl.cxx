/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "perftest.h"
#include "perftestPlugin.h"
#include "perftestSupport.h"
#include "MessagingIF.h"
#include "perftest_cpp.h"
#include "RTIDDSImpl.h"
#include "ndds/ndds_cpp.h"
#include "qos_string.h"

#ifdef RTI_SECURE_PERFTEST
#include "security/security_default.h"
#endif

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


template class RTIDDSImpl<TestDataKeyed_t>;
template class RTIDDSImpl<TestData_t>;
template class RTIDDSImpl<TestDataKeyedLarge_t>;
template class RTIDDSImpl<TestDataLarge_t>;

template <typename T>
int RTIDDSImpl<T>::_WaitsetEventCount = 5;
template <typename T>
unsigned int RTIDDSImpl<T>::_WaitsetDelayUsec = 100;

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
const std::string RTIDDSImpl<T>::SECURE_LIBRARY_NAME =
        "nddssecurity";
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

/*********************************************************
 * Shutdown
 */
template <typename T>
void RTIDDSImpl<T>::Shutdown()
{
    //delete array of peers
    for (int i = 0; i < _peer_host_count; ++i) {
        DDS_String_free(_peer_host[i]);
        _peer_host[i] = NULL;
    }

    if (_participant != NULL) {
        perftest_cpp::MilliSleep(2000);

        if (_reader != NULL) {
            DDSDataReaderListener* reader_listener = _reader->get_listener();
            if (reader_listener != NULL) {
                delete(reader_listener);
            }
            _subscriber->delete_datareader(_reader);
        }

        DDSDomainParticipantListener* participant_listener = _participant->get_listener();
        if (participant_listener != NULL) {
            delete(participant_listener);
        }

        _participant->delete_contained_entities();
        DDSTheParticipantFactory->delete_participant(_participant);
    }
    if(_pongSemaphore != NULL) {
        RTIOsapiSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }
    // Unregister _loggerDevice
    NDDSConfigLogger::finalize_instance();
    DDSDomainParticipantFactory::finalize_instance();
}

/*********************************************************
 * PrintCmdLineHelp
 */
template <typename T>
void RTIDDSImpl<T>::PrintCmdLineHelp()
{
    /**************************************************************************/
    std::string usage_string = std::string(
            "\t-sendQueueSize <number>       - Sets number of samples (or batches) in send\n") +
            "\t                                queue, default 50\n" +
            "\t-domain <ID>                  - RTI DDS Domain, default 1\n" +
            "\t-qosFile <filename>           - Name of XML file for DDS Qos profiles, \n" +
            "\t                                default: perftest_qos_profiles.xml\n" +
            "\t-qosLibrary <lib name>        - Name of QoS Library for DDS Qos profiles, \n" +
            "\t                                default: PerftestQosLibrary\n" +
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
    usage_string += _transport.helpMessageString();
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

    fprintf(stderr, "%s", usage_string.c_str());
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

    // Command line params
    for (i = 0; i < argc; ++i) {
        if (IS_OPTION(argv[i], "-pub")) {
            _isPublisher = true;
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

            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <length> after -dataLen\n");
                return false;
            }

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < (unsigned long)perftest_cpp::OVERHEAD_BYTES) {
                fprintf(stderr, "-dataLen must be >= %d\n", perftest_cpp::OVERHEAD_BYTES);
                return false;
            }

            if (_DataLen > (unsigned long)MAX_PERFTEST_SAMPLE_SIZE) {
                fprintf(stderr, "-dataLen must be <= %d\n", MAX_PERFTEST_SAMPLE_SIZE);
                return false;
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
                    fprintf(stderr, "-unbounded <allocation_threshold> must be >= %d\n",  perftest_cpp::OVERHEAD_BYTES);
                    return false;
                }
                if (_useUnbounded > (unsigned long)MAX_BOUNDED_SEQ_SIZE)
                {
                    fprintf(stderr,"-unbounded <allocation_threshold> must be <= %d\n", MAX_BOUNDED_SEQ_SIZE);
                    return false;
                }
            }
        } else if (IS_OPTION(argv[i], "-sendQueueSize")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <count> after -sendQueueSize\n");
                return false;
            }
            _SendQueueSize = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-heartbeatPeriod")) {
            if ((i == (argc - 1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <period> after -heartbeatPeriod\n");
                return false;
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i],"%d:%u",&sec,&nanosec) != 2) {
                fprintf(stderr, "-heartbeatPeriod value must have the format <sec>:<nanosec>\n");
                return false;
            }

            if (sec > 0 || nanosec > 0) {
                _HeartbeatPeriod.sec = sec;
                _HeartbeatPeriod.nanosec = nanosec;
            }
        } else if (IS_OPTION(argv[i], "-fastHeartbeatPeriod")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <period> after -fastHeartbeatPeriod\n");
                return false;
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i],"%d:%u",&sec,&nanosec) != 2) {
                fprintf(stderr, "-fastHeartbeatPeriod value must have the format <sec>:<nanosec>\n");
                return false;
            }

            if (sec > 0 || nanosec > 0) {
                _FastHeartbeatPeriod.sec = sec;
                _FastHeartbeatPeriod.nanosec = nanosec;
            }
        } else if (IS_OPTION(argv[i], "-domain")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <id> after -domain\n");
                return false;
            }
            _DomainID = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-qosFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <filename> after -qosFile\n");
                return false;
            }
            _ProfileFile = argv[i];
        } else if (IS_OPTION(argv[i], "-qosLibrary")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <library name> after -qosLibrary\n");
                return false;
            }
            _ProfileLibraryName = argv[i];
        } else if (IS_OPTION(argv[i], "-bestEffort")) {
            _IsReliable = false;
        } else if (IS_OPTION(argv[i], "-durability")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <kind> after -durability\n");
                return false;
            }
            _Durability = strtol(argv[i], NULL, 10);

            if ((_Durability < 0) || (_Durability > 3)) {
                fprintf(stderr, "durability kind must be 0(volatile), 1(transient local), 2(transient), or 3(persistent) \n");
                return false;
            }
        } else if (IS_OPTION(argv[i], "-dynamicData")) {
            _isDynamicData = true;
        } else if (IS_OPTION(argv[i], "-noDirectCommunication")) {
            _DirectCommunication = false;
        } else if (IS_OPTION(argv[i], "-instances")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <count> after -instances\n");
                return false;
            }
            _InstanceCount = strtol(argv[i], NULL, 10);
            _InstanceMaxCountReader = _InstanceCount;

            if (_InstanceCount <= 0) {
                fprintf(stderr, "instance count cannot be negative or zero\n");
                return false;
            }
        } else if (IS_OPTION(argv[i], "-instanceHashBuckets")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <count> after -instanceHashBuckets\n");
                return false;
            }
            _InstanceHashBuckets = strtol(argv[i], NULL, 10);

            if (_InstanceHashBuckets <= 0 && _InstanceHashBuckets != -1) {
                fprintf(stderr, "instance hash buckets cannot be negative or zero\n");
                return false;
            }
        } else if (IS_OPTION(argv[i], "-batchSize")) {

            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <#bytes> after -batchSize\n");
                return false;
            }
            _BatchSize = strtol(argv[i], NULL, 10);

            if (_BatchSize < 0 || _BatchSize > (int)MAX_SYNCHRONOUS_SIZE) {
                fprintf(stderr,
                        "Batch size '%d' should be between [0,%d]\n",
                        _BatchSize,
                        MAX_SYNCHRONOUS_SIZE);
                return false;
            }
            isBatchSizeProvided = true;
        } else if (IS_OPTION(argv[i], "-keepDurationUsec")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <usec> after -keepDurationUsec\n");
                return false;
            }
            _KeepDurationUsec = strtol(argv[i], NULL, 10);
        } else if (IS_OPTION(argv[i], "-noPositiveAcks")) {
            _UsePositiveAcks = false;
        }
        else if (IS_OPTION(argv[i], "-verbosity"))
        {
            errno = 0;
            int verbosityLevel = strtol(argv[++i], NULL, 10);

            if (errno) {
                fprintf(stderr, "Unexpected value after -verbosity\n");
                return false;
            }

            switch (verbosityLevel) {
                case 0: NDDSConfigLogger::get_instance()->
                            set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                        fprintf(stderr, "Setting verbosity to SILENT\n");
                        break;
                case 1: NDDSConfigLogger::get_instance()->
                            set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                        fprintf(stderr, "Setting verbosity to ERROR\n");
                        break;
                case 2: NDDSConfigLogger::get_instance()->
                            set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                        fprintf(stderr, "Setting verbosity to WARNING\n");
                        break;
                case 3: NDDSConfigLogger::get_instance()->
                            set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                        fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                        break;
                default: fprintf(stderr,
                            "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                        verbosityLevel = 1;
                        break;
            }
        }
        else if (IS_OPTION(argv[i], "-waitsetDelayUsec")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <usec> after -waitsetDelayUsec\n");
                return false;
            }
            _WaitsetDelayUsec = (unsigned int) strtol (argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-waitsetEventCount")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -waitsetEventCount\n");
                return false;
            }
            _WaitsetEventCount = strtol (argv[i], NULL, 10);
            if (_WaitsetEventCount < 0)
            {
                fprintf(stderr, "waitset event count cannot be negative\n");
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-latencyTest"))
        {
            _LatencyTest = true;
        }
        else if (IS_OPTION(argv[i], "-enableAutoThrottle"))
        {
            _AutoThrottle = true;
        }
        else if (IS_OPTION(argv[i], "-enableTurboMode") )
        {
            _TurboMode = true;
        }
        else if (IS_OPTION(argv[i], "-noXmlQos") )
        {
            _UseXmlQos = false;
        }
        else if (IS_OPTION(argv[i], "-asynchronous") )
        {
            _IsAsynchronous = true;
        }
        else if (IS_OPTION(argv[i], "-flowController"))
        {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <flow Controller Name> after -flowController\n");
                return false;
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
                fprintf(stderr, "Bad <flow> '%s' for custom flow controller\n",_FlowControllerCustom.c_str());
                _FlowControllerCustom = "default";
            }
        }
        else if (IS_OPTION(argv[i], "-peer")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <address> after -peer\n");
                return false;
            }
            if (_peer_host_count +1 < RTIPERFTEST_MAX_PEERS) {
                _peer_host[_peer_host_count++] = DDS_String_dup(argv[i]);
            } else {
                fprintf(stderr,"The maximun of -initial peers is %d\n", RTIPERFTEST_MAX_PEERS);
                return false;
            }
        } else if (IS_OPTION(argv[i], "-cft")) {
            _useCft = true;
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <start>:<end> after -cft\n");
                return false;
            }

            if (strchr(argv[i],':') != NULL) { // In the case that there are 2 parameter
                unsigned int cftStart = 0;
                unsigned int cftEnd = 0;
                if (sscanf(argv[i],"%u:%u",&cftStart,&cftEnd) != 2) {
                    fprintf(stderr, "-cft value must have the format <start>:<end>\n");
                    return false;
                }
                _CFTRange[0] = cftStart;
                _CFTRange[1] = cftEnd;
            } else {
                _CFTRange[0] = strtol(argv[i], NULL, 10);
                _CFTRange[1] = _CFTRange[0];
            }

            if (_CFTRange[0] > _CFTRange[1]) {
                fprintf(stderr, "-cft <start> value cannot be bigger than <end>\n");
                return false;
            }
            if (_CFTRange[0] < 0 ||
                    _CFTRange[0] >= (unsigned int)MAX_CFT_VALUE ||
                    _CFTRange[1] < 0 ||
                    _CFTRange[1] >= (unsigned int)MAX_CFT_VALUE) {
                fprintf(stderr, "-cft <start>:<end> values should be between [0,%d] \n", MAX_CFT_VALUE);
                return false;
            }
        } else if (IS_OPTION(argv[i], "-writeInstance")) {
            if ((i == (argc-1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <number> after -writeInstance\n");
                return false;
            }
            _instancesToBeWritten = strtol(argv[i], NULL, 10);
        }
      #ifdef RTI_SECURE_PERFTEST
        else if (IS_OPTION(argv[i], "-secureSign")) {
            _secureIsSigned = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptBoth")) {
            _secureIsDataEncrypted = true;
            _secureIsSMEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptData")) {
            _secureIsDataEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptSM")) {
            _secureIsSMEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptDiscovery")) {
            _secureIsDiscoveryEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureGovernanceFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
               fprintf(stderr, "Missing <file> after -secureGovernanceFile\n");
               return false;
            }
            _secureGovernanceFile = argv[i];
            fprintf(stdout, "Warning -- authentication, encryption, signing arguments "
                    "will be ignored, and the values specified by the Governance file will "
                    "be used instead\n");
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-securePermissionsFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -securePermissionsFile\n");
                return false;
            }
            _securePermissionsFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureCertAuthority")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -secureCertAuthority\n");
                return false;
            }
            _secureCertAuthorityFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureCertFile")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -secureCertFile\n");
                return false;
            }
            _secureCertificateFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-securePrivateKey")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -securePrivateKey\n");
                return false;
            }
            _securePrivateKeyFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureLibrary")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <file> after -secureLibrary\n");
                return false;
            }
            _secureLibrary = argv[i];
        }
        else if (IS_OPTION(argv[i], "-secureDebug")) {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <level> after -secureDebug\n");
                 return false;
            }
            _secureDebugLevel = strtol(argv[i], NULL, 10);
        }
      #endif
        else {
            if (i > 0) {
                std::map<std::string, unsigned int> transportCmdOpts =
                        PerftestTransport::getTransportCmdLineArgs();

                std::map<std::string, unsigned int>::iterator it =
                        transportCmdOpts.find(argv[i]);
                if(it != transportCmdOpts.end()) {
                    /*
                     * Increment the counter with the number of arguments
                     * obtained from the map.
                     */
                    i = i + it->second;
                    continue;
                }

                fprintf(stderr, "%s: not recognized\n", argv[i]);
                return false;
            }
        }
    }

    /* If we are using scan, we get the minimum and set it in Datalen */
    if (_isScan) {
        _DataLen = minScanSize;
    }

    /* Check if we need to enable Large Data. This works also for -scan */
    if (_DataLen > (unsigned long) (std::min)(
                MAX_SYNCHRONOUS_SIZE,
                MAX_BOUNDED_SEQ_SIZE)) {
        _isLargeData = true;
        if (_useUnbounded == 0) {
            _useUnbounded = MAX_BOUNDED_SEQ_SIZE;
        }
    } else { /* No Large Data */
        _useUnbounded = 0;
        _isLargeData = false;
    }

    /* If we are using batching */
    if (_BatchSize > 0) {

        /* We will not use batching for a latency test */
        if (_LatencyTest) {
            if (isBatchSizeProvided) {
                fprintf(stderr, "Batching cannot be used in a Latency test.\n");
                return false;
            } else {
                _BatchSize = 0; //Disable Batching
            }
        }

        /* Check if using asynchronous */
        if (_IsAsynchronous) {
            fprintf(stderr, "Batching cannot be used with asynchronous writing.\n");
            return false;
        }

        /*
         * Large Data + batching cannot be set. But batching is enabled by default,
         * so in that case, we just disabled batching, else, the customer set it up,
         * so we explitly fail
         */
        if (_isLargeData) {
            if (isBatchSizeProvided) {
                fprintf(stderr, "Batching cannot be used with Large Data.\n");
                return false;
            } else {
                _BatchSize = -2;
            }
        } else if ((unsigned long) _BatchSize < _DataLen * 2) {
            /*
             * We don't want to use batching if the batch size is not large
             * enough to contain at least two samples (in this case we avoid the
             * checking at the middleware level).
             */
            if (isBatchSizeProvided || _isScan) {
                /*
                 * Batchsize disabled. A message will be print if _batchsize < 0 in
                 * perftest_cpp::PrintConfiguration()
                 */
                _BatchSize = -1;
            }
            else {
                _BatchSize = 0;
            }
        }
    }

    if (_TurboMode) {
        if (_IsAsynchronous) {
            fprintf(stderr, "Turbo Mode cannot be used with asynchronous writing.\n");
            return false;
        }
        if (_isLargeData) {
            fprintf(stderr, "Turbo Mode disabled, using large data.\n");
            _TurboMode = false;
        }
    }

    // Manage _instancesToBeWritten
    if (_instancesToBeWritten != -1) {
        if ((long)_InstanceCount < _instancesToBeWritten) {
            fprintf(
                    stderr,
                    "Specified '-WriteInstance' (%ld) invalid: Bigger than the number of instances (%lu).\n",
                    _instancesToBeWritten, _InstanceCount);
            return false;
        }
    }
    if (_isPublisher && _useCft) {
        fprintf(stderr,
                "Content Filtered Topic is not a parameter in the publisher side.\n");
    }

    if(!_transport.parseTransportOptions(argc, argv)) {
        fprintf(stderr,
                "Failure parsing the transport options.\n");
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
    stringStream << "\tDomain: " << _DomainID << "\n";

    // Dynamic Data
    stringStream << "\tDynamic Data: ";
    if (_isDynamicData) {
        stringStream << "Yes\n";
    } else {
        stringStream << "No\n";
    }

    // Dynamic Data
    if (_isPublisher) {
        stringStream << "\tAsynchronous Publishing: ";
        if (_isLargeData || _IsAsynchronous) {
            stringStream << "Yes\n";
            stringStream << "\tFlow Controller: "
                         << _FlowControllerCustom
                         << "\n";
        } else {
            stringStream << "No\n";
        }
    }

    // Turbo Mode / AutoThrottle
    if (_TurboMode) {
        stringStream << "\tTurbo Mode: Enabled\n";
    }
    if (_AutoThrottle) {
        stringStream << "\tAutoThrottle: Enabled\n";
    }

    // XML File
    stringStream << "\tXML File: ";
    if (!_UseXmlQos) {
        stringStream << "Disabled\n";
    } else {
        stringStream << _ProfileFile << "\n";
    }

    stringStream << "\n" << _transport.printTransportConfigurationSummary();


    // set initial peers and not use multicast
    if (_peer_host_count > 0) {
        stringStream << "\tInitial peers: ";
        for (int i = 0; i < _peer_host_count; ++i) {
            stringStream << _peer_host[i];
            if (i == _peer_host_count - 1) {
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
class DomainListener : public DDSDomainParticipantListener
{
    virtual void on_inconsistent_topic(
        DDSTopic *topic,
        const DDS_InconsistentTopicStatus& /*status*/)
    {
        fprintf(stderr,"Found inconsistent topic. Expecting %s of type %s.\n",
               topic->get_name(), topic->get_type_name());
        fflush(stderr);
    }

    virtual void on_offered_incompatible_qos(
        DDSDataWriter *writer,
        const DDS_OfferedIncompatibleQosStatus &status)
    {
        fprintf(stderr,"Found incompatible reader for writer %s QoS is %d.\n",
               writer->get_topic()->get_name(), status.last_policy_id);
        fflush(stderr);
    }

    virtual void on_requested_incompatible_qos(
        DDSDataReader *reader,
        const DDS_RequestedIncompatibleQosStatus &status)
    {
        fprintf(stderr,"Found incompatible writer for reader %s QoS is %d.\n",
               reader->get_topicdescription()->get_name(), status.last_policy_id);
        fflush(stderr);
    }
};

/*********************************************************
 * RTIPublisher
 */
template<typename T>
class RTIPublisher : public IMessagingWriter
{
  private:
    typename T::DataWriter *_writer;
    T data;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    DDS_InstanceHandle_t *_instance_handles;
    RTIOsapiSemaphore *_pongSemaphore;
    long _instancesToBeWritten;
    bool _isReliable;

 public:
    RTIPublisher(DDSDataWriter *writer, unsigned long num_instances, RTIOsapiSemaphore * pongSemaphore, int instancesToBeWritten)
    {
        _writer = T::DataWriter::narrow(writer);
        data.bin_data.maximum(0);
        _num_instances = num_instances;
        _instance_counter = 0;
        _instance_handles =
                (DDS_InstanceHandle_t *) malloc(sizeof(DDS_InstanceHandle_t)*(_num_instances + 1)); // One extra for MAX_CFT_VALUE
        _pongSemaphore = pongSemaphore;
        _instancesToBeWritten = instancesToBeWritten;

        for (unsigned long i = 0; i < _num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                data.key[c] = (unsigned char) (i >> c * 8);
            }
            _instance_handles[i] = _writer->register_instance(data);
        }
        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            data.key[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
        }
        _instance_handles[_num_instances] = _writer->register_instance(data);

        DDS_DataWriterQos qos;
        _writer->get_qos(qos);
        _isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    }

    ~RTIPublisher() {
        Shutdown();
    }

    void Shutdown() {
        if (_writer->get_listener() != NULL) {
            delete(_writer->get_listener());
            _writer->set_listener(NULL);
        }

        free(_instance_handles);
    }

    void Flush()
    {
        _writer->flush();
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        long key = 0;

        data.entity_id = message.entity_id;
        data.seq_num = message.seq_num;
        data.timestamp_sec = message.timestamp_sec;
        data.timestamp_usec = message.timestamp_usec;
        data.latency_ping = message.latency_ping;
        data.bin_data.loan_contiguous((DDS_Octet*)message.data, message.size, message.size);

        if (!isCftWildCardKey) {
            if (_num_instances > 1) {
                if (_instancesToBeWritten == -1) {
                    key = _instance_counter++ % _num_instances;
                } else { // send sample to a specific subscriber
                    key = _instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }
        for (int c = 0; c < KEY_SIZE; c++) {
            data.key[c] = (unsigned char)(key >> c * 8);
        }

        if (!isCftWildCardKey) {
            retcode = _writer->write(data, _instance_handles[key]);
        } else { // send CFT_MAX sample
            retcode = _writer->write(data, _instance_handles[_num_instances]);
        }
        data.bin_data.unloan();

        if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Write error %d.\n", retcode);
            return false;
        }

        return true;
    }

    void WaitForReaders(int numSubscribers)
    {
        DDS_PublicationMatchedStatus status;

        while (true)
        {
            _writer->get_publication_matched_status(status);
            if (status.current_count >= numSubscribers)
            {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }

    bool waitForPingResponse() {
        if(_pongSemaphore != NULL) {
            if(RTIOsapiSemaphore_take(_pongSemaphore, NULL)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout)
    {
        struct RTINtpTime blockDurationIn;
        RTINtpTime_packFromMillisec(blockDurationIn, 0, timeout);

        if(_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_take(_pongSemaphore, &blockDurationIn)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse()
    {
        if(_pongSemaphore != NULL) {
            if(RTIOsapiSemaphore_give(_pongSemaphore)
                    != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                fprintf(stderr,"Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount() {
        DDS_DataWriterProtocolStatus status;
        _writer->get_datawriter_protocol_status(status);
        return (unsigned int)status.pulled_sample_count;
    };

    void waitForAck(int sec, unsigned int nsec) {
        if (_isReliable) {
            DDS_Duration_t timeout = {sec, nsec};
            _writer->wait_for_acknowledgments(timeout);
        } else {
            perftest_cpp::MilliSleep(nsec / 1000000);
        }
    }
};

/* Dynamic Data equivalent function from RTIPublisher */
class RTIDynamicDataPublisher : public IMessagingWriter
{
  private:
    DDSDynamicDataWriter *_writer;
    DDS_DynamicData data;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    DDS_InstanceHandle_t *_instance_handles;
    RTIOsapiSemaphore *_pongSemaphore;
    long _instancesToBeWritten;
    int _last_message_size;
    bool _isReliable;

public:
    RTIDynamicDataPublisher(
            DDSDataWriter *writer,
            unsigned long num_instances,
            RTIOsapiSemaphore *pongSemaphore,
            DDS_TypeCode *typeCode,
            int instancesToBeWritten) :
            data(typeCode, DDS_DYNAMIC_DATA_PROPERTY_DEFAULT)

    {
        _writer = DDSDynamicDataWriter::narrow(writer);
        DDS_Octet key_octets[KEY_SIZE];
        DDS_ReturnCode_t retcode;

        _last_message_size = 0;
        _num_instances = num_instances;
        _instance_counter = 0;
        _instancesToBeWritten = instancesToBeWritten;
        _instance_handles = (DDS_InstanceHandle_t *) malloc(
                sizeof(DDS_InstanceHandle_t) * (num_instances + 1));
        if (_instance_handles == NULL) {
            fprintf(stderr, "_instance_handles malloc failed.\n");
        }
        _pongSemaphore = pongSemaphore;

        for (unsigned long i = 0; i < _num_instances; ++i) {
            for (int c = 0; c < KEY_SIZE; c++) {
                key_octets[c] = (unsigned char) (i >> c * 8);
            }
            retcode = data.set_octet_array(
                    "key",
                    DynamicDataMembersId::GetInstance().at("key"),
                    KEY_SIZE,
                    key_octets);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_octet_array(key) failed: %d.\n", retcode);
            }

            _instance_handles[i] = _writer->register_instance(data);
        }
        // Register the key of MAX_CFT_VALUE
        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (unsigned char)(MAX_CFT_VALUE >> c * 8);
        }
        retcode = data.set_octet_array(
                    "key",
                    DynamicDataMembersId::GetInstance().at("key"),
                    KEY_SIZE,
                    key_octets);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_octet_array(key) failed: %d.\n", retcode);
        }
        _instance_handles[_num_instances] = _writer->register_instance(data);

        DDS_DataWriterQos qos;
        _writer->get_qos(qos);
        _isReliable = (qos.reliability.kind == DDS_RELIABLE_RELIABILITY_QOS);
    }

    ~RTIDynamicDataPublisher() {
        Shutdown();
    }

    void Shutdown() {
        if (_writer->get_listener() != NULL) {
            delete(_writer->get_listener());
            _writer->set_listener(NULL);
        }

        free(_instance_handles);
    }

    void Flush()
    {
        _writer->flush();
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {
        DDS_ReturnCode_t retcode;
        DDS_Octet key_octets[KEY_SIZE];
        long key = 0;

        if (_last_message_size != message.size) {
            //Cannot use data.clear_member("bind_data") because:
            //DDS_DynamicData_clear_member:unsupported for non-sparse types
            data.clear_all_members();

            DDS_OctetSeq octetSeq;
            bool succeeded = octetSeq.from_array(
                    (DDS_Octet *) message.data,
                    message.size);
            if (!succeeded) {
                fprintf(stderr, "from_array() failed.\n");
            }
            retcode = data.set_octet_seq(
                    "bin_data",
                    DynamicDataMembersId::GetInstance().at("bin_data"),
                    octetSeq);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr, "set_octet_seq(bin_data) failed: %d.\n", retcode);
            }
            _last_message_size = message.size;
        }
        retcode = data.set_long(
                "entity_id",
                DynamicDataMembersId::GetInstance().at("entity_id"),
                message.entity_id);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(entity_id) failed: %d.\n", retcode);
        }
        retcode = data.set_ulong(
                "seq_num",
                DynamicDataMembersId::GetInstance().at("seq_num"),
                message.seq_num);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_ulong(seq_num) failed: %d.\n", retcode);
        }
        retcode = data.set_long(
                "timestamp_sec",
                DynamicDataMembersId::GetInstance().at("timestamp_sec"),
                message.timestamp_sec);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(timestamp_sec) failed: %d.\n", retcode);
        }
        retcode = data.set_ulong(
                "timestamp_usec",
                DynamicDataMembersId::GetInstance().at("timestamp_usec"),
                message.timestamp_usec);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_ulong(timestamp_usec) failed: %d.\n", retcode);
        }
        retcode = data.set_long(
                "latency_ping",
                DynamicDataMembersId::GetInstance().at("latency_ping"),
                message.latency_ping);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_long(latency_ping) failed: %d.\n", retcode);
        }

        if (!isCftWildCardKey) {
            if (_num_instances > 1) {
                if (_instancesToBeWritten == -1) {
                    key = _instance_counter++ % _num_instances;
                } else { // send sample to a specific subscriber
                    key = _instancesToBeWritten;
                }
            }
        } else {
            key = MAX_CFT_VALUE;
        }
        for (int c = 0; c < KEY_SIZE; c++) {
            key_octets[c] = (unsigned char) (key >> c * 8);
        }
        retcode = data.set_octet_array(
                "key",
                DynamicDataMembersId::GetInstance().at("key"),
                KEY_SIZE,
                key_octets);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "set_octet_array(key) failed: %d.\n", retcode);
        }

        if (!isCftWildCardKey) {
            retcode = _writer->write(data, _instance_handles[key]);
        } else {
            retcode = _writer->write(data, _instance_handles[_num_instances]);
        }

        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Write error %d.\n", retcode);
            return false;
        }

        return true;
    }

    void WaitForReaders(int numSubscribers) {
        DDS_PublicationMatchedStatus status;

        while (true) {
            DDS_ReturnCode_t retcode = _writer->get_publication_matched_status(
                    status);
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "WaitForReaders _writer->get_publication_matched_status "
                        "failed: %d.\n",
                        retcode);
            }
            if (status.current_count >= numSubscribers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }

    bool waitForPingResponse() {
        if (_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_take(_pongSemaphore, NULL)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    /* time out in milliseconds */
    bool waitForPingResponse(int timeout) {
        struct RTINtpTime blockDuration;
        RTINtpTime_packFromMillisec(blockDuration, 0, timeout);

        if (_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_take(_pongSemaphore, &blockDuration)
                    == RTI_OSAPI_SEMAPHORE_STATUS_ERROR) {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse() {
        if (_pongSemaphore != NULL) {
            if (RTIOsapiSemaphore_give(_pongSemaphore)
                    != RTI_OSAPI_SEMAPHORE_STATUS_OK) {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount() {
        DDS_DataWriterProtocolStatus status;
        _writer->get_datawriter_protocol_status(status);
        return (unsigned int)status.pulled_sample_count;
    };

    void waitForAck(int sec, unsigned int nsec) {
        if (_isReliable) {
            DDS_Duration_t timeout = {sec, nsec};
            _writer->wait_for_acknowledgments(timeout);
        } else {
            perftest_cpp::MilliSleep(nsec / 1000000);
        }
    }
};

/*********************************************************
 * ReceiverListener
 */
template <typename T>
class ReceiverListener : public DDSDataReaderListener
{
  private:

    typename T::Seq     _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage       _message;
    IMessagingCB     *_callback;

  public:

    ReceiverListener(IMessagingCB *callback): _message()
    {
        _callback = callback;
    }

    void on_data_available(DDSDataReader *reader)
    {

        typename T::DataReader *datareader;

        datareader = T::DataReader::narrow(reader);
        if (datareader == NULL)
        {
            fprintf(stderr,"DataReader narrow error.\n");
            return;
        }

        DDS_ReturnCode_t retcode = datareader->take(
                _data_seq, _info_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE);

        if (retcode == DDS_RETCODE_NO_DATA)
        {
            fprintf(stderr,"called back no data\n");
            return;
        }
        else if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Error during taking data %d\n", retcode);
            return;
        }

        int seq_length = _data_seq.length();
        for (int i = 0; i < seq_length; ++i)
        {
            if (_info_seq[i].valid_data)
            {
                _message.entity_id = _data_seq[i].entity_id;
                _message.seq_num = _data_seq[i].seq_num;
                _message.timestamp_sec = _data_seq[i].timestamp_sec;
                _message.timestamp_usec = _data_seq[i].timestamp_usec;
                _message.latency_ping = _data_seq[i].latency_ping;
                _message.size = _data_seq[i].bin_data.length();
                _message.data = (char *)_data_seq[i].bin_data.get_contiguous_bufferI();

                _callback->ProcessMessage(_message);

            }
        }

        retcode = datareader->return_loan(_data_seq, _info_seq);
        if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr,"Error during return loan %d.\n", retcode);
            fflush(stderr);
        }

    }

};

/* Dynamic Data equivalent function from ReceiverListener */
class DynamicDataReceiverListener : public DDSDataReaderListener
{
  private:
    DDS_DynamicDataSeq _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage _message;
    IMessagingCB *_callback;

  public:
    DynamicDataReceiverListener(IMessagingCB *callback): _message()
    {
        _callback = callback;
    }

    void on_data_available(DDSDataReader *reader)
    {
        DDSDynamicDataReader *datareader = DDSDynamicDataReader::narrow(reader);
        if (datareader == NULL) {
            fprintf(stderr, "DataReader narrow error.\n");
            return;
        }

        DDS_ReturnCode_t retcode = datareader->take(
                _data_seq,
                _info_seq,
                DDS_LENGTH_UNLIMITED,
                DDS_ANY_SAMPLE_STATE,
                DDS_ANY_VIEW_STATE,
                DDS_ANY_INSTANCE_STATE);

        if (retcode == DDS_RETCODE_NO_DATA) {
            fprintf(stderr, "No data received\n");
            return;
        } else if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Error while taking data %d\n", retcode);
            return;
        }

        DDS_OctetSeq octetSeq;
        for (int i = 0; i < _data_seq.length(); ++i) {
            if (_info_seq[i].valid_data) {
                retcode = _data_seq[i].get_long(
                        _message.entity_id,
                        "entity_id",
                        DynamicDataMembersId::GetInstance().at("entity_id"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(entity_id) failed: %d.\n",
                            retcode);
                    _message.entity_id = 0;
                }
                retcode = _data_seq[i].get_ulong(
                        _message.seq_num,
                        "seq_num",
                        DynamicDataMembersId::GetInstance().at("seq_num"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_ulong(seq_num) failed: %d.\n",
                            retcode);
                    _message.seq_num = 0;
                }
                retcode = _data_seq[i].get_long(
                        _message.timestamp_sec,
                        "timestamp_sec",
                        DynamicDataMembersId::GetInstance().at("timestamp_sec"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(timestamp_sec) failed: %d.\n",
                            retcode);
                    _message.timestamp_sec = 0;
                }
                retcode = _data_seq[i].get_ulong(
                        _message.timestamp_usec,
                        "timestamp_usec",
                        DynamicDataMembersId::GetInstance().at("timestamp_usec"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_ulong(timestamp_usec) failed: %d.\n",
                            retcode);
                    _message.timestamp_usec = 0;
                }
                retcode = _data_seq[i].get_long(
                        _message.latency_ping,
                        "latency_ping",
                        DynamicDataMembersId::GetInstance().at("latency_ping"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_long(latency_ping) failed: %d.\n",
                            retcode);
                    _message.latency_ping = 0;
                }
                retcode = _data_seq[i].get_octet_seq(
                        octetSeq,
                        "bin_data",
                        DynamicDataMembersId::GetInstance().at("bin_data"));
                if (retcode != DDS_RETCODE_OK) {
                    fprintf(stderr,
                            "on_data_available() get_octet_seq(bin_data) failed: %d.\n",
                            retcode);
                }
                _message.size = octetSeq.length(); // size comming from bin_data
                _message.data = (char *)octetSeq.get_contiguous_buffer();

                _callback->ProcessMessage(_message);
            }
        }

        retcode = datareader->return_loan(_data_seq, _info_seq);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "Error during return loan %d.\n", retcode);
            fflush(stderr);
        }
    }
};

/*********************************************************
 * RTISubscriber
 */
template <typename T>
class RTISubscriber : public IMessagingReader
{
  private:
    typename T::DataReader *_reader;
    typename T::Seq         _data_seq;
    DDS_SampleInfoSeq     _info_seq;
    TestMessage           _message;
    DDSWaitSet           *_waitset;
    DDSConditionSeq       _active_conditions;

    int      _data_idx;
    bool      _no_data;

  public:

    RTISubscriber(DDSDataReader *reader): _message()
    {
        _reader = T::DataReader::narrow(reader);
        _data_idx = 0;
        _no_data = false;

        // null listener means using receive thread
        if (_reader->get_listener() == NULL) {
            DDS_WaitSetProperty_t property;
            property.max_event_count         = RTIDDSImpl<T>::_WaitsetEventCount;
            property.max_event_delay.sec     = (int)RTIDDSImpl<T>::_WaitsetDelayUsec / 1000000;
            property.max_event_delay.nanosec = (RTIDDSImpl<T>::_WaitsetDelayUsec % 1000000) * 1000;

            _waitset = new DDSWaitSet(property);

            DDSStatusCondition *reader_status;
            reader_status = reader->get_statuscondition();
            reader_status->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
            _waitset->attach_condition(reader_status);
        }
    }

    ~RTISubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_reader->get_listener() != NULL) {
            delete(_reader->get_listener());
            _reader->set_listener(NULL);
        }
        // loan may be outstanding during shutdown
        _reader->return_loan(_data_seq, _info_seq);
    }

    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        int seq_length;

        while (true) {

            // no outstanding reads
            if (_no_data)
            {
                _waitset->wait(_active_conditions, DDS_DURATION_INFINITE);

                if (_active_conditions.length() == 0)
                {
                    //printf("Read thread woke up but no data\n.");
                    //return NULL;
                    continue;
                }

                retcode = _reader->take(
                    _data_seq, _info_seq,
                    DDS_LENGTH_UNLIMITED,
                    DDS_ANY_SAMPLE_STATE,
                    DDS_ANY_VIEW_STATE,
                    DDS_ANY_INSTANCE_STATE);
                if (retcode == DDS_RETCODE_NO_DATA)
                {
                    //printf("Called back no data.\n");
                    //return NULL;
                    continue;
                }
                else if (retcode != DDS_RETCODE_OK)
                {
                    fprintf(stderr,"Error during taking data %d.\n", retcode);
                    return NULL;
                }

                _data_idx = 0;
                _no_data = false;
            }

            seq_length = _data_seq.length();
            // check to see if hit end condition
            if (_data_idx == seq_length)
            {
                _reader->return_loan(_data_seq, _info_seq);
                _no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (_info_seq[_data_idx].valid_data == false)
                        && (++_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (_data_idx == seq_length) {
                continue;
            }

            _message.entity_id = _data_seq[_data_idx].entity_id;
            _message.seq_num = _data_seq[_data_idx].seq_num;
            _message.timestamp_sec = _data_seq[_data_idx].timestamp_sec;
            _message.timestamp_usec = _data_seq[_data_idx].timestamp_usec;
            _message.latency_ping = _data_seq[_data_idx].latency_ping;
            _message.size = _data_seq[_data_idx].bin_data.length();
            _message.data = (char *)_data_seq[_data_idx].bin_data.get_contiguous_bufferI();

            ++_data_idx;

            return &_message;
        }
    }

    void WaitForWriters(int numPublishers)
    {
        DDS_SubscriptionMatchedStatus status;

        while (true)
        {
            _reader->get_subscription_matched_status(status);

            if (status.current_count >= numPublishers)
            {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }
};

/* Dynamic Data equivalent function from RTISubscriber */
template <typename T>
class RTIDynamicDataSubscriber : public IMessagingReader
{
  private:
    DDSDynamicDataReader *_reader;
    DDS_DynamicDataSeq _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage _message;
    DDSWaitSet *_waitset;
    DDSConditionSeq _active_conditions;

    int _data_idx;
    bool _no_data;

  public:

    RTIDynamicDataSubscriber(DDSDataReader *reader): _message()
    {
        _reader = DDSDynamicDataReader::narrow(reader);
        if (_reader == NULL) {
            fprintf(stderr,"DDSDynamicDataReader::narrow(reader) error.\n");
        }
        _data_idx = 0;
        _no_data = false;

        // null listener means using receive thread
        if (_reader->get_listener() == NULL) {

            DDS_WaitSetProperty_t property;
            property.max_event_count = RTIDDSImpl<T>::_WaitsetEventCount;
            property.max_event_delay.sec =
                    (int) RTIDDSImpl<T>::_WaitsetDelayUsec / 1000000;
            property.max_event_delay.nanosec = (RTIDDSImpl<T>::_WaitsetDelayUsec
                    % 1000000) * 1000;
            _waitset = new DDSWaitSet(property);
            DDSStatusCondition *reader_status;
            reader_status = reader->get_statuscondition();
            reader_status->set_enabled_statuses(DDS_DATA_AVAILABLE_STATUS);
            _waitset->attach_condition(reader_status);
        }
    }

    ~RTIDynamicDataSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
        if (_reader->get_listener() != NULL) {
            delete(_reader->get_listener());
            _reader->set_listener(NULL);
        }
        // loan may be outstanding during shutdown
        _reader->return_loan(_data_seq, _info_seq);
    }

    TestMessage *ReceiveMessage()
    {
        DDS_ReturnCode_t retcode;
        int seq_length;
        DDS_OctetSeq octetSeq;

        while (true) {
            // no outstanding reads
            if (_no_data)
            {
                _waitset->wait(_active_conditions, DDS_DURATION_INFINITE);

                if (_active_conditions.length() == 0)
                {
                    continue;
                }

                retcode = _reader->take(
                        _data_seq,
                        _info_seq,
                        DDS_LENGTH_UNLIMITED,
                        DDS_ANY_SAMPLE_STATE,
                        DDS_ANY_VIEW_STATE,
                        DDS_ANY_INSTANCE_STATE);
                if (retcode == DDS_RETCODE_NO_DATA)
                {
                    //printf("Called back no data.\n");
                    //return NULL;
                    continue;
                }
                else if (retcode != DDS_RETCODE_OK)
                {
                    fprintf(stderr,"Error during taking data %d.\n", retcode);
                    return NULL;
                }

                _data_idx = 0;
                _no_data = false;
            }

            seq_length = _data_seq.length();
            // check to see if hit end condition
            if (_data_idx == seq_length)
            {
                _reader->return_loan(_data_seq, _info_seq);
                _no_data = true;
                continue;
            }

            // skip non-valid data
            while ( (_info_seq[_data_idx].valid_data == false) &&
                    (++_data_idx < seq_length)){
                //No operation required
            }

            // may have hit end condition
            if (_data_idx == seq_length) {
                continue;
            }

            retcode = _data_seq[_data_idx].get_long(
                _message.entity_id,
                "entity_id",
                DynamicDataMembersId::GetInstance().at("entity_id"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(entity_id) failed: %d.\n",
                        retcode);
                _message.entity_id = 0;
            }
            retcode = _data_seq[_data_idx].get_ulong(
                _message.seq_num,
                "seq_num",
                DynamicDataMembersId::GetInstance().at("seq_num"));
            if (retcode != DDS_RETCODE_OK){
                fprintf(stderr,
                        "ReceiveMessage() get_ulong(seq_num) failed: %d.\n",
                        retcode);
                _message.seq_num = 0;
            }
            retcode = _data_seq[_data_idx].get_long(
                _message.timestamp_sec,
                "timestamp_sec",
                DynamicDataMembersId::GetInstance().at("timestamp_sec"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(timestamp_sec) failed: %d.\n",
                        retcode);
                _message.timestamp_sec = 0;
            }
            retcode = _data_seq[_data_idx].get_ulong(
                _message.timestamp_usec,
                "timestamp_usec",
                DynamicDataMembersId::GetInstance().at("timestamp_usec"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_ulong(timestamp_usec) failed: %d.\n",
                        retcode);
                _message.timestamp_usec = 0;
            }
            retcode = _data_seq[_data_idx].get_long(
                _message.latency_ping,
                "latency_ping",
                DynamicDataMembersId::GetInstance().at("latency_ping"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_long(latency_ping) failed: %d.\n",
                        retcode);
                _message.latency_ping = 0;
            }
            retcode = _data_seq[_data_idx].get_octet_seq(
                octetSeq,
                "bin_data",
                DynamicDataMembersId::GetInstance().at("bin_data"));
            if (retcode != DDS_RETCODE_OK) {
                fprintf(stderr,
                        "ReceiveMessage() get_octet_seq(bin_data) failed: %d.\n",
                        retcode);
            }
            _message.size = octetSeq.length();
            _message.data = (char *)octetSeq.get_contiguous_buffer();

            ++_data_idx;

            return &_message;
        }
    }

    void WaitForWriters(int numPublishers)
    {
        DDS_SubscriptionMatchedStatus status;

        while (true) {
            _reader->get_subscription_matched_status(status);

            if (status.current_count >= numPublishers) {
                break;
            }
            perftest_cpp::MilliSleep(1000);
        }
    }
};

/*******************************************************************************
 * SECURITY PLUGIN
 */
#ifdef RTI_SECURE_PERFTEST

template<typename T>
bool RTIDDSImpl<T>::configureSecurePlugin(DDS_DomainParticipantQos& dpQos) {
    // configure use of security plugins, based on provided arguments

    DDS_ReturnCode_t retcode;

    // load plugin
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.load_plugin",
            "com.rti.serv.secure",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.load_plugin\n");
        return false;
    }

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    retcode = DDSPropertyQosPolicyHelper::assert_property(
            dpQos.property,
            "com.rti.serv.secure.create_function",
            "RTI_Security_PluginSuite_create",
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.create_function\n");
        return false;
    }


    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.library",
            _secureLibrary.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property com.rti.serv.secure.library\n");
        return false;
    }

  #else // Static library linking

    retcode = DDSPropertyQosPolicyHelper::assert_pointer_property(
            dpQos.property,
            "com.rti.serv.secure.create_function_ptr",
            (void *) RTI_Security_PluginSuite_create);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add pointer_property "
                "com.rti.serv.secure.create_function_ptr\n");
        return false;
    }

  #endif

    /*
     * Below, we are using com.rti.serv.secure properties in order to be
     * backward compatible with RTI Connext DDS 5.3.0 and below. Later versions
     * use the properties that are specified in the DDS Security specification
     * (see also the RTI Security Plugins Getting Started Guide). However,
     * later versions still support the legacy properties as an alternative.
     */

    // check if governance file provided
    if (_secureGovernanceFile.empty()) {
        // choose a pre-built governance file
        _secureGovernanceFile = "./resource/secure/signed_PerftestGovernance_";
        if (_secureIsDiscoveryEncrypted) {
            _secureGovernanceFile += "Discovery";
        }

        if (_secureIsSigned) {
            _secureGovernanceFile += "Sign";
        }

        if (_secureIsDataEncrypted && _secureIsSMEncrypted) {
            _secureGovernanceFile += "EncryptBoth";
        } else if (_secureIsDataEncrypted) {
            _secureGovernanceFile += "EncryptData";
        } else if (_secureIsSMEncrypted) {
            _secureGovernanceFile += "EncryptSubmessage";
        }

        _secureGovernanceFile += ".xml";

        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                _secureGovernanceFile.c_str(),
                false);
    } else {
        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.governance_file",
                _secureGovernanceFile.c_str(),
                false);
    }
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.governance_file\n");
        return false;
    }

    // permissions file
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.access_control.permissions_file",
            _securePermissionsFile.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.permissions_file\n");
        return false;
    }

    // permissions authority file
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.access_control.permissions_authority_file",
            _secureCertAuthorityFile.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.access_control.permissions_authority_file\n");
        return false;
    }

    // certificate authority
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.ca_file",
            _secureCertAuthorityFile.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.ca_file\n");
        return false;
    }

    // public key
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.certificate_file",
            _secureCertificateFile.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.certificate_file\n");
        return false;
    }

    // private key
    retcode = DDSPropertyQosPolicyHelper::add_property(
            dpQos.property,
            "com.rti.serv.secure.authentication.private_key_file",
            _securePrivateKeyFile.c_str(),
            false);
    if (retcode != DDS_RETCODE_OK) {
        printf("Failed to add property "
                "com.rti.serv.secure.authentication.private_key_file\n");
        return false;
    }

    if (_secureDebugLevel != -1) {
        char buf[16];
        sprintf(buf, "%d", _secureDebugLevel);
        retcode = DDSPropertyQosPolicyHelper::add_property(
                dpQos.property,
                "com.rti.serv.secure.logging.log_level",
                buf,
                false);
        if (retcode != DDS_RETCODE_OK) {
            printf("Failed to add property "
                    "com.rti.serv.secure.logging.log_level\n");
            return false;
        }
    }

    return true;
}

template <typename T>
bool RTIDDSImpl<T>::validateSecureArgs()
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

    return true;
}

template <typename T>
std::string RTIDDSImpl<T>::printSecureArgs()
{
    std::ostringstream stringStream;
    stringStream << "Secure Configuration:\n";

    stringStream << "\tEncrypt discovery: ";
    if (_secureIsDiscoveryEncrypted) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt topic (user) data: ";
    if (_secureIsDataEncrypted) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tEncrypt submessage: ";
    if (_secureIsSMEncrypted) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tSign data: ";
    if (_secureIsSigned) {
        stringStream << "True\n";
    } else {
        stringStream << "False\n";
    }

    stringStream << "\tGovernance file: ";
    if (_secureGovernanceFile.empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureGovernanceFile << "\n";
    }

    stringStream << "\tPermissions file: ";
    if (_securePermissionsFile.empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _securePermissionsFile << "\n";
    }

    stringStream << "\tPrivate key file: ";
    if (_securePrivateKeyFile.empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _securePrivateKeyFile << "\n";
    }

    stringStream << "\tCertificate file: ";
    if (_secureCertificateFile.empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureCertificateFile << "\n";
    }

    stringStream << "\tCertificate authority file: ";
    if (_secureCertAuthorityFile.empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureCertAuthorityFile << "\n";
    }

    stringStream << "\tPlugin library: ";
    if (_secureLibrary.empty()) {
        stringStream << "Not Specified\n";
    } else {
        stringStream << _secureLibrary << "\n";
    }

    if (_secureDebugLevel != -1) {
        stringStream << "\tDebug level: " <<  _secureDebugLevel << "\n";
    }

    return stringStream.str();
}

#endif

/*********************************************************
 * Initialize
 */
template <typename T>
bool RTIDDSImpl<T>::Initialize(int argc, char *argv[])
{
    DDS_DomainParticipantQos qos;
    DDS_DomainParticipantFactoryQos factory_qos;
    DomainListener *listener = new DomainListener();

    // Register _loggerDevice
    if (!NDDSConfigLogger::get_instance()->set_output_device(&_loggerDevice)) {
        fprintf(stderr,"Failed set_output_device for Logger.\n");
        return false;
    }

    _factory = DDSDomainParticipantFactory::get_instance();

    if (!ParseConfig(argc, argv))
    {
        return false;
    }

    // only if we run the latency test we need to wait
    // for pongs after sending pings
    _pongSemaphore = _LatencyTest ?
        RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL) :
        NULL;

    // setup the QOS profile file to be loaded
    _factory->get_qos(factory_qos);
    if (_UseXmlQos) {
        factory_qos.profile.url_profile.ensure_length(1, 1);
        factory_qos.profile.url_profile[0] = DDS_String_dup(_ProfileFile);
    } else {
        factory_qos.profile.string_profile.from_array(
                PERFTEST_QOS_STRING,
                PERFTEST_QOS_STRING_SIZE);
    }
    _factory->set_qos(factory_qos);

    if (_factory->reload_profiles() != DDS_RETCODE_OK)
    {
        fprintf(stderr,"Problem opening QOS profiles file %s.\n", _ProfileFile);
        return false;
    }

    if (_factory->set_default_library(_ProfileLibraryName) != DDS_RETCODE_OK)
    {
        fprintf(stderr,"No QOS Library named \"%s\" found in %s.\n",
               _ProfileLibraryName, _ProfileFile);
        return false;
    }

    // Configure DDSDomainParticipant QOS
    if (_factory->get_participant_qos_from_profile(
            qos,
            _ProfileLibraryName,
            "BaseProfileQos") != DDS_RETCODE_OK) {
        fprintf(
                stderr,
                "Problem setting QoS Library \"%s::BaseProfileQos\" for participant_qos.\n",
                _ProfileLibraryName);
    }

  #ifdef RTI_SECURE_PERFTEST
    if (_secureUseSecure) {
        // validate arguments
        if (!validateSecureArgs()) {
            fprintf(stderr, "Failed to configure security plugins\n");
            return false;
        }
        // configure
        if (!configureSecurePlugin(qos)) {
            fprintf(stderr, "Failed to configure security plugins\n");
            return false;
        }
    }
  #endif

    // set initial peers and not use multicast
    if (_peer_host_count > 0) {
        qos.discovery.initial_peers.from_array(
            (const char **)_peer_host,
            _peer_host_count);
        qos.discovery.multicast_receive_addresses.length(0);
    }

    if (!configureTransport(_transport, qos)){
        return false;
    };

    if (_AutoThrottle) {
        DDSPropertyQosPolicyHelper::add_property(qos.property,
                "dds.domain_participant.auto_throttle.enable", "true", false);
    }

    // Creates the participant
    _participant = _factory->create_participant(
        _DomainID, qos, listener,
        DDS_INCONSISTENT_TOPIC_STATUS |
        DDS_OFFERED_INCOMPATIBLE_QOS_STATUS |
        DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);

    if (_participant == NULL || _loggerDevice.checkShmemErrors()) {
        if (_loggerDevice.checkShmemErrors()) {
            fprintf(
                    stderr,
                    "The participant creation failed due to issues in the Shared Memory configuration of your OS.\n"
                    "For more information about how to configure Shared Memory see: http://community.rti.com/kb/osx510 \n"
                    "If you want to skip the use of Shared memory in RTI Perftest, "
                    "specify the transport using \"-transport <kind>\", e.g. \"-transport UDPv4\".\n");
        }
        fprintf(stderr,"Problem creating participant.\n");
        return false;
    }

    // Register the types and create the topics
    if (!_isDynamicData) {
        T::TypeSupport::register_type(_participant, _typename);
    } else {
        DDSDynamicDataTypeSupport* dynamicDataTypeSupportObject =
                new DDSDynamicDataTypeSupport(
                        T::TypeSupport::get_typecode(),
                        DDS_DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
        dynamicDataTypeSupportObject->register_type(_participant, _typename);
    }

    // Create the DDSPublisher and DDSSubscriber
    _publisher = _participant->create_publisher_with_profile(
            _ProfileLibraryName,
            "BaseProfileQos",
            NULL,
            DDS_STATUS_MASK_NONE);
    if (_publisher == NULL)
    {
        fprintf(stderr,"Problem creating publisher.\n");
        return false;
    }

    _subscriber = _participant->create_subscriber_with_profile(
            _ProfileLibraryName,
            "BaseProfileQos",
            NULL,
            DDS_STATUS_MASK_NONE);
    if (_subscriber == NULL)
    {
        fprintf(stderr,"Problem creating subscriber.\n");
        return false;
    }

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
            (unsigned long) _SendQueueSize);

    /*
     * If we are using batching we need to take into account tha the Send Queue
     * will be per-batch, therefore for the number of samples:
     */
    if (_BatchSize > 0) {
        initializeSampleCount = (std::max)(
                _SendQueueSize * (_BatchSize / _DataLen),
                initializeSampleCount);
    }

    return initializeSampleCount;
}

/*********************************************************
 * CreateWriter
 */
template <typename T>
IMessagingWriter *RTIDDSImpl<T>::CreateWriter(const char *topic_name)
{
    DDSDataWriter *writer = NULL;
    DDS_DataWriterQos dw_qos;
    std::string qos_profile;

    DDSTopic *topic = _participant->create_topic(
                       topic_name,
                       _typename,
                       DDS_TOPIC_QOS_DEFAULT,
                       NULL,
                       DDS_STATUS_MASK_NONE);

    if (topic == NULL) {
        fprintf(stderr,"Problem creating topic %s.\n", topic_name);
        return NULL;
    }

    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0) {
        qos_profile = "ThroughputQos";
    } else if (strcmp(topic_name, LATENCY_TOPIC_NAME) == 0) {
        qos_profile = "LatencyQos";
    } else if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) == 0) {
        qos_profile = "AnnouncementQos";
    } else {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME,
                LATENCY_TOPIC_NAME,
                ANNOUNCEMENT_TOPIC_NAME);
        return NULL;
    }

    if (_factory->get_datawriter_qos_from_profile(dw_qos, _ProfileLibraryName,
            qos_profile.c_str())
        != DDS_RETCODE_OK) {
        fprintf(stderr,"No QOS Profile named \"%s\" found in QOS Library \"%s\" in file %s.\n",
                qos_profile.c_str(), _ProfileLibraryName, _ProfileFile);
        return NULL;
    }

    if (!_UsePositiveAcks
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dw_qos.protocol.disable_positive_acks = true;
        if (_KeepDurationUsec != -1) {
            dw_qos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration =
                DDS_Duration_t::from_micros(_KeepDurationUsec);
        }
    }

    if (_isLargeData || _IsAsynchronous) {
        dw_qos.publish_mode.kind = DDS_ASYNCHRONOUS_PUBLISH_MODE_QOS;
        if (_FlowControllerCustom != "default") {
            dw_qos.publish_mode.flow_controller_name =
                    DDS_String_dup(("dds.flow_controller.token_bucket."+_FlowControllerCustom).c_str());
        }
    }

    // only force reliability on throughput/latency topics
    if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) != 0) {
        if (_IsReliable) {
            // default: use the setting specified in the qos profile
            // dw_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }
        else {
            // override to best-effort
            dw_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

    // These QOS's are only set for the Throughput datawriter
    if (qos_profile == "ThroughputQos") {

        if (_transport.useMulticast) {
            dw_qos.protocol.rtps_reliable_writer.enable_multicast_periodic_heartbeat =
                    RTI_TRUE;
        }

        if (_BatchSize > 0) {
            dw_qos.batch.enable = true;
            dw_qos.batch.max_data_bytes = _BatchSize;
            dw_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos.writer_resource_limits.max_batches = _SendQueueSize;
        } else {
            dw_qos.resource_limits.max_samples = _SendQueueSize;
        }

        if (_HeartbeatPeriod.sec > 0 || _HeartbeatPeriod.nanosec > 0) {
            // set the heartbeat_period
            dw_qos.protocol.rtps_reliable_writer.heartbeat_period =
                _HeartbeatPeriod;
            // make the late joiner heartbeat compatible
            dw_qos.protocol.rtps_reliable_writer.late_joiner_heartbeat_period =
                _HeartbeatPeriod;
        }

        if (_FastHeartbeatPeriod.sec > 0 || _FastHeartbeatPeriod.nanosec > 0) {
            // set the fast_heartbeat_period
            dw_qos.protocol.rtps_reliable_writer.fast_heartbeat_period =
                _FastHeartbeatPeriod;
        }

        if (_AutoThrottle) {
            DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
                    "dds.data_writer.auto_throttle.enable", "true", false);
        }

        if (_TurboMode) {
            DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
                    "dds.data_writer.enable_turbo_mode", "true", false);
            dw_qos.batch.enable = false;
            dw_qos.resource_limits.max_samples = DDS_LENGTH_UNLIMITED;
            dw_qos.writer_resource_limits.max_batches = _SendQueueSize;
        }

        dw_qos.resource_limits.initial_samples = _SendQueueSize;
        dw_qos.resource_limits.max_samples_per_instance
            = dw_qos.resource_limits.max_samples;

        dw_qos.durability.kind = (DDS_DurabilityQosPolicyKind)_Durability;
        dw_qos.durability.direct_communication = _DirectCommunication;

        dw_qos.protocol.rtps_reliable_writer.heartbeats_per_max_samples = _SendQueueSize / 10;

        dw_qos.protocol.rtps_reliable_writer.low_watermark = _SendQueueSize * 1 / 10;
        dw_qos.protocol.rtps_reliable_writer.high_watermark = _SendQueueSize * 9 / 10;

        /*
         * If _SendQueueSize is 1 low watermark and high watermark would both be
         * 0, which would cause the middleware to fail. So instead we set the
         * high watermark to the low watermark + 1 in such case.
         */
        if (dw_qos.protocol.rtps_reliable_writer.high_watermark
            == dw_qos.protocol.rtps_reliable_writer.low_watermark) {
            dw_qos.protocol.rtps_reliable_writer.high_watermark =
                    dw_qos.protocol.rtps_reliable_writer.low_watermark + 1;
        }

        dw_qos.protocol.rtps_reliable_writer.max_send_window_size = _SendQueueSize;
        dw_qos.protocol.rtps_reliable_writer.min_send_window_size = _SendQueueSize;
    }

    if (qos_profile == "LatencyQos"
            && !_DirectCommunication
            && (_Durability == DDS_TRANSIENT_DURABILITY_QOS
                || _Durability == DDS_PERSISTENT_DURABILITY_QOS)) {
        dw_qos.durability.kind = (DDS_DurabilityQosPolicyKind)_Durability;
        dw_qos.durability.direct_communication = _DirectCommunication;
    }

    dw_qos.resource_limits.max_instances = _InstanceCount + 1; // One extra for MAX_CFT_VALUE
    dw_qos.resource_limits.initial_instances = _InstanceCount +1;

    if (_useUnbounded > 0) {
        char buf[10];
        sprintf(buf, "%lu", _useUnbounded);
        DDSPropertyQosPolicyHelper::add_property(dw_qos.property,
               "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
               buf, false);
    }

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            dw_qos.resource_limits.instance_hash_buckets =
                _InstanceHashBuckets;
        } else {
            dw_qos.resource_limits.instance_hash_buckets = _InstanceCount;
        }
    }

    writer = _publisher->create_datawriter(
        topic, dw_qos, NULL,
        DDS_STATUS_MASK_NONE);

    if (writer == NULL)
    {
        fprintf(stderr,"Problem creating writer.\n");
        return NULL;
    }

    if (!_isDynamicData) {
        return new RTIPublisher<T>(writer, _InstanceCount, _pongSemaphore, _instancesToBeWritten);
    } else {
        return new RTIDynamicDataPublisher(writer, _InstanceCount, _pongSemaphore, T::TypeSupport::get_typecode(), _instancesToBeWritten);
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
DDSTopicDescription *RTIDDSImpl<T>::CreateCft(
        const char *topic_name,
        DDSTopic *topic)
{
    std::string condition;
    DDS_StringSeq parameters(2 * KEY_SIZE);
    if (_CFTRange[0] == _CFTRange[1]) { // If same elements, no range
        printf("CFT enabled for instance: '%d' \n",_CFTRange[0]);
        char cft_param[KEY_SIZE][128];
        for (int i = 0; i < KEY_SIZE ; i++) {
            sprintf(cft_param[i],"%d", (unsigned char)(_CFTRange[0] >> i * 8));
        }
        const char* param_list[] = { cft_param[0], cft_param[1], cft_param[2], cft_param[3]};
        parameters.from_array(param_list, KEY_SIZE);
        condition = "(%0 = key[0] AND %1 = key[1] AND %2 = key[2] AND %3 = key[3]) OR"
                "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";
    } else { // If range
        printf("CFT enabled for instance range: [%d,%d] \n",_CFTRange[0],_CFTRange[1]);
        char cft_param[2 * KEY_SIZE][128];
        for (int i = 0; i < 2 * KEY_SIZE ; i++ ) {
            if ( i < KEY_SIZE ) {
                sprintf(cft_param[i],"%d", (unsigned char)(_CFTRange[0] >> i * 8));
            } else { // KEY_SIZE < i < KEY_SIZE * 2
                sprintf(cft_param[i],"%d", (unsigned char)(_CFTRange[1] >> i * 8));
            }
        }
        const char* param_list[] = { cft_param[0], cft_param[1],
            cft_param[2], cft_param[3],cft_param[4],
            cft_param[5], cft_param[6], cft_param[7]
        };
        parameters.from_array(param_list, 2 * KEY_SIZE);
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
    return _participant->create_contentfilteredtopic(
            topic_name,
            topic,
            condition.c_str(),
            parameters);
}

/*********************************************************
 * CreateReader
 */
template <typename T>
IMessagingReader *RTIDDSImpl<T>::CreateReader(
        const char *topic_name,
        IMessagingCB *callback)
{
    DDSDataReader *reader = NULL;
    DDS_DataReaderQos dr_qos;
    std::string qos_profile;
    DDSTopicDescription* topic_desc = NULL; // Used to create the DDS DataReader

    DDSTopic *topic = _participant->create_topic(
                       topic_name, _typename,
                       DDS_TOPIC_QOS_DEFAULT, NULL,
                       DDS_STATUS_MASK_NONE);

    if (topic == NULL) {
        fprintf(stderr,"Problem creating topic %s.\n", topic_name);
        return NULL;
    }
    topic_desc = topic;

    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0) {
        qos_profile = "ThroughputQos";
    } else if (strcmp(topic_name, LATENCY_TOPIC_NAME) == 0) {
        qos_profile = "LatencyQos";
    } else if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) == 0) {
        qos_profile = "AnnouncementQos";
    } else {
        fprintf(stderr,
                "topic name must either be %s or %s or %s.\n",
                THROUGHPUT_TOPIC_NAME,
                LATENCY_TOPIC_NAME,
                ANNOUNCEMENT_TOPIC_NAME);
        return NULL;
    }

    if (_factory->get_datareader_qos_from_profile(dr_qos, _ProfileLibraryName, qos_profile.c_str())
        != DDS_RETCODE_OK)
    {
        fprintf(stderr,"No QOS Profile named \"%s\" found in QOS Library \"%s\" in file %s.\n",
                qos_profile.c_str(), _ProfileLibraryName, _ProfileFile);
        return NULL;
    }

    // only force reliability on throughput/latency topics
    if (strcmp(topic_name, ANNOUNCEMENT_TOPIC_NAME) != 0)
    {
        if (_IsReliable)
        {
            dr_qos.reliability.kind = DDS_RELIABLE_RELIABILITY_QOS;
        }
        else
        {
            dr_qos.reliability.kind = DDS_BEST_EFFORT_RELIABILITY_QOS;
        }
    }

    if (!_UsePositiveAcks
            && (qos_profile == "ThroughputQos" || qos_profile == "LatencyQos")) {
        dr_qos.protocol.disable_positive_acks = true;
    }

    // only apply durability on Throughput datareader
    if (qos_profile == "ThroughputQos"
            || (qos_profile == "LatencyQos"
                    && !_DirectCommunication
                    && (_Durability == DDS_TRANSIENT_DURABILITY_QOS
                            || _Durability == DDS_PERSISTENT_DURABILITY_QOS)))
    {
        dr_qos.durability.kind = (DDS_DurabilityQosPolicyKind) _Durability;
        dr_qos.durability.direct_communication = _DirectCommunication;
    }

    dr_qos.resource_limits.initial_instances = _InstanceCount + 1;
    if (_InstanceMaxCountReader != DDS_LENGTH_UNLIMITED) {
        _InstanceMaxCountReader++;
    }
    dr_qos.resource_limits.max_instances = _InstanceMaxCountReader;

    if (_InstanceCount > 1) {
        if (_InstanceHashBuckets > 0) {
            dr_qos.resource_limits.instance_hash_buckets =
                _InstanceHashBuckets;
        } else {
            dr_qos.resource_limits.instance_hash_buckets = _InstanceCount;
        }
    }

    if (_transport.allowsMulticast()) {
        dr_qos.multicast.value.ensure_length(1, 1);
        dr_qos.multicast.value[0].receive_address = DDS_String_dup(
                _transport.getMulticastAddr(topic_name).c_str());

        if (dr_qos.multicast.value[0].receive_address == NULL) {
            fprintf(stderr,
                    "topic name must either be %s or %s or %s.\n",
                    THROUGHPUT_TOPIC_NAME,
                    LATENCY_TOPIC_NAME,
                    ANNOUNCEMENT_TOPIC_NAME);
            return NULL;
        }

        dr_qos.multicast.value[0].receive_port = 0;
        dr_qos.multicast.value[0].transports.length(0);
    }

    if (_useUnbounded > 0) {
        char buf[10];
        sprintf(buf, "%lu", _useUnbounded);
        DDSPropertyQosPolicyHelper::add_property(dr_qos.property,
                "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                buf, false);
    }

    /* Create CFT Topic */
    if (strcmp(topic_name, THROUGHPUT_TOPIC_NAME) == 0 && _useCft) {
        topic_desc = CreateCft(topic_name, topic);
        if (topic_desc == NULL) {
            printf("Create_contentfilteredtopic error\n");
            return NULL;
        }
    }

    if (callback != NULL) {

        if (!_isDynamicData) {
            reader = _subscriber->create_datareader(
                    topic_desc,
                    dr_qos,
                    new ReceiverListener<T>(callback),
                    DDS_DATA_AVAILABLE_STATUS);
        } else {
            reader = _subscriber->create_datareader(
                    topic_desc,
                    dr_qos,
                    new DynamicDataReceiverListener(callback),
                    DDS_DATA_AVAILABLE_STATUS);
        }

    } else {
        reader = _subscriber->create_datareader(
                topic_desc,
                dr_qos,
                NULL,
                DDS_STATUS_MASK_NONE);
    }

    if (reader == NULL)
    {
        fprintf(stderr,"Problem creating reader.\n");
        return NULL;
    }

    if (!strcmp(topic_name, THROUGHPUT_TOPIC_NAME) ||
        !strcmp(topic_name, LATENCY_TOPIC_NAME)) {
        _reader = reader;
    }

    if (!_isDynamicData) {
        return new RTISubscriber<T>(reader);
    } else {
        return new RTIDynamicDataSubscriber<T>(reader);
    }
}

#ifdef RTI_WIN32
  #pragma warning(pop)
#endif
