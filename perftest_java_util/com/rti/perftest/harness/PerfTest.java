/* $Id: PerfTest.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $

(c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
Permission to modify and use for internal purposes granted.   	
This software is provided "as is", without warranty, express or implied.

modification history:
--------------------
09jul10,jsr Added new waitForPingResponse for best effort mode
08jul10,jsr Fixing LatencyListener constructor calls
07jul10,eys Cleanup perftest parameters
03may10,jsr Adapted for the new Latencytest option
21may09,fcs Fixed channel initialization
14may09,fcs Fixed scan output
14may09,fcs Added instances to INI
14may09,fcs Fixed command-line arguments processing
29may09,jsr Added detection of wrong command line parameter
23apr09,jsr Changed to stderr the error and status messages
21apr09,jsr Reformat the help menu
24oct08,rbw If next size in scan is too big, do the best we can
23oct08,rbw Fail test if send() fails
08oct08,rbw Refactored DDS dependencies to
            com.rti.perftest.impl.PerfTestLauncher
22aug08,eys Flush after every send when initializing the channel
22aug08,eys decrease checking interval on subscriber to gather latency
            numbers
20aug08,eys initialize channel by writing InstanceCount initially
14aug08,ch  changed key to be 4 bytes long -> overhead change
13aug08,ch  added check to not exceed MAX_BINDATA_SIZE
09aug08,ch  increased sleep after FINISHED_MESSAGE send to make sure
            it gets through in the multi-instance case
11jun08,rbw Enable reflective test implementation loading
12may08,hhw Fixed some length checks to accommodate 32 byte overhead.
09may08,ch  Changed OVERHEAD
04may08,hhw Modified batch processing.
            Synchronized to perftest.cxx changes.
22apr08,fcs Removed yield from spinning
21apr08,ch  Output modifications for automation
21apr08,fcs Changed OVERHEAD
08apr08,rbw Fixed compile errors
07apr08,hhw Now printing end of test in listener to avoid race condition. 
04apr08,rbw Reverted some of yesterday's summary calculation changes
03apr08,rbw Improved summary output
03apr08,rbw Improvements to time calculations
03apr08,rbw More printf() fixes
02apr08,rbw Improved config file checking; fixed syntax error in printf()
            calls; minor stylistic improvements
02apr08,rbw Moved to package com.rti.perftest.harness to distinguish between
            (1) RTI-specific test implementation and (2) generic test harness
02apr08,rbw Implemented *.ini file parsing
01apr08,rbw Follow Java naming conventions
01apr08,rbw Created
=========================================================================== */

package com.rti.perftest.harness;

import java.io.File;
import java.io.IOException;

import com.rti.perftest.IMessaging;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;
import com.rti.perftest.ini.Ini;

import java.util.concurrent.TimeUnit;


// ===========================================================================

/**
 * The implementation-independent "main" class. Its subclasses provide the
 * implementation class that binds the test harness to a specific middleware
 * technology.
 */
public final class PerfTest {
    // -----------------------------------------------------------------------
    // Public Fields
    // -----------------------------------------------------------------------

    public static final String LATENCY_TOPIC_NAME = "Latency";
    public static final String THROUGHPUT_TOPIC_NAME = "Throughput";
    public static final String ANNOUNCEMENT_TOPIC_NAME = "Announcement";

    // Number of bytes sent in messages besides user data
    public static final int OVERHEAD_BYTES = 28;



    // -----------------------------------------------------------------------
    // Package Fields
    // -----------------------------------------------------------------------

    // Flag used to indicate message is used for initialization only
    /*package*/ static final int INITIALIZE_SIZE = 1234;

    // Flag used to indicate end of test
    /*package*/ static final int FINISHED_SIZE = 1235;

    // Flag used to indicate end of test
    /*package*/ static final int LENGTH_CHANGED_SIZE = 1236;


    /*package*/ static int subID = 0;

    /*package*/ static int pubID = 0;

    /*package*/ static boolean printIntervals = true;
    
    
    
    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    private static boolean _isDebug = false;
    
