/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using System.Net;
using System.Net.Sockets;
using System.Text.RegularExpressions;
using System.IO;
using System.Linq;
using Omg.Dds.Core;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Domain;
using Rti.Dds.Publication;
using Rti.Dds.Subscription;
using Rti.Dds.Topics;
using Rti.Dds.Core.Policy;

namespace PerformanceTest
{
    public enum Transport
    {
        None,
        Udpv4,
        Udpv6,
        Tcpv4,
        Tlsv4,
        Dtlsv4,
        Wanv4,
        Shmem,
        Udpv4Shmem,
        Udpv4Udpv6,
        Udpv6Shmem,
        Udpv4Udpv6Shmem
    }

    public class TransportConfig
    {
        public TransportConfig()
        {
            Kind = Transport.None;
            NameString = string.Empty;
            PrefixString = string.Empty;
            TakenFromQoS = false;
        }

        public TransportConfig(
                Transport kind,
                string nameString,
                string prefixString)
        {
            this.Kind = kind;
            this.NameString = nameString;
            this.PrefixString = prefixString;
        }

        public Transport Kind { set; get; }
        public string NameString { set; get; }
        public string PrefixString { set; get; }
        public bool TakenFromQoS { set; get; }
    }

    public class SecureTransportOptions
    {
        public SecureTransportOptions()
        {
            CertAuthorityFile = string.Empty;
            CertificateFile = string.Empty;
            PrivateKeyFile = string.Empty;
        }

        public string CertAuthorityFile { set; get; }
        public string CertificateFile { set; get; }
        public string PrivateKeyFile { set; get; }
    };

    public class TcpTransportOptions
    {
        public TcpTransportOptions()
        {
            ServerBindPort = "7400";
            WanNetwork = false;
            PublicAddress = "";
        }

        public string ServerBindPort { set; get; }
        public bool WanNetwork { set; get; }
        public string PublicAddress { set; get; }
    }

    public class WanTransportOptions
    {
        public WanTransportOptions()
        {
            WanServerAddress = "";
            WanServerPort = "3478";
            WanId = "";
            SecureWan = false;
        }

        public string WanServerAddress { set; get; }
        public string WanServerPort { set; get; }
        public string WanId { set; get; }
        public bool SecureWan { set; get; }
    }

    public class PerftestTransport
    {
        /**************************************************************************/
        /* CLASS MEMBERS */

        private Parameters parameters;
        private TransportConfig TransportConfig = new TransportConfig();
        // TCP specific options
        private readonly TcpTransportOptions TcpOptions = new TcpTransportOptions();
        // Security files
        private readonly SecureTransportOptions SecureOptions = new SecureTransportOptions();
        // Wan specific options
        private readonly WanTransportOptions WanOptions = new WanTransportOptions();

        private static readonly Dictionary<string, TransportConfig> TransportConfigMap;

        // Tag used when adding logging output.
        private const string ClassLoggingString = "PerftestTransport:";

        // Default location of the security related files
        public const string TransportPrivateKeyFilePub = "./resource/secure/pubkey.pem";
        public const string TransportPrivateKeyFileSub = "./resource/secure/subkey.pem";
        public const string TransportCertificateFilePub = "./resource/secure/pub.pem";
        public const string TransportCertificateFileSub = "./resource/secure/sub.pem";
        public const string TransportCertAuthorityFile = "./resource/secure/cacert.pem";

        public bool UseMulticast;
        public bool CustomMulticastAddrSet;

        /*
        * This is the minimum size across all the active transports
        * message_size_max
        */
        public ulong MinimumMessageSizeMax = MessageSizeMaxNotSet;

        /*
        * When configuring the transport we might need to share information so it
        * is displayed in the summary, we will save it here.
        */
        public string LoggingString = string.Empty;

        private const ulong DefaultMessageSizeMax = 65536;
        public static readonly ulong MessageSizeMaxNotSet = long.MaxValue;
        public static readonly ulong MessageOverheadBytes = 567;
        // This number is calculated in C++ as:
        // - COMMEND_WRITER_MAX_RTPS_OVERHEAD <- Size of the overhead for RTPS in the
        //                                       worst case
        // - 48 <- Max transport overhead (this would be for ipv6).
        // - Encapsulation (RTI_CDR_ENCAPSULATION_HEADER_SIZE) + alignment  in the worst case (3)
        // MESSAGE_OVERHEAD_BYTES = (COMMEND_WRITER_MAX_RTPS_OVERHEAD + 48 + RTI_CDR_ENCAPSULATION_HEADER_SIZE + 3)
        public static readonly ulong CommendWriterMaxRtpsOverhead = 512;

        public SortedDictionary<string, string> MulticastAddrMap = new SortedDictionary<string, string>();

        /**************************************************************************/
        /* CLASS CONSTRUCTOR AND DESTRUCTOR */

