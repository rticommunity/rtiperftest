/* $Id: RTIDDSImpl.cs,v 1.2.2.1 2014/04/01 11:56:52 juanjo Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

 modification history
 --------------------
 5.1.0.9,27mar14,jmc PERFTEST-27 Fixing resource limits when using
                     Turbo Mode
 5.1.0,19dec13,jmc PERFTEST-3 Added autothrottle and turbomode
 5.1.0,19dec13,jmc PERFTEST-2 window size in batching path and
                   domain id now is 1
 1.1b,29aug13,jmc CORE-5854 multicast disabled by default
 1.1b,29aug13,jmc CORE-5919 Moved hardcoded QoS to XML file when
                  possible
 1.1b,29aug13,jmc CORE-5867 transport builtin mask to only shmem
 1.0a,13jul10,jsr Added WaitForPingResponse with timeout
 1.0a,07jul10,jsr Fixed NotifyPingResponse and WaitForPingResponse to
                  return bool instead of void
 1.0a,29jun10,jsr Fix heartbeat and fastheartbeat for windows
 1.0a,14may10,jsr Added LatencyTest option
 1.0a,10mar10,gn  Ported tcp related changes form c++
 1.0a,26may09,fcs Fixed test finalization for keyed topics
 1.0a,21may09,fcs Optimized send op for unkeyed topics
 1.0a,14may09,fcs Added instances to INI
 1.0a,14may09,fcs Fixed command-line arguments processing
 1.0a,08may09,jsr Fixed default profile names
 1.0a,29apr09,jsr Added detection of wrong command line parameter
 1.0b,23apr09,jsr Changed to stderr the error and status messages
 1.0b,21apr09,jsr Reformat the help menu
 1.0b,17apr09,jsr Fixed #12322, added -waitsetDelayUsec and -waitsetEventCount
                  command line option
 1.0b,16apr09,jsr Fixing send method
 1.0b,23jan09,jsr Added heartbeat period and fastheartbeat period option
 1.0a,20aug08,eys Move InstanceCount to perftest.h
 1.0c,11aug08,ch  Durability
 1.0b,13may08,rbw Updated error handling code
 1.0c,14aug08,ch  optimized changing the key value before write
 1.0c,11aug08,ch  Key support, multi-instances, durability
 1.0c,17jun08,rbw Fixed presentation QoS policy bug introduced earlier
 1.0c,30may08,rbw Updated to reflect .Net API changes
 1.0a,13may08,hhw Now using topic presentation scope.
 1.0a,01may08,hhw Removed singleCore option.
                  Increased shared memory buffer for shm tests
                  KEEP_ALL is used for both reliable/unreliable.
 1.0a,22apr08,fcs Fixed batching/best_effort scenario
 1.0a,18mar08,hhw Created.
===================================================================== */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

namespace PerformanceTest
{
    class RTIDDSImpl : IMessaging
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
         * GetMaxBinDataSize
         */
        public int GetMaxBinDataSize()
        {
            return MAX_BINDATA_SIZE.VALUE;
        }

