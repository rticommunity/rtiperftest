/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.ddsimpl;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.HashMap;
import java.util.StringTokenizer;

import com.rti.dds.domain.DomainParticipant;
import com.rti.dds.domain.DomainParticipantFactory;
import com.rti.dds.domain.DomainParticipantFactoryQos;
import com.rti.dds.domain.DomainParticipantListener;
import com.rti.dds.domain.DomainParticipantQos;
import com.rti.dds.infrastructure.Duration_t;
import com.rti.dds.infrastructure.PropertyQosPolicyHelper;
import com.rti.dds.infrastructure.PublishModeQosPolicyKind;
import com.rti.dds.infrastructure.ReliabilityQosPolicyKind;
import com.rti.dds.infrastructure.ResourceLimitsQosPolicy;
import com.rti.dds.infrastructure.StatusKind;
import com.rti.dds.infrastructure.TransportBuiltinKind;
import com.rti.dds.infrastructure.TransportMulticastSettings_t;
import com.rti.dds.infrastructure.DurabilityQosPolicyKind;
import com.rti.dds.infrastructure.StringSeq;
import com.rti.dds.publication.DataWriter;
import com.rti.dds.publication.DataWriterQos;
import com.rti.dds.publication.Publisher;
import com.rti.dds.subscription.DataReader;
import com.rti.dds.subscription.DataReaderQos;
import com.rti.dds.subscription.Subscriber;
import com.rti.dds.topic.*;
import com.rti.ndds.config.LogCategory;
import com.rti.ndds.config.LogVerbosity;
import com.rti.ndds.config.Logger;
import com.rti.perftest.IMessaging;
import com.rti.perftest.IMessagingCB;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.MAX_SYNCHRONOUS_SIZE;
import com.rti.perftest.gen.MAX_BOUNDED_SEQ_SIZE;
import com.rti.perftest.gen.MAX_CFT_VALUE;
import com.rti.perftest.gen.THROUGHPUT_TOPIC_NAME;
import com.rti.perftest.gen.LATENCY_TOPIC_NAME;
import com.rti.perftest.gen.ANNOUNCEMENT_TOPIC_NAME;
import com.rti.perftest.gen.KEY_SIZE;
import com.rti.perftest.gen.DEFAULT_THROUGHPUT_BATCH_SIZE;
import com.rti.perftest.gen.MAX_PERFTEST_SAMPLE_SIZE;
import com.rti.perftest.harness.PerfTest;


// ===========================================================================

/**
 * An implementation of the IMessaging interface based on RTI Data
 * Distribution Service.
 */
public final class RTIDDSImpl<T> implements IMessaging {
    // -----------------------------------------------------------------------
    // Package Fields
    // -----------------------------------------------------------------------

    /*package*/ static int waitsetEventCount = 5;
    /*package*/ static int waitsetDelayUsec  = 100;



    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------
    private static String THROUGHPUT_MULTICAST_ADDR = "239.255.1.1";
    private static String LATENCY_MULTICAST_ADDR = "239.255.1.2";
    private static String ANNOUNCEMENT_MULTICAST_ADDR = "239.255.1.100";
    private static String PROFILE_LIBRARY_NAME = "PerftestQosLibrary";
    private static String SECUREPRIVATEKEYFILEPUB = "./resource/secure/pubkey.pem";
    private static String SECUREPRIVATEKEYFILESUB = "./resource/secure/subkey.pem";
    private static String SECURECERTIFICATEFILEPUB = "./resource/secure/pub.pem";
    private static String SECURECERTIFICATEFILESUB = "./resource/secure/sub.pem";
    private static String SECURECERTAUTHORITYFILE = "./resource/secure/cacert.pem";
    private static String SECUREPERMISIONFILEPUB = "./resource/secure/signed_PerftestPermissionsPub.xml";
    private static String SECUREPERMISIONFILESUB = "./resource/secure/signed_PerftestPermissionsSub.xml";
    private static String SECURELIBRARYNAME = "nddssecurity";

    private static final int RTIPERFTEST_MAX_PEERS = 1024;

    private int     _sendQueueSize = 50;
    private Duration_t _heartbeatPeriod = new Duration_t(0,0);
    private Duration_t _fastHeartbeatPeriod = new Duration_t (0,0);
    private long     _dataLen = 100;
    private long     _useUnbounded = 0;
    private int     _domainID = 1;
    private String  _profileFile = "perftest_qos_profiles.xml";
    private boolean _isReliable = true;
    private boolean _AutoThrottle = false;
    private boolean _TurboMode = false;
    private int     _instanceCount = 1;
    private int     _instanceMaxCountReader = -1;
    private int     _instanceHashBuckets = -1;
    private int     _durability = 0;
    private boolean _directCommunication = true;
    private int     _batchSize = DEFAULT_THROUGHPUT_BATCH_SIZE.VALUE;
    private int     _keepDurationUsec = -1;
    private boolean _usePositiveAcks = true;
    private boolean _isDebug = false;
    private boolean _latencyTest = false;
    private boolean _isLargeData = false;
    private boolean _isScan = false;
    private boolean _isPublisher = false;
    private boolean _IsAsynchronous = false;
    private boolean _isDynamicData = false;
    private String  _FlowControllerCustom = "default";
    String[] valid_flow_controller = {"default", "1Gbps", "10Gbps"};
    private int     _peer_host_count = 0;
    private String[] _peer_host = new String[RTIPERFTEST_MAX_PEERS];
    private boolean _useCft = false;
    private int     _instancesToBeWritten = -1;
    private int[]   _CFTRange = {0, 0};

    private PerftestTransport _transport;


    private boolean _secureUseSecure = false;
    private boolean _secureIsSigned = false;
    private boolean _secureIsDataEncrypted = false; // User Data
    private boolean _secureIsSMEncrypted = false; // Sub-message
    private boolean _secureIsDiscoveryEncrypted = false;
    private String _secureCertAuthorityFile = null;
    private String _secureCertificateFile = null;
    private String _securePrivateKeyFile = null;

    /*
     * if _GovernanceFile is specified, overrides the options for
     * signing, encrypting, and authentication.
     */
    private String _governanceFile = null;
    private String _securePermissionsFile = null;
    private String _secureLibrary = null;
    private int _secyreDebugLevel = -1;

    private DomainParticipantFactory _factory = null;
    private DomainParticipant        _participant = null;
    private Subscriber               _subscriber = null;
    private Publisher                _publisher = null;
    private DataReader               _reader = null;
    private String _typename = null;