        static PerftestTransport()
        {
            TransportConfigMap = new Dictionary<string, TransportConfig>()
            {
                { "Default", new TransportConfig(
                    Transport.None,
                    "--",
                    "--")},
                { "UDPv4", new TransportConfig(
                    Transport.Udpv4,
                    "UDPv4",
                    "dds.transport.UDPv4.builtin")},
                { "UDPv6", new TransportConfig(
                    Transport.Udpv6,
                    "UDPv6",
                    "dds.transport.UDPv6.builtin")},
                { "TCP", new TransportConfig(
                    Transport.Tcpv4,
                    "TCP",
                    "dds.transport.TCPv4.tcp1")},
                { "TLS", new TransportConfig(
                    Transport.Tlsv4,
                    "TLS",
                    "dds.transport.TCPv4.tcp1")},
                { "DTLS", new TransportConfig(
                    Transport.Dtlsv4,
                    "DTLS",
                    "dds.transport.DTLS.dtls1")},
                { "WAN", new TransportConfig(
                    Transport.Wanv4,
                    "WAN",
                    "dds.transport.WAN.wan1")},
                { "SHMEM", new TransportConfig(
                    Transport.Shmem,
                    "SHMEM",
                    "dds.transport.shmem.builtin")}
            };
        }

        public PerftestTransport()
        {
            MulticastAddrMap.Add(LATENCY_TOPIC_NAME.Value, "239.255.1.2");
            MulticastAddrMap.Add(ANNOUNCEMENT_TOPIC_NAME.Value, "239.255.1.100");
            MulticastAddrMap.Add(THROUGHPUT_TOPIC_NAME.Value, "239.255.1.1");
        }

        public string PrintTransportConfigurationSummary()
        {
            StringBuilder sb = new StringBuilder("Transport Information:\n");
            sb.Append("\tKind: ").Append(TransportConfig.NameString).Append('\n');

            if (!string.IsNullOrEmpty(parameters.AllowInterfaces))
            {
                sb.Append("\tNic: ").Append(parameters.AllowInterfaces).Append('\n');
            }

            sb.Append("\tUse Multicast: ");

            if (AllowsMulticast() && UseMulticast)
            {
                sb.Append("True");
            }
            else
            {
                sb.Append("False");
                if (UseMulticast)
                {
                    sb.Append(" (Multicast is not supported for ");
                    sb.Append(TransportConfig.NameString);
                    sb.Append(')');
                }
            }
            sb.Append('\n');

            if (CustomMulticastAddrSet)
            {
                sb.Append("\tUsing custom Multicast Addresses:");
                sb.Append("\n\t\tThroughput Address: ");
                sb.Append(GetMulticastAddr(THROUGHPUT_TOPIC_NAME.Value));
                sb.Append("\n\t\tLatency Address: ");
                sb.Append(GetMulticastAddr(LATENCY_TOPIC_NAME.Value));
                sb.Append("\n\t\tAnnouncement Address: ");
                sb.Append(GetMulticastAddr(ANNOUNCEMENT_TOPIC_NAME.Value));
                sb.Append('\n');
            }

            if (!string.IsNullOrEmpty(LoggingString))
            {
                sb.Append(LoggingString);
            }

            if (TransportConfig.Kind == Transport.Tcpv4
                    || TransportConfig.Kind == Transport.Tlsv4)
            {
                sb.Append("\tTCP Server Bind Port: ").Append(TcpOptions.ServerBindPort).Append('\n');
                sb.Append("\tTCP LAN/WAN mode: ");
                if (TcpOptions.WanNetwork)
                {
                    sb.Append("WAN\n");
                }
                else
                {
                    sb.Append("LAN\n");
                }
                if (TcpOptions.WanNetwork)
                {
                    sb.Append("\tTCP Public Address: ").Append(TcpOptions.PublicAddress).Append('\n');
                }
            }

            if (TransportConfig.Kind == Transport.Wanv4)
            {
                sb.Append("\tWAN Server Address: ");
                sb.Append(WanOptions.WanServerAddress).Append(':');
                sb.Append(WanOptions.WanServerPort).Append('\n');

                sb.Append("\tWAN Id: ").Append(WanOptions.WanId).Append('\n');

                sb.Append("\tWAN Secure: ").Append(WanOptions.SecureWan).Append('\n');
            }

            if (TransportConfig.Kind == Transport.Tlsv4
                    || TransportConfig.Kind == Transport.Dtlsv4
                    || (TransportConfig.Kind == Transport.Wanv4 && WanOptions.SecureWan))
            {
                sb.Append("\tCertificate authority file: ").Append(SecureOptions.CertAuthorityFile).Append('\n');
                sb.Append("\tCertificate file: ").Append(SecureOptions.CertificateFile).Append('\n');
                sb.Append("\tPrivate key file: ").Append(SecureOptions.PrivateKeyFile).Append('\n');
            }

            if (!string.IsNullOrEmpty(parameters.ConfigureTransportVerbosity))
            {
                sb.Append("\tVerbosity: ").Append(parameters.ConfigureTransportVerbosity).Append('\n');
            }

            return sb.ToString();
        }