        /*********************************************************
         * PrintCmdLineHelp
         */
        public void PrintCmdLineHelp()
        {
            const string usage_string =
            /**************************************************************************/
            "\t-sendQueueSize <number> - Sets number of samples (or batches) in send\n" +
	        "\t                          queue, default 50\n" +
            "\t-domain <ID>            - RTI DDS Domain, default 1\n      " +
            "\t-qosprofile <filename>  - Name of XML file for DDS Qos profiles,\n" +
            "\t                          default perftest.xml\n" +
            "\t-nic <ipaddr>           - Use only the nic specified by <ipaddr>,\n" +
	        "\t                          If unspecificed, use all available interfaces\n" +
            "\t-multicast              - Use multicast to send data, default not to\n"+
            "\t                          use multicast\n"+
            "\t-nomulticast            - Do not use multicast to send data (default)\n"+
            "\t-multicastAddress <ipaddr> - Multicast address to use for receiving\n" +
            "\t                          latency/announcement (pub) or \n" +
            "\t                          throughtput (sub) data.\n" +
	        "\t                          If unspecified: latency 239.255.1.2,\n" +
	        "\t                          announcement 239.255.1.100,\n" +
	        "\t                          throughput 239.255.1.1\n" +
            "\t-bestEffort             - Run test in best effort mode,\n"+
            "\t                          default reliable\n" +
            "\t-batchSize <bytes>      - Size in bytes of batched message, default 0\n" +
            "\t                          (no batching)\n" +
            "\t-noPositiveAcks         - Disable use of positive acks in reliable\n" +
            "\t                          protocol, default use positive acks\n" +
            "\t-keepDurationUsec <usec> - Minimum time (us) to keep samples when\n" +
            "\t                          positive acks are disabled, default 1000 us\n" +
            "\t-enableSharedMemory     - Enable use of shared memory transport and,\n" +
            "\t                          disable all the other transports, default\n" + 
            "\t                          shared memory not enabled\n" +
            "\t-enableTcpOnly          - Enable use of tcp transport and disable all\n" +
	        "\t                          the other transports, default do not use\n" +
	        "\t                          tcp transport\n" +
            "\t-heartbeatPeriod <sec>:<nanosec>     - Sets the regular heartbeat\n" +
            "\t                          period for throughput DataWriter,\n" +
            "\t                          default 0:0 (use XML QoS Profile value)\n" +
            "\t-fastHeartbeatPeriod <sec>:<nanosec> - Sets the fast heartbeat period\n" +
            "\t                          for the throughput DataWriter,\n" +
            "\t                          default 0:0 (use XML QoS Profile value)\n" +
            "\t-durability <0|1|2|3>   - Set durability QOS,\n"+
            "\t                          0 - volatile, 1 - transient local,\n"+
            "\t                          2 - transient, 3 - persistent, default 0\n" +
            "\t-noDirectCommunication  - Use brokered mode for persistent durability\n" +
            "\t-instanceHashBuckets <#count> - Number of hash buckets for instances.\n" +
            "\t                          If unspecified, same as number of\n" +
	        "\t                          instances\n" +
            "\t-waitsetDelayUsec <usec>   - UseReadThread related. Allows you to\n" +
            "\t                          process incoming data in groups, based on the\n" +
            "\t                          time rather than individually. It can be used\n" +
            "\t                          combined with -waitsetEventCount,\n" +
	        "\t                          default 100 usec\n" +
            "\t-waitsetEventCount <count> - UseReadThread related. Allows you to\n" +
            "\t                          process incoming data in groups, based on the\n" +
            "\t                          number of samples rather than individually.\n" +
            "\t                          It can be used combined with\n" +
	        "\t                          -waitsetDelayUsec, default 5\n" + 
    		"\t-enableAutoThrottle     - Enables the AutoThrottling feature in the\n" +
			"\t                          throughput DataWriter (pub)\n" +
		    "\t-enableTurboMode        - Enables the TurboMode feature in the throughput\n" +
			"\t                          DataWriter (pub)\n"
            ;

            Console.Error.Write(usage_string);
        }

        /*********************************************************
         * ParseConfig
         */
        bool ParseConfig(int argc, string[] argv)
        {
            // first scan for configFile
            for (int i = 0; i < argc; ++i)
            {
                if ("-configFile".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <fileName> after -configFile\n");
                        return false;
                    }
                    _ConfigFile = argv[i];
                }
            }