    private TypeHelper<T> _myDataType = null;
    private RTIDDSLoggerDevice _loggerDevice = null;

    private static HashMap<String, String> _qoSProfileNameMap = new HashMap<String, String>();

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- From IMessaging: --------------------------------------------------

    public RTIDDSImpl(TypeHelper<T> typeHelper) {
        _myDataType = typeHelper;
        _transport = new PerftestTransport();

        _qoSProfileNameMap.put(LATENCY_TOPIC_NAME.VALUE, "LatencyQos");
        _qoSProfileNameMap.put(ANNOUNCEMENT_TOPIC_NAME.VALUE, "AnnouncementQos");
        _qoSProfileNameMap.put(THROUGHPUT_TOPIC_NAME.VALUE, "ThroughputQos");
    }

    public void dispose() {
        shutdown();
    }

    public void shutdown() {
        if (_participant != null) {
            // This synchronized block just protects this method from itself
            synchronized (_participant) {
                try {
                    Thread.sleep(2000);
                } catch (InterruptedException interrupted) {
                }

                if (_reader != null) {
                    _subscriber.delete_datareader(_reader);
                }

                try {
                    Thread.sleep(4000);
                } catch (InterruptedException interrupted) {
                }

                _participant.delete_contained_entities();
                DomainParticipantFactory.get_instance().delete_participant(
                        _participant);
                _participant = null;
            }
        }
        // Unregister _loggerDevice
        try {
            Logger.get_instance().set_output_device(null);
        } catch (Exception e) {
            System.err.print("Failed set_output_device for Logger.\n");
        }
    }

    public int getBatchSize() {
        return _batchSize;
    }

    public int getInitializationSampleCount() {

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
        initializeSampleCount = Math.max(
                initializeSampleCount,
                _sendQueueSize);

        /*
         * If we are using batching we need to take into account tha the Send
         * Queue will be per-batch, therefore for the number of samples:
         */
        if (_batchSize > 0) {
            initializeSampleCount = Math.max(
                    (int) (_sendQueueSize * (_batchSize / _dataLen)),
                    initializeSampleCount);
        }

        return initializeSampleCount;
    }

    public void printCmdLineHelp() {
        /**************************************************************************/
        String usage_string =
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
            for (String flow : valid_flow_controller) {
                usage_string += "\"" + flow + "\" ";
            }
            usage_string += "\n" +
            "\t                                Default: \"default\" (If using asynchronous).\n" +
            "\t-peer <address>               - Adds a peer to the peer host address list.\n" +
            "\t                                This argument may be repeated to indicate multiple peers\n" +
            "\n";
            usage_string += _transport.helpMessageString();
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

        System.err.print(usage_string);
    }

    public boolean initialize(int argc, String[] argv) {

        // Register _loggerDevice
        _loggerDevice = new RTIDDSLoggerDevice();
        try {
            Logger.get_instance().set_output_device(_loggerDevice);
        } catch (Exception e) {
            System.err.print("Failed set_output_device for Logger.\n");
            return false;
        }

        _typename = _myDataType.getTypeSupport().get_type_nameI();

        _factory = DomainParticipantFactory.get_instance();

        if (!parseConfig(argc, argv)) {
            return false;
        }

        // setup the QOS profile file to be loaded
        DomainParticipantFactoryQos factory_qos =
            new DomainParticipantFactoryQos();
        _factory.get_qos(factory_qos);
        factory_qos.profile.url_profile.clear();
        factory_qos.profile.url_profile.add(_profileFile);
        _factory.set_qos(factory_qos);

        try {
            _factory.reload_profiles();
        } catch (Exception e) {
            System.err.print("Problem opening QOS profiles file " + _profileFile + ".\n");
            System.err.println(e.getMessage());
            return false;
        }

        try {
            _factory.set_default_library(PROFILE_LIBRARY_NAME);
        } catch (Exception e) {
            System.err.print(
                    "No QOS Library named \"" +
                    PROFILE_LIBRARY_NAME +
                    "\" found in " +
                    _profileFile + ".\n");
            System.err.println(e.getMessage());
            return false;
        }

        // Configure DDSDomainParticipant QOS
        DomainParticipantQos qos = new DomainParticipantQos();
        _factory.get_participant_qos_from_profile(qos, PROFILE_LIBRARY_NAME, "BaseProfileQos");

        if (_secureUseSecure) {
            // validate arguments
            if (!validateSecureArgs()) {
                System.err.println("Failure validating arguments");
                return false;
            }
            configureSecurePlugin(qos);
        }

        // set initial peers and not use multicast
        if (_peer_host_count > 0) {
            qos.discovery.initial_peers.clear();
            qos.discovery.initial_peers.setMaximum(_peer_host_count);
            for (int i = 0; i < _peer_host_count; ++i) {
                 qos.discovery.initial_peers.add(_peer_host[i]);
            }
            qos.discovery.multicast_receive_addresses.clear();
        }

        if(!_transport.configureTransport(qos)) {
            return false;
        }

        if (_AutoThrottle) {
            PropertyQosPolicyHelper.add_property(
                    qos.property,
                    "dds.domain_participant.auto_throttle.enable",
                    "true",
                    false);
        }

        // Creates the participant
        DomainParticipantListener listener = new DomainListener();
        _participant = _factory.create_participant(
            _domainID, qos, listener,
            (StatusKind.INCONSISTENT_TOPIC_STATUS |
             StatusKind.OFFERED_INCOMPATIBLE_QOS_STATUS |
             StatusKind.REQUESTED_INCOMPATIBLE_QOS_STATUS));

        if (_participant == null || _loggerDevice.checkShmemErrors()) {
            if (_loggerDevice.checkShmemErrors()) {
                System.err.print(
                        "The participant creation failed due to issues in the Shared Memory configuration of your OS.\n" +
                        "For more information about how to configure Shared Memory see: http://community.rti.com/kb/osx510 \n" +
                        "If you want to skip the use of Shared memory in RTI Perftest, " +
                        "specify the transport using \"-transport <kind>\", e.g. \"-transport UDPv4\".\n");
            }
            System.err.print("Problem creating participant.\n");
            return false;
        }

        // Register the types and create the topics
         _myDataType.getTypeSupport().register_typeI(
            _participant, _typename);

        // Create the Publisher and Subscriber
        {
            _publisher = _participant.create_publisher_with_profile(PROFILE_LIBRARY_NAME, "BaseProfileQos", null,
                    StatusKind.STATUS_MASK_NONE);

            if (_publisher == null) {
                System.err.print("Problem creating publisher.\n");
                return false;
            }

            _subscriber = _participant.create_subscriber_with_profile(PROFILE_LIBRARY_NAME, "BaseProfileQos", null,
                    StatusKind.STATUS_MASK_NONE);

            if (_subscriber == null) {
                System.err.print("Problem creating subscriber.\n");
                return false;
            }
        }

        return true;
    }

