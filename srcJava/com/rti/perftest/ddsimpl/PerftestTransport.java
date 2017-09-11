/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.nio.file.WatchEvent.Kind;
import java.util.HashMap;
import com.rti.dds.infrastructure.PropertyQosPolicyHelper;
import com.rti.dds.infrastructure.TransportBuiltinKind;
import com.rti.dds.domain.DomainParticipantQos;

public class PerftestTransport {

    /* Internal classes and enums */
    
    public enum Transport {
        TRANSPORT_DEFAULT,
        TRANSPORT_UDPv4,
        TRANSPORT_UDPv6,
        TRANSPORT_TCPv4,
        TRANSPORT_TLSv4,
        TRANSPORT_DTLSv4,
        TRANSPORT_WANv4,
        TRANSPORT_SHMEM
    };

    public class TransportConfig {

        public Transport kind = Transport.TRANSPORT_DEFAULT;
        public String nameString = "";
        public String prefixString = "";

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

    /**************************************************************************/
    /* CLASS CONSTRUCTOR AND DESTRUCTOR */

    public PerftestTransport() {

        transportConfig = new TransportConfig();
        tcpOptions = new TcpTransportOptions();
        secureOptions = new SecureTransportOptions();
        wanOptions = new WanTransportOptions();
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
    sb.append("\t                                Default: UDPv4\n");
    sb.append("\t-nic <ipaddr>                 - Use only the nic specified by <ipaddr>.\n");
    sb.append("\t                                If not specified, use all available\n");
    sb.append("\t                                interfaces\n");
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

    public void printTransportConfigurationSummary() {

        StringBuilder sb = new StringBuilder("Transport Information:\n");
        sb.append("\tKind: ").append(transportConfig.nameString).append("\n");

        if (!allowInterfaces.isEmpty()) {
            sb.append("\tNic: ").append(allowInterfaces).append("\n");
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

        System.err.print(sb.toString());
    }

    public boolean allowsMulticast() {
        if (transportConfig.kind != Transport.TRANSPORT_TCPv4
                && transportConfig.kind != Transport.TRANSPORT_TLSv4
                && transportConfig.kind != Transport.TRANSPORT_WANv4
                && transportConfig.kind != Transport.TRANSPORT_SHMEM) {
            return true;
        } else {
            return false;
        }
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
                    Transport.TRANSPORT_DEFAULT,
                    "Default (UDPv4) / Custom (Taken from QoS profile)",
                    "dds.transport.UDPv4.builtin"));
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
            /*
             * By default, if the transport is not set, it should be UDPv4, if it is not
             * It means that we have modified the QOS, so we won't use the -nic param.
             */
            if (transportConfig.kind == Transport.TRANSPORT_DEFAULT
                    && qos.transport_builtin.mask != TransportBuiltinKind.UDPv4) {
                System.err.println(classLoggingString
                        + " Ignoring -nic option, Transport has been modified via QoS");
                return;
            }

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

    private void setTransportVerbosity(DomainParticipantQos qos) {

        if (!verbosity.isEmpty()) {

            /*
             * By default, if the transport is not set, it should be UDPv4,
             * if it is not it means that we have modified the QOS, so we won't
             * use the -transportVerbosity param.
             */
            if (transportConfig.kind == Transport.TRANSPORT_DEFAULT
                    && qos.transport_builtin.mask != TransportBuiltinKind.UDPv4) {
                System.err.println(classLoggingString
                        + " Ignoring -transportVerbosity option, Transport has been modified via QoS");
                return;
            }

            String propertyName = transportConfig.prefixString + ".verbosity";

            // The name of the property in TCPv4 is different
            if (transportConfig.kind == Transport.TRANSPORT_TCPv4) {
                propertyName = transportConfig.prefixString + ".logging_verbosity_bitmap";
            } else if (transportConfig.kind == Transport.TRANSPORT_UDPv4
                    || transportConfig.kind == Transport.TRANSPORT_UDPv6
                    || transportConfig.kind == Transport.TRANSPORT_SHMEM) {
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

        qos.transport_builtin.mask = TransportBuiltinKind.SHMEM;

        // SHMEM transport properties
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

        switch (transportConfig.kind) {

        case TRANSPORT_DEFAULT:
            // We do not set any transport_builtin mask, either is UDPv4 or is something custom writen in the xml.
            break;

        case TRANSPORT_UDPv4:
            qos.transport_builtin.mask = TransportBuiltinKind.UDPv4;
            break;

        case TRANSPORT_UDPv6:
            qos.transport_builtin.mask = TransportBuiltinKind.UDPv6;
            break;

        case TRANSPORT_SHMEM:
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
            System.err.println(classLoggingString 
                    + " Transport is not supported");
            return false;

        } // Switch

        if (transportConfig.kind != Transport.TRANSPORT_SHMEM) {
            setAllowInterfacesList(qos);
        } else {
            allowInterfaces = "";
        }

        setTransportVerbosity(qos);

        return true;
    }

}
//===========================================================================
