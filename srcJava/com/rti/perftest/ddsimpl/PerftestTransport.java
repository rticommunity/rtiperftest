/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.net.InetAddress;
import java.nio.file.WatchEvent.Kind;
import java.util.Map;
import java.util.HashMap;
import java.util.regex.Pattern;
import com.rti.dds.infrastructure.PropertyQosPolicyHelper;
import com.rti.dds.infrastructure.TransportBuiltinKind;
import com.rti.dds.domain.DomainParticipantQos;
import com.rti.perftest.gen.THROUGHPUT_TOPIC_NAME;
import com.rti.perftest.gen.LATENCY_TOPIC_NAME;
import com.rti.perftest.gen.ANNOUNCEMENT_TOPIC_NAME;

public class PerftestTransport {

    /* Internal classes and enums */

    public enum Transport {
        TRANSPORT_NOT_SET,
        TRANSPORT_UDPv4,
        TRANSPORT_UDPv6,
        TRANSPORT_TCPv4,
        TRANSPORT_TLSv4,
        TRANSPORT_DTLSv4,
        TRANSPORT_WANv4,
        TRANSPORT_SHMEM,
        TRANSPORT_UDPv4_SHMEM,
        TRANSPORT_UDPv4_UDPv6,
        TRANSPORT_UDPv6_SHMEM,
        TRANSPORT_UDPv4_UDPv6_SHMEM
    };

    public class TransportConfig {

        public Transport kind = Transport.TRANSPORT_NOT_SET;
        public String nameString = "";
        public String prefixString = "";
        public boolean takenFromQoS = false;

        public TransportConfig()
        {
        }

        public TransportConfig(
                Transport inputKind,
                String inputNameString,
                String inputPrefixString)
        {
            kind = inputKind;
            nameString = inputNameString;
            prefixString = inputPrefixString;
        }
    };

    public class SecureTransportOptions {
        public String certAuthorityFile = "";
        public String certificateFile = "";
        public String privateKeyFile = "";
    };

    public class TcpTransportOptions {
        public String serverBindPort = "7400";
        public boolean wanNetwork = false;
        public String publicAddress = "";
    };

    public class WanTransportOptions {
        public String wanServerAddress = "";
        public String wanServerPort = "3478";
        public String wanId = "";
        public boolean secureWan = false;
    };

    /**************************************************************************/
    /* PUBLIC CLASS MEMBERS */

    public TransportConfig transportConfig = null;
    public String allowInterfaces = "";
    public String verbosity = "";
    // TCP specific options
    public TcpTransportOptions tcpOptions = null;
    // Security files
    public SecureTransportOptions secureOptions = null;
    // Wan specific options
    public WanTransportOptions wanOptions = null;

    public long dataLen = 100;
    public boolean useMulticast = false;
    public boolean customMulticastAddrSet = false;

    /**************************************************************************/
    /* PRIVATE CLASS MEMBERS*/

    private static HashMap<String, TransportConfig> transportConfigMap = null;

    // Tag used when adding logging output.
    private static String classLoggingString = "PerftestTransport:";

    // Default location of the security related files
    private static String TRANSPORT_PRIVATEKEY_FILE_PUB = "./resource/secure/pubkey.pem";
    private static String TRANSPORT_PRIVATEKEY_FILE_SUB = "./resource/secure/subkey.pem";
    private static String TRANSPORT_CERTIFICATE_FILE_PUB = "./resource/secure/pub.pem";
    private static String TRANSPORT_CERTIFICATE_FILE_SUB = "./resource/secure/sub.pem";
    private static String TRANSPORT_CERTAUTHORITY_FILE = "./resource/secure/cacert.pem";

    private static HashMap<String, String> multicastAddrMap = new HashMap<String, String>();

    /**************************************************************************/
    /* CLASS CONSTRUCTOR AND DESTRUCTOR */

    public PerftestTransport() {

        transportConfig = new TransportConfig();
        tcpOptions = new TcpTransportOptions();
        secureOptions = new SecureTransportOptions();
        wanOptions = new WanTransportOptions();

        multicastAddrMap.put(LATENCY_TOPIC_NAME.VALUE, "239.255.1.1");
        multicastAddrMap.put(ANNOUNCEMENT_TOPIC_NAME.VALUE, "239.255.1.2");
        multicastAddrMap.put(THROUGHPUT_TOPIC_NAME.VALUE, "239.255.1.100");

    }

    /**************************************************************************/