    // When running a scan, this determines the number of
    // latency pings that will be sent before increasing the 
    // data size
    private static final int NUM_LATENCY_PINGS_PER_DATA_SIZE = 1000;

    private int     _dataLen = 100;      
    private int     _batchSize = 0;
    private int     _maxBinDataSize = TestMessage.MAX_DATA_SIZE;
    private int     _samplesPerBatch = 1;
    private int     _numIter = 0;
    private boolean _isPub = false;
    private boolean _isScan = false;
    private boolean _useReadThread = false;
    private int     _spinLoopCount = 0;
    private int     _sleepMillisec = 0;
    private int     _latencyCount = -1;
    private int     _numSubscribers = 1;
    private int     _numPublishers  = 1;
    private int     _instanceCount = 1;
    private IMessaging _messagingImpl = null;
    private File    _configFile = new File("perftest.ini");
    private String[] _messagingArgv = null;
    private int      _messagingArgc = 0;
    private boolean  _latencyTest = false;
    private boolean  _isReliable = true;

    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------

    public static void runTest(Class<? extends IMessaging> defaultImplClass,
                               String[] argv) {
        PerfTest app = new PerfTest();
        app.run(defaultImplClass, argv);
        app.dispose();
    }


    public void dispose() {
        _messagingImpl.shutdown();
        System.err.print("Test ended.\n");
    }



    // -----------------------------------------------------------------------
    // Package Methods
    // -----------------------------------------------------------------------

    /**
     * Get a time stamp with microsecond precision.
     * 
     * Note that the result is not guaranteed to be measured relative to any
     * particular point in time -- it may even be negative! 
     */
    /*package*/ static long getTimeUsec() {
//        return System.currentTimeMillis() * 1000;
        return System.nanoTime() / 1000;
    }



    // -----------------------------------------------------------------------
    // Private Methods
    // -----------------------------------------------------------------------

    /**
     * Reflectively find and instantiate a test implementation.
     */
    private IMessaging newInstance(Class<?> implClass) {
        try {
            return (IMessaging) implClass.newInstance();
        } catch (RuntimeException rex) {
            throw rex;
        } catch (Exception ex) {
            throw new IllegalStateException(ex.getMessage(), ex);
        }
    }


    private void run(Class<? extends IMessaging> implClass,
                     String[] argv) {
        _messagingImpl = newInstance(implClass);

        if ( !parseConfig(argv) ) {
            return;
        }

        if ( !_messagingImpl.initialize(_messagingArgc, _messagingArgv) ) {
            return;
        }
        _batchSize = _messagingImpl.getBatchSize();
        _maxBinDataSize = _messagingImpl.getMaxBinDataSize();

        if (_batchSize != 0) {
            _samplesPerBatch = _batchSize/_dataLen;
            if (_samplesPerBatch == 0) {
                _samplesPerBatch = 1;
            }
        } else {
            _samplesPerBatch = 1;
        }

        if (_isPub) {
            publisher();
        } else {
            subscriber();
        }
    }


