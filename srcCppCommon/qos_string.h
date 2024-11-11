#include <string.h>
#define PERFTEST_QOS_STRING_SIZE 29
#define PERFTEST_QOS_STRING_TOTAL_SIZE 28725
const char * PERFTEST_QOS_STRING[PERFTEST_QOS_STRING_SIZE] = {
"<?xml version=\"1.0\" encoding=\"iso-8859-1\"?>\n\n<!--\n(c) 2005-2024 Copyright, Real-Time Innovations, Inc. All rights reserved.\nSubject to Eclipse Public License v1.0; see LICENSE.md for details.\n-->\n\n<!--\nThis file contains the QoS configurations used by RTI Perftest, RTI's\nperformance test for measuring latency and throughput of RTI Connext DDS.\n\nThe format of this file is described in the RTI Connext Core Libraries\nand Utilities User's Manual in the chapter titled \"Configuring QoS with XML.\"\n-->\n\n<dds xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n  xsi:noNamespaceSchemaLocation=\"https://community.rti.com/schema/7.3.0/rti_dds_profiles.xsd\">\n\n  <qos_library name=\"PerftestQosLibrary\">\n\n    <!-- Auxiliary QoS Snippets used to build the profiles -->\n\n    <qos_profile name=\"Reliability.StrictReliable\">\n      <base_name>\n        <element>BuiltinQosSnippetLib::QosPolicy.Reliability.Reliable</element>\n        <element>BuiltinQosSnippetLib::QosPolicy.History.KeepAll</element>\n      </base_name>\n\n      <datawrite",
"r_qos>\n        <reliability>\n          <max_blocking_time>\n            <sec>DURATION_INFINITE_SEC</sec>\n            <nanosec>DURATION_INFINITE_NSEC</nanosec>\n          </max_blocking_time>\n        </reliability>\n\n        <protocol>\n          <rtps_reliable_writer>\n            <max_heartbeat_retries>LENGTH_UNLIMITED</max_heartbeat_retries>\n          </rtps_reliable_writer>\n        </protocol>\n      </datawriter_qos>\n\n\n      \n    </qos_profile>\n\n    <qos_profile name=\"Presentation.TopicPresentation\">\n      <!--\n      These changes are here to make sure that the DataReader can access\n      the samples in the order that they were written by the DataWriter.\n      By default, DDS does not guarantee any ordering when presenting samples\n      to the DataReader.\n\n      This profile is useful in perftest when using keyed types, since we write\n      in a round-robin fashion and we want to make sure that the DataReader reads\n      the samples in the same order that they were written, otherwise it would\n      detect some ",
"samples as lost.\n      --> \n      <publisher_qos>\n        <presentation>\n          <access_scope>TOPIC_PRESENTATION_QOS</access_scope>\n          <ordered_access>true</ordered_access>\n        </presentation>\n      </publisher_qos>\n\n      <subscriber_qos>\n        <presentation>\n          <access_scope>TOPIC_PRESENTATION_QOS</access_scope>\n          <ordered_access>true</ordered_access>\n        </presentation>\n      </subscriber_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.IncreaseResourceLimits\">\n\n      <participant_qos>\n        <resource_limits>\n          <!--\n          Maximum number of properties associated with the DomainParticipant. Default is 32.\n          -->\n          <participant_property_list_max_length>64</participant_property_list_max_length>\n          <!--\n          Maximum length for content filter property in the DomainParticipant. Default is 256.\n          -->\n          <contentfilter_property_max_length>512</contentfilter_property_max_length>\n        </resource_limits>\n      </parti",
"cipant_qos>\n\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Transport.TCPv4.BaseConfiguration\">\n\n      <participant_qos>\n        <property>\n          <value>\n            <element>\n              <name>dds.transport.TCPv4.tcp1.library</name>\n              <value>nddstransporttcp</value>\n            </element>\n            <element>\n              <name>dds.transport.TCPv4.tcp1.create_function</name>\n              <value>NDDS_Transport_TCPv4_create</value>\n            </element>\n            <element>\n              <name>dds.transport.TCPv4.tcp1.disable_nagle</name>\n              <value>1</value>\n            </element>\n          </value>\n        </property>\n      </participant_qos>\n    </qos_profile>\n\n\n    <qos_profile name=\"Perftest.Transport.TCPv4.IncreaseSendReceiveBuffers\">\n      <participant_qos>\n        <property>\n          <value>\n            <element>\n              <name>dds.transport.TCPv4.tcp1.send_socket_buffer_size</name>\n              <value>1048576</value>\n            </element>\n            <elem",
"ent>\n              <name>dds.transport.TCPv4.tcp1.recv_socket_buffer_size</name>\n              <value>2097152</value>\n            </element>\n          </value>\n        </property>\n      </participant_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Transport.UDPv4.IncreaseSendReceiveBuffers\">\n      <!-- \n        The default values for the send and receive socket buffer sizes are 131072 bytes.\n        We increase them to 1MB and 2MB respectively to be able to receive and store\n        more packets in the buffers before processing them and avoid packet losses.\n      -->\n      <participant_qos>\n        <property>\n          <value>\n            <element>\n              <name>dds.transport.UDPv4.builtin.send_socket_buffer_size</name>\n              <value>1048576</value>\n            </element>\n            <element>\n              <name>dds.transport.UDPv4.builtin.recv_socket_buffer_size</name>\n              <value>2097152</value>\n            </element>\n          </value>\n        </property>\n      </participant",
"_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.FlowController.1Gbps\">\n      <participant_qos>\n        <property>\n          <value>\n            <element>\n              <name>dds.flow_controller.token_bucket.1Gbps.token_bucket.max_tokens</name>\n              <value>30</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.1Gbps.token_bucket.tokens_added_per_period</name>\n              <value>20</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.1Gbps.token_bucket.bytes_per_token</name>\n              <value>65536</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.1Gbps.token_bucket.period.sec</name>\n              <value>0</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.1Gbps.token_bucket.period.nanosec</name>\n              <value>10000000</value>\n            </element>\n          </value",
">\n        </property>\n      </participant_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.FlowController.10Gbps\">\n      <participant_qos>\n        <property>\n          <value>\n            <element>\n              <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.max_tokens</name>\n              <value>300</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.tokens_added_per_period</name>\n              <value>200</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.bytes_per_token</name>\n              <value>65536</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.period.sec</name>\n              <value>0</value>\n            </element>\n            <element>\n              <name>dds.flow_controller.token_bucket.10Gbps.token_bucket.period.nanosec</name>\n              <value>10000000",
"</value>\n            </element>\n          </value>\n        </property>\n      </participant_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.ReliableSettings\">\n      <datawriter_qos>\n        <protocol>\n          <rtps_reliable_writer>\n\n            <!--\n            Some of the settings used here depend on the specific\n            scenario and sample size, hence, they are modified in RTI Perftest's\n            code. Still, we document them here.\n            -->\n\n            <!--\n            <low_watermark>_SendQueueSize * 0.1</low_watermark>\n\n            When the writer's cache gets down to this number of samples, it\n            will slow the rate at which it sends heartbeats to readers.\n            -->\n\n            <!--\n            <high_watermark>_SendQueueSize * 0.9</high_watermark>\n\n            When the writer's cache is filled to this level, it will begin\n            sending heartbeats at a faster rate in order to get faster\n            acknowledgements (positive or negative) of its samples to allow",
" it\n            to empty its cache and avoid blocking.\n            -->\n\n            <!--\n            <heartbeats_per_max_samples>_SendQueueSize * 0.1</heartbeats_per_max_samples>\n\n            Governs how often heartbeats are \"piggybacked\" on data samples.\n            Piggybacking heartbeats will get a faster positive or negative\n            acknowledgement so that repairs can happen faster if needed.\n            -->\n\n            <!--\n            <min_send_window_size>datawriter_qos.resource_limits.max_samples</min_send_window_size>\n            <max_send_window_size>datawriter_qos.resource_limits.max_samples</max_send_window_size>\n\n            Minimum and maximum size of send window of unacknowledged samples. In\n            this case we set both to the same number to avoid having to grow the\n            send window.\n            -->\n\n            <!--\n            If the number of samples in the writer's cache hasn't risen to\n            high_watermark, this is the rate at which the DataWriter will\n            se",
"nd out periodic heartbeats.\n            -->\n            <heartbeat_period>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>10000000</nanosec>\n            </heartbeat_period>\n\n            <!--\n            If the number of samples in the writer's cache has risen to\n            high_watermark, and has not yet fallen to low_watermark, this is\n            the rate at which the writer will send periodic heartbeats to\n            its readers.\n            -->\n            <fast_heartbeat_period>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>1000000</nanosec>\n            </fast_heartbeat_period>\n\n            <!--\n            If a durable reader starts up after the writer already has some\n            samples in its cache, this is the rate at which it will heartbeat\n            the new reader. It should generally be a shorter period of time\n            than the normal heartbeat period in order to help the new reader\n            catch up.\n            -->\n            <late_joiner_heart",
"beat_period>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>10000000</nanosec>\n            </late_joiner_heartbeat_period>\n\n            <!--\n            The number of times a reliable writer will send a heartbeat to\n            a reader without receiving a response before it will consider the\n            reader to be inactive and no longer await acknowledgements before\n            discarding sent data.\n            -->\n            <max_heartbeat_retries>LENGTH_UNLIMITED</max_heartbeat_retries>\n\n            <!--\n            When a DataWriter receives a negative acknowledgement (NACK) from\n            a DataReader for a particular data sample, it will send a repair\n            packet to that reader.\n\n            The amount of time the writer waits between receiving the NACK and\n            sending the repair will be a random value between the minimum\n            and maximum values specified here. Narrowing the range, and\n            shifting it towards zero, will make the writer more reactive.",
"\n            However, by leaving some delay you increase the chance that the\n            writer will learn of additional readers that missed the same data,\n            in which case it will be able to send a single multicast repair\n            instead of multiple unicast repairs, thereby using the available\n            network bandwidth more efficiently. The higher the fanout in your\n            system (i.e., the more readers per writer), and the greater the\n            load on your network, the more you should consider specifying a\n            non-zero delay here.\n            -->\n            <min_nack_response_delay>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>DURATION_ZERO_NSEC</nanosec>\n            </min_nack_response_delay>\n            <max_nack_response_delay>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>DURATION_ZERO_NSEC</nanosec>\n            </max_nack_response_delay>\n\n            <!--\n            When positive acknowledgements have been disabled, the DataWri",
"ter will\n            consider samples as positively \"acknowledged\" after this duration has\n            elapsed if it has not heard otherwise. Disabling positive acknowledgements\n            has the benefit of reducing network usage. Using this feature must be\n            considered carefully because it changes how and when a DataWriter\n            considers a sample as acknowledged.\n\n            We disable the positive acknowledgements in perftest by using:\n            -disablePositiveAcks. You can alternatively enable them via QoS by\n            adding <disable_positive_acks>true</disable_positive_acks> in the\n            <protocol> QoS of the DW and DR of the ThroughputQoS and LatencyQoS.\n\n            The default value of _KeepDurationUsec can be configured here or via the\n            command line parameter -keepDuration.\n            -->\n            <disable_positive_acks_min_sample_keep_duration>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>100000</nanosec>\n            </disable_positi",
"ve_acks_min_sample_keep_duration>\n          </rtps_reliable_writer>\n        </protocol>\n\n      </datawriter_qos>\n\n      <datareader_qos>\n        <protocol>\n          <rtps_reliable_reader>\n            <!--\n            When the DataReader receives a heartbeat from a DataWriter\n            (indicating what sequence numbers it has published), the following\n            parameters indicate how long it will wait before replying with\n            a positive (assuming they aren't disabled) or negative\n            acknowledgement.\n\n            The time the reader waits will be a random duration between\n            the minimum and maximum values. Narrowing this range, and shifting\n            it towards zero, will make the system more reactive. However, it\n            increases the chance of (N)ACK spikes. The higher the fanout in\n            your system (i.e., the number of readers per writer), the more\n            you should consider specifying a range here.\n            -->\n            <min_heartbeat_response_delay>\n ",
"             <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>DURATION_ZERO_NSEC</nanosec>\n            </min_heartbeat_response_delay>\n            <max_heartbeat_response_delay>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>DURATION_ZERO_NSEC</nanosec>\n            </max_heartbeat_response_delay>\n          </rtps_reliable_reader>\n        </protocol>\n      </datareader_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Throughput.ReliableSettings\">\n      <base_name>\n        <element>PerftestQosLibrary::Perftest.ReliableSettings</element>\n      </base_name>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Throughput.ResourceLimits\">\n\n      <datawriter_qos>\n        <!-- These resource limits are changed in code. -->\n      </datawriter_qos>\n\n      <datareader_qos>\n        <resource_limits>\n          <!--\n          The initial and maximum number of data samples. The middleware will\n          make sure to allocate space for the initial_samples, and then if\n          needed, it will gr",
"ow the allocated memory up to a point where it\n          supports max_samples.\n\n          For the initial number of samples we choose a number that should be\n          enough for most use-cases (therefore no need to grow), but that\n          should not affect the memory consumption by reserving too much\n          memory.\n          -->\n          <max_samples>10000</max_samples>\n          <initial_samples>128</initial_samples>\n\n          <!--\n          The maximum number of samples that can be stored for a single\n          instance. If the throughput topic is not keyed, there is only a\n          single instance, so this value should always be set the same\n          as max_samples.\n\n          For a keyed topic, you might want to use this parameter to institute\n          a degree of \"fairness\" among the instances.\n          -->\n          <max_samples_per_instance>10000</max_samples_per_instance>\n        </resource_limits>\n\n        <reader_resource_limits>\n          <!--\n          The maximum number of samples tha",
"t Connext will store from a\n          single DataWriter. If you run this application with only a single\n          DataWriter (that is, in a one-to-one or one-to-many configuration),\n          there is no reason for this value to be set to anything less than\n          max_samples. If you have many writers and need to institute\n          a degree of \"fairness\" among them, you can decrease this value.\n          -->\n          <max_samples_per_remote_writer>10000</max_samples_per_remote_writer>\n\n          <!--\n          The maximum number of data samples that the application can receive\n          from Connext in a single call to DataReader::read() or\n          take(). If more data exists in the middleware, the application will\n          need to issue multiple read()/take() calls.\n\n          When reading data using listeners, the expected number of samples\n          available for delivery in a single take() call is typically small:\n          usually just one in the case of unbatched data, or the number of\n         ",
" samples in a single batch in the case of batched data. When polling\n          for data or using Waitsets, however, multiple samples (or batches)\n          could be retrieved at once, depending on the data rate.\n\n          A larger value for this parameter makes the API simpler to use, at\n          the expense of some additional memory consumption.\n          -->\n          <max_samples_per_read>65536</max_samples_per_read>\n        </reader_resource_limits>\n      </datareader_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Throughput.BatchingConfig\">\n      <datawriter_qos>\n        <!--\n        When sending many small data-samples, you can increated network efficiency\n        by batching multiple samples together in a single protocol-level message\n        (usually corresponding to a single network datagram). Batching can offer very\n        substantial throughput gains, but often at the expense of latency, although\n        in some configurations, the latency penalty can be very small or zero,\n        pos",
"sibly even negative.\n        -->\n\n        <!--\n        Important: This setting is configured automatically within the source code,\n        but only if batching is enabled. _SendQueueSize = 50 (default)\n        <writer_resource_limits>\n          <max_batches>_SendQueueSize</max_batches>\n        </writer_resource_limits>\n        -->\n\n        <batch>\n          <!--\n          This profile does not enable batching, although the remaining\n          batching settings are configured as if it did. To enable the batch\n          configuration below, turn batching on using the app's command-line.\n          -->\n          <enable>false</enable>\n\n          <!--\n          Batches can be \"flushed\" to the network based on a maximum size.\n          This size can be based on the total number of bytes in the\n          accumulated data samples, the total number of bytes in the\n          accumulated sample meta-data (e.g., timestamps, sequence numbers,\n          etc.), and/or the number of samples. Whenever the first of these\n     ",
"     limits is reached, the batch will be flushed.\n\n          Important: This setting is configured automatically within the source code.\n          <max_data_bytes>_BatchSize</max_data_bytes>\n          -->\n          <max_meta_data_bytes>LENGTH_UNLIMITED</max_meta_data_bytes>\n          <max_samples>LENGTH_UNLIMITED</max_samples>\n\n          <!--\n          The middleware will associate a source timestamp with a batch when\n          it is started. The duration below indicates the amount of time that\n          may pass before the middleware will insert an additional timestamp\n          into the middle of an existing batch.\n\n          Shortening this duration can give readers increased timestamp\n          resolution. However, lengthening this duration\n          decreases the amount of meta-data on the network, potentially\n          improving throughput, especially if the data samples are very small.\n          If this delay is set to an infinite time period, timestamps will\n          be inserted only once per batch.",
" Furthermore, the middleware will\n          not need to check the time with each sample in the batch, reducing\n          the amount of computation on the send path and potentially improving\n          both latency and throughput performance.\n          -->\n          <source_timestamp_resolution>\n            <sec>DURATION_INFINITE_SEC</sec>\n            <nanosec>DURATION_INFINITE_NSEC</nanosec>\n          </source_timestamp_resolution>\n\n          <!--\n          The maximum flush delay. A batch is flushed automatically after the\n          delay specified by this parameter. As its value is DURATION_INFINITE,\n          the flush event will be triggered by max_data_bytes.\n          -->\n          <max_flush_delay>\n            <sec>DURATION_INFINITE_SEC</sec>\n            <nanosec>DURATION_INFINITE_NSEC</nanosec>\n          </max_flush_delay>\n\n          <!--\n          By default, the same DataWriter can be used from multiple threads.\n          If you know that your application will only write data from a single\n          ",
"thread, you can switch off a level of locking that occurs when\n          samples are added to a batch. When sending very small samples very\n          fast, this decreased overhead can improve performance.\n\n          However, even in the case of single-threaded access, the impact of\n          locking can be negligible, and deactivating the lock puts your\n          application at risk of memory corruption if multiple threads do\n          write to the same DataWriter - either without your knowledge or as\n          a result of application maintenance. Therefore, RTI recommends that\n          you only set thread_safe_write to false after detailed testing has\n          confirmed that your application does indeed behave correctly and\n          with improved performance.\n          -->\n          <thread_safe_write>false</thread_safe_write>\n        </batch>\n      </datawriter_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Latency.ReliableSettings\">\n      <base_name>\n        <element>PerftestQosLibrary::Perfte",
"st.ReliableSettings</element>\n      </base_name>\n\n      <datawriter_qos>\n        <protocol>\n          <rtps_reliable_writer>\n            <low_watermark>10</low_watermark>\n            <high_watermark>100</high_watermark>\n            <heartbeats_per_max_samples>1000</heartbeats_per_max_samples>\n            <min_send_window_size>LENGTH_UNLIMITED</min_send_window_size>\n            <max_send_window_size>LENGTH_UNLIMITED</max_send_window_size>\n          </rtps_reliable_writer>\n        </protocol>\n      </datawriter_qos>\n\n      <datareader_qos>\n        <protocol>\n          <rtps_reliable_reader>\n            <heartbeat_suppression_duration>\n              <sec>DURATION_ZERO_SEC</sec>\n              <nanosec>DURATION_ZERO_NSEC</nanosec>\n            </heartbeat_suppression_duration>\n          </rtps_reliable_reader>\n        </protocol>\n      </datareader_qos>\n\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Latency.ResourceLimits\">\n\n      <datawriter_qos>\n        <resource_limits>\n          <max_samples>LENGTH_UNLIMI",
"TED</max_samples>\n          <initial_samples>100</initial_samples>\n          <max_samples_per_instance>LENGTH_UNLIMITED</max_samples_per_instance>\n        </resource_limits>\n      </datawriter_qos>\n\n      <datareader_qos>\n        <!--\n        The number of samples for which the middleware will set aside space.\n        See the comments above for more information.\n        -->\n        <resource_limits>\n          <max_samples>100</max_samples>\n          <initial_samples>100</initial_samples>\n          <max_samples_per_instance>100</max_samples_per_instance>\n        </resource_limits>\n\n        <reader_resource_limits>\n          <max_samples_per_remote_writer>100</max_samples_per_remote_writer>\n        </reader_resource_limits>\n      </datareader_qos>\n    </qos_profile>\n\n    <qos_profile name=\"Perftest.Security\">\n      <participant_qos>\n        <property>\n          <value>\n            <!--\n              This QoS is applied when using security. Default starting in 7.0.0\n              is aes-256-gcm\n            -->\n ",
"           <element>\n              <name>com.rti.serv.secure.cryptography.encryption_algorithm</name>\n              <value>aes-128-gcm</value>\n            </element>\n\n          </value>\n        </property>\n\n      </participant_qos>\n    </qos_profile>\n\n    <!--\n    Base QoS Profile:\n    Used by the rest of the profiles. Participants will be created using this profile.\n    -->\n    <qos_profile name=\"BaseProfileQos\">\n\n      <base_name>\n        <!-- General Settings that affect to all the Perftest profiles -->\n        <element>PerftestQosLibrary::Presentation.TopicPresentation</element>\n\n        <!-- Settings related to Transport -->\n        <element>PerftestQosLibrary::Perftest.Transport.UDPv4.IncreaseSendReceiveBuffers</element>\n        <element>PerftestQosLibrary::Perftest.Transport.TCPv4.BaseConfiguration</element>\n        <element>PerftestQosLibrary::Perftest.Transport.TCPv4.IncreaseSendReceiveBuffers</element>\n\n        <!-- Some Resource Limits had to be increased in order to run our application -->\n       ",
" <element>PerftestQosLibrary::Perftest.IncreaseResourceLimits</element>\n\n        <!-- Load the flow controllers for asynchronous publishing-->\n        <element>PerftestQosLibrary::Perftest.FlowController.1Gbps</element>\n        <element>PerftestQosLibrary::Perftest.FlowController.10Gbps</element>\n\n        <!-- For security some settings are modified from the default behavior -->\n        <element>PerftestQosLibrary::Perftest.Security</element>\n      </base_name>\n\n      <participant_qos>\n        <participant_name>\n          <name>Perftest Participant</name>\n        </participant_name>\n\n        <!--\n        Some of the properties we set may not be recognized if we are not \n        loading all the libraries (e.g: the tcp transport settings), in order to\n        avoid errors in the application we just skip their validation.\n        -->\n        <property>\n          <value>\n            <element>\n              <name>dds.participant.property_validation_action</name>\n              <value>1</value>\n            </element",
">\n          </value>\n        </property>\n      </participant_qos>\n    </qos_profile>\n\n    <!--\n    Throughput QoS Profile:\n    This is the profile used by the throughput-testing portion of the application,\n    in the topic that sends the pings (from the Perftest publisher to the Perftest\n    subscriber).\n    -->\n    <qos_profile name=\"ThroughputQos\" base_name=\"BaseProfileQos\">\n\n      <base_name>\n        <element>PerftestQosLibrary::Reliability.StrictReliable</element>\n        <element>PerftestQosLibrary::Perftest.Throughput.ReliableSettings</element>\n        <element>PerftestQosLibrary::Perftest.Throughput.BatchingConfig</element>\n        <element>PerftestQosLibrary::Perftest.Throughput.ResourceLimits</element>\n      </base_name>\n  \n      <datawriter_qos>\n        <writer_resource_limits>\n          <max_remote_reader_filters>256</max_remote_reader_filters>\n        </writer_resource_limits>\n      </datawriter_qos>\n\n    </qos_profile>\n\n    <!--\n    Latency QoS Profile:\n    This is the profile used by the latency",
"-testing portion of the application,\n    in the topic that sends the pongs (from the Perftest subscriber to the Perftest\n    publisher).\n    -->\n    <qos_profile name=\"LatencyQos\" base_name=\"BaseProfileQos\">\n\n      <base_name>\n        <element>PerftestQosLibrary::Reliability.StrictReliable</element>\n        <element>PerftestQosLibrary::Perftest.Latency.ReliableSettings</element>\n        <element>PerftestQosLibrary::Perftest.Latency.ResourceLimits</element>\n      </base_name>\n\n      <datareader_qos>\n        <history>\n          <kind>KEEP_LAST_HISTORY_QOS</kind>\n        </history>\n      </datareader_qos>\n\n    </qos_profile>\n\n    <!--\n    Announcement QoS Profile:\n    This profile is used by the test harness for the announcement topic,\n    which is used to synchronize the publishing and subscribing size\n    to start the test.\n    -->\n    <qos_profile name=\"AnnouncementQos\" base_name=\"LatencyQos\">\n\n      <base_name>\n        <element>BuiltinQosSnippetLib::QosPolicy.Durability.TransientLocal</element>\n      </base_",
"name>\n\n    </qos_profile>\n\n  </qos_library>\n</dds>" };

#define PERFTEST_QOS_STRING_asString(str) {\
       int i;\
       (str)[0] = 0;\
       for(i = 0; i < PERFTEST_QOS_STRING_SIZE; ++i) {\
            strcat(str, PERFTEST_QOS_STRING[i]);\
       }\
}
