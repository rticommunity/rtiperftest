/* $Id: perftest_cs.cs,v 1.1.2.1 2014/04/01 11:56:53 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history
--------------------
1.0a,13jul10,jsr Added WaitForPingResponse with timeout
1.0a,09jul10,jsr Fixed LatencyListener constructor
1.0a,08jul10,jsr Fixed LatencyListener constructor calls
1.0a,07jul10,jsr Fixed _LatencyCount option default value
1.0a,11may10,jsr Added LatencyTest option
1.0a,14may09,fcs Added instances to INI
1.0a,14may09,fcs Fixed command-line arguments processing
1.0a,14may09,fcs Fixed output in scan mode
1.0a,29may09,jsr Added detection of wrong command line parameter
1.0a,23apr09,jsr Changed to stderr the error and status messages
1.0a,21apr09,jsr Reformat the help menu
1.0a,02oct08,eys Added 99.99% latency
1.0a,22aug08,eys Flush after every send when initializing the channel
1.0a,22aug08,eys decrease checking interval on subscriber to gather latency
                 numbers
1.0a,20aug08,eys initialize channel by writing InstanceCount initially
1.0a,14aug08,ch  changed key to be 4 bytes long -> overhead change
1.0a,13aug08,ch  added check to not exceed MAX_BINDATA_SIZE
1.0a,12may08,hhw Fixed some length checks to accommodate 32 byte overhead.
1.0a,09may08,ch  Changed OVERHEAD bytes
1.0a,04may08,hhw Modified batch processing for sending latency count.
                 Synchronized to perftest.cxx changes.
1.0a,22apr08,fcs Fixed lost count
1.0a,21apr08,ch  Added median to the ouput
1.0a,21apr08,fcs Changed OVERHEAD bytes
1.0a,07apr08,hhw Now printing end of test in listener to avoid race condition.
1.0a,18mar08,hhw Created.
===================================================================== */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices; // for DllImport]
using System.Threading;
using System.Text;


namespace PerformanceTest {

    public class perftest_cs : IDisposable
    {

       /*********************************************************
        * Main
        */
        static void Main(string[] argv)
        {
            perftest_cs app = new perftest_cs();
           
            app.Run(argv);

            app.Dispose();
        }
        
        void Run(string[] argv)
        {
            _MessagingImpl = new RTIDDSImpl();

            if ( !ParseConfig(argv) )
            {
                return;
            }

            if (!_MessagingImpl.Initialize(_MessagingArgc, _MessagingArgv)) 
            {
                return;
            }

            _BatchSize = _MessagingImpl.GetBatchSize();
            _MaxBinDataSize = _MessagingImpl.GetMaxBinDataSize();

            if (_BatchSize != 0) {
                _SamplesPerBatch = _BatchSize/_DataLen;
                if (_SamplesPerBatch == 0) {
                   _SamplesPerBatch = 1;
                }
            } else {
                _SamplesPerBatch = 1;
            }

            if (_IsPub) {
                Publisher();
            } else {
                Subscriber();
            }
        }

        /*********************************************************
         * Destructor
         */
        public void Dispose() 
        {
            _MessagingImpl.Shutdown();
            Console.Error.Write("Test ended.\n");
            Console.Out.Flush();
        }

        /*********************************************************
         * Constructor 
         */
        perftest_cs() 
        {
            QueryPerformanceFrequency(ref _ClockFrequency);
        }

