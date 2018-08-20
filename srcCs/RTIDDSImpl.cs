/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using DDS;
using NDDS;

namespace PerformanceTest
{
    class RTIDDSImpl<T> : IMessaging where T : class, DDS.ICopyable<T>
    {

        /*********************************************************
         * Dispose
         */
        public void Dispose()
        {
            Shutdown();
        }

        /*********************************************************
         * Shutdown
         */
        public void Shutdown()
        {
            if (_participant != null)
            {
                lock (_participant)
                {
                    System.Threading.Thread.Sleep(2000);

                    if (_reader != null) {
                        _subscriber.delete_datareader(ref _reader);
                    }

                    System.Threading.Thread.Sleep(4000);
                    _participant.delete_contained_entities();
                    DDS.DomainParticipantFactory.get_instance().delete_participant(ref _participant);
                    _participant = null;
                }
            }
        }

        /*********************************************************
         * GetBatchSize
         */
        public int GetBatchSize()
        {
            return _BatchSize;
        }

        /*********************************************************
         * GetInitializationSampleCount
         */
        public int GetInitializationSampleCount()
        {
            /*
             * There is a minimum number of samples that we want to send no matter
             * what the conditions are:
             */
            int initializeSampleCount = 50;

            /*
             * If we are using reliable, the maximum burst of that we can send is
             * limited by max_send_window_size (or max samples, but we will assume
             * this is not the case for this). In such case we should send
             * max_send_window_size samples.
             *
             * If we are not using reliability this should not matter.
             */
            initializeSampleCount = Math.Max(
                    initializeSampleCount,
                    _SendQueueSize);

            /*
             * If we are using batching we need to take into account tha the Send
             * Queue will be per-batch, therefore for the number of samples:
             */
            if (_BatchSize > 0)
            {
                initializeSampleCount = Math.Max(
                        _SendQueueSize * (_BatchSize / (int)_DataLen),
                        initializeSampleCount);
            }

            return initializeSampleCount;
        }

        /*********************************************************
         * PrintCmdLineHelp
         */
        public void PrintCmdLineHelp()
        {
            string usage_string =
            "\t-sendQueueSize <number>       - Sets number of samples (or batches) in send\n" +
            "\t                                queue, default 50\n" +
            "\t-domain <ID>                  - RTI DDS Domain, default 1\n" +
            "\t-qosFile <filename>           - Name of XML file for DDS Qos profiles,\n" +
            "\t                                default: perftest_qos_profiles.xml\n" +
            "\t-qosLibrary <lib name>        - Name of QoS Library for DDS Qos profiles, \n" +
            "\t                                default: PerftestQosLibrary\n" +
            "\t-bestEffort                   - Run test in best effort mode, default reliable\n" +
            "\t-batchSize <bytes>            - Size in bytes of batched message, default 8kB\n" +
            "\t                                (Disabled for LatencyTest mode or if dataLen > 4kB)\n" +
            "\t-noPositiveAcks               - Disable use of positive acks in reliable \n" +
            "\t                                protocol, default use positive acks\n" +
            "\t-keepDurationUsec <usec>      - Minimum time (us) to keep samples when\n" +
            "\t                                positive acks are disabled, default 1000 us\n" +
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
            "\t-asynchronous                 - Use asynchronous writer\n" +
            "\t                                Default: Not set\n" +
            "\t-flowController <flow>        - In the case asynchronous writer use a specific flow controller.\n" +
            "\t                                There are several flow controller predefined:\n" +
            "\t                                ";
            foreach (string flow in valid_flow_controller)
            {
                usage_string += "\"" + flow + "\" ";
            }
            usage_string += "\n" +
            "\t                                Default: \"default\" (If using asynchronous).\n" +
            "\t-peer <address>               - Adds a peer to the peer host address list.\n" +
            "\t                                This argument may be repeated to indicate multiple peers\n" +
            "\n";
            usage_string += _transport.HelpMessageString();
            usage_string += "\n" +
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

            Console.Error.Write(usage_string);
        }

        /*********************************************************
         * PrintConfiguration
         */
        public string PrintConfiguration()
        {
            StringBuilder sb = new StringBuilder();

            // Domain ID
            sb.Append("\tDomain: ");
            sb.Append(_DomainID);
            sb.Append("\n");

            // Dynamic Data
            sb.Append("\tDynamic Data: ");
            if (_isDynamicData)
            {
                sb.Append("Yes\n");
            }
            else
            {
                sb.Append("No\n");
            }

            // Dynamic Data
            if (_isPublisher)
            {
                sb.Append("\tAsynchronous Publishing: ");
                if (_isLargeData || _IsAsynchronous)
                {
                    sb.Append("Yes\n");
                    sb.Append("\tFlow Controller: ");
                    sb.Append(_FlowControllerCustom);
                    sb.Append("\n");
                }
                else
                {
                    sb.Append("No\n");
                }
            }

            // Turbo Mode / AutoThrottle
            if (_TurboMode)
            {
                sb.Append("\tTurbo Mode: Enabled\n");
            }
            if (_AutoThrottle)
            {
                sb.Append("\tAutoThrottle: Enabled\n");
            }

            // XML File
            sb.Append("\tXML File: ");
            sb.Append(_ProfileFile);
            sb.Append("\n");


            sb.Append("\n");
            sb.Append(_transport.PrintTransportConfigurationSummary());


            // set initial peers and not use multicast
            if (_peer_host_count > 0)
            {
                sb.Append("Initial peers: ");
                for (int i = 0; i < _peer_host_count; ++i)
                {
                    sb.Append(_peer_host[i]);
                    if (i == _peer_host_count - 1)
                    {
                        sb.Append("\n");
                    }
                    else
                    {
                        sb.Append(", ");
                    }
                }
            }

            if (_secureUseSecure)
            {
                sb.Append("\n");
                sb.Append(PrintSecureArgs());
            }

            return sb.ToString();
        }