    public IMessagingWriter createWriter(String topicName) {

        DataWriterQos dwQos = new DataWriterQos();
        String qosProfile = null;

        Topic topic = _participant.create_topic(
                           topicName, _typename,
                           DomainParticipant.TOPIC_QOS_DEFAULT, null,
                           StatusKind.STATUS_MASK_NONE);

        if (topic == null) {
            System.err.println("Problem creating topic " + topicName);
            return null;
        }

        qosProfile = getQoSProfileName(topicName);
        if (qosProfile == null) {
            System.err.print("Problem getting qos profile.\n");
            return null;
        }

        try {
            _factory.get_datawriter_qos_from_profile(dwQos, PROFILE_LIBRARY_NAME, qosProfile);
        } catch (Exception e) {
            System.err.print("No QOS Profile named \"" + qosProfile + "\" found in QOS Library \""
                          + PROFILE_LIBRARY_NAME + "\" in file " + _profileFile + ".\n");
            System.err.println(e.getMessage());
            return null;
        }

        configureWriterQos(topicName, qosProfile, dwQos);

        DataWriter writer = _publisher.create_datawriter(
                topic,
                dwQos,
                null,
                StatusKind.STATUS_MASK_NONE);

        if (writer == null) {
            System.err.print("Problem creating writer.\n");
            return null;
        }

        return new RTIPublisher<T>(writer,_instanceCount, _myDataType.clone(),_instancesToBeWritten);
    }

    static int byteToUnsignedInt(byte b) {
        return 0x00 << 24 | b & 0xff;
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
    public TopicDescription createCft(String topicName, Topic topic) {
        String condition;
        String param_list[];
        if (_CFTRange[0] == _CFTRange[1]) { // If same elements, no range
            param_list = new String[KEY_SIZE.VALUE];
            System.err.println("CFT enabled for instance: '"+_CFTRange[0]+"'");

            for (int i = 0; i < KEY_SIZE.VALUE ; i++) {
                param_list[i] = String.valueOf(byteToUnsignedInt((byte)(_CFTRange[0] >>> i * 8)));
            }

            condition = "(%0 = key[0] AND  %1 = key[1] AND %2 = key[2] AND  %3 = key[3]) OR " +
                        "(255 = key[0] AND 255 = key[1] AND 0 = key[2] AND 0 = key[3])";

        } else { // If range
            param_list = new String[KEY_SIZE.VALUE*2];
            System.err.println("CFT enabled for instance range: ["+_CFTRange[0]+","+_CFTRange[1]+"] ");

            for (int i = 0; i < KEY_SIZE.VALUE * 2 ; i++) {
                param_list[i] = String.valueOf(byteToUnsignedInt((byte)(
                            _CFTRange[i < KEY_SIZE.VALUE? 0 : 1]
                                    >>> i % KEY_SIZE.VALUE * 8)));
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
        return _participant.create_contentfilteredtopic(
                topicName, topic, condition, new StringSeq(java.util.Arrays.asList(param_list)));
    }

    public IMessagingReader createReader(String topicName,
                                         IMessagingCB callback) {
        Topic topic = _participant.create_topic(
                           topicName, _typename,
                           DomainParticipant.TOPIC_QOS_DEFAULT, null,
                           StatusKind.STATUS_MASK_NONE);
        if (topic == null) {
            System.err.println("Problem creating topic " + topicName);
            return null;
        }
        TopicDescription  topic_desc = topic; // Used to create the DDS DataReader

        String qosProfile;
        qosProfile = getQoSProfileName(topicName);
        if (qosProfile == null) {
            System.err.print("Problem getting qos profile.\n");
            return null;
        }

        DataReaderQos drQos = new DataReaderQos();
        try {
            _factory.get_datareader_qos_from_profile(drQos, PROFILE_LIBRARY_NAME, qosProfile);
        } catch (Exception e) {
            System.err.print("No QOS Profile named \"" + qosProfile + "\" found in QOS Library \""
                          + PROFILE_LIBRARY_NAME + "\" in file " + _profileFile + ".\n");
            System.err.println(e.getMessage());
            return null;
        }

        configureReaderQos(topicName, qosProfile, drQos);

        ReceiverListener<T> readerListener = null;
        int statusFlag = StatusKind.STATUS_MASK_NONE;

        if (callback != null) {
            readerListener = new ReceiverListener<T>(
                    callback,
                    _myDataType.clone());
            statusFlag = StatusKind.DATA_AVAILABLE_STATUS;
        }

        if (THROUGHPUT_TOPIC_NAME.VALUE.equals(topicName) && _useCft) {
            topic_desc = createCft(topicName, topic);
            if (topic_desc == null) {
                System.err.println("create_contentfilteredtopic error");
                return null;
            }
        }
        DataReader reader = _subscriber.create_datareader(
                topic_desc,
                drQos,
                readerListener,
                statusFlag);

        if (reader == null) {
            System.err.print("Problem creating reader.\n");
            return null;
        }

        if (LATENCY_TOPIC_NAME.VALUE.equals(topicName) ||
            THROUGHPUT_TOPIC_NAME.VALUE.equals(topicName)) {
            _reader = reader;
        }

        return new RTISubscriber<T>(reader,_myDataType.clone());
    }

    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------
    private boolean validateSecureArgs() {
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

    private String printSecureArgs() {

        String secure_arguments_string =
                "Secure Arguments:\n" +
                "\t encrypt discovery: " + _secureIsDiscoveryEncrypted + "\n" +
                "\t encrypt topic (user) data: " + _secureIsDataEncrypted + "\n" +
                "\t encrypt submessage: " + _secureIsSMEncrypted + "\n" +
                "\t sign data: " +_secureIsSigned + "\n";

        if (_governanceFile != null) {
            secure_arguments_string += "\t governance file: " + _governanceFile
                    + "\n";
        } else {
            secure_arguments_string += "\t governance file: Not specified\n";
        }

        if (_securePermissionsFile != null) {
            secure_arguments_string += "\t permissions file: " + _securePermissionsFile
                    + "\n";
        } else {
            secure_arguments_string += "\t permissions file: Not specified\n";
        }

        if (_securePrivateKeyFile != null) {
            secure_arguments_string += "\t private key file: " + _securePrivateKeyFile
                    + "\n";
        } else {
            secure_arguments_string += "\t private key file: Not specified\n";
        }

        if (_secureCertificateFile != null) {
            secure_arguments_string += "\t certificate file: " + _secureCertificateFile
                    + "\n";
        } else {
            secure_arguments_string += "\t certificate file: Not specified\n";
        }

        if (_secureCertAuthorityFile != null) {
            secure_arguments_string += "\t certificate authority file: "
                    + _secureCertAuthorityFile + "\n";
        } else {
            secure_arguments_string += "\t certificate authority file: Not specified\n";
        }

        if (_secureLibrary != null) {
            secure_arguments_string += "\t plugin library: " + _secureLibrary + "\n";
        } else {
            secure_arguments_string += "\t plugin library: Not specified\n";
        }

        if( _secyreDebugLevel != -1 ){
            secure_arguments_string += "\t debug level: " + _secyreDebugLevel + "\n";
        }
        return secure_arguments_string;
    }

    private void configureSecurePlugin(DomainParticipantQos dpQos) {
        // configure use of security plugins, based on provided arguments

        // load plugin
        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.load_plugin",
                "com.rti.serv.secure",
                false);

        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.create_function",
                "RTI_Security_PluginSuite_create",
                false);

        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.library",
                _secureLibrary,
                false);

        /*
         * Below, we are using com.rti.serv.secure properties in order to be
         * backward compatible with RTI Connext DDS 5.3.0 and below. Later
         * versions use the properties that are specified in the DDS Security
         * specification (see also the RTI Security Plugins Getting Started
         * Guide). However, later versions still support the legacy properties
         * as an alternative.
         */

        // check if governance file provided
        if (_governanceFile == null) {
            // choose a pre-built governance file
            _governanceFile = "resource/secure/signed_PerftestGovernance_";

            if (_secureIsDiscoveryEncrypted) {
                _governanceFile += "Discovery";
            }

            if (_secureIsSigned) {
                _governanceFile += "Sign";
            }

            if (_secureIsDataEncrypted && _secureIsSMEncrypted) {
                _governanceFile += "EncryptBoth";
            } else if (_secureIsDataEncrypted) {
                _governanceFile += "EncryptData";
            } else if (_secureIsSMEncrypted) {
                _governanceFile += "EncryptSubmessage";
            }

            _governanceFile += ".xml";

            PropertyQosPolicyHelper.add_property(
                    dpQos.property,
                    "com.rti.serv.secure.access_control.governance_file",
                    _governanceFile,
                    false);
        } else {
            PropertyQosPolicyHelper.add_property(
                    dpQos.property,
                    "com.rti.serv.secure.access_control.governance_file",
                    _governanceFile,
                    false);
        }

        // permissions file
        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.permissions_file",
                _securePermissionsFile,
                false);

        // permissions authority file
        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.access_control.permissions_authority_file",
                _secureCertAuthorityFile,
                false);