        /*********************************************************
        * ParseConfig
        */   
        bool ParseConfig(string[] argv)
        {
            _MessagingArgv = new String [argv.Length];

            string usage_string = 
                /**************************************************************************/
                "Usage:\n" +
                "       perftest_cs [options]\n" +
                "\nWhere [options] are (case insensitive, partial match OK):\n\n" +
                "\t-help                   - Print this usage message and exit\n" +
                "\t-pub                    - Set test to be a publisher\n" +
                "\t-sub                    - Set test to be a subscriber (default)\n" +
                "\t-sidMultiSubTest <id>   - Set id of the subscriber in\n"+
                "\t                          a multi-subscriber test, default 0\n" +
                "\t-pidMultiPubTest <id>   - Set id of the publisher in a multi-publisher\n" +
                "\t                          test, default 0. Only publisher 0 sends\n" +
                "\t                          latency pings.\n" +
                "\t-configFile <ilename>   - Set the name of the .ini configuration file,\n" +
                "\t                          default perftest.ini\n"  +
                "\t-dataLen <bytes>        - Set length of payload for each send,\n" +
                "\t                          default 100\n"  +
                "\t-numIter <count>        - Set number of messages to send,\n" +
		"\t                          default 0 (infinite)\n" +
		"\t-instances <#instance>  - set the number of instances (keys) to iterate\n" +
		"\t                          over when publishing, default 1\n" +

                "\t-sleep <millisec>       - Time to sleep between each send, default 0\n" +
                "\t-spin <count>           - Number of times to run in spin loop between\n" +
                "\t                          each send, default 0\n" +
                "\t-latencyCount <count>   - Number samples (or batches) to send before a\n" +
                "\t                          latency ping packet is sent, default\n" +
                "\t                          10000 if -latencyTest is not specified,\n" +
                "\t                          1 if -latencyTest is specified\n" +
                "\t-numSubscribers <count> - Number of subscribers running in test,\n" +
                "\t                          default 1\n" +
                "\t-numPublishers <count>  - Number of publishers running in test,\n" +
                "\t                          default 1\n" +
                "\t-scan                   - Run test in scan mode, traversing a range of\n"+
                "\t                          data sizes, 32 - " + TestMessage.MAX_DATA_SIZE + "\n" +
                "\t-noPrintIntervals       - Don't print statistics at intervals during\n"+
                "\t                          test\n" +
                "\t-useReadThread          - Use separate thread instead of callback to\n"+
                "\t                          read data\n" +
                "\t-latencyTest            - Run a latency test consisting of a ping-pong \n" +
	        "\t                          synchronous communication\n" +
                "\t-debug                  - Run in debug mode\n";
            

            int argc = argv.Length;
            
            if (argc < 0)
            {
                Console.Error.Write(usage_string);
                _MessagingImpl.PrintCmdLineHelp();
                return false;
            }
                

            // first scan for configFile
            for (int i = 0; i < argc; ++i) 
            {
                if ( "-help".StartsWith(argv[i], true, null) ) 
                {
                    Console.Error.Write(usage_string);
                    _MessagingImpl.PrintCmdLineHelp();
                    return false;
                }
                else if ("-configFile".StartsWith(argv[i], true, null))
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
                Nini.Config.IniConfigSource configSource = new Nini.Config.IniConfigSource();

                try
                {
                    configSource.Load(_ConfigFile);
                } catch (System.IO.IOException e) {
                    Console.Error.Write("Problem loading configuration file.\n");
                    Console.Error.WriteLine(e.Message);
                    return false;
                }
                configSource.CaseSensitive = false;

                Nini.Config.IConfig config = configSource.Configs["perftest"];

                if (config == null)
                {
                    Console.Error.Write("Could not find section [perftest] in file " + _ConfigFile + ".\n");
                    return false;
                }
                _NumIter        = config.GetInt("num iter", _NumIter);
                _DataLen        = config.GetInt("data length", _DataLen);
                _SpinLoopCount  = config.GetInt("spin loop count", _SpinLoopCount);
                _SleepMillisec  = config.GetInt("sleep millisec", _SleepMillisec);
                _LatencyCount   = config.GetInt("latency count", _LatencyCount);
                _NumSubscribers = config.GetInt("num subscribers", _NumSubscribers);
                _NumPublishers  = config.GetInt("num publishers", _NumPublishers);
                _IsScan         = config.GetBoolean("scan", _IsScan);
                _PrintIntervals = config.GetBoolean("print intervals", _PrintIntervals);
                _UseReadThread  = config.GetBoolean("use read thread", _UseReadThread);
                _IsDebug        = config.GetBoolean("is debug", _IsDebug);
                _InstanceCount  = config.GetInt("instances", _InstanceCount);
                _LatencyTest = config.GetBoolean("run latency test", _LatencyTest);
                _isReliable = config.GetBoolean("is reliable", _isReliable);
            }


            // now load everything else, command line params override config file
            for (int i = 0; i < argc; ++i) 
            {
                if ("-pub".StartsWith(argv[i], true, null))
                {
                    _IsPub = true;
                }
                else if ("-sub".StartsWith(argv[i], true, null))
                {
                    _IsPub = false;
                }
                else if ("-sidMultiSubTest".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <id> after -sidMultiSubTest\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _SubID))
                    {
                        Console.Error.Write("Bad id for subscriber\n");
                        return false;
                    }
                }
                else if ("-pidMultiPubTest".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <id> after -pidMultiPubTest\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _PubID))
                    {
                        Console.Error.Write("Bad id for publisher\n");
                        return false;
                    }
                }
                else if ("-numIter".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <iter> after -numIter\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _NumIter))
                    {
                        Console.Error.Write("Bad numIter\n");
                        return false;
                    }
                }
                else if ("-dataLen".StartsWith(argv[i], true, null))
                {
                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <length> after -dataLen\n");
                        return false;
                    }

                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    if (!Int32.TryParse(argv[i], out _DataLen))
                    {
                        Console.Error.Write("Bad dataLen\n");
                        return false;
                    }
                    if (_DataLen < OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("dataLen must be >= " + OVERHEAD_BYTES);
                        return false;
                    }
                    if (_DataLen > TestMessage.MAX_DATA_SIZE)
                    {
                        Console.Error.WriteLine("dataLen must be <= " + TestMessage.MAX_DATA_SIZE);
                        return false;
                    }
                } 
                else if ( "-spin".StartsWith(argv[i], true, null) ) {
                    if (( i == (argc-1)) || argv[++i].StartsWith("-") ) {
                        Console.Error.Write("Missing <count> after -spin\n");
                        return false;
                    }
                    if ( !Int32.TryParse(argv[i], out _SpinLoopCount) ) {
                        Console.Error.Write("Bad spin count\n");
                        return false;
                    }
                } 
                else if ( "-sleep".StartsWith(argv[i], true, null) ) {
                    if (( i == (argc-1)) || argv[++i].StartsWith("-") ) {
                        Console.Error.Write("Missing <millisec> after -sleep\n");
                        return false;
                    }
                    if ( !Int32.TryParse(argv[i], out _SleepMillisec) ) {
                        Console.Error.Write("Bad sleep millisec\n");
                        return false;
                    }
                } 
                else if ( "-latencyCount".StartsWith(argv[i], true, null) ) {
                    if (( i == (argc-1)) || argv[++i].StartsWith("-") ) {
                        Console.Error.Write("Missing <count> after -latencyCount\n");
                        return false;
                    }
                    if ( !Int32.TryParse(argv[i], out _LatencyCount) ) {
                        Console.Error.Write("Bad latency count\n");
                        return false;
                    }
                }
                else if ("-numSubscribers".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -numSubscribers\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _NumSubscribers))
                    {
                        Console.Error.Write("Bad num subscribers\n");
                        return false;
                    }
                }
                else if ("-numPublishers".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -numPublishers\n");
                        return false;
                    }
                    if (!Int32.TryParse(argv[i], out _NumPublishers))
                    {
                        Console.Error.Write("Bad num publishers\n");
                        return false;
                    }
                }
                else if ("-scan".StartsWith(argv[i], true, null))
                {
                    _IsScan = true;
                }
                else if ("-noPrintIntervals".StartsWith(argv[i], true, null))
                {
                    _PrintIntervals = false;
                }
                else if ("-useReadThread".StartsWith(argv[i], true, null))
                {
                    _UseReadThread = true;
                }
                else if ( "-latencyTest".StartsWith(argv[i],true,null) )
                {
                    _LatencyTest = true;

                    _MessagingArgv[_MessagingArgc] = argv[i];

                    if (_MessagingArgv[_MessagingArgc] == null)
                    {
                        Console.Error.Write("Problem allocating memory\n");
                        return false;
                    }

                    _MessagingArgc++;
                }
                else if ( "-bestEffort".StartsWith(argv[i],true,null) )
                {
                    _isReliable = false;

                    _MessagingArgv[_MessagingArgc] = argv[i];

                    if (_MessagingArgv[_MessagingArgc] == null)
                    {
                        Console.Error.Write("Problem allocating memory\n");
                        return false;
                    }

                    _MessagingArgc++;
                }
                else if ("-instances".StartsWith(argv[i], true, null))
                {
                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <count> after -instances\n");
                        return false;
                    }

                    _MessagingArgv[_MessagingArgc++] = argv[i];

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
                else if ("-debug".StartsWith(argv[i], true, null))
                {
                    _IsDebug = true;

                    _MessagingArgv[_MessagingArgc] = argv[i];

                    if (_MessagingArgv[_MessagingArgc] == null)
                    {
                        Console.Error.Write("Problem allocating memory\n");
                        return false;
                    }

                    _MessagingArgc++;
                }
                else {
                    _MessagingArgv[_MessagingArgc++] = argv[i];
                }
            }

            if (_LatencyTest)
            {
                if (_PubID != 0)
                {
                    Console.Error.Write("Only the publisher with ID = 0 can run the latency test\n");
                    return false;
                }

                // With latency test, latency should be 1
                if (_LatencyCount == -1)
                {
                    _LatencyCount = 1;
                }
            }

	    if (_LatencyCount == -1)
	    {
		_LatencyCount = 10000;
	    }

            if (_IsScan && _NumPublishers > 1) 
            {
                Console.Error.Write("_IsScan is not supported with more than one publisher\n");
                return false;
            }

            if ((_NumIter > 0) && (_NumIter < _LatencyCount))
            {
                Console.Error.Write("numIter ({0}) must be greater than latencyCount ({1}).\n",
                              _NumIter, _LatencyCount);
                return false;
            }

            return true;
        }

        /*********************************************************
         * Listener for the Subscriber side
         *
         * Keeps stats on data received per second.
         * Returns a ping for latency packets
         */
        class ThroughputListener :  IMessagingCB
        {
            public ulong packets_received = 0;
            public ulong bytes_received = 0;
            public ulong missing_packets = 0;
            public bool  end_test = false;
            public int   last_data_length = -1;

            // store info for the last data set
            public int   interval_data_length = -1;
            public ulong interval_packets_received = 0;
            public ulong interval_bytes_received = 0;
            public ulong interval_missing_packets = 0;
            public ulong interval_time = 0, begin_time = 0;
            
            private IMessagingWriter _writer = null;
            private IMessagingReader _reader = null;
            private ulong[] _last_seq_num = null;

            private int _finish_count = 0;
            private int _num_publishers;


            public ThroughputListener(IMessagingWriter writer,
                                      int numPublishers)
            {
                _writer = writer;
                _last_seq_num = new ulong[numPublishers];
                _num_publishers = numPublishers;
            }

            public ThroughputListener(IMessagingWriter writer, IMessagingReader reader,
                                      int numPublishers)
            {
                _writer = writer;
                _reader = reader;
                _last_seq_num = new ulong[numPublishers];
                _num_publishers = numPublishers;
            }
            
            public void ProcessMessage(TestMessage message) 
            {
                // Check for test initialization messages
                if (message.size == INITIALIZE_SIZE)
                {
                    _writer.Send(message);
                    _writer.Flush();
                    return;
                } 
                else if (message.size == FINISHED_SIZE)
                {
                    // only respond to publisher id 0
                    if (message.entity_id != 0) {
                        return;
                    }

                    if (end_test == true)
                    {
                        return;
                    }

                    _finish_count++;

                    if (_finish_count >= _num_publishers) {
                        // detect missing packets
                        if (message.seq_num != _last_seq_num[message.entity_id]) {
                            // only track if skipped, might have restarted pub
                            if (message.seq_num > _last_seq_num[message.entity_id])
                            {
                                missing_packets +=
                                    message.seq_num - _last_seq_num[message.entity_id];
                            }
                        }

                        // store the info for this interval
                        ulong now = perftest_cs.GetTimeUsec();
                        interval_time = now - begin_time;
                        interval_packets_received = packets_received;
                        interval_bytes_received = bytes_received;
                        interval_missing_packets = missing_packets;
                        interval_data_length = last_data_length;
                        end_test = true;
                    }

                    _writer.Send(message);
                    _writer.Flush();

                    if (_finish_count >= _num_publishers) {
                        Console.Write("Length: {0,5}  Packets: {1,8}  Packets/s(ave): {2,7:F0}  " +
                                      "Mbps(ave): {3,7:F1}  Lost: {4}\n",
                                      interval_data_length + OVERHEAD_BYTES,
                                      interval_packets_received,
                                      interval_packets_received * 1000000 /interval_time,
                                      interval_bytes_received * 1000000.0 / interval_time * 8.0 / 1000.0 / 1000.0,
                                      interval_missing_packets);
                    }
                    return;
                }

                // Send back a packet if this is a ping
                if (message.latency_ping == _SubID)
                {
                    _writer.Send(message);
                    _writer.Flush();
                }

                // Always check if need to reset internals
                if (message.size == LENGTH_CHANGED_SIZE)
                {
                    ulong now;
                    // store the info for this interval
                    now = perftest_cs.GetTimeUsec();

                    // may have many length changed packets to support best effort
                    if (interval_data_length != last_data_length)
                    {
                        // detect missing packets
                        if (message.seq_num != _last_seq_num[message.entity_id]) {
                            // only track if skipped, might have restarted pub
                            if (message.seq_num > _last_seq_num[message.entity_id])
                            {
                                missing_packets +=
                                    message.seq_num - _last_seq_num[message.entity_id];
                            }
                        }

                        interval_time = now - begin_time;
                        interval_packets_received = packets_received;
                        interval_bytes_received = bytes_received;
                        interval_missing_packets = missing_packets;
                        interval_data_length = last_data_length;

                        Console.Write("Length: {0,5}  Packets: {1,8}  Packets/s(ave): {2,7:F0}  " +
                                      "Mbps(ave): {3,7:F1}  Lost: {4}\n",
                                      interval_data_length + OVERHEAD_BYTES,
                                      interval_packets_received,
                                      interval_packets_received*1000000/interval_time,
                                      interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                                      interval_missing_packets);
                        Console.Out.Flush();
                    }
                    
                    packets_received = 0;
                    bytes_received = 0;
                    missing_packets = 0;
                    // length changed only used in scan mode in which case
                    // there is only 1 publisher with ID 0
                    _last_seq_num[0] = 0;
                    begin_time = now;
                    return;
                }
                
                // case where not running a scan
                if (message.size != last_data_length)
                {
                    packets_received = 0;
                    bytes_received = 0;
                    missing_packets = 0;

                    for (int i=0; i<_num_publishers; i++) {
                        _last_seq_num[i] = 0;
                    }

                    begin_time = perftest_cs.GetTimeUsec();

                    if (_PrintIntervals)
                    {
                        Console.Write("\n\n********** New data length is {0}\n",
                                      message.size + OVERHEAD_BYTES);
                        Console.Out.Flush();
                    }
                }

                last_data_length = message.size;
                ++packets_received;
                bytes_received += (ulong) (message.size + OVERHEAD_BYTES);
                
                // detect missing packets
                if (_last_seq_num[message.entity_id] == 0) {
                    _last_seq_num[message.entity_id] = message.seq_num;
                } else {
                    if (message.seq_num != ++_last_seq_num[message.entity_id]) {
                        // only track if skipped, might have restarted pub
                        if (message.seq_num > _last_seq_num[message.entity_id])
                        {
                            missing_packets +=
                                message.seq_num - _last_seq_num[message.entity_id];
                        }
                        _last_seq_num[message.entity_id] = message.seq_num;
                    }
                }
            }

            /*********************************************************
             * Used for receiving data using a thread instead of callback
             *
             */
            public void ReadThread()
            {
                TestMessage message = null;
                while (!end_test)
                {
                    // Receive message should block until a message is received
                    message = _reader.ReceiveMessage();
                    if (message != null)
                    {
                        ProcessMessage(message);
                    }
                }
            }
        }

        /*********************************************************
         * Subscriber
         */
        void Subscriber()
        {
            ThroughputListener reader_listener = null;
            IMessagingReader   reader;
            IMessagingWriter   writer;
            IMessagingWriter   announcement_writer;

            // create latency pong writer
            writer = _MessagingImpl.CreateWriter(_LatencyTopicName);

            if (writer == null) {
                Console.Error.Write("Problem creating latency writer.\n");
                return;
            }

            // Check if using callbacks or read thread
            if (!_UseReadThread)
            {
                // create latency pong reader
                reader_listener = new ThroughputListener(writer, _NumPublishers);
                reader = _MessagingImpl.CreateReader(_ThroughputTopicName, reader_listener);
                if (reader == null)
                {
                    Console.Error.Write("Problem creating throughput reader.\n");
                    return;
                }
            }
            else
            {
                reader = _MessagingImpl.CreateReader(_ThroughputTopicName, null);
                if (reader == null)
                {
                    Console.Error.Write("Problem creating throughput reader.\n");
                    return;
                }
                reader_listener = new ThroughputListener(writer, reader, _NumPublishers);

                Thread thread = new Thread(new ThreadStart(reader_listener.ReadThread));
                thread.Start();
            }
            
            // Create announcement writer
            announcement_writer = _MessagingImpl.CreateWriter(_AnnouncementTopicName);
            
            if (announcement_writer == null) {
                Console.Error.Write("Problem creating announcement writer.\n");
                return;
            }

            // Synchronize with publishers
            Console.Error.Write("Waiting to discover {0} publishers ...\n", _NumPublishers);
            reader.WaitForWriters(_NumPublishers);
            announcement_writer.WaitForReaders(_NumPublishers);
            
            // Send announcement message
            TestMessage message = new TestMessage();
            message.entity_id = _SubID;
            message.data = new byte[1];
            message.size = 1;
            announcement_writer.Send(message);
            announcement_writer.Flush();

            Console.Error.Write("Waiting for data...\n");
            
            // wait for data
            ulong  now, prev_time, delta;
            ulong  prev_count = 0;
            ulong  prev_bytes = 0;
            ulong  ave_count = 0;
            int    last_data_length = -1;
            ulong  mps, bps;
            double mps_ave = 0.0, bps_ave = 0.0;
            ulong  msgsent, bytes, last_msgs, last_bytes;


            now = GetTimeUsec();
            while (true) { 
                prev_time = now;
                System.Threading.Thread.Sleep(1000);
                now = GetTimeUsec();

                if (reader_listener.end_test)
                {
                    break;
                }

                if (_PrintIntervals) {
                    if (last_data_length != reader_listener.last_data_length)
                    {
                        last_data_length = reader_listener.last_data_length;
                        prev_count = reader_listener.packets_received;
                        prev_bytes = reader_listener.bytes_received;
                        bps_ave = 0;
                        mps_ave = 0;
                        ave_count = 0;
                        continue;
                    }

                    last_msgs = reader_listener.packets_received;
                    last_bytes = reader_listener.bytes_received;
                    msgsent = last_msgs - prev_count;
                    bytes = last_bytes - prev_bytes;
                    prev_count = last_msgs;
                    prev_bytes = last_bytes;
                    delta = now - prev_time;
                    mps = (msgsent * 1000000 / delta);
                    bps = (bytes * 1000000 / delta);

                    // calculations of overall average of mps and bps
                    ++ave_count;
                    bps_ave = bps_ave + (double)(bps - bps_ave) / (double)ave_count;
                    mps_ave = mps_ave + (double)(mps - mps_ave) / (double)ave_count;

                    if (last_msgs > 0) 
                    {
                        Console.Write("Packets: {0,8}  Packets/s: {1,7}  Packets/s(ave): {2,7:F0}  " +
                                     "Mbps: {3,7:F1}  Mbps(ave): {4,7:F1}  Lost: {5}\n",
                                     last_msgs, mps, mps_ave,
                                    bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                                     reader_listener.missing_packets);
                    }
                }
            }

            if (_UseReadThread)
            {
                reader.Shutdown();
            }

            System.Threading.Thread.Sleep(1000);
            Console.Error.Write("Finishing test...\n");
            Console.Out.Flush();
        }

        /*********************************************************
         * Data listener for the Announcement
         *
         * Receives an announcement message from a Subscriber once
         * the subscriber has discovered every Publisher.
         */
        class AnnouncementListener : IMessagingCB 
        {
            public int announced_subscribers;

            public AnnouncementListener() {
                announced_subscribers = 0;
            }
            
            public void ProcessMessage(TestMessage message)
            {
                announced_subscribers++;
            }
        }
        
        /*********************************************************
         * Data listener for the Publisher side.
         *
         * Receives latency ping from Subscriber and does
         * round trip latency calculations
         */
        class LatencyListener : IMessagingCB 
        {
            private ulong latency_sum = 0;
            private ulong latency_sum_square = 0;
            private ulong count = 0;
            private uint  latency_min = 0;
            private uint  latency_max = 0;
            private int   last_data_length = 0;
            public  bool  end_test = false;
            private uint[] _latency_history = null;
            private uint  clock_skew_count = 0;
            private uint _num_latency = 0;

            private IMessagingReader _reader = null;
            private IMessagingWriter _writer = null;

            public LatencyListener(IMessagingWriter writer, uint num_latency) 
            {
                if (num_latency > 0)
                {
                    _latency_history = new uint[num_latency];
                    _num_latency = num_latency;
                }
                _writer = writer;
            }

            public LatencyListener(IMessagingReader reader, IMessagingWriter writer, uint num_latency)
            {
                if (num_latency > 0)
                {
                    _latency_history = new uint[num_latency];
                    _num_latency = num_latency;
                }
                _reader = reader;
		_writer = writer;
            }

            public void ProcessMessage(TestMessage message)
            {
                ulong now, sentTime;
                long sec;
                ulong usec;
                uint latency;
                double latency_ave;
                double latency_std;

                now = GetTimeUsec();

                
                switch (message.size)
                {
                    // Initializing message, don't process
                    case INITIALIZE_SIZE:
                        return;

                    // Test finished message
                    case FINISHED_SIZE:
                      // may get this message multiple times for 1 to N tests
                      if (end_test == true)
                      {
                          return;
                      }
                      end_test = true;
                      goto case LENGTH_CHANGED_SIZE;

                    // Data length is changing size
                    case LENGTH_CHANGED_SIZE:

                        // will get a LENGTH_CHANGED message on startup before any data
                        if (count == 0)
                        {
                            return;
                        }

                        if (clock_skew_count != 0)
                        {
                            Console.Error.Write("The following latency result may not be accurate because clock skew happens {0} times\n",
                            clock_skew_count);
                        }

                        // sort the array (in ascending order)
                        System.Array.Sort(_latency_history, 0, (int)count);
                        latency_ave = latency_sum / count;
                        latency_std = System.Math.Sqrt(latency_sum_square / count - (latency_ave * latency_ave));
                        Console.Write("Length: {0,5}  Latency: Ave {1,6:F0} us  Std {2,6:F1} us  " +
                                      "Min {3,6} us  Max {4,6} us  50% {5,6} us  90% {6,6} us  99% {7,6} us  99.99% {8,6} us\n",
                                      last_data_length + OVERHEAD_BYTES, latency_ave, latency_std, latency_min, latency_max,
                                      _latency_history[count*50/100],
                                      _latency_history[count*90/100],
                                      _latency_history[count*99/100],
                                      _latency_history[(int)(count*(9999.0/10000))]);
                        Console.Out.Flush();
                        latency_sum = 0;
                        latency_sum_square = 0;
                        latency_min = 0;
                        latency_max = 0;
                        count = 0;
                        clock_skew_count = 0;

                        goto done;

                    default:
                        break;
                }

                sec = message.timestamp_sec;
                usec = message.timestamp_usec;
                sentTime = ((ulong)sec << 32) | (ulong)usec;

                if (now >= sentTime) 
                {
                    latency = (uint)(now - sentTime);

                    // keep track of one-way latency
                    latency /= 2;
                } 
                else 
                {
                    Console.Error.Write("Clock skew suspected: received time {0} usec, sent time {1} usec",
                                    now, sentTime);
                        ++clock_skew_count;
                    return;
                }

                // store value for percentile calculations
                if (_latency_history != null)
                {
                    if (count >= _num_latency)
                    {
                        Console.Error.Write("Too many latency pongs received.  Do you have more than 1 app with -pidMultiPubTest = 0 or -sidMultiSubTest = 0?\n");
                        return;
                    }
                    else 
                    {
                        _latency_history[count] = latency;
                    }
                }

                if (latency_min == 0)
                {
                    latency_min = latency;
                    latency_max = latency;
                }
                else
                {
                    if (latency < latency_min)
                    {
                        latency_min = latency;
                    }
                    if (latency > latency_max)
                    {
                        latency_max = latency;
                    }
                }

                ++count;
                latency_sum += latency;
                latency_sum_square += ((ulong)latency * (ulong)latency);

                // if data sized changed
                if (last_data_length != message.size)
                {
                    last_data_length = message.size;

                    if (last_data_length != 0)
                    {
                        if (_PrintIntervals)
                        {
                            Console.Write("\n\n********** New data length is {0}\n",
                                          last_data_length + OVERHEAD_BYTES);
                        }
                    }
                }
                else
                {
                    if (_PrintIntervals)
                    {
                        latency_ave = (double)latency_sum / (double)count;
                        latency_std = System.Math.Sqrt(
                            (double)latency_sum_square / (double)count - (latency_ave * latency_ave));

                        Console.Write("One way Latency: {0,6} us  Ave {1,6:F0} us  Std {2,6:F1} us  Min {3,6} us  Max {4,6}\n",
                            latency, latency_ave, latency_std, latency_min, latency_max);
                    }
                }
            done:
                if (_writer != null)
                {
                    _writer.NotifyPingResponse();
                }
            }

            public void ReadThread()
            {
                TestMessage message;
                while (!end_test)
                {
                    // Receive message should block until a message is received
                    message = _reader.ReceiveMessage();
                    if (message != null)
                    {
                        ProcessMessage(message);
                    }
                }
            }
        }
        
        /*********************************************************
         * Publisher
         */
        void Publisher()
        {
            LatencyListener reader_listener = null;
            IMessagingReader reader;
            IMessagingWriter writer;
            IMessagingReader announcement_reader;
            AnnouncementListener  announcement_reader_listener = null;
            uint num_latency;
            int initializeSampleCount = 50;

            // create throughput/ping writer
            writer = _MessagingImpl.CreateWriter(_ThroughputTopicName);

            if (writer == null)
            {
                Console.Error.Write("Problem creating throughput writer.\n");
                return;
            }

            // calculate number of latency pings that will be sent per data size
            if (_IsScan)
            {
                num_latency = NUM_LATENCY_PINGS_PER_DATA_SIZE;
            }
            else
            {
                num_latency = (uint)((_NumIter/_SamplesPerBatch) / _LatencyCount);
            }

            // in batch mode, might have to send another ping
            if (_SamplesPerBatch > 1) {
                ++num_latency;
            }

            // Only publisher with ID 0 will send/receive pings
            if (_PubID == 0) 
            {
                // Check if using callbacks or read thread
                if (!_UseReadThread)
                {
                    // create latency pong reader
                    reader_listener = new LatencyListener(_LatencyTest?writer:null, num_latency);
                    reader = _MessagingImpl.CreateReader(_LatencyTopicName, reader_listener);
                    if (reader == null)
                    {
                        Console.Error.Write("Problem creating latency reader.\n");
                        return;
                    }
                }
                else
                {
                    reader = _MessagingImpl.CreateReader(_LatencyTopicName, null);
                    if (reader == null)
                    {
                        Console.Error.Write("Problem creating latency reader.\n");
                        return;
                    }
                    reader_listener = new LatencyListener(reader, _LatencyTest?writer:null, num_latency);
                    
                    Thread thread = new Thread(new ThreadStart(reader_listener.ReadThread));
                    thread.Start();
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
            announcement_reader_listener = new AnnouncementListener();
            announcement_reader = _MessagingImpl.CreateReader(_AnnouncementTopicName,
                                                              announcement_reader_listener);
            if (announcement_reader == null)
            {
                Console.Error.Write("Problem creating announcement reader.\n");
                return;
            }

            Console.Error.Write("Waiting to discover {0} subscribers...\n", _NumSubscribers);
            writer.WaitForReaders(_NumSubscribers);

            // We have to wait until every Subscriber sends an announcement message
            // indicating that it has discovered every Publisher
            Console.Error.Write("Waiting for subscribers announcement ...\n");
            while (_NumSubscribers > announcement_reader_listener.announced_subscribers) {
                System.Threading.Thread.Sleep(1000);
            }
            
            // Allocate data and set size
            TestMessage message = new TestMessage();
            message.entity_id = _PubID;
            message.data = new byte[TestMessage.MAX_DATA_SIZE];

            Console.Error.Write("Publishing data...\n");

            // initialize data pathways by sending some initial pings
            if (initializeSampleCount < _InstanceCount) {
                initializeSampleCount = _InstanceCount;
            }
            message.size = INITIALIZE_SIZE;
            for (int i = 0; i < initializeSampleCount; i++)
            {
                // Send test initialization message
                writer.Send(message);
            }
            writer.Flush();

            // Set data size, account for other bytes in message
            message.size = _DataLen - OVERHEAD_BYTES;

            // Sleep 1 second, then begin test
            System.Threading.Thread.Sleep(1000);

            int num_pings = 0;
            int scan_number = 4; // 4 means start sending with dataLen = 2^5 = 32
            bool last_scan = false;
            int pingID = -1;
            int current_index_in_batch = 0;
            int ping_index_in_batch = 0;
            bool sentPing = false;

            /********************
             *  Main sending loop
             */
            for ( ulong loop=0; (_NumIter == 0)||(loop<(ulong)_NumIter); ++loop ) 
            {      
                if ( _SleepMillisec > 0 ) {
                    System.Threading.Thread.Sleep(_SleepMillisec);
                }
                
                if ( _SpinLoopCount > 0 ) {
                    // spin, spin, spin
                    for (int m=0; m<_SpinLoopCount; ++m) {
                        double a, b, c;
                        a = 1.1;
                        b = 3.1415;
                        c = a/b*m;
                    }
                }

                pingID = -1;


                // only send latency pings if is publisher with ID 0
                // In batch mode, latency pings are sent once every LatencyCount batches
                if ( (_PubID == 0) && (((loop/(ulong)_SamplesPerBatch) % (ulong)_LatencyCount) == 0) ) 
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
                    if ( current_index_in_batch == ping_index_in_batch && !sentPing ) 
                    {

                        // If running in scan mode, dataLen under test is changed
                        // after NUM_LATENCY_PINGS_PER_DATA_SIZE pings are sent
                        if (_IsScan)
                        {

                            // change data length for scan mode
                            if ((num_pings % NUM_LATENCY_PINGS_PER_DATA_SIZE == 0))
                            {
                                int new_size;
                                
                                // flush anything that was previously sent
                                writer.Flush();

                                if (last_scan)
                                {   
                                // end of scan test
                                    break;
                                }

                                new_size = (1 << (++scan_number)) - OVERHEAD_BYTES;
                                
                                if (scan_number == 17)
                                {   
                                // end of scan test
                                    break;
                                }

                                if(new_size > _MaxBinDataSize) {
                                    // last scan
                                    new_size = _MaxBinDataSize - OVERHEAD_BYTES;
                                    last_scan = true;
                                }

                                // must reduce the number of latency pings to send
                                // for larger data sizes because the time to send
                                // the same number of messages for a given size
                                // increases with the data size.
                                // 
                                // If we don't do this, the time to run a complete
                                // scan would be in the hours
                                switch (scan_number) 
                                {
                                  case 16:
                                    new_size = TestMessage.MAX_DATA_SIZE - OVERHEAD_BYTES;
                                    _LatencyCount /= 2;
                                    break;
                                    
                                  case 9:
                                  case 11:
                                  case 13:
                                  case 14:
                                  case 15:
                                    _LatencyCount /= 2;
                                    break;
                                  default:
                                    break;
                                }  
                                
                                if (_LatencyCount == 0)
                                {
                                    _LatencyCount = 1;
                                }
                                
                                message.size = LENGTH_CHANGED_SIZE;
                                // must set latency_ping so that a subscriber sends us
                                // back the LENGTH_CHANGED_SIZE message
                                message.latency_ping = num_pings % _NumSubscribers;

                                for (int i=0; i<30; ++i) {
                                    writer.Send(message);
                                    writer.Flush();
                                }
                      
                                // sleep to allow packet to be pinged back
                                System.Threading.Thread.Sleep(1000);
                                
                                message.size = new_size;
                                /* Reset _SamplePerBatch */
                                if (_BatchSize != 0) 
                                {
                                    _SamplesPerBatch = _BatchSize/(message.size + OVERHEAD_BYTES);
                                    if (_SamplesPerBatch == 0) {
                                        _SamplesPerBatch = 1;
                                    }
                                }
                                else 
                                {
                                    _SamplesPerBatch = 1;
                                }
                                ping_index_in_batch = 0;
                                current_index_in_batch = 0;
                            }
                        }

                        // Each time ask a different subscriber to echo back
                        pingID = num_pings % _NumSubscribers;
                        ulong now = GetTimeUsec();
                        message.timestamp_sec = (int)((now >> 32) & 0xFFFFFFFF);
                        message.timestamp_usec = (uint)(now & 0xFFFFFFFF);

                        ++num_pings;
                        ping_index_in_batch = (ping_index_in_batch + 1) % _SamplesPerBatch;
                        sentPing = true;
                    }
                }

                current_index_in_batch = (current_index_in_batch + 1) % _SamplesPerBatch;

                message.seq_num = (uint)loop;
                message.latency_ping = pingID;
                writer.Send(message);

                if (_LatencyTest && sentPing)
                {
                    if (_isReliable) {
                        writer.WaitForPingResponse();
                    }
                    else {
			/* time out in milliseconds */
                        writer.WaitForPingResponse(200);
                    }
                }

                // come to the beginning of another batch
                if (current_index_in_batch == 0)
                {
                    sentPing = false;
                }
            }

            // In case of batching, flush
            writer.Flush();

            // Test has finished, send end of test message, send multiple
            // times in case of best effort
            message.size = FINISHED_SIZE;
            for (int j=0; j<30; ++j)
            {
                writer.Send(message);
                writer.Flush();
            }

            if (_UseReadThread)
            {
                reader.Shutdown();
            }

            System.Threading.Thread.Sleep(1000);
            Console.Error.Write("Finishing test...\n");
            Console.Out.Flush();
            return;
        }

        [DllImport("kernel32.dll")]
        extern static short QueryPerformanceCounter(ref long x);
        [DllImport("kernel32.dll")]
        extern static short QueryPerformanceFrequency(ref long x); 
        
        public static ulong GetTimeUsec() 
        {
            long current_count = 0;
            QueryPerformanceCounter(ref current_count);
            return (ulong) ((current_count * 1000000 / _ClockFrequency));
        }

        private int  _DataLen = 100;
        private int  _BatchSize = 0;
        private int  _MaxBinDataSize = TestMessage.MAX_DATA_SIZE;
        private int  _SamplesPerBatch = 1;
        private int  _NumIter = 0;
        private bool _IsPub = false;
        private bool _IsScan = false;
        private bool  _UseReadThread = false;
        private int  _SpinLoopCount = 0;
        private int  _SleepMillisec = 0;
        private int  _LatencyCount = -1;
        private int  _NumSubscribers = 1;
        private int  _NumPublishers  = 1;
        private int  _InstanceCount = 1;
        private IMessaging _MessagingImpl = null;
        private string _ConfigFile = "perftest.ini";
        private bool _LatencyTest = false;
        private bool _isReliable = true;

        private static int  _SubID = 0;
        private static int  _PubID = 0;
        private static bool _PrintIntervals = true;
        private static bool _IsDebug = false;
        private static long _ClockFrequency = 0;
        public const string _LatencyTopicName = "Latency";
        public const string _ThroughputTopicName = "Throughput";
        public const string _AnnouncementTopicName = "Announcement";

        public string[] _MessagingArgv = null;
        public int _MessagingArgc = 0;

        // Number of bytes sent in messages besides user data
        public const int OVERHEAD_BYTES = 28;

        // When running a scan, this determines the number of
        // latency pings that will be sent before increasing the 
        // data size
        private const int NUM_LATENCY_PINGS_PER_DATA_SIZE = 1000;

        // Flag used to indicate message is used for initialization only
        private const int INITIALIZE_SIZE = 1234;
        // Flag used to indicate end of test
        private const int FINISHED_SIZE = 1235;
        // Flag used to indicate end of test
        private const int LENGTH_CHANGED_SIZE = 1236;

    }

} // namespace


