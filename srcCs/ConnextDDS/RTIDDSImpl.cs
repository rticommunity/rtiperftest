/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Omg.Dds.Core;
using Rti.Dds.Core;
using Rti.Dds.Core.Policy;
using Omg.Dds.Core.Policy;
using Rti.Dds.Core.Status;
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Subscription;
using Rti.Dds.Topics;

namespace PerformanceTest
{
    public class RTIDDSImpl<T> : IMessaging where T : class, IEquatable<T>
    {
        private const int RTIPERFTEST_MAX_PEERS = 1024;
        private Parameters parameters;
        private int instanceMaxCountReader = -1;
        private readonly bool directCommunication = true;
        private bool isLargeData;
        private ulong maxUnfragmentedRTPSPayloadSize = PerftestTransport.MessageSizeMaxNotSet;
        private readonly string[] validFlowController = { "default", "1Gbps", "10Gbps" };
        private int peerHostCount = 0;
        private readonly string[] peerHostArray = new string[RTIPERFTEST_MAX_PEERS];
        private readonly int[] cftRange = { 0, 0 };
        private readonly PerftestTransport transport = new PerftestTransport();

        /* Security related variables */
        private bool SecureUseSecure;

        private const string SecurePrivatePubKeyFile = "./resource/secure/pubkey.pem";
        private const string SecurePrivateKeyFileSub = "./resource/secure/subkey.pem";
        private const string SecureCertificateFilePub = "./resource/secure/pub.pem";
        private const string SecureCertificateFileSub = "./resource/secure/sub.pem";
        private const string SecureAuthorityFile = "./resource/secure/cacert.pem";
        private const string SecurePermissionFilePub = "./resource/secure/signed_PerftestPermissionsPub.xml";
        private const string SecurePermissionFileSub = "./resource/secure/signed_PerftestPermissionsSub.xml";
        private const string SecureLibraryName = "nddssecurity";
        private const string LWSecureLibraryName = "nddslightweightsecurity";

        public DomainParticipant participant;
        private Subscriber subscriber;
        private Publisher publisher;
        private readonly ITypeHelper<T> dataTypeHelper;
        private Semaphore pongSemaphore;
        private readonly SortedDictionary<string, string> qosProfileNameMap = new SortedDictionary<string, string>();
        public int BatchSize { get; set; }
        public int InitialBurstSampleCount { get; set; }

        public RTIDDSImpl(ITypeHelper<T> myDataTypeHelper)
        {
            dataTypeHelper = myDataTypeHelper;

            qosProfileNameMap.Add(LATENCY_TOPIC_NAME.Value, "LatencyQos");
            qosProfileNameMap.Add(ANNOUNCEMENT_TOPIC_NAME.Value, "AnnouncementQos");
            qosProfileNameMap.Add(THROUGHPUT_TOPIC_NAME.Value, "ThroughputQos");

            Perftest.OVERHEAD_BYTES = (ulong)myDataTypeHelper.GetSerializedOverheadSize();
        }

        public void Dispose()
        {
            if (participant != null)
            {
                lock (participant)
                {
                    participant.Dispose();
                }
            }
            GC.SuppressFinalize(this);
        }

        public void Shutdown()
        {
            if (participant != null)
            {
                lock (participant)
                {
                    participant.Dispose();
                }
            }
        }

        public bool Initialize(Parameters parameters)
        {
            this.parameters = parameters;
            if (!ParseConfig())
            {
                return false;
            }

            string baseProfile = parameters.QosLibrary + "::BaseProfileQos";
            string pingProfile = parameters.QosLibrary + "::ThroughputQos";
            string pongProfile = parameters.QosLibrary + "::ThroughputQos";

            var qosProvider = new QosProvider(parameters.QosFile);
            var participantQos = GetParticipantQos(baseProfile);

            if (parameters.LatencyTest)
            {
                pongSemaphore = new Semaphore(0, 1);
            }

            if (SecureUseSecure) {
                // validate arguments
                if (!ValidateSecureArgs()) {
                    Console.Error.WriteLine("Failure validating arguments");
                    return false;
                }
                ConfigureSecurePlugin(ref participantQos);
            }

            participant = DomainParticipantFactory.Instance.CreateParticipant(parameters.Domain, participantQos);
            if (participant == null) {
                Console.Error.Write("Problem creating participant.\n");
                return false;
            }

            publisher = participant.CreatePublisher(qosProvider.GetPublisherQos(pingProfile));
            if (publisher == null)
            {
                Console.Error.WriteLine("Problem creating publisher.");
                return false;
            }
            subscriber = participant.CreateSubscriber(qosProvider.GetSubscriberQos(pongProfile));
            if (subscriber == null)
            {
                Console.Error.WriteLine("Problem creating subscriber.");
                return false;
            }
            return true;
        }

        public int ConfigureInitialBurstSampleCount()
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
            initializeSampleCount = Math.Max(initializeSampleCount, (int)parameters.SendQueueSize);