    private boolean parseConfig(String[] argv) {

        _messagingArgc = 0;
        _messagingArgv = new String[argv.length];

        String usage_string = 
            /**************************************************************************/
            "Usage:\n" +
            "\tperftest_java [options]\n" +
            "\nWhere [options] are (case insensitive, partial match OK):\n\n" +
            "\t-help                   - Print this usage message and exit\n" +
            "\t-pub                    - Set test to be a publisher\n" +
            "\t-sub                    - Set test to be a subscriber (default)\n" +
            "\t-sidMultiSubTest <id>   - Set id of the subscriber in a multi-subscriber\n" +
            "\t                          test, default 0\n" +
            "\t-pidMultiPubTest <id>   - Set id of the publisher in a multi-publisher\n" +
            "\t                          test, default 0. Only publisher 0 sends\n" +
            "\t                          latency pings\n" +
            "\t-configFile <filename>  - Set the name of the .ini configuration file,\n" +
            "\t                          default perftest.ini\n" +
            "\t-dataLen <bytes>        - Set length of payload for each send,\n" +
	    "\t                          default 100\n" +
            "\t-numIter <count>        - Set number of messages to send,\n" +
	    "\t                          default 0 (infinite)\n" +
            "\t-instances <#instance>  - set the number of instances (keys) to iterate\n" +
	    "\t                          over when publishing, default 1\n" +
            "\t-sleep <millisec>       - Time to sleep between each send, default 0\n" +
            "\t-spin <count>           - Number of times to run in spin loop between\n"+
            "\t                          each send, default 0\n" +
            "\t-latencyCount <count>   - Number samples (or batches) to send before\n" +
            "\t                          a latency ping packet is sent, default\n" +
	    "\t                          10000 if -latencyTest is not specified,\n" +
	    "\t                          1 if -latencyTest is specified\n" +
            "\t-numSubscribers <count> - Number of subscribers running in test,\n" +
            "\t                          default 1\n" +
            "\t-numPublishers <count>  - Number of publishers running in test,\n"+
            "\t                          default 1\n" +
            "\t-scan                   - Run test in scan mode, traversing a range of\n" +
            "\t                          data sizes, 32 - " + TestMessage.MAX_DATA_SIZE + "\n" +
            "\t-noPrintIntervals       - Don't print statistics at intervals during\n" +
            "\t                          test\n" +
            "\t-useReadThread          - Use separate thread instead of callback to\n"+
            "\t                          read data\n" +
            "\t-latencyTest            - Run a latency test consisting of a ping-pong \n" +
            "\t                          synchronous communication \n" +
            "\t-debug                  - Run in debug mode\n";


        int argc = argv.length;
        if (argc < 0) {
            System.err.print(usage_string);
            _messagingImpl.printCmdLineHelp();
            return false;
        }


        // first scan for configFile
        for (int i = 0; i < argc; ++i) {
            if ( "-help".toLowerCase().startsWith(argv[i].toLowerCase())) {
                System.err.print(usage_string);
                _messagingImpl.printCmdLineHelp();
                return false;
            }
            if ("-configFile".toLowerCase().startsWith(argv[i].toLowerCase())) {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
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

            Ini.Section config = configSource.section("perftest");
            if (config == null) {
                System.err.println(
                        "Could not find section [perftest] in file " +
                        _configFile +
                        ".");
                return false;
            }

            _numIter        = config.getInt("num iter", _numIter);
            _dataLen        = config.getInt("data length", _dataLen);
            _spinLoopCount  = config.getInt("spin loop count", _spinLoopCount);
            _sleepMillisec  = config.getInt("sleep millisec", _sleepMillisec);
            _latencyCount   = config.getInt("latency count", _latencyCount);
            _numSubscribers = config.getInt("num subscribers", _numSubscribers);
            _numPublishers = config.getInt("num publishers", _numPublishers);
            _isScan         = config.getBoolean("scan", _isScan);
            printIntervals  = config.getBoolean("print intervals", printIntervals);
            _useReadThread  = config.getBoolean("use read thread", _useReadThread);
            _isDebug        = config.getBoolean("is debug", _isDebug);
            _instanceCount  = config.getInt("instances", _instanceCount);
            _latencyTest	= config.getBoolean("run latency test", _latencyTest);
            _isReliable = config.getBoolean("is reliable", _isReliable);
        } else {
            System.err.println(
                    "Warning: config file doesn't exist: " + _configFile);
        }


        // now load everything else, command line params override config file
        for (int i = 0; i < argc; ++i) 
        {
            if ("-pub".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isPub = true;
            }
            else if ("-sub".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _isPub = false;
            } 
            else if ("-sidMultiSubTest".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <id> after -sidMultiSubTest\n");
                    return false;
                }
                try {
                    subID = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad id for subscriber\n");
                    return false;
                }
            } 
            else if ("-pidMultiPubTest".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <id> after -pidMultiPubTest\n");
                    return false;
                }
                try {
                    pubID = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad id for publisher\n");
                    return false;
                }
            }
            else if ("-numIter".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <iter> after -numIter\n");
                    return false;
                }
                try {
                    _numIter = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad numIter\n");
                    return false;
                }
            }
            else if ("-dataLen".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _messagingArgv[_messagingArgc++] = argv[i];

                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <length> after -dataLen\n");
                    return false;
                }