        public bool AllowsMulticast()
        {
            return TransportConfig.Kind != Transport.Tcpv4
                    && TransportConfig.Kind != Transport.Tlsv4
                    && TransportConfig.Kind != Transport.Wanv4
                    && TransportConfig.Kind != Transport.Shmem;
        }

        public bool ParseTransportOptions(Parameters _parameters)
        {
            parameters = _parameters;

            string transportString = "Default";

            if (parameters.TransportSet)
            {
                if (parameters.Transport.Length == 0)
                {
                    transportString = "UDPv4";
                }
                else
                {
                    transportString = parameters.Transport;
                }
            }

            if (parameters.EnableTCP)
            {
                transportString = "TCP";
            }

            if (parameters.EnableUDPv6)
            {
                transportString = "UDPv6";
            }

            if (parameters.EnableSharedMemory)
            {
                transportString = "SHMEM";
            }

            if (!parameters.AllowInterfacesSet)
            {
                parameters.AllowInterfaces = string.Empty;
            }

            TcpOptions.ServerBindPort = parameters.ConfigureTransportServerBindPort;

            TcpOptions.WanNetwork = parameters.ConfigureTransportWan;

            if (parameters.ConfigureTransportPublicAddressSet)
            {
                TcpOptions.PublicAddress = parameters.ConfigureTransportPublicAddress;
            }

            if (parameters.ConfigureTransportCertAuthoritySet)
            {
                SecureOptions.CertAuthorityFile = parameters.ConfigureTransportCertAuthority;
            }

            if (parameters.ConfigureTransportWanServerAddressSet)
            {
                WanOptions.WanServerAddress = parameters.ConfigureTransportWanServerAddress;
            }

            WanOptions.WanServerPort = parameters.ConfigureTransportWanServerPort;

            if (parameters.ConfigureTransportWanIdSet)
            {
                WanOptions.WanId = parameters.ConfigureTransportWanId;
            }

            WanOptions.SecureWan = parameters.ConfigureTransportSecureWan;

            UseMulticast = parameters.Multicast;

            if (parameters.NoMulticastSet)
            {
                UseMulticast = !parameters.NoMulticast;
            }

            if (parameters.MulticastAddrSet)
            {
                UseMulticast = true;
                if (!ParseMulticastAddresses(parameters.MulticastAddr))
                {
                    Console.Error.WriteLine(ClassLoggingString
                            + " Error parsing -multicastAddr");
                    return false;
                }
            }

            if (!SetTransport(transportString))
            {
                Console.Error.Write(ClassLoggingString + " Error Setting the transport");
                return false;
            }

            // We only need to set the secure properties for this
            if (TransportConfig.Kind == Transport.Tlsv4
                    || TransportConfig.Kind == Transport.Dtlsv4
                    || TransportConfig.Kind == Transport.Wanv4)
            {
                PopulateSecurityFiles();
            }
            return true;
        }

        /**************************************************************************/

        private bool SetTransport(String transportString)
        {
            try
            {
                TransportConfig = TransportConfigMap[transportString];
            }
            catch (KeyNotFoundException)
            {
                Console.Error.Write(ClassLoggingString + " \""
                        + transportString + "\" is not a valid transport. "
                        + "List of supported transport:");
                foreach (string transportName in TransportConfigMap.Keys)
                {
                    Console.Error.Write("\t\"" + transportName + "\"");
                }
                return false;
            }

            return true;
        }

        private void PopulateSecurityFiles()
        {
            if (string.IsNullOrEmpty(SecureOptions.CertificateFile))
            {
                if (parameters.Pub)
                {
                    SecureOptions.CertificateFile = TransportCertificateFilePub;
                }
                else
                {
                    SecureOptions.CertificateFile = TransportCertificateFileSub;
                }
            }

            if (string.IsNullOrEmpty(SecureOptions.PrivateKeyFile))
            {
                if (parameters.Pub)
                {
                    SecureOptions.PrivateKeyFile = TransportPrivateKeyFilePub;
                }
                else
                {
                    SecureOptions.PrivateKeyFile = TransportPrivateKeyFileSub;
                }
            }

            if (string.IsNullOrEmpty(SecureOptions.CertAuthorityFile))
            {
                SecureOptions.CertAuthorityFile = TransportCertAuthorityFile;
            }
        }

        /**************************************************************************/