            /*
             * If we are using batching we need to take into account tha the Send
             * Queue will be per-batch, therefore for the number of samples:
             */
            if (BatchSize > 0)
            {
                initializeSampleCount = Math.Max(
                        (int)parameters.SendQueueSize * ((int)BatchSize / (int)parameters.DataLen),
                        initializeSampleCount);
            }

            return initializeSampleCount;
        }

        public bool DataSizeRelatedCalculations()
        {
            isLargeData = parameters.DataLen > maxUnfragmentedRTPSPayloadSize;

            // Manage parameter -batchSize
            if (parameters.BatchSize > 0)
            {
                /*
                 * Large Data + batching cannot be set. But batching is enabled by default,
                 * so in that case, we just disabled batching, else, the customer set it up,
                 * so we explicitly fail
                 */
                if (isLargeData)
                {
                    if (parameters.BatchSizeSet)
                    {
                        Console.Error.WriteLine("Batching cannot be used with Large Data.");
                        return false;
                    }
                    else
                    {
                        parameters.batchSize = -2;
                    }
                }
                else if ((ulong)parameters.BatchSize < parameters.DataLen * 2)
                {
                    /*
                     * We don't want to use batching if the batch size is not large
                     * enough to contain at least two samples (in this case we avoid the
                     * checking at the middleware level).
                     */
                    if (parameters.BatchSizeSet)
                    {
                        /*
                         * Batchsize disabled. A message will be print if _batchSize < 0 in
                         * perftest_cpp::PrintConfiguration()
                         */
                        parameters.batchSize = -1;
                    }
                    else
                    {
                        parameters.batchSize = 0;
                    }
                }
            }
            if (parameters.PubRateSet && parameters.BatchSizeSet)
            {
                parameters.batchSize = -3;
            }

            if (parameters.EnableTurboMode)
            {
                if (parameters.Asynchronous)
                {
                    Console.Error.WriteLine("Turbo Mode cannot be used with asynchronous writing.");
                    return false;
                }
                if (isLargeData)
                {
                    Console.Error.WriteLine("Turbo Mode disabled, using large data.");
                    parameters.EnableTurboMode = false;
                }
            }

            BatchSize = parameters.BatchSize;
            InitialBurstSampleCount = ConfigureInitialBurstSampleCount();

            return true;
        }

        public string PrintConfiguration()
        {
            StringBuilder sb = new StringBuilder();

            // Domain ID
            sb.Append("\tDomain: ");
            sb.Append(parameters.Domain);
            sb.Append('\n');

            // Dynamic Data
            if (parameters.Pub)
            {
                sb.Append("\tAsynchronous Publishing: ");
                if (isLargeData || parameters.Asynchronous)
                {
                    sb.Append("Yes\n");
                    sb.Append("\tFlow Controller: ");
                    sb.Append(parameters.FlowController);
                    sb.Append('\n');
                }
                else
                {
                    sb.Append("No\n");
                }
            }

            // Turbo Mode / AutoThrottle
            if (parameters.EnableTurboMode)
            {
                sb.Append("\tTurbo Mode: Enabled\n");
            }
            if (parameters.EnableAutoThrottle)
            {
                sb.Append("\tAutoThrottle: Enabled\n");
            }

            sb.Append("\tCRC: ");
            sb.Append(parameters.Crc);
            if (parameters.Crc)
            {
                sb.Append(" (");
                sb.Append(parameters.CrcKind);
                sb.Append(")");
            }
            sb.Append('\n');

            sb.Append("\ttMessage Length Header Extension Enabled: ");
            sb.Append(parameters.MessageLength);
            sb.Append('\n');

            // XML File
            sb.Append("\tXML File: ");
            sb.Append(parameters.QosFile);
            sb.Append('\n');
            sb.Append('\n');
            sb.Append(transport.PrintTransportConfigurationSummary());

            // set initial peers and not use multicast
            if (peerHostCount > 0)
            {
                sb.Append("Initial peers: ");
                for (int i = 0; i < peerHostCount; ++i)
                {
                    sb.Append(peerHostArray[i]);
                    if (i == peerHostCount - 1)
                    {
                        sb.Append('\n');
                    }
                    else
                    {
                        sb.Append(", ");
                    }
                }
            }

            if (SecureUseSecure)
            {
                sb.Append('\n');
                sb.Append(PrintSecureArgs());
            }

            // Large Data
            if (parameters.DataLen > maxUnfragmentedRTPSPayloadSize)
            {
                sb.Append("\n[IMPORTANT]: -datalen (");
                sb.Append(parameters.DataLen);
                sb.Append(") is greater than\n");
                sb.Append("             the minimum message_size_max across all\n");
                sb.Append("             enabled transports (");
                sb.Append(maxUnfragmentedRTPSPayloadSize);
                sb.Append("). Samples will be fragmented.\n");
            }

            // We want to expose if we are using or not the unbounded type
            if (parameters.UnboundedSize != 0)
            {
                sb.Append("\n[IMPORTANT]: Using the Unbounded Sequence Type.");
                if (parameters.DataLen > (ulong)MAX_BOUNDED_SEQ_SIZE.Value)
                {
                    sb.Append(" -datalen (");
                    sb.Append(parameters.DataLen);
                    sb.Append(") is \n             larger than MAX_BOUNDED_SEQ_SIZE (");
                    sb.Append(MAX_BOUNDED_SEQ_SIZE.Value);
                    sb.Append(")");
                }
                sb.Append("\n");
            }

            return sb.ToString();
        }

