/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices; // for DllImport]
using System.Threading;
using System.Text;
using System.Timers;
using DDS;

namespace PerformanceTest {

    /*********************************************************
     * DataTypeHelper (non keyed)
     */
    class DataTypeHelper : ITypeHelper<TestData_t>
    {

        TestData_t _myData;
        ulong _maxPerftestSampleSize = perftest_cs.getMaxPerftestSampleSizeCS();

        public DataTypeHelper(ulong maxPerftestSampleSize)
        {
            _myData = new TestData_t();
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DataTypeHelper(TestData_t myData, ulong maxPerftestSampleSize)
        {
                _myData = myData;
                _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public void fillKey(int value)
        {

            _myData.key[0] = (byte)(value);
            _myData.key[1] = (byte)(value >> 8);
            _myData.key[2] = (byte)(value >> 16);
            _myData.key[3] = (byte)(value >> 24);

        }

        public void copyFromMessage(TestMessage message)
        {

            _myData.entity_id = message.entity_id;
            _myData.seq_num = message.seq_num;
            _myData.timestamp_sec = message.timestamp_sec;
            _myData.timestamp_usec = message.timestamp_usec;
            _myData.latency_ping = message.latency_ping;
            _myData.bin_data.loan(message.data, message.size);

        }

        public TestMessage copyFromSeqToMessage(LoanableSequence<TestData_t> data_sequence, int index)
        {

            TestData_t msg = ((TestData_tSeq)data_sequence).get_at(index);
            TestMessage message = new TestMessage();

            message.entity_id = msg.entity_id;
            message.seq_num = msg.seq_num;
            message.timestamp_sec = msg.timestamp_sec;
            message.timestamp_usec = msg.timestamp_usec;
            message.latency_ping = msg.latency_ping;
            message.size = msg.bin_data.length;
            message.data = msg.bin_data.buffer;

            return message;
        }

        public TestData_t getData()
        {
            return _myData;
        }

        public void setBinDataMax(int newMax)
        {
            _myData.bin_data.maximum = newMax;
        }

        public void binDataUnloan()
        {
            _myData.bin_data.unloan();
        }

        public DDS.AbstractTypedTypeSupport<TestData_t> getTypeSupport()
        {
            return TestData_tTypeSupport.get_instance();
        }

        public ITypeHelper<TestData_t> clone()
        {
            return new DataTypeHelper(_myData, _maxPerftestSampleSize);
        }

        public DDS.LoanableSequence<TestData_t> createSequence()
        {
            return new TestData_tSeq();
        }

        public ulong getMaxPerftestSampleSize() {
            return _maxPerftestSampleSize;
        }

    }

    /*********************************************************
     * DataTypeLargeHelper (non keyed)
     */
    class DataTypeLargeHelper : ITypeHelper<TestDataLarge_t>
    {

        TestDataLarge_t _myData;
        ulong _maxPerftestSampleSize = perftest_cs.getMaxPerftestSampleSizeCS();

        public DataTypeLargeHelper(ulong maxPerftestSampleSize)
        {
            _myData = new TestDataLarge_t();
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DataTypeLargeHelper(TestDataLarge_t myData, ulong maxPerftestSampleSize)
        {
                _myData = myData;
                _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public void fillKey(int value)
        {

            _myData.key[0] = (byte)(value);
            _myData.key[1] = (byte)(value >> 8);
            _myData.key[2] = (byte)(value >> 16);
            _myData.key[3] = (byte)(value >> 24);

        }

        public void copyFromMessage(TestMessage message)
        {

            _myData.entity_id = message.entity_id;
            _myData.seq_num = message.seq_num;
            _myData.timestamp_sec = message.timestamp_sec;
            _myData.timestamp_usec = message.timestamp_usec;
            _myData.latency_ping = message.latency_ping;
            _myData.bin_data.loan(message.data, message.size);

        }

        public TestMessage copyFromSeqToMessage(LoanableSequence<TestDataLarge_t> data_sequence, int index)
        {

            TestDataLarge_t msg = ((TestDataLarge_tSeq)data_sequence).get_at(index);
            TestMessage message = new TestMessage();

            message.entity_id = msg.entity_id;
            message.seq_num = msg.seq_num;
            message.timestamp_sec = msg.timestamp_sec;
            message.timestamp_usec = msg.timestamp_usec;
            message.latency_ping = msg.latency_ping;
            message.size = msg.bin_data.length;
            message.data = msg.bin_data.buffer;

            return message;
        }

        public TestDataLarge_t getData()
        {
            return _myData;
        }

        public void setBinDataMax(int newMax)
        {
            _myData.bin_data.maximum = newMax;
        }

        public void binDataUnloan()
        {
            _myData.bin_data.unloan();
        }

        public DDS.AbstractTypedTypeSupport<TestDataLarge_t> getTypeSupport()
        {
            return TestDataLarge_tTypeSupport.get_instance();
        }

        public ITypeHelper<TestDataLarge_t> clone()
        {
            return new DataTypeLargeHelper(_myData, _maxPerftestSampleSize);
        }

        public DDS.LoanableSequence<TestDataLarge_t> createSequence()
        {
            return new TestDataLarge_tSeq();
        }

        public ulong getMaxPerftestSampleSize() {
            return _maxPerftestSampleSize;
        }

    }


    /*********************************************************
     * DataTypeKeyedHelper
     */
    class DataTypeKeyedHelper : ITypeHelper<TestDataKeyed_t>
    {

        TestDataKeyed_t _myData;
        ulong _maxPerftestSampleSize = perftest_cs.getMaxPerftestSampleSizeCS();

        public DataTypeKeyedHelper(ulong maxPerftestSampleSize)
        {
            _myData = new TestDataKeyed_t();
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DataTypeKeyedHelper(TestDataKeyed_t myData, ulong maxPerftestSampleSize)
        {
            _myData = myData;
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public void fillKey(int value)
        {

            _myData.key[0] = (byte)(value);
            _myData.key[1] = (byte)(value >> 8);
            _myData.key[2] = (byte)(value >> 16);
            _myData.key[3] = (byte)(value >> 24);

        }

        public void copyFromMessage(TestMessage message)
        {

            _myData.entity_id = message.entity_id;
            _myData.seq_num = message.seq_num;
            _myData.timestamp_sec = message.timestamp_sec;
            _myData.timestamp_usec = message.timestamp_usec;
            _myData.latency_ping = message.latency_ping;
            _myData.bin_data.loan(message.data, message.size);

        }

        public TestMessage copyFromSeqToMessage(LoanableSequence<TestDataKeyed_t> data_sequence, int index)
        {

            TestDataKeyed_t msg = ((TestDataKeyed_tSeq)data_sequence).get_at(index);
            TestMessage message = new TestMessage();

            message.entity_id = msg.entity_id;
            message.seq_num = msg.seq_num;
            message.timestamp_sec = msg.timestamp_sec;
            message.timestamp_usec = msg.timestamp_usec;
            message.latency_ping = msg.latency_ping;
            message.size = msg.bin_data.length;
            message.data = msg.bin_data.buffer;

            return message;
        }

        public TestDataKeyed_t getData()
        {
            return _myData;
        }

        public void setBinDataMax(int newMax)
        {
            _myData.bin_data.maximum = newMax;
        }

        public void binDataUnloan()
        {
            _myData.bin_data.unloan();
        }

        public DDS.AbstractTypedTypeSupport<TestDataKeyed_t> getTypeSupport()
        {
            return TestDataKeyed_tTypeSupport.get_instance();
        }

        public ITypeHelper<TestDataKeyed_t> clone()
        {
            return new DataTypeKeyedHelper(_myData, _maxPerftestSampleSize);
        }

        public DDS.LoanableSequence<TestDataKeyed_t> createSequence()
        {
            return new TestDataKeyed_tSeq();
        }

        public ulong getMaxPerftestSampleSize() {
            return _maxPerftestSampleSize;
        }

    }

    /*********************************************************
     * DataTypeKeyedLargeHelper
     */
    class DataTypeKeyedLargeHelper : ITypeHelper<TestDataKeyedLarge_t>
    {

        TestDataKeyedLarge_t _myData;
        ulong _maxPerftestSampleSize = perftest_cs.getMaxPerftestSampleSizeCS();

        public DataTypeKeyedLargeHelper(ulong maxPerftestSampleSize)
        {
            _myData = new TestDataKeyedLarge_t();
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DataTypeKeyedLargeHelper(TestDataKeyedLarge_t myData, ulong maxPerftestSampleSize)
        {
            _myData = myData;
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public void fillKey(int value)
        {

            _myData.key[0] = (byte)(value);
            _myData.key[1] = (byte)(value >> 8);
            _myData.key[2] = (byte)(value >> 16);
            _myData.key[3] = (byte)(value >> 24);

        }

        public void copyFromMessage(TestMessage message)
        {

            _myData.entity_id = message.entity_id;
            _myData.seq_num = message.seq_num;
            _myData.timestamp_sec = message.timestamp_sec;
            _myData.timestamp_usec = message.timestamp_usec;
            _myData.latency_ping = message.latency_ping;
            _myData.bin_data.loan(message.data, message.size);

        }

        public TestMessage copyFromSeqToMessage(LoanableSequence<TestDataKeyedLarge_t> data_sequence, int index)
        {

            TestDataKeyedLarge_t msg = ((TestDataKeyedLarge_tSeq)data_sequence).get_at(index);
            TestMessage message = new TestMessage();

            message.entity_id = msg.entity_id;
            message.seq_num = msg.seq_num;
            message.timestamp_sec = msg.timestamp_sec;
            message.timestamp_usec = msg.timestamp_usec;
            message.latency_ping = msg.latency_ping;
            message.size = msg.bin_data.length;
            message.data = msg.bin_data.buffer;

            return message;
        }

        public TestDataKeyedLarge_t getData()
        {
            return _myData;
        }

        public void setBinDataMax(int newMax)
        {
            _myData.bin_data.maximum = newMax;
        }

        public void binDataUnloan()
        {
            _myData.bin_data.unloan();
        }

        public DDS.AbstractTypedTypeSupport<TestDataKeyedLarge_t> getTypeSupport()
        {
            return TestDataKeyedLarge_tTypeSupport.get_instance();
        }

        public ITypeHelper<TestDataKeyedLarge_t> clone()
        {
            return new DataTypeKeyedLargeHelper(_myData, _maxPerftestSampleSize);
        }

        public DDS.LoanableSequence<TestDataKeyedLarge_t> createSequence()
        {
            return new TestDataKeyedLarge_tSeq();
        }

        public ulong getMaxPerftestSampleSize() {
            return _maxPerftestSampleSize;
        }

    }

    class DynamicDataTypeHelper : ITypeHelper<DynamicData>
    {

        bool _isKeyed;
        DynamicData _myData;
        ulong _maxPerftestSampleSize = perftest_cs.getMaxPerftestSampleSizeCS();

        public DynamicDataTypeHelper(DDS.TypeCode typeCode, bool isKeyed, ulong maxPerftestSampleSize)
        {
            _isKeyed = isKeyed;
            _myData = new DynamicData(typeCode, DynamicData.DYNAMIC_DATA_PROPERTY_DEFAULT);
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DynamicDataTypeHelper(DynamicData myData, ulong maxPerftestSampleSize)
        {
            _myData = myData;
            _maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public void fillKey(int value)
        {
            Byte[] byteArray = new Byte[4];
            byteArray[0] = (byte)(value);
            byteArray[1] = (byte)(value >> 8);
            byteArray[2] = (byte)(value >> 16);
            byteArray[3] = (byte)(value >> 24);
            _myData.set_byte_array("key", 1, byteArray);
        }

        public void copyFromMessage(TestMessage message)
        {

            ByteSeq my_byteSeq = new ByteSeq();
            my_byteSeq.ensure_length(message.size, message.size);

            _myData.set_int("entity_id", 2, message.entity_id);
            _myData.set_uint("seq_num", 3, message.seq_num);
            _myData.set_int("timestamp_sec", 4, message.timestamp_sec);
            _myData.set_uint("timestamp_usec", 5, message.timestamp_usec);
            _myData.set_int("latency_ping", 6, message.latency_ping);
            _myData.set_byte_seq("bin_data", 7, my_byteSeq);
        }

        public TestMessage copyFromSeqToMessage(LoanableSequence<DynamicData> data_sequence, int index)
        {

            DynamicData msg = ((DynamicDataSeq)data_sequence).get_at(index);
            TestMessage message = new TestMessage();

            message.entity_id = msg.get_int("entity_id", 2);
            message.seq_num = msg.get_uint("seq_num", 3);
            message.timestamp_sec = msg.get_int("timestamp_sec", 4);
            message.timestamp_usec = msg.get_uint("timestamp_usec", 5);
            message.latency_ping = msg.get_int("latency_ping", 6);

            ByteSeq bin_data = new ByteSeq();
            msg.get_byte_seq(bin_data, "bin_data", 7);
            message.data = new byte[bin_data.length];
            bin_data.to_array(message.data);
            message.size = bin_data.length;

            return message;
        }

        public DynamicData getData()
        {
            return _myData;
        }

        public void bindataUnloan()
        {
        }

        public AbstractTypedTypeSupport<DynamicData> getTypeSupport()
        {

            if (!_isKeyed)
            {
                return new DynamicDataTypeSupport(
                        TestData_tTypeSupport.get_typecode(),
                        DynamicDataTypeProperty_t.DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
            }
            else
            {
                return new DynamicDataTypeSupport(
                        TestDataKeyed_tTypeSupport.get_typecode(),
                        DynamicDataTypeProperty_t.DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
            }
        }

        public ITypeHelper<DynamicData> clone()
        {
            return new DynamicDataTypeHelper(_myData, _maxPerftestSampleSize);
        }

        public LoanableSequence<DynamicData> createSequence()
        {
            return new DynamicDataSeq();
        }

        public void setBinDataMax(int newMax)
        {
            ByteSeq bin_data = new ByteSeq();
            _myData.get_byte_seq(bin_data, "bin_data", 7);
            bin_data.maximum = newMax;
        }

        public void binDataUnloan()
        {
        }

        public ulong getMaxPerftestSampleSize() {
            return _maxPerftestSampleSize;
        }
    }

    /*********************************************************
     * perftest_cs
     */
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
            if (!ParseConfig(argv))
            {
                return;
            }
            ulong _maxPerftestSampleSize = Math.Max(_DataLen,LENGTH_CHANGED_SIZE);
            if (_useUnbounded > 0) {
                Console.Write("Using unbounded Sequences, memory_manager " + _useUnbounded.ToString() + ".\n");
                if (_isKeyed)
                {
                    Console.Error.Write("Using keyed Data.\n");
                    if (_isDynamicData)
                    {
                        Console.Error.Write("Using Dynamic Data.\n");
                        _MessagingImpl = new RTIDDSImpl<DynamicData>(new DynamicDataTypeHelper(TestDataKeyedLarge_t.get_typecode(),_isKeyed,_maxPerftestSampleSize));
                    }
                    else
                    {
                        _MessagingImpl = new RTIDDSImpl<TestDataKeyedLarge_t>(new DataTypeKeyedLargeHelper(_maxPerftestSampleSize));
                    }
                }
                else {
                    Console.Error.Write("Using unkeyed Data.\n");
                    if (_isDynamicData)
                    {
                        Console.Error.Write("Using Dynamic Data.\n");
                        _MessagingImpl = new RTIDDSImpl<DynamicData>(new DynamicDataTypeHelper(TestDataLarge_t.get_typecode(),_isKeyed,_maxPerftestSampleSize));
                    }
                    else
                    {
                        _MessagingImpl = new RTIDDSImpl<TestDataLarge_t>(new DataTypeLargeHelper(_maxPerftestSampleSize));
                    }
                }
            } else {
                if (_isKeyed)
                {
                    Console.Error.Write("Using keyed Data.\n");
                    if (_isDynamicData)
                    {
                        Console.Error.Write("Using Dynamic Data.\n");
                        _MessagingImpl = new RTIDDSImpl<DynamicData>(new DynamicDataTypeHelper(TestDataKeyed_t.get_typecode(),_isKeyed,_maxPerftestSampleSize));
                    }
                    else
                    {
                        _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>(new DataTypeKeyedHelper(_maxPerftestSampleSize));
                    }
                }
                else {
                    Console.Error.Write("Using unkeyed Data.\n");
                    if (_isDynamicData)
                    {
                        Console.Error.Write("Using Dynamic Data.\n");
                        _MessagingImpl = new RTIDDSImpl<DynamicData>(new DynamicDataTypeHelper(TestData_t.get_typecode(),_isKeyed,_maxPerftestSampleSize));
                    }
                    else
                    {
                        _MessagingImpl = new RTIDDSImpl<TestData_t>(new DataTypeHelper(_maxPerftestSampleSize));
                    }
                }
            }

            if (!_MessagingImpl.Initialize(_MessagingArgc, _MessagingArgv))
            {
                return;
            }

            _BatchSize = _MessagingImpl.GetBatchSize();

            if (_BatchSize != 0) {
                _SamplesPerBatch = _BatchSize/(int)_DataLen;
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
            if (_MessagingImpl != null)
            {
                _MessagingImpl.Shutdown();
            }
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
                "\t-dataLen <bytes>        - Set length of payload for each send,\n" +
                "\t                          default 100\n"  +
                "\t-unbounded <managerMemory> - Use unbounded Sequences\n" +
                "\t                             default if bounded,  managerMemory not set.\n" +
                "\t                             default if unbounded, managerMemory is "+ MAX_BOUNDED_SEQ_SIZE.VALUE +".\n" +
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
                "\t-spin <count>           - Number of times to run in spin loop between\n" +
                "\t                          each send, default 0 (deprecated)\n" +
                "\t-latencyCount <count>   - Number samples (or batches) to send before a\n" +
                "\t                          latency ping packet is sent, default\n" +
                "\t                          10000 if -latencyTest is not specified,\n" +
                "\t                          1 if -latencyTest is specified\n" +
                "\t-numSubscribers <count> - Number of subscribers running in test,\n" +
                "\t                          default 1\n" +
                "\t-numPublishers <count>  - Number of publishers running in test,\n" +
                "\t                          default 1\n" +
                "\t-scan                   - Run test in scan mode, traversing a range of\n"+
                "\t                          data sizes, 32 - " + getMaxPerftestSampleSizeCS() + "\n" +
                "\t-noPrintIntervals       - Don't print statistics at intervals during\n"+
                "\t                          test\n" +
                "\t-useReadThread          - Use separate thread instead of callback to\n"+
                "\t                          read data\n" +
                "\t-latencyTest            - Run a latency test consisting of a ping-pong \n" +
                "\t                          synchronous communication\n" +
                "\t-verbosity <level>      - Run with different levels of verbosity:\n" +
                "\t                          0 - SILENT, 1 - ERROR, 2 - WARNING,\n" +
                "\t                          3 - ALL. Default: 1\n" +
                "\t-pubRate <samples/s>:<method>    - Limit the throughput to the specified number\n" +
                "\t                                   of samples/s, default 0 (don't limit)\n" +
                "\t                                   [OPTIONAL] Method to control the throughput can be:\n" +
                "\t                                   'spin' or 'sleep'\n" +
                "\t                                   Default method: spin\n" +
                "\t-keyed                  - Use keyed data (default: unkeyed)\n"+
                "\t-executionTime <sec>    - Set a maximum duration for the test. The\n"+
                "\t                          first condition triggered will finish the\n"+
                "\t                          test: number of samples or execution time.\n"+
                "\t                          Default 0 (don't set execution time)\n" +
                "\t-writerStats            - Display the Pulled Sample count stats for\n" +
                "\t                          reliable protocol debugging purposes.\n" +
                "\t                          Default: Not set\n" +
                "\t-cpu                   - Display the cpu percent use by the process\n" +
                "\t                          Default: Not set\n" +
                "\t-cft <start>:<end>      - Use a Content Filtered Topic for the Throughput topic in the subscriber side.\n" +
                "\t                          Specify 2 parameters: <start> and <end> to receive samples with a key in that range.\n" +
                "\t                          Specify only 1 parameter to receive samples with that exact key.\n" +
                "\t                          Default: Not set\n";


            int argc = argv.Length;

            if (argc < 0)
            {
                Console.Error.Write(usage_string);
                _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>(
                        new DataTypeKeyedHelper(
                                Math.Max(_DataLen,LENGTH_CHANGED_SIZE)));
                _MessagingImpl.PrintCmdLineHelp();
                return false;
            }

            /*
             * PERFTEST-108
             * We add this boolean value to check if we are explicity changing
             * the number of iterations via command line paramenter. This will
             * only be used if this is a latency test to decrease or not the
             * default number of iterations.
             */
            bool numIterSet = false;

            for (int i = 0; i < argc; ++i)
            {
                if ( "-help".StartsWith(argv[i], true, null) )
                {
                    Console.Error.Write(usage_string);
                    _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>(
                            new DataTypeKeyedHelper(
                                    Math.Max(_DataLen,LENGTH_CHANGED_SIZE)));
                    _MessagingImpl.PrintCmdLineHelp();
                    return false;
                }
            }

            for (int i = 0; i < argc; ++i)
            {
                if ("-pub".StartsWith(argv[i], true, null))
                {
                    _IsPub = true;
                    _MessagingArgv[_MessagingArgc++] = argv[i];
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

                    if (!UInt64.TryParse(argv[i], out _NumIter))
                    {
                        Console.Error.Write("Bad numIter\n");
                        return false;
                    }

                    if (_NumIter < 1)
                    {
                        Console.Error.Write("-numIter must be > 0\n");
                        return false;
                    }

                    numIterSet = true;
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

                    if (!UInt64.TryParse(argv[i], out _DataLen))
                    {
                        Console.Error.Write("Bad dataLen\n");
                        return false;
                    }
                    if (_DataLen < OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("dataLen must be >= " + OVERHEAD_BYTES);
                        return false;
                    }
                    if (_DataLen > getMaxPerftestSampleSizeCS())
                    {
                        Console.Error.WriteLine("dataLen must be <= " + getMaxPerftestSampleSizeCS());
                        return false;
                    }
                    if (_useUnbounded == 0 && (int)_DataLen > MAX_BOUNDED_SEQ_SIZE.VALUE) {
                        _useUnbounded = (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE;
                    }
                }
                else if ("-unbounded".StartsWith(argv[i], true, null))
                {
                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    if ((i == (argc - 1)) || argv[i+1].StartsWith("-"))
                    {
                        _useUnbounded = (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE;
                    } else {
                        ++i;
                        _MessagingArgv[_MessagingArgc++] = argv[i];
                        if (!UInt64.TryParse(argv[i], out _useUnbounded))
                        {
                            Console.Error.Write("Bad managerMemory value\n");
                            return false;
                        }
                    }

                    if (_useUnbounded < OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("_useUnbounded must be >= " + OVERHEAD_BYTES);
                        return false;
                    }
                    if (_useUnbounded > getMaxPerftestSampleSizeCS())
                    {
                        Console.Error.WriteLine("_useUnbounded must be <= " + getMaxPerftestSampleSizeCS());
                        return false;
                    }
                }
                else if ( "-spin".StartsWith(argv[i], true, null) ) {
                    Console.Error.Write("-spin option is deprecated. It will be removed "+
                        "in upcoming releases.\n");
                    if (( i == (argc-1)) || argv[++i].StartsWith("-") ) {
                        Console.Error.Write("Missing <count> after -spin\n");
                        return false;
                    }
                    if ( !UInt64.TryParse(argv[i], out _SpinLoopCount) ) {
                        Console.Error.Write("Bad spin count\n");
                        return false;
                    }
                }
                else if ( "-sleep".StartsWith(argv[i], true, null) ) {
                    if (( i == (argc-1)) || argv[++i].StartsWith("-") ) {
                        Console.Error.Write("Missing <millisec> after -sleep\n");
                        return false;
                    }
                    if ( !UInt64.TryParse(argv[i], out _SleepNanosec) ) {
                        Console.Error.Write("Bad sleep millisec\n");
                        return false;
                    }
                    _SleepNanosec *= 1000000;
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
                else if ("-dynamicData".StartsWith(argv[i], true, null))
                {
                    _isDynamicData = true;
                }
                else if ("-scan".StartsWith(argv[i], true, null))
                {
                    _IsScan = true;
                }
                else if ("-keyed".StartsWith(argv[i], true, null))
                {
                    _isKeyed = true;
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
                else if ("-writerStats".StartsWith(argv[i], true, null))
                {
                    _displayWriterStats = false;
                }
                else if ("-verbosity".StartsWith(argv[i], true, null))
                {
                    _MessagingArgv[_MessagingArgc++] = argv[i];
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <level> after -verbosity\n");
                        return false;
                    }
                    _MessagingArgv[_MessagingArgc++] = argv[i];
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
                else if ("-pubRate".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <samples/s>:<method> after -pubRate\n");
                        return false;
                    }
                    if (argv[i].Contains(":")) {
                        try {
                            String[] st = argv[i].Split(':');
                            if (!UInt64.TryParse(st[0], out _pubRate))
                            {
                                Console.Error.Write("Bad number for -pubRate\n");
                                return false;
                            }
                            if ("spin".Equals(st[1])){
                                Console.Error.Write("-pubRate method: spin.\n");
                            } else if ("sleep".Equals(st[1])){
                                _pubRateMethodSpin = false;
                                Console.Error.Write("-pubRate method: sleep.\n");
                            } else {
                                Console.Error.Write("<method> for pubRate '" + st[1] + "' is not valid. It must be 'spin' or 'sleep'.\n");
                                return false;
                            }
                        }
                        catch (ArgumentNullException)
                        {
                            Console.Error.Write("Bad pubRate\n");
                            return false;
                        }
                    } else {
                        if (!UInt64.TryParse(argv[i], out _pubRate))
                        {
                            Console.Error.Write("Bad number for -pubRate\n");
                            return false;
                        }
                    }

                    if (_pubRate > 10000000)
                    {
                        Console.Error.Write("-pubRate cannot be greater than 10000000.\n");
                        return false;
                    }
                    else if (_pubRate < 0)
                    {
                        Console.Error.Write("-pubRate cannot be smaller than 0 (set 0 for unlimited).\n");
                        return false;
                    }
                }
                else if ("-executionTime".StartsWith(argv[i], true, null))
                {
                    if ((i == (argc - 1)) || argv[++i].StartsWith("-"))
                    {
                        Console.Error.Write("Missing <seconds> after -executionTime\n");
                        return false;
                    }

                    if (!UInt64.TryParse(argv[i], out _executionTime))
                    {
                        Console.Error.Write("Bad number for -executionTime\n");
                        return false;
                    }
                }
                else if ("-cpu".StartsWith(argv[i], true, null)) {
                    _showCpu = true;
                }
                else if ("-cft".StartsWith(argv[i], true, null)) {
                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    if ((i == (argc - 1)) || argv[++i].StartsWith("-")) {
                        Console.Error.Write("Missing <start>:<end> after -cft\n");
                        return false;
                    }

                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    _useCft = true;
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

                /*
                 * PERFTEST-108
                 * If we are in a latency test, the default value for _NumIter
                 * has to be smaller (to avoid certain issues in platforms with
                 * low memory). Therefore, unless we explicitly changed the
                 * _NumIter value we will use a smaller default:
                 * "numIterDefaultLatencyTest"
                 */
                if (numIterSet)
                {
                    _NumIter = numIterDefaultLatencyTest;
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

            if ((_NumIter > 0) && (_NumIter < (ulong)_LatencyCount))
            {
                Console.Error.Write("numIter ({0}) must be greater than latencyCount ({1}).\n",
                              _NumIter, _LatencyCount);
                return false;
            }

            //manage the parameter: -pubRate -sleep -spin
            if (_IsPub && _pubRate >0) {
                if (_SpinLoopCount > 0) {
                    Console.Error.Write( "'-spin' is not compatible with -pubRate. " +
                        "Spin/Sleep value will be set by -pubRate.");
                    _SpinLoopCount = 0;
                }
                if (_SleepNanosec > 0) {
                    Console.Error.Write("'-sleep' is not compatible with -pubRate. " +
                        "Spin/Sleep value will be set by -pubRate.");
                    _SleepNanosec = 0;
                }
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

            private int _num_publishers;
            private List<int> _finished_publishers;
            public CpuMonitor cpu;
            private bool _useCft;


            public ThroughputListener(IMessagingWriter writer,
                    bool useCft,
                    int numPublishers)
            {
                _writer = writer;
                _last_seq_num = new ulong[numPublishers];
                _num_publishers = numPublishers;
                _finished_publishers = new List<int>();
                cpu = new CpuMonitor();
                _useCft = useCft;
            }

            public ThroughputListener(IMessagingWriter writer,
                    IMessagingReader reader,
                    bool useCft,
                    int numPublishers)
            {
                _writer = writer;
                _reader = reader;
                _last_seq_num = new ulong[numPublishers];
                _num_publishers = numPublishers;
                _finished_publishers = new List<int>();
                _useCft = useCft;
            }

            public void ProcessMessage(TestMessage message)
            {
                if (message.entity_id >= _num_publishers ||
                    message.entity_id < 0)
                {
                    Console.Write("ProcessMessage: message content no valid. message.entity_id out of bounds\n");
                    return;
                }
                // Check for test initialization messages
                if (message.size == INITIALIZE_SIZE)
                {
                    _writer.Send(message);
                    _writer.Flush();
                    return;
                }
                else if (message.size == FINISHED_SIZE)
                {
                    /*
                     * PERFTEST-97
                     * We check the entity_id of the publisher to see if it has
                     * already send a FINISHED_SIZE message. If he has we ignore
                     * any new one. Else, we add it to a vector. Once that
                     * vector contains all the ids of the publishers the
                     * subscriber is suppose to know, that means that all the
                     * publishers have finished sending data samples, so it is
                     * time to finish the subscriber.
                     */
                    if (_finished_publishers.Contains(message.entity_id)) {
                        return;
                    }

                    if (end_test == true)
                    {
                        return;
                    }

                    _finished_publishers.Add(message.entity_id);

                    if (_finished_publishers.Count >= _num_publishers) {
                        if (!_useCft) {
                            // detect missing packets
                            if (message.seq_num != _last_seq_num[message.entity_id]) {
                                // only track if skipped, might have restarted pub
                                if (message.seq_num > _last_seq_num[message.entity_id])
                                {
                                    missing_packets +=
                                        message.seq_num - _last_seq_num[message.entity_id];
                                }
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

                    if (_finished_publishers.Count >= _num_publishers) {
                        String outputCpu = "";
                        if (_showCpu) {
                            outputCpu = cpu.get_cpu_average();
                        }
                        Console.Write("Length: {0,5}  Packets: {1,8}  Packets/s(ave): {2,7:F0}  " +
                                      "Mbps(ave): {3,7:F1}  Lost: {4}{5}\n",
                                      interval_data_length + OVERHEAD_BYTES,
                                      interval_packets_received,
                                      interval_packets_received * 1000000 /interval_time,
                                      interval_bytes_received * 1000000.0 / interval_time * 8.0 / 1000.0 / 1000.0,
                                      interval_missing_packets,
                                      outputCpu
                        );
                    }
                    return;
                }

                // Send back a packet if this is a ping
                if ((message.latency_ping == _SubID)  ||
                        (_useCft && message.latency_ping != -1)) {
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
                        if (!_useCft) {
                            // detect missing packets
                            if (message.seq_num != _last_seq_num[message.entity_id]) {
                                // only track if skipped, might have restarted pub
                                if (message.seq_num > _last_seq_num[message.entity_id])
                                {
                                    missing_packets +=
                                        message.seq_num - _last_seq_num[message.entity_id];
                                }
                            }
                        }

                        interval_time = now - begin_time;
                        interval_packets_received = packets_received;
                        interval_bytes_received = bytes_received;
                        interval_missing_packets = missing_packets;
                        interval_data_length = last_data_length;
                        String outputCpu = "";
                        if (_showCpu) {
                            outputCpu = cpu.get_cpu_average();
                        }
                        Console.Write("Length: {0,5}  Packets: {1,8}  Packets/s(ave): {2,7:F0}  " +
                                      "Mbps(ave): {3,7:F1}  Lost: {4}{5}\n",
                                      interval_data_length + OVERHEAD_BYTES,
                                      interval_packets_received,
                                      interval_packets_received*1000000/interval_time,
                                      interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                                      interval_missing_packets,
                                      outputCpu
                        );
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
                if (!_useCft) {
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
                reader_listener = new ThroughputListener(writer, _useCft, _NumPublishers);
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
                reader_listener = new ThroughputListener(writer, reader, _useCft, _NumPublishers);

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

            if (_showCpu) {
                reader_listener.cpu.initialize();
            }

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
                        String outputCpu = "";
                        if (_showCpu) {
                            outputCpu = reader_listener.cpu.get_cpu_instant();
                        }
                        Console.Write("Packets: {0,8}  Packets/s: {1,7}  Packets/s(ave): {2,7:F0}  " +
                                     "Mbps: {3,7:F1}  Mbps(ave): {4,7:F1}  Lost: {5}{6}\n",
                                     last_msgs, mps, mps_ave,
                                    bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                                     reader_listener.missing_packets,outputCpu);
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
            //PerformanceCounter myAppCpu;
            //PerformanceCounter cpuUsage;
            public CpuMonitor cpu;


            public LatencyListener(IMessagingWriter writer, uint num_latency)
            {
                if (num_latency > 0)
                {
                    _latency_history = new uint[num_latency];
                    _num_latency = num_latency;
                }
                _writer = writer;
                cpu = new CpuMonitor();
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
                        String outputCpu = "";
                        if (_showCpu) {
                            outputCpu = cpu.get_cpu_average();
                        }
                        Console.Write("Length: {0,5}  Latency: Ave {1,6:F0} us  Std {2,6:F1} us  " +
                                      "Min {3,6} us  Max {4,6} us  50% {5,6} us  90% {6,6} us  99% {7,6} us  99.99% {8,6} us  99.9999% {9,6} us{10}\n",
                                      last_data_length + OVERHEAD_BYTES, latency_ave, latency_std, latency_min, latency_max,
                                      _latency_history[count*50/100],
                                      _latency_history[count*90/100],
                                      _latency_history[count*99/100],
                                      _latency_history[(int)(count*(9999.0/10000))],
                                      _latency_history[(int)(count*(999999.0/1000000))],
                                      outputCpu
                        );
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

                        String outputCpu = "";
                        if (_showCpu) {
                            outputCpu = cpu.get_cpu_instant();
                        }
                        Console.Write("One way Latency: {0,6} us  Ave {1,6:F0} us  Std {2,6:F1} us  Min {3,6} us  Max {4,6}{5}\n",
                            latency, latency_ave, latency_std, latency_min, latency_max,outputCpu);
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
                num_latency = (uint)((_NumIter/(ulong)_SamplesPerBatch) / (ulong)_LatencyCount);

                if ((num_latency / (ulong)_SamplesPerBatch) % (ulong)_LatencyCount > 0)

                {

                    num_latency++;

                }
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

            ulong spinPerUsec = 0;
            ulong sleepUsec = 1000;
            if (_pubRate > 0) {
                if (_pubRateMethodSpin) {
                    spinPerUsec = NDDS.Utility.get_spin_per_microsecond();
                    /* A return value of 0 means accuracy not assured */
                    if (spinPerUsec == 0) {
                        Console.Error.Write("Error initializing spin per microsecond. "+
                            "-pubRate cannot be used\n"+"Exiting.\n");
                        return;
                    }
                    _SpinLoopCount = (1000000*spinPerUsec)/_pubRate;
                } else {
                    _SleepNanosec =1000000000/_pubRate;
                }
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
            message.data = new byte[Math.Max(_DataLen,LENGTH_CHANGED_SIZE)];

            Console.Error.Write("Publishing data...\n");
            if (_showCpu) {
                reader_listener.cpu.initialize();
            }

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
            message.size = (int)_DataLen - OVERHEAD_BYTES;

            // Sleep 1 second, then begin test
            System.Threading.Thread.Sleep(1000);

            int num_pings = 0;
            int scan_number = 4; // 4 means start sending with dataLen = 2^5 = 32
            bool last_scan = false;
            int pingID = -1;
            int current_index_in_batch = 0;
            int ping_index_in_batch = 0;
            bool sentPing = false;

            ulong time_now = 0, time_last_check = 0, time_delta = 0;
            ulong pubRate_sample_period = 1;
            ulong rate = 0;

            time_last_check = GetTimeUsec();

            /* Minimum value for pubRate_sample_period will be 1 so we execute 100 times
               the control loop every second, or every sample if we want to send less
               than 100 samples per second */
            if (_pubRate > 100) {
                pubRate_sample_period = _pubRate / 100;
            }

            if (_executionTime > 0)
            {
                setTimeout(_executionTime);
            }
            /********************
             *  Main sending loop
             */

            for (ulong loop = 0; ((_IsScan) || (loop < _NumIter)) &&
                                 (!_testCompleted); ++loop)
            {

                if ((_pubRate > 0) &&
                (loop > 0) &&
                (loop % pubRate_sample_period == 0)) {

                    time_now = GetTimeUsec();

                    time_delta = time_now - time_last_check;
                    time_last_check = time_now;
                    rate = (pubRate_sample_period*1000000)/time_delta;

                    if ( _pubRateMethodSpin) {
                        if (rate > _pubRate) {
                            _SpinLoopCount += spinPerUsec;
                        } else if (rate < _pubRate && _SpinLoopCount > spinPerUsec)
                        {
                            _SpinLoopCount -= spinPerUsec;
                        } else if (rate < _pubRate && _SpinLoopCount <= spinPerUsec)
                        {
                            _SpinLoopCount = 0;
                        }
                    } else {
                        if (rate > _pubRate) {
                           _SleepNanosec += sleepUsec; //plus 1 MicroSec
                        } else if (rate < _pubRate && _SleepNanosec > sleepUsec) {
                            _SleepNanosec -=  sleepUsec; //less 1 MicroSec
                        } else if (rate < _pubRate && _SleepNanosec <= sleepUsec) {
                            _SleepNanosec = 0;
                        }
                    }
                }

                if ( _SpinLoopCount > 0 ) {
                    NDDS.Utility.spin(_SpinLoopCount);
                }

                if ( _SleepNanosec > 0 ) {//sleep Milisec
                    System.Threading.Thread.Sleep((int)_SleepNanosec/1000000);
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

                                if (new_size > MAX_SYNCHRONOUS_SIZE.VALUE)
                                {
                                    // last scan
                                    new_size = MAX_SYNCHRONOUS_SIZE.VALUE - OVERHEAD_BYTES;
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
                                        new_size = MAX_SYNCHRONOUS_SIZE.VALUE - OVERHEAD_BYTES;
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

                                for (int i=0; i< initializeSampleCount; ++i) {
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

                        if (_displayWriterStats && _PrintIntervals) {
                            Console.Write("Pulled samples: {0,7}\n", writer.getPulledSampleCount());
                        }
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
            writer.resetWriteInstance();
            for (int j = 0; j < initializeSampleCount; ++j) {
                writer.Send(message);
                writer.Flush();
            }

            if (_UseReadThread)
            {
                reader.Shutdown();
            }

            System.Threading.Thread.Sleep(1000);

            if (_displayWriterStats)
            {
                Console.Write("Pulled samples: {0,7}\n", writer.getPulledSampleCount());
            }

            if (_testCompleted)
            {
                Console.Error.Write("Finishing test due to timer...\n");
            }
            else
            {
                Console.Error.Write("Finishing test...\n");
            }
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

        private static void Timeout(object source, ElapsedEventArgs e)
        {
            _testCompleted = true;
        }

        private void setTimeout(ulong executionTime)
        {
            timer = new System.Timers.Timer();
            timer.Elapsed += new ElapsedEventHandler(Timeout);
            timer.Interval = executionTime*1000;
            Console.Error.WriteLine("Setting timeout to " + executionTime + " seconds.");
            timer.Enabled = true;
        }

        private ulong  _DataLen = 100;
        private ulong _useUnbounded = 0;
        private int  _BatchSize = 0;
        private int  _SamplesPerBatch = 1;

        private ulong _NumIter = 100000000;
        private bool _IsPub = false;
        private bool _IsScan = false;
        private bool  _UseReadThread = false;
        private ulong  _SpinLoopCount = 0;
        private ulong  _SleepNanosec = 0;
        private int  _LatencyCount = -1;
        private int  _NumSubscribers = 1;
        private int  _NumPublishers  = 1;
        private int  _InstanceCount = 1;
        private IMessaging _MessagingImpl = null;
        private bool _LatencyTest = false;
        private bool _isReliable = true;
        private ulong _pubRate = 0;
        private bool _pubRateMethodSpin = true;
        private bool _isKeyed = false;
        private bool _isDynamicData = false;
        private ulong _executionTime = 0;
        private bool _displayWriterStats = false;
        private bool  _useCft = false;
        private System.Timers.Timer timer = null;

        private static int  _SubID = 0;
        private static int  _PubID = 0;
        private static bool _PrintIntervals = true;
        private static bool _showCpu  = false;
        private static long _ClockFrequency = 0;
        private static bool _testCompleted = false;
        public const string _LatencyTopicName = "Latency";
        public const string _ThroughputTopicName = "Throughput";
        public const string _AnnouncementTopicName = "Announcement";

        /*
         * PERFTEST-108
         * If we are performing a latency test, the default number for _NumIter
         * will be 10 times smaller than the default when performing a
         * throughput test. This will allow Perftest to work better in embedded
         * platforms since the _NumIter parameter sets the size of certain
         * arrays in the latency test mode.
         */
        public const ulong numIterDefaultLatencyTest = 10000000;

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
        public const int LENGTH_CHANGED_SIZE = 1236;

        static public ulong getMaxPerftestSampleSizeCS(){
            if (MAX_PERFTEST_SAMPLE_SIZE.VALUE > 2147483591){
                return 2147483591; //max value for a buffer in C#
            }else {
                return (ulong) MAX_PERFTEST_SAMPLE_SIZE.VALUE;
            }
        }
    }

} // namespace
