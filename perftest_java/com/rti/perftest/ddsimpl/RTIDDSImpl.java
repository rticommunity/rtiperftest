/* $Id: RTIDDSImpl.java,v 1.2.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
-------------------- 
5.1.0.9,27mar14,jmc PERFTEST-27 Fixing resource limits when using
                    Turbo Mode
5.1.0,19dec13,jmc PERFTEST-3 Added autothrottle and turbomode
5.1.0,19dec13,jmc PERFTEST-2 window size in batching path and
                   domain id now is 1
29aug13,jmc CORE-5854 multicast disabled by default
29aug13,jmc CORE-5919 Moved hardcoded QoS to XML file when
                      possible
29aug13,jmc CORE-5867 transport builtin mask to only shmem
07jul10,eys Cleanup perftest parameters
07jul10,jsr Fixed keepDurationUsec help
29jun10,jsr Fix heartbeat and fastheartbeat for windows
01may10,jsr Added latencyTest option
10mar10,gn  Ported tcp implementation from c++
26may09,fcs Fixed test finalization for keyed topics
14may09,fcs Added instances to INI
14may09,fcs Fixed command-line arguments processing
08may09,jsr Fixed default profile names
29apr09,jsr Added detection of wrong command line parameter
23apr09,jsr Changed to stderr the error and status messages
21apr09,jsr Reformat the help menu
17apr09,jsr Fixed #12322, added -waitsetDelayUsec and -waitsetEventCount 
            command line option
17apr09,jsr Fixed #12656, -keepDurationUsec command line option and 
            resolution
22jan09,jsr Added EDS support
08dec08,jsr Added heartbeat_period and fastHeartbeat_period parameters
04sep08,rbw Fixed key logic. Conformed fields to Java naming conventions.
            Refactored endpoint QoS configuration.
20aug08,eys Move InstanceCount to perftest.java
15aug08,ch  added PresentationQosPolicyAccessScopeKind.TOPIC_PRESENTATION_QOS
09aug08,ch  Key support, multi-instances, durability
11jun08,rbw Follow Java naming conventions. Fixed compiler warnings.
01may08,hhw Removed singleCore option.
            Increased shared memory buffer for shm tests
            KEEP_ALL is used for both reliable/unreliable.
            Added announcement msg support.
28apr08,rbw Fixed string comparison bug
02apr08,rbw Improved config file checking; minor stylistic improvements
02apr08,rbw Moved PerfTest to package com.rti.perftest.harness to distinguish
            between (1) RTI-specific test implementation and (2) generic test
            harness
02apr08,rbw Implemented *.ini file parsing
01apr08,rbw Fixed string comparisons
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */

package com.rti.perftest.ddsimpl;

import java.io.File;
import java.io.IOException;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.util.StringTokenizer;

import com.rti.dds.domain.DomainParticipant;
import com.rti.dds.domain.DomainParticipantFactory;
import com.rti.dds.domain.DomainParticipantFactoryQos;
import com.rti.dds.domain.DomainParticipantListener;
import com.rti.dds.domain.DomainParticipantQos;
import com.rti.dds.infrastructure.Duration_t;
import com.rti.dds.infrastructure.HistoryQosPolicyKind;
import com.rti.dds.infrastructure.PropertyQosPolicyHelper;
import com.rti.dds.infrastructure.ReliabilityQosPolicyKind;
import com.rti.dds.infrastructure.ResourceLimitsQosPolicy;
import com.rti.dds.infrastructure.StatusKind;
import com.rti.dds.infrastructure.TransportBuiltinKind;
import com.rti.dds.infrastructure.TransportMulticastSettings_t;
import com.rti.dds.infrastructure.DurabilityQosPolicyKind;
import com.rti.dds.infrastructure.PresentationQosPolicyAccessScopeKind;
import com.rti.dds.infrastructure.DiscoveryConfigBuiltinPluginKind;
import com.rti.dds.publication.DataWriter;
import com.rti.dds.publication.DataWriterQos;
import com.rti.dds.publication.Publisher;
import com.rti.dds.publication.PublisherQos;
import com.rti.dds.subscription.DataReader;
import com.rti.dds.subscription.DataReaderQos;
import com.rti.dds.subscription.Subscriber;
import com.rti.dds.subscription.SubscriberQos;
import com.rti.dds.topic.Topic;
import com.rti.ndds.config.LogCategory;
import com.rti.ndds.config.LogVerbosity;
import com.rti.ndds.config.Logger;
import com.rti.perftest.IMessaging;
import com.rti.perftest.IMessagingCB;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;
import com.rti.perftest.gen.TestData_tTypeSupport;
import com.rti.perftest.gen.MAX_BINDATA_SIZE;
import com.rti.perftest.harness.PerfTest;
import com.rti.perftest.ini.Ini;