    public static HashMap<String, Integer> getTransportCmdLineArgs() {

        HashMap<String, Integer> cmdLineArgsMap = new HashMap<String, Integer>();

        cmdLineArgsMap.put("-transport", new Integer(1));
        cmdLineArgsMap.put("-enableTCP", new Integer(0));
        cmdLineArgsMap.put("-enableUDPv6", new Integer(0));
        cmdLineArgsMap.put("-enableSharedMemory", new Integer(0));
        cmdLineArgsMap.put("-nic", new Integer(1));
        cmdLineArgsMap.put("-allowInterfaces", new Integer(1));
        cmdLineArgsMap.put("-transportServerBindPort", new Integer(1));
        cmdLineArgsMap.put("-transportVerbosity", new Integer(1));
        cmdLineArgsMap.put("-transportWan", new Integer(0));
        cmdLineArgsMap.put("-transportPublicAddress", new Integer(1));
        cmdLineArgsMap.put("-transportCertAuthority", new Integer(1));
        cmdLineArgsMap.put("-transportCertFile", new Integer(1));
        cmdLineArgsMap.put("-transportPrivateKey", new Integer(1));
        cmdLineArgsMap.put("-transportWanServerAddress", new Integer(1));
        cmdLineArgsMap.put("-transportWanServerPort", new Integer(1));
        cmdLineArgsMap.put("-transportWanId", new Integer(1));
        cmdLineArgsMap.put("-transportSecureWan", new Integer(0));
        cmdLineArgsMap.put("-multicast", new Integer(0));
        cmdLineArgsMap.put("-multicastAddr", new Integer(1));
        cmdLineArgsMap.put("-nomulticast", new Integer(0));

        return cmdLineArgsMap;
    }

    public String helpMessageString() {

    StringBuilder sb = new StringBuilder();
    sb.append("\t===================== Transport Specific Options ======================\n");
    sb.append("\n");
    sb.append("\t-transport <kind>             - Set transport to be used. The rest of\n");
    sb.append("\t                                the transports will be disabled.\n");
    sb.append("\t                                Values:\n");
    sb.append("\t                                    UDPv4\n");
    sb.append("\t                                    UDPv6\n");
    sb.append("\t                                    SHMEM\n");
    sb.append("\t                                    TCP\n");
    sb.append("\t                                    TLS\n");
    sb.append("\t                                    DTLS\n");
    sb.append("\t                                    WAN\n");
    sb.append("\t                                    Use XML\n");
    sb.append("\t                                Default: Use XML (UDPv4|SHMEM).\n");
    sb.append("\t-nic/-allowInterfaces <addr>  - Use only the NIC specified by <ipaddr> to\n");
    sb.append("\t                                receive packets. This will be the only\n");
    sb.append("\t                                address announced at discovery time.\n");
    sb.append("\t                                If not specified, use all available\n");
    sb.append("\t                                interfaces\n");
    sb.append("\t-multicast                    - Use multicast to send data. Each topic");
    sb.append("\t                                will use a different address:\n");
    sb.append("\t                                <address> is optional, if unspecified:\n");
    for (Map.Entry<String, String> map : multicastAddrMap.entrySet()) {
        sb.append("                                            ");
        sb.append(map.getKey()).append(" ").append(map.getValue()).append("\n");
    }
    sb.append("\t-multicastAddr <address>      - Use multicast to send data and set\n");
    sb.append("\t                                the input <address>|<addr,addr,addr>\n");
    sb.append("\t                                as the multicast addresses for the\n");
    sb.append("\t                                three topics in the application.\n");
    sb.append("\t                                If only one address is provided, that\n");
    sb.append("\t                                one and the 2 consecutive ones will be\n");
    sb.append("\t                                used for the 3 topics used by Perftest.\n");
    sb.append("\t                                If one address is set, this one must be\n");
    sb.append("\t                                in multicast range and lower than\n");
    sb.append("\t                                239.255.255.253 or the equivalent on IPv6\n");
    sb.append("\t-transportVerbosity <level>   - Verbosity of the transport\n");
    sb.append("\t                                Default: 0 (errors only)\n");
    sb.append("\t-transportServerBindPort <p>  - Port used by the transport to accept\n");
    sb.append("\t                                TCP/TLS connections <optional>\n");
    sb.append("\t                                Default: 7400\n");
    sb.append("\t-transportWan                   Use TCP/TLS across LANs and Firewalls.\n");
    sb.append("\t                                Default: Not Set, LAN mode.\n");
    sb.append("\t-transportPublicAddress <ip>  - Public IP address and port (WAN address\n");
    sb.append("\t                                and port) (separated with ‘:’ ) related\n");
    sb.append("\t                                to the transport instantiation. This is\n");
    sb.append("\t                                required when using server mode.\n");
    sb.append("\t                                Default: Not Set.\n");
    sb.append("\t-transportWanServerAddress <a>- Address where to find the WAN Server\n");
    sb.append("\t                                Default: Not Set (Required)\n");
    sb.append("\t-transportWanServerPort <p>     Port where to find the WAN Server.\n");
    sb.append("\t                                Default: 3478.\n");
    sb.append("\t-transportWanId <id>          - Id to be used for the WAN transport.\n");
    sb.append("\t                                Default: Not Set (Required).\n");
    sb.append("\t-transportSecureWan           - Use WAN with security.\n");
    sb.append("\t                                Default: False.\n");
    sb.append("\t-transportCertAuthority <file>- Certificate authority file <optional>\n");
    sb.append("\t                                Default: \"");
    sb.append(TRANSPORT_CERTAUTHORITY_FILE).append("\"\n");
    sb.append("\t-transportCertFile <file>     - Certificate file <optional>\n");
    sb.append("\t                                Default (Publisher): \"");
    sb.append(TRANSPORT_CERTIFICATE_FILE_PUB).append("\"\n");
    sb.append("\t                                Default (Subscriber): \"");
    sb.append(TRANSPORT_CERTIFICATE_FILE_SUB).append("\"\n");
    sb.append("\t-transportPrivateKey <file>   - Private key file <optional>\n");
    sb.append("\t                                Default (Publisher): \"");
    sb.append(TRANSPORT_PRIVATEKEY_FILE_PUB).append("\"\n");
    sb.append("\t                                Default (Subscriber): \"");
    sb.append(TRANSPORT_PRIVATEKEY_FILE_SUB).append("\"\n");

        return sb.toString();
    }