        // certificate authority
        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.authentication.ca_file",
                _secureCertAuthorityFile,
                false);

        // public key
        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.authentication.certificate_file",
                _secureCertificateFile,
                false);

        // private key
        PropertyQosPolicyHelper.add_property(
                dpQos.property,
                "com.rti.serv.secure.authentication.private_key_file",
                _securePrivateKeyFile,
                false);

        if (_secyreDebugLevel != -1) {
            PropertyQosPolicyHelper.add_property(
                    dpQos.property,
                    "com.rti.serv.secure.logging.log_level",
                    (new Integer(_secyreDebugLevel)).toString(),
                    false);
        }
    }

    private void configureWriterQos(
            String topicName, String qosProfile, DataWriterQos dwQos) {
        // Configure ACKs
        if (!_usePositiveAcks
                && ("ThroughputQos".equals(qosProfile)
                    || "LatencyQos".equals(qosProfile))) {
            dwQos.protocol.disable_positive_acks = true;
            if (_keepDurationUsec != -1) {
                dwQos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration.sec =
                    Duration_t.from_micros(_keepDurationUsec).sec;
                dwQos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration.nanosec =
                    Duration_t.from_micros(_keepDurationUsec).nanosec;
            }
        }

        if (_useUnbounded > 0) {
            PropertyQosPolicyHelper.add_property(
                    dwQos.property, "dds.data_writer.history.memory_manager.fast_pool.pool_buffer_max_size",
                    Long.toString(_useUnbounded), false);
            PropertyQosPolicyHelper.add_property(
                    dwQos.property, "dds.data_writer.history.memory_manager.java_stream.min_size",
                    Long.toString(_useUnbounded), false);
            PropertyQosPolicyHelper.add_property(
                    dwQos.property, "dds.data_writer.history.memory_manager.java_stream.trim_to_size",
                    "1", false);
        }

        if (_isLargeData || _IsAsynchronous)
        {
            dwQos.publish_mode.kind = PublishModeQosPolicyKind.ASYNCHRONOUS_PUBLISH_MODE_QOS;
            if (!_FlowControllerCustom.toLowerCase().startsWith("default".toLowerCase())) {
                dwQos.publish_mode.flow_controller_name = "dds.flow_controller.token_bucket."+_FlowControllerCustom;
            }
        }

        // Configure reliability
        if (!ANNOUNCEMENT_TOPIC_NAME.VALUE.equals(topicName)) {
            if (_isReliable) {
                // default: use the setting specified in the qos profile
                // dwQos.reliability.kind = ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;

            } else {
                // override to best-effort
                dwQos.reliability.kind =
                    ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
            }
        }

        // These QOS's are only set for the Throughput datawriter
        if ("ThroughputQos".equals(qosProfile)) {

            if (_transport.useMulticast) {
                dwQos.protocol.rtps_reliable_writer.enable_multicast_periodic_heartbeat = true;
            }

            if (_batchSize > 0) {
                dwQos.batch.enable = true;
                dwQos.batch.max_data_bytes = _batchSize;
                dwQos.resource_limits.max_samples = ResourceLimitsQosPolicy.LENGTH_UNLIMITED;
                dwQos.writer_resource_limits.max_batches = _sendQueueSize;
            } else {
                dwQos.resource_limits.max_samples = _sendQueueSize;
            }

            if (_heartbeatPeriod.sec > 0 || _heartbeatPeriod.nanosec > 0) {
                // set the heartbeat_period
                dwQos.protocol.rtps_reliable_writer.heartbeat_period.sec = _heartbeatPeriod.sec;
                dwQos.protocol.rtps_reliable_writer.heartbeat_period.nanosec = _heartbeatPeriod.nanosec;
                // make the late joiner heartbeat compatible
                dwQos.protocol.rtps_reliable_writer.late_joiner_heartbeat_period.sec = _heartbeatPeriod.sec;
                dwQos.protocol.rtps_reliable_writer.late_joiner_heartbeat_period.nanosec = _heartbeatPeriod.nanosec;
            }
            if (_fastHeartbeatPeriod.sec > 0 || _fastHeartbeatPeriod.nanosec > 0) {
                // set the fast_heartbeat_period
                dwQos.protocol.rtps_reliable_writer.fast_heartbeat_period.sec = _fastHeartbeatPeriod.sec;
                dwQos.protocol.rtps_reliable_writer.fast_heartbeat_period.nanosec = _fastHeartbeatPeriod.nanosec;
            }

            if (_AutoThrottle) {
                PropertyQosPolicyHelper.add_property(
                        dwQos.property, "dds.data_writer.auto_throttle.enable",
                        "true", false);
            }

            if (_TurboMode) {
                PropertyQosPolicyHelper.add_property(
                        dwQos.property, "dds.data_writer.enable_turbo_mode",
                        "true", false);
                dwQos.batch.enable = false;
                dwQos.resource_limits.max_samples = ResourceLimitsQosPolicy.LENGTH_UNLIMITED;
                dwQos.writer_resource_limits.max_batches = _sendQueueSize;
            }

            dwQos.resource_limits.initial_samples = _sendQueueSize;
            dwQos.resource_limits.max_samples_per_instance
                = dwQos.resource_limits.max_samples;

            switch(_durability){
            case 0:
                dwQos.durability.kind = DurabilityQosPolicyKind.VOLATILE_DURABILITY_QOS;
                break;
            case 1:
                dwQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_LOCAL_DURABILITY_QOS;
                break;
            case 2:
                dwQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
                break;
            case 3:
                dwQos.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
                break;
            }

            dwQos.durability.direct_communication = _directCommunication;

            dwQos.protocol.rtps_reliable_writer.heartbeats_per_max_samples = _sendQueueSize / 10;

            dwQos.protocol.rtps_reliable_writer.low_watermark = _sendQueueSize * 1 / 10;
            dwQos.protocol.rtps_reliable_writer.high_watermark = _sendQueueSize * 9 / 10;

            /*
             * If _SendQueueSize is 1 low watermark and high watermark would both be
             * 0, which would cause the middleware to fail. So instead we set the
             * high watermark to the low watermark + 1 in such case.
             */
            if (dwQos.protocol.rtps_reliable_writer.high_watermark
                == dwQos.protocol.rtps_reliable_writer.high_watermark) {
                dwQos.protocol.rtps_reliable_writer.high_watermark =
                        dwQos.protocol.rtps_reliable_writer.high_watermark + 1;
            }

            dwQos.protocol.rtps_reliable_writer.max_send_window_size =
                    _sendQueueSize;
            dwQos.protocol.rtps_reliable_writer.min_send_window_size =
                    _sendQueueSize;
        }

        if ("LatencyQos".equals(qosProfile)
                && !_directCommunication
                && (_durability == DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS.ordinal()
                    || _durability == DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS.ordinal())) {

            if (_durability == DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS.ordinal()) {
                dwQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
            } else{
                dwQos.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
            }

            dwQos.durability.direct_communication = _directCommunication;
        }

        dwQos.resource_limits.max_instances = _instanceCount + 1; // One extra for MAX_CFT_VALUE
        dwQos.resource_limits.initial_instances = _instanceCount + 1;

        if (_instanceCount > 1) {
            if (_instanceHashBuckets > 0) {
                dwQos.resource_limits.instance_hash_buckets = _instanceHashBuckets;
            } else {
                dwQos.resource_limits.instance_hash_buckets = _instanceCount;
            }
        }
    }

    private void configureReaderQos(
            String topicName, String qosProfile, DataReaderQos drQos) {
        // Configure ACKs
        if (!_usePositiveAcks
                && ("ThroughputQos".equals(qosProfile)
                    || "LatencyQos".equals(qosProfile))) {
            drQos.protocol.disable_positive_acks = true;
        }

        // Configure reliability
        if (!ANNOUNCEMENT_TOPIC_NAME.VALUE.equals(topicName)) {
            if (_isReliable) {
                drQos.reliability.kind = ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
            } else {
                drQos.reliability.kind = ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
            }
        }

        if (_useUnbounded > 0) {
            PropertyQosPolicyHelper.add_property(
                    drQos.property, "dds.data_reader.history.memory_manager.fast_pool.pool_buffer_max_size",
                    Long.toString(_useUnbounded), false);
            PropertyQosPolicyHelper.add_property(
                    drQos.property, "dds.data_reader.history.memory_manager.java_stream.min_size",
                    Long.toString(_useUnbounded), false);
            PropertyQosPolicyHelper.add_property(
                    drQos.property, "dds.data_reader.history.memory_manager.java_stream.trim_to_size",
                    "1", false);
        }

        // These QOS's are only set for the Throughput reader
        if ("ThroughputQos".equals(qosProfile)) {
            switch(_durability){
            case 0:
                drQos.durability.kind = DurabilityQosPolicyKind.VOLATILE_DURABILITY_QOS;
                break;
            case 1:
                drQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_LOCAL_DURABILITY_QOS;
                break;
            case 2:
                drQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
                break;
            case 3:
                drQos.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
                break;
            }

            drQos.durability.direct_communication = _directCommunication;
        }

        if ("LatencyQos".equals(qosProfile)
                && !_directCommunication
                && (_durability == DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS.ordinal()
                    || _durability == DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS.ordinal())) {

            if(_durability == DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS.ordinal()){
                drQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
            } else{
                drQos.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
            }
            drQos.durability.direct_communication = _directCommunication;
        }

        drQos.resource_limits.initial_instances = _instanceCount + 1;
        if (_instanceMaxCountReader != -1) {
            _instanceMaxCountReader++;
        }
        drQos.resource_limits.max_instances = _instanceMaxCountReader;

        if (_instanceCount > 1) {
            if (_instanceHashBuckets > 0) {
                drQos.resource_limits.instance_hash_buckets = _instanceHashBuckets;
            } else {
                drQos.resource_limits.instance_hash_buckets = _instanceCount;
            }
        }

        if (_transport.useMulticast && _transport.allowsMulticast()) {
            String multicast_addr = _transport.getMulticastAddr(topicName);
            if (multicast_addr == null) {
                System.err.println("topic name must either be "
                        + THROUGHPUT_TOPIC_NAME.VALUE + " or "
                        + LATENCY_TOPIC_NAME.VALUE + " or "
                        + ANNOUNCEMENT_TOPIC_NAME.VALUE);
                return ;
            }

            TransportMulticastSettings_t multicast_setting =
                new TransportMulticastSettings_t();
            try {
                multicast_setting.receive_address =
                    InetAddress.getByName(multicast_addr);
            } catch (UnknownHostException uhx) {
                throw new IllegalStateException(uhx.getMessage(), uhx);
            }
            multicast_setting.receive_port = 0;
            multicast_setting.transports.clear();
            drQos.multicast.value.clear();
            drQos.multicast.value.add(multicast_setting);
        }
    }

    public String printConfiguration() {

        StringBuilder sb = new StringBuilder();

        // Domain ID
        sb.append("\tDomain: ");
        sb.append(_domainID);
        sb.append("\n");

        // Dynamic Data
        sb.append("\tDynamic Data: ");
        if (_isDynamicData) {
            sb.append("Yes\n");
        } else {
            sb.append("No\n");
        }

        // Dynamic Data
        if (_isPublisher) {
            sb.append("\tAsynchronous Publishing: ");
            if (_isLargeData || _IsAsynchronous) {
                sb.append("Yes\n");
                sb.append("\tFlow Controller: ");
                sb.append(_FlowControllerCustom);
                sb.append("\n");
            } else {
                sb.append("No\n");
            }
        }

        // Turbo Mode / AutoThrottle
        if (_TurboMode) {
            sb.append("\tTurbo Mode: Enabled\n");
        }
        if (_AutoThrottle) {
            sb.append("\tAutoThrottle: Enabled\n");
        }

        // XML File
        sb.append("\tXML File: ");
        sb.append(_profileFile);
        sb.append("\n");


        sb.append("\n");
        sb.append(_transport.printTransportConfigurationSummary());


        // set initial peers and not use multicast
        if (_peer_host_count > 0) {
            sb.append("Initial peers: ");
            for ( int i = 0; i < _peer_host_count; ++i) {
                sb.append(_peer_host[i]);
                if (i == _peer_host_count - 1) {
                    sb.append("\n");
                } else {
                    sb.append(", ");
                }
            }
        }

        if (_secureUseSecure) {
            sb.append("\n");
            sb.append(printSecureArgs());
        }

        return sb.toString();
    }

    private boolean parseConfig(int argc, String[] argv) {
        long minScanSize = MAX_PERFTEST_SAMPLE_SIZE.VALUE;
        boolean isBatchSizeProvided = false;

        for (int i = 0; i < argc; ++i) {

            if ("-scan".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isScan = true;
                /*
                * Check if we have custom scan values. In such case we are just
                * interested in the minimum one.
                */
                if ((i != (argc - 1)) && !argv[1+i].startsWith("-")) {
                    ++i;
                    long auxScan = 0;
                    StringTokenizer st = new StringTokenizer(argv[i], ":", true);
                    while (st.hasMoreTokens()) {
                        String s = st.nextToken();
                        if (!s.equals(":")) {
                            auxScan = Long.parseLong(s);
                            if (auxScan < minScanSize) {
                                minScanSize = auxScan;
                            }
                        }
                    }
                /*
                 * If we do not specify any custom value for the -scan, we would
                 * set minScanSize to the minimum size in the default set for -scan.
                 */
                } else {
                    minScanSize = 32;
                }
            }
            else if ("-pub".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isPublisher = true;
            }
            else if ("-dynamicData".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isDynamicData = true;
            }
            else if ("-dataLen".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <length> after -dataLen\n");
                    return false;
                }
                try {
                    _dataLen = Long.parseLong(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad dataLen\n");
                    return false;
                }
                if (_dataLen < PerfTest.OVERHEAD_BYTES) {
                    System.err.println("dataLen must be >= " + PerfTest.OVERHEAD_BYTES);
                    return false;
                }
                if (_dataLen > PerfTest.getMaxPerftestSampleSizeJava()) {
                    System.err.println("dataLen must be <= " + PerfTest.getMaxPerftestSampleSizeJava());
                    return false;
                }
                if (_useUnbounded == 0 && _dataLen > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                    _useUnbounded = Math.min(
                            MAX_BOUNDED_SEQ_SIZE.VALUE, 2 * _dataLen);
                }
            }else if ("-unbounded".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[i+1].startsWith("-")) {
                    _useUnbounded = Math.min(
                            MAX_BOUNDED_SEQ_SIZE.VALUE, 2 * _dataLen);
                } else {
                    ++i;
                    try {
                        _useUnbounded = Long.parseLong(argv[i]);
                    } catch (NumberFormatException nfx) {
                        System.err.print("Bad allocation_threshold value.\n");
                        return false;
                    }
                }

                if (_useUnbounded < PerfTest.OVERHEAD_BYTES) {
                    System.err.println("unbounded must be >= " + PerfTest.OVERHEAD_BYTES);
                    return false;
                }
                if (_useUnbounded > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                    System.err.println("unbounded must be <= " + MAX_BOUNDED_SEQ_SIZE.VALUE);
                    return false;
                }
            } else if ("-sendQueueSize".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <count> after -sendQueueSize\n");
                    return false;
                }
                try {
                    _sendQueueSize = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad sendQueueSize\n");
                    return false;
                }
            } else if ("-heartbeatPeriod".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <period> after -heartbeatPeriod\n");
                    return false;
                }
                try {
                    StringTokenizer st = new StringTokenizer(argv[i],":", false);
                    String sec = st.nextToken();
                    String nanosec = st.nextToken();
                    _heartbeatPeriod.sec = Integer.valueOf(sec).intValue();
                    _heartbeatPeriod.nanosec = Integer.valueOf(nanosec).intValue();
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad heartbeatPeriod\n");
                    return false;
                }
            } else if ("-fastHeartbeatPeriod".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <period> after -fastHeartbeatPeriod\n");
                    return false;
                }
                try {
                    StringTokenizer st = new StringTokenizer(argv[i],":", false);
                    String sec = st.nextToken();
                    String nanosec = st.nextToken();
                    _fastHeartbeatPeriod.sec = Integer.valueOf(sec).intValue();
                    _fastHeartbeatPeriod.nanosec = Integer.valueOf (nanosec).intValue();
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad fastHeartbeatPeriod\n");
                    return false;
                }
            } else if ("-domain".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <id> after -domain\n");
                    return false;
                }
                try {
                    _domainID = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad domain id\n");
                    return false;
                }
            } else if ("-qosFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing filename after -qosFile\n");
                    return false;
                }
                _profileFile = argv[i];
            } else if ("-qosLibrary".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <library name> after -qosLibrary\n");
                    return false;
                }
                PROFILE_LIBRARY_NAME = argv[i];
            } else if ("-bestEffort".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isReliable = false;
            } else if ("-durability".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <kind> after -durability\n");
                    return false;
                }
                try {
                    _durability = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad <kind> for durability\n");
                    return false;
                }
                if ((_durability < 0) || (_durability > 3)) {
                    System.err.print("durability <kind> must be 0(volatile),1(transient local),2(transient),3(persistent) \n");
                    return false;
                }
            } else if ("-noDirectCommunication".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _directCommunication = false;
            }
            else if ("-latencyTest".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                _latencyTest = true;
            }
            else if ("-instances".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <count> after -instances\n");
                    return false;
                }
                try {
                    _instanceCount = Integer.parseInt(argv[i]);
                    _instanceMaxCountReader = _instanceCount;
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad <count> for instances\n");
                    return false;
                }
                if (_instanceCount < 0) {
                    System.err.print("instance count cannot be negative\n");
                    return false;
                }
            } else if ("-instanceHashBuckets".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <count> after -instanceHashBuckets\n");
                    return false;
                }
                try {
                    _instanceHashBuckets = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad <count> for instanceHashBuckets\n");
                    return false;
                }
                if (_instanceHashBuckets <= 0 && _instanceHashBuckets != -1) {
                    System.err.print("instanceHashBucket count cannot be negative or zero\n");
                    return false;
                }
            } else if ("-batchSize".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <#bytes> after -batchSize\n");
                    return false;
                }
                try {
                    _batchSize = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad #bytes for batch\n");
                    return false;
                }
                if (_batchSize < 0 || _batchSize > MAX_SYNCHRONOUS_SIZE.VALUE) {
                    System.err.print("Batch size '" + _batchSize +
                            "' should be between [0," +
                            MAX_SYNCHRONOUS_SIZE.VALUE +
                            "]\n");
                    return false;
                }
                isBatchSizeProvided = true;
            }
            else if ("-keepDurationUsec".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <usec> after -keepDurationUsec\n");
                    return false;
                }
                try {
                    _keepDurationUsec = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad usec for keep duration\n");
                    return false;
                }
            }
            else if ("-noPositiveAcks".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _usePositiveAcks = false;
            } else if ("-verbosity".toLowerCase().startsWith(argv[i].toLowerCase())) {
                try {
                    int verbosityLevel = Integer.parseInt(argv[++i]);
                    switch (verbosityLevel) {
                        case 0: Logger.get_instance().set_verbosity(
                                    LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_SILENT);
                                System.err.print("Setting verbosity to SILENT\n");
                                break;
                        case 1: Logger.get_instance().set_verbosity(
                                    LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_ERROR);
                                System.err.print("Setting verbosity to ERROR\n");
                                break;
                        case 2: Logger.get_instance().set_verbosity(
                                    LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_WARNING);
                                System.err.print("Setting verbosity to WARNING\n");
                                break;
                        case 3: Logger.get_instance().set_verbosity(
                                    LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
                                System.err.print("Setting verbosity to STATUS_ALL\n");
                                break;
                        default: System.err.print(
                                    "Invalid value for the verbosity parameter." +
                                    " Setting verbosity to ERROR (1)\n");
                                break;
                    }
                } catch (NumberFormatException nfx) {
                    System.err.print("Unexpected value after -verbosity\n");
                    return false;
                }
            } else if ("-waitsetDelayUsec".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <usec> after -waitsetDelayUsec\n");
                    return false;
                }
                try {
                    waitsetDelayUsec = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad usec for waitset delay Usec\n");
                    return false;
                }
                if (waitsetDelayUsec < 0) {
                    System.err.print("waitset delay usec cannot be negative\n");
                    return false;
                }
            } else if ("-waitsetEventCount".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <count> after -waitsetEventCount\n");
                    return false;
                }
                try {
                    waitsetEventCount = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad value for waitset event count\n");
                    return false;
                }
                if (waitsetEventCount < 0) {
                    System.err.print("waitset event count cannot be negative\n");
                    return false;
                }
            } else if ("-enableAutoThrottle".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _AutoThrottle = true;
            } else if ("-enableTurboMode".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _TurboMode = true;
            } else if ("-secureSign".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _secureIsSigned = true;
                _secureUseSecure = true;
            } else if ("-secureEncryptBoth".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _secureIsDataEncrypted = true;
                _secureIsSMEncrypted = true;
                _secureUseSecure = true;
            } else if ("-secureEncryptData".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _secureIsDataEncrypted = true;
                _secureUseSecure = true;
            } else if ("-secureEncryptSM".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _secureIsSMEncrypted = true;
                _secureUseSecure = true;
            } else if ("-secureEncryptDiscovery".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _secureIsDiscoveryEncrypted = true;
                _secureUseSecure = true;
            } else if ("-secureGovernanceFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <file> after -secureGovernanceFile\n");
                    return false;
                }
                _governanceFile  = argv[i];
                System.out.println("Warning -- authentication, encryption, signing arguments " +
                         "will be ignored, and the values specified by the Governance file will " +
                         "be used instead");
                 _secureUseSecure = true;
            } else if ("-securePermissionsFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <file> after -securePermissionsFile\n");
                    return false;
                }
                _securePermissionsFile  = argv[i];
                _secureUseSecure = true;
            } else if ("-secureCertAuthority".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <file> after -secureCertAuthority\n");
                    return false;
                }
                _secureCertAuthorityFile = argv[i];
                _secureUseSecure = true;
            } else if ("-secureCertFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <file> after -secureCertAuthority\n");
                    return false;
                }
                _secureCertificateFile = argv[i];
                _secureUseSecure = true;
            } else if ("-securePrivateKey".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <file> after -securePrivateKey\n");
                    return false;
                }
                _securePrivateKeyFile = argv[i];
                _secureUseSecure = true;
            } else if ("-secureLibrary".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <file> after -secureLibrary\n");
                    return false;
                }
                _secureLibrary = argv[i];
            } else if ("-secureDebug".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <level> after -secureDebug\n");
                     return false;
                }
                try {
                    _secyreDebugLevel = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad value for -secureDebug\n");
                    return false;
                }
            } else if ("-asynchronous".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _IsAsynchronous = true;
            } else if ("-flowController".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <flow Controller Name> after -flowController\n");
                    return false;
                }
                _FlowControllerCustom = argv[i];

                // verify if the flow controller name is correct, else use "default"
                boolean valid_flow_control = false;
                for (String flow : valid_flow_controller) {
                    if (_FlowControllerCustom.equals(flow)) {
                        valid_flow_control = true;
                    }
                }

                if (!valid_flow_control) {
                    System.err.print("Bad <flow> '"+_FlowControllerCustom+"' for custom flow controller\n");
                    _FlowControllerCustom = "default";
                }
            } else if ("-peer".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <address> after -peer\n");
                    return false;
                }
                if (_peer_host_count +1 < RTIPERFTEST_MAX_PEERS) {
                    _peer_host[_peer_host_count++] = argv[i];
                } else {
                    System.err.print("The maximum of -initial peers is " + RTIPERFTEST_MAX_PEERS + "\n");
                    return false;
                }
            } else if ("-cft".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _useCft = true;
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <start>:<end> after -cft\n");
                    return false;
                }
                if (argv[i].contains(":")) {
                    try {
                        StringTokenizer st = new StringTokenizer(argv[i],":", false);
                        String startCFT = st.nextToken();
                        String endCFT = st.nextToken();
                        _CFTRange[0] = Integer.parseInt(startCFT);
                        _CFTRange[1] = Integer.parseInt(endCFT);
                    } catch (NumberFormatException nfx) {
                        System.err.print("Bad <start>:<end> after -cft\n");
                        return false;
                    }
                } else {
                    _CFTRange[0] = Integer.parseInt(argv[i]);
                    _CFTRange[1] = _CFTRange[0];
                }

                if (_CFTRange[0] > _CFTRange[1]) {
                    System.err.println("-cft <start> value cannot be bigger than <end>");
                    return false;
                }
                if (_CFTRange[0] < 0 ||
                        _CFTRange[0] >= MAX_CFT_VALUE.VALUE ||
                        _CFTRange[1] < 0 ||
                        _CFTRange[1] >= MAX_CFT_VALUE.VALUE) {
                    System.err.println("-cft <start>:<end> values should be between [0," + MAX_CFT_VALUE.VALUE + "]");
                    return false;
                }
            } else if ("-writeInstance".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <number> after -writeInstance\n");
                    return false;
                }
                _instancesToBeWritten = Integer.parseInt(argv[i]);
            } else {

                Integer value = _transport.getTransportCmdLineArgs().get(argv[i]);
                if (value != null) {
                    // Increment the counter with the number of arguments
                    // obtained from the map.
                    i = i + value.intValue();
                } else {
                    System.err.print(argv[i] + ": not recognized\n");
                    return false;
                }
            }
        }

        /* If we are using scan, we get the minimum and set it in Datalen */
        if (_isScan) {
            _dataLen = minScanSize;
        }

        /* Check if we need to enable Large Data. This works also for -scan */
        if (_dataLen > Math.min(
                MAX_SYNCHRONOUS_SIZE.VALUE,
                MAX_BOUNDED_SEQ_SIZE.VALUE)) {
            _isLargeData = true;
            if (_useUnbounded == 0) {
                _useUnbounded = MAX_BOUNDED_SEQ_SIZE.VALUE;
            }
        } else { /* No Large Data */
            _isLargeData = false;
        }

        /* If we are using batching */
        if (_batchSize > 0) {

            /* We will not use batching for a latency test */
            if (_latencyTest) {
                if (isBatchSizeProvided) {
                    System.err.println(
                            "Batching cannot be used in a Latency test.");
                    return false;
                }
                else {
                    _batchSize = 0; // Disable Batching
                }
            }

            /* Check if using asynchronous */
            if (_IsAsynchronous) {
                if (isBatchSizeProvided) {
                    System.err.println(
                            "Batching cannot be used with asynchronous writing.\n");
                    return false;
                }
                else {
                    _batchSize = 0; // Disable Batching
                }
            }

            /*
             * Large Data + batching cannot be set. But batching is enabled by default,
             * so in that case, we just disabled batching, else, the customer set it up,
             * so we explitly fail
             */
            if (_isLargeData) {
                if (isBatchSizeProvided) {
                    System.err.println(
                            "Batching cannot be used with Large Data.");
                    return false;
                } else {
                    _batchSize = -2;
                }
            } else if ((_batchSize < _dataLen * 2) && !_isScan) {
                /*
                 * We don't want to use batching if the batch size is not large
                 * enough to contain at least two samples (in this case we avoid the
                 * checking at the middleware level).
                 */
                if (isBatchSizeProvided) {
                    /*
                     * Batchsize disabled. A message will be print if _batchSize < 0 in
                     * perftest_cpp::PrintConfiguration()
                     */
                    _batchSize = -1;
                } else {
                    _batchSize = 0;
                }
            }
        }

        if (_TurboMode) {
            if (_IsAsynchronous) {
                System.err.println("Turbo Mode cannot be used with asynchronous writing.");
                return false;
            }
            if (_isLargeData) {
                System.err.println("Turbo Mode disabled, using large data.");
                _TurboMode = false;
            }
        }

        // Manage _instancesToBeWritten
        if (_instancesToBeWritten != -1) {
            if (_instanceCount < _instancesToBeWritten) {
                System.err.println("Specified '-WriteInstance' (" + _instancesToBeWritten +
                        ") invalid: Bigger than the number of instances (" + _instanceCount + ").");
                return false;
            }
        }
        if (_isPublisher && _useCft) {
            System.err.println("Content Filtered Topic is not a parameter in the publisher side.\n");
        }
        if (_transport != null) {
            if(!_transport.parseTransportOptions(argc, argv)) {
                System.err.println("Failure parsing the transport options.");
                return false;
            }
        } else {
            System.err.println("_transport is not initialized");
            return false;
        }

        return true;
    }

    public String getQoSProfileName(String topicName) {
        // get() function return null if the map contains no mapping for the key
        if(_qoSProfileNameMap.get(topicName) == null){
            System.err.println(
                    "topic name must either be " +
                    LATENCY_TOPIC_NAME.VALUE + " or " +
                    ANNOUNCEMENT_TOPIC_NAME.VALUE  + " or " +
                    THROUGHPUT_TOPIC_NAME.VALUE);
            return null;
        }
        return _qoSProfileNameMap.get(topicName).toString();
    }

}

// ===========================================================================
