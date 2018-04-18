/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "RTISocketImpl.h"

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

std::string valid_flow_controller_socket[] = {"default", "1Gbps", "10Gbps"};

/*********************************************************
 * Shutdown
 */
void RTISocketImpl::Shutdown()
{

    if (_plugin != NULL)
    {
        _plugin->delete_cEA(_plugin, NULL);
    }

    if (_pongSemaphore != NULL)
    {
        RTIOsapiSemaphore_delete(_pongSemaphore);
        _pongSemaphore = NULL;
    }

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
    for (unsigned int i = 0; i < sizeof(valid_flow_controller_socket) / sizeof(valid_flow_controller_socket[0]); i++)
    {
        usage_string += "\"" + valid_flow_controller_socket[i] + "\" ";
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
            for (unsigned int i = 0; i < sizeof(valid_flow_controller_socket) / sizeof(valid_flow_controller_socket[0]); i++)
            {
                if (_FlowControllerCustom == valid_flow_controller_socket[i])
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
    TestData_t data;
    NDDS_Transport_Plugin *_plugin;
    NDDS_Transport_SendResource_t _send_resource;
    NDDS_Transport_Address_t _dst_address;
    NDDS_Transport_Port_t _send_port;
    NDDS_Transport_Buffer_t _send_buffer;
    char *_payload;
    int _payload_size;

    RTIOsapiSemaphore *_pongSemaphore;

    unsigned int _pulled_sample_count;

  public:
    RTISocketPublisher(
        NDDS_Transport_Plugin *plugin,
        NDDS_Transport_SendResource_t send_resource,
        NDDS_Transport_Address_t dst_address,
        NDDS_Transport_Port_t send_port,
        RTIOsapiSemaphore *pongSemaphore)
    {
        _plugin = plugin;
        _send_resource = send_resource;
        _dst_address = dst_address;
        _send_port = send_port;
        _pongSemaphore = pongSemaphore;

        _payload = new char;
        _payload_size = 0;

        _send_buffer.length = _payload_size;
        _send_buffer.pointer = _payload;

        _pulled_sample_count = 0;
    }

    ~RTISocketPublisher()
    {
        Shutdown();
    }

    void Shutdown()
    {

        if (_send_resource != NULL)
        {
            _plugin->destroy_sendresource_srEA(_plugin, &_send_resource);
        }

        if (_payload != NULL)
        {
            RTIOsapiHeap_freeArray(_payload);
        }
    }

    void Flush()
    {
        /* Dummy funtion */
    }

    bool Send(const TestMessage &message, bool isCftWildCardKey)
    {

        if(message.size != _payload_size || _payload_size == 0){
            _payload_size = message.size + perftest_cpp::OVERHEAD_BYTES;
            RTIOsapiHeap_freeArray(_payload);
            RTIOsapiHeap_allocateArray(&_payload, _payload_size, char);

            _send_buffer.length = _payload_size;
            _send_buffer.pointer = _payload;
        }


        /*TODO review*/
        RTIOsapiMemory_move(_payload, &message, sizeof(message));
        //RTIOsapiMemory_copy(payload, &message, perftest_cpp::OVERHEAD_BYTES+8);


        bool result = _plugin->send(
            _plugin,
            &_send_resource,
            &_dst_address,
            _send_port,
            0,
            &_send_buffer,
            1,
            NULL);

        if (!result)
        {
            fprintf(stderr, "Write error using sockets\n");
            return false;
        }
        ++_pulled_sample_count;

        return true;
    }

    void WaitForReaders(int numSubscribers)
    {
        /*Dummy Function*/
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
        return _pulled_sample_count;
    };

    void wait_for_acknowledgments(long sec, unsigned long nsec)
    {
        /*TODO idk how to cach that ack.*/
        //perftest_cpp::MilliSleep(1000);
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
    NDDS_Transport_Buffer_t _recv_buffer;
    NDDS_Transport_Port_t _recv_port;

    TestMessage _message;

    struct REDAWorker worker;

    int _data_idx;
    bool _no_data;


    RTIOsapiSemaphore *_pongSemaphore;

  public:
    RTISocketSubscriber(
        NDDS_Transport_Plugin *plugin,
        NDDS_Transport_SendResource_t recv_resource,
        NDDS_Transport_Port_t recv_port)
    {
        _plugin = plugin;
        _recv_resource = recv_resource;
        _recv_port = recv_port;
        _recv_buffer.length = 0;
        _recv_buffer.pointer = NULL;

        worker._name = "worker";

        _message.data = NULL;
        _message.size = 0;
        //_message.key = new unsigned char[4];
        _message.entity_id = 0;
        _message.seq_num = 0;
        _message.timestamp_sec = 0;
        _message.timestamp_usec = 0;
        _message.latency_ping = 0;

        unsigned int size = 63000 + perftest_cpp::OVERHEAD_BYTES;

        RTIOsapiHeap_allocateArray(
            &_recv_buffer.pointer,
            size,
            char);

        _recv_buffer.length = size;
        if (_recv_buffer.pointer == NULL)
        {
            fprintf(stderr, "RTIOsapiHeap_allocateArray error\n");
        }

    }

    ~RTISocketSubscriber()
    {
        Shutdown();
    }

    void Shutdown()
    {

        if (_recv_resource != NULL)
        {
            /*TODO fail when is destroy*/
            //_plugin->destroy_recvresource_rrEA(_plugin, &_recv_resource);
        }

    }

    TestMessage *ReceiveMessage()
    {

        NDDS_Transport_Message_t transp_message = NDDS_TRANSPORT_MESSAGE_INVALID;

        //TestData_t data;

        bool result = _plugin->receive_rEA(
                _plugin,
                &transp_message,
                &_recv_buffer,
                &_recv_resource,
                &worker);

        if (!result)
        {
            fprintf(stderr, "error receiving data\n");
            return NULL;
        }

        RTIOsapiMemory_copy(
            &_message,
            _recv_buffer.pointer,
            perftest_cpp::OVERHEAD_BYTES + 8);

        if (_plugin->return_loaned_buffer_rEA != NULL)
        {
            _plugin->return_loaned_buffer_rEA(
                _plugin,
                &_recv_resource,
                &transp_message,
                NULL);
        }

        return &_message;
    }

    void WaitForWriters(int numPublishers)
    {
        /*Dummy Function*/
    }
};

/*********************************************************
 * Initialize
 */
bool RTISocketImpl::Initialize(int argc, char *argv[])
{
    if (!ParseConfig(argc, argv))
    {
        return false;
    }

    // only if we run the latency test we need to wait for pongs after sending pings
    _pongSemaphore = _LatencyTest
        ? RTIOsapiSemaphore_new(RTI_OSAPI_SEMAPHORE_KIND_BINARY, NULL)
        : NULL;

    struct NDDS_Transport_UDPv4_Property_t updv4_prop =
        NDDS_TRANSPORT_UDPV4_PROPERTY_DEFAULT;
    struct NDDS_Transport_Shmem_Property_t shmem_prop =
        NDDS_TRANSPORT_SHMEM_PROPERTY_DEFAULT;

    if (_useShmem)
    {
        _plugin = NDDS_Transport_Shmem_new(&shmem_prop);
    }
    else
    {
        if (_receiveNic != NULL)
        {
            updv4_prop.parent.allow_interfaces_list = &_receiveNic;
            updv4_prop.parent.allow_interfaces_list_length = 1;
            updv4_prop.send_blocking = false;
            updv4_prop.no_zero_copy = false;

            /*
             * updv4_prop.send_socket_buffer_size = 63000;
             * updv4_prop.recv_socket_buffer_size = 63000;
             *
             * The default value for send_socket_buffer_size and receive is
             * 131072 and does not allow to go lower.
             * NDDS_TRANSPORT_UDPV4_SEND_SOCKET_BUFFER_SIZE_DEFAULT   (131072)
             * NDDS_TRANSPORT_UDPV4_RECV_SOCKET_BUFFER_SIZE_DEFAULT   (131072)
             *
             * This could be necesary to modify for large data.
             */

        }

        _plugin = NDDS_Transport_UDPv4_new(&updv4_prop);
    }

    if (_plugin == NULL)
    {
        fprintf(stderr, "error creating transport plugin\n");
        return false;
    }


    return true;
}

/*********************************************************
 * CreateWriter
 */
IMessagingWriter *RTISocketImpl::CreateWriter(const char *topic_name)
{

    int result = 0;

    NDDS_Transport_SendResource_t send_resource = NULL;
    NDDS_Transport_Port_t send_port = 0;
    NDDS_Transport_Address_t dst_address =
        NDDS_TRANSPORT_ADDRESS_INVALID;

    int portOffset = 0;
    if (!strcmp(topic_name, perftest_cpp::_LatencyTopicName))
    {
        portOffset += 1;
    }
    if (!strcmp(topic_name, perftest_cpp::_ThroughputTopicName))
    {
        portOffset += 2;
    }

    send_port = PRESRtps_getWellKnownUnicastPort(
            _DomainID,
            0,
            7400,
            250,
            2,
            portOffset);

    /* Create send resource */
    if (!_useShmem)
    {
        result = NDDS_Transport_UDP_string_to_address_cEA(
                _plugin,
                &dst_address,
                _sendNic);

        if (result != 1)
        {
            fprintf(stderr, "NDDS_Transport_UDP_string_to_address_cEA error\n");
            return NULL;
        }
    }

    printf("Creating send resource, topic name: %s \t port: %d\n", topic_name, send_port);

    result = _plugin->create_sendresource_srEA(
            _plugin,
            &send_resource,
            &dst_address,
            send_port,
            0);
    if (result != 1)
    {
        fprintf(stderr, "create_sendresource_srEA error\n");
        return NULL;
    }

    return new RTISocketPublisher(
            _plugin,
            send_resource,
            dst_address,
            send_port,
            _pongSemaphore);
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
    int result = 0;
    NDDS_Transport_RecvResource_t recv_resource = NULL;
    NDDS_Transport_Port_t recv_port = 0;

    int portOffset = 0;
    if (!strcmp(topic_name, perftest_cpp::_LatencyTopicName) )
    {
        portOffset += 1;
    }
    if (!strcmp(topic_name, perftest_cpp::_ThroughputTopicName))
    {
        portOffset += 2;
    }

    // parameters order (domain_id, participant_id, port_base, domain_id_gain, participant_id_gain, port_offset)
    recv_port = PRESRtps_getWellKnownUnicastPort(
            _DomainID,
            0,
            7400,
            250,
            2,
            portOffset);

    printf("Creating receive resource, topic name: %s \t port: %d\n", topic_name, recv_port);

    /* Create receive resource */
    result = _plugin->create_recvresource_rrEA(
            _plugin,
            &recv_resource,
            &recv_port,
            NULL,
            0);

    if (result != 1)
    {
        fprintf(stderr, "Create_recvresource_rrEA error\n");
        return NULL;
    }

    return new RTISocketSubscriber(
            _plugin,
            recv_resource,
            recv_port);
}

#ifdef RTI_WIN32
#pragma warning(pop)
#endif