            // now load configuration values from config file
            if (_ConfigFile.Length > 0)
            {
                Nini.Config.IConfig config;
                Nini.Config.IniConfigSource configSource = new Nini.Config.IniConfigSource();

                try
                {
                    configSource.Load(_ConfigFile);
                }
                catch (System.IO.IOException e)
                {
                    Console.Error.Write("Problem loading configuration file.\n");
                    Console.Error.WriteLine(e.Message);
                    return false;
                }

                configSource.CaseSensitive = false;

                // parse generic section                
                config = configSource.Configs["perftest"];

                if (config == null)
                {
                    Console.Error.Write("Could not find section [perftest] in file " + _ConfigFile + ".\n");
                    return false;
                }

                _DataLen = config.GetInt("data length", _DataLen);
                _InstanceCount  = config.GetInt("instances", _InstanceCount);
                _LatencyTest = config.GetBoolean("run latency test", _LatencyTest);
                _IsDebug = config.GetBoolean("is debug", _IsDebug);

                // parse specific section
                config = configSource.Configs["RTIImpl"];

                if (config == null)
                {
                    Console.Error.Write("Could not find section [RTIImpl] in file " + _ConfigFile + ".\n");
                    return false;
                }

                _SendQueueSize = config.GetInt("send queue size", _SendQueueSize);
                _DomainID = config.GetInt("domain", _DomainID);
                _ProfileFile = config.Get("qos profile file", _ProfileFile);
                _Nic = config.Get("interface", _Nic);
                _IsMulticast = config.GetBoolean("is multicast", _IsMulticast);
                _IsReliable = config.GetBoolean("is reliable", _IsReliable);
                _BatchSize = config.GetInt("batch size", _BatchSize);
                _KeepDurationUsec = (uint)config.GetInt("keep duration usec", (int)_KeepDurationUsec);
                _UsePositiveAcks = config.GetBoolean("use positive acks", _UsePositiveAcks);
                _UseSharedMemory = config.GetBoolean("enable shared memory", _UseSharedMemory);
                _UseTcpOnly = config.GetBoolean("enable tcp only", _UseTcpOnly);
                _WaitsetEventCount = config.GetInt("waitset event count", _WaitsetEventCount);
                _WaitsetDelayUsec = (uint)config.GetInt("waitset delay usec", (int)_WaitsetDelayUsec);
                _Durability = config.GetInt("durability", _Durability);
                _DirectCommunication = config.GetBoolean("direct communication", _DirectCommunication);
                _HeartbeatPeriod.sec = config.GetInt("heartbeat period sec", _HeartbeatPeriod.sec);
                _HeartbeatPeriod.nanosec = (uint)config.GetInt("heartbeat period nanosec", (int)_HeartbeatPeriod.nanosec);
                _FastHeartbeatPeriod.sec = config.GetInt("fast heartbeat period sec", _FastHeartbeatPeriod.sec);
                _FastHeartbeatPeriod.nanosec = (uint)config.GetInt("fast heartbeat period nanosec", (int)_FastHeartbeatPeriod.nanosec);
		_InstanceHashBuckets = config.GetInt(
		    "instance hash buckets", _InstanceHashBuckets);
            }

