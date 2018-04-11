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

int RTISocketImpl::_WaitsetEventCount = 5;
unsigned int RTISocketImpl::_WaitsetDelayUsec = 100;


std::string valid_flow_controller[] = {"default", "1Gbps", "10Gbps"};

/*********************************************************
 * Shutdown
 */
void RTISocketImpl::Shutdown()
{
    //delete array of peers
    for (int i = 0; i < _peer_host_count; ++i)
    {
        DDS_String_free(_peer_host[i]);
        _peer_host[i] = NULL;
    }

    if (_participant != NULL)
    {
        perftest_cpp::MilliSleep(2000);

        if (_reader != NULL)
        {
            DDSDataReaderListener *reader_listener = _reader->get_listener();
            if (reader_listener != NULL)
            {
                delete (reader_listener);
            }
            _subscriber->delete_datareader(_reader);
        }

        DDSDomainParticipantListener *participant_listener = _participant->get_listener();
        if (participant_listener != NULL)
        {
            delete (participant_listener);
        }

        _participant->delete_contained_entities();
        DDSTheParticipantFactory->delete_participant(_participant);
    }

    if (_pongSemaphore != NULL)
    {
        RTIOsapiSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }

    DDSDomainParticipantFactory::finalize_instance();
}

/*********************************************************
 * PrintCmdLineHelp
 */