                _messagingArgv[_messagingArgc++] = argv[i];

                try {
                    _dataLen = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad dataLen\n");
                    return false;
                }
                if (_dataLen < OVERHEAD_BYTES) {
                    System.err.println("dataLen must be >= " + OVERHEAD_BYTES);
                    return false;
                }
                if (_dataLen > TestMessage.MAX_DATA_SIZE) {
                    System.err.println("dataLen must be <= " + TestMessage.MAX_DATA_SIZE);
                    return false;
                }
            } 
            else if ( "-spin".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                if (( i == (argc-1)) || argv[++i].startsWith("-") ) {
                    System.err.print("Missing <count> after -spin\n");
                    return false;
                }
                try {
                    _spinLoopCount = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad spin count\n");
                    return false;
                }
            } 
            else if ( "-sleep".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                if (( i == (argc-1)) || argv[++i].startsWith("-") ) {
                    System.err.print("Missing <millisec> after -sleep\n");
                    return false;
                }
                try {
                    _sleepMillisec = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad sleep millisec\n");
                    return false;
                }
            } 
            else if ( "-latencyCount".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                if (( i == (argc-1)) || argv[++i].startsWith("-") ) {
                    System.err.print("Missing <count> after -latencyCount\n");
                    return false;
                }
                try {
                    _latencyCount = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad latency count\n");
                    return false;
                }
            }
            else if ("-numSubscribers".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <count> after -numSubscribers\n");
                    return false;
                }
                try {
                    _numSubscribers = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad num subscribers\n");
                    return false;
                }
            }
            else if ("-numPublishers".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <count> after -numPublishers\n");
                    return false;
                }
                try {
                    _numPublishers = Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad num publishers\n");
                    return false;
                }
            }
            else if ("-scan".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _isScan = true;
            }
            else if ("-noPrintIntervals".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                printIntervals = false;
            } 
            else if ("-useReadThread".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _useReadThread = true;
            }
            else if ("-latencyTest".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _latencyTest = true;
                _messagingArgv[_messagingArgc++] = argv[i];
            }
            else if ("-bestEffort".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isReliable = false;
                _messagingArgv[_messagingArgc++] = argv[i];
            } 
            else if ("-instances".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _messagingArgv[_messagingArgc++] = argv[i];

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <count> after -instances\n");
                    return false;
                }

                _messagingArgv[_messagingArgc++] = argv[i];

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
            }
            else if ("-debug".toLowerCase().startsWith(argv[i].toLowerCase())) 
            {
                _isDebug = true;
                _messagingArgv[_messagingArgc++] = argv[i];
            }
            else {
                _messagingArgv[_messagingArgc++] = argv[i];
            }
        }

        if (_isScan && _numPublishers > 1) 
        {
            System.err.print("_isScan is not supported with more than one publisher\n");
            return false;
        }

        if(_latencyTest) {		
            if(pubID != 0) {
                System.err.printf("Only the publisher with ID = 0 can run the latency test\n");
                return false;
            }
            // With latency test, latency should be 1
            if(_latencyCount == -1) {
                _latencyCount = 1;
            }
	}  

	if(_latencyCount == -1) {
	    _latencyCount = 10000;
	}

        if ((_numIter > 0) && (_numIter < _latencyCount)) {
            System.err.printf(
                "numIter (%1$d) must be greater than latencyCount (%2$d).\n",
                _numIter,
                _latencyCount);
            return false;
        }

        return true;
    }

    /**
     * Subscriber
     */
    private void subscriber() {
        // create latency pong writer
        ThroughputListener reader_listener;
        IMessagingReader   reader;
        IMessagingWriter writer;
        IMessagingWriter announcement_writer;

        writer = _messagingImpl.createWriter(LATENCY_TOPIC_NAME);
        if (writer == null) {
            System.err.print("Problem creating latency writer.\n");
            return;
        }

        // Check if using callbacks or read thread
        if (!_useReadThread) {
            // create latency pong reader
            reader_listener = new ThroughputListener(writer, _numPublishers);
            reader = _messagingImpl.createReader(THROUGHPUT_TOPIC_NAME, reader_listener);
            if (reader == null) {
                System.err.print("Problem creating throughput reader.\n");
                return;
            }
        } else {
            reader = _messagingImpl.createReader(THROUGHPUT_TOPIC_NAME, null);
            if (reader == null) {
                System.err.print("Problem creating throughput reader.\n");
                return;
            }
            reader_listener = new ThroughputListener(writer, reader, _numPublishers);

            final ThroughputListener final_listener = reader_listener;
            Thread thread = new Thread(
                    new Runnable() {
                        public void run() {
                            final_listener.ReadThread();
                        }
                    });
            thread.start();
        }

        // Create announcement writer
        announcement_writer = _messagingImpl.createWriter(ANNOUNCEMENT_TOPIC_NAME);
        
        if (announcement_writer == null) {
             System.err.print("Problem creating announcement writer.\n");
            return;
        }

        // Synchronize with publishers
        System.err.printf("Waiting to discover %1$d publishers ...\n", _numPublishers);
        reader.waitForWriters(_numPublishers);
        announcement_writer.waitForReaders(_numPublishers);
        
        // Send announcement message
        TestMessage message = new TestMessage();
        message.entity_id = subID;
        message.data = new byte[1];
        message.size = 0;
        boolean sent = announcement_writer.send(message);
        announcement_writer.flush();
        if (!sent) {
            System.err.println("*** send() failure: announcement message");
            return;
        }

        System.err.print("Waiting for data...\n");

        // wait for data
        long  now, prev_time, delta;
        long  prev_count = 0;
        long  prev_bytes = 0;
        long  ave_count = 0;
        int    last_data_length = -1;
        long  mps, bps;
        double mps_ave = 0.0, bps_ave = 0.0;
        long  msgsent, bytes, last_msgs, last_bytes;


        now = getTimeUsec();
        while (true) { 
            prev_time = now;
            sleep(1000);
            now = getTimeUsec();

            if (reader_listener.endTest) {
                break;
            }

            if (printIntervals) {
                if (last_data_length != reader_listener.lastDataLength) {
                    last_data_length = reader_listener.lastDataLength;
                    prev_count = reader_listener.packetsReceived;
                    prev_bytes = reader_listener.bytesReceived;
                    bps_ave = 0;
                    mps_ave = 0;
                    ave_count = 0;
                    continue;
                }

                last_msgs = reader_listener.packetsReceived;
                last_bytes = reader_listener.bytesReceived;
                msgsent = last_msgs - prev_count;
                bytes = last_bytes - prev_bytes;
                prev_count = last_msgs;
                prev_bytes = last_bytes;
                delta = now - prev_time;
                mps = (msgsent * 1000000 / delta);
                bps = (bytes * 1000000 / delta);

                // calculations of overall average of mps and bps
                ++ave_count;
                bps_ave = bps_ave + (bps - bps_ave) / ave_count;
                mps_ave = mps_ave + (mps - mps_ave) / ave_count;

                if (last_msgs > 0) {
                    System.out.printf(
                            "Packets: %1$8d  Packets/s: %2$7d  Packets/s(ave): %3$7.0f  " +
                            "Mbps: %4$7.1f  Mbps(ave): %5$7.1f  Lost: %6$d\n",
                            last_msgs, mps, mps_ave,
                            bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                            reader_listener.missingPackets);
                }
            }
        }

        if (_useReadThread) {
            reader.shutdown();
        }

        sleep(1000);
        System.err.print("Finishing test...\n");
    }


    /**
     * Publisher
     */
    private void publisher() {
        LatencyListener reader_listener = null;
        IMessagingReader reader;
        IMessagingWriter writer;
        AnnouncementListener  announcement_reader_listener = null;
        IMessagingReader announcement_reader;
        int num_latency;
        int initialize_sample_count = 50;

        // create throughput/ping writer
        writer = _messagingImpl.createWriter(THROUGHPUT_TOPIC_NAME);

        if (writer == null) {
            System.err.print("Problem creating throughput writer.\n");
            return;
        }

        // calculate number of latency pings that will be sent per data size
        if (_isScan) {
            num_latency = NUM_LATENCY_PINGS_PER_DATA_SIZE;
        } else {
            num_latency = ((_numIter/_samplesPerBatch) / _latencyCount);
        }

        // in batch mode, might have to send another ping
        if (_samplesPerBatch > 1) {
          ++num_latency;
        }

        // Only publisher with ID 0 will send/receive pings
        if (pubID == 0) {
            // Check if using callbacks or read thread
            if (!_useReadThread) {
                // create latency pong reader
                reader_listener = new LatencyListener(num_latency,_latencyTest?writer:null);
                reader = _messagingImpl.createReader(LATENCY_TOPIC_NAME, reader_listener);
                if (reader == null)
                {
                    System.err.print("Problem creating latency reader.\n");
                    return;
                }
            }
            else
            {
                reader = _messagingImpl.createReader(LATENCY_TOPIC_NAME, null);
                if (reader == null)
                {
                    System.err.print("Problem creating latency reader.\n");
                    return;
                }
                reader_listener = new LatencyListener(reader, _latencyTest?writer:null, num_latency);
                
                final LatencyListener final_listener = reader_listener;
                Thread thread = new Thread(
                    new Runnable() {
                            public void run() {
                                final_listener.readThread();
                            }
                        });
                thread.start();
            }
        }
        else {
            reader = null;
        }

        /* Create Announcement reader
         * A Subscriber will send a message on this channel once it discovers
         * every Publisher
         */
        announcement_reader_listener = new AnnouncementListener();
        announcement_reader = _messagingImpl.createReader(ANNOUNCEMENT_TOPIC_NAME, 
                                                          announcement_reader_listener);
        if (announcement_reader == null)
        {
            System.err.print("Problem creating announcement reader.\n");
            return ;
        }

        System.err.printf("Waiting to discover %1$d subscriber(s)...\n", _numSubscribers);
        writer.waitForReaders(_numSubscribers);

        // We have to wait until every Subscriber sends an announcement message
        // indicating that it has discovered every Publisher
        System.err.print("Waiting for subscribers announcement ...\n");
        while (_numSubscribers > announcement_reader_listener.announced_subscribers) {
            sleep(1000);
        }
        
        // Allocate data and set size
        TestMessage message = new TestMessage();
        message.entity_id = pubID;
        message.data = new byte[TestMessage.MAX_DATA_SIZE];

        System.err.print("Publishing data...\n");

        // initialize data pathways by sending some initial pings
        if (initialize_sample_count < _instanceCount) {
            initialize_sample_count = _instanceCount;
        }
        message.size = INITIALIZE_SIZE;
        for (int i = 0; i < initialize_sample_count; i++)
        {
            // Send test initialization message
            if (!writer.send(message)) {
                System.out.println(
                        "*** send() failure: initialization message");
                return;
            }

            if (i % 10 == 0) {
                sleep(1);
            }
        }
        writer.flush();

        // Set data size, account for other bytes in message
        message.size = _dataLen - OVERHEAD_BYTES;

        // Sleep 1 second, then begin test
        sleep(1000);

        int num_pings = 0;
        int scan_number = 4; // 4 means start sending with dataLen = 2^5 = 32
        boolean last_scan = false;
        int pingID = -1;
        int current_index_in_batch = 0;
        int ping_index_in_batch = 0;
        boolean sentPing = false;

        /********************
         *  Main sending loop
         */
        for (long loop=0; _numIter == 0 || loop < _numIter; ++loop ) 
        {
            if ( _sleepMillisec > 0 ) {
                sleep(_sleepMillisec);
            }

            if ( _spinLoopCount > 0 ) {
                // spin, spin, spin
                for (int m = 0; m < _spinLoopCount; ++m) {
                    double a, b, c;
                    a = 1.1;
                    b = 3.1415;
                    c = a/b*m;
                    a = c;
                }
            }

            pingID = -1;

            // only send latency pings if is publisher with ID 0
            // In batch mode, latency pings are sent once every LatencyCount batches
            if ( (pubID == 0) && (((loop/_samplesPerBatch) %_latencyCount) == 0) ) 
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
                    if (_isScan) {
                        
                        // change data length for scan mode
                        if ((num_pings % NUM_LATENCY_PINGS_PER_DATA_SIZE == 0))
                        {
                            int new_size;
                            
                                // flush anything that was previously sent
                            writer.flush();

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

                            if (new_size > _maxBinDataSize) {
                                // last scan
                                new_size = _maxBinDataSize - OVERHEAD_BYTES;
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
                                _latencyCount /= 2;
                                break;
                                
                              case 9:
                              case 11:
                              case 13:
                              case 14:
                              case 15:
                                _latencyCount /= 2;
                                break;
                              default:
                                break;
                            }  
                            
                            if (_latencyCount == 0)
                            {
                                _latencyCount = 1;
                            }
                            
                            message.size = LENGTH_CHANGED_SIZE;
                            // must set latency_ping so that a subscriber sends us
                            // back the LENGTH_CHANGED_SIZE message
                            message.latency_ping = num_pings % _numSubscribers;

                            for (int i=0; i<30; ++i) {
                                boolean sent = writer.send(message);
                                writer.flush();
                                if (!sent) {
                                    System.err.println("*** send() failure");
                                    return;
                                }
                            }
                            
                            // sleep to allow packet to be pinged back
                            sleep(1000);
                            
                            message.size = new_size;
                            /* Reset _SamplePerBatch */
                            if (_batchSize != 0) {
                                _samplesPerBatch = _batchSize/(message.size + OVERHEAD_BYTES);
                                if (_samplesPerBatch == 0) {
                                    _samplesPerBatch = 1;
                                }
                            } else {
                                _samplesPerBatch = 1;
                            }
                            ping_index_in_batch = 0;
                            current_index_in_batch = 0;
                        }
                    }

                    // Each time ask a different subscriber to echo back
                    pingID = num_pings % _numSubscribers;
                    long now = getTimeUsec();   // may be negative!
                    /* Note that we're not really treating 'now' as a time in
                     * microseconds: we're just taking a 64-bit number and
                     * splitting it into 2 32-bit numbers. We'll reassemble on
                     * the other side.
                     */
                    message.timestamp_sec  = (int)(now >>> 32); // high int
                    message.timestamp_usec = (int) now;         // low int
                    
                    ++num_pings;
                    ping_index_in_batch = (ping_index_in_batch + 1) % _samplesPerBatch;
                    sentPing = true;
                }
            }

            current_index_in_batch = (current_index_in_batch + 1) % _samplesPerBatch;

            message.seq_num = (int)loop;
            message.latency_ping = pingID;
            if (!writer.send(message)) {
                System.err.println("*** send() failure");
                return;
            }

            if(_latencyTest && sentPing) {
                if (_isReliable) {
                    writer.waitForPingResponse();
                } else {
                    writer.waitForPingResponse((long)200,TimeUnit.MILLISECONDS);
                }
                
            }

            // come to the beginning of another batch
            if (current_index_in_batch == 0) {
                sentPing = false;
            }
        }

        // In case of batching, flush
        writer.flush();

        // Test has finished, send end of test message, send multiple
        // times in case of best effort
        message.size = FINISHED_SIZE;
        for (int j=0; j<30; ++j) {
            boolean sent = writer.send(message);
            writer.flush();
            if (!sent) {
                System.err.println("*** send() failure");
                return;
            }
        }

        if (_useReadThread) {
            assert reader != null;
            reader.shutdown();
        }

        sleep(1000);
        System.err.print("Finishing test...\n");

        return;
    }


    private static void sleep(int millis) {
        try {
            Thread.sleep(millis);
        } catch (InterruptedException interrupted) {
            System.err.println("Interrupted");
            return;
        }
    }

}

// ===========================================================================
// End of $Id: PerfTest.java,v 1.1.2.1 2014/04/01 11:56:54 juanjo Exp $