        private void SetAllowInterfacesList(ref DomainParticipantQos qos)
        {
            if (!string.IsNullOrEmpty(parameters.AllowInterfaces))
            {
                if (TransportConfig.Kind == Transport.None)
                {
                    Console.Error.WriteLine(ClassLoggingString
                           + " Ignoring -nic/-allowInterfaces option.");
                    return;
                }

                if (TransportConfig.Kind == Transport.Udpv4Udpv6Shmem
                        || TransportConfig.Kind == Transport.Udpv4Udpv6)
                {
                    qos = qos.WithProperty(policy =>
                        policy.Add(
                            "dds.transport.UDPv4.builtin.parent.allow_interfaces",
                            parameters.AllowInterfaces));

                    qos = qos.WithProperty(policy =>
                        policy.Add(
                            "dds.transport.UDPv6.builtin.parent.allow_interfaces",
                            parameters.AllowInterfaces));
                }
                else
                {
                    string propertyName = TransportConfig.PrefixString;

                    if (TransportConfig.Kind == Transport.Wanv4)
                    {
                        propertyName += ".parent";
                    }
                    propertyName += ".parent.allow_interfaces";

                    qos = qos.WithProperty(policy =>
                        policy.Add(propertyName, parameters.AllowInterfaces));
                }
            }
        }

        private void SetTransportVerbosity(ref DomainParticipantQos qos)
        {
            string transportVerbosity = "1";

            if (!string.IsNullOrEmpty(parameters.ConfigureTransportVerbosity))
            {
                transportVerbosity = parameters.ConfigureTransportVerbosity;
            }

            if (TransportConfig.Kind == Transport.None)
            {
                Console.Error.Write(ClassLoggingString
                    + " Ignoring -transportVerbosity option.");
                return;
            }

            string propertyName = TransportConfig.PrefixString + ".verbosity";

            // The name of the property in TCPv4 is different
            if (TransportConfig.Kind == Transport.Tcpv4)
            {
                propertyName = TransportConfig.PrefixString + ".logging_verbosity_bitmap";
            }
            else if (TransportConfig.Kind == Transport.Udpv4
                    || TransportConfig.Kind == Transport.Udpv6
                    || TransportConfig.Kind == Transport.Shmem
                    || TransportConfig.Kind == Transport.Udpv4Shmem
                    || TransportConfig.Kind == Transport.Udpv6Shmem
                    || TransportConfig.Kind == Transport.Udpv4Udpv6
                    || TransportConfig.Kind == Transport.Udpv4Udpv6Shmem)
            {
                // We do not change logging for the builtin transports.
                return;
            }

            qos = qos.WithProperty(policy =>
                policy.Add(propertyName, transportVerbosity));
        }