// ===========================================================================

/**
 * An implementation of the IMessaging interface based on RTI Data
 * Distribution Service.
 */
public final class RTIDDSImpl implements IMessaging {
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

    private int     _sendQueueSize = 50;
    private Duration_t _heartbeatPeriod = new Duration_t(0,0);
    private Duration_t _fastHeartbeatPeriod = new Duration_t (0,0);
    private int     _dataLen = 100;
    private int     _domainID = 1;
    private String  _nic = "";
    private String  _profileFile = "perftest.xml";
    private File    _configFile = new File("perftest.ini");
    private boolean _isReliable = true;
    private boolean _isMulticast = false;
    private boolean _AutoThrottle = false;
    private boolean _TurboMode = false;
    private boolean _UseTcpOnly = false;
    private int     _instanceCount = 1;
    private int     _instanceHashBuckets = -1;
    private int     _durability = 0;
    private boolean _directCommunication = true;
    private int     _batchSize = 0;
    private int     _keepDurationUsec = 1000;
    private boolean _usePositiveAcks = true;
    private boolean _useSharedMemory = false;
    private boolean _isDebug = false;
    private boolean _latencyTest = false;

    private DomainParticipantFactory _factory = null;
    private DomainParticipant        _participant = null;
    private Subscriber               _subscriber = null;
    private Publisher                _publisher = null;
    private DataReader               _reader = null;
    private String _typename = TestData_tTypeSupport.get_type_name();



    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    // --- From IMessaging: --------------------------------------------------

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
    }


    public int getBatchSize() {
        return _batchSize;
    }

    public int getMaxBinDataSize() {
        return MAX_BINDATA_SIZE.VALUE;
    }

    public void printCmdLineHelp() {
        String usage_string =
            /**************************************************************************/
            "\t-sendQueueSize <number> - Sets number of samples (or batches) in send\n" +
	        "\t                          queue, default 50\n" +
            "\t-domain <ID>            - RTI DDS Domain, default 1\n" +
            "\t-qosprofile <filename>  - Name of XML file for DDS Qos profiles, default\n" +
            "\t                          perftest.xml\n" +
            "\t-nic <ipaddr>           - Use only the nic specified by <ipaddr>.\n" +
	        "\t                          If unspecified, use all available interfaces\n" +
			"\t-multicast              - Use multicast to send data, default not to\n"+
            "\t                          use multicast\n" + 
            "\t-nomulticast            - Do not use multicast to send data (default),\n" +
            "\t-multicastAddress <ipaddr> - Multicast address to use for receiving \n" +
            "\t                          latency/announcement (pub) and \n" +
            "\t                          throughtput (sub) data. \n" +
            "\t                          If unspecified: latency 239.255.1.2,\n" +
            "\t                          announcement 239.255.1.100,\n" +
	        "\t                          throughput 239.255.1.1\n" +
            "\t-bestEffort             - Run test in best effort mode, default reliable\n" +
            "\t-batchSize <bytes>      - Size in bytes of batched message,\n" +
	        "\t                          default 0 (no batching)\n" +
            "\t-noPositiveAcks         - Disable use of positive acks in reliable\n" +
            "\t                          protocol, default use positive acks\n" +
            "\t-keepDurationUsec <usec> - Minimum time (us) to keep samples when\n" +
            "\t                          positive acks are disabled, default 1000 us\n" +
            "\t-enableSharedMemory     - Enable use of shared memory transport and\n" +
            "\t                          disable all the other transports, default\n"+
            "\t                			 shared memory not enabled\n" +
            "\t-enableTcpOnly          - Enable use of tcp transport and disable all\n"+
	        "\t                          the other transports, default do not use\n" +
	        "\t                          tcp transport\n" +
            "\t-heartbeatPeriod <sec>:<nanosec>     - Sets the regular heartbeat period\n" +
            "\t                          for the throughput DataWriter, default 0:0\n" +
            "\t                          (use XML QoS Profile value)\n" +
            "\t-fastHeartbeatPeriod <sec>:<nanosec> - Sets the fast heartbeat period\n" +
            "\t                          for the throughput DataWriter, default 0:0\n" +
            "\t                          (use XML QoS Profile value)\n" +
            "\t-durability <0|1|2|3>   - Set durability QOS, 0 - volatile,\n" +
            "\t                          1 - transient local, 2 - transient,\n" +
            "\t                          3 - persistent, default 0\n" +
            "\t-noDirectCommunication  - Use brokered mode for persistent durability\n" +
            "\t-instanceHashBuckets <#count> - Number of hash buckets for instances.\n" +
            "\t                          If unspecified, same as number of\n" +
	        "\t                          instances.\n" +
            "\t-waitsetDelayUsec <usec>   - UseReadThread related. Allows you to\n" +
            "\t                          process incoming data in groups, based on the\n" +
            "\t                          time rather than individually. It can be used \n" +
            "\t                          combined with -waitsetEventCount,\n" +
            "\t                          default 100 usec\n" +
            "\t-waitsetEventCount <count> - UseReadThread related. Allows you to\n" +
            "\t                          process incoming data in groups, based on the\n" +
            "\t                          number of samples rather than individually. It\n" +
            "\t                          can be used combined with -waitsetDelayUsec,\n" +
            "\t                          default 5\n" +
    		"\t-enableAutoThrottle     - Enables the AutoThrottling feature in the\n"+
    		"\t                          throughput DataWriter (pub)\n"+
    	    "\t-enableTurboMode        - Enables the TurboMode feature in the throughput\n"+
    		"\t                          DataWriter (pub)\n"
            ;
        System.err.print(usage_string);
    }


    public boolean initialize(int argc, String[] argv) {
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
        _factory.get_participant_qos_from_profile(qos, "PerftestQosLibrary" , "BaseProfileQos");

        // set participant to be able to receive and process packets up to 64K

        qos.transport_builtin.mask = TransportBuiltinKind.UDPv4;
        
        // set transports to use
        if (_UseTcpOnly) {
            qos.transport_builtin.mask = TransportBuiltinKind.MASK_NONE;
			PropertyQosPolicyHelper.add_property(qos.property,
                                                 "dds.transport.load_plugins", "dds.transport.TCPv4.tcp1",
                                                 false);
        }
        else{
            if (_useSharedMemory)
            {
                qos.transport_builtin.mask = TransportBuiltinKind.SHMEM;
            }
        }
        
        if (_AutoThrottle) {
        	PropertyQosPolicyHelper.add_property(qos.property,
        			"dds.domain_participant.auto_throttle.enable", "true",
                    false);
    	}
        
        if (!_UseTcpOnly) {
        
            if (_nic.length() > 0) {
                PropertyQosPolicyHelper.add_property(
                    qos.property,
                    "dds.transport.UDPv4.builtin.parent.allow_interfaces",
                    _nic,
                    false);
            }

            // Shmem transport properties
            final int receivedMessageCountMax = 2 * 1024 * 1024 / _dataLen;

            PropertyQosPolicyHelper.add_property(
                qos.property,
                "dds.transport.shmem.builtin.received_message_count_max",
                String.valueOf(receivedMessageCountMax),
                false);
        }

        // Creates the participant
        DomainParticipantListener listener = new DomainListener();
        _participant = _factory.create_participant(
            _domainID, qos, listener,
            (StatusKind.INCONSISTENT_TOPIC_STATUS |
             StatusKind.OFFERED_INCOMPATIBLE_QOS_STATUS |
             StatusKind.REQUESTED_INCOMPATIBLE_QOS_STATUS));

        if (_participant == null) {
            System.err.print("Problem creating participant.\n");
            return false;
        }

        // Register the types and create the topics
        TestData_tTypeSupport.register_type(
            _participant, _typename);

        // Create the Publisher and Subscriber
        {
            _publisher = _participant.create_publisher_with_profile(
                "PerftestQosLibrary", "BaseProfileQos", null, StatusKind.STATUS_MASK_NONE);

            if (_publisher == null) {
                System.err.print("Problem creating publisher.\n");
                return false;
            }

            _subscriber = _participant.create_subscriber_with_profile(
                "PerftestQosLibrary", "BaseProfileQos", null, StatusKind.STATUS_MASK_NONE);

            if (_subscriber == null) {
                System.err.print("Problem creating subscriber.\n");
                return false;
            }
        }

        return true;
    }


    public IMessagingWriter createWriter(String topicName) {
        DataWriter writer = null;
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

        if (PerfTest.THROUGHPUT_TOPIC_NAME.equals(topicName)) {
            if (_usePositiveAcks) {
                qosProfile = "ThroughputQos";
            } else {
                qosProfile = "NoAckThroughputQos";
            }
        } else if (PerfTest.LATENCY_TOPIC_NAME.equals(topicName)) {
            if (_usePositiveAcks) {
                qosProfile = "LatencyQos";
            } else {
                qosProfile = "NoAckLatencyQos";
            }
        } else if (PerfTest.ANNOUNCEMENT_TOPIC_NAME.equals(topicName)) {
            qosProfile = "AnnouncementQos";
        }
        else {
            System.err.println(
                    "topic name must either be " +
                    PerfTest.THROUGHPUT_TOPIC_NAME + " or " +
                    PerfTest.LATENCY_TOPIC_NAME  + " or " +
                    PerfTest.ANNOUNCEMENT_TOPIC_NAME);
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

        writer = _publisher.create_datawriter(
            topic, dwQos, null,
            StatusKind.STATUS_MASK_NONE);

        if (writer == null) {
            System.err.print("Problem creating writer.\n");
            return null;
        }

        RTIPublisher pub = new RTIPublisher(writer,_instanceCount);

        return pub;
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

        String qosProfile;
        if (PerfTest.THROUGHPUT_TOPIC_NAME.equals(topicName)) {
            if (_usePositiveAcks) {
                qosProfile = "ThroughputQos";
            } else {
                qosProfile = "NoAckThroughputQos";
            }
        } else if (PerfTest.LATENCY_TOPIC_NAME.equals(topicName)) {
            if (_usePositiveAcks) {
                qosProfile = "LatencyQos";
            } else {
                qosProfile = "NoAckLatencyQos";
            }
        } else if (PerfTest.ANNOUNCEMENT_TOPIC_NAME.equals(topicName)) {
            qosProfile = "AnnouncementQos";
        }
        else {
            System.err.println(
                    "topic name must either be " +
                    PerfTest.THROUGHPUT_TOPIC_NAME + " or " +
                    PerfTest.LATENCY_TOPIC_NAME  + " or " +
                    PerfTest.ANNOUNCEMENT_TOPIC_NAME);
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

        DataReader reader;
        if (callback != null) {
            ReceiverListener readerListener = new ReceiverListener(callback);
            reader = _subscriber.create_datareader(
                topic,
                drQos,
                readerListener,
                StatusKind.DATA_AVAILABLE_STATUS);
        } else {
            reader = _subscriber.create_datareader(
                topic, drQos, null, StatusKind.STATUS_MASK_NONE);
        }


        if (reader == null) {
            System.err.print("Problem creating reader.\n");
            return null;
        }

        if (PerfTest.LATENCY_TOPIC_NAME.equals(topicName) ||
            PerfTest.THROUGHPUT_TOPIC_NAME.equals(topicName)) {
            _reader = reader;
        }

        IMessagingReader sub = new RTISubscriber(reader);
        return sub;
    }



    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------

    private void configureWriterQos(
            String topicName, String qosProfile, DataWriterQos dwQos) {
        // Configure ACKs
        if (_usePositiveAcks) {
            dwQos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration.sec = (_keepDurationUsec * 1000) / 1000000000;
            dwQos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration.nanosec = (_keepDurationUsec * 1000) % 1000000000;
        }

        // Configure reliability
        if (!PerfTest.ANNOUNCEMENT_TOPIC_NAME.equals(topicName)) {
            if (_isReliable) {
                dwQos.reliability.kind = ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
 
            } else {
                dwQos.reliability.kind =
                    ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
            }
        }

        // These QOS's are only set for the Throughput datawriter
        if ("ThroughputQos".equals(qosProfile) ||
            "NoAckThroughputQos".equals(qosProfile)) {
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

            dwQos.protocol.rtps_reliable_writer.max_send_window_size = 
            		_sendQueueSize;
            dwQos.protocol.rtps_reliable_writer.min_send_window_size = 
            		_sendQueueSize;

            
        }

        if (("LatencyQos".equals(qosProfile) ||
             "NoAckLatencyQos".equals(qosProfile)) &&
             !_directCommunication &&
             (_durability == 2 ||
              _durability == 3)){

            if(_durability == 2){
                dwQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
            } else{
                dwQos.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
            }

            dwQos.durability.direct_communication = _directCommunication;
        }

        dwQos.resource_limits.max_instances = _instanceCount;
        dwQos.resource_limits.initial_instances = _instanceCount;

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
        // Configure reliability
        if (!PerfTest.ANNOUNCEMENT_TOPIC_NAME.equals(topicName)) {
            if (_isReliable) {
                drQos.reliability.kind = ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
            } else {
                drQos.reliability.kind = ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
            }
        }
      
        // These QOS's are only set for the Throughput reader
        if ("ThroughputQos".equals(qosProfile) ||
            "NoAckThroughputQos".equals(qosProfile)) {
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

        if (("LatencyQos".equals(qosProfile) ||
             "NoAckLatencyQos".equals(qosProfile)) &&
            !_directCommunication &&
            (_durability == 2 ||
             _durability == 3)){

            if(_durability == 2){
                drQos.durability.kind = DurabilityQosPolicyKind.TRANSIENT_DURABILITY_QOS;
            } else{
                drQos.durability.kind = DurabilityQosPolicyKind.PERSISTENT_DURABILITY_QOS;
            }
            drQos.durability.direct_communication = _directCommunication;
        }
           
        drQos.resource_limits.initial_instances = _instanceCount;
        drQos.resource_limits.max_instances = _instanceCount;

        if (_instanceCount > 1) {
            if (_instanceHashBuckets > 0) {
                drQos.resource_limits.instance_hash_buckets = _instanceHashBuckets;
            } else {
                drQos.resource_limits.instance_hash_buckets = _instanceCount;
            }
        }

        if (!_UseTcpOnly) {
        
            if (_isMulticast) {
                String multicast_addr;

                if (PerfTest.THROUGHPUT_TOPIC_NAME.equals(topicName)) {
                    multicast_addr = THROUGHPUT_MULTICAST_ADDR;
                } else if (PerfTest.LATENCY_TOPIC_NAME.equals(topicName)) {
                    multicast_addr = LATENCY_MULTICAST_ADDR;
                } else {
                    multicast_addr = ANNOUNCEMENT_MULTICAST_ADDR;
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
    }


    private boolean parseConfig(int argc, String[] argv) {

        // first scan for configFile
        for (int i = 0; i < argc; ++i) {
            if ("-configFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <fileName> after -configFile\n");
                    return false;
                }
                _configFile = new File(argv[i]);
            }
        }

        // now load configuration values from config file
        if (_configFile.isFile()) {
            Ini configSource;
            try {
                configSource = Ini.load(
                        _configFile, false /*case-sensitive*/);
            } catch (IOException e) {
                System.err.print("Problem loading configuration file.\n");
                System.err.println(e.getMessage());
                return false;
            }

            // parse generic section                
            Ini.Section config = configSource.section("perftest");
            if (config == null) {
                System.err.println(
                        "Could not find section [perftest] in file " +
                        _configFile +
                        ".");
                return false;
            }

            _dataLen = config.getInt("data length", _dataLen);
            _instanceCount = config.getInt("instances", _instanceCount);
            _latencyTest = config.getBoolean("run latency test", _latencyTest);
            _isDebug = config.getBoolean("is debug", _isDebug);

            // parse specific section
            config = configSource.section("RTIImpl");
            if (config == null)
            {
                System.err.println(
                        "Could not find section [RTIImpl] in file " +
                        _configFile +
                        ".");
                return false;
            }

            _sendQueueSize = config.getInt("send queue size", _sendQueueSize);
            _domainID = config.getInt("domain", _domainID);
            _profileFile = config.get("qos profile file", _profileFile);
            _nic = config.get("interface", _nic);
            _isMulticast = config.getBoolean("is multicast", _isMulticast);
            _isReliable = config.getBoolean("is reliable", _isReliable);
            _batchSize = config.getInt("batch size", _batchSize);
            _keepDurationUsec = config.getInt("keep duration usec", _keepDurationUsec);
            _usePositiveAcks = config.getBoolean("use positive acks", _usePositiveAcks);
            _useSharedMemory = config.getBoolean("enable shared memory", _useSharedMemory);
            _UseTcpOnly = config.getBoolean("enable tcp only", _UseTcpOnly);
            waitsetEventCount = config.getInt("waitset event count", waitsetEventCount);
            waitsetDelayUsec = config.getInt("waitset delay usec", waitsetDelayUsec);
            _durability = config.getInt("durability", _durability);
            _directCommunication = config.getBoolean("direct communication", _directCommunication);
            _heartbeatPeriod.sec = config.getInt("heartbeat period sec", _heartbeatPeriod.sec);
            _heartbeatPeriod.nanosec = config.getInt("heartbeat period nanosec", (int)_heartbeatPeriod.nanosec);
            _fastHeartbeatPeriod.sec = config.getInt("fast heartbeat period sec", _fastHeartbeatPeriod.sec);
            _fastHeartbeatPeriod.nanosec = config.getInt("fast heartbeat period nanosec",(int) _fastHeartbeatPeriod.nanosec);
            _instanceHashBuckets = config.getInt("instance hash buckets", _instanceHashBuckets);

        } else {
            System.err.println(
                    "Warning: config file doesn't exist: " + _configFile);
        }

        // now load everything else, command line params override config file
        for (int i = 0; i < argc; ++i) {
            if ("-dataLen".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <length> after -dataLen\n");
                    return false;
                }
                try {
                    _dataLen = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad dataLen\n");
                    return false;
                }
                if (_dataLen < PerfTest.OVERHEAD_BYTES) {
                    System.err.println("dataLen must be >= " + PerfTest.OVERHEAD_BYTES);
                    return false;
                }
                if (_dataLen > TestMessage.MAX_DATA_SIZE) {
                    System.err.println("dataLen must be <= " + TestMessage.MAX_DATA_SIZE);
                    return false;
                }
                if (_dataLen > MAX_BINDATA_SIZE.VALUE) {
                    System.err.println("dataLen must be <= " + MAX_BINDATA_SIZE.VALUE);
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
            } else if ("-qosprofile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing filename after -qosprofile\n");
                    return false;
                }
                _profileFile = argv[i];
            } else if ("-nomulticast".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isMulticast = false;
		    } else if ("-multicast".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isMulticast = true;
            } else if ("-multicastAddress".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <multicast address> after -multicastAddress\n");
                    return false;
                }
                THROUGHPUT_MULTICAST_ADDR = argv[i];
                LATENCY_MULTICAST_ADDR = argv[i];
                ANNOUNCEMENT_MULTICAST_ADDR = argv[i];
            } else if ("-nic".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <address> after -nic\n");
                    return false;
                }
                _nic = argv[i];
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
                if (_batchSize < 0) {
                    System.err.print("batch size cannot be negative\n");
                    return false;
                }
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
                if (_keepDurationUsec < 0) {
                    System.err.print("keep duration cannot be negative\n");
                    return false;
                }
            }
            else if ("-noPositiveAcks".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _usePositiveAcks = false;
            } else if ("-enableTcpOnly".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _UseTcpOnly  = true;
            } else if ("-enableSharedMemory".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _useSharedMemory = true;
            } else if ("-debug".toLowerCase().startsWith(argv[i].toLowerCase())) {
                Logger.get_instance().set_verbosity_by_category(
                    LogCategory.NDDS_CONFIG_LOG_CATEGORY_API,
                    LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
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
            	System.err.print("Auto Throttling enabled. Automatically adjusting the DataWriter\'s writing rate\n");
		_AutoThrottle = true;
            } else if ("-enableTurboMode".toLowerCase().startsWith(argv[i].toLowerCase())) {
            	_TurboMode = true;
            } else if ("-configFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                /* Ignore config file */
                ++i;
            } else {
                System.err.print(argv[i] + ": not recognized\n");
                return false;
            }
        }
        return true;
    }

}

// ===========================================================================
// End of $Id: RTIDDSImpl.java,v 1.2.2.1 2014/04/01 11:56:54 juanjo Exp $