void RTISocketImpl::PrintCmdLineHelp()
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
                               "\t-multicast <address>          - Use multicast to send data.\n" +
                               "\t                                Default not to use multicast\n" +
                               "\t                                <address> is optional, if unspecified:\n" +
                               "\t                                                latency 239.255.1.2,\n" +
                               "\t                                                announcement 239.255.1.100,\n" +
                               "\t                                                throughput 239.255.1.1\n" +
                               "\t-bestEffort                   - Run test in best effort mode, default reliable\n" +
                               "\t-batchSize <bytes>            - Size in bytes of batched message, default 0\n" +
                               "\t                                (no batching)\n" +
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
    for (unsigned int i = 0; i < sizeof(valid_flow_controller) / sizeof(valid_flow_controller[0]); i++)
    {
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
bool RTISocketImpl::ParseConfig(int argc, char *argv[])
{
    unsigned long _scan_max_size = 0;
    int i;
    int sec = 0;
    unsigned int nanosec = 0;

    // Command line params
    for (i = 0; i < argc; ++i)
    {
        if (IS_OPTION(argv[i], "-pub"))
        {
            _isPublisher = true;
        }
        else if (IS_OPTION(argv[i], "-scan"))
        {
            _isScan = true;
            if ((i != (argc - 1)) && *argv[1 + i] != '-')
            {
                ++i;
                unsigned long aux_scan;
                char *pch;
                pch = strtok(argv[i], ":");
                while (pch != NULL)
                {
                    if (sscanf(pch, "%lu", &aux_scan) != 1)
                    {
                        fprintf(stderr, "-scan <size> value must have the format '-scan <size1>:<size2>:...:<sizeN>'\n");
                        return false;
                    }
                    pch = strtok(NULL, ":");
                    if (aux_scan >= _scan_max_size)
                    {
                        _scan_max_size = aux_scan;
                    }
                }
            }
        }
        else if (IS_OPTION(argv[i], "-dataLen"))
        {

            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <length> after -dataLen\n");
                return false;
            }

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < (unsigned long)perftest_cpp::OVERHEAD_BYTES)
            {
                fprintf(stderr, "-dataLen must be >= %d\n", perftest_cpp::OVERHEAD_BYTES);
                return false;
            }

            if (_DataLen > (unsigned long)MAX_PERFTEST_SAMPLE_SIZE)
            {
                fprintf(stderr, "-dataLen must be <= %d\n", MAX_PERFTEST_SAMPLE_SIZE);
                return false;
            }
            if (_useUnbounded == 0 && _DataLen > (unsigned long)MAX_BOUNDED_SEQ_SIZE)
            {
                _useUnbounded = (std::min)(
                    2 * _DataLen, (unsigned long)MAX_BOUNDED_SEQ_SIZE);
            }
        }
        else if (IS_OPTION(argv[i], "-unbounded"))
        {
            if ((i == (argc - 1)) || *argv[i + 1] == '-')
            {
                _useUnbounded = (std::min)(
                    2 * _DataLen, (unsigned long)MAX_BOUNDED_SEQ_SIZE);
            }
            else
            {
                ++i;
                _useUnbounded = strtol(argv[i], NULL, 10);

                if (_useUnbounded < (unsigned long)perftest_cpp::OVERHEAD_BYTES)
                {
                    fprintf(stderr, "-unbounded <allocation_threshold> must be >= %d\n", perftest_cpp::OVERHEAD_BYTES);
                    return false;
                }
                if (_useUnbounded > (unsigned long)MAX_BOUNDED_SEQ_SIZE)
                {
                    fprintf(stderr, "-unbounded <allocation_threshold> must be <= %d\n", MAX_BOUNDED_SEQ_SIZE);
                    return false;
                }
            }
        }
        else if (IS_OPTION(argv[i], "-sendQueueSize"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -sendQueueSize\n");
                return false;
            }
            _SendQueueSize = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-heartbeatPeriod"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <period> after -heartbeatPeriod\n");
                return false;
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i], "%d:%u", &sec, &nanosec) != 2)
            {
                fprintf(stderr, "-heartbeatPeriod value must have the format <sec>:<nanosec>\n");
                return false;
            }

            if (sec > 0 || nanosec > 0)
            {
                _HeartbeatPeriod.sec = sec;
                _HeartbeatPeriod.nanosec = nanosec;
            }
        }
        else if (IS_OPTION(argv[i], "-fastHeartbeatPeriod"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <period> after -fastHeartbeatPeriod\n");
                return false;
            }

            sec = 0;
            nanosec = 0;

            if (sscanf(argv[i], "%d:%u", &sec, &nanosec) != 2)
            {
                fprintf(stderr, "-fastHeartbeatPeriod value must have the format <sec>:<nanosec>\n");
                return false;
            }

            if (sec > 0 || nanosec > 0)
            {
                _FastHeartbeatPeriod.sec = sec;
                _FastHeartbeatPeriod.nanosec = nanosec;
            }
        }
        else if (IS_OPTION(argv[i], "-domain"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <id> after -domain\n");
                return false;
            }
            _DomainID = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-qosFile"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <filename> after -qosFile\n");
                return false;
            }
            _ProfileFile = argv[i];
        }
        else if (IS_OPTION(argv[i], "-qosLibrary"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <library name> after -qosLibrary\n");
                return false;
            }
            _ProfileLibraryName = argv[i];
        }
        else if (IS_OPTION(argv[i], "-multicast"))
        {
            _IsMulticast = true;
            if ((i != (argc - 1)) && *argv[i + 1] != '-')
            {
                i++;
                THROUGHPUT_MULTICAST_ADDR = argv[i];
                LATENCY_MULTICAST_ADDR = argv[i];
                ANNOUNCEMENT_MULTICAST_ADDR = argv[i];
            }
        }
        else if (IS_OPTION(argv[i], "-nomulticast"))
        {
            _IsMulticast = false;
        }
        else if (IS_OPTION(argv[i], "-bestEffort"))
        {
            _IsReliable = false;
        }
        else if (IS_OPTION(argv[i], "-durability"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <kind> after -durability\n");
                return false;
            }
            _Durability = strtol(argv[i], NULL, 10);

            if ((_Durability < 0) || (_Durability > 3))
            {
                fprintf(stderr, "durability kind must be 0(volatile), 1(transient local), 2(transient), or 3(persistent) \n");
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-dynamicData"))
        {
            _isDynamicData = true;
            fprintf(stderr, "Using Dynamic Data.\n");
        }
        else if (IS_OPTION(argv[i], "-noDirectCommunication"))
        {
            _DirectCommunication = false;
        }
        else if (IS_OPTION(argv[i], "-instances"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -instances\n");
                return false;
            }
            _InstanceCount = strtol(argv[i], NULL, 10);
            _InstanceMaxCountReader = _InstanceCount;

            if (_InstanceCount <= 0)
            {
                fprintf(stderr, "instance count cannot be negative or zero\n");
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-instanceHashBuckets"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -instanceHashBuckets\n");
                return false;
            }
            _InstanceHashBuckets = strtol(argv[i], NULL, 10);

            if (_InstanceHashBuckets <= 0 && _InstanceHashBuckets != -1)
            {
                fprintf(stderr, "instance hash buckets cannot be negative or zero\n");
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-batchSize"))
        {

            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <#bytes> after -batchSize\n");
                return false;
            }
            _BatchSize = strtol(argv[i], NULL, 10);

            if (_BatchSize < 0 || _BatchSize > (unsigned int)MAX_SYNCHRONOUS_SIZE)
            {
                fprintf(stderr, "Batch size '%d' should be between [0,%d]\n",
                        _BatchSize,
                        MAX_SYNCHRONOUS_SIZE);
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-keepDurationUsec"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <usec> after -keepDurationUsec\n");
                return false;
            }
            _KeepDurationUsec = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-noPositiveAcks"))
        {
            _UsePositiveAcks = false;
        }
        else if (IS_OPTION(argv[i], "-verbosity"))
        {
            errno = 0;
            int verbosityLevel = strtol(argv[++i], NULL, 10);

            if (errno)
            {
                fprintf(stderr, "Unexpected value after -verbosity\n");
                return false;
            }

            switch (verbosityLevel)
            {
            case 0:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                fprintf(stderr, "Setting verbosity to SILENT\n");
                break;
            case 1:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                fprintf(stderr, "Setting verbosity to ERROR\n");
                break;
            case 2:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                fprintf(stderr, "Setting verbosity to WARNING\n");
                break;
            case 3:
                NDDSConfigLogger::get_instance()->set_verbosity(NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                fprintf(stderr, "Setting verbosity to STATUS_ALL\n");
                break;
            default:
                fprintf(stderr,
                        "Invalid value for the verbosity parameter. Setting verbosity to ERROR (1)\n");
                verbosityLevel = 1;
                break;
            }
        }
        else if (IS_OPTION(argv[i], "-waitsetDelayUsec"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <usec> after -waitsetDelayUsec\n");
                return false;
            }
            _WaitsetDelayUsec = (unsigned int)strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-waitsetEventCount"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <count> after -waitsetEventCount\n");
                return false;
            }
            _WaitsetEventCount = strtol(argv[i], NULL, 10);
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
            fprintf(stderr, "Auto Throttling enabled. Automatically adjusting the DataWriter\'s writing rate\n");
            _AutoThrottle = true;
        }
        else if (IS_OPTION(argv[i], "-enableTurboMode"))
        {
            _TurboMode = true;
        }
        else if (IS_OPTION(argv[i], "-noXmlQos"))
        {
            _UseXmlQos = false;
        }
        else if (IS_OPTION(argv[i], "-asynchronous"))
        {
            _IsAsynchronous = true;
        }
        else if (IS_OPTION(argv[i], "-flowController"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <flow Controller Name> after -flowController\n");
                return false;
            }
            _FlowControllerCustom = argv[i];

            // verify if the flow controller name is correct, else use "default"
            bool valid_flow_control = false;
            for (unsigned int i = 0; i < sizeof(valid_flow_controller) / sizeof(valid_flow_controller[0]); i++)
            {
                if (_FlowControllerCustom == valid_flow_controller[i])
                {
                    valid_flow_control = true;
                }
            }

            if (!valid_flow_control)
            {
                fprintf(stderr, "Bad <flow> '%s' for custom flow controller\n", _FlowControllerCustom.c_str());
                _FlowControllerCustom = "default";
            }
        }
        else if (IS_OPTION(argv[i], "-peer"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <address> after -peer\n");
                return false;
            }
            if (_peer_host_count + 1 < RTIPERFTEST_MAX_PEERS)
            {
                _peer_host[_peer_host_count++] = DDS_String_dup(argv[i]);
            }
            else
            {
                fprintf(stderr, "The maximun of -initial peers is %d\n", RTIPERFTEST_MAX_PEERS);
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-cft"))
        {
            _useCft = true;
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <start>:<end> after -cft\n");
                return false;
            }

            if (strchr(argv[i], ':') != NULL)
            { // In the case that there are 2 parameter
                unsigned int cftStart = 0;
                unsigned int cftEnd = 0;
                if (sscanf(argv[i], "%u:%u", &cftStart, &cftEnd) != 2)
                {
                    fprintf(stderr, "-cft value must have the format <start>:<end>\n");
                    return false;
                }
                _CFTRange[0] = cftStart;
                _CFTRange[1] = cftEnd;
            }
            else
            {
                _CFTRange[0] = strtol(argv[i], NULL, 10);
                _CFTRange[1] = _CFTRange[0];
            }

            if (_CFTRange[0] > _CFTRange[1])
            {
                fprintf(stderr, "-cft <start> value cannot be bigger than <end>\n");
                return false;
            }
            if (_CFTRange[0] < 0 ||
                _CFTRange[0] >= (unsigned int)MAX_CFT_VALUE ||
                _CFTRange[1] < 0 ||
                _CFTRange[1] >= (unsigned int)MAX_CFT_VALUE)
            {
                fprintf(stderr, "-cft <start>:<end> values should be between [0,%d] \n", MAX_CFT_VALUE);
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-writeInstance"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <number> after -writeInstance\n");
                return false;
            }
            _instancesToBeWritten = strtol(argv[i], NULL, 10);
        }
#ifdef RTI_SECURE_PERFTEST
        else if (IS_OPTION(argv[i], "-secureSign"))
        {
            _secureIsSigned = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptBoth"))
        {
            _secureIsDataEncrypted = true;
            _secureIsSMEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptData"))
        {
            _secureIsDataEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptSM"))
        {
            _secureIsSMEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureEncryptDiscovery"))
        {
            _secureIsDiscoveryEncrypted = true;
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureGovernanceFile"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <file> after -secureGovernanceFile\n");
                return false;
            }
            _secureGovernanceFile = argv[i];
            fprintf(stdout, "Warning -- authentication, encryption, signing arguments "
                            "will be ignored, and the values specified by the Governance file will "
                            "be used instead\n");
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-securePermissionsFile"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <file> after -securePermissionsFile\n");
                return false;
            }
            _securePermissionsFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureCertAuthority"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <file> after -secureCertAuthority\n");
                return false;
            }
            _secureCertAuthorityFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureCertFile"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <file> after -secureCertFile\n");
                return false;
            }
            _secureCertificateFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-securePrivateKey"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <file> after -securePrivateKey\n");
                return false;
            }
            _securePrivateKeyFile = argv[i];
            _secureUseSecure = true;
        }
        else if (IS_OPTION(argv[i], "-secureLibrary"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <file> after -secureLibrary\n");
                return false;
            }
            _secureLibrary = argv[i];
        }
        else if (IS_OPTION(argv[i], "-secureDebug"))
        {
            if ((i == (argc - 1)) || *argv[++i] == '-')
            {
                fprintf(stderr, "Missing <level> after -secureDebug\n");
                return false;
            }
            _secureDebugLevel = strtol(argv[i], NULL, 10);
        }
