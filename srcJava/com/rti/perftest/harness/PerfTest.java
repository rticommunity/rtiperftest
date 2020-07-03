/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest.harness;
import com.rti.perftest.gen.THROUGHPUT_TOPIC_NAME;
import com.rti.perftest.gen.LATENCY_TOPIC_NAME;
import com.rti.perftest.gen.ANNOUNCEMENT_TOPIC_NAME;

import com.rti.perftest.IMessaging;
import com.rti.perftest.IMessagingReader;
import com.rti.perftest.IMessagingWriter;
import com.rti.perftest.TestMessage;
import com.rti.perftest.harness.PerftestTimerTask;
import com.rti.perftest.gen.MAX_BOUNDED_SEQ_SIZE;
import com.rti.perftest.gen.MAX_PERFTEST_SAMPLE_SIZE;

import com.rti.perftest.gen.THROUGHPUT_TOPIC_NAME;
import com.rti.perftest.gen.LATENCY_TOPIC_NAME;
import com.rti.perftest.gen.ANNOUNCEMENT_TOPIC_NAME;

import com.rti.perftest.ddsimpl.PerftestVersion;
import com.rti.ndds.Utility;
import com.rti.dds.infrastructure.ProductVersion_t;
import com.rti.ndds.config.Version;

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

    public static final int timeout_wait_for_ack_sec = 0;
    public static final int timeout_wait_for_ack_nsec = 10000000;

    // Number of bytes sent in messages besides user data
    public static int OVERHEAD_BYTES = 28;

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

    // Value used to compare against to check if the latency_min has
    // been reset.
    public static final int LATENCY_RESET_VALUE = Integer.MAX_VALUE;

    /*package*/ static int subID = 0;

    /*package*/ static int pubID = 0;

    /*package*/ static boolean printIntervals = true;

    /*package*/ static boolean _showCpu = false;

    static boolean _printHeaders = true;
    static String _outputFormat = "csv";
    public static PerftestPrinter _printer = null;

    // -----------------------------------------------------------------------
    // Private Fields
    // -----------------------------------------------------------------------

    //private static boolean _isDebug = false;

    private long     _dataLen = 100;
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
    private boolean  _isKeyed = false;
    private boolean  _pubRateMethodSpin = true;
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

    public int getSamplesPerBatch(){
        int batchSize = _messagingImpl.getBatchSize();
        int samplesPerBatch;

        if (batchSize > 0) {
            samplesPerBatch = batchSize / (int) _dataLen;
            if (samplesPerBatch == 0) {
                samplesPerBatch = 1;
            }
        } else {
            samplesPerBatch = 1;
        }

        return samplesPerBatch;
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

        printVersion();

        _messagingImpl = messagingImpl;

        if ( !parseConfig(argv) ) {
            return;
        }

        if ( !_messagingImpl.initialize(_messagingArgc, _messagingArgv) ) {
            return;
        }

        if ("json".equals(_outputFormat)) {
            _printer = new PerftestJSONPrinter();
        } else if ("legacy".equals(_outputFormat)) {
            _printer = new PerftestLegacyPrinter();
        } else {
            _printer = new PerftestCSVPrinter();
        }

        _printer.initialize(
                    printIntervals,
                    _printHeaders,
                    _showCpu);

        printConfiguration();

        if (_isPub) {
            publisher();
        } else {
            subscriber();
        }
    }

    private ProductVersion_t getDDSVersion() {
        return Version.get_instance().get_product_version();
    }

    private ProductVersion_t getPerftestVersion() {
        return PerftestVersion.getInstance().getProductVersion();
    }

    private void printVersion() {
        ProductVersion_t perftestV = getPerftestVersion();
        ProductVersion_t ddsV = getDDSVersion();

        StringBuffer perftestVString = new StringBuffer(128);

        if ((int)perftestV.major == 9
                && (int)perftestV.minor == 9
                && (int)perftestV.release == 9) {
            perftestVString.append("Master");
        } else {
            perftestVString.append((int)perftestV.major).append(".");
            perftestVString.append((int)perftestV.minor).append(".");
            perftestVString.append((int)perftestV.release);

            if( perftestV.revision != 0 ) {
                perftestVString.append(".").append((int) perftestV.revision);
            }
        }

        StringBuffer ddsVString = new StringBuffer(128);
        ddsVString.append((int)ddsV.major).append(".");
        ddsVString.append((int)ddsV.minor).append(".");
        ddsVString.append((int)ddsV.release);


        System.out.print(
                "RTI Perftest "
                + perftestVString.toString()
                + " (RTI Connext DDS "
                + ddsVString.toString()
                + ")\n");
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
            "\t                                    [32,63000] or [63001,2147482620] bytes,\n" +
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
            "\t                          Default: Not set\n" +
            "\t-noOutputHeaders         - Skip displaying the header row with\n" +
            "\t                          the titles of the tables and the summary.\n" +
            "\t                          Default: false (it will display titles)\n" +
            "\t-outputFormat <format>  - Set the output format.\n" +
            "\t                          The following formats are available:\n" +
            "\t                           - 'csv'\n" +
            "\t                           - 'json'\n" +
            "\t                           - 'legacy'\n" +
            "\t                          Default: 'csv'\n";

        int argc = argv.length;
        if (argc < 0) {
            System.err.print(usage_string);
            _messagingImpl.printCmdLineHelp();
            return false;
        }

        /*
         * PERFTEST-108
         * We add this boolean value to check if we are explicity changing the
         * number of iterations via command-line paramenter. This will only be
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
            } else if ( "-spin".toLowerCase().startsWith(argv[i].toLowerCase()))
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
                _isKeyed = true;
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
                    if (argv[i].contains("sleep".toLowerCase())){
                        _pubRateMethodSpin = false;
                    } else if (!argv[i].contains("spin".toLowerCase())) {
                        System.err.println("<samples/s>:<method> for pubRate '"
                                + argv[i]
                                + "' is not valid. It must contain 'spin' or 'sleep'.");
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
                    System.err.print(
                            "-executionTime value must be a positive number greater than 0\n");
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
            else if ("-noOutputHeaders".toLowerCase().startsWith(argv[i].toLowerCase()))
                {
                    _printHeaders = false;
                }
                else if ("-outputFormat".toLowerCase().startsWith(argv[i].toLowerCase()))
                {
                    if ((i == (argc - 1)) || argv[++i].startsWith("-"))
                    {
                        System.err.print("Missing <format> after " +
                                "-outputFormat\n");
                        return false;
                    }
                        if ("csv".equals(argv[i])) {
                            _outputFormat = "csv";
                        } else if ("json".equals(argv[i])) {
                            _outputFormat = "json";
                        } else if ("legacy".equals(argv[i])) {
                            _outputFormat = "legacy";
                        } else {
                            System.err.print("<format> for outputFormat '" +
                                    argv[i] + "' is not valid. It must be" +
                                    "'csv', 'json' or 'legacy'.\n");
                            return false;
                        }
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
            _dataLen = _scanDataLenSizes.get(_scanDataLenSizes.size() - 1); // Max size
            if (_executionTime == 0){
                _executionTime = 60;
            }
            // Check if large data or small data
            if (_scanDataLenSizes.get(0) < MAX_BOUNDED_SEQ_SIZE.VALUE
                    && _scanDataLenSizes.get(_scanDataLenSizes.size() - 1) > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                System.err.printf("The sizes of -scan [");
                for (int i = 0; i < _scanDataLenSizes.size(); i++) {
                    System.err.printf(_scanDataLenSizes.get(i) + " ");
                }
                System.err.printf("] should be either all smaller or all bigger than " +
                        MAX_BOUNDED_SEQ_SIZE.VALUE + "\n");
                return false;
            }
        }

        return true;
    }

    private void printConfiguration() {

        StringBuilder sb = new StringBuilder();

        // Throughput/Latency mode
        if (_isPub) {
            sb.append("\nMode: ");

            if (_latencyTest) {
                sb.append("LATENCY TEST (Ping-Pong test)\n");
            } else {
                sb.append("THROUGHPUT TEST\n");
                sb.append("      (Use \"-latencyTest\" for Latency Mode)\n");
            }
        }

        sb.append("\nPerftest Configuration:\n");

        // Reliable/Best Effort
        sb.append("\tReliability: ");
        if (_isReliable) {
            sb.append("Reliable\n");
        } else {
            sb.append("Best Effort\n");
        }

        // Keyed/Unkeyed
        sb.append("\tKeyed: ");
        if (_isKeyed) {
            sb.append("Yes\n");
        } else {
            sb.append("No\n");
        }

        // Publisher/Subscriber and Entity ID
        if (_isPub) {
            sb.append("\tPublisher ID: ");
            sb.append(pubID);
            sb.append("\n");
        } else {
            sb.append("\tSubscriber ID: ");
            sb.append(subID);
            sb.append("\n");
        }

        if (_isPub) {

            sb.append("\tLatency count: 1 latency sample every ");
            sb.append(_latencyCount);
            sb.append("\n");

            // Scan/Data Sizes
            sb.append("\tData Size: ");
            if (_isScan) {
                for (int i = 0; i < _scanDataLenSizes.size(); i++ ) {
                    sb.append(_scanDataLenSizes.get(i));
                    if (i == _scanDataLenSizes.size() - 1) {
                        sb.append("\n");
                    } else {
                        sb.append(", ");
                    }
                }
            } else {
                sb.append(_dataLen);
                sb.append("\n");
            }

            // Batching
            int batchSize = _messagingImpl.getBatchSize();

            sb.append("\tBatching: ");
            if (batchSize > 0) {
                sb.append(batchSize);
                sb.append(" Bytes (Use \"-batchSize 0\" to disable batching)\n");
            } else if (batchSize == 0) {
                sb.append("No (Use \"-batchSize\" to setup batching)\n");
            } else { // < 0 (Meaning, Disabled by RTI Perftest)
                sb.append("Disabled by RTI Perftest.\n");
                if (batchSize == -1) {
                    if (_latencyTest) {
                        sb.append("\t\t  BatchSize disabled for a Latency Test\n");
                    } else {
                        sb.append("\t\t  BatchSize is smaller than 2 times\n");
                        sb.append("\t\t  the minimum sample size.\n");
                    }
                }
                if (batchSize == -2) {
                    sb.append("\t\t  BatchSize cannot be used with\n");
                    sb.append("\t\t  Large Data.\n");
                }
            }

            // Publication Rate
            sb.append("\tPublication Rate: ");
            if (_pubRate > 0) {
                sb.append(_pubRate);
                sb.append(" Samples/s (");
                if (_pubRateMethodSpin) {
                    sb.append("Spin)\n");
                } else {
                    sb.append("Sleep)\n");
                }
            } else {
                sb.append("Unlimited (Not set)\n");
            }

            // Execution Time or Num Iter
            if (_executionTime > 0) {
                sb.append("\tExecution time: ");
                sb.append(_executionTime);
                sb.append(" seconds\n");
            } else {
                sb.append("\tNumber of samples: " );
                sb.append(_numIter);
                sb.append("\n");
            }
        }

        // Listener/WaitSets
        sb.append("\tReceive using: ");
        if (_useReadThread) {
            sb.append("WaitSets\n");
        } else {
            sb.append("Listeners\n");
        }

        sb.append(_messagingImpl.printConfiguration());
        System.err.println(sb.toString());
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

        writer = _messagingImpl.createWriter(LATENCY_TOPIC_NAME.VALUE);
        if (writer == null) {
            System.err.print("Problem creating latency writer.\n");
            return;
        }

        // Check if using callbacks or read thread
        if (!_useReadThread) {
            // create latency pong reader
            reader_listener = new ThroughputListener(writer, _useCft, _numPublishers);
            reader = _messagingImpl.createReader(THROUGHPUT_TOPIC_NAME.VALUE, reader_listener);
            if (reader == null) {
                System.err.print("Problem creating throughput reader.\n");
                return;
            }
        } else {
            reader = _messagingImpl.createReader(THROUGHPUT_TOPIC_NAME.VALUE, null);
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
        announcement_writer = _messagingImpl.createWriter(ANNOUNCEMENT_TOPIC_NAME.VALUE);

        if (announcement_writer == null) {
             System.err.print("Problem creating announcement writer.\n");
            return;
        }

        // Synchronize with publishers
        System.err.printf("Waiting to discover %1$d publishers ...\n", _numPublishers);
        reader.waitForWriters(_numPublishers);
        // In a multi publisher test, only the first publisher will have a reader.
        writer.waitForReaders(1);
        announcement_writer.waitForReaders(_numPublishers);

        // Announcement message that will be used by the announcement_writer
        // to send information to the Publisher. This message size will indicate
        // different things.

        TestMessage announcement_msg = new TestMessage();
        announcement_msg.entity_id = subID;
        announcement_msg.data = new byte[LENGTH_CHANGED_SIZE];
        announcement_msg.size = INITIALIZE_SIZE;

        // Send announcement message
        boolean sent = announcement_writer.send(announcement_msg, false);
        announcement_writer.flush();
        if (!sent) {
            System.err.println("*** send() failure: announcement message");
            return;
        }

        System.err.print("Waiting for data ...\n");

        _printer.print_initial_output();

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
                announcement_msg.entity_id = subID;
                announcement_msg.size = LENGTH_CHANGED_SIZE;
                announcement_writer.send(announcement_msg, false);
                announcement_writer.flush();
                reader_listener.change_size = false;
            }
            if (reader_listener.end_test) {
                announcement_msg.entity_id = subID;
                announcement_msg.size = FINISHED_SIZE;
                announcement_writer.send(announcement_msg, false);
                announcement_writer.flush();
                break;
            }
            double outputCpu = 0.0;
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

                // Calculations of missing package percent
                if (last_msgs + reader_listener.missingPackets == 0) {
                    reader_listener.missingPacketsPercent = 0.0;
                } else {
                    reader_listener.missingPacketsPercent =
                            (reader_listener.missingPackets * 100.0)
                            / (float) (last_msgs + reader_listener.missingPackets);
                }

                if (last_msgs > 0) {
                    _printer.print_throughput_interval(
                            last_msgs,
                            mps,
                            mps_ave,
                            bps,
                            bps_ave,
                            reader_listener.missingPackets,
                            reader_listener.missingPacketsPercent,
                            outputCpu);
                }
            }
        }

        if (_useReadThread) {
            reader.shutdown();
        }
        _printer.print_final_output();
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
        int announcement_sample_count = 50;
        int samplesPerBatch = 1;

        // create throughput/ping writer
        writer = _messagingImpl.createWriter(THROUGHPUT_TOPIC_NAME.VALUE);

        if (writer == null) {
            System.err.print("Problem creating throughput writer.\n");
            return;
        }

        samplesPerBatch = getSamplesPerBatch();

        num_latency = (((int)_numIter/samplesPerBatch) / _latencyCount);
        if ((num_latency/samplesPerBatch) % _latencyCount > 0) {
            num_latency++;
        }

        // in batch mode, might have to send another ping
        if (samplesPerBatch > 1) {
          ++num_latency;
        }

        // Only publisher with ID 0 will send/receive pings
        if (pubID == 0) {
            // Check if using callbacks or read thread
            if (!_useReadThread) {
                // create latency pong reader
                reader_listener = new LatencyListener(num_latency,_latencyTest?writer:null);
                reader = _messagingImpl.createReader(LATENCY_TOPIC_NAME.VALUE, reader_listener);
                if (reader == null)
                {
                    System.err.print("Problem creating latency reader.\n");
                    return;
                }
            }
            else
            {
                reader = _messagingImpl.createReader(LATENCY_TOPIC_NAME.VALUE, null);
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
        announcement_reader = _messagingImpl.createReader(ANNOUNCEMENT_TOPIC_NAME.VALUE,
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

        System.err.printf("Waiting to discover %1$d subscribers ...\n", _numSubscribers);
        writer.waitForReaders(_numSubscribers);
        // Only publisher with ID 0 will have a reader.
        if (reader != null) {
            reader.waitForWriters(_numSubscribers);
        }
        announcement_reader.waitForWriters(_numSubscribers);

        // We have to wait until every Subscriber sends an announcement message
        // indicating that it has discovered every Publisher
        System.err.print("Waiting for subscribers announcement ...\n");
        while (_numSubscribers
                > announcement_reader_listener.subscriber_list.size()) {
            sleep(1000);
        }

        // Allocate data and set size
        TestMessage message = new TestMessage();
        message.entity_id = pubID;
        message.data = new byte[Math.max((int)_dataLen,LENGTH_CHANGED_SIZE)];

        message.size = INITIALIZE_SIZE;

        /*
         * Initial burst of data:
         *
         * The purpose of this initial burst of Data is to ensure that most
         * memory allocations in the critical path are done before the test
         * begings, for both the Writer and the Reader that receives the samples.
         * It will also serve to make sure that all the instances are registered
         * in advance in the subscriber application.
         *
         * We query the MessagingImplementation class to get the suggested sample
         * count that we should send. This number might be based on the reliability
         * protocol implemented by the middleware behind. Then we choose between that
         * number and the number of instances to be sent.
         */

         int initializeSampleCount = Math.max(
                _messagingImpl.getInitializationSampleCount(),
                _instanceCount);

        System.err.println(
                "Sending " + initializeSampleCount + " initialization pings ...");

        for (int i = 0; i < initializeSampleCount; i++) {
            // Send test initialization message
            if (!writer.send(message, true)) {
                System.err.println(
                        "*** send() failure: initialization message");
                return;
            }
        }
        writer.flush();

        System.err.print("Publishing data ...\n");
        
        _printer.print_initial_output();

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
            if ( (pubID == 0) && (((loop/samplesPerBatch) %_latencyCount) == 0) )
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
                        writer.waitForAck(
                            timeout_wait_for_ack_sec,
                            timeout_wait_for_ack_nsec);

                        if (scan_count == _scanDataLenSizes.size()) {
                            break; // End of scan test
                        }

                        message.size = LENGTH_CHANGED_SIZE;
                        // must set latency_ping so that a subscriber sends us
                        // back the LENGTH_CHANGED_SIZE message
                        message.latency_ping = num_pings % _numSubscribers;

                        /*
                         * If the Throughput topic is reliable, we can send the packet and do
                         * a wait for acknowledgements. However, if the Throughput topic is
                         * Best Effort, waitForAck() will return inmediately.
                         * This would cause that the Send() would be exercised too many times,
                         * in some cases causing the network to be flooded, a lot of packets being
                         * lost, and potentially CPU starbation for other processes.
                         * We can prevent this by adding a small sleep() if the test is best
                         * effort.
                         */

                        announcement_reader_listener.subscriber_list.clear();
                        while (announcement_reader_listener.subscriber_list.size()
                                < _numSubscribers) {
                            writer.send(message, true);
                            writer.waitForAck(
                                timeout_wait_for_ack_sec,
                                timeout_wait_for_ack_nsec);
                        }

                        message.size = (int)(_scanDataLenSizes.get(scan_count++) - OVERHEAD_BYTES);
                        /* Reset _SamplePerBatch */
                        samplesPerBatch = getSamplesPerBatch();
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
                    ping_index_in_batch = (ping_index_in_batch + 1) % samplesPerBatch;
                    sentPing = true;

                    if (_displayWriterStats && printIntervals) {
                        System.out.printf(
                                "Pulled samples: %1$7d\n",
                                writer.getPulledSampleCount());
                    }
                }
            }

            current_index_in_batch = (current_index_in_batch + 1) % samplesPerBatch;

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
        while (announcement_reader_listener.subscriber_list.size() > 0
                && i < announcement_sample_count) {
            writer.send(message, true);
            i++;
            writer.waitForAck(
                timeout_wait_for_ack_sec,
                timeout_wait_for_ack_nsec);
        }

        if (pubID == 0) {
            reader_listener.print_summary_latency(true);
            reader_listener.end_test = true;
        } else {
            System.out.println("Latency results are only shown when -pidMultiPubTest = 0");
        }

        if (_useReadThread) {
            assert reader != null;
            reader.shutdown();
        }

        if (_displayWriterStats) {
            System.out.printf(
                    "Pulled samples: %1$7d\n",
                    writer.getPulledSampleCount());
        }
        _printer.print_final_output();
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

