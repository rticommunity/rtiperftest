/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Threading;
using System.Threading.Tasks;
using System.Text;
using System.Timers;
using System.Runtime.CompilerServices;

namespace PerformanceTest
{
    public class Perftest : IDisposable
    {
        public Parameters parameters;
        private ulong dataSize = 100;
        private ulong numIter = 100000000;
        private ulong spinLoopCount = 0;
        private ulong sleepNanosec = 0;
        private int latencyCount = -1;
        private int numSubscribers = 1;
        private IMessaging messagingImpl;
        private bool latencyTest = false;
        private bool isReliable = true;
        private ulong pubRate = 0;
        private bool pubRateMethodSpin = true;
        private ulong executionTime = 0;
        private bool displayWriterStats;
        private System.Timers.Timer timer;
        private PerftestPrinter printer;
        private static int subID;
        private static int pubID;
        private static bool printIntervals = true;
        private static bool showCpu;
        private static bool testCompleted;
        public readonly TimeSpan timeoutWaitForAckTimeSpan = new TimeSpan(0, 0, 0, 0, 10);
        public static readonly PerftestVersion version = new PerftestVersion(0, 0, 0, 0);

        /*
         * PERFTEST-108
         * If we are performing a latency test, the default number for _NumIter
         * will be 10 times smaller than the default when performing a
         * throughput test. This will allow Perftest to work better in embedded
         * platforms since the _NumIter parameter sets the size of certain
         * arrays in the latency test mode.
         */
        public const ulong numIterDefaultLatencyTest = 10000000;

        public string[] messagingArgv = null;
        public int messagingArgc = 0;

        // Number of bytes sent in messages besides user data

        // Flag used to indicate message is used for initialization only
        public const int INITIALIZE_SIZE = 1234;
        // Flag used to indicate end of test
        public const int FINISHED_SIZE = 1235;
        // Flag used to indicate end of test
        public const int LENGTH_CHANGED_SIZE = 1236;

        /*
         * Value used to compare against to check if the latency_min has
         * been reset.
         */
        public const uint LATENCY_RESET_VALUE = uint.MaxValue;

        public const uint CDR_ENCAPSULATION_HEADER_SIZE = 4;

        public static ulong OVERHEAD_BYTES { get; set; } = 28;

        public static void Main(string[] argv)
        {
            using Perftest app = new Perftest();
            app.Run(argv);
        }

        private void Run(string[] argv)
        {
            PrintVersion();

            try
            {
                if (!ParseConfig(argv))
                {
                    return;
                }
            }catch(NullReferenceException){
                return;
            }

            ulong maxPerftestSampleSize = Math.Max(dataSize, LENGTH_CHANGED_SIZE);

            if (parameters.UnboundedSizeSet)
            {
                if (parameters.Keyed)
                {
                    messagingImpl = new RTIDDSImpl<TestDataKeyedLarge_t>(
                            new DataTypeKeyedLargeHelper(maxPerftestSampleSize));
                }
                else
                {
                    messagingImpl = new RTIDDSImpl<TestDataLarge_t>(
                            new DataTypeLargeHelper(maxPerftestSampleSize));
                }
            }
            else
            {
                if (parameters.Keyed)
                {
                    messagingImpl = new RTIDDSImpl<TestDataKeyed_t>(
                            new DataTypeKeyedHelper(maxPerftestSampleSize));
                }
                else
                {
                    messagingImpl = new RTIDDSImpl<TestData_t>(
                            new DataTypeHelper(maxPerftestSampleSize));
                }
            }

            if (!messagingImpl.Initialize(parameters))
            {
                return;
            }

            printer = new PerftestPrinter(parameters);

            PrintConfiguration();

            if (parameters.Pub)
            {
                Publisher();
            }
            else
            {
                Subscriber();
            }
        }

        public void Dispose()
        {
            messagingImpl?.Dispose();
            Console.Error.WriteLine("Test ended.");
            Console.Error.Flush();
            GC.SuppressFinalize(this);
        }