#endif
        else
        {
            if (i > 0)
            {
                std::map<std::string, unsigned int> transportCmdOpts =
                    PerftestTransport::getTransportCmdLineArgs();

                std::map<std::string, unsigned int>::iterator it =
                    transportCmdOpts.find(argv[i]);
                if (it != transportCmdOpts.end())
                {
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

    if (_IsAsynchronous && _BatchSize > 0)
    {
        fprintf(stderr, "Batching cannnot be used with asynchronous writing.\n");
        return false;
    }

    if (_isScan)
    {
        _DataLen = _scan_max_size;
        // Check if large data or small data
        if (_scan_max_size > (unsigned long)(std::min)(MAX_SYNCHRONOUS_SIZE, MAX_BOUNDED_SEQ_SIZE))
        {
            if (_useUnbounded == 0)
            {
                _useUnbounded = MAX_BOUNDED_SEQ_SIZE;
            }
            _isLargeData = true;
        }
        else if (_scan_max_size <= (unsigned long)(std::min)(MAX_SYNCHRONOUS_SIZE, MAX_BOUNDED_SEQ_SIZE))
        {
            _useUnbounded = 0;
            _isLargeData = false;
        }
        else
        {
            return false;
        }
        if (_isLargeData && _BatchSize > 0)
        {
            fprintf(stderr, "Batching cannnot be used with asynchronous writing.\n");
            return false;
        }
    }
    else
    { // If not Scan, compare sizes of Batching and dataLen
        /*
         * We don't want to use batching if the sample is the same size as the batch
         * nor if the sample is bigger (in this case we avoid the checking in the
         * middleware).
         */
        if (_BatchSize > 0 && (unsigned long)_BatchSize <= _DataLen)
        {
            fprintf(stderr,
                    "Batching disabled: BatchSize (%d) is equal or smaller "
                    "than the sample size (%lu).\n",
                    _BatchSize,
                    _DataLen);
            _BatchSize = 0;
        }
    }

    if (_DataLen > (unsigned long)MAX_SYNCHRONOUS_SIZE)
    {
        fprintf(stderr, "Large data settings enabled.\n");
        _isLargeData = true;
    }

    if (_TurboMode)
    {
        if (_IsAsynchronous)
        {
            fprintf(stderr, "Turbo Mode cannot be used with asynchronous writing.\n");
            return false;
        }
        if (_isLargeData)
        {
            fprintf(stderr, "Turbo Mode disabled, using large data.\n");
            _TurboMode = false;
        }
    }

    // Manage _instancesToBeWritten
    if (_instancesToBeWritten != -1)
    {
        if ((long)_InstanceCount < _instancesToBeWritten)
        {
            fprintf(
                stderr,
                "Specified '-WriteInstance' (%ld) invalid: Bigger than the number of instances (%lu).\n",
                _instancesToBeWritten, _InstanceCount);
            return false;
        }
    }
    if (_isPublisher && _useCft)
    {
        fprintf(stderr,
                "Content Filtered Topic is not a parameter in the publisher side.\n");
    }

    if (!_transport.parseTransportOptions(argc, argv))
    {
        fprintf(stderr,
                "Failure parsing the transport options.\n");
        return false;
    };

    return true;
}

/*********************************************************
 * DomainListener
 */
class DomainListener : public DDSDomainParticipantListener
{
    virtual void on_inconsistent_topic(
        DDSTopic *topic,
        const DDS_InconsistentTopicStatus & /*status*/)
    {
        fprintf(stderr, "Found inconsistent topic. Expecting %s of type %s.\n",
                topic->get_name(), topic->get_type_name());
        fflush(stderr);
    }

    virtual void on_offered_incompatible_qos(
        DDSDataWriter *writer,
        const DDS_OfferedIncompatibleQosStatus &status)
    {
        fprintf(stderr, "Found incompatible reader for writer %s QoS is %d.\n",
                writer->get_topic()->get_name(), status.last_policy_id);
        fflush(stderr);
    }

    virtual void on_requested_incompatible_qos(
        DDSDataReader *reader,
        const DDS_RequestedIncompatibleQosStatus &status)
    {
        fprintf(stderr, "Found incompatible writer for reader %s QoS is %d.\n",
                reader->get_topicdescription()->get_name(), status.last_policy_id);
        fflush(stderr);
    }
};

/*********************************************************
 * RTIPublisher
 */
class RTISocketPublisher : public IMessagingWriter
{
  private:

    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_SendResource_t _send_resource;
    NDDS_Transport_Address_t _dst_address;
    NDDS_Transport_Port_t _send_port;

    //typename T::DataWriter *_writer;
    unsigned long _num_instances;
    unsigned long _instance_counter;
    DDS_InstanceHandle_t *_instance_handles;
    RTIOsapiSemaphore *_pongSemaphore;
    long _instancesToBeWritten;

  public:
    RTISocketPublisher(
        NDDS_Transport_Plugin plugin,
        NDDS_Transport_SendResource_t send_resource;
        NDDS_Transport_Address_t dst_address;
        NDDS_Transport_Port_t send_port;
    )
    {
        _plugin = &plugin;
        _send_resource = send_resource;
        _dst_address = dst_address;
        _send_port = send_port;
    }

    ~RTIPublisher()
    {
        Shutdown();
    }

    void Shutdown()
    {
        // TODO, destruct the resources and the plugin
        /*
        if (_writer->get_listener() != NULL)
        {
            delete (_writer->get_listener());
            _writer->set_listener(NULL);
        }

        free(_instance_handles);
        */

    }

    void Flush()
    {
        //_writer->flush();
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {

        char * payload;
        int payload_size = message.size;

        RTIOsapiHeap_allocateArray(&payload, payload_size, sizeof(TestMessage));

        send_buffer.length = payload_size;
        send_buffer.pointer = payload;

        bool result = plugin->send(
                plugin,
                &send_resource,
                &dst_address,
                send_port,
                0,
                &send_buffer,
                1,
                NULL);

        if (!result)
        {
            fprintf(stderr, "Write error using sockets\n");
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

    bool waitForPingResponse()
    {
        if (_pongSemaphore != NULL)
        {
            if (!RTIOsapiSemaphore_take(_pongSemaphore, NULL))
            {
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

        if (_pongSemaphore != NULL)
        {
            if (!RTIOsapiSemaphore_take(_pongSemaphore, &blockDurationIn))
            {
                fprintf(stderr, "Unexpected error taking semaphore\n");
                return false;
            }
        }
        return true;
    }

    bool notifyPingResponse()
    {
        if (_pongSemaphore != NULL)
        {
            if (!RTIOsapiSemaphore_give(_pongSemaphore))
            {
                fprintf(stderr, "Unexpected error giving semaphore\n");
                return false;
            }
        }
        return true;
    }

    unsigned int getPulledSampleCount()
    {
        DDS_DataWriterProtocolStatus status;
        _writer->get_datawriter_protocol_status(status);
        return (unsigned int)status.pulled_sample_count;
    };

    void wait_for_acknowledgments(long sec, unsigned long nsec)
    {
        DDS_Duration_t timeout = {sec, nsec};
        //TODO change for a simple Wait _writer->wait_for_acknowledgments(timeout);
    }
};

/*********************************************************
 * ReceiverListener
 */
class ReceiverListener : public DDSDataReaderListener
{
  private:
    typename T::Seq _data_seq;
    DDS_SampleInfoSeq _info_seq;
    TestMessage _message;
    IMessagingCB *_callback;

  public:
    ReceiverListener(IMessagingCB *callback) : _message()
    {
        _callback = callback;
    }

    void on_data_available(DDSDataReader *reader)
    {

        typename T::DataReader *datareader;

        datareader = T::DataReader::narrow(reader);
        if (datareader == NULL)
        {
            fprintf(stderr, "DataReader narrow error.\n");
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
            fprintf(stderr, "called back no data\n");
            return;
        }
        else if (retcode != DDS_RETCODE_OK)
        {
            fprintf(stderr, "Error during taking data %d\n", retcode);
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
            fprintf(stderr, "Error during return loan %d.\n", retcode);
            fflush(stderr);
        }
    }
};

/*********************************************************
 * RTISubscriber
 */
class RTISocketSubscriber : public IMessagingReader
{
  private:
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_SendResource_t _recv_resource;
    NDDS_Transport_Port_t _recv_port;
    struct RTINtpTime timestamp = {0, 0};

    TestMessage _message;

    int _data_idx;
    bool _no_data;

  public:
    RTISubscriber(
        NDDS_Transport_Plugin plugin,
        NDDS_Transport_SendResource_t recv_resource;
        NDDS_Transport_Port_t recv_port;)
    {
        _plugin = &plugin;
        _recv_resource = recv_resource;
        _recv_port = recv_port;
    }

    ~RTISubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {
        //TODO
        /*
        if (_reader->get_listener() != NULL)
        {
            delete (_reader->get_listener());
            _reader->set_listener(NULL);
        }
        // loan may be outstanding during shutdown
        _reader->return_loan(_data_seq, _info_seq);
        */
    }

    TestMessage *ReceiveMessage()
    {

        NDDS_Transport_Message_t transp_message = NDDS_TRANSPORT_MESSAGE_INVALID;

        while (true)
        {

            bool result = plugin->receive_rEA(
                    plugin,
                    &transp_message,
                    &recv_buffer,
                    &recv_resource,
                    &worker);

            if (!result)
            {
                fprintf(stderr, "error receiving data\n");
                continue;
            }

            RTIOsapiMemory_copy(
                    &_message,
                    transp_message.buffer.pointer,
                    sizeof(TestMessage);

            if (_message.timestamp_sec == 0 && _message.timestamp_usec == 0)
            {
                if (plugin->return_loaned_buffer_rEA != NULL)
                {
                    plugin->return_loaned_buffer_rEA(
                            plugin,
                            &recv_resource,
                            &transp_message,
                            NULL);
                }
                break;
            }

            if (debug)
            {
                printf("Received PING sample with length %d\n",
                        transp_message.buffer.length);
            }
        }

        return &_message;
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

/*********************************************************
 * Initialize
 */
bool RTISocketImpl::Initialize(int argc, char *argv[])
{
    DDS_DomainParticipantQos qos;
    DDS_DomainParticipantFactoryQos factory_qos;
    DomainListener *listener = new DomainListener();

    _factory = DDSDomainParticipantFactory::get_instance();

    if (!ParseConfig(argc, argv))
    {
        return false;
    }

    // only if we run the latency test we need to wait
    // for pongs after sending pings
    _pongSemaphore = _LatencyTest ? RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL) : NULL;

    // setup the QOS profile file to be loaded
    _factory->get_qos(factory_qos);
    if (_UseXmlQos)
    {
        factory_qos.profile.url_profile.ensure_length(1, 1);
        factory_qos.profile.url_profile[0] = DDS_String_dup(_ProfileFile);
    }
    else
    {
        fprintf(stderr, "Not using xml file for QoS.\n");
        factory_qos.profile.string_profile.from_array(
            PERFTEST_QOS_STRING,
            PERFTEST_QOS_STRING_SIZE);
    }
    _factory->set_qos(factory_qos);

    if (_factory->reload_profiles() != DDS_RETCODE_OK)
    {
        fprintf(stderr, "Problem opening QOS profiles file %s.\n", _ProfileFile);
        return false;
    }

    if (_factory->set_default_library(_ProfileLibraryName) != DDS_RETCODE_OK)
    {
        fprintf(stderr, "No QOS Library named \"%s\" found in %s.\n",
                _ProfileLibraryName, _ProfileFile);
        return false;
    }

    // Configure DDSDomainParticipant QOS
    if (_factory->get_participant_qos_from_profile(
            qos,
            _ProfileLibraryName,
            "BaseProfileQos") != DDS_RETCODE_OK)
    {
        fprintf(
            stderr,
            "Problem setting QoS Library \"%s::BaseProfileQos\" for participant_qos.\n",
            _ProfileLibraryName);
    }

    // set initial peers and not use multicast
    if (_peer_host_count > 0)
    {
        printf("Initial peers:\n");
        for (int i = 0; i < _peer_host_count; ++i)
        {
            printf("\t%s\n", _peer_host[i]);
        }
        qos.discovery.initial_peers.from_array(
            (const char **)_peer_host,
            _peer_host_count);
        qos.discovery.multicast_receive_addresses.length(0);
    }

    if (!configureTransport(_transport, qos))
    {
        return false;
    };
    _transport.printTransportConfigurationSummary();

    if (_AutoThrottle)
    {
        DDSPropertyQosPolicyHelper::add_property(qos.property,
                                                 "dds.domain_participant.auto_throttle.enable", "true", false);
    }

    // Creates the participant
    _participant = _factory->create_participant(
        _DomainID, qos, listener,
        DDS_INCONSISTENT_TOPIC_STATUS |
            DDS_OFFERED_INCOMPATIBLE_QOS_STATUS |
            DDS_REQUESTED_INCOMPATIBLE_QOS_STATUS);

    if (_participant == NULL)
    {
        fprintf(stderr, "Problem creating participant.\n");
        return false;
    }

    // Register the types and create the topics
    if (!_isDynamicData)
    {
        T::TypeSupport::register_type(_participant, _typename);
    }
    else
    {
        DDSDynamicDataTypeSupport *dynamicDataTypeSupportObject =
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
        fprintf(stderr, "Problem creating publisher.\n");
        return false;
    }

    _subscriber = _participant->create_subscriber_with_profile(
        _ProfileLibraryName,
        "BaseProfileQos",
        NULL,
        DDS_STATUS_MASK_NONE);
    if (_subscriber == NULL)
    {
        fprintf(stderr, "Problem creating subscriber.\n");
        return false;
    }

    /**************************************************************************/
    /*****************************sockets stuff********************************/

    if (use_shmem)
    {
        plugin = NDDS_Transport_Shmem_new(&shmem_prop);
    }
    else
    {
        if (receive_nic != NULL)
        {
            updv4_prop.parent.allow_interfaces_list = &receive_nic;
            updv4_prop.parent.allow_interfaces_list_length = 1;
        }

        plugin = NDDS_Transport_UDPv4_new(&updv4_prop);
    }

    if (plugin == NULL)
    {
        fprintf(stderr, "error creating transport plugin\n");
        goto done;
    }

    /**************************************************************************/
    /**************************************************************************/


    return true;
}

/*********************************************************
 * CreateWriter
 */
IMessagingWriter *RTISocketImpl::CreateWriter(const char *topic_name)
{

    struct RTINtpTime timestamp;
    int ok = 0;
    int result = 0;
    // TODO remove line //NDDS_Transport_Plugin *plugin = NULL; //The plugin is menmer of the class
    struct NDDS_Transport_UDPv4_Property_t updv4_prop =
        NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
    struct NDDS_Transport_Shmem_Property_t shmem_prop =
        NDDS_TRANSPORT_SHMEM_PROPERTY_DEFAULT;
    NDDS_Transport_RecvResource_t recv_resource = NULL;
    NDDS_Transport_SendResource_t send_resource = NULL;
    NDDS_Transport_Port_t recv_port = 0;
    NDDS_Transport_Port_t send_port = 0;
    NDDS_Transport_Address_t dst_address =
        NDDS_TRANSPORT_ADDRESS_INVALID;
    char *payload = NULL;
    struct RTIOsapiThread *recv_thread = NULL;
    struct PayloadListener payload_listener =
        PayloadListener_INITIALIZER;
    struct RTINtpTime block_buration = {5, 0};
    int count = 0;
    int last_sample = 0;
    struct DDS_Duration_t ten_milli = {0, 10000000};
    struct DDS_Duration_t ten_sec = {10, 0};
    struct RTINtpTime one_sec = {1, 0};
    struct RTINtpTime start_timestamp = {0, 0};
    struct RTINtpTime diff_timestamp = {0, 0};
    NDDS_Transport_Buffer_t send_buffer = {0, NULL};
    RTIOsapiSemaphoreStatus semStatus;

    //TODO create a offset depend of the topic name for the port
    send_port = PRESRtps_getWellKnownUnicastPort(
        domainId,
        1,
        7400,
        250,
        2,
        11);

    /* Create send resource */
    if (!use_shmem)
    {
        result = NDDS_Transport_UDP_string_to_address_cEA(
                plugin,
                &dst_address,
                send_nic);

        if (result != 1)
        {
            fprintf(stderr, "NDDS_Transport_UDP_string_to_address_cEA error\n");
            return NULL;
        }
    }

    result = plugin->create_sendresource_srEA(
            plugin,
            &send_resource,
            &dst_address,
            send_port,
            0);
    if (result != 1)
    {
        fprintf(stderr, "create_sendresource_srEA error\n");
        return NULL;
    }

    return new RTISocketPublisher(plugin, send_resource, dst_address, send_port);


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
DDSTopicDescription *RTISocketImpl::CreateCft(
    const char *topic_name,
    DDSTopic *topic)
{
    std::string condition;
    DDS_StringSeq parameters(2 * KEY_SIZE);
    if (_CFTRange[0] == _CFTRange[1])
    { // If same elements, no range
        printf("CFT enabled for instance: '%d' \n", _CFTRange[0]);
        char cft_param[KEY_SIZE][128];
        for (int i = 0; i < KEY_SIZE; i++)
        {
            sprintf(cft_param[i], "%d", (unsigned char)(_CFTRange[0] >> i * 8));
        }
        const char *param_list[] = {cft_param[0], cft_param[1], cft_param[2], cft_param[3]};
        parameters.from_array(param_list, KEY_SIZE);
        condition = "(%0 = key[0] AND %1 = key[1] AND %2 = key[2] AND %3 = key[3]) OR"
                    "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";
    }
    else
    { // If range
        printf("CFT enabled for instance range: [%d,%d] \n", _CFTRange[0], _CFTRange[1]);
        char cft_param[2 * KEY_SIZE][128];
        for (int i = 0; i < 2 * KEY_SIZE; i++)
        {
            if (i < KEY_SIZE)
            {
                sprintf(cft_param[i], "%d", (unsigned char)(_CFTRange[0] >> i * 8));
            }
            else
            { // KEY_SIZE < i < KEY_SIZE * 2
                sprintf(cft_param[i], "%d", (unsigned char)(_CFTRange[1] >> i * 8));
            }
        }
        const char *param_list[] = {cft_param[0], cft_param[1],
                                    cft_param[2], cft_param[3], cft_param[4],
                                    cft_param[5], cft_param[6], cft_param[7]};
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
IMessagingReader *RTISocketImpl::CreateReader(
    const char *topic_name,
    IMessagingCB *callback)
{

    struct RTINtpTime timestamp;
    int ok = 0;
    int result = 0;
    // TODO remove line //NDDS_Transport_Plugin *plugin = NULL; //The plugin is menmer of the class
    struct NDDS_Transport_UDPv4_Property_t updv4_prop =
        NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
    struct NDDS_Transport_Shmem_Property_t shmem_prop =
        NDDS_TRANSPORT_SHMEM_PROPERTY_DEFAULT;
    NDDS_Transport_RecvResource_t recv_resource = NULL;
    NDDS_Transport_SendResource_t send_resource = NULL;
    NDDS_Transport_Port_t recv_port = 0;
    NDDS_Transport_Port_t send_port = 0;
    NDDS_Transport_Address_t dst_address =
        NDDS_TRANSPORT_ADDRESS_INVALID;
    char *payload = NULL;
    struct RTIOsapiThread *recv_thread = NULL;
    struct PayloadListener payload_listener =
        PayloadListener_INITIALIZER;
    struct RTINtpTime block_buration = {5, 0};
    int count = 0;
    int last_sample = 0;
    struct DDS_Duration_t ten_milli = {0, 10000000};
    struct DDS_Duration_t ten_sec = {10, 0};
    struct RTINtpTime one_sec = {1, 0};
    struct RTINtpTime start_timestamp = {0, 0};
    struct RTINtpTime diff_timestamp = {0, 0};
    NDDS_Transport_Buffer_t send_buffer = {0, NULL};
    RTIOsapiSemaphoreStatus semStatus;

    /* Port in which to receive PONG data */
    // parameters order (domain_id, participant_id, port_base, domain_id_gain, participant_id_gain, port_offset)
    recv_port = PRESRtps_getWellKnownUnicastPort(
            domainId,
            0, // TODO // 0 - 1 depend of is publiser or the subcriber // maybe not
            7400,
            250,
            2,
            11);

    /* Create receive resource */
    result = plugin->create_recvresource_rrEA(
            plugin,
            &recv_resource,
            &recv_port,
            NULL,
            0);

    if (result != 1)
    {
        fprintf(stderr, "Create_recvresource_rrEA error\n");
        return NULL;
    }
    return new RTISocketSubscriber(plugin, recv_resource, recv_port);
}

#ifdef RTI_WIN32
#pragma warning(pop)
#endif
