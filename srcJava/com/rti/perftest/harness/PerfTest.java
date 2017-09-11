/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;

import com.rti.dds.infrastructure.Duration_t;
import com.rti.perftest.IMessaging;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;
import com.rti.perftest.harness.PerftestTimerTask;
import com.rti.perftest.gen.MAX_SYNCHRONOUS_SIZE;
import com.rti.perftest.gen.MAX_BOUNDED_SEQ_SIZE;
import com.rti.perftest.gen.MAX_PERFTEST_SAMPLE_SIZE;
import com.rti.ndds.Utility;

import java.util.StringTokenizer;
import java.util.ArrayList;
import java.util.Collections;
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

    public static final Duration_t timeout_wait_for_ack =
            new Duration_t(0, 10000000);

    // Number of bytes sent in messages besides user data
    public static final int OVERHEAD_BYTES = 28;

    // MAX_PERFTEST_SAMPLE_SIZE for java (2GB-5B)
    public static final int MAX_PERFTEST_SAMPLE_SIZE_JAVA = 2147483642;

    /*
     * PERFTEST-108
     * If we are performing a latency test, the default number for _NumIter will
     * be 10 times smaller than the default when performing a throughput test.
     * This will allow Perftest to work better in embedded platforms since the
     * _NumIter parameter sets the size of certain arrays in the latency test
     * mode.
     */
    public static final long numIterDefaultLatencyTest = 10000000;


    // -----------------------------------------------------------------------
    // Package Fields
    // -----------------------------------------------------------------------

    // Flag used to indicate message is used for initialization only
    /*package*/ static final int INITIALIZE_SIZE = 1234;

    // Flag used to indicate end of test
    /*package*/ static final int FINISHED_SIZE = 1235;

    // Flag used to indicate end of test
    public static final int LENGTH_CHANGED_SIZE = 1236;


    /*package*/ static int subID = 0;

    /*package*/ static int pubID = 0;

    /*package*/ static boolean printIntervals = true;

    /*package*/ static boolean _showCpu = false;

    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    //private static boolean _isDebug = false;

    private long     _dataLen = 100;
    private int     _batchSize = 0;
    private int     _samplesPerBatch = 1;
    private long    _numIter = 100000000;
    private boolean _isPub = false;
    private boolean _isScan = false;
    private ArrayList<Long> _scanDataLenSizes = new ArrayList<Long>();
    private boolean _useReadThread = false;
    private long     _spinLoopCount = 0;
    private long    _sleepNanosec = 0;
    private int     _latencyCount = -1;
    private int     _numSubscribers = 1;
    private int     _numPublishers  = 1;
    private int     _instanceCount = 1;
    private IMessaging _messagingImpl = null;
    private String[] _messagingArgv = null;
    private int      _messagingArgc = 0;
    private boolean  _latencyTest = false;
    private boolean  _isReliable = true;
    private long     _pubRate = 0;
    private boolean _pubRateMethodSpin = true;
    private long     _executionTime = 0;
    private boolean  _displayWriterStats = false;
    private boolean  _useCft = false;
    private PerftestTimerTask timertask = new PerftestTimerTask(this);
    /* Indicates when the test should exit due to timeout */
    private boolean testCompleted = false;
    private boolean testCompletedScan = true;

    // Set the default values into the array _scanDataLenSizes vector
    public void set_default_scan_values(){
        _scanDataLenSizes.add((long)32);
        _scanDataLenSizes.add((long)64);
        _scanDataLenSizes.add((long)128);
        _scanDataLenSizes.add((long)256);
        _scanDataLenSizes.add((long)512);
        _scanDataLenSizes.add((long)1024);
        _scanDataLenSizes.add((long)2048);
        _scanDataLenSizes.add((long)4096);
        _scanDataLenSizes.add((long)8192);
        _scanDataLenSizes.add((long)16384);
        _scanDataLenSizes.add((long)32768);
        _scanDataLenSizes.add((long)63000);

    }
    // -----------------------------------------------------------------------
    // Public Methods
    // -----------------------------------------------------------------------
    public static void runTest(IMessaging messagingImpl,
                               String[] argv) {
        PerfTest app = new PerfTest();
        app.run(messagingImpl, argv);
        app.dispose();
    }


    public void dispose() {
        _messagingImpl.shutdown();
        System.err.print("Test ended.\n");
    }

    public void finishTest() {
        testCompleted = true;
    }

    public void finishTestScan() {
        testCompletedScan = true;
    }

    static public int getMaxPerftestSampleSizeJava(){
        if (MAX_PERFTEST_SAMPLE_SIZE.VALUE > 2147483642){
            return 2147483642; //max value for a buffer in Java
        }else {
            return MAX_PERFTEST_SAMPLE_SIZE.VALUE;
        }
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

    private void run(IMessaging messagingImpl,
                     String[] argv) {

        _messagingImpl = messagingImpl;

        if ( !parseConfig(argv) ) {
            return;
        }

        if ( !_messagingImpl.initialize(_messagingArgc, _messagingArgv) ) {
            return;
        }
        _batchSize = _messagingImpl.getBatchSize();

        if (_batchSize != 0) {
            _samplesPerBatch = _batchSize/(int)_dataLen;
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
            "\t-sidMultiSubTest <id>   - Set id of the subscriber in a\n" +
            "\t                          multi-subscriber test, default 0\n" +
            "\t-pidMultiPubTest <id>   - Set id of the publisher in a multi-publisher\n" +
            "\t                          test, default 0. Only publisher 0 sends\n" +
            "\t                          latency pings\n" +
            "\t-dataLen <bytes>        - Set length of payload for each send,\n" +
            "\t                          default 100\n" +
            "\t-unbounded <allocation_threshold> - Use unbounded Sequences\n" +
            "\t                                    <allocation_threshold> is optional, default 2*dataLen up to "+ MAX_BOUNDED_SEQ_SIZE.VALUE +" Bytes.\n" +
            "\t-numIter <count>        - Set number of messages to send, default is\n" +
            "\t                          100000000 for Throughput tests or 10000000\n" +
            "\t                          for Latency tests. See -executionTime.\n" +
            "\t-instances <#instance>  - Set the number of instances (keys) to iterate\n" +
            "\t                          over when publishing, default 1\n" +
            "\t-writeInstance <instance> - Set the instance number to be sent. \n" +
            "\t                          Digit: 'specific instance-digit'\n" +
            "\t                          -WriteInstance parameter cannot be bigger than the number of instances.\n" +
            "\t                          default 'Round-Robin schedule'\n" +
            "\t-sleep <millisec>       - Time to sleep between each send, default 0\n" +
            "\t-latencyCount <count>   - Number samples (or batches) to send before\n" +
            "\t                          a latency ping packet is sent, default\n" +
            "\t                          10000 if -latencyTest is not specified,\n" +
            "\t                          1 if -latencyTest is specified.\n" +
            "\t-numSubscribers <count> - Number of subscribers running in test,\n" +
            "\t                          default 1\n" +
            "\t-numPublishers <count>  - Number of publishers running in test,\n"+
            "\t                          default 1\n" +
            "\t-scan <size1>:<size2>:...:<sizeN> - Run test in scan mode, traversing\n" +
            "\t                                    a range of sample data sizes from\n" +
            "\t                                    [32,63000] or [63001,2147483128] bytes,\n" +
            "\t                                    in the case that you are using large data or not.\n" +
            "\t                                    The list of sizes is optional.\n" +
            "\t                                    Default values are '32:64:128:256:512:1024:2048:4096:8192:16384:32768:63000'\n" +
            "\t                                    Default: Not set\n" +
            "\t-noPrintIntervals       - Don't print statistics at intervals during\n" +
            "\t                          test\n" +
            "\t-useReadThread          - Use separate thread instead of callback to\n"+
            "\t                          read data\n" +
            "\t-latencyTest            - Run a latency test consisting of a ping-pong \n" +
            "\t                          synchronous communication \n" +
            "\t-verbosity <level>      - Run with different levels of verbosity:\n" +
            "\t                          0 - SILENT, 1 - ERROR, 2 - WARNING,\n" +
            "\t                          3 - ALL. Default: 1\n" +
            "\t-pubRate <samples/s>:<method>    - Limit the throughput to the specified number\n" +
            "\t                                   of samples/s, default 0 (don't limit)\n" +
            "\t                                   [OPTIONAL] Method to control the throughput can be:\n" +
            "\t                                   'spin' or 'sleep'\n" +
            "\t                                 Default method: spin\n" +
            "\t-keyed                  - Use keyed data (default: unkeyed)\n"+
            "\t-executionTime <sec>    - Set a maximum duration for the test. The\n"+
            "\t                          first condition triggered will finish the\n"+
            "\t                          test: number of samples or execution time.\n"+
            "\t                          Default 0 (don't set execution time)\n"+
            "\t-writerStats            - Display the Pulled Sample count stats for\n"+
            "\t                          reliable protocol debugging purposes.\n"+
            "\t                          Default: Not set\n" +
            "\t-cpu                   - Display the cpu percent use by the process\n" +
            "\t                          Default: Not set\n" +
            "\t-cft <start>:<end>      - Use a Content Filtered Topic for the Throughput topic in the subscriber side.\n" +
            "\t                          Specify 2 parameters: <start> and <end> to receive samples with a key in that range.\n" +
            "\t                          Specify only 1 parameter to receive samples with that exact key.\n" +
            "\t                          Default: Not set\n";

        int argc = argv.length;
        if (argc < 0) {
            System.err.print(usage_string);
            _messagingImpl.printCmdLineHelp();
            return false;
        }

        /*
         * PERFTEST-108
         * We add this boolean value to check if we are explicity changing the
         * number of iterations via command line paramenter. This will only be
         * used if this is a latency test to decrease or not the default number
         * of iterations.
         */
        boolean numIterSet = false;

        for (int i = 0; i < argc; ++i)
        {
            if ("-help".toLowerCase().startsWith(argv[i].toLowerCase())) {
                System.err.print(usage_string);
                _messagingImpl.printCmdLineHelp();
                return false;
            }
            if ("-pub".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _isPub = true;
                _messagingArgv[_messagingArgc++] = argv[i];
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
                    _numIter = (long)Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad numIter\n");
                    return false;
                }
                if (_numIter < 1) {
                    System.err.print("-numIter must be > 0");
                    return false;
                }

                numIterSet = true;
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
                    _dataLen = Long.parseLong(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad dataLen\n");
                    return false;
                }
                if (_dataLen < OVERHEAD_BYTES) {
                    System.err.println("dataLen must be >= " + OVERHEAD_BYTES);
                    return false;
                }
                if (_dataLen > getMaxPerftestSampleSizeJava()) {
                    System.err.println("dataLen must be <= " + getMaxPerftestSampleSizeJava());
                    return false;
                }
            }else if ("-unbounded".toLowerCase().startsWith(argv[i].toLowerCase())) {
                _messagingArgv[_messagingArgc++] = argv[i];

                if ((i == (argc - 1)) || argv[i+1].startsWith("-")) {
                    //It if not necessary on that class, just the validation and parse
                } else {
                    ++i;
                    _messagingArgv[_messagingArgc++] = argv[i];
                }
            }
            else if ( "-spin".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                System.err.println("-spin option is deprecated. It will be removed "
                        +"in upcoming releases.");
                if (( i == (argc-1)) || argv[++i].startsWith("-") ) {
                    System.err.print("Missing <count> after -spin\n");
                    return false;
                }
                try {
                    _spinLoopCount = Long.parseLong(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.println("Bad spin count");
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
                    _sleepNanosec = Integer.parseInt(argv[i]);
                    _sleepNanosec *= 1000000;
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
                _messagingArgv[_messagingArgc++] = argv[i];
                if ((i != (argc - 1)) && !argv[1+i].startsWith("-")) {
                    ++i;
                    _messagingArgv[_messagingArgc++] = argv[i];
                    StringTokenizer st = new StringTokenizer(argv[i], ":", true);
                    while (st.hasMoreTokens()) {
                        String s = st.nextToken();
                        if (!s.equals(":")) {
                            _scanDataLenSizes.add(Long.parseLong(s));
                        }
                    }
                    if (_scanDataLenSizes.size() < 2) {
                        System.err.print("'-scan <size1>:<size2>:...:<sizeN>' the number of size should be equal or greater then two.\n");
                        return false;
                    }
                    Collections.sort(_scanDataLenSizes);
                    if (_scanDataLenSizes.get(0) < OVERHEAD_BYTES) {
                        System.err.println("-scan sizes must be >= " +
                                OVERHEAD_BYTES);
                        return false;
                    }
                    if (_scanDataLenSizes.get(_scanDataLenSizes.size() - 1) >
                            MAX_PERFTEST_SAMPLE_SIZE.VALUE) {
                        System.err.println("-scan sizes must be <= " +
                                MAX_PERFTEST_SAMPLE_SIZE.VALUE);
                        return false;
                    }
                } else {
                    set_default_scan_values();
                }
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
            else if ("-keyed".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                // Do nothing, the keyed option has already been parsed, but we still
                // need to account it as a valid option.
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
            else if ("-verbosity".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                _messagingArgv[_messagingArgc++] = argv[i];

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <level> after -verbosity\n");
                    return false;
                }
                _messagingArgv[_messagingArgc++] = argv[i];
            }
            else if ("-writerStats".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                _displayWriterStats = true;
            }
            else if ( "-pubRate".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if (( i == (argc-1)) || argv[++i].startsWith("-") ) {
                    System.err.println("Missing <samples/s>:<method> after -pubRate");
                    return false;
                }
                if (argv[i].contains(":")) {
                    try {
                        _pubRate = Long.parseLong(argv[i].substring(0,argv[i].indexOf(":")));
                    } catch (NumberFormatException nfx) {
                        System.err.println("Bad pubRate rate");
                        return false;
                    }
                    // Validate pubRate <method> spin or sleep
                    if (argv[i].contains("spin".toLowerCase())){
                        System.err.println("-pubRate method: spin.");
                    } else if (argv[i].contains("sleep".toLowerCase())){
                        _pubRateMethodSpin = false;
                        System.err.println("-pubRate method: sleep.");
                    } else {
                        System.err.println("<samples/s>:<method> for pubRate '" + argv[i] + "' is not valid. It must contain 'spin' or 'sleep'.");
                        return false;
                    }
                } else {
                    try {
                        _pubRate = Long.parseLong(argv[i]);
                    } catch (NumberFormatException nfx) {
                        System.err.println("Bad pubRate rate");
                        return false;
                    }
                }

                if (_pubRate > 10000000) {
                    System.err.println("-pubRate cannot be greater than 10000000.");
                    return false;
                } else if (_pubRate < 0) {
                    System.err.println("-pubRate cannot be smaller than 0 (set 0 for unlimited).");
                    return false;
                }

            }
            else if ("-executionTime".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                {
                    System.err.print("Missing <seconds> after -executionTime\n");
                    return false;
                }
                try {
                    _executionTime = (long)Integer.parseInt(argv[i]);
                } catch (NumberFormatException nfx) {
                    System.err.print("Bad executionTime\n");
                    return false;
                }
            }
            else if ("-cpu".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                _showCpu = true;
            }
            else if ("-cft".toLowerCase().startsWith(argv[i].toLowerCase()))
            {
                _messagingArgv[_messagingArgc++] = argv[i];

                if ((i == (argc - 1)) || argv[++i].startsWith("-")) {
                    System.err.print("Missing <start>:<end> after -cft\n");
                    return false;
                }

                _messagingArgv[_messagingArgc++] = argv[i];

                _useCft = true;
            }
            else {
                _messagingArgv[_messagingArgc++] = argv[i];
            }
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
            
            /*
             * PERFTEST-108
             * If we are in a latency test, the default value for _NumIter has
             * to be smaller (to avoid certain issues in platforms with low
             * memory). Therefore, unless we explicitly changed the _NumIter
             * value we will use a smaller default: "numIterDefaultLatencyTest"
             */
            if (!numIterSet) {
                _numIter = numIterDefaultLatencyTest;
            }
        }

        if(_latencyCount == -1) {
            _latencyCount = 10000;
        }

        if (_numIter < _latencyCount) {
            System.err.printf(
                "numIter (%1$d) must be greater than latencyCount (%2$d).\n",
                _numIter,
                _latencyCount);
            return false;
        }

        //manage the parameter: -pubRate -sleep -spin
        if (_isPub && _pubRate >0) {
            if (_spinLoopCount > 0) {
                System.err.printf( "'-spin' is not compatible with -pubRate. " +
                    "Spin/Sleep value will be set by -pubRate.");
                _spinLoopCount = 0;
            }
            if (_sleepNanosec > 0) {
                System.err.printf("'-sleep' is not compatible with -pubRate. " +
                    "Spin/Sleep value will be set by -pubRate.");
                _sleepNanosec = 0;
            }
        }

        if (_isScan) {
            if (_dataLen != 100) { // Different that the default value
                System.err.printf("DataLen will be ignored since -scan is present.\n");
            }
            _dataLen = _scanDataLenSizes.get(_scanDataLenSizes.size() - 1); // Max size
            if (_executionTime == 0){
                System.err.printf("Setting timeout to 60 seconds (-scan).\n");
                _executionTime = 60;
            }
            // Check if large data or small data
            if (_scanDataLenSizes.get(0) < Math.min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)
                    && _scanDataLenSizes.get(_scanDataLenSizes.size() - 1) > Math.min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)) {
                System.err.printf("The sizes of -scan [");
                for (int i = 0; i < _scanDataLenSizes.size(); i++) {
                    System.err.printf(_scanDataLenSizes.get(i) + " ");
                }
                System.err.printf("] should be either all smaller or all bigger than " +
                        Math.min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE) + "\n");
                return false;
            }
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
            reader_listener = new ThroughputListener(writer, _useCft, _numPublishers);
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
            reader_listener = new ThroughputListener(writer, reader, _useCft, _numPublishers);

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
        boolean sent = announcement_writer.send(message, false);
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

            if (reader_listener.change_size) { // ACK change_size
                TestMessage message_change_size = new TestMessage();
                message_change_size.entity_id = subID;
                announcement_writer.send(message_change_size, false);
                announcement_writer.flush();
                reader_listener.change_size = false;
            }
            if (reader_listener.end_test) {
                TestMessage message_end_test = new TestMessage();
                message_end_test.entity_id = subID;
                announcement_writer.send(message_end_test, false);
                announcement_writer.flush();
                break;
            }
            String outputCpu = "";
            if (PerfTest._showCpu) {
                outputCpu = reader_listener.CpuMonitor.get_cpu_instant();
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
                            "Mbps: %4$7.1f  Mbps(ave): %5$7.1f  Lost: %6$d " + outputCpu + "\n",
                            last_msgs, mps, mps_ave,
                            bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                            reader_listener.missingPackets
                    );
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

        num_latency = (((int)_numIter/_samplesPerBatch) / _latencyCount);
        if ((num_latency/_samplesPerBatch) % _latencyCount > 0) {
            num_latency++;
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

        long spinPerUsec = 0;
        long sleepUsec = 1000;
        if (_pubRate > 0) {
            if ( _pubRateMethodSpin) {
                spinPerUsec = Utility.get_spin_per_microsecond();
                /* A return value of 0 means accuracy not assured */
                if (spinPerUsec == 0) {
                    System.err.println("Error initializing spin per microsecond. "
                        + "-pubRate cannot be used\n"
                            +"Exiting...");
                    return;
                }
                _spinLoopCount = 1000000*spinPerUsec/_pubRate;
            } else {
                _sleepNanosec =1000000000/_pubRate;
            }
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
        message.data = new byte[Math.max((int)_dataLen,LENGTH_CHANGED_SIZE)];

        System.err.print("Publishing data...\n");

        // initialize data pathways by sending some initial pings
        if (initialize_sample_count < _instanceCount) {
            initialize_sample_count = _instanceCount;
        }
        message.size = INITIALIZE_SIZE;
        for (int i = 0; i < initialize_sample_count; i++)
        {
            // Send test initialization message
            if (!writer.send(message, true)) {
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
        message.size = (int)_dataLen - OVERHEAD_BYTES;

        // Sleep 1 second, then begin test
        sleep(1000);

        int num_pings = 0;
        int scan_count = 0;
        int pingID = -1;
        int current_index_in_batch = 0;
        int ping_index_in_batch = 0;
        boolean sentPing = false;

        long time_now = 0, time_last_check = 0, time_delta = 0;
        long pubRate_sample_period = 1;
        long rate = 0;

        time_last_check = getTimeUsec();

        /* Minimum value for pubRate_sample_period will be 1 so we execute 100 times
           the control loop every second, or every sample if we want to send less
           than 100 samples per second */
        if (_pubRate > 100) {
            pubRate_sample_period = _pubRate / 100;
        }

        if (_executionTime > 0 && !_isScan) {
          timertask.setTimeout(_executionTime, _isScan);
        }

        /********************
         *  Main sending loop
         */
        for (long loop=0; ((_isScan) || (loop < _numIter)) &&
                           (!testCompleted) ; ++loop ) {

            /* This if has been included to perform the control loop
               that modifies the publication rate according to -pubRate */
            if ((_pubRate > 0) &&
                    (loop > 0) &&
                    (loop % pubRate_sample_period == 0)) {

                time_now = getTimeUsec();

                time_delta = time_now - time_last_check;
                time_last_check = time_now;
                rate = (pubRate_sample_period*1000000)/time_delta;
                if ( _pubRateMethodSpin) {
                    if (rate > _pubRate) {
                        _spinLoopCount += spinPerUsec;
                    } else if (rate < _pubRate && _spinLoopCount > spinPerUsec) {
                        _spinLoopCount -= spinPerUsec;
                    } else if (rate < _pubRate && _spinLoopCount <= spinPerUsec) {
                        _spinLoopCount = 0;
                    }
                } else { //sleep
                    if (rate > _pubRate) {
                        _sleepNanosec += sleepUsec; //plus 1 MicroSec
                    } else if (rate < _pubRate && _sleepNanosec > sleepUsec) {
                        _sleepNanosec -=  sleepUsec; //less 1 MicroSec
                    } else if (rate < _pubRate && _sleepNanosec <= sleepUsec) {
                       _sleepNanosec = 0;
                    }
                }
            }

            if ( _spinLoopCount > 0 ) {
                Utility.spin(_spinLoopCount);
            }

            if ( _sleepNanosec > 0 ) {
                try {
                    TimeUnit.NANOSECONDS.sleep(_sleepNanosec);
                } catch (InterruptedException interrupted) {
                    System.err.println("Sleep NanoSec does not work.");
                    return;
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
                    // after _executionTime
                    if (_isScan && testCompletedScan) {
                        testCompletedScan = false;
                        timertask = new PerftestTimerTask(this);
                        timertask.setTimeout(_executionTime, _isScan);

                        // flush anything that was previously sent
                        writer.flush();
                        writer.wait_for_acknowledgments(timeout_wait_for_ack);

                        announcement_reader_listener.announced_subscribers =
                                _numSubscribers;

                        if (scan_count == _scanDataLenSizes.size()) {
                            break; // End of scan test
                        }

                        message.size = LENGTH_CHANGED_SIZE;
                        // must set latency_ping so that a subscriber sends us
                        // back the LENGTH_CHANGED_SIZE message
                        message.latency_ping = num_pings % _numSubscribers;
                        int i = 0;
                        while (announcement_reader_listener.announced_subscribers > 0) {
                            writer.send(message, true);
                            writer.wait_for_acknowledgments(timeout_wait_for_ack);
                        }

                        message.size = (int)(_scanDataLenSizes.get(scan_count++) - OVERHEAD_BYTES);
                        /* Reset _SamplePerBatch */
                        if (_batchSize != 0) {
                            _samplesPerBatch = _batchSize / (message.size + OVERHEAD_BYTES);
                            if (_samplesPerBatch == 0) {
                                _samplesPerBatch = 1;
                            }
                        } else {
                            _samplesPerBatch = 1;
                        }
                        ping_index_in_batch = 0;
                        current_index_in_batch = 0;
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

                    if (_displayWriterStats && printIntervals) {
                        System.out.printf(
                                "Pulled samples: %1$7d\n",
                                writer.getPulledSampleCount());
                    }
                }
            }

            current_index_in_batch = (current_index_in_batch + 1) % _samplesPerBatch;

            message.seq_num = (int)loop;
            message.latency_ping = pingID;
            if (!writer.send(message, false)) {
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
        int i = 0;
        announcement_reader_listener.announced_subscribers = _numSubscribers;
        while (announcement_reader_listener.announced_subscribers > 0
                && i < initialize_sample_count) {
            writer.send(message, true);
            i++;
            writer.wait_for_acknowledgments(timeout_wait_for_ack);
        }
        reader_listener.print_summary_latency();
        reader_listener.end_test = true;

        if (_useReadThread) {
            assert reader != null;
            reader.shutdown();
        }

        if (_displayWriterStats) {
            System.out.printf(
                    "Pulled samples: %1$7d\n",
                    writer.getPulledSampleCount());
        }

        if (testCompleted) {
            System.err.println("Finishing test due to timer...");
        } else {
            System.err.println("Finishing test...");
        }
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