            // now load everything else, command line params override config file
            for (int i = 0; i < argc; ++i)
            {
                if ("-dataLen".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <length> after -dataLen\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _DataLen))
                    {
                        Console.Error.Write("Bad dataLen\n");
                        return false;
                    }
                    if (_DataLen < perftest_cs.OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("dataLen must be >= " + perftest_cs.OVERHEAD_BYTES);
                        return false;
                    }
                    if (_DataLen > TestMessage.MAX_DATA_SIZE)
                    {
                        Console.Error.WriteLine("dataLen must be <= " + TestMessage.MAX_DATA_SIZE);
                        return false;
                    }
                    if (_DataLen > MAX_BINDATA_SIZE.VALUE)
                    {
                        Console.Error.WriteLine("dataLen must be <= " + MAX_BINDATA_SIZE.VALUE);
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
                        _HeartbeatPeriod.nanosec = uint.Parse(st[1]);
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
                        _FastHeartbeatPeriod.nanosec = uint.Parse(st[1]);
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
                else if ("-qosprofile".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing filename after -qosprofile\n");
                        return false;
                    }
                    _ProfileFile = argv[i];
                }
                else if ("-multicast".StartsWith(argv[i], true, null))
                {
                    _IsMulticast = true;
                }
                else if ("-nomulticast".StartsWith(argv[i], true, null))
                {
                    _IsMulticast = false;
                }
                else if ("-multicastAddress".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <multicast address> after -multicastAddress\n");
                        return false;
                    }
                    THROUGHPUT_MULTICAST_ADDR = argv[i];
                    LATENCY_MULTICAST_ADDR = argv[i];
                    ANNOUNCEMENT_MULTICAST_ADDR = argv[i];
                }
                else if ("-nic".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <address> after -nic\n");
                        return false;
                    }
                    _Nic = argv[i];
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
                    if (_BatchSize < 0)
                    {
                        Console.Error.Write("batch size cannot be negative\n");
                        return false;
                    }
                }
                else if ("-keepDurationUsec".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <usec> after -keepDurationUsec\n");
                        return false;
                    }
                    if (!UInt32.TryParse(argv[i], out _KeepDurationUsec))
                    {
                        Console.Error.Write("Bad usec for keep duration\n");
                        return false;
                    }
                    if (_KeepDurationUsec < 0)
                    {
                        Console.Error.Write(" keep duration usec cannot be negative or null\n");
                        return false;
                    }
                }
                else if ("-noPositiveAcks".StartsWith(argv[i], true, null))
                {
                    _UsePositiveAcks = false;
                }
                else if ("-enableSharedMemory".StartsWith(argv[i], true, null))
                {
                    _UseSharedMemory = true;
                }
                else if ("-enableTcpOnly".StartsWith(argv[i], true, null))
                {
                    _UseTcpOnly = true;
                }
                else if ("-debug".StartsWith(argv[i], true, null))
                {
                    NDDS.ConfigLogger.get_instance().set_verbosity_by_category(
                        NDDS.LogCategory.NDDS_CONFIG_LOG_CATEGORY_API,
                        NDDS.LogVerbosity.NDDS_CONFIG_LOG_VERBOSITY_STATUS_ALL);
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
                    Console.Error.Write("Auto Throttling enabled. Automatically adjusting the DataWriter\'s writing rate\n");
		    _AutoThrottle = true;
                }
                else if ("-enableTurboMode".StartsWith(argv[i], true, null))
                {
                    _TurboMode = true;
                }
                else if ("-configFile".StartsWith(argv[i], true, null)) {
                    /* Ignore config file */
                    ++i;
                }
                else {
                    Console.Error.Write(argv[i] + ": not recognized\n");
                    return false;
                }
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
                Console.Error.WriteLine("Found inconsistent topic " +
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
        class RTIPublisher : IMessagingWriter
        {
            private TestData_tDataWriter _writer = null;
            private TestData_t _data = new TestData_t();

            private int _num_instances;
            private long _instance_counter;
            DDS.InstanceHandle_t[] _instance_handles;
            private Semaphore _pongSemaphore = null;

            public RTIPublisher(DDS.DataWriter writer, int num_instances, Semaphore pongSemaphore)
            {
                _writer = (TestData_tDataWriter)writer;
                _data.bin_data.maximum = 0;

                _num_instances = num_instances;
                _instance_counter = 0;
                _instance_handles = new DDS.InstanceHandle_t[num_instances];
                _pongSemaphore = pongSemaphore;

                for(int i=0; i<_num_instances; ++i) {
                    _data.key[0] = (byte) (i);
                    _data.key[1] = (byte) (i >> 8);
                    _data.key[2] = (byte) (i >> 16);
                    _data.key[3] = (byte) (i >> 24);

                    _instance_handles[i] = _writer.register_instance(_data);                       
                }

            }

            public void Flush()
            {
                _writer.flush();
            }

            public bool Send(TestMessage message)
            {
                int key = 0;
                _data.entity_id = message.entity_id;
                _data.seq_num = message.seq_num;
                _data.timestamp_sec = message.timestamp_sec;
                _data.timestamp_usec = message.timestamp_usec;
                _data.latency_ping = message.latency_ping;
                _data.bin_data.loan(message.data, message.size);

                if (_num_instances > 1) {
                    key = (int) (_instance_counter++ % _num_instances);

                    _data.key[0] = (byte) (key);
                    _data.key[1] = (byte) (key >> 8);
                    _data.key[2] = (byte) (key >> 16);
                    _data.key[3] = (byte) (key >> 24);
                }

                try
                {
                    _writer.write(_data, ref _instance_handles[key]);
                }
                catch (DDS.Exception ex)
                {
                    Console.Error.Write("Write error {0}\n", ex);
                    _data.bin_data.unloan();
                    return false;
                }
                finally
                {
                    _data.bin_data.unloan();
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

        }

        /*********************************************************
         * ReceiverListener
         */
        class ReceiverListener : DDS.DataReaderListener
        {
            private TestData_tSeq     _data_seq = new TestData_tSeq();
            private DDS.SampleInfoSeq _info_seq = new DDS.SampleInfoSeq();
            private TestMessage       _message = new TestMessage();
            private IMessagingCB      _callback;

            public ReceiverListener(IMessagingCB callback)
            {
                _callback = callback;
                _message.data = new byte[MAX_BINDATA_SIZE.VALUE];
            }

            public override void on_data_available(DDS.DataReader reader)
            {
                TestData_tDataReader datareader;

                int i;
                TestData_t message;
                int seqLen;

                datareader = (TestData_tDataReader) reader;
                if (datareader == null)
                {
                    Console.Error.Write("DataReader narrow error\n");
                    return;
                }

                try
                {
                    datareader.take(
                        _data_seq, _info_seq,
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
                            message = _data_seq.get_at(i);

                            _message.entity_id = message.entity_id;
                            _message.seq_num = message.seq_num;
                            _message.timestamp_sec = message.timestamp_sec;
                            _message.timestamp_usec = message.timestamp_usec;
                            _message.latency_ping = message.latency_ping;
                            _message.size = message.bin_data.length;
                            _message.data = message.bin_data.buffer;

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
                        datareader.return_loan(_data_seq, _info_seq);
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
        class RTISubscriber : IMessagingReader
        {
            private TestData_tDataReader _reader = null;
            private TestData_tSeq      _data_seq = new TestData_tSeq();
            private DDS.SampleInfoSeq  _info_seq = new DDS.SampleInfoSeq();
            private TestMessage         _message = new TestMessage();
            private DDS.WaitSet         _waitset = null;
            private DDS.ConditionSeq _active_conditions = new DDS.ConditionSeq();

            private int      _data_idx = 0;
            private bool      _no_data = true;

            public RTISubscriber(DDS.DataReader reader)
            {
                _reader = (TestData_tDataReader) reader;
                _message.data = new byte[MAX_BINDATA_SIZE.VALUE];

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
                _reader.return_loan(_data_seq, _info_seq);
            }

            public TestMessage ReceiveMessage()
            {
                TestData_t message;

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
                            _reader.take(
                                _data_seq, _info_seq,
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
                        _reader.return_loan(_data_seq, _info_seq);
                        _no_data = true;
                        // for some reason, woke up, only got meta-data messages
                        continue;
                    }

                    // skip non-valid data
                    while ( (!_info_seq.get_at(_data_idx).valid_data) && 
                            (++_data_idx < seq_length));

                    // may have hit end condition
                    if (_data_idx == seq_length) { continue; }

                    message = _data_seq.get_at(_data_idx);

                    _message.entity_id = message.entity_id;
                    _message.seq_num = message.seq_num;
                    _message.timestamp_sec = message.timestamp_sec;
                    _message.timestamp_usec = message.timestamp_usec;
                    _message.latency_ping = message.latency_ping;
                    _message.size = message.bin_data.length;
                    _message.data = message.bin_data.buffer;                   

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
         * Initialize
         */
        public bool Initialize(int argc, string[] argv)
        {
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
            _factory.get_participant_qos_from_profile(qos, "PerftestQosLibrary", "BaseProfileQos");

            // set transports to use
            qos.transport_builtin.mask = (int)DDS.TransportBuiltinKind.TRANSPORTBUILTIN_UDPv4;
            if (_UseTcpOnly)
            {
                qos.transport_builtin.mask = (int)DDS.TransportBuiltinKindMask.TRANSPORTBUILTIN_MASK_NONE;
                try
                {
                    DDS.PropertyQosPolicyHelper.add_property(qos.property_qos,
                                             "dds.transport.load_plugins",
                                             "dds.transport.TCPv4.tcp1",
                                             false);
                }
                catch (DDS.Exception e)
                {
                    Console.Error.WriteLine("! unable to set property dds.transport.load_plugins");
                    Console.Error.WriteLine(e.Message);
                }
            }
            else
            {
                if (_UseSharedMemory)
                {
                    qos.transport_builtin.mask = (int)DDS.TransportBuiltinKind.TRANSPORTBUILTIN_SHMEM;
                }
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
            if (!_UseTcpOnly)
            {
                if (_Nic.Length > 0)
                {
                    DDS.PropertyQosPolicyHelper.add_property(qos.property_qos,
                        "dds.transport.UDPv4.builtin.parent.allow_interfaces", _Nic, false);
                }

                int received_message_count_max = 1024*1024*2 / _DataLen;

                DDS.PropertyQosPolicyHelper.add_property(qos.property_qos,
                    "dds.transport.shmem.builtin.received_message_count_max", received_message_count_max.ToString(), false);
            }
            // Creates the participant
            _participant = _factory.create_participant(
                _DomainID, qos, listener,
                (DDS.StatusMask)
                (DDS.StatusKind.INCONSISTENT_TOPIC_STATUS |
                 DDS.StatusKind.OFFERED_INCOMPATIBLE_QOS_STATUS |
                 DDS.StatusKind.REQUESTED_INCOMPATIBLE_QOS_STATUS));

            if (_participant == null)
            {
                Console.Error.Write("Problem creating participant.\n");
                return false;
            }

            // Register the types and create the topics
            TestData_tTypeSupport.register_type(
                _participant, _typename);

            // Create the Publisher and Subscriber
            {

                _publisher = _participant.create_publisher_with_profile(
                    "PerftestQosLibrary", "BaseProfileQos", null, DDS.StatusMask.STATUS_MASK_NONE);

                if (_publisher == null)
                {
                    Console.Error.WriteLine("Problem creating publisher.");
                    return false;
                }

                _subscriber = _participant.create_subscriber_with_profile(
                    "PerftestQosLibrary", "BaseProfileQos", null, DDS.StatusMask.STATUS_MASK_NONE);

                if (_subscriber == null)
                {
                    Console.Error.WriteLine("Problem creating subscriber.");
                    return false;
                }

            }

            return true;
        }


        /*********************************************************
          * CreateWriter
          */
        public IMessagingWriter CreateWriter(string topic_name)
        {
            DDS.DataWriter writer = null;
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

            if (topic_name == perftest_cs._ThroughputTopicName)
            {
                if (_UsePositiveAcks)
                {
                    qos_profile = "ThroughputQos";
                }
                else
                {
                    qos_profile = "NoAckThroughputQos";
                }
            }
            else if (topic_name == perftest_cs._LatencyTopicName)
            {
                if (_UsePositiveAcks)
                {
                    qos_profile = "LatencyQos";
                }
                else
                {
                    qos_profile = "NoAckLatencyQos";
                }
            }
            else if (topic_name == perftest_cs._AnnouncementTopicName)
            {
                qos_profile = "AnnouncementQos";
            }
            else
            {
                Console.Error.WriteLine("topic name must either be "
                    + perftest_cs._ThroughputTopicName
                    + " or " + perftest_cs._LatencyTopicName
                    + " or " + perftest_cs._AnnouncementTopicName);
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

            if (_UsePositiveAcks) 
            {
                dw_qos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration.sec = (int)_KeepDurationUsec/1000000;
                dw_qos.protocol.rtps_reliable_writer.disable_positive_acks_min_sample_keep_duration.nanosec = _KeepDurationUsec%1000000;
            }

            // only force reliability on throughput/latency topics
            if (topic_name != perftest_cs._AnnouncementTopicName)
            {
                if (_IsReliable)
                {
                    dw_qos.reliability.kind = DDS.ReliabilityQosPolicyKind.RELIABLE_RELIABILITY_QOS;
                }
                else
                {
                    dw_qos.reliability.kind = DDS.ReliabilityQosPolicyKind.BEST_EFFORT_RELIABILITY_QOS;
                }
            }

            // These QOS's are only set for the Throughput datawriter
            if ((qos_profile == "ThroughputQos") || (qos_profile == "NoAckThroughputQos"))
            {
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
                
                dw_qos.protocol.rtps_reliable_writer.max_send_window_size =
                    _SendQueueSize;
                dw_qos.protocol.rtps_reliable_writer.min_send_window_size =
                    _SendQueueSize;
            }


            if (("LatencyQos" == qos_profile ||
                 "NoAckLatencyQos"  == qos_profile) &&
                 !_DirectCommunication &&
                (_Durability == 2 ||
                 _Durability == 3)){

                dw_qos.durability.kind = (DDS.DurabilityQosPolicyKind)_Durability;
                dw_qos.durability.direct_communication = _DirectCommunication;
            }

            dw_qos.resource_limits.max_instances = _InstanceCount;
            dw_qos.resource_limits.initial_instances = _InstanceCount;

            if (_InstanceCount > 1) {
                if (_InstanceHashBuckets > 0) {
                    dw_qos.resource_limits.instance_hash_buckets = _InstanceHashBuckets;
                } else {
                    dw_qos.resource_limits.instance_hash_buckets = _InstanceCount;
                }
            }

            writer = _publisher.create_datawriter(
                topic, dw_qos, null,
                DDS.StatusMask.STATUS_MASK_NONE);

            if (writer == null)
            {
                Console.Error.Write("Problem creating writer.\n");
                return null;
            }

            RTIPublisher pub = new RTIPublisher(writer,_InstanceCount,_pongSemaphore);

            return pub;
        }


        /*********************************************************
         * CreateReader
         */
        public IMessagingReader CreateReader(string topic_name, IMessagingCB callback)
        {
            ReceiverListener reader_listener = null;
            DDS.DataReader reader = null;
            DDS.DataReaderQos dr_qos = new DDS.DataReaderQos();
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

            if (topic_name == perftest_cs._ThroughputTopicName)
            {
                if (_UsePositiveAcks)
                {
                    qos_profile = "ThroughputQos";
                }
                else
                {
                    qos_profile = "NoAckThroughputQos";
                }
            }
            else if (topic_name == perftest_cs._LatencyTopicName)
            {
                if (_UsePositiveAcks)
                {
                    qos_profile = "LatencyQos";
                }
                else
                {
                    qos_profile = "NoAckLatencyQos";
                }
            }
            else if (topic_name == perftest_cs._AnnouncementTopicName)
            {
                qos_profile = "AnnouncementQos";
            }
            else
            {
                Console.Error.WriteLine("topic name must either be "
                    + perftest_cs._ThroughputTopicName
                    + " or " + perftest_cs._LatencyTopicName
                    + " or " + perftest_cs._AnnouncementTopicName);
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
            if (topic_name != perftest_cs._AnnouncementTopicName)
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

            if ("ThroughputQos" == qos_profile ||
                "NoAckThroughputQos" == qos_profile) {
                dr_qos.durability.kind = (DDS.DurabilityQosPolicyKind)_Durability;
                dr_qos.durability.direct_communication = _DirectCommunication;
            }

            if (("LatencyQos" == qos_profile ||
                "NoAckLatencyQos" == qos_profile) &&
                !_DirectCommunication &&
                (_Durability == 2 ||
                 _Durability == 3)){

                dr_qos.durability.kind = (DDS.DurabilityQosPolicyKind)_Durability;
                dr_qos.durability.direct_communication = _DirectCommunication;
            }
           
            dr_qos.resource_limits.initial_instances = _InstanceCount;
            dr_qos.resource_limits.max_instances = _InstanceCount;

            if (_InstanceCount > 1) {
                if (_InstanceHashBuckets > 0) {
                    dr_qos.resource_limits.instance_hash_buckets = _InstanceHashBuckets;
                } else {
                    dr_qos.resource_limits.instance_hash_buckets = _InstanceCount;
                }
            }

            if (!_UseTcpOnly)
            {
                if (_IsMulticast)
                {
                    string multicast_addr;


                    if (topic_name == perftest_cs._ThroughputTopicName)
                    {
                        multicast_addr = THROUGHPUT_MULTICAST_ADDR;
                    }
                    else if (topic_name == perftest_cs._LatencyTopicName)
                    {
                        multicast_addr = LATENCY_MULTICAST_ADDR;
                    }
                    else
                    {
                        multicast_addr = ANNOUNCEMENT_MULTICAST_ADDR;
                    }

                    DDS.TransportMulticastSettings_t multicast_setting = new DDS.TransportMulticastSettings_t();
                    multicast_setting.receive_address = multicast_addr;
                    multicast_setting.receive_port = 0;
                    multicast_setting.transports.length = 0;
                    dr_qos.multicast.value.ensure_length(1, 1);
                    dr_qos.multicast.value.set_at(0, multicast_setting);
                }
            }


            if (callback != null)
            {
                reader_listener = new ReceiverListener(callback);
                reader = _subscriber.create_datareader(
                    topic, dr_qos, reader_listener, (DDS.StatusMask)DDS.StatusKind.DATA_AVAILABLE_STATUS);
            }
            else
            {
                reader = _subscriber.create_datareader(
                    topic, dr_qos, null, DDS.StatusMask.STATUS_MASK_NONE);
            }


            if (reader == null)
            {
                Console.Error.Write("Problem creating reader.\n");
                return null;
            }

            if (topic_name == perftest_cs._LatencyTopicName ||
                topic_name == perftest_cs._ThroughputTopicName) {
                _reader = reader;
            }

            IMessagingReader sub = new RTISubscriber(reader);
            return sub;
        }

        private int    _SendQueueSize = 50;
        private int    _DataLen = 100;
        private int    _DomainID = 1;
        private string _Nic = "";
        private string _ProfileFile = "perftest.xml";
        private string _ConfigFile = "perftest.ini";
        private bool   _IsReliable = true;
        private bool   _IsMulticast = false;
        private bool   _AutoThrottle = false;
        private bool   _TurboMode = false;
        private int    _BatchSize = 0;
        private int    _InstanceCount = 1;
        private int     _InstanceHashBuckets = -1;
        private int     _Durability = 0;
        private bool    _DirectCommunication = true;
        private uint   _KeepDurationUsec = 1000;
        private bool   _UsePositiveAcks = true;
        private bool   _UseSharedMemory = false;
        private bool    _LatencyTest = false;
        private bool   _UseTcpOnly = false;
        private bool   _IsDebug = false;

        private DDS.Duration_t _HeartbeatPeriod = DDS.Duration_t.DURATION_ZERO;     /* this means, use the perftest.xml QoS file value*/
        private DDS.Duration_t _FastHeartbeatPeriod = DDS.Duration_t.DURATION_ZERO; /* this means, use the perftest.xml QoS file value*/

        private static int   _WaitsetEventCount = 5;
        private static uint  _WaitsetDelayUsec = 100;

        private static string THROUGHPUT_MULTICAST_ADDR = "239.255.1.1";
        private static string LATENCY_MULTICAST_ADDR = "239.255.1.2";
        private static string ANNOUNCEMENT_MULTICAST_ADDR = "239.255.1.100";
        private const string _ProfileLibraryName = "PerftestQosLibrary";

        private DDS.DomainParticipantFactory _factory = null;
        private DDS.DomainParticipant        _participant = null;
        private DDS.Subscriber               _subscriber = null;
        private DDS.Publisher                _publisher = null;
        private DDS.DataReader               _reader = null;
        private string _typename = TestData_tTypeSupport.get_type_name();

        private Semaphore _pongSemaphore = null;
    }
}