        /*********************************************************
         * ParseParameters
         */
        private static Parameters ParseParameters(string[] args)
        {
            // Create a root command with some options
            var rootCommand = new System.CommandLine.RootCommand
            {
                new System.CommandLine.Option<bool>(
                    new string[] { "--pub", "-pub" },
                    description: "Set test to be a publisher."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--sub", "-sub" },
                    getDefaultValue: () => true,
                    description: "Set test to be a subscriber (default)."),
                new System.CommandLine.Option<int>(
                    new string[] { "--sidMultiSubTest", "-sidMultiSubTest" },
                    getDefaultValue: () => 0,
                    description: "Set id of the subscriber in a multi-subscriber test."),
                new System.CommandLine.Option<int>(
                    new string[] { "--pidMultiPubTest", "-pidMultiPubTest" },
                    getDefaultValue: () => 0,
                    description: "Set id of the publisher in a multi-publisher test "
                    + "Only publisher 0 sends latency pings."),
                new System.CommandLine.Option<ulong>(
                    new string[] { "--dataLen", "-dataLen" },
                    description: "Set length of payload for each send. [default: 100]"),
                new System.CommandLine.Option<ulong>(
                    new string[] { "--numIter", "-numIter" },
                    description: "Set number of messages to send, default is "
                                    + "100000000 for Throughput tests or 10000000 "
                                    + "for Latency tests. See -executionTime."),
                new System.CommandLine.Option<uint>(
                    new string[] { "--instances", "-instances" },
                    description: "Set the number of instances (keys) to iterate "
                                    + "over when publishing. [default: 1]"),
                new System.CommandLine.Option<int>(
                    new string[] { "--writeInstance", "-writeInstance" },
                    description: "Set the instance number to be sent."
                                    + " Digit: 'specific instance-digit'"
                                    + " WriteInstance parameter cannot be bigger"
                                    + " than the number of instances."),
                new System.CommandLine.Option<uint>(
                    new string[] { "--sleep", "-sleep" },
                    getDefaultValue: () => 0,
                    description: "Time to sleep between each send."),
                new System.CommandLine.Option<uint>(
                    new string[] { "--latencyCount", "-latencyCount" },
                    getDefaultValue: () => 10000,
                    description: "Number samples (or batches) to send before a "
                                    + "latency ping packet is sent. [default: "
                                    + "10000 if -latencyTest is not specified, "
                                    + "1 if -latencyTest is specified]"),
                new System.CommandLine.Option<uint>(
                    new string[] { "--numSubcribers", "-numSubscribers" },
                    getDefaultValue: () => 1,
                    description: "Number of subscribers running in test."),
                new System.CommandLine.Option<uint>(
                    new string[] { "--numPublishers", "-numPublishers" },
                    getDefaultValue: () => 1,
                    description: "Number of publishers running in test."),
                new System.CommandLine.Option<string>(
                    new string[] { "--scan", "-scan" },
                    description: "This option is deprecated in this version of perftest."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--noPrintIntervals", "-noPrintIntervals" },
                    description: "Don't print statistics at intervals during test."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--useReadThread", "-useReadThread" },
                    description: "Use separate thread instead of callback to read data."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--latencyTest", "-latencyTest" },
                    description: "Run a latency test consisting of a ping-pong synchronous communication."),
                new System.CommandLine.Option<int>(
                    new string[] { "--verbosity", "-verbosity" },
                    description: "Run with different levels of verbosity: 0 - SILENT, 1 - ERROR, 2 - WARNING,"
                                    + " 3 - ALL. [default: 1]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--pubRate", "-pubRate" },
                    description: "Limit the throughput to the specified number"
                                    + " of samples/s, default 0 (don't limit)"
                                    + " [OPTIONAL] Method to control the throughput can be:"
                                    + " 'spin' or 'sleep'."
                                    + " [default: 'spin']"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--keyed", "-keyed" },
                    description: "Use keyed data. [default: unkeyed]"),
                new System.CommandLine.Option<ulong>(
                    new string[] { "--executionTime", "-executionTime" },
                    description: "Set a maximum duration for the test. The"
                                    + " first condition triggered will finish the"
                                    + " test: number of samples or execution time."
                                    + " [default: 0 (don't set execution time)]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--writerStats", "-writerStats" },
                    description: "Display the Pulled Sample count stats for"
                                    + " reliable protocol debugging purposes."
                                    + " [default: Not set]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--cpu", "-cpu" },
                    description: "Display the cpu percent use by the process. [default: Not set]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--cft", "-cft" },
                    description: "Use a Content Filtered Topic for the Throughput topic in the subscriber side."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--noOutputHeaders", "-noOutputHeaders" },
                    description: "Skip displaying the header row with the titles of the tables and the summary."
                                    + " [default: false (it will display titles)]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--outputFormat", "-outputFormat" },
                    getDefaultValue: () => "csv",
                    description: "Set the output format."
                                    + " The following formats are available:"
                                    + " - 'csv'"
                                    + "- 'json'"
                                    + "- 'legacy'"),
                new System.CommandLine.Option<uint>(
                    new string[] { "--sendQueueSize", "-sendQueueSize" },
                    getDefaultValue: () => 50,
                    description: "Sets number of samples (or batches) in send queue."),
                new System.CommandLine.Option<int>(
                    new string[] { "--domain", "-domain" },
                    getDefaultValue: () => 1,
                    description: "RTI DDS Domain."),
                new System.CommandLine.Option<string>(
                    new string[] { "--qosFile", "-qosFile" },
                    getDefaultValue: () => "perftest_qos_profiles.xml",
                    description: "Name of XML file for DDS Qos profile."),
                new System.CommandLine.Option<string>(
                    new string[] { "--qosLibrary", "-qosLibrary" },
                    getDefaultValue: () => "PerftestQosLibrary",
                    description: "Name of QoS Library for DDS Qos profiles."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--bestEffort", "-bestEffort" },
                    description: "Run test in best effort mode. [default: reliable]"),
                new System.CommandLine.Option<int>(
                    new string[] { "--batchSize", "-batchSize" },
                    description: "Size in bytes of batched message, default 8kB"
                                    + " (Disabled for LatencyTest mode or if dataLen > 4kB)."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--noPositiveAcks", "-noPositiveAcks" },
                    description: "Disable use of positive acks in reliable"
                                    + " protocol. [default: use positive acks]"),
                new System.CommandLine.Option<uint>(
                    new string[] { "--keepDurationUsec", "-keepDurationUsec" },
                    getDefaultValue: () => 1000,
                    description: "Minimum time (us) to keep samples when"
                                    + " positive acks are disabled."),
                new System.CommandLine.Option<uint>(
                    new string[] { "--durability", "-durability" },
                    getDefaultValue: () => 0,
                    description: "Set durability QOS, 0 - volatile,"
                                    + " 1 - transient local, 2 - transient,"
                                    + " 3 - persistent."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--dynamicData", "-dynamicData" },
                    description: "Makes use of the Dynamic Data APIs instead"
                                    + " of using the generated types."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--noDirectCommunication", "-noDirectCommunication" },
                    description: "Use brokered mode for persistent durability."),
                new System.CommandLine.Option<uint>(
                    new string[] { "--waitsetDelayUsec", "-waitsetDelayUsec" },
                    getDefaultValue: () => 100,
                    description: "UseReadThread related. Allows you to"
                                    + " process incoming data in groups, based on the"
                                    + " time rather than individually. It can be used"
                                    + " combined with -waitsetEventCount."),
                new System.CommandLine.Option<ulong>(
                    new string[] { "--waitsetEventCount", "-waitsetEventCount" },
                    getDefaultValue: () => 5,
                    description: "UseReadThread related. Allows you to"
                                    + " process incoming data in groups, based on the"
                                    + " number of samples rather than individually. It"
                                    + " can be used combined with -waitsetDelayUsec."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--enableAutoThrottle", "-enableAutoThrottle" },
                    description: "Enables the AutoThrottling feature in the"
                                    + " throughput DataWriter (pub). [default: Not set]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--enableTurboMode", "-enableTurboMode" },
                    description: "Enables the TurboMode feature in the"
                                    + " throughput DataWriter (pub). [default: Not set]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--crc", "-crc" },
                    description: "Enable CRC [default: Not set]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--crcKind", "-crcKind" },
                    getDefaultValue: () => "CRC_32_CUSTOM",
                    description: "Modify the default value to compute the CRC.\n"
                                 + "Options: CRC_32_CUSTOM | CRC_32_LEGACY\n"
                                 + "[Default: CRC_32_CUSTOM]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--enable-message-length", "-enable-message-length" },
                    description: "Enable enable_message_length_header_extension. [default: Not set]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--asynchronous", "-asynchronous" },
                    description: "Use asynchronous writer. [default: Not set]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--flowController", "-flowController" },
                    getDefaultValue: () => "default",
                    description: "In the case asynchronous writer use a specific flow controller."
                                    + " There are several flow controller predefined:"
                                    + " [default: \"default\" (If using asynchronous)]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--peer", "-peer" },
                    description: "Adds a peer to the peer host address list."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--unbounded", "-unbounded" },
                    description: "Use unbounded Sequences"),
                new System.CommandLine.Option<ulong>(
                    new string[] { "--unboundedSize", "-unboundedSize" },
                    getDefaultValue: () => 0,
                    description: "Optional. Determines the allocation threshold when -unbounded is specified"
                    + " [default 2*dataLen] up to "+ MAX_BOUNDED_SEQ_SIZE.Value +" Bytes.\n"),
                new System.CommandLine.Option<string>(
                    new string[] { "--transport", "-transport" },
                    description: " Set transport to be used. The rest of the transports will be disabled.\n"
                    + "Values:\nUDPv4\nUDPv6\nSHMEM\nTCP\nTLS\nDTLS\nWAN\n[default: UDPv4]\n"),
                new System.CommandLine.Option<int>(
                    new string[] { "--instanceHashBuckets", "-instanceHashBuckets" },
                    description: ""),
                new System.CommandLine.Option<string>(
                    new string[] { "--secureGovernanceFile", "-secureGovernanceFile" },
                    description: "Governance file when using security."),
                new System.CommandLine.Option<string>(
                    new string[] { "--securePermissionsFile", "-securePermissionsFile" },
                    description: "Permissions file <optional>."),
                new System.CommandLine.Option<string>(
                    new string[] { "--secureCertAuthority", "-secureCertAuthority" },
                    description: "Certificate authority file <optional>."),
                new System.CommandLine.Option<string>(
                    new string[] { "--secureCertFile", "-secureCertFile" },
                    description: "Certificate file <optional>."),
                new System.CommandLine.Option<string>(
                    new string[] { "--securePrivateKey", "-securePrivateKey" },
                    description: "Private key file <optional>."),
                new System.CommandLine.Option<string>(
                    new string[] { "--secureLibrary", "-secureLibrary" },
                    description: ""),
                new System.CommandLine.Option<bool>(
                    new string[] { "--lightWeightSecurity", "-lightWeightSecurity" },
                    description: "Use the Lightweight security Library."),
                new System.CommandLine.Option<string>(
                    new string[] { "--secureEncryptionAlgorithm", "-secureEncryptionAlgo" },
                    description: "Set the value for the Encryption Algorithm"),
                new System.CommandLine.Option<int>(
                    new string[] { "--secureDebug", "-secureDebug" },
                    getDefaultValue: () => -1,
                    description: ""),
                new System.CommandLine.Option<bool>(
                    new string[] { "--secureEnableAAD", "-secureEnableAAD" },
                    getDefaultValue: () => false,
                    description: "Enable AAD when using security."),
                new System.CommandLine.Option<string>(
                    new string[] { "--securePSK", "-securePSK" },
                    description: "Enables PSK with the argument's seed. [Default: Not Used]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--securePSKAlgorithm", "-securePSKAlgorithm" },
                    description: "PSK Algoritm to use. [Default: AES256+GCM]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--enableTCP", "-enableTCP" },
                    description: "Enables TCP" ),
                new System.CommandLine.Option<bool>(
                    new string[] { "--enableUDPv6", "-enableUDPv6" },
                    description: "Enables UDPv6"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--enableSharedMemory", "-enableSharedMemory" },
                    description: "Enables Shared Memory (SHMEM)"),
                new System.CommandLine.Option<string>(
                    new string[] { "--nic", "-nic" },
                    description: "Use only the NIC specified by <ipaddr> to receive packets. This will be the only "
                            + " address announced at discovery time. If not specified, use all available interfaces."),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportVerbosity", "-configureTransportVerbosity" },
                    description: "Verbosity of the transport. [default: 0 (errors only)]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--allowInterfaces", "-allowInterfaces" },
                    description: "Use only the NIC specified by <ipaddr> to receive packets. This will be the only "
                            + " address announced at discovery time. If not specified, use all available interfaces."),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportServerBindPort", "-configureTransportServerBindPort" },
                    getDefaultValue: () => "7400",
                    description: "Port used by the transport to accept TCP/TLS connections <optional>."),
                new System.CommandLine.Option<bool>(
                    new string[] { "--configureTransportWan", "-configureTransportWan" },
                    description: "Public IP address and port (WAN address and port) (separated with �:� ) "
                    + " related to the transport instantiation. This is required when using server mode."
                    + " [default: Not Set]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportPublicAddress", "-configureTransportPublicAddress" },
                    description: "Use TCP/TLS across LANs and Firewalls. [default: Not Set, LAN mode]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportCertAuthority", "-configureTransportCertAuthority" },
                    getDefaultValue: () => PerftestTransport.TransportCertAuthorityFile,
                    description: "Certificate authority file <optional>."),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportCertFile", "-configureTransportCertFile" },
                    description: "Certificate file <optional>. [default (Publisher): \""
                    + PerftestTransport.TransportCertificateFilePub + "\"].\n[default (Subscriber): \""
                    + PerftestTransport.TransportCertificateFileSub + "\"]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportPrivateKey", "-configureTransportPrivateKey" },
                    description: "Private key file <optional>. [default (Publisher): \""
                    + PerftestTransport.TransportPrivateKeyFilePub + "\"].\n[default (Subscriber): \""
                    + PerftestTransport.TransportPrivateKeyFileSub + "\"]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportWanServerAddress", "-configureTransportWanServerAddress" },
                    description: "Address where to find the WAN Server. [default: Not Set (Required)]"),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportWanServerPort", "-configureTransportWanServerPort" },
                    getDefaultValue: () => "3478",
                    description: "Port where to find the WAN Server."),
                new System.CommandLine.Option<string>(
                    new string[] { "--configureTransportWanId", "-configureTransportWanId" },
                    description: "Id to be used for the WAN transport. [default: Not Set (Required)]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--configureTransportSecureWan", "-configureTransportSecureWan" },
                    description: "Use WAN with security. default: False]"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--multicast", "-multicast" },
                    description: "Use multicast to send data. Each topic will use a different address:"
                    + " <address> is optional, if unspecified:"),
                new System.CommandLine.Option<string>(
                    new string[] { "--multicastAddr", "-multicastAddr" },
                    description: "Use multicast to send data and set the input <address>|<addr,addr,addr>"
                    + " as the multicast addresses for the three topics in the application."
                    + " If only one address is provided, that one and the 2 consecutive ones will be"
                    + " used for the 3 topics used by Perftest. If one address is set, this one must be"
                    + " in multicast range and lower than 239.255.255.253 or the equivalent on IPv6.\n"),
                new System.CommandLine.Option<bool>(
                    new string[] { "--noMulticast", "-noMulticast" },
                    description: "Disable multicast.")
            };

            Parameters result = null;
            rootCommand.Handler = System.CommandLine.Invocation.CommandHandler.Create(
                (Parameters parameters) => result = parameters);

            System.CommandLine.CommandExtensions.Invoke(rootCommand, args);

            if (result.UnboundedSize == 0) {
                result.UnboundedSizeSet = false;
            }

            return result;
        }

        /*********************************************************
        * ParseConfig
        */
        private bool ParseConfig(string[] argv)
        {
            parameters = ParseParameters(argv);

            messagingArgv = new String[argv.Length];

            /*
             * PERFTEST-108
             * We add this boolean value to check if we are explicitly changing
             * the number of iterations via command-line paramenter. This will
             * only be used if this is a latency test to decrease or not the
             * default number of iterations.
             */
            if (!parameters.Pub)
            {
                parameters.Sub = true;
                subID = parameters.SidMultiSubTest;
            }
            else
            {
                pubID = parameters.PidMultiPubTest;
            }

            if (parameters.NumIterSet)
            {
                if (parameters.NumIter == 0)
                {
                    Console.Error.Write("-numIter must be > 0\n");
                    return false;
                }
                numIter = parameters.NumIter;
            }

            if (parameters.DataLenSet)
            {
                if (parameters.DataLen < Perftest.OVERHEAD_BYTES)
                {
                    Console.Error.WriteLine("dataLen must be >= " + Perftest.OVERHEAD_BYTES);
                    return false;
                }
                else if (parameters.DataLen > Perftest.GetMaxPerftestSampleSize())
                {
                    Console.Error.WriteLine("dataLen must be <= " + Perftest.GetMaxPerftestSampleSize());
                    return false;
                }
                if (parameters.UnboundedSize == 0 && (int)parameters.DataLen > MAX_BOUNDED_SEQ_SIZE.Value) {
                    parameters.UnboundedSize = Math.Min(
                            (ulong)MAX_BOUNDED_SEQ_SIZE.Value,
                            2 * parameters.DataLen);
                }
            }
            else
            {
                parameters.DataLen = 100;
            }

            dataSize = parameters.DataLen;

            if (parameters.UnboundedSizeSet)
            {
                if (parameters.UnboundedSize < Perftest.OVERHEAD_BYTES)
                {
                    Console.Error.WriteLine(
                            "unboundedSize must be >= "
                            + Perftest.OVERHEAD_BYTES
                            + " and is "
                            + parameters.UnboundedSize);
                    return false;
                }
                if (parameters.UnboundedSize > (ulong)MAX_PERFTEST_SAMPLE_SIZE.Value)
                {
                    Console.Error.WriteLine(
                            "unboundedSize must be <= " +
                            MAX_PERFTEST_SAMPLE_SIZE.Value
                            + " and is "
                            + parameters.UnboundedSize);
                    return false;
                }
            }

            if (parameters.Unbounded && !parameters.UnboundedSizeSet)
            {
                parameters.UnboundedSize = 2 * parameters.DataLen;
            }

            sleepNanosec = parameters.Sleep * 1000000;
            latencyCount = (int)parameters.LatencyCount;
            numSubscribers = (int)parameters.NumSubscribers;
            printIntervals = !parameters.NoPrintIntervals;
            latencyTest = parameters.LatencyTest;
            isReliable = !parameters.BestEffort;
            displayWriterStats = parameters.WriterStats;

            if(parameters.InstancesSet)
            {
                if (parameters.Instances == 0)
                {
                    Console.Error.Write("instance count cannot be negative or null\n");
                    return false;
                }
            }

            try
            {
                if (!"csv".Equals(parameters.OutputFormat) && !"json".Equals(parameters.OutputFormat)
                    && !"legacy".Equals(parameters.OutputFormat))
                {
                    Console.Error.Write("<format> for outputFormat '" +
                            parameters.OutputFormat + "' is not valid. It must be" +
                            "'csv', 'json' or 'legacy'.\n");
                    return false;
                }
            }
            catch (ArgumentNullException)
            {
                Console.Error.Write("Bad <format>. It must be 'csv'" +
                        ", 'json' or 'legacy'\n");
                return false;
            }

            if (parameters.PubRateSet)
            {
                if (parameters.PubRate.Contains(":"))
                {
                    try
                    {
                        String[] st = parameters.PubRate.Split(':');
                        if (!ulong.TryParse(st[0], out pubRate))
                        {
                            Console.Error.Write("Bad number for -pubRate\n");
                            return false;
                        }
                        if ("sleep".Equals(st[1]))
                        {
                            pubRateMethodSpin = false;
                        }
                        else if (!"spin".Equals(st[1]))
                        {
                            Console.Error.Write("<method> for pubRate '" + st[1]
                                    + "' is not valid. It must be 'spin' or 'sleep'.\n");
                            return false;
                        }
                    }
                    catch (ArgumentNullException)
                    {
                        Console.Error.Write("Bad pubRate\n");
                        return false;
                    }
                }
                else
                {
                    if (!ulong.TryParse(parameters.PubRate, out pubRate))
                    {
                        Console.Error.Write("Bad number for -pubRate\n");
                        return false;
                    }
                }

                if (pubRate > 10000000)
                {
                    Console.Error.Write("-pubRate cannot be greater than 10000000.\n");
                    return false;
                }
            }

            executionTime = parameters.ExecutionTime;
            showCpu = parameters.Cpu;

            if (latencyTest)
            {
                if (pubID != 0)
                {
                    Console.Error.Write("Only the publisher with ID = 0 can run the latency test\n");
                    return false;
                }

                // With latency test, latency should be 1
                if (latencyCount == -1)
                {
                    latencyCount = 1;
                }

                /*
                 * PERFTEST-108
                 * If we are in a latency test, the default value for _NumIter
                 * has to be smaller (to avoid certain issues in platforms with
                 * low memory). Therefore, unless we explicitly changed the
                 * _NumIter value we will use a smaller default:
                 * "numIterDefaultLatencyTest"
                 */
                if (parameters.NumIterSet)
                {
                    numIter = numIterDefaultLatencyTest;
                }
            }

            if (latencyCount == -1)
            {
                latencyCount = 10000;
            }

            if ((numIter > 0) && (numIter < (ulong)latencyCount))
            {
                Console.Error.Write("numIter ({0}) must be greater than latencyCount ({1}).\n",
                              numIter, latencyCount);
                return false;
            }

            //manage the parameter: -pubRate -sleep -spin
            if (parameters.Pub && pubRate > 0)
            {
                if (spinLoopCount > 0)
                {
                    Console.Error.Write("'-spin' is not compatible with -pubRate. " +
                        "Spin/Sleep value will be set by -pubRate.");
                    spinLoopCount = 0;
                }
                if (sleepNanosec > 0)
                {
                    Console.Error.Write("'-sleep' is not compatible with -pubRate. " +
                        "Spin/Sleep value will be set by -pubRate.");
                    sleepNanosec = 0;
                }
            }
            return true;
        }

        private void PrintConfiguration()
        {
            StringBuilder sb = new StringBuilder();

            // Throughput/Latency mode
            if (parameters.Pub)
            {
                sb.Append("\nMode: ");

                if (latencyTest)
                {
                    sb.Append("LATENCY TEST (Ping-Pong test)\n");
                }
                else
                {
                    sb.Append("THROUGHPUT TEST\n");
                    sb.Append("      (Use \"-latencyTest\" for Latency Mode)\n");
                }
            }

            sb.Append("\nPerftest Configuration:\n");

            // Reliable/Best Effort
            sb.Append("\tReliability: ");
            if (isReliable)
            {
                sb.Append("Reliable\n");
            }
            else
            {
                sb.Append("Best Effort\n");
            }

            // Keyed/Unkeyed
            sb.Append("\tKeyed: ");
            if (parameters.Keyed)
            {
                sb.Append("Yes\n");
            }
            else
            {
                sb.Append("No\n");
            }

            // Publisher/Subscriber and Entity ID
            if (parameters.Pub)
            {
                sb.Append("\tPublisher ID: ");
                sb.Append(pubID);
                sb.Append('\n');
            }
            else
            {
                sb.Append("\tSubscriber ID: ");
                sb.Append(subID);
                sb.Append('\n');
            }

            if (parameters.Pub)
            {
                sb.Append("\tLatency count: 1 latency sample every ");
                sb.Append(latencyCount);
                sb.Append('\n');

                // Scan/Data Sizes
                sb.Append("\tData Size: ");
                sb.Append(dataSize);
                sb.Append('\n');

                // Batching
                int batchSize = messagingImpl.BatchSize;

                sb.Append("\tBatching: ");
                if (batchSize > 0)
                {
                    sb.Append(batchSize);
                    sb.Append(" Bytes (Use \"-batchSize 0\" to disable batching)\n");
                }
                else if (batchSize == 0)
                {
                    sb.Append("No (Use \"-batchSize\" to setup batching)\n");
                }
                else
                { // < 0 (Meaning, Disabled by RTI Perftest)
                    sb.Append("Disabled by RTI Perftest.\n");
                    if (batchSize == -1)
                    {
                        if (latencyTest)
                        {
                            sb.Append("\t\t  BatchSize disabled for a Latency Test\n");
                        }
                        else
                        {
                            sb.Append("\t\t  BatchSize is smaller than 2 times\n");
                            sb.Append("\t\t  the minimum sample size.\n");
                        }
                    }
                    if (batchSize == -2)
                    {
                        sb.Append("\t\t  BatchSize cannot be used with\n");
                        sb.Append("\t\t  Large Data.\n");
                    }
                }

                // Publication Rate
                sb.Append("\tPublication Rate: ");
                if (pubRate > 0)
                {
                    sb.Append(pubRate);
                    sb.Append(" Samples/s (");
                    if (pubRateMethodSpin)
                    {
                        sb.Append("Spin)\n");
                    }
                    else
                    {
                        sb.Append("Sleep)\n");
                    }
                }
                else
                {
                    sb.Append("Unlimited (Not set)\n");
                }

                // Execution Time or Num Iter
                if (executionTime > 0)
                {
                    sb.Append("\tExecution time: ");
                    sb.Append(executionTime);
                    sb.Append(" seconds\n");
                }
                else
                {
                    sb.Append("\tNumber of samples: ");
                    sb.Append(numIter);
                    sb.Append('\n');
                }
            }

            // Listener/WaitSets
            sb.Append("\tReceive using: ");
            if (parameters.UseReadThread)
            {
                sb.Append("WaitSets\n");
            }
            else
            {
                sb.Append("Listeners\n");
            }

            sb.Append(messagingImpl.PrintConfiguration());

            Console.Error.WriteLine(sb.ToString());
        }

        private void Subscriber()
        {
            ThroughputListener readerListener = null;
            IMessagingReader reader;
            IMessagingWriter writer;
            IMessagingWriter announcementWriter;

            // create latency pong writer
            writer = messagingImpl.CreateWriter(LATENCY_TOPIC_NAME.Value);

            if (writer == null)
            {
                Console.Error.Write("Problem creating latency writer.\n");
                return;
            }

            // Check if using callbacks or read thread
            if (!parameters.UseReadThread)
            {
                // create latency pong reader
                readerListener = new ThroughputListener(writer, printer, parameters);
                reader = messagingImpl.CreateReader(THROUGHPUT_TOPIC_NAME.Value, readerListener);
                if (reader == null)
                {
                    Console.Error.Write("Problem creating throughput reader.\n");
                    return;
                }
            }
            else
            {
                reader = messagingImpl.CreateReader(THROUGHPUT_TOPIC_NAME.Value, null);
                if (reader == null)
                {
                    Console.Error.Write("Problem creating throughput reader.\n");
                    return;
                }
                readerListener = new ThroughputListener(writer, reader, printer, parameters);
                Task.Run(() => readerListener.ReadThread());
            }

            // Create announcement writer
            announcementWriter =
                    messagingImpl.CreateWriter(ANNOUNCEMENT_TOPIC_NAME.Value);

            if (announcementWriter == null)
            {
                Console.Error.Write("Problem creating announcement writer.\n");
                return;
            }

            // Synchronize with publishers
            Console.Error.Write("Waiting to discover {0} publishers ...\n", parameters.NumPublishers);
            reader.WaitForWriters((int)parameters.NumPublishers);
            // In a multi publisher test, only the first publisher will have a reader.
            writer.WaitForReaders(1);
            announcementWriter.WaitForReaders((int)parameters.NumPublishers);

            // Send announcement message
            TestMessage message = new TestMessage();
            message.entityId = subID;
            message.Size = 1;
            announcementWriter.Send(message, false);
            announcementWriter.Flush();

            Console.Error.Write("Waiting for data ...\n");

            printer.PrintInitialOutput();

            // wait for data
            ulong now, prevTime, delta;
            ulong prevCount = 0;
            ulong prevBytes = 0;
            ulong aveCount = 0;
            int lastDataLength = -1;
            ulong mps, bps;
            double mpsAve = 0.0, bpsAve = 0.0;
            ulong msgSent, bytes, lastMsgs, lastBytes;
            double missingPacketsPercent = 0;

            now = GetTimeUsec();
            while (true)
            {
                prevTime = now;
                Thread.Sleep(1000);
                now = GetTimeUsec();

                if (readerListener.changeSize)
                { // ACK change_size
                    TestMessage messageChangeSize = new TestMessage();
                    messageChangeSize.entityId = subID;
                    // messageChangeSize.data = new List<byte>(new byte[1]);
                    // messageChangeSize.size = 1;
                    messageChangeSize.Size = 1;
                    announcementWriter.Send(messageChangeSize, false);
                    announcementWriter.Flush();
                    readerListener.changeSize = false;
                }

                if (readerListener.endTest)
                {
                    TestMessage messageEndTest = new TestMessage();
                    messageEndTest.entityId = subID;
                    // messageEndTest.data = new List<byte>(new byte[1]);
                    // messageEndTest.size = 1;
                    messageEndTest.Size = 1;
                    announcementWriter.Send(messageEndTest, false);
                    announcementWriter.Flush();
                    break;
                }

                double outputCpu = 0.0;
                if (readerListener.packetsReceived > 0 && showCpu)
                {
                    outputCpu = readerListener.cpu.GetCpuInstant();
                }

                if (printIntervals)
                {
                    if (lastDataLength != readerListener.lastDataLength)
                    {
                        lastDataLength = readerListener.lastDataLength;
                        prevCount = readerListener.packetsReceived;
                        prevBytes = readerListener.bytesReceived;
                        bpsAve = 0;
                        mpsAve = 0;
                        aveCount = 0;
                        continue;
                    }

                    lastMsgs = readerListener.packetsReceived;
                    lastBytes = readerListener.bytesReceived;
                    msgSent = lastMsgs - prevCount;
                    bytes = lastBytes - prevBytes;
                    prevCount = lastMsgs;
                    prevBytes = lastBytes;
                    delta = now - prevTime;
                    mps = msgSent * 1000000 / delta;
                    bps = bytes * 1000000 / delta;

                    // calculations of overall average of mps and bps
                    ++aveCount;
                    bpsAve += (double)(bps - bpsAve) / (double)aveCount;
                    mpsAve += (double)(mps - mpsAve) / (double)aveCount;

                    // Calculations of missing package percent
                    if (lastMsgs + readerListener.missingPackets == 0)
                    {
                        missingPacketsPercent = 0.0;
                    }
                    else
                    {
                        missingPacketsPercent =
                                readerListener.missingPackets
                                / (float)(lastMsgs
                                    + readerListener.missingPackets);
                    }

                    if (lastMsgs > 0)
                    {
                        printer.PrintThroughputInterval(
                            lastMsgs,
                            mps,
                            mpsAve,
                            bps,
                            bpsAve,
                            readerListener.missingPackets,
                            missingPacketsPercent,
                            outputCpu);
                    }
                }
            }

            printer.PrintFinalOutput();
            Thread.Sleep(1000);
            Console.Error.Write("Finishing test...\n");
            Console.Out.Flush();
        }

        private void Publisher()
        {
            // create throughput/ping writer
            IMessagingWriter throughputWriter = messagingImpl.CreateWriter(THROUGHPUT_TOPIC_NAME.Value);
            if (throughputWriter == null)
            {
                Console.Error.Write("Problem creating throughput writer.\n");
                return;
            }

            int samplesPerBatch = GetSamplesPerBatch();
            uint numLatency = (uint)(numIter / (ulong)samplesPerBatch / (ulong)latencyCount);

            if (numLatency / (ulong)samplesPerBatch % (ulong)latencyCount > 0)
            {
                numLatency++;
            }

            // in batch mode, we might have to send another ping
            if (samplesPerBatch > 1)
            {
                ++numLatency;
            }

            LatencyListener readerListener = null;
            IMessagingReader reader;

            // Only publisher with ID 0 will send/receive pings
            if (pubID == 0)
            {
                // Check if using callbacks or read thread
                if (!parameters.UseReadThread)
                {
                    // create latency pong reader
                    readerListener = new LatencyListener(
                                latencyTest ? throughputWriter : null,
                                parameters,
                                printer,
                                numLatency);

                    reader = messagingImpl.CreateReader(
                            LATENCY_TOPIC_NAME.Value,
                            readerListener);

                    if (reader == null)
                    {
                        Console.Error.Write("Problem creating latency reader.\n");
                        return;
                    }
                }
                else
                {
                    reader = messagingImpl.CreateReader(LATENCY_TOPIC_NAME.Value, null);
                    if (reader == null)
                    {
                        Console.Error.Write("Problem creating latency reader.\n");
                        return;
                    }
                    readerListener = new LatencyListener(
                                reader,
                                latencyTest ? throughputWriter : null,
                                parameters,
                                printer,
                                numLatency);
                    Task.Run(() => readerListener.ReadThread());
                }
            }
            else
            {
                reader = null;
            }
            /* Create Announcement reader
             * A Subscriber will send a message on this channel once it discovers
             * every Publisher
             */
            AnnouncementListener announcementReaderListener = new AnnouncementListener();
            IMessagingReader announcementReader = messagingImpl.CreateReader(
                    ANNOUNCEMENT_TOPIC_NAME.Value,
                    announcementReaderListener);
            if (announcementReader == null)
            {
                Console.Error.Write("Problem creating announcement reader.\n");
                return;
            }
            ulong spinsPerUsec = 0;
            const ulong sleepUsec = 1000;
            if (pubRate > 0)
            {
                if (pubRateMethodSpin)
                {
                    spinsPerUsec = GetSpinsPerMicrosecond();
                    /* A return value of 0 means accuracy not assured */
                    if (spinsPerUsec == 0)
                    {
                        Console.Error.Write(
                            "Error initializing spin per microsecond. "
                            + "-pubRate cannot be used\n"
                            + "Exiting.\n");
                        return;
                    }
                    spinLoopCount = 1000000 * spinsPerUsec / pubRate;
                }
                else
                {
                    sleepNanosec = 1000000000 / pubRate;
                }
            }

            Console.Error.WriteLine($"Waiting to discover {numSubscribers} subscribers ...");
            throughputWriter.WaitForReaders(numSubscribers);
            // Only publisher with ID 0 will have a reader.
            reader?.WaitForWriters(numSubscribers);
            announcementReader.WaitForWriters(numSubscribers);

            // We have to wait until every Subscriber sends an announcement message
            // indicating that it has discovered every Publisher
            Console.Error.Write("Waiting for subscribers announcement ...\n");
            while (numSubscribers > announcementReaderListener.announcedSubscriberReplies)
            {
                Thread.Sleep(1000);
            }

            // Allocate data and set size
            TestMessage message = new TestMessage();
            message.entityId = pubID;
            message.Size = INITIALIZE_SIZE;

            /*
             * Initial burst of data:
             *
             * The purpose of this initial burst of Data is to ensure that most
             * memory allocations in the critical path are done before the test
             * begins, for both the Writer and the Reader that receives the samples.
             * It will also serve to make sure that all the instances are registered
             * in advance in the subscriber application.
             *
             * We query the MessagingImplementation class to get the suggested sample
             * count that we should send. This number might be based on the reliability
             * protocol implemented by the middleware behind. Then we choose between that
             * number and the number of instances to be sent.
             */

            int initializeSampleCount = Math.Max(
                   messagingImpl.InitialBurstSampleCount,
                   (int)parameters.Instances);

            Console.Error.WriteLine(
                    "Sending " + initializeSampleCount + " initialization pings ...");

            for (int i = 0; i < initializeSampleCount; i++)
            {
                // Send test initialization message
                throughputWriter.Send(message, true);
            }
            throughputWriter.Flush();

            Console.Error.WriteLine("Publishing data ...");

            printer.PrintInitialOutput();

            // Set data size, account for other bytes in message
            message.Size = (int)(dataSize - OVERHEAD_BYTES);

            // Sleep 1 second, then begin test
            Thread.Sleep(1000);

            int numPings = 0;
            int pingID = -1;
            int currentIndexInBatch = 0;
            int pingIndexInBatch = 0;
            bool sentPing = false;

            ulong timeNow = 0, timeLastCheck = 0, timeDelta = 0;
            ulong pubRateSamplePeriod = 1;
            ulong rate = 0;

            timeLastCheck = GetTimeUsec();

            /* Minimum value for pubRate_sample_period will be 1 so we execute 100 times
               the control loop every second, or every sample if we want to send less
               than 100 samples per second */
            if (pubRate > 100)
            {
                pubRateSamplePeriod = pubRate / 100;
            }

            if (executionTime > 0)
            {
                SetTimeout(executionTime);
            }

            // Main sending loop
            for (ulong loop = 0; loop < numIter && !testCompleted; ++loop)
            {
                if ((pubRate > 0)
                        && (loop > 0)
                        && (loop % pubRateSamplePeriod == 0))
                {
                    timeNow = GetTimeUsec();

                    timeDelta = timeNow - timeLastCheck;
                    timeLastCheck = timeNow;
                    // rate is the amount of loops that have to be executed in the next second to achieve pubRate
                    rate = pubRateSamplePeriod * 1000000 / timeDelta;

                    if (pubRateMethodSpin)
                    {
                        if (rate > pubRate)
                        {
                            spinLoopCount += spinsPerUsec;
                        }
                        else if (rate < pubRate && spinLoopCount > spinsPerUsec)
                        {
                            spinLoopCount -= spinsPerUsec;
                        }
                        else if (rate < pubRate && spinLoopCount <= spinsPerUsec)
                        {
                            spinLoopCount = 0;
                        }
                    }
                    else
                    {
                        if (rate > pubRate)
                        {
                            sleepNanosec += sleepUsec; //plus 1 MicroSec
                        }
                        else if (rate < pubRate && sleepNanosec > sleepUsec)
                        {
                            sleepNanosec -= sleepUsec; //less 1 MicroSec
                        }
                        else if (rate < pubRate && sleepNanosec <= sleepUsec)
                        {
                            sleepNanosec = 0;
                        }
                    }
                }

                if (spinLoopCount > 0)
                {
                    Spin();
                }

                if (sleepNanosec > 0)
                {
                    Thread.Sleep((int)sleepNanosec / 1000000);
                }

                pingID = -1;

                // only send latency pings if is publisher with ID 0
                // In batch mode, latency pings are sent once every LatencyCount batches
                if ((pubID == 0) && ((loop / (ulong)samplesPerBatch % (ulong)latencyCount) == 0))
                {
                    /* In batch mode only send a single ping in a batch.
                     *
                     * However, the ping is sent in a round robin position within
                     * the batch.  So keep track of which position(index) the
                     * current sample is within the batch, and which position
                     * within the batch should contain the ping. Only send the ping
                     * when both are equal.
                     *
                     * Note when not in batch mode, current_index_in_batch = ping_index_in_batch
                     * always.  And the if() is always true.
                     */
                    if (currentIndexInBatch == pingIndexInBatch && !sentPing)
                    {
                        // Each time ask a different subscriber to echo back
                        pingID = numPings % numSubscribers;
                        ulong now = GetTimeUsec();
                        message.timestampSec = (int)((now >> 32) & 0xFFFFFFFF);
                        message.timestampUsec = (uint)(now & 0xFFFFFFFF);

                        ++numPings;
                        pingIndexInBatch =
                                (pingIndexInBatch + 1) % samplesPerBatch;
                        sentPing = true;

                        if (displayWriterStats && printIntervals)
                        {
                            Console.WriteLine(
                                "Pulled samples: {0,7}",
                                throughputWriter.GetPulledSampleCount());
                        }
                    }
                }

                currentIndexInBatch = (currentIndexInBatch + 1) % samplesPerBatch;

                message.seqNum = (uint)loop;
                message.latencyPing = pingID;
                throughputWriter.Send(message, false);
                if (latencyTest && sentPing)
                {
                    if (isReliable)
                    {
                        throughputWriter.WaitForPingResponse();
                    }
                    else
                    {
                        /* time out in milliseconds */
                        throughputWriter.WaitForPingResponse(TimeSpan.FromMilliseconds(200));
                    }
                }

                // come to the beginning of another batch
                if (currentIndexInBatch == 0)
                {
                    sentPing = false;
                }
            }

            // In case of batching, flush
            throughputWriter.Flush();

            // Test has finished, send end of test message, send multiple
            // times in case of best effort
            // message.size = FINISHED_SIZE;
            message.Size = FINISHED_SIZE;
            int j = 0;
            const int announcementSampleCount = 50;
            announcementReaderListener.announcedSubscriberReplies =
                    numSubscribers;
            while (announcementReaderListener.announcedSubscriberReplies > 0
                    && j < announcementSampleCount)
            {
                throughputWriter.Send(message, true);
                throughputWriter.Flush();
                try
                {
                    throughputWriter.WaitForAck(timeoutWaitForAckTimeSpan);
                }catch (System.TimeoutException){}
                j++;
            }
            if (pubID == 0)
            {
                readerListener.PrintSummaryLatency(true);
                readerListener.EndTest = true;
            }
            else
            {
                Console.Error.WriteLine("Latency results are only shown when -pidMultiPubTest = 0");
            }

            if (displayWriterStats)
            {
                Console.Error.WriteLine("Pulled samples: {0,7}", throughputWriter.GetPulledSampleCount());
            }

            printer.PrintFinalOutput();
            Console.Error.WriteLine("Finishing test...");
            Console.Out.Flush();
        }

        public static ulong GetTimeUsec()
        {
            return (ulong)(1_000_000 * System.Diagnostics.Stopwatch.GetTimestamp()
                            / System.Diagnostics.Stopwatch.Frequency);
        }

        private static void Timeout(object source, ElapsedEventArgs e)
        {
            testCompleted = true;
        }

        private void SetTimeout(ulong executionTime)
        {
            if (timer == null)
            {
                timer = new System.Timers.Timer();
                timer.Elapsed += Timeout;
                Console.Error.WriteLine($"Setting timeout to {executionTime} seconds.");
                timer.Interval = executionTime * 1000;
                timer.Enabled = true;
            }
        }

        public int GetSamplesPerBatch()
        {
            int batchSize = messagingImpl.BatchSize;
            int samplesPerBatch;

            if (batchSize > 0)
            {
                samplesPerBatch = batchSize / (int)dataSize;
                if (samplesPerBatch == 0)
                {
                    samplesPerBatch = 1;
                }
            }
            else
            {
                samplesPerBatch = 1;
            }

            return samplesPerBatch;
        }

        public static void PrintVersion()
        {
            PerftestVersion perftestV = version;
            Rti.Config.ProductVersion ddsV = Rti.Dds.Core.ServiceEnvironment.Instance.Version;

            if (perftestV.version.Major == 0
                    && perftestV.version.Minor == 0
                    && perftestV.version.Build == 0)
            {
                Console.Error.Write("RTI Perftest Develop");
            }
            else if (perftestV.version.Major == 9
                    && perftestV.version.Minor == 9
                    && perftestV.version.Build == 9)
            {
                Console.Error.Write("RTI Perftest Master");
            }
            else
            {
                Console.Error.Write(
                        "RTI Perftest "
                        + perftestV.version.Major + "."
                        + perftestV.version.Minor + "."
                        + perftestV.version.Build);
                if (perftestV.version.Revision != 0)
                {
                    Console.Error.Write("." + perftestV.version.Revision);
                }
            }
            Console.Error.WriteLine("\n" + ddsV);
        }

        static public ulong GetMaxPerftestSampleSize()
        {
            return (ulong)MAX_PERFTEST_SAMPLE_SIZE.Value;
        }

        [MethodImpl(MethodImplOptions.NoOptimization)]
        private static ulong GetSpinsPerMicrosecond()
        {
            ulong spins = 0;
            ulong startTime = GetTimeUsec();
            ulong diff;
            // Start counting how many spins can be made within 1 usec
            do
            {
                double a, b, c;
                a = 1.1;
                b = 3.1415;
                c = a / b * spins;
                spins++;
                diff = GetTimeUsec() - startTime;
            }
            while (diff < 100);

            return spins / 100;
        }

        [MethodImpl(MethodImplOptions.NoOptimization)]
        private void Spin()
        {
            for (ulong i = 0; i < spinLoopCount; i++)
            {
                double a, b, c;
                a = 1.1;
                b = 3.1415;
                c = a / b * i;
            }
        }
    }
} // PerformanceTest Namespace