        /*********************************************************
         * ParseConfig
         */
        bool ParseConfig(int argc, string[] argv)
        {
            ulong minScanSize = (ulong) MAX_PERFTEST_SAMPLE_SIZE.VALUE;
            bool isBatchSizeProvided = false;

            for (int i = 0; i < argc; ++i)
            {
                if ("-pub".StartsWith(argv[i], true, null))
                {
                    _isPublisher = true;
                }
                else if ("-sub".StartsWith(argv[i], true, null))
                {
                    _isPublisher = false;
                }
                else if ("-dynamicData".StartsWith(argv[i], true, null))
                {
                    _isDynamicData = true;
                }
                else if ("-scan".StartsWith(argv[i], true, null))
                {
                    _isScan = true;
                    /*
                    * Check if we have custom scan values. In such case we are just
                    * interested in the minimum one.
                    */
                    if ((i != (argc - 1)) && !argv[1+i].StartsWith("-")) {
                        ++i;
                        ulong auxScan = 0;
                        String[] listScan = argv[i].Split(':');
                        for (int j = 0; j < listScan.Length; j++) {
                            if (!UInt64.TryParse(listScan[j], out auxScan)) {
                                Console.Error.Write(
                                        "-scan <size> value must have the format '-scan <size1>:<size2>:...:<sizeN>'\n");
                                return false;
                            }
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
                } else if ("-dataLen".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <length> after -dataLen\n");
                        return false;
                    }
                    if (!UInt64.TryParse(argv[i], out _DataLen))
                    {
                        Console.Error.Write("Bad dataLen\n");
                        return false;
                    }
                    if (_DataLen < perftest_cs.OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("dataLen must be >= " + perftest_cs.OVERHEAD_BYTES);
                        return false;
                    }
                    if (_DataLen > perftest_cs.getMaxPerftestSampleSizeCS())
                    {
                        Console.Error.WriteLine("dataLen must be <= " + perftest_cs.getMaxPerftestSampleSizeCS());
                        return false;
                    }
                    if (_useUnbounded == 0 && (int)_DataLen > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                        _useUnbounded = Math.Min(
                                (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE,
                                2 * _DataLen);
                    }
                }
                else if ("-unbounded".StartsWith(argv[i], true, null))
                {

                    if ((i == (argc - 1)) || argv[i+1].StartsWith("-"))
                    {
                        _useUnbounded = Math.Min(
                                (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE,
                                2 * _DataLen);
                    } else {
                        ++i;
                        if (!UInt64.TryParse(argv[i], out _useUnbounded))
                        {
                            Console.Error.Write("Bad allocation_threshold value\n");
                            return false;
                        }
                    }

                    if (_useUnbounded < perftest_cs.OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("_useUnbounded must be >= " + perftest_cs.OVERHEAD_BYTES);
                        return false;
                    }
                    if (_useUnbounded > (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE)
                    {
                        Console.Error.WriteLine("_useUnbounded must be <= " +
                                MAX_BOUNDED_SEQ_SIZE.VALUE);
                        return false;
                    }
                }
                else if ("-sendQueueSize".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -sendQueueSize\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _SendQueueSize))
                    {
                        Console.Error.Write("Bad sendQueueSize\n");
                        return false;
                    }
                }
                else if ("-heartbeatPeriod".StartsWith(argv[i],true,null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <period> after -heartbeatPeriod\n");
                        return false;
                    }
                    try {
                        String[] st = argv[i].Split(':');
                        _HeartbeatPeriod.sec = int.Parse(st[0]);
                        _HeartbeatPeriod.nanosec = UInt32.Parse(st[1]);
                    }
                    catch (ArgumentNullException)
                    {
                        Console.Error.Write("Bad heartbeatPeriod\n");
                        return false;
                    }
                } else if ("-fastHeartbeatPeriod".StartsWith(argv[i],true,null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <period> after -fastHeartbeatPeriod\n");
                        return false;
                    }
                    try
                    {
                        String[] st = argv[i].Split(':');
                        _FastHeartbeatPeriod.sec = int.Parse(st[0]);
                        _FastHeartbeatPeriod.nanosec = UInt32.Parse(st[1]);
                    }
                    catch (ArgumentNullException)
                    {
                        Console.Error.Write("Bad fastHeartbeatPeriod\n");
                        return false;
                    }
                }
                else if ("-domain".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <id> after -domain\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _DomainID))
                    {
                        Console.Error.Write("Bad domain id\n");
                        return false;
                    }
                }
                else if ("-qosFile".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing filename after -qosFile\n");
                        return false;
                    }
                    _ProfileFile = argv[i];
                }
                else if ("-qosLibrary".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.WriteLine("Missing library name after -qosLibrary");
                        return false;
                    }
                    _ProfileLibraryName = argv[i];
                }
                else if ("-bestEffort".StartsWith(argv[i], true, null))
                {
                    _IsReliable = false;

                }
                else if ("-durability".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <kind> after -durability\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _Durability))
                    {
                        Console.Error.Write("Bad durability kind\n");
                        return false;
                    }
                    if ((_Durability < 0) || (_Durability > 3))
                    {
                        Console.Error.Write("durability kind needs to be 0(volatile),1(transient local),2(transient),3(persistent) \n");
                        return false;
                    }
                }
                else if ("-noDirectCommunication".StartsWith(argv[i], true, null))
                {
                    _DirectCommunication = false;
                }
                else if ("-instances".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -instances\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _InstanceCount))
                    {
                        Console.Error.Write("Bad count for number of instances\n");
                        return false;
                    }
                    _InstanceMaxCountReader = _InstanceCount;
                    if (_InstanceCount <= 0)
                    {
                        Console.Error.Write("instance count cannot be negative or null\n");
                        return false;
                    }
                }
                else if ("-instanceHashBuckets".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -instanceHashBuckets\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _InstanceHashBuckets))
                    {
                        Console.Error.Write("Bad count for instanceHashBuckets\n");
                        return false;
                    }
                    if (_InstanceHashBuckets <= 0 && _InstanceHashBuckets != -1)
                    {
                        Console.Error.Write(" instanceHashBuckets count cannot be negative or null\n");
                        return false;
                    }
                }
                else if ("-batchSize".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <#bytes> after -batchSize\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _BatchSize))
                    {
                        Console.Error.Write("Bad #bytes for batch\n");
                        return false;
                    }
                    if (_BatchSize < 0 || _BatchSize > MAX_SYNCHRONOUS_SIZE.VALUE)
                    {
                        Console.Error.Write("Batch size '" + _BatchSize +
                                "' should be between [0," +
                                MAX_SYNCHRONOUS_SIZE.VALUE +
                                "]\n");
                        return false;
                    }
                    isBatchSizeProvided = true;
                }
                else if ("-keepDurationUsec".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <usec> after -keepDurationUsec\n");
                        return false;
                    }
                    if (!Int64.TryParse(argv[i], out _KeepDurationUsec))
                    {
                        Console.Error.Write("Bad usec for keep duration\n");
                        return false;
                    }
                }
                else if ("-noPositiveAcks".StartsWith(argv[i], true, null))
                {
                    _UsePositiveAcks = false;
                }
                else if ("-verbosity".StartsWith(argv[i], true, null))
                {
                    int verbosityLevel = 0;
                    if (!Int32.TryParse(argv[++i], out verbosityLevel))
                    {
                        Console.Error.Write("Unexpected value after -verbosity\n");
                        return false;
                    }

                    switch (verbosityLevel) {
                        case 0: NDDS.ConfigLogger.get_instance().set_verbosity(
                                    NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                                Console.Error.Write("Setting verbosity to SILENT\n");
                                break;
                        case 1: NDDS.ConfigLogger.get_instance().set_verbosity(
                                    NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                                Console.Error.Write("Setting verbosity to ERROR\n");
                                break;
                        case 2: NDDS.ConfigLogger.get_instance().set_verbosity(
                                    NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                                Console.Error.Write("Setting verbosity to WARNING\n");
                                break;
                        case 3: NDDS.ConfigLogger.get_instance().set_verbosity(
                                    NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                                Console.Error.Write("Setting verbosity to STATUS_ALL\n");
                                break;
                        default: Console.Error.Write(
                                        "Invalid value for the verbosity parameter." +
                                        " Setting verbosity to ERROR (1)\n");
                                break;
                    }
                }
                else if ("-waitsetDelayUsec".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <usec> after -keepDurationUsec\n");
                        return false;
                    }
                    if (!UInt32.TryParse(argv[i], out _WaitsetDelayUsec))
                    {
                        Console.Error.Write("Bad usec for wait delay Usec\n");
                        return false;
                    }
                    if (_WaitsetDelayUsec < 0)
                    {
                        Console.Error.Write(" waitset delay usec cannot be negative or null\n");
                        return false;
                    }
                }
                else if ("-waitsetEventCount".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -waitsetEventCount\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _WaitsetEventCount))
                    {
                        Console.Error.Write("Bad count for wait delay Usec\n");
                        return false;
                    }
                    if (_WaitsetEventCount < 0)
                    {
                        Console.Error.Write(" waitset event count cannot be negative or null\n");
                        return false;
                    }
                }
                else if ("-latencyTest".StartsWith(argv[i], true, null))
                {
                    _LatencyTest = true;
                }
                else if ("-enableAutoThrottle".StartsWith(argv[i], true, null))
                {
                    _AutoThrottle = true;
                }
                else if ("-enableTurboMode".StartsWith(argv[i], true, null))
                {
                    _TurboMode = true;
                }
                else if ("-secureSign".StartsWith(argv[i], true, null))
                {
                    _secureIsSigned = true;
                    _secureUseSecure = true;
                }
                else if ("-secureEncryptBoth".StartsWith(argv[i], true, null))
                {
                    _secureIsDataEncrypted = true;
                    _secureIsSMEncrypted = true;
                    _secureUseSecure = true;
                }
                else if ("-secureEncryptData".StartsWith(argv[i], true, null))
                {
                    _secureIsDataEncrypted = true;
                    _secureUseSecure = true;
                }
                else if ("-secureEncryptSM".StartsWith(argv[i], true, null))
                {
                    _secureIsSMEncrypted = true;
                    _secureUseSecure = true;
                }
                else if ("-secureEncryptDiscovery".StartsWith(argv[i], true, null))
                {
                    _secureIsDiscoveryEncrypted = true;
                    _secureUseSecure = true;
                } else if ("-secureGovernanceFile".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <file> after -secure:governanceFile\n");
                        return false;
                    }
                    _secureGovernanceFile  = argv[i];
                    Console.Error.Write("Warning -- authentication, encryption, signing arguments " +
                             "will be ignored, and the values specified by the Governance file will " +
                             "be used instead\n");
                    _secureUseSecure = true;
                }
                else if ("-securePermissionsFile".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <file> after -secure:permissionsFile\n");
                        return false;
                    }
                    _securePermissionsFile  = argv[i];
                    _secureUseSecure = true;
                }
                else if ("-secureCertAuthority".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <file> after -secure:certAuthority\n");
                        return false;
                    }
                    _secureCertAuthorityFile = argv[i];
                    _secureUseSecure = true;
                }
                else if ("-secureCertFile".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <file> after -secure:certFile\n");
                        return false;
                    }
                    _secureCertificateFile = argv[i];
                    _secureUseSecure = true;
                }
                else if ("-securePrivateKey".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <file> after -secure:privateKey\n");
                        return false;
                    }
                    _securePrivateKeyFile = argv[i];
                    _secureUseSecure = true;
                }
                else if ("-secureLibrary".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <library> after -secure:privateKey\n");
                        return false;
                    }
                    _secureLibrary = argv[i];
                    _secureUseSecure = true;
                }
                else if ("-secureDebug".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -secureDebug\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _secureDebugLevel))
                    {
                        Console.Error.Write("Bad value for -secureDebug\n");
                        return false;
                    }
                }
                else if ("-asynchronous".StartsWith(argv[i], true, null))
                {
                    _IsAsynchronous = true;
                }
                else if ("-flowController".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <flow Controller Name> after -flowController\n");
                        return false;
                    }
                    _FlowControllerCustom = argv[i].ToString();

                    // verify if the flow controller name is correct, else use "default"
                    bool valid_flow_control = false;
                    foreach (string flow in valid_flow_controller)
                    {
                        if (_FlowControllerCustom.Equals(flow))
                        {
                            valid_flow_control = true;
                        }
                    }

                    if (!valid_flow_control)
                    {
                        Console.Error.Write("Bad <flow> '" + _FlowControllerCustom + "' for custom flow controller\n");
                        _FlowControllerCustom = "default";
                    }
                } else if ("-peer".StartsWith(argv[i], true, null)) {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-")){
                        Console.Error.Write("Missing <address> after -peer\n");
                        return false;
                    }
                    if (_peer_host_count +1 < RTIPERFTEST_MAX_PEERS) {
                        _peer_host[_peer_host_count++] = argv[i];
                    } else {
                        Console.Error.Write("The maximum of -initial peers is " + RTIPERFTEST_MAX_PEERS + "\n");
                        return false;
                    }
                } else if ("-cft".StartsWith(argv[i], true, null)) {
                    _useCft = true;
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-")) {
                        Console.Error.Write("Missing <start>:<end> after -cft\n");
                        return false;
                    }

                    if (argv[i].Contains(":")) {
                        try {
                            String[] st = argv[i].Split(':');
                            if (!Int32.TryParse(st[0], out _CFTRange[0]))
                            {
                                Console.Error.Write("Bad <start> for -cft\n");
                                return false;
                            }
                            if (!Int32.TryParse(st[1], out _CFTRange[1]))
                            {
                                Console.Error.Write("Bad <end> for  -cft\n");
                                return false;
                            }
                        }
                        catch (ArgumentNullException)
                        {
                            Console.Error.Write("Bad cft\n");
                            return false;
                        }
                    } else {
                        if (!Int32.TryParse(argv[i], out _CFTRange[0]))
                        {
                            Console.Error.Write("Bad <start> for  -cft\n");
                            return false;
                        }
                        _CFTRange[1] = _CFTRange[0];
                    }

                    if (_CFTRange[0] > _CFTRange[1]) {
                        Console.Error.Write("-cft <start> value cannot be bigger than <end>\n");
                        return false;
                    }
                    if (_CFTRange[0] < 0 ||
                            _CFTRange[0] >= MAX_CFT_VALUE.VALUE ||
                            _CFTRange[1] < 0 ||
                            _CFTRange[1] >= MAX_CFT_VALUE.VALUE) {
                        Console.Error.Write("-cft <start>:<end> values should be between [0," + MAX_CFT_VALUE.VALUE + "]\n");
                        return false;
                    }
                } else if ("-writeInstance".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <number> after -writeInstance\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _instancesToBeWritten))
                    {
                        Console.Error.Write("Bad <start> for  -cft\n");
                        return false;
                    }
                }
                else
                {
                    try
                    {
                        int value = PerftestTransport.GetTransportCmdLineArgs()[argv[i]];
                        // Increment the counter with the number of arguments
                        // obtained from the map.
                        i = i + value;
                    }
                    catch (KeyNotFoundException)
                    {
                        Console.Error.Write(argv[i] + ": not recognized\n");
                        return false;
                    }
                }
            }

            /* If we are using scan, we get the minimum and set it in Datalen */
            if (_isScan) {
                _DataLen = minScanSize;
            }

            /* Check if we need to enable Large Data. This works also for -scan */
            if (_DataLen > (ulong) Math.Min(
                        MAX_SYNCHRONOUS_SIZE.VALUE,
                        MAX_BOUNDED_SEQ_SIZE.VALUE)) {
                _isLargeData = true;
                if (_useUnbounded == 0) {
                    _useUnbounded = (ulong) MAX_BOUNDED_SEQ_SIZE.VALUE;
                }
            } else { /* No Large Data */
                _isLargeData = false;
            }

            /* If we are using batching */
            if (_BatchSize > 0) {

                /* We will not use batching for a latency test */
                if (_LatencyTest) {
                    if (isBatchSizeProvided) {
                        Console.Error.WriteLine(
                                "Batching cannot be used in a Latency test.");
                        return false;
                    } else {
                        _BatchSize = 0; //Disable Batching
                    }
                }

                /* Check if using asynchronous */
                if (_IsAsynchronous) {
                    if (isBatchSizeProvided) {
                        Console.Error.WriteLine(
                                "Batching cannot be used with asynchronous writing.\n");
                        return false;
                    } else {
                        _BatchSize = 0; //Disable Batching
                    }
                }

                /*
                 * Large Data + batching cannot be set. But batching is enabled by default,
                 * so in that case, we just disabled batching, else, the customer set it up,
                 * so we explitly fail
                 */
                if (_isLargeData) {
                    if (isBatchSizeProvided) {
                        Console.Error.WriteLine(
                                "Batching cannot be used with Large Data.");
                        return false;
                    } else {
                        _BatchSize = -2;
                    }
                } else if ((ulong) _BatchSize < _DataLen * 2) {
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
                    Console.Error.WriteLine("Turbo Mode cannot be used with asynchronous writing.");
                    return false;
                }
                if (_isLargeData) {
                    Console.Error.WriteLine("Turbo Mode disabled, using large data.");
                    _TurboMode = false;
                }
            }

            // Manage _instancesToBeWritten
            if (_instancesToBeWritten != -1) {
                if (_InstanceCount < _instancesToBeWritten) {
                    Console.Error.WriteLine("Specified '-WriteInstance' (" + _instancesToBeWritten +
                            ") invalid: Bigger than the number of instances (" + _InstanceCount + ").");
                    return false;
                }
            }
            if (_isPublisher && _useCft) {
                Console.Error.WriteLine("Content Filtered Topic is not a parameter in the publisher side.\n");
            }

            if (_transport != null)
            {
                if (!_transport.ParseTransportOptions(argv))
                {
                    Console.Error.WriteLine("Failure parsing the transport options.");
                    return false;
                }
            }
            else
            {
                Console.Error.WriteLine("_transport is not initialized");
                return false;
            }

            return true;
        }


        /*********************************************************
         * DomainListener
         */
        class DomainListener : DDS.DomainParticipantListener
        {
            public override void on_inconsistent_topic(
                DDS.Topic topic,
                ref DDS.InconsistentTopicStatus status)
            {
                Console.Error.WriteLine("Found inconsistent topic. Expecting " +
                    topic.get_name() + " of type " + topic.get_type_name());
            }

            public override void on_offered_incompatible_qos(
                DDS.DataWriter writer,
                DDS.OfferedIncompatibleQosStatus status)
            {
                Console.Error.WriteLine("Found incompatible reader for writer " +
                        writer.get_topic().get_name() + ": QoS is "
                        + status.last_policy_id);
            }

            public override void on_requested_incompatible_qos(
                DDS.DataReader reader,
                DDS.RequestedIncompatibleQosStatus status)
            {
                Console.Error.WriteLine("Found incompatible writer for reader " +
                        reader.get_topicdescription().get_name() + ": QoS is "
                        + status.last_policy_id);
            }
        }

        /*********************************************************
         * RTIPublisher
         */
        public class RTIPublisher<Type> : IMessagingWriter where Type : class, DDS.ICopyable<Type>
        {
            protected DataWriter _writer = null;
            protected ITypeHelper<Type> _DataType = null;
            protected int _num_instances;
            protected long _instance_counter;
            protected InstanceHandle_t[] _instance_handles;
            protected Semaphore _pongSemaphore = null;
            protected int _instancesToBeWritten = -1;
            protected bool _isReliable = false;

            public RTIPublisher(
                    DataWriter writer,
                    int num_instances,
                    Semaphore pongSemaphore,
                    ITypeHelper<Type> DataType,
                    int instancesToBeWritten)
            {
                _writer = writer;
                _DataType = DataType;

                _num_instances = num_instances;
                _instance_counter = 0;
                _instance_handles = new DDS.InstanceHandle_t[_num_instances + 1]; // One extra for MAX_CFT_VALUE
                _pongSemaphore = pongSemaphore;
                _instancesToBeWritten = instancesToBeWritten;

                _DataType.setBinDataMax(0);

                for (int i = 0; i < _num_instances; ++i)
                {
                    _DataType.fillKey(i);
                    _instance_handles[i] = writer.register_instance_untyped(
                            _DataType.getData());
                }
                // Register the key of MAX_CFT_VALUE
                _DataType.fillKey(MAX_CFT_VALUE.VALUE);
                _instance_handles[_num_instances] = writer.register_instance_untyped(
                        _DataType.getData());

                DDS.DataWriterQos dw_qos = new DDS.DataWriterQos();
                _writer.get_qos(dw_qos);
                _isReliable = (dw_qos.reliability.kind 
                                == DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS);
            }

            public void Flush()
            {
                _writer.flush();
            }

            public virtual bool Send(TestMessage message, bool isCftWildCardKey)
            {
                int key = 0;
                _DataType.copyFromMessage(message);

                if (!isCftWildCardKey) {
                    if (_num_instances > 1) {
                        if (_instancesToBeWritten == -1) {
                            key = (int) (_instance_counter++ % _num_instances);
                        } else {
                            key = _instancesToBeWritten;
                        }
                    }
                } else {
                    key = MAX_CFT_VALUE.VALUE;
                }
                _DataType.fillKey(key);

                try
                {
                    if (!isCftWildCardKey) {
                        _writer.write_untyped(_DataType.getData(), ref _instance_handles[key]);
                    } else {
                        _writer.write_untyped(_DataType.getData(), ref _instance_handles[_num_instances]);
                    }
                }
                catch (DDS.Exception ex)
                {
                    Console.Error.Write("Write error {0}\n", ex);
                    _DataType.binDataUnloan();
                    return false;
                }
                finally
                {
                    _DataType.binDataUnloan();
                }

                return true;
            }

            public void WaitForReaders(int numSubscribers)
            {
                DDS.PublicationMatchedStatus status = new DDS.PublicationMatchedStatus();

                while (true)
                {
                    _writer.get_publication_matched_status(ref status);
                    if (status.current_count >= numSubscribers)
                    {
                        break;
                    }
                    System.Threading.Thread.Sleep(1000);
                }
            }

            public bool NotifyPingResponse()
            {
                if (_pongSemaphore != null)
                {
                    try {
                        _pongSemaphore.Release();
                    } catch ( System.Exception ex ) {
                        Console.Error.WriteLine("Exception occured: " + ex.Message);
                        return false;
                    }

                }
                return true;
            }

            public bool WaitForPingResponse()
            {
                if (_pongSemaphore != null)
                {
                    try {
                        _pongSemaphore.WaitOne();
                    } catch ( System.Exception ex ) {
                        Console.Error.WriteLine("Exception occured: " + ex.Message);
                        return false;
                    }
                }
                return true;
            }

            public bool WaitForPingResponse(int timeout)
            {
                if (_pongSemaphore != null)
                {
                    try {
                        _pongSemaphore.WaitOne(timeout, false);
                    } catch ( System.Exception ex ) {
                        Console.Error.WriteLine("Exception occured: " + ex.Message);
                        return false;
                    }
                }
                return true;
            }

            public long getPulledSampleCount()
            {
                DataWriterProtocolStatus status = new DataWriterProtocolStatus();
                _writer.get_datawriter_protocol_status(ref status);
                return status.pulled_sample_count;
            }

            public void waitForAck(int sec, uint nsec) {
                if (_isReliable) {
                    try {
                        Duration_t duration = new Duration_t();
                        duration.sec = sec;
                        duration.nanosec = nsec;
                        _writer.wait_for_acknowledgments(duration);
                    } catch (DDS.Retcode_Timeout) {} // Expected exception
                } else {
                    System.Threading.Thread.Sleep((int)nsec / 1000000);
                }
            }
        }

        /*********************************************************
         * RTIDynamicDataPublisher
         */
        class RTIDynamicDataPublisher : RTIPublisher<DynamicData>
        {
            public RTIDynamicDataPublisher(
                    DataWriter writer,
                    int num_instances,
                    Semaphore pongSemaphore,
                    ITypeHelper<DynamicData> DataType,
                    int instancesToBeWritten)
                : base(writer, num_instances, pongSemaphore, DataType, instancesToBeWritten) {}


            public override bool Send(TestMessage message, bool isCftWildCardKey)
            {
                int key = 0;
                _DataType.copyFromMessage(message);
                if (!isCftWildCardKey) {
                    if (_num_instances > 1) {
                        if (_instancesToBeWritten == -1) {
                            key = (int) (_instance_counter++ % _num_instances);
                        } else {
                            key = _instancesToBeWritten;
                        }

                    }
                } else {
                    key = MAX_CFT_VALUE.VALUE;
                }
                _DataType.fillKey(key);
                try
                {
                    if (!isCftWildCardKey) {
                        ((DynamicDataWriter)_writer).write(
                                (DynamicData)_DataType.getData(),
                                ref _instance_handles[key]);
                    } else {
                        ((DynamicDataWriter)_writer).write(
                                (DynamicData)_DataType.getData(),
                                ref _instance_handles[_num_instances]);
                    }
                }
                catch (DDS.Exception ex)
                {
                    Console.Error.Write("Write error {0}\n", ex);
                    _DataType.binDataUnloan();
                    return false;
                }
                finally
                {
                    _DataType.binDataUnloan();
                }

                return true;
            }
        }

        /*********************************************************
         * ReceiverListener
         */
        class ReceiverListener<Type> : DDS.DataReaderListener where Type : class, DDS.ICopyable<Type>
        {
            private DDS.LoanableSequence<Type> _data_seq = null;
            private DDS.SampleInfoSeq _info_seq = new DDS.SampleInfoSeq();
            private TestMessage       _message = new TestMessage();
            private IMessagingCB      _callback;
            private ITypeHelper<Type> _DataType = null;

            public ReceiverListener(IMessagingCB callback, ITypeHelper<Type> DataType)
            {
                _callback = callback;
                _DataType = DataType;
                _data_seq = _DataType.createSequence();
                _message.data = new byte[_DataType.getMaxPerftestSampleSize()];
            }

            public override void on_data_available(DDS.DataReader reader)
            {

                int i;
                int seqLen;

                try
                {
                    reader.take_untyped( _data_seq, _info_seq,
                        DDS.ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                         DDS.SampleStateKind.ANY_SAMPLE_STATE,
                         DDS.ViewStateKind.ANY_VIEW_STATE,
                         DDS.InstanceStateKind.ANY_INSTANCE_STATE);
                }
                catch (DDS.Retcode_NoData)
                {
                    Console.Error.Write("called back no data\n");
                    return;
                }
                catch (System.Exception ex)
                {
                    Console.Error.WriteLine("Error during taking data " + ex.Message);
                    return;
                }

                try
                {
                    seqLen = _data_seq.length;
                    for (i = 0; i < seqLen; ++i)
                    {
                        if (_info_seq.get_at(i).valid_data)
                        {
                            _message = _DataType.copyFromSeqToMessage(_data_seq, i);
                            _callback.ProcessMessage(_message);
                        }
                    }
                }
                catch (DDS.Retcode_NoData)
                {
                    Console.Error.WriteLine("called back no data");
                    return;
                }
                finally
                {
                    try
                    {
                        reader.return_loan_untyped(_data_seq, _info_seq);
                    }
                    catch (System.Exception ex)
                    {
                        Console.Error.WriteLine("Error during return loan " + ex.Message);
                    }
                }
            }
        }

        /*********************************************************
         * RTISubscriber
         */
        class RTISubscriber<Type> : IMessagingReader where Type : class, DDS.ICopyable<Type>
        {
            private ITypeHelper<Type> _DataType = null;
            private DDS.DataReader     _reader = null;
            private DDS.LoanableSequence<Type> _data_seq = null;
            private DDS.SampleInfoSeq  _info_seq = new DDS.SampleInfoSeq();
            private TestMessage         _message = new TestMessage();
            private DDS.WaitSet         _waitset = null;
            private DDS.ConditionSeq _active_conditions = new DDS.ConditionSeq();

            private int      _data_idx = 0;
            private bool      _no_data = true;

            public RTISubscriber(DDS.DataReader reader, ITypeHelper<Type> DataType)
            {
                _reader = reader;
                _DataType = DataType;
                _data_seq = _DataType.createSequence();
                _message.data = new byte[_DataType.getMaxPerftestSampleSize()];

                // null listener means using receive thread
                if (_reader.get_listener() == null)
                {

                    DDS.WaitSetProperty_t property = new DDS.WaitSetProperty_t();
                    property.max_event_count = _WaitsetEventCount;
                    property.max_event_delay.sec = (int)_WaitsetDelayUsec / 1000000;
                    property.max_event_delay.nanosec = (_WaitsetDelayUsec % 1000000) * 1000;

                    _waitset = new DDS.WaitSet(ref property);

                    DDS.StatusCondition reader_status;

                    reader_status = reader.get_statuscondition();
                    reader_status.set_enabled_statuses((DDS.StatusMask)DDS.StatusKind.DATA_AVAILABLE_STATUS);
                    _waitset.attach_condition(reader_status);
                }
            }

            public void Shutdown()
            {
                // loan may be outstanding during shutdown
                _reader.return_loan_untyped(_data_seq, _info_seq);
            }

            public TestMessage ReceiveMessage()
            {

                while (true) {
                    // no outstanding reads
                    if (_no_data)
                    {
                        _waitset.wait(_active_conditions, DDS.Duration_t.DURATION_INFINITE);

                        if (_active_conditions.length == 0)
                        {
                            //Console.Error.Write("Read thread woke up but no data\n");
                            //return null;
                            continue;
                        }

                        try
                        {
                            _reader.take_untyped(_data_seq, _info_seq,
                                DDS.ResourceLimitsQosPolicy.LENGTH_UNLIMITED,
                                DDS.SampleStateKind.ANY_SAMPLE_STATE,
                                DDS.ViewStateKind.ANY_VIEW_STATE,
                                DDS.InstanceStateKind.ANY_INSTANCE_STATE);
                        }
                        catch (DDS.Retcode_NoData)
                        {
                            continue;
                        }
                        catch (DDS.Exception ex)
                        {
                            Console.Error.Write("Error during taking data {0}\n", ex);
                            return null;
                        }

                        _data_idx = 0;
                        _no_data = false;
                    }

                    int seq_length = _data_seq.length;
                    // check to see if hit end condition
                    if (_data_idx == seq_length)
                    {
                        _reader.return_loan_untyped(_data_seq, _info_seq);
                        _no_data = true;
                        // for some reason, woke up, only got meta-data messages
                        continue;
                    }

                    // skip non-valid data
                    while ( (!_info_seq.get_at(_data_idx).valid_data) &&
                            (++_data_idx < seq_length));

                    // may have hit end condition
                    if (_data_idx == seq_length) { continue; }

                    _message = _DataType.copyFromSeqToMessage(_data_seq, _data_idx);

                    ++_data_idx;

                    return _message;
                }
            }

            public void WaitForWriters(int numPublishers)
            {
                DDS.SubscriptionMatchedStatus status = new DDS.SubscriptionMatchedStatus();

                while (true)
                {
                    _reader.get_subscription_matched_status(ref status);
                    if (status.current_count >= numPublishers)
                    {
                        break;
                    }
                    System.Threading.Thread.Sleep(1000);
                }
            }
        }

        /*********************************************************
         * Constructor
         */
        public RTIDDSImpl(ITypeHelper<T> myDataTypeHelper)
        {
            _DataTypeHelper = myDataTypeHelper;
            _transport = new PerftestTransport();

            _qoSProfileNameMap.Add(LATENCY_TOPIC_NAME.VALUE, "LatencyQos");
            _qoSProfileNameMap.Add(ANNOUNCEMENT_TOPIC_NAME.VALUE, "AnnouncementQos");
            _qoSProfileNameMap.Add(THROUGHPUT_TOPIC_NAME.VALUE, "ThroughputQos");
        }

        /*********************************************************
         * Initialize
         */
        public bool Initialize(int argc, string[] argv)
        {
            _typename = _DataTypeHelper.getTypeSupport().get_type_name_untyped();

            DDS.DomainParticipantQos qos = new DDS.DomainParticipantQos();
            DDS.DomainParticipantFactoryQos factory_qos = new DDS.DomainParticipantFactoryQos();
            DomainListener listener = new DomainListener();

            _factory = DDS.DomainParticipantFactory.get_instance();

            if (!ParseConfig(argc, argv))
            {
                return false;
            }

            if (_LatencyTest)
            {
                _pongSemaphore = new Semaphore(0, 1);
            }

            // setup the QOS profile file to be loaded
            _factory.get_qos(factory_qos);
            factory_qos.profile.url_profile.ensure_length(1, 1);
            factory_qos.profile.url_profile.set_at(0, _ProfileFile);
            _factory.set_qos(factory_qos);


            try
            {
                _factory.reload_profiles();
            }
            catch (DDS.Exception e)
            {
                Console.Error.Write("Problem opening QOS profiles file " + _ProfileFile + ".\n");
                Console.Error.WriteLine(e.Message);
                return false;
            }

            try
            {
                _factory.set_default_library(_ProfileLibraryName);
            }
            catch (DDS.Exception e)
            {
                Console.Error.Write("No QOS Library named \"" + _ProfileLibraryName + "\" found in " +
                    _ProfileFile + ".\n");
                Console.Error.WriteLine(e.Message);
                return false;
            }

            // Configure DDSDomainParticipant QOS
            _factory.get_participant_qos_from_profile(qos, _ProfileLibraryName, "BaseProfileQos");

            if (_secureUseSecure) {
                // validate arguments
                if (!ValidateSecureArgs()) {
                    Console.Error.WriteLine("Failure validating arguments");
                    return false;
                }
                ConfigureSecurePlugin(qos);
            }

            // set initial peers and not use multicast
            if (_peer_host_count > 0) {
                qos.discovery.initial_peers.ensure_length(_peer_host_count, _peer_host_count);
                qos.discovery.initial_peers.from_array(_peer_host);
                qos.discovery.multicast_receive_addresses = new DDS.StringSeq();
            }

            if (!_transport.ConfigureTransport(qos))
            {
                return false;
            }

            if (_AutoThrottle) {
            	try
                {
                    DDS.PropertyQosPolicyHelper.add_property(qos.property_qos,
                                             "dds.domain_participant.auto_throttle.enable",
                                             "true",
                                             false);
                }
                catch (DDS.Exception e)
                {
                    Console.Error.WriteLine("! unable to set property dds.domain_participant.auto_throttle.enable");
                    Console.Error.WriteLine(e.Message);
                }
            }

            // Creates the participant
            _participant = _factory.create_participant(
                _DomainID, qos, listener,
                (DDS.StatusMask)
                (DDS.StatusKind.INCONSISTENT_TOPIC_STATUS |
                 DDS.StatusKind.OFFERED_INCOMPATIBLE_QOS_STATUS |
                 DDS.StatusKind.REQUESTED_INCOMPATIBLE_QOS_STATUS));

            if (_participant == null) {
                Console.Error.Write("Problem creating participant.\n");
                return false;
            }

            // Register the types and create the topics
            _DataTypeHelper.getTypeSupport().register_type_untyped(_participant, _typename);

            // Create the Publisher and Subscriber
            {

                _publisher = _participant.create_publisher_with_profile(
                    _ProfileLibraryName, "BaseProfileQos", null, DDS.StatusMask.STATUS_MASK_NONE);

                if (_publisher == null)
                {
                    Console.Error.WriteLine("Problem creating publisher.");
                    return false;
                }

                _subscriber = _participant.create_subscriber_with_profile(
                    _ProfileLibraryName, "BaseProfileQos", null, DDS.StatusMask.STATUS_MASK_NONE);

                if (_subscriber == null)
                {
                    Console.Error.WriteLine("Problem creating subscriber.");
                    return false;
                }

            }

            return true;
        }

        /*********************************************************
         * ValidateSecureArgs
         */
        private bool ValidateSecureArgs()
        {
            if (_secureUseSecure) {
                if (_securePrivateKeyFile == null) {
                    if (_isPublisher) {
                        _securePrivateKeyFile = SECUREPRIVATEKEYFILEPUB;
                    } else {
                        _securePrivateKeyFile = SECUREPRIVATEKEYFILESUB;
                    }
                }

                if (_secureCertificateFile == null) {
                    if (_isPublisher) {
                        _secureCertificateFile = SECURECERTIFICATEFILEPUB;
                    } else {
                        _secureCertificateFile = SECURECERTIFICATEFILESUB;
                    }
                }

                if (_secureCertAuthorityFile == null) {
                    _secureCertAuthorityFile = SECURECERTAUTHORITYFILE;
                }

                if (_securePermissionsFile == null) {
                    if (_isPublisher) {
                        _securePermissionsFile = SECUREPERMISIONFILEPUB;
                    } else {
                        _securePermissionsFile = SECUREPERMISIONFILESUB;
                    }
                }

                if (_secureLibrary == null) {
                    _secureLibrary = SECURELIBRARYNAME;
                }
            }

            return true;
        }

        /*********************************************************
         * printSecureArgs
         */
        private string PrintSecureArgs()
        {

            string secure_arguments_string =
                    "Secure Arguments:\n" +
                    "\t encrypt discovery: " + _secureIsDiscoveryEncrypted + "\n" +
                    "\t encrypt topic (user) data: " + _secureIsDataEncrypted + "\n" +
                    "\t encrypt submessage: " + _secureIsSMEncrypted + "\n" +
                    "\t sign data: " + _secureIsSigned + "\n";

            if (_secureGovernanceFile != null)
            {
                secure_arguments_string += "\t governance file: " + _secureGovernanceFile
                        + "\n";
            }
            else
            {
                secure_arguments_string += "\t governance file: Not specified\n";
            }

            if (_securePermissionsFile != null)
            {
                secure_arguments_string += "\t permissions file: " + _securePermissionsFile
                        + "\n";
            }
            else
            {
                secure_arguments_string += "\t permissions file: Not specified\n";
            }

            if (_securePrivateKeyFile != null)
            {
                secure_arguments_string += "\t private key file: " + _securePrivateKeyFile
                        + "\n";
            }
            else
            {
                secure_arguments_string += "\t private key file: Not specified\n";
            }

            if (_secureCertificateFile != null)
            {
                secure_arguments_string += "\t certificate file: " + _secureCertificateFile
                        + "\n";
            }
            else
            {
                secure_arguments_string += "\t certificate file: Not specified\n";
            }

            if (_secureCertAuthorityFile != null)
            {
                secure_arguments_string += "\t certificate authority file: "
                        + _secureCertAuthorityFile + "\n";
            }
            else
            {
                secure_arguments_string += "\t certificate authority file: Not specified\n";
            }

            if (_secureLibrary != null)
            {
                secure_arguments_string += "\t plugin library: " + _secureLibrary + "\n";
            }
            else
            {
                secure_arguments_string += "\t plugin library: Not specified\n";
            }

            if (_secureDebugLevel != -1)
            {
                secure_arguments_string += "\t debug level: " + _secureDebugLevel + "\n";
            }
            return secure_arguments_string;
        }

        /*********************************************************
         * ConfigureSecurePlugin
         */
        private void ConfigureSecurePlugin(DDS.DomainParticipantQos dpQos)
        {
            // configure use of security plugins, based on provided arguments

            // load plugin
            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.load_plugin",
                    "com.rti.serv.secure",
                    false);

            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.create_function",
                    "RTI_Security_PluginSuite_create",
                    false);

            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.library",
                    _secureLibrary,
                    false);

            /*
             * Below, we are using com.rti.serv.secure properties in order to
             * be backward compatible with RTI Connext DDS 5.3.0 and below.
             * Later versions use the properties that are specified in the DDS
             * Security specification (see also the RTI Security Plugins
             * Getting Started Guide). However, later versions still support
             * the legacy properties as an alternative.
             */

            // check if governance file provided
            if (_secureGovernanceFile == null)
            {
                // choose a pre-built governance file
                _secureGovernanceFile = "resource/secure/signed_PerftestGovernance_";

                if (_secureIsDiscoveryEncrypted)
                {
                    _secureGovernanceFile += "Discovery";
                }

                if (_secureIsSigned)
                {
                    _secureGovernanceFile += "Sign";
                }

                if (_secureIsDataEncrypted && _secureIsSMEncrypted)
                {
                    _secureGovernanceFile += "EncryptBoth";
                }
                else if (_secureIsDataEncrypted)
                {
                    _secureGovernanceFile += "EncryptData";
                }
                else if (_secureIsSMEncrypted)
                {
                    _secureGovernanceFile += "EncryptSubmessage";
                }

                _secureGovernanceFile += ".xml";
                
                DDS.PropertyQosPolicyHelper.add_property(
                        dpQos.property_qos,
                        "com.rti.serv.secure.access_control.governance_file",
                        _secureGovernanceFile,
                        false);
            }
            else
            {
                DDS.PropertyQosPolicyHelper.add_property(
                        dpQos.property_qos,
                        "com.rti.serv.secure.access_control.governance_file",
                        _secureGovernanceFile,
                        false);
            }

            // permissions file
            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.access_control.permissions_file",
                    _securePermissionsFile,
                    false);

            // permissions authority file
            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.access_control.permissions_authority_file",
                    _secureCertAuthorityFile,
                    false);

            // certificate authority
            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.authentication.ca_file",
                    _secureCertAuthorityFile,
                    false);

            // public key
            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.authentication.certificate_file",
                    _secureCertificateFile,
                    false);

            // private key
            DDS.PropertyQosPolicyHelper.add_property(
                    dpQos.property_qos,
                    "com.rti.serv.secure.authentication.private_key_file",
                    _securePrivateKeyFile,
                    false);

            if (_secureDebugLevel != -1)
            {
                DDS.PropertyQosPolicyHelper.add_property(
                        dpQos.property_qos,
                        "com.rti.serv.secure.logging.log_level",
                        _secureDebugLevel.ToString(),
                        false);
            }
        }


        /*********************************************************
          * CreateWriter
          */
        public IMessagingWriter CreateWriter(string topic_name)
        {
            DDS.DataWriterQos dw_qos = new DDS.DataWriterQos();
            string qos_profile = null;

            DDS.Topic topic = _participant.create_topic(
                               topic_name, _typename,
                               DDS.DomainParticipant.TOPIC_QOS_DEFAULT, null,
                               DDS.StatusMask.STATUS_MASK_NONE);

            if (topic == null)
            {
                Console.Error.WriteLine("Problem creating topic " + topic_name);
                return null;
            }

            qos_profile = getQoSProfileName(topic_name);
            if (qos_profile == null)
            {
                Console.Error.WriteLine("Problem getting qos profile.\n");
                return null;
            }

            try
            {
                _factory.get_datawriter_qos_from_profile(dw_qos, _ProfileLibraryName, qos_profile);
            }
            catch (DDS.Exception e)
            {
                Console.Error.Write("No QOS Profile named \"" + qos_profile + "\" found in QOS Library \""
                              + _ProfileLibraryName + "\" in file " + _ProfileFile + ".\n");
                Console.Error.WriteLine(e.Message);
                return null;
            }

            if (!_UsePositiveAcks 
                    && (qos_profile == "ThroughputQos"
                        || qos_profile == "LatencyQos")) {
                dw_qos.protocol.disable_positive_acks = true;
                if (_KeepDurationUsec != -1) {
                    dw_qos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration =
                        DDS.Duration_t.from_micros((ulong)_KeepDurationUsec);
                }
            }

            if (_isLargeData || _IsAsynchronous)
            {
                dw_qos.publish_mode.kind = DDS.PublishModeQosPolicyKind.ASYNCHRONOUS_PUBLISH_MODE_QOS;
                if (!_FlowControllerCustom.StartsWith("default", true, null))
                {
                    dw_qos.publish_mode.flow_controller_name = "dds.flow_controller.token_bucket." + _FlowControllerCustom;
                }
            }

            // only force reliability on throughput/latency topics
            if (topic_name != ANNOUNCEMENT_TOPIC_NAME.VALUE)
            {
                if (_IsReliable)
                {
                    // default: use the setting specified in the qos profile
                    // dw_qos.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
                }
                else
                {
                    // override to best-effort
                    dw_qos.reliability.kind = DDS.ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
                }
            }

            // These QOS's are only set for the Throughput datawriter
            if (qos_profile == "ThroughputQos")
            {

                if (_transport.useMulticast) {
                    dw_qos.protocol.rtps_reliable_writer.enable_multicast_periodic_heartbeat = true;
                }

                if (_BatchSize > 0)
                {
                    dw_qos.batch.enable = true;
                    dw_qos.batch.max_data_bytes = _BatchSize;
                    dw_qos.resource_limits.max_samples = DDS.ResourceLimitsQosPolicy.LENGTH_UNLIMITED;
                    dw_qos.writer_resource_limits.max_batches = _SendQueueSize;
                }
                else
                {
                    dw_qos.resource_limits.max_samples = _SendQueueSize;
                }

                if (_HeartbeatPeriod.sec > 0 || _HeartbeatPeriod.nanosec > 0)
                {
                    dw_qos.protocol.rtps_reliable_writer.heartbeat_period = _HeartbeatPeriod;
                    // make the late joiner heartbeat compatible
                    dw_qos.protocol.rtps_reliable_writer.late_joiner_heartbeat_period = _HeartbeatPeriod;
                }

                if (_FastHeartbeatPeriod.sec > 0 || _FastHeartbeatPeriod.nanosec > 0)
                {
                    dw_qos.protocol.rtps_reliable_writer.fast_heartbeat_period = _FastHeartbeatPeriod;
                }
				if (_AutoThrottle) {
	            	try
	                {
	                    DDS.PropertyQosPolicyHelper.add_property(dw_qos.property_qos,
	                                             "dds.data_writer.auto_throttle.enable",
	                                             "true",
	                                             false);
	                }
	                catch (DDS.Exception e)
	                {
	                    Console.Error.WriteLine("! unable to set property dds.data_writer.auto_throttle.enable");
	                    Console.Error.WriteLine(e.Message);
	                }
            	}
            	if (_TurboMode) {
	            	try
	                {
	                    DDS.PropertyQosPolicyHelper.add_property(dw_qos.property_qos,
	                                             "dds.data_writer.enable_turbo_mode",
	                                             "true",
	                                             false);
	                }
	                catch (DDS.Exception e)
	                {
	                    Console.Error.WriteLine("! unable to set property dds.data_writer.enable_turbo_mode");
	                    Console.Error.WriteLine(e.Message);
	                }

                    dw_qos.batch.enable = false;
                    dw_qos.resource_limits.max_samples = DDS.ResourceLimitsQosPolicy.LENGTH_UNLIMITED;
                    dw_qos.writer_resource_limits.max_batches = _SendQueueSize;
            	}
                dw_qos.resource_limits.initial_samples = _SendQueueSize;
                dw_qos.resource_limits.max_samples_per_instance
                    = dw_qos.resource_limits.max_samples;

                dw_qos.durability.kind = (DDS.DurabilityQosPolicyKind)_Durability;
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

                dw_qos.protocol.rtps_reliable_writer.max_send_window_size =
                    _SendQueueSize;
                dw_qos.protocol.rtps_reliable_writer.min_send_window_size =
                    _SendQueueSize;
            }


            if ("LatencyQos" == qos_profile
                    && !_DirectCommunication
                    && ((DDS.DurabilityQosPolicyKind)_Durability 
                            == DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS
                        || (DDS.DurabilityQosPolicyKind)_Durability 
                            == DDS.DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS)) {

                dw_qos.durability.kind = (DDS.DurabilityQosPolicyKind)_Durability;
                dw_qos.durability.direct_communication = _DirectCommunication;
            }

            dw_qos.resource_limits.max_instances = _InstanceCount + 1; // One extra for MAX_CFT_VALUE
            dw_qos.resource_limits.initial_instances = _InstanceCount + 1;

            if (_InstanceCount > 1) {
                if (_InstanceHashBuckets > 0) {
                    dw_qos.resource_limits.instance_hash_buckets = _InstanceHashBuckets;
                } else {
                    dw_qos.resource_limits.instance_hash_buckets = _InstanceCount;
                }
            }

            if (_useUnbounded > 0) {
                DDS.PropertyQosPolicyHelper.add_property(dw_qos.property_qos,
                        "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
                        _useUnbounded.ToString(), false);
            }

            DataWriter writer = _publisher.create_datawriter(
                    topic,
                    dw_qos,
                    null,
                    StatusMask.STATUS_MASK_NONE);

            if (writer == null)
            {
                Console.Error.Write("Problem creating writer.\n");
                return null;
            }

            if (new DynamicData() is T)
            {
                return new RTIDynamicDataPublisher(
                    (DynamicDataWriter)writer,
                    _InstanceCount,
                    _pongSemaphore,
                    (DynamicDataTypeHelper)_DataTypeHelper.clone(),
                    _instancesToBeWritten);
            }
            else
            {
                return new RTIPublisher<T>(
                    writer,
                    _InstanceCount,
                    _pongSemaphore,
                    _DataTypeHelper.clone(),
                    _instancesToBeWritten);
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
         * Beside, there is a special case where all the subscribers will receive the samples, it is MAX_CFT_VALUE = 65535 = [255,255,0,0,]
         */
        public DDS.ContentFilteredTopic createCft(String topic_name, DDS.Topic topic) {
            string condition;
            DDS.StringSeq parameters = new DDS.StringSeq();
            if (_CFTRange[0] == _CFTRange[1]) { // If same elements, no range
                parameters.ensure_length(KEY_SIZE.VALUE,KEY_SIZE.VALUE);
                Console.Error.WriteLine("CFT enabled for instance: '"+_CFTRange[0]+"'");
                for (int i = 0; i < KEY_SIZE.VALUE ; i++) {
                    parameters.set_at(i,Convert.ToString((byte)(_CFTRange[0] >> i * 8)));
                }
                condition = "(%0 = key[0] AND  %1 = key[1] AND %2 = key[2] AND  %3 = key[3]) OR " +
                        "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";
            } else { // If range
                parameters.ensure_length(KEY_SIZE.VALUE * 2,KEY_SIZE.VALUE * 2);
                Console.Error.WriteLine("CFT enabled for instance range: ["+_CFTRange[0]+","+_CFTRange[1]+"] ");
                for (int i = 0; i < KEY_SIZE.VALUE * 2 ; i++) {
                    if ( i < KEY_SIZE.VALUE ) {
                        parameters.set_at(i,Convert.ToString((byte)(_CFTRange[0] >> i * 8)));
                    } else { // KEY_SIZE < i < KEY_SIZE * 2
                        parameters.set_at(i,Convert.ToString((byte)(_CFTRange[1] >> i * 8)));
                    }
                }
                condition = "" +
                        "(" +
                            "(" +
                                "(%3 < key[3]) OR" +
                                "(%3 <= key[3] AND %2 < key[2]) OR" +
                                "(%3 <= key[3] AND %2 <= key[2] AND %1 < key[1]) OR" +
                                "(%3 <= key[3] AND %2 <= key[2] AND %1 <= key[1] AND %0 <= key[0])" +
                            ") AND (" +
                                "(%7 > key[3]) OR" +
                                "(%7 >= key[3] AND %6 > key[2]) OR" +
                                "(%7 >= key[3] AND %6 >= key[2] AND %5 > key[1]) OR" +
                                "(%7 >= key[3] AND %6 >= key[2] AND %5 >= key[1] AND %4 >= key[0])" +
                            ") OR (" +
                                "255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3]" +
                        ")";
            }
            return _participant.create_contentfilteredtopic(
                    topic_name, topic, condition,  parameters);
        }

        /*********************************************************
         * CreateReader
         */
        public IMessagingReader CreateReader(string topic_name, IMessagingCB callback)
        {
            DDS.DataReaderQos dr_qos = new DDS.DataReaderQos();
            string qos_profile = null;
            DDS.ITopicDescription topic_desc = null;

            DDS.Topic topic = _participant.create_topic(
                               topic_name, _typename,
                               DDS.DomainParticipant.TOPIC_QOS_DEFAULT, null,
                               DDS.StatusMask.STATUS_MASK_NONE);

            if (topic == null)
            {
                Console.Error.WriteLine("Problem creating topic " + topic_name);
                return null;
            }

            qos_profile = getQoSProfileName(topic_name);
            if (qos_profile == null)
            {
                Console.Error.WriteLine("Problem getting qos profile.\n");
                return null;
            }

            try
            {
                _factory.get_datareader_qos_from_profile(dr_qos, _ProfileLibraryName, qos_profile);
            }
            catch (DDS.Exception e)
            {
                Console.Error.Write("No QOS Profile named \"" + qos_profile + "\" found in QOS Library \""
                              + _ProfileLibraryName + "\" in file " + _ProfileFile + ".\n");
                Console.Error.WriteLine(e.Message);
                return null;
            }

            // only force reliability on throughput/latency topics
            if (topic_name != ANNOUNCEMENT_TOPIC_NAME.VALUE)
            {
                if (_IsReliable)
                {
                    dr_qos.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
                }
                else
                {
                    dr_qos.reliability.kind = DDS.ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
                }
            }

            if (!_UsePositiveAcks 
                    && (qos_profile == "ThroughputQos"
                            || qos_profile == "LatencyQos")) {
                dr_qos.protocol.disable_positive_acks = true;
            }

            if ("ThroughputQos" == qos_profile 
                    || ("LatencyQos" == qos_profile
                        && !_DirectCommunication
                        && ((DDS.DurabilityQosPolicyKind)_Durability 
                                == DDS.DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS
                            || (DDS.DurabilityQosPolicyKind)_Durability 
                                == DDS.DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS))) {

                dr_qos.durability.kind = (DDS.DurabilityQosPolicyKind)_Durability;
                dr_qos.durability.direct_communication = _DirectCommunication;
            }

            dr_qos.resource_limits.initial_instances = _InstanceCount + 1;
            if (_InstanceMaxCountReader != -1) {
                _InstanceMaxCountReader++;
            }
            dr_qos.resource_limits.max_instances = _InstanceMaxCountReader;

            if (_InstanceCount > 1) {
                if (_InstanceHashBuckets > 0) {
                    dr_qos.resource_limits.instance_hash_buckets = _InstanceHashBuckets;
                } else {
                    dr_qos.resource_limits.instance_hash_buckets = _InstanceCount;
                }
            }

            if (_transport.useMulticast && _transport.AllowsMulticast())
            {
                string multicast_addr;

                multicast_addr = _transport.getMulticastAddr(topic_name);
                if (multicast_addr == null)
                {
                    Console.Error.WriteLine("topic name must either be "
                            + THROUGHPUT_TOPIC_NAME.VALUE
                            + " or " + LATENCY_TOPIC_NAME.VALUE
                            + " or " + ANNOUNCEMENT_TOPIC_NAME.VALUE);
                    return null;
                }

                DDS.TransportMulticastSettings_t multicast_setting = new DDS.TransportMulticastSettings_t();
                multicast_setting.receive_address = multicast_addr;
                multicast_setting.receive_port = 0;
                multicast_setting.transports.length = 0;
                dr_qos.multicast.value.ensure_length(1, 1);
                dr_qos.multicast.value.set_at(0, multicast_setting);
            }

            DDS.DataReader reader = null;
            ReceiverListener<T> reader_listener = null;
            StatusMask statusMask = StatusMask.STATUS_MASK_NONE;

            if (callback != null)
            {
                reader_listener = new ReceiverListener<T>(callback, _DataTypeHelper.clone());
                statusMask = (DDS.StatusMask)DDS.StatusKind.DATA_AVAILABLE_STATUS;

            }

            if (_useUnbounded > 0) {
                DDS.PropertyQosPolicyHelper.add_property(dr_qos.property_qos,
                        "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                        _useUnbounded.ToString(), false);
            }

            if ( _useCft && topic_name == THROUGHPUT_TOPIC_NAME.VALUE)
            {
                topic_desc = createCft(topic_name, topic);
                if (topic_desc == null) {
                    Console.Error.WriteLine("Create_contentfilteredtopic error");
                    return null;
                }
            } else {
                topic_desc = topic;
            }

            reader = _subscriber.create_datareader(
                    topic_desc,
                    dr_qos,
                    reader_listener,
                    statusMask);

            if (reader == null)
            {
                Console.Error.Write("Problem creating reader.\n");
                return null;
            }

            return new RTISubscriber<T>(reader, _DataTypeHelper.clone());
        }

        public string getQoSProfileName(string topicName)
        {
            string name;
            if (_qoSProfileNameMap.TryGetValue(topicName, out name)) {
                return name;
            }
            else {
                Console.Error.WriteLine("topic name must either be "
                        + THROUGHPUT_TOPIC_NAME.VALUE
                        + " or " + LATENCY_TOPIC_NAME.VALUE
                        + " or " + ANNOUNCEMENT_TOPIC_NAME.VALUE);
                return null;
            }
        }

        static int RTIPERFTEST_MAX_PEERS = 1024;

        private int    _SendQueueSize = 50;
        private ulong    _DataLen = 100;
        private ulong     _useUnbounded = 0;
        private int    _DomainID = 1;
        private string _ProfileFile = "perftest_qos_profiles.xml";
        private bool   _IsReliable = true;
        private bool   _AutoThrottle = false;
        private bool   _TurboMode = false;
        private int    _BatchSize = (int)DEFAULT_THROUGHPUT_BATCH_SIZE.VALUE; // Default 8 kB
        private int    _InstanceCount = 1;
        private int    _InstanceMaxCountReader = -1;
        private int     _InstanceHashBuckets = -1;
        private int     _Durability = 0;
        private bool    _DirectCommunication = true;
        private long    _KeepDurationUsec = -1;
        private bool   _UsePositiveAcks = true;
        private bool    _LatencyTest = false;
        private bool   _isLargeData = false;
        private bool   _isScan = false;
        private bool   _isPublisher = false;
        private bool _IsAsynchronous = false;
        private bool _isDynamicData = false;
        private string _FlowControllerCustom = "default";
        string[] valid_flow_controller = { "default", "1Gbps", "10Gbps" };
        private int     _peer_host_count = 0;
        private string[] _peer_host = new string[RTIPERFTEST_MAX_PEERS];
        private bool    _useCft = false;
        private int     _instancesToBeWritten = -1;
        private int[]   _CFTRange = {0,0};
        private PerftestTransport _transport;



        /* Security related variables */
        private bool _secureUseSecure = false;
        private bool _secureIsSigned = false;
        private bool _secureIsDataEncrypted = false; // User Data
        private bool _secureIsSMEncrypted = false; // Sub-message
        private bool _secureIsDiscoveryEncrypted = false;
        private string _secureCertAuthorityFile = null;
        private string _secureCertificateFile = null;
        private string _securePrivateKeyFile = null;

        /*
         * if _GovernanceFile is specified, overrides the options for
         * signing, encrypting, and authentication.
         */
        private string _secureGovernanceFile = null;
        private string _securePermissionsFile = null;
        private string _secureLibrary = null;
        private int _secureDebugLevel = -1;

        private DDS.Duration_t _HeartbeatPeriod = DDS.Duration_t.DURATION_ZERO;     /* this means, use the perftest_qos_profiles.xml QoS file value*/
        private DDS.Duration_t _FastHeartbeatPeriod = DDS.Duration_t.DURATION_ZERO; /* this means, use the perftest_qos_profiles.xml QoS file value*/

        private static int   _WaitsetEventCount = 5;
        private static uint  _WaitsetDelayUsec = 100;

        private static string SECUREPRIVATEKEYFILEPUB = "./resource/secure/pubkey.pem";
        private static string SECUREPRIVATEKEYFILESUB = "./resource/secure/subkey.pem";
        private static string SECURECERTIFICATEFILEPUB = "./resource/secure/pub.pem";
        private static string SECURECERTIFICATEFILESUB = "./resource/secure/sub.pem";
        private static string SECURECERTAUTHORITYFILE = "./resource/secure/cacert.pem";
        private static string SECUREPERMISIONFILEPUB = "./resource/secure/signed_PerftestPermissionsPub.xml";
        private static string SECUREPERMISIONFILESUB = "./resource/secure/signed_PerftestPermissionsSub.xml";
        private static string SECURELIBRARYNAME = "nddssecurity";

        private string _ProfileLibraryName = "PerftestQosLibrary";

        private DDS.DomainParticipantFactory _factory = null;
        private DDS.DomainParticipant        _participant = null;
        private DDS.Subscriber               _subscriber = null;
        private DDS.Publisher                _publisher = null;
        private DDS.DataReader               _reader = null;
        private string                       _typename = null;
        private ITypeHelper<T>                  _DataTypeHelper = null;

        private Semaphore _pongSemaphore = null;

        private SortedDictionary<string, string> _qoSProfileNameMap =
                new SortedDictionary<string, string>();
    }
}