        private void ConfigureSecurityFiles(ref DomainParticipantQos qos)
        {
            if (!string.IsNullOrEmpty(SecureOptions.CertAuthorityFile))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".tls.verify.ca_file",
                    SecureOptions.CertAuthorityFile));
            }

            if (!string.IsNullOrEmpty(SecureOptions.CertificateFile))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".tls.identity.certificate_chain_file",
                    SecureOptions.CertificateFile));
            }

            if (!string.IsNullOrEmpty(SecureOptions.PrivateKeyFile))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".tls.identity.private_key_file",
                    SecureOptions.PrivateKeyFile));
            }
        }

        private DomainParticipantQos ConfigureTcpTransport(DomainParticipantQos initialQos)
        {
            var qos = initialQos.WithTransportBuiltin(policy =>
                    policy.Mask = TransportBuiltinMask.None);

            Dictionary<string, string> properties = new Dictionary<string, string>();

            properties.Add("dds.transport.load_plugins", TransportConfig.PrefixString);

            if (!string.IsNullOrEmpty(TcpOptions.ServerBindPort))
            {
                properties.Add(TransportConfig.PrefixString + ".server_bind_port",
                    TcpOptions.ServerBindPort);
            }

            if (TcpOptions.WanNetwork)
            {
                properties.Add(TransportConfig.PrefixString + ".parent.classid",
                    "NDDS_TRANSPORT_CLASSID_TCPV4_WAN");

                if (TcpOptions.ServerBindPort != "0")
                {
                    if (!string.IsNullOrEmpty(TcpOptions.PublicAddress))
                    {
                        properties.Add(TransportConfig.PrefixString + ".public_address",
                            TcpOptions.PublicAddress);
                    }
                    else
                    {
                        Console.Error.WriteLine(ClassLoggingString
                                + " Public Address is required if Server Bind Port != 0");
                        return null;
                    }
                }
            }

            if (TransportConfig.Kind == Transport.Tlsv4)
            {
                if (TcpOptions.WanNetwork)
                {
                    properties.Add(TransportConfig.PrefixString + ".parent.classid",
                        "NDDS_TRANSPORT_CLASSID_TLSV4_WAN");
                }
                else
                {
                    properties.Add(TransportConfig.PrefixString + ".parent.classid",
                        "NDDS_TRANSPORT_CLASSID_TLSV4_LAN");
                }

                ConfigureSecurityFiles(ref qos);
            }

            qos = qos.WithProperty(Property.FromDictionary(properties));

            return qos;
        }

        private void ConfigureDtlsTransport(ref DomainParticipantQos qos)
        {
            qos = qos.WithTransportBuiltin(policy =>
                    policy.Mask = Rti.Dds.Core.Policy.TransportBuiltinMask.None);

            qos = qos.WithProperty(policy =>
            {
                policy.Add("dds.transport.load_plugins",
                TransportConfig.PrefixString);
                policy.Add(TransportConfig.PrefixString + ".library",
                "nddstransporttls");
                policy.Add(TransportConfig.PrefixString + ".create_function",
                "NDDS_Transport_DTLS_create");
            });

            ConfigureSecurityFiles(ref qos);
        }

        private bool ConfigureWanTransport(ref DomainParticipantQos qos)
        {
            qos = qos.WithTransportBuiltin(policy =>
                    policy.Mask = Rti.Dds.Core.Policy.TransportBuiltinMask.None);

            qos = qos.WithProperty(policy =>
                policy.Add("dds.transport.load_plugins",
                TransportConfig.PrefixString));

            qos = qos.WithProperty(policy =>
                policy.Add(TransportConfig.PrefixString + ".library",
                "nddstransportwan"));

            qos = qos.WithProperty(policy =>
                policy.Add(TransportConfig.PrefixString + ".create_function",
                "NDDS_Transport_WAN_create"));

            if (!string.IsNullOrEmpty(WanOptions.WanServerAddress))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".server",
                    WanOptions.WanServerAddress));
            }
            else
            {
                Console.Error.Write(ClassLoggingString + " Wan Server Address is required");
                return false;
            }

            if (!string.IsNullOrEmpty(WanOptions.WanServerPort))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".server_port",
                    WanOptions.WanServerPort));
            }

            if (!string.IsNullOrEmpty(WanOptions.WanId))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".transport_instance_id",
                    WanOptions.WanId));
            }
            else
            {
                Console.Error.Write(ClassLoggingString + " Wan ID is required");
                return false;
            }

            if (WanOptions.SecureWan)
            {
                qos = qos.WithProperty(policy =>
                    policy.Add(TransportConfig.PrefixString + ".enable_security",
                    "1"));

                ConfigureSecurityFiles(ref qos);
            }

            return true;
        }

        private void ConfigureShmemTransport(ref DomainParticipantQos qos)
        {
            ulong parentMsgSizeMax = MinimumMessageSizeMax;
            bool messageSizeMaxSet = false;

            /*
            * If the property defining the message_size_max for shared memory is not
            * set and we are using exclusively SHMEM, then we will calculate
            * automatically the message_size_max to accommodate the sample in a single
            * packet and avoid fragmentation.
            */

            if (qos.Property.Value.TryGetValue("dds.transport.shmem.builtin.parent.message_size_max",
                out var propertyEntry))
            {
                parentMsgSizeMax = Convert.ToUInt64(propertyEntry.Value);
                messageSizeMaxSet = true;
            }
            else
            {
                if (qos.TransportBuiltin.Mask == Rti.Dds.Core.Policy.TransportBuiltinMask.Shmem
                    && (parameters.DataLen + MessageOverheadBytes) > parentMsgSizeMax)
                {
                    parentMsgSizeMax = parameters.DataLen + MessageOverheadBytes;
                    MinimumMessageSizeMax = parentMsgSizeMax;
                }

                qos = qos.WithProperty(policy =>
                    policy.Add("dds.transport.shmem.builtin.parent.message_size_max",
                    parentMsgSizeMax.ToString()));
                LoggingString += "\tSHMEM message_size_max: " + parentMsgSizeMax + "\n";
            }

            ulong maxBufferSize = Math.Max(121634816 /* 116MB */, parentMsgSizeMax);

            if (!messageSizeMaxSet &&
                    parentMsgSizeMax > maxBufferSize)
            {
                parentMsgSizeMax = maxBufferSize;

                MinimumMessageSizeMax = parentMsgSizeMax;

                qos = qos.WithProperty(policy =>
                    policy.Add("dds.transport.shmem.builtin.parent.message_size_max",
                    maxBufferSize.ToString()));
                LoggingString += "\tSHMEM message_size_max: " + parentMsgSizeMax + "\n";
            }

            /*
             * From user manual "Properties for Builtin Shared-Memory Transport":
             * To optimize memory usage, specify a receive queue size less than that
             * required to hold the maximum number of messages which are all of the
             * maximum size.
             *
             * In most situations, the average message size may be far less than the
             * maximum message size. So for example, if the maximum message size is 64K
             * bytes, and you configure the plugin to buffer at least 10 messages, then
             * 640K bytes of memory would be needed if all messages were 64K bytes.
             * Should this be desired, then receive_buffer_size should be set to 640K
             * bytes.
             *
             * However, if the average message size is only 10K bytes, then you could
             * set the receive_buffer_size to 100K bytes. This allows you to optimize
             * the memory usage of the plugin for the average case and yet allow the
             * plugin to handle the extreme case.
             *
             * The receivedMessageCountMax should be set to a value that can hold
             * more than “-sendQueueSize" samples in perftest in order block in the
             * send window before starting to lose messages on the Shared Memory
             * transport
             */

            /*
             * This is the flow Controller default token size. Change this if you modify
             * the qos file to add a different "bytes_per_token" property
             */
            ulong flowControllerTokenSize = MinimumMessageSizeMax;

            /*
             * We choose the minimum between the flow Controller max fragment size and
             * the message_size_max - RTPS headers.
             */
            ulong fragmentSize = Math.Min(
                    parentMsgSizeMax - CommendWriterMaxRtpsOverhead,
                    flowControllerTokenSize - CommendWriterMaxRtpsOverhead);

            ulong rtpsMessagesPerSample = Math.Max(1, (parameters.DataLen / fragmentSize) + 1);

            ulong receivedMessageCountMax = (2 * parameters.SendQueueSize) + (1 * rtpsMessagesPerSample);

            ulong receiveBufferSize = Math.Min(
                    maxBufferSize,
                    receivedMessageCountMax * (CommendWriterMaxRtpsOverhead + fragmentSize));

            if (!qos.Property.Value.TryGetValue("dds.transport.shmem.builtin.received_message_count_max",
                out propertyEntry))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add("dds.transport.shmem.builtin.received_message_count_max",
                    receivedMessageCountMax.ToString()));

                LoggingString += "\tSHMEM received_message_count_max: "
                        + receivedMessageCountMax
                        + "\n";
            }

            if (!qos.Property.Value.TryGetValue("dds.transport.shmem.builtin.receive_buffer_size",
                out propertyEntry))
            {
                qos = qos.WithProperty(policy =>
                    policy.Add("dds.transport.shmem.builtin.receive_buffer_size",
                    receiveBufferSize.ToString()));

                LoggingString += "\tSHMEM receive_buffer_size: "
                        + receiveBufferSize
                        + "\n";
            }
        }

        /*
        * Gets the MessageSizeMax given the name (String) of a transport from a
        * DDS_DomainParticipantQos object. If the value is not present, returns
        * DEFAULT_MESSAGE_SIZE_MAX.
        */
        private static ulong GetTransportMessageSizeMax(
                String targetTransportName,
                DomainParticipantQos qos)
        {
            string propertyName = TransportConfigMap[targetTransportName].PrefixString + ".parent.message_size_max";

            if (qos.Property.Value.TryGetValue(
                propertyName,
                out var propertyEntry))
            {
                return Convert.ToUInt64(propertyEntry.Value);
            }
            else
            {
                return DefaultMessageSizeMax;
            }
        }

        /*
         * Configures the minimumMessageSizeMax value in the PerftestTransport object with
         * the minimum value for all the enabled transports in the XML configuration.
         */
        private void GetTransportMinimumMessageSizeMax(DomainParticipantQos qos)
        {
            ulong transportMessageSizeMax = MessageSizeMaxNotSet;
            int mask = (int)qos.TransportBuiltin.Mask;

            if ((mask & (int)TransportBuiltinMask.Shmem) != 0)
            {
                transportMessageSizeMax = GetTransportMessageSizeMax("SHMEM", qos);
            }
            if ((mask & (int)TransportBuiltinMask.Udpv4) != 0)
            {
                transportMessageSizeMax = GetTransportMessageSizeMax("UDPv4", qos);
            }
            if ((mask & (int)TransportBuiltinMask.Udpv6) != 0)
            {
                transportMessageSizeMax = GetTransportMessageSizeMax("UDPv6", qos);
            }

            if (TransportConfig.Kind == Transport.Tcpv4
                    || TransportConfig.Kind == Transport.Tlsv4)
            {
                transportMessageSizeMax = GetTransportMessageSizeMax("TCP", qos);
            }

            if (TransportConfig.Kind == Transport.Dtlsv4)
            {
                transportMessageSizeMax = GetTransportMessageSizeMax("DTLS", qos);
            }

            if (TransportConfig.Kind == Transport.Wanv4)
            {
                transportMessageSizeMax = GetTransportMessageSizeMax("WAN", qos);
            }

            MinimumMessageSizeMax = transportMessageSizeMax;
        }

        public bool ConfigureTransport(ref DomainParticipantQos qos)
        {
            /*
             * If transportConfig.kind is not set, then we want to use the value
             * provided by the Participant Qos, so first we read it from there and
             * update the value of transportConfig.kind with whatever was already set.
             */
            if (TransportConfig.Kind == Transport.None)
            {
                TransportConfig = qos.TransportBuiltin.Mask switch
                {
                    TransportBuiltinMask.Udpv4 => new TransportConfig(
                                Transport.Udpv4,
                                "UDPv4",
                                "dds.transport.UDPv4.builtin"),
                    TransportBuiltinMask.Udpv6 => new TransportConfig(
                                Transport.Udpv6,
                                "UDPv6",
                                "dds.transport.UDPv6.builtin"),
                    TransportBuiltinMask.Shmem => new TransportConfig(
                                Transport.Shmem,
                                "SHMEM",
                                "dds.transport.shmem.builtin"),
                    TransportBuiltinMask.Shmem | TransportBuiltinMask.Udpv4 => new TransportConfig(
                                Transport.Udpv4Shmem,
                                "UDPv4 & SHMEM",
                                "dds.transport.UDPv4.builtin"),
                    TransportBuiltinMask.Udpv6 | TransportBuiltinMask.Udpv4 => new TransportConfig(
                                Transport.Udpv4Udpv6,
                                "UDPv4 & UDPv6",
                                "dds.transport.UDPv4.builtin"),
                    TransportBuiltinMask.Udpv6 | TransportBuiltinMask.Shmem => new TransportConfig(
                                Transport.Udpv6Shmem,
                                "UDPv6 & SHMEM",
                                "dds.transport.UDPv6.builtin"),
                    TransportBuiltinMask.Udpv4 | TransportBuiltinMask.Udpv6 | TransportBuiltinMask.Shmem
                    => new TransportConfig(
                                Transport.Udpv4Udpv6Shmem,
                                "UDPv4 & UDPv6 & SHMEM",
                                "dds.transport.UDPv4.builtin"),
                    /*
                    * This would mean that the mask is either empty or a
                    * different value that we do not support yet. So we keep
                    * the value as "TRANSPORT_NOT_SET"
                    */
                    _ => null
                };
                TransportConfig.TakenFromQoS = true;
            }

            switch (TransportConfig.Kind)
            {
                case Transport.Udpv4:
                    qos = qos.WithTransportBuiltin(policy =>
                        policy.Mask = TransportBuiltinMask.Udpv4);
                    break;

                case Transport.Udpv6:
                    qos = qos.WithTransportBuiltin(policy =>
                        policy.Mask = TransportBuiltinMask.Udpv6);
                    break;

                case Transport.Shmem:
                    qos = qos.WithTransportBuiltin(policy =>
                        policy.Mask = TransportBuiltinMask.Shmem);
                    break;
            }

            /*
            * Once the configurations have been established, we can get the
            * MessageSizeMax for the Transport, which should be the minimum of
            * all the enabled transports
            */
            GetTransportMinimumMessageSizeMax(qos);

            switch (TransportConfig.Kind)
            {
                case Transport.Udpv4:
                    break;

                case Transport.Udpv6:
                    break;

                case Transport.Shmem:
                    ConfigureShmemTransport(ref qos);
                    break;

                case Transport.Tcpv4:
                    qos = ConfigureTcpTransport(qos);
                    if (qos == null)
                    {
                        Console.Error.Write(ClassLoggingString + " Failed to configure TCP plugin");
                        return false;
                    }
                    break;

                case Transport.Tlsv4:
                    qos = ConfigureTcpTransport(qos);
                    if (qos == null)
                    {
                        Console.Error.Write(ClassLoggingString + " Failed to configure TCP - TLS plugin");
                        return false;
                    }
                    break;

                case Transport.Dtlsv4:
                    ConfigureDtlsTransport(ref qos);
                    break;

                case Transport.Wanv4:
                    if (!ConfigureWanTransport(ref qos))
                    {
                        Console.Error.Write(ClassLoggingString + " Failed to configure WAN plugin");
                        return false;
                    }
                    break;

                default:

                    /*
                    * If shared memory is enabled we want to set up its
                    * specific configuration
                    */

                    if (qos.TransportBuiltin.Mask == TransportBuiltinMask.Shmem)
                    {
                        ConfigureShmemTransport(ref qos);
                    }

                    break;
            } // Switch

            if (TransportConfig.Kind != Transport.None
                    && TransportConfig.Kind != Transport.Shmem)
            {
                SetAllowInterfacesList(ref qos);
            }
            else
            {
                // We are not using the allow interface string, so we clear it
                parameters.AllowInterfaces = string.Empty;
            }

            SetTransportVerbosity(ref qos);

            return true;
        }

        private static string IncreaseAddressByOne(string addr)
        {
            bool success = false;
            string nextAddr;
            Byte[] buffer;

            try
            {
                buffer = IPAddress.Parse(addr).GetAddressBytes();
            }
            catch (System.Exception e)
            {
                Console.Error.Write(ClassLoggingString
                        + " Error parsing address. Exception: " + e.Source + "\n");
                return null;
            }

            /*
            * Increase the full address by one value.
            * if the Address is 255.255.255.255 (or the equivalent for IPv6)
            * this function will FAIL
            */
            for (int i = buffer.Length - 1; i >= 0 && !success; i--)
            {
                if (buffer[i] == (Byte)255)
                {
                    buffer[i] = 0;
                }
                else
                {
                    /* Increase the value and exit */
                    buffer[i]++;
                    success = true;
                }
            }

            if (!success)
            {
                Console.Error.Write(ClassLoggingString
                        + " IP value too high. Please use -help for more information"
                        + " about -multicastAddr command line\n");
                return null;
            }

            /* Get the string format of the address */
            try
            {
                nextAddr = new IPAddress(buffer).ToString();
            }
            catch (System.Exception e)
            {
                Console.Error.Write(ClassLoggingString
                        + " Error recovering address from byte format : "
                        + e.Source + "\n");
                return null;
            }

            return nextAddr;
        }

        private bool ParseMulticastAddresses(string arg)
        {
            /*
            * Split the string into different parts delimited with ',' character.
            * With a "a,b,c" input this will result in three different addresses
            * "a","b" and "c"
            */
            string[] addresses = arg.Split(',');

            /* If three addresses are given */
            if (addresses.Length == 3)
            {
                MulticastAddrMap[THROUGHPUT_TOPIC_NAME.Value] = addresses[0];
                MulticastAddrMap[LATENCY_TOPIC_NAME.Value] = addresses[1];
                MulticastAddrMap[ANNOUNCEMENT_TOPIC_NAME.Value] = addresses[2];
            }
            else if (addresses.Length == 1)
            {
                /* If only one address is given */
                MulticastAddrMap[THROUGHPUT_TOPIC_NAME.Value] = addresses[0];

                /* Calculate the consecutive one */
                MulticastAddrMap[LATENCY_TOPIC_NAME.Value] = IncreaseAddressByOne(addresses[0]);
                if (MulticastAddrMap[LATENCY_TOPIC_NAME.Value] == null)
                {
                    Console.Error.Write(ClassLoggingString
                            + " Fail to increase the value of given IP address\n");
                    return false;
                }

                /* Calculate the consecutive one */
                MulticastAddrMap[ANNOUNCEMENT_TOPIC_NAME.Value]
                        = IncreaseAddressByOne(MulticastAddrMap[LATENCY_TOPIC_NAME.Value]);
                if (MulticastAddrMap[ANNOUNCEMENT_TOPIC_NAME.Value] == null)
                {
                    Console.Error.Write(ClassLoggingString
                            + " Fail to increase the value of given IP address\n");
                    return false;
                }
            }
            else
            {
                Console.Error.Write(ClassLoggingString
                        + " Error parsing Address/es '" + arg
                        + "' for -multicastAddr option\n"
                        + "Use -help option to see the correct sintax\n");
                return false;
            }
            /* Last check. All the IPs must be in IP format and multicast range */
            if (!IsMulticast(MulticastAddrMap[THROUGHPUT_TOPIC_NAME.Value])
                    || !IsMulticast(MulticastAddrMap[LATENCY_TOPIC_NAME.Value])
                    || !IsMulticast(MulticastAddrMap[ANNOUNCEMENT_TOPIC_NAME.Value]))
            {
                Console.Error.Write(ClassLoggingString
                        + " Error parsing the address/es " + arg
                        + " for -multicastAddr option\n" +
                        "Use -help option to see the correct sintax\n");
                return false;
            }

            return true;
        }

        public static bool IsMulticast(string addr)
        {
            IPAddress address;

            try
            {
                /* Get address from string into IP format */
                address = IPAddress.Parse(addr);
            }
            catch (System.Exception e)
            {
                Console.Error.Write(ClassLoggingString
                        + " Error parsing address. Exception: " + e.Source + "\n");
                return false;
            }

            /*
             * Check if is a IPV6 multicast address or if is a IPv4 multicast
             * address checking the value of the most significant octet.
             */
            return address.IsIPv6Multicast
                    || (address.GetAddressBytes()[0] >= (byte)224
                        && address.GetAddressBytes()[0] <= (byte)239);
        }

        public string GetMulticastAddr(string topicName)
        {
            if (MulticastAddrMap.TryGetValue(topicName, out string address))
            {
                return address;
            }
            else
            {
                return null;
            }
        }
    }
} // Perftest Namespace