        public bool ParseConfig()
        {

            if (parameters.Durability > 3)
            {
                Console.Error.Write("durability kind needs to be 0(volatile),1(transient local),2(transient),3(persistent) \n");
                return false;
            }

            if(parameters.InstancesSet)
            {
                instanceMaxCountReader = (int)parameters.Instances;
            }
            else
            {
                parameters.Instances = 1;
            }

            if(parameters.InstanceHashBucketsSet)
            {
                if (parameters.InstanceHashBuckets <= 0 && parameters.InstanceHashBuckets != -1)
                {
                    Console.Error.Write(" instanceHashBuckets count cannot be negative or null\n");
                    return false;
                }
            }

            // This variable causes a need to determine if an argument with default value has been
            // set by user
            // IsBatchSizeProvided = true;

            if (!parameters.BatchSizeSet)
            {
                parameters.batchSize = (int)DEFAULT_THROUGHPUT_BATCH_SIZE.Value;

                if(parameters.BatchSize < 0)
                {
                    Console.Error.Write("Batch size '" + parameters.BatchSize +
                                "' should be greater than, or equal to, 0\n");
                    return false;
                }
            }

            // Verify if the flow controller name is correct, else use "default"
            bool validFlowControl = false;
            foreach (string flow in validFlowController)
            {
                if (parameters.FlowController.Equals(flow))
                {
                    validFlowControl = true;
                }
            }

            if (!validFlowControl)
            {
                Console.Error.Write("Bad <flow> '" + parameters.FlowController + "' for custom flow controller\n");
                parameters.FlowController = "default";
            }

            if (parameters.PeerSet)
            {
                if (peerHostCount + 1 < RTIPERFTEST_MAX_PEERS)
                {
                    peerHostArray[peerHostCount++] = parameters.Peer;
                }
                else
                {
                    Console.Error.Write("The maximum of -initial peers is " + RTIPERFTEST_MAX_PEERS + "\n");
                    return false;
                }
            }

            if (parameters.CftSet)
            {
                if (parameters.Cft.Contains(":"))
                {
                    try
                    {
                        String[] st = parameters.Cft.Split(':');
                        if (!int.TryParse(st[0], out cftRange[0]))
                        {
                            Console.Error.Write("Bad <start> for -cft\n");
                            return false;
                        }
                        if (!int.TryParse(st[1], out cftRange[1]))
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
                }
                else
                {
                    if (!int.TryParse(parameters.Cft, out cftRange[0]))
                    {
                        Console.Error.Write("Bad <start> for  -cft\n");
                        return false;
                    }
                    cftRange[1] = cftRange[0];
                }

                if (cftRange[0] > cftRange[1])
                {
                    Console.Error.Write("-cft <start> value cannot be bigger than <end>\n");
                    return false;
                }
                if (cftRange[0] < 0 ||
                        cftRange[0] >= MAX_CFT_VALUE.Value ||
                        cftRange[1] < 0 ||
                        cftRange[1] >= MAX_CFT_VALUE.Value)
                {
                    Console.Error.Write("-cft <start>:<end> values should be between [0," + MAX_CFT_VALUE.Value + "]\n");
                    return false;
                }
            }

            if (parameters.VerbositySet)
            {
                switch (parameters.Verbosity)
                {
                    case 0:
                        Rti.Config.Logger.Instance.SetVerbosity(Rti.Config.Verbosity.Silent);
                        Console.Error.Write("Setting verbosity to SILENT\n");
                        break;
                    case 1:
                        Rti.Config.Logger.Instance.SetVerbosity(Rti.Config.Verbosity.Error);
                        Console.Error.Write("Setting verbosity to ERROR\n");
                        break;
                    case 2:
                        Rti.Config.Logger.Instance.SetVerbosity(Rti.Config.Verbosity.Warning);
                        Console.Error.Write("Setting verbosity to WARNING\n");
                        break;
                    case 3:
                        Rti.Config.Logger.Instance.SetVerbosity(Rti.Config.Verbosity.StatusAll);
                        Console.Error.Write("Setting verbosity to STATUS_ALL\n");
                        break;
                    default:
                        Console.Error.Write(
                                "Invalid value for the verbosity parameter." +
                                " Setting verbosity to ERROR (1)\n");
                        Rti.Config.Logger.Instance.SetVerbosity(Rti.Config.Verbosity.Error);
                        break;
                }
            }

            if (!string.IsNullOrEmpty(parameters.SecureGovernanceFile)
                || !string.IsNullOrEmpty(parameters.SecurePermissionsFile)
                || !string.IsNullOrEmpty(parameters.SecureCertAuthority)
                || !string.IsNullOrEmpty(parameters.SecureCertFile)
                || !string.IsNullOrEmpty(parameters.SecurePrivateKey)
                || !string.IsNullOrEmpty(parameters.SecureLibrary))
            {
                SecureUseSecure = true;
            }

            /* Check if we need to enable Large Data. */
            if (parameters.DataLen > MAX_BOUNDED_SEQ_SIZE.Value)
            {
                isLargeData = true;
                if (parameters.UnboundedSize == 0)
                {
                    parameters.UnboundedSize = MAX_BOUNDED_SEQ_SIZE.Value;
                }
            }
            else
            { /* No Large Data */
                isLargeData = false;
            }

            /* If we are using batching */
            if (parameters.BatchSize > 0)
            {
                /* We will not use batching for a latency test */
                if (parameters.LatencyTest)
                {
                    if (parameters.BatchSizeSet)
                    {
                        Console.Error.WriteLine(
                                "Batching cannot be used in a Latency test.");
                        return false;
                    }
                    else
                    {
                        parameters.batchSize = 0; //Disable Batching
                    }
                }
            }

            // Manage parameters.WriteInstance
            if (parameters.WriteInstance != -1)
            {
                if (parameters.Instances < parameters.WriteInstance)
                {
                    Console.Error.WriteLine("Specified '-WriteInstance' (" + parameters.WriteInstance +
                            ") invalid: Bigger than the number of instances (" + parameters.WriteInstance + ").");
                    return false;
                }
            }
            if (parameters.Pub && parameters.CftSet)
            {
                Console.Error.WriteLine("Content Filtered Topic is not a parameter in the publisher side.\n");
            }

            if (transport != null)
            {
                if (!transport.ParseTransportOptions(parameters))
                {
                    Console.Error.WriteLine("Failure parsing the transport options.");
                    return false;
                }
            }
            else
            {
                Console.Error.WriteLine("transport is not initialized");
                return false;
            }

            return true;
        }

        private bool ValidateSecureArgs()
        {
            if (!parameters.LightWeightSecurity)
            {
                if (parameters.SecurePrivateKey == null)
                {
                    if (parameters.Pub)
                    {
                        parameters.SecurePrivateKey = SecurePrivatePubKeyFile;
                    }
                    else
                    {
                        parameters.SecurePrivateKey = SecurePrivateKeyFileSub;
                    }
                }

                if (parameters.SecureCertFile == null)
                {
                    if (parameters.Pub)
                    {
                        parameters.SecureCertFile = SecureCertificateFilePub;
                    }
                    else
                    {
                        parameters.SecureCertFile = SecureCertificateFileSub;
                    }
                }

                if (parameters.SecureCertAuthority == null)
                {
                    parameters.SecureCertAuthority = SecureAuthorityFile;
                }

                if (parameters.SecurePermissionsFile == null)
                {
                    if (parameters.Pub)
                    {
                        parameters.SecurePermissionsFile = SecurePermissionFilePub;
                    }
                    else
                    {
                        parameters.SecurePermissionsFile = SecurePermissionFileSub;
                    }
                }
            }

            if (parameters.SecureLibrary == null)
            {
                if (!parameters.LightWeightSecurity)
                {
                    parameters.SecureLibrary = SecureLibraryName;
                }
                else
                {
                    parameters.SecureLibrary = LWSecureLibraryName;
                }
            }

            return true;
        }

        private string PrintSecureArgs()
        {
            string secureArgumentsString = "Secure Arguments:\n";

            if (!parameters.LightWeightSecurity)
            {
                if (!string.IsNullOrEmpty(parameters.SecureGovernanceFile))
                {
                    secureArgumentsString += "\t governance file: " + parameters.SecureGovernanceFile
                            + "\n";
                }

                if (parameters.SecurePermissionsFile != null)
                {
                    secureArgumentsString += "\t permissions file: " + parameters.SecurePermissionsFile
                            + "\n";
                }
                else
                {
                    secureArgumentsString += "\t permissions file: Not specified\n";
                }

                if (parameters.SecurePrivateKey != null)
                {
                    secureArgumentsString += "\t private key file: " + parameters.SecurePrivateKey
                            + "\n";
                }
                else
                {
                    secureArgumentsString += "\t private key file: Not specified\n";
                }

                if (parameters.SecureCertFile != null)
                {
                    secureArgumentsString += "\t certificate file: " + parameters.SecureCertFile
                            + "\n";
                }
                else
                {
                    secureArgumentsString += "\t certificate file: Not specified\n";
                }

                if (parameters.SecureCertAuthority != null)
                {
                    secureArgumentsString += "\t certificate authority file: "
                            + parameters.SecureCertAuthority + "\n";
                }
                else
                {
                    secureArgumentsString += "\t certificate authority file: Not specified\n";
                }

                if (parameters.SecureEncryptionAlgo != null)
                {
                    secureArgumentsString += "\t Encryption Algorithm: " + parameters.SecureEncryptionAlgo + "\n";
                }
            }

            secureArgumentsString += "\tPSK: ";
            if (parameters.SecurePSK != null)
            {
                secureArgumentsString += "In Use. Key: \""
                    + parameters.SecurePSK
                    + "\", Algorithm = "
                    + parameters.SecurePSKAlgorithm
                    + "\n";
            } else {
                secureArgumentsString += "Not Used\n";
            }

            secureArgumentsString += "\t Additional Authenticated Data: " + parameters.SecureEnableAAD + "\n";

            if (parameters.SecureLibrary != null)
            {
                secureArgumentsString += "\t Security Library: " + parameters.SecureLibrary + "\n";
            }
            else
            {
                secureArgumentsString += "\t Security Library: Not specified\n";
            }

            if (parameters.SecureDebug != -1)
            {
                secureArgumentsString += "\t debug level: " + parameters.SecureDebug + "\n";
            }

            return secureArgumentsString;
        }

        private void ConfigureSecurePlugin(ref DomainParticipantQos dpQos)
        {
            // configure use of security plugins, based on provided arguments

            // load plugin
            dpQos = dpQos.WithProperty(policy =>
                policy.Add("com.rti.serv.load_plugin",
                "com.rti.serv.secure"));

            dpQos = dpQos.WithProperty(policy =>
                policy.Add("com.rti.serv.secure.create_function",
                "RTI_Security_PluginSuite_create"));

            dpQos = dpQos.WithProperty(policy =>
                policy.Add("com.rti.serv.secure.library",
                parameters.SecureLibrary));

            //  Below, we are using com.rti.serv.secure properties in order to
            //  be backward compatible with RTI Connext DDS 5.3.0 and below.
            //  Later versions use the properties that are specified in the DDS
            //  Security specification (see also the RTI Security Plugins
            //  Getting Started Guide). However, later versions still support
            //  the legacy properties as an alternative.

            if (!parameters.LightWeightSecurity)
            {
                // check if governance file provided
                if (parameters.SecureGovernanceFile != null)
                {
                    dpQos = dpQos.WithProperty(policy =>
                        policy.Add("com.rti.serv.secure.access_control.governance_file",
                        parameters.SecureGovernanceFile));
                }
                else
                {
                    Console.Error.WriteLine("SecureGovernanceFile is required when using security.");
                    return;
                }

                // permissions file
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.access_control.permissions_file",
                    parameters.SecurePermissionsFile));

                // permissions authority file (legacy property, it should be permissions_file)
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.access_control.permissions_authority_file",
                    parameters.SecureCertAuthority));

                // certificate authority
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.authentication.ca_file",
                    parameters.SecureCertAuthority));

                // public key
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.authentication.certificate_file",
                    parameters.SecureCertFile));


                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.cryptography.max_receiver_specific_macs",
                    "4"));

                // private key
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.authentication.private_key_file",
                    parameters.SecurePrivateKey));


                if (parameters.SecureEncryptionAlgo != null)
                {
                    dpQos = dpQos.WithProperty(policy =>
                        policy.Add("com.rti.serv.secure.cryptography.encryption_algorithm",
                        parameters.SecureEncryptionAlgo));
                }
            }

            if (parameters.SecureEnableAAD)
            {
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.cryptography.enable_additional_authenticated_data",
                    "1"));
            }

            if (parameters.SecurePSK != null || parameters.SecurePSKAlgorithm != null)
            {
                if (parameters.SecurePSK == null)
                {
                    parameters.SecurePSK = "DefaultValue";
                }

                if (parameters.SecurePSKAlgorithm == null)
                {
                    parameters.SecurePSKAlgorithm = "AES256+GCM";
                }

                if (parameters.SecurePSKAlgorithm.Contains("GMAC"))
                {
                    dpQos = dpQos.WithProperty(policy =>
                        policy.Add("com.rti.serv.secure.dds.sec.access.rtps_psk_protection_kind", "SIGN"));
                }

                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.dds.sec.crypto.rtps_psk_secret_passphrase",
                    parameters.SecurePSK));

                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.dds.sec.crypto.rtps_psk_symmetric_cipher_algorithm",
                    parameters.SecurePSKAlgorithm));
            }

            if (parameters.SecureDebug != -1)
            {
                dpQos = dpQos.WithProperty(policy =>
                    policy.Add("com.rti.serv.secure.logging.log_level",
                    parameters.SecureDebug.ToString()));
            }
        }

        public IMessagingWriter CreateWriter(string topicName)
        {
            return new RTIWriter<T>(
                publisher.CreateDataWriter(
                    participant.CreateTopic<T>(topicName),
                    GetWriterQos(topicName)),
                (int)parameters.Instances,
                pongSemaphore,
                dataTypeHelper.Clone(),
                parameters.WriteInstance);
        }

        /*
         * CreateCFT
         * The CFT allows to the subscriber to receive a specific instance or a range of them.
         * In order generate the CFT it is necessary to create a condition:
         *      - In the case of a specific instance, it is necessary to convert to _CFTRange[0] into a key notation.
         *        Then it is enough with check that every element of key is equal to the instance.
         *        Example: _CFTRange[0] = 300. condition ="(0 = key[0] AND 0 = key[1] AND 1 = key[2] AND  44 = key[3])"
         *          So, in the case that the key = { 0, 0, 1, 44}, it will be received.
         *      - In the case of a range of instances, it is necessary to convert to _CFTRange[0] and _CFTRange[1] into a key notation.
         *        Then it is enough with check that the key is in the range of instances.
         *        Example: _CFTRange[1] = 300 and _CFTRange[1] = 1.
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
         *          The main goal for comaparing a instances and a key is by analyze the elements by more significant to the lest significant.
         *          So, in the case that the key is between [ {0, 0, 0, 1} and { 0, 0, 1, 44} ], it will be received.
         * Beside, there is a special case where all the subscribers will receive the samples, it is MAX_CFT_VALUE = 65535 = [255,255,0,0,]
         */
        public Rti.Dds.Topics.ContentFilteredTopic<T> CreateCft(String topicName, Topic<T> topic)
        {
            List<string> parameters = new List<string>();
            string condition;

            if (cftRange[0] == cftRange[1])
            { // If same elements, no range
                // parameters.ensure_length(KEY_SIZE.Value,KEY_SIZE.Value);
                Console.Error.WriteLine("CFT enabled for instance: '" + cftRange[0] + "'");
                for (int i = 0; i < KEY_SIZE.Value; i++)
                {
                    parameters.Add(Convert.ToString((byte)(cftRange[0] >> i * 8)));
                }
                condition = "(%0 = key[0] AND  %1 = key[1] AND %2 = key[2] AND  %3 = key[3]) OR " +
                        "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";
            }
            else
            { // If range
                // parameters.ensure_length(KEY_SIZE.VALUE * 2,KEY_SIZE.VALUE * 2);
                Console.Error.WriteLine("CFT enabled for instance range: [" + cftRange[0] + "," + cftRange[1] + "] ");
                for (int i = 0; i < KEY_SIZE.Value * 2; i++)
                {
                    parameters.Add(Convert.ToString((byte)(
                            cftRange[i < KEY_SIZE.Value ? 0 : 1]
                                    >> i % KEY_SIZE.Value * 8)));
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
                            ")" +
                        ")";
            }

            Filter filter = new Filter(condition, parameters);
            return participant.CreateContentFilteredTopic(topicName, topic, filter);
        }

        public IMessagingReader CreateReader(string topicName, IMessagingCallback callback)
        {
            DataReader<T> reader = null;

            if (parameters.CftSet && topicName == THROUGHPUT_TOPIC_NAME.Value)
            {
                reader = subscriber.CreateDataReader<T>(
                        CreateCft(
                            topicName,
                            participant.CreateTopic<T>(topicName)),
                        GetReaderQos(topicName));
            }
            else
            {
                // Rti.Config.Logger.Instance.SetVerbosity(Rti.Config.Verbosity.Warning);
                reader = subscriber.CreateDataReader<T>(
                    participant.CreateTopic<T>(topicName),
                    GetReaderQos(topicName));
            }

            if (callback != null)
            {
                reader.DataAvailable += _ =>
                {
                    // This is the equivalent of the On_data_available function.
                    using var samples = reader.Take();
                    foreach (var sample in samples)
                    {
                        if (sample.Info.ValidData)
                        {
                            callback.ProcessMessage(dataTypeHelper.SampleToMessage(sample.Data));
                        }
                    }
                };
                reader.StatusCondition.EnabledStatuses = StatusMask.DataAvailable;
            }
            else
            {
                reader.StatusCondition.EnabledStatuses = StatusMask.None;
            }
            return new RTIReader<T>(
                    reader,
                    dataTypeHelper.Clone(),
                    parameters);
        }

        private DomainParticipantQos GetParticipantQos(string profile)
        {
            var qosProvider = new QosProvider(parameters.QosFile);
            var participantQos = qosProvider.GetDomainParticipantQos(profile);

            // set initial peers and not use multicast
            if (peerHostCount > 0)
            {
                participantQos = participantQos.WithDiscovery(policy =>
                {
                    policy.InitialPeers.InsertRange(0, peerHostArray);
                    policy.MulticastReceiveAddresses.Clear();
                    // qos.discovery.multicast_receive_addresses = new DDS.StringSeq();
                });
            }

            if (!transport.ConfigureTransport(ref participantQos))
            {
                return null;
            }

            maxUnfragmentedRTPSPayloadSize = transport.MinimumMessageSizeMax - (PerftestTransport.MessageOverheadBytes);

            if (!DataSizeRelatedCalculations())
            {
                Console.Error.Write("[Error] Failed to configure the data size settings.\n");
                return null;
            }

            if (parameters.EnableAutoThrottle)
            {
                participantQos = participantQos.WithProperty(policy =>
                    policy.Add("dds.domain_participant.auto_throttle.enable", "true"));
            }

            if (parameters.Crc)
            {
                if (parameters.CrcKind == null)
                {
                    parameters.CrcKind = "CRC_32_CUSTOM";
                }

                participantQos = participantQos.
                    WithWireProtocol(policy => policy.ComputeCrc = true).
                    WithProperty(policy =>
                        policy.Add("dds.participant.wire_protocol.computed_Crc_kind",
                            parameters.CrcKind));
            }

            if (parameters.MessageLength)
            {
                participantQos = participantQos.WithProperty(policy =>
                    policy.Add("dds.participant.wire_protocol.enable_message_length_header_extension",
                        "true"));
            }

            return participantQos;
        }

        private DataReaderQos GetReaderQos(string topicName)
        {
            string qosProfile = parameters.QosLibrary + "::" + GetQoSProfileName(topicName);
            var qosProvider = new QosProvider(parameters.QosFile);

            DataReaderQos dataReaderQos = qosProvider.GetDataReaderQos(qosProfile);

            // only force reliability on throughput/latency topics
            if (topicName != ANNOUNCEMENT_TOPIC_NAME.Value)
            {
                if (!parameters.BestEffort)
                {
                    dataReaderQos = dataReaderQos.WithReliability(policy =>
                            policy.Kind = ReliabilityKind.Reliable);
                }
                else
                {
                    dataReaderQos = dataReaderQos.WithReliability(policy =>
                            policy.Kind = ReliabilityKind.BestEffort);
                }
            }

            if (parameters.NoPositiveAcks
                    && (topicName == THROUGHPUT_TOPIC_NAME.Value
                            || topicName == LATENCY_TOPIC_NAME.Value))
            {
                dataReaderQos = dataReaderQos.WithProtocol(policy =>
                    policy.DisablePositiveAcks = true);
            }

            if (topicName == THROUGHPUT_TOPIC_NAME.Value
                    || (topicName == LATENCY_TOPIC_NAME.Value
                        && !directCommunication
                        && ((DurabilityKind) parameters.Durability == DurabilityKind.Transient
                            || (DurabilityKind) parameters.Durability == DurabilityKind.Persistent)))
            {
                dataReaderQos = dataReaderQos = dataReaderQos.WithDurability(policy =>
                {
                    policy.Kind = (DurabilityKind) parameters.Durability;
                    policy.DirectCommunication = directCommunication;
                });
            }

            dataReaderQos = dataReaderQos.WithResourceLimits(policy =>
                policy.InitialInstances = (int)(parameters.Instances + 1));
            if (instanceMaxCountReader != -1)
            {
                instanceMaxCountReader++;
            }

            dataReaderQos = dataReaderQos.WithResourceLimits(policy =>
                policy.MaxInstances = instanceMaxCountReader);

            if (parameters.Instances > 1)
            {
                if (parameters.InstanceHashBuckets > 0)
                {
                    dataReaderQos = dataReaderQos.WithResourceLimits(policy =>
                        policy.InstanceHashBuckets = parameters.InstanceHashBuckets);
                }
                else
                {
                    dataReaderQos = dataReaderQos.WithResourceLimits(policy =>
                        policy.InstanceHashBuckets = (int)parameters.Instances);
                }
            }

            if (transport.UseMulticast && transport.AllowsMulticast())
            {
                string multicastAddr = transport.GetMulticastAddr(topicName);

                if (multicastAddr == null)
                {
                    Console.Error.WriteLine("topic name must either be "
                            + THROUGHPUT_TOPIC_NAME.Value
                            + " or " + LATENCY_TOPIC_NAME.Value
                            + " or " + ANNOUNCEMENT_TOPIC_NAME.Value);
                    return null;
                }

                dataReaderQos = dataReaderQos.WithMulticast(policy =>
                {
                    policy.Value.Add(Rti.Dds.Core.Policy.TransportMulticastSettings.Default);
                    policy.Value[0] = policy.Value[0].With(policy2 =>
                    {
                        policy2.ReceiveAddress = multicastAddr;
                        policy2.ReceivePort = 0;
                    });
                });
            }

            if (parameters.UnboundedSize > 0)
            {
                dataReaderQos = dataReaderQos.WithProperty(policy =>
                            policy.Add("dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                            parameters.UnboundedSize.ToString()));
            }

            return dataReaderQos;
        }

        private DataWriterQos GetWriterQos(string topicName)
        {
            string qosProfile = parameters.QosLibrary + "::" + GetQoSProfileName(topicName);

            var qosProvider = new QosProvider(parameters.QosFile);

            DataWriterQos dataWriterQos = qosProvider.GetDataWriterQos(qosProfile);

            if (parameters.NoPositiveAcks
                    && (qosProfile == "PerftestQosLibrary::ThroughputQos"
                        || qosProfile == "PerftestQosLibrary::LatencyQos"))
            {
                dataWriterQos = dataWriterQos.WithProtocol(policy =>
                {
                    policy.DisablePositiveAcks = true;
                    if (parameters.KeepDurationUsec != -1)
                    {
                        policy.RtpsReliableWriter.DisablePositiveAcksMinSampleKeepDuration =
                            Duration.FromMilliseconds((ulong)parameters.KeepDurationUsec / 1000);
                    }
                });
            }

            if (parameters.Asynchronous)
            {
                dataWriterQos = dataWriterQos.WithPublishMode(policy =>
                {
                    policy.Kind = PublishModeKind.Asynchronous;
                    if (!parameters.FlowController.StartsWith("default", true, null))
                    {
                        policy.FlowControllerName = "dds.flow_controller.token_bucket." + parameters.FlowController;
                    }
                });
            }

            // only force reliability on throughput/latency topics
            if (topicName != ANNOUNCEMENT_TOPIC_NAME.Value)
            {
                if (!parameters.BestEffort)
                {
                    // default: use the setting specified in the qos profile
                    dataWriterQos = dataWriterQos.WithReliability(policy =>
                        policy.Kind = ReliabilityKind.Reliable);
                }
                else
                {
                    // override to best-effort
                    dataWriterQos = dataWriterQos.WithReliability(policy =>
                        policy.Kind = ReliabilityKind.BestEffort);
                }
            }

            // These Qos settings are only set for the Throughput datawriter
            if (topicName == THROUGHPUT_TOPIC_NAME.Value)
            {
                if (transport.UseMulticast)
                {
                    dataWriterQos = dataWriterQos.WithProtocol(policy =>
                        policy.RtpsReliableWriter.EnableMulticastPeriodicHeartbeat = true);
                }

                if (parameters.BatchSize > 0)
                {
                    dataWriterQos = dataWriterQos.WithBatch(policy =>
                    {
                        policy.Enable = true;
                        policy.MaxDataBytes = parameters.BatchSize;
                    });
                    dataWriterQos = dataWriterQos.WithResourceLimits(policy =>
                        policy.MaxSamples = LengthUnlimited.Value);
                    dataWriterQos = dataWriterQos.WithWriterResourceLimits(policy =>
                        policy.MaxBatches = (int)parameters.SendQueueSize);
                }
                else
                {
                    dataWriterQos = dataWriterQos.WithResourceLimits(policy =>
                        policy.MaxSamples = (int)parameters.SendQueueSize);
                }

                if (parameters.EnableAutoThrottle)
                {
                    dataWriterQos = dataWriterQos.WithProperty(policy =>
                            policy.Add("dds.data_writer.auto_throttle.enable", "true"));
                }
                if (parameters.EnableTurboMode)
                {
                    dataWriterQos = dataWriterQos.WithProperty(policy =>
                            policy.Add("dds.data_writer.enable_turbo_mode", "true"));
                    dataWriterQos = dataWriterQos.WithBatch(policy =>
                            policy.Enable = false);
                    dataWriterQos = dataWriterQos.WithWriterResourceLimits(policy =>
                            policy.MaxBatches = (int)parameters.SendQueueSize);
                    dataWriterQos = dataWriterQos.WithResourceLimits(policy =>
                            policy.MaxSamples = AllocationSettings.Unlimited);
                }
            }

            if (parameters.UnboundedSize > 0)
            {
                dataWriterQos = dataWriterQos.WithProperty(policy =>
                            policy.Add("dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
                            parameters.UnboundedSize.ToString()));
            } else {
                dataWriterQos = dataWriterQos.WithProperty(policy =>
                            policy.Add("dds.data_writer.history.memory_manager.pluggable_allocator.underlying_allocator",
                            "fast_buffer_pool"));
            }

            return dataWriterQos;
        }

        public string GetQoSProfileName(string topicName)
        {
            if (qosProfileNameMap.TryGetValue(topicName, out string name))
            {
                return name;
            }
            else
            {
                Console.Error.WriteLine("topic name must either be "
                        + THROUGHPUT_TOPIC_NAME.Value
                        + " or " + LATENCY_TOPIC_NAME.Value
                        + " or " + ANNOUNCEMENT_TOPIC_NAME.Value);
                return null;
            }
        }
    }
} // PerformanceTest Namespace