    public String printTransportConfigurationSummary() {

        StringBuilder sb = new StringBuilder("Transport Information:\n");
        sb.append("\tKind: ").append(transportConfig.nameString).append("\n");

        if (!allowInterfaces.isEmpty()) {
            sb.append("\tNic: ").append(allowInterfaces).append("\n");
        }

        sb.append( "\tUse Multicast: ");
        sb.append((allowsMulticast() && useMulticast) ? "True" : "False");
        if (!allowsMulticast() && useMulticast) {
            sb.append (" (Multicast is not supported for ");
            sb.append( transportConfig.nameString );
            sb.append(")");
        }
        sb.append( "\n");

        if (customMulticastAddrSet) {
            sb.append( "\tUsing custom Multicast Addresses:");
            sb.append("\n\t\tThroughtput Address: ");
            sb.append(getMulticastAddr(THROUGHPUT_TOPIC_NAME.VALUE));
            sb.append("\n\t\tLatency Address: ");
            sb.append(getMulticastAddr(LATENCY_TOPIC_NAME.VALUE));
            sb.append("\n\t\tAnnouncement Address: ");
            sb.append(getMulticastAddr(ANNOUNCEMENT_TOPIC_NAME.VALUE));
            sb.append("\n");
        }

        if (transportConfig.kind == Transport.TRANSPORT_TCPv4
                || transportConfig.kind == Transport.TRANSPORT_TLSv4) {

            sb.append("\tTCP Server Bind Port: ").append(tcpOptions.serverBindPort).append("\n");
            sb.append("\tTCP LAN/WAN mode: ");
            if (tcpOptions.wanNetwork) {
                sb.append("WAN\n");
            } else {
                sb.append("LAN\n");
            }
            if (tcpOptions.wanNetwork) {
                sb.append("\tTCP Public Address: ").append(tcpOptions.publicAddress).append("\n");
            }
        }

        if (transportConfig.kind == Transport.TRANSPORT_WANv4) {

            sb.append("\tWAN Server Address: ");
            sb.append(wanOptions.wanServerAddress).append(":");
            sb.append(wanOptions.wanServerPort).append("\n");

            sb.append("\tWAN Id: ").append(wanOptions.wanId).append("\n");

            sb.append("\tWAN Secure: ").append(wanOptions.secureWan).append("\n");
        }

        if (transportConfig.kind == Transport.TRANSPORT_TLSv4
                || transportConfig.kind == Transport.TRANSPORT_DTLSv4
                || (transportConfig.kind == Transport.TRANSPORT_WANv4 && wanOptions.secureWan)) {

            sb.append("\tCertificate authority file: ").append(secureOptions.certAuthorityFile).append("\n");
            sb.append("\tCertificate file: ").append(secureOptions.certificateFile).append("\n");
            sb.append("\tPrivate key file: ").append(secureOptions.privateKeyFile).append("\n");
        }

        if (!verbosity.isEmpty()) {
            sb.append("\tVerbosity: ").append(verbosity).append("\n");
        }

        return sb.toString();
    }

    public boolean allowsMulticast() {
        return (transportConfig.kind != Transport.TRANSPORT_TCPv4
                && transportConfig.kind != Transport.TRANSPORT_TLSv4
                && transportConfig.kind != Transport.TRANSPORT_WANv4
                && transportConfig.kind != Transport.TRANSPORT_SHMEM);
    }

    public boolean parseTransportOptions(int argc, String[] argv) {

        boolean isPublisher = false;
        String transportString = "Default";

        for (int i = 0; i < argc; ++i) {

            if ("-pub".toLowerCase().startsWith(argv[i].toLowerCase())) {
                isPublisher = true;
            }
            else if ("-dataLen".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <length> after -dataLen");
                    return false;
                }
                try {
                    dataLen = Long.parseLong(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad dataLen\n");
                    return false;
                }

            } else if ("-transport".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <kind> after -transport");
                    return false;
                }
                transportString = argv[i];

            } else if ("-enableTCP".toLowerCase().startsWith(argv[i].toLowerCase())) {

                // Legacy option
                transportString = "TCP";

            } else if ("-enableUDPv6".toLowerCase().startsWith(argv[i].toLowerCase())) {

                // Legacy option
                transportString = "UDPv6";

            } else if ("-enableSharedMemory".toLowerCase().startsWith(argv[i].toLowerCase())) {

                // Legacy option
                transportString = "SHMEM";

            } else if ("-nic".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <address> after -nic");
                    return false;
                }
                allowInterfaces = argv[i];

            } else if ("-allowInterfaces".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <address> after -allowInterfaces");
                    return false;
                }
                allowInterfaces = argv[i];

            } else if ("-transportVerbosity".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <level> after -transportVerbosity");
                    return false;
                }
                verbosity = argv[i];

            } else if ("-transportServerBindPort".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <number> after -transportServerBindPort");
                    return false;
                }
                tcpOptions.serverBindPort = argv[i];

            } else if ("-transportWan".toLowerCase().startsWith(argv[i].toLowerCase())) {

                tcpOptions.wanNetwork = true;

            } else if ("-transportPublicAddress".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <address> after -transportPublicAddress");
                    return false;
                }
                tcpOptions.publicAddress = argv[i];

            } else if ("-transportPublicAddress".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <address> after -transportPublicAddress");
                    return false;
                }
                tcpOptions.publicAddress = argv[i];

            } else if ("-transportCertAuthority".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing Missing <path> after -transportCertAuthority");
                    return false;
                }
                secureOptions.certAuthorityFile = argv[i];

            } else if ("-transportCertFile".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <path> after -transportCertFile");
                    return false;
                }
                secureOptions.certificateFile = argv[i];

            } else if ("-transportPrivateKey".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <path> after -transportPrivateKey");
                    return false;
                }
                secureOptions.privateKeyFile = argv[i];

            } else if ("-transportWanServerAddress".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <address> after -transportWanServerAddress");
                    return false;
                }
                wanOptions.wanServerAddress = argv[i];

            } else if ("-transportWanServerPort".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <port> after -transportWanServerPort");
                    return false;
                }
                wanOptions.wanServerPort = argv[i];

            } else if ("-transportWanId".toLowerCase().startsWith(argv[i].toLowerCase())) {

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <id> after -transportWanId");
                    return false;
                }
                wanOptions.wanId = argv[i];

            } else if ("-transportSecureWan".toLowerCase().startsWith(argv[i].toLowerCase())) {

                wanOptions.secureWan = true;
            } else if ("-nomulticast".toLowerCase().startsWith(argv[i].toLowerCase())) {
                useMulticast = false;
            } else if ("-multicast".toLowerCase().startsWith(argv[i].toLowerCase())) {
                useMulticast = true;
            }
            else if ("-multicastAddr".toLowerCase().startsWith(argv[i].toLowerCase())) {
                useMulticast = true;
                customMulticastAddrSet = true;
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.println(classLoggingString
                            + " Missing <address> after -multicastAddr");
                    return false;
                }
                if (!parseMulticastAddresses(argv[i])) {
                    System.err.println(classLoggingString
                            + " Error parsing -multicastAddr");
                    return false;
                }

            }
        }

        if (!setTransport(transportString)) {
            System.err.println(classLoggingString + " Error Setting the transport");
            return false;
        }

        // We only need to set the secure properties for this
        if (transportConfig.kind == Transport.TRANSPORT_TLSv4
                || transportConfig.kind == Transport.TRANSPORT_DTLSv4
                || transportConfig.kind == Transport.TRANSPORT_WANv4) {

            populateSecurityFiles(isPublisher);
        }
        return true;
    }

    /**************************************************************************/

    private HashMap<String, TransportConfig> getTransportConfigMap() {
        if (transportConfigMap == null) {

            // First, we create the Map
            transportConfigMap = new HashMap<String, TransportConfig>();

            transportConfigMap.put("Default", new TransportConfig(
                    Transport.TRANSPORT_NOT_SET,
                    "--",
                    "--"));
            transportConfigMap.put("UDPv4", new TransportConfig(
                    Transport.TRANSPORT_UDPv4,
                    "UDPv4",
                    "dds.transport.UDPv4.builtin"));
            transportConfigMap.put("UDPv6", new TransportConfig(
                    Transport.TRANSPORT_UDPv6,
                    "UDPv6",
                    "dds.transport.UDPv6.builtin"));
            transportConfigMap.put("TCP", new TransportConfig(
                    Transport.TRANSPORT_TCPv4,
                    "TCP",
                    "dds.transport.TCPv4.tcp1"));
            transportConfigMap.put("TLS", new TransportConfig(
                    Transport.TRANSPORT_TLSv4,
                    "TLS",
                    "dds.transport.TCPv4.tcp1"));
            transportConfigMap.put("DTLS", new TransportConfig(
                    Transport.TRANSPORT_DTLSv4,
                    "DTLS",
                    "dds.transport.DTLS.dtls1"));
            transportConfigMap.put("WAN", new TransportConfig(
                    Transport.TRANSPORT_WANv4,
                    "WAN",
                    "dds.transport.WAN.wan1"));
            transportConfigMap.put("SHMEM", new TransportConfig(
                    Transport.TRANSPORT_SHMEM,
                    "SHMEM",
                    "dds.transport.shmem.builtin"));
        }
        return transportConfigMap;
    }

    private boolean setTransport(String transportString) {

        HashMap<String, TransportConfig> configMap = getTransportConfigMap();

        transportConfig = configMap.get(transportString);

        if (transportConfig == null) {
            System.err.println(classLoggingString + " \""
                    + transportString + "\" is not a valid transport. "
                    + "List of supported transport:");
            for (String key : configMap.keySet() ) {
                System.err.println("\t\"" + key + "\"");
            }
            return false;
        }

        return true;
    }

    private void populateSecurityFiles(boolean isPublisher) {

        if (secureOptions.certificateFile.isEmpty()) {
            if (isPublisher) {
                secureOptions.certificateFile = TRANSPORT_CERTIFICATE_FILE_PUB;
            } else {
                secureOptions.certificateFile = TRANSPORT_CERTIFICATE_FILE_SUB;
            }
        }

        if (secureOptions.privateKeyFile.isEmpty()) {
            if (isPublisher) {
                secureOptions.privateKeyFile = TRANSPORT_PRIVATEKEY_FILE_PUB;
            } else {
                secureOptions.privateKeyFile = TRANSPORT_PRIVATEKEY_FILE_SUB;
            }
        }

        if (secureOptions.certAuthorityFile.isEmpty()) {
            secureOptions.certAuthorityFile = TRANSPORT_CERTAUTHORITY_FILE;
        }
    }

    /**************************************************************************/

    private void setAllowInterfacesList(DomainParticipantQos qos) {

        if (!allowInterfaces.isEmpty()) {

            if (transportConfig.kind == Transport.TRANSPORT_NOT_SET) {
                System.err.println(classLoggingString
                        + " Ignoring -nic/-allowInterfaces option.");
                return;
            }

            if (transportConfig.kind == Transport.TRANSPORT_UDPv4_UDPv6_SHMEM
                    || transportConfig.kind == Transport.TRANSPORT_UDPv4_UDPv6) {

                String propertyName =
                        "dds.transport.UDPv4.builtin.parent.allow_interfaces";

                PropertyQosPolicyHelper.add_property(
                    qos.property,
                    propertyName,
                    allowInterfaces,
                    false);

                propertyName =
                        "dds.transport.UDPv6.builtin.parent.allow_interfaces";

                PropertyQosPolicyHelper.add_property(
                        qos.property,
                        propertyName,
                        allowInterfaces,
                        false);

            } else {

                String propertyName = transportConfig.prefixString;

                if (transportConfig.kind == Transport.TRANSPORT_WANv4) {
                    propertyName += ".parent";
                }

                propertyName += ".parent.allow_interfaces";

                PropertyQosPolicyHelper.add_property(
                        qos.property,
                        propertyName,
                        allowInterfaces,
                        false);
            }
        }
    }

    private void setTransportVerbosity(DomainParticipantQos qos) {

        if (!verbosity.isEmpty()) {

            /*
             * By default, if the transport is not set, it should be UDPv4,
             * if it is not it means that we have modified the QOS, so we won't
             * use the -transportVerbosity param.
             */
            if (transportConfig.kind == Transport.TRANSPORT_NOT_SET) {
                System.err.println(classLoggingString
                        + " Ignoring -transportVerbosity option.");
                return;
            }

            String propertyName = transportConfig.prefixString + ".verbosity";

            // The name of the property in TCPv4 is different
            if (transportConfig.kind == Transport.TRANSPORT_TCPv4) {
                propertyName = transportConfig.prefixString + ".logging_verbosity_bitmap";
            } else if (transportConfig.kind == Transport.TRANSPORT_UDPv4
                    || transportConfig.kind == Transport.TRANSPORT_UDPv6
                    || transportConfig.kind == Transport.TRANSPORT_SHMEM
                    || transportConfig.kind == Transport.TRANSPORT_UDPv4_SHMEM
                    || transportConfig.kind == Transport.TRANSPORT_UDPv6_SHMEM
                    || transportConfig.kind == Transport.TRANSPORT_UDPv4_UDPv6
                    || transportConfig.kind == Transport.TRANSPORT_UDPv4_UDPv6_SHMEM) {
                // We do not change logging for the builtin transports.
                return;
            }

            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    propertyName,
                    verbosity,
                    false);
        }
    }

    private void configureSecurityFiles(DomainParticipantQos qos) {

        if (!secureOptions.certAuthorityFile.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".tls.verify.ca_file",
                    secureOptions.certAuthorityFile,
                    false);
        }

        if (!secureOptions.certificateFile.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".tls.identity.certificate_chain_file",
                    secureOptions.certificateFile,
                    false);
        }

        if (!secureOptions.privateKeyFile.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".tls.identity.private_key_file",
                    secureOptions.privateKeyFile,
                    false);
        }
    }

    private boolean configureTcpTransport(DomainParticipantQos qos) {

        qos.transport_builtin.mask = TransportBuiltinKind.MASK_NONE;

        PropertyQosPolicyHelper.add_property(
                qos.property,
                "dds.transport.load_plugins",
                transportConfig.prefixString,
                false);

        if (!tcpOptions.serverBindPort.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".server_bind_port",
                    tcpOptions.serverBindPort,
                    false);
        }

        if (tcpOptions.wanNetwork) {
            PropertyQosPolicyHelper.assert_property(
                    qos.property,
                    transportConfig.prefixString + ".parent.classid",
                    "NDDS_TRANSPORT_CLASSID_TCPV4_WAN",
                    false);

            if (!tcpOptions.serverBindPort.equals("0")) {
                if (!tcpOptions.publicAddress.isEmpty()) {

                    PropertyQosPolicyHelper.add_property(
                            qos.property,
                            transportConfig.prefixString + ".public_address",
                            tcpOptions.publicAddress,
                            false);
                } else {
                    System.err.println(classLoggingString
                            + " Public Address is required if Server Bind Port != 0");
                    return false;
                }
            }
        }

        if (transportConfig.kind == Transport.TRANSPORT_TLSv4) {

            if (tcpOptions.wanNetwork) {
                PropertyQosPolicyHelper.assert_property(
                        qos.property,
                        transportConfig.prefixString + ".parent.classid",
                        "NDDS_TRANSPORT_CLASSID_TLSV4_WAN",
                        false);
            } else {
                PropertyQosPolicyHelper.assert_property(
                        qos.property,
                        transportConfig.prefixString + ".parent.classid",
                        "NDDS_TRANSPORT_CLASSID_TLSV4_LAN",
                        false);
            }

            configureSecurityFiles(qos);
        }

        return true;
    }

    private void configureDtlsTransport(DomainParticipantQos qos) {

        qos.transport_builtin.mask = TransportBuiltinKind.MASK_NONE;

        PropertyQosPolicyHelper.add_property(
                qos.property,
                "dds.transport.load_plugins",
                transportConfig.prefixString,
                false);

        PropertyQosPolicyHelper.add_property(
                qos.property,
                transportConfig.prefixString + ".library",
                "nddstransporttls",
                false);

        PropertyQosPolicyHelper.add_property(
                qos.property,
                transportConfig.prefixString + ".create_function",
                "NDDS_Transport_DTLS_create",
                false);

        configureSecurityFiles(qos);
    }

    private boolean configureWanTransport(DomainParticipantQos qos) {

        qos.transport_builtin.mask = TransportBuiltinKind.MASK_NONE;

        PropertyQosPolicyHelper.add_property(
                qos.property,
                "dds.transport.load_plugins",
                transportConfig.prefixString,
                false);

        PropertyQosPolicyHelper.add_property(
                qos.property,
                transportConfig.prefixString + ".library",
                "nddstransportwan",
                false);

        PropertyQosPolicyHelper.add_property(
                qos.property,
                transportConfig.prefixString + ".create_function",
                "NDDS_Transport_WAN_create",
                false);

        if (!wanOptions.wanServerAddress.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".server",
                    wanOptions.wanServerAddress,
                    false);
        } else {
            System.err.println(classLoggingString
                    + " Wan Server Address is required");
            return false;
        }

        if (!wanOptions.wanServerPort.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".server_port",
                    wanOptions.wanServerPort,
                    false);
        }

        if (!wanOptions.wanId.isEmpty()) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".transport_instance_id",
                    wanOptions.wanId,
                    false);
        } else {
            System.err.println(classLoggingString
                    + " Wan ID is required");
            return false;
        }

        if (wanOptions.secureWan) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    transportConfig.prefixString + ".enable_security",
                    "1",
                    false);

            configureSecurityFiles(qos);
        }

        return true;
    }

    private void configureShmemTransport(DomainParticipantQos qos) {

        // Number of messages that can be buffered in the receive queue.
        int receivedMessageCountMax = 2 * 1024 * 1024 / (int) dataLen;
        if (receivedMessageCountMax < 1) {
            receivedMessageCountMax = 1;
        }

        PropertyQosPolicyHelper.add_property(
                qos.property,
                "dds.transport.shmem.builtin.received_message_count_max",
                String.valueOf(receivedMessageCountMax),
                false);
    }

    public boolean configureTransport(DomainParticipantQos qos) {

        /*
        * If transportConfig.kind is not set, then we want to use the value
        * provided by the Participant Qos, so first we read it from there and
        * update the value of transportConfig.kind with whatever was already set.
        */
        if (transportConfig.kind == Transport.TRANSPORT_NOT_SET) {

            int mask = qos.transport_builtin.mask;
            switch (mask) {
                case TransportBuiltinKind.UDPv4:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_UDPv4,
                            "UDPv4",
                            "dds.transport.UDPv4.builtin");
                    break;
                case TransportBuiltinKind.UDPv6:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_UDPv6,
                            "UDPv6",
                            "dds.transport.UDPv6.builtin");
                    break;
                case TransportBuiltinKind.SHMEM:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_SHMEM,
                            "SHMEM",
                            "dds.transport.shmem.builtin");
                    break;
                case TransportBuiltinKind.SHMEM|TransportBuiltinKind.UDPv4:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_UDPv4_SHMEM,
                            "UDPv4 & SHMEM",
                            "dds.transport.UDPv4.builtin");
                    break;
                case TransportBuiltinKind.UDPv6|TransportBuiltinKind.UDPv4:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_UDPv4_UDPv6,
                            "UDPv4 & UDPv6",
                            "dds.transport.UDPv4.builtin");
                    break;
                case TransportBuiltinKind.UDPv6|TransportBuiltinKind.SHMEM:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_UDPv6_SHMEM,
                            "UDPv6 & SHMEM",
                            "dds.transport.UDPv6.builtin");
                    break;
                case TransportBuiltinKind.UDPv4|TransportBuiltinKind.UDPv6|TransportBuiltinKind.SHMEM:
                    transportConfig = new TransportConfig(
                            Transport.TRANSPORT_UDPv4_UDPv6_SHMEM,
                            "UDPv4 & UDPv6 & SHMEM",
                            "dds.transport.UDPv4.builtin");
                    break;
                default:
                    /*
                     * This would mean that the mask is either empty or a
                     * different value that we do not support yet. So we keep
                     * the value as "TRANSPORT_NOT_SET"
                     */
                    break;
            }
            transportConfig.takenFromQoS = true;
        }

        switch (transportConfig.kind) {

            case TRANSPORT_UDPv4:
                qos.transport_builtin.mask = TransportBuiltinKind.UDPv4;
                break;

            case TRANSPORT_UDPv6:
                qos.transport_builtin.mask = TransportBuiltinKind.UDPv6;
                break;

            case TRANSPORT_SHMEM:
                qos.transport_builtin.mask = TransportBuiltinKind.SHMEM;
                configureShmemTransport(qos);
                break;

            case TRANSPORT_TCPv4:
                if (!configureTcpTransport(qos)) {
                    System.err.println(classLoggingString
                            + " Failed to configure TCP plugin");
                    return false;
                }
                break;

            case TRANSPORT_TLSv4:
                if (!configureTcpTransport(qos)) {
                    System.err.println(classLoggingString
                            + " Failed to configure TCP - TLS plugin");
                    return false;
                }
                break;

            case TRANSPORT_DTLSv4:
                configureDtlsTransport(qos);
                break;

            case TRANSPORT_WANv4:
                if (!configureWanTransport(qos)) {
                    System.err.println(classLoggingString
                            + " Failed to configure WAN plugin");
                    return false;
                }
                break;

            default:
                /*
                * If shared memory is enabled we want to set up its
                * specific configuration
                */
                if ((qos.transport_builtin.mask & TransportBuiltinKind.SHMEM)
                        != 0) {
                    configureShmemTransport(qos);
                }
                break;
        } // Switch

        if (transportConfig.kind != Transport.TRANSPORT_NOT_SET
                && transportConfig.kind != Transport.TRANSPORT_SHMEM) {
            setAllowInterfacesList(qos);
        } else {
            allowInterfaces = "";
        }

        setTransportVerbosity(qos);

        return true;
    }

    public String getMulticastAddr(String topicName)
    {
        //get() function return null if the map contains no mapping for the key
        if (multicastAddrMap.get(topicName) == null) {
            return null;
        }
        return multicastAddrMap.get(topicName).toString();
    }

    public boolean isMulticast(String address){

        InetAddress addr;
        try {
            addr = InetAddress.getByName(address);
        } catch (Exception e) {
            System.err.println(classLoggingString + " Error parsing address."
                    + " Exception: " + e.getMessage());
            return false;
        }

        return addr.isMulticastAddress();
    }

    public String increaseAddressByOne(String addr) {
        boolean success = false;
        String nextAddr;
        byte[] buffer;

        try {
            buffer = InetAddress.getByName(addr).getAddress();
        } catch (Exception e) {
            System.err.println(classLoggingString
                    + " Error parsing address." + " Exception: " + e.getMessage());
            return null;
        }

        /*
        * Increase the full address by one value.
        * if the Address is 255.255.255.255 (or the equivalent for IPv6) this
        * function will FAIL
        */
        for (int i = buffer.length - 1; i >= 0 && !success; i--) {
            if (buffer[i] == (byte) 255) {
                buffer[i] = 0;
            } else {
                /* Increase the value and exit */
                buffer[i]++;
                success = true;
            }
        }

        if (!success) {
            System.err.println(classLoggingString
                    + " IP value too high. Please use -help for more information"
                    + " about -multicastAddr command line\n");
            return null;
        }

        /* Get the string format of the address */
        try {
            nextAddr = InetAddress.getByAddress(buffer).getHostAddress();
        } catch (Exception e) {
            System.err.println(classLoggingString
                    + " Error recovering address from byte format");
            return null;
        }

        return nextAddr;
    }

    private boolean parseMulticastAddresses(String arg) {

        /*
         * Split the string into diferents parts delimited with ',' character.
         * With a "a,b,c" input this will result in three diferent addresses
         * "a","b" and "c"
         */
        String [] addresses = arg.split(",",0);

        /* If three addresses are given */
        if (addresses.length == 3) {

            multicastAddrMap.put(THROUGHPUT_TOPIC_NAME.VALUE, addresses[0]);
            multicastAddrMap.put(LATENCY_TOPIC_NAME.VALUE, addresses[1]);
            multicastAddrMap.put(ANNOUNCEMENT_TOPIC_NAME.VALUE, addresses[2]);

        } else if (addresses.length == 1) {
            /* If only one address is given */
            String aux = new String();
            multicastAddrMap.put(THROUGHPUT_TOPIC_NAME.VALUE, addresses[0]);

            /* Calculate the consecutive one */
            aux = increaseAddressByOne(addresses[0]);
            if (aux == null) {
                System.err.println(classLoggingString
                        + " Fail to increase the value of IP addres given");
                return false;
            }
            multicastAddrMap.put(LATENCY_TOPIC_NAME.VALUE, aux);

            /* Calculate the consecutive one */
            aux = increaseAddressByOne(aux);
            if (aux == null) {
                System.err.println(classLoggingString
                        + " Fail to increase the value of IP addres given");
                return false;
            }
            multicastAddrMap.put(ANNOUNCEMENT_TOPIC_NAME.VALUE, aux);

        } else {
            System.err.println(classLoggingString
                    + " Error parsing Address/es '" + arg
                    + "' for -multicastAddr option\n"
                    + "Use -help option to see the correct sintax");
            return false;
        }
        /* Last check. All the IPs must be in IP format and multicast range */
        if (!isMulticast(multicastAddrMap.get(THROUGHPUT_TOPIC_NAME.VALUE).toString())
                || !isMulticast(multicastAddrMap.get(LATENCY_TOPIC_NAME.VALUE).toString())
                || !isMulticast(multicastAddrMap.get(ANNOUNCEMENT_TOPIC_NAME.VALUE).toString())) {
            System.err.println(classLoggingString + " Error parsing Address/es '"
                    + arg + "' for -multicastAddr option\n"
                    + "\tUse -help option to see the correct sintax");
            return false;
        }

        return true;
    }

}
//===========================================================================
