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
            _myData = new TestData_t();
            _myData.copy_from(myData);
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
            _myData = new TestDataLarge_t();
            _myData.copy_from(myData);
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
            _myData = new TestDataKeyed_t();
            _myData.copy_from(myData);
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
            _myData = new TestDataKeyedLarge_t();
            _myData.copy_from(myData);
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

    class DynamicDataMembersId
    {
        private static DynamicDataMembersId _instance = new DynamicDataMembersId();
        private Dictionary<string, int> membersId;

        private DynamicDataMembersId(){
            membersId = new Dictionary<string, int>();
            membersId.Add("key", 1);
            membersId.Add("entity_id", 2);
            membersId.Add("seq_num", 3);
            membersId.Add("timestamp_sec", 4);
            membersId.Add("timestamp_usec", 5);
            membersId.Add("latency_ping", 6);
            membersId.Add("bin_data", 7);
        }
        public static DynamicDataMembersId Instance
        {
            get
            {
                return _instance;
            }
        }

        public int at(string key) {
            return membersId[key];
        }
    }

    class DynamicDataTypeHelper : ITypeHelper<DynamicData>
    {
        bool _isKeyed;
        DynamicData _myData;
        ulong _maxPerftestSampleSize = perftest_cs.getMaxPerftestSampleSizeCS();
        Byte[] _byteArray;
        int _last_message_size;

        public DynamicDataTypeHelper(DDS.TypeCode typeCode, bool isKeyed, ulong maxPerftestSampleSize)
        {
            _isKeyed = isKeyed;
            _myData = new DynamicData(typeCode, DynamicData.DYNAMIC_DATA_PROPERTY_DEFAULT);
            _maxPerftestSampleSize = maxPerftestSampleSize;
            _byteArray = new Byte[KEY_SIZE.VALUE];
        }

        public DynamicDataTypeHelper(DynamicData myData, ulong maxPerftestSampleSize)
        {
            _myData = myData;
            _maxPerftestSampleSize = maxPerftestSampleSize;
            _byteArray = new Byte[KEY_SIZE.VALUE];
        }

        public void fillKey(int value)
        {
            _byteArray[0] = (byte)(value);
            _byteArray[1] = (byte)(value >> 8);
            _byteArray[2] = (byte)(value >> 16);
            _byteArray[3] = (byte)(value >> 24);
            _myData.set_byte_array(
                    "key",
                    DynamicDataMembersId.Instance.at("key"),
                    _byteArray);
        }

        public void copyFromMessage(TestMessage message)
        {
            if (_last_message_size != message.size) {
                ByteSeq bin_data = new ByteSeq();
                bin_data.ensure_length(message.size, message.size);
                _myData.clear_all_members();
                _myData.set_byte_seq(
                    "bin_data",
                    DynamicDataMembersId.Instance.at("bin_data"),
                    bin_data);
                _last_message_size = message.size;
            }
            _myData.set_int(
                    "entity_id",
                    DynamicDataMembersId.Instance.at("entity_id"),
                    message.entity_id);
            _myData.set_uint(
                    "seq_num",
                    DynamicDataMembersId.Instance.at("seq_num"),
                    message.seq_num);
            _myData.set_int(
                    "timestamp_sec",
                    DynamicDataMembersId.Instance.at("timestamp_sec"),
                    message.timestamp_sec);
            _myData.set_uint(
                    "timestamp_usec",
                    DynamicDataMembersId.Instance.at("timestamp_usec"),
                    message.timestamp_usec);
            _myData.set_int(
                    "latency_ping",
                    DynamicDataMembersId.Instance.at("latency_ping"),
                    message.latency_ping);
        }

        public TestMessage copyFromSeqToMessage(LoanableSequence<DynamicData> data_sequence, int index)
        {

            DynamicData msg = ((DynamicDataSeq)data_sequence).get_at(index);
            TestMessage message = new TestMessage();

            message.entity_id = msg.get_int(
                    "entity_id",
                    DynamicDataMembersId.Instance.at("entity_id"));
            message.seq_num = msg.get_uint(
                    "seq_num",
                    DynamicDataMembersId.Instance.at("seq_num"));
            message.timestamp_sec = msg.get_int(
                    "timestamp_sec",
                    DynamicDataMembersId.Instance.at("timestamp_sec"));
            message.timestamp_usec = msg.get_uint(
                    "timestamp_usec",
                    DynamicDataMembersId.Instance.at("timestamp_usec"));
            message.latency_ping = msg.get_int(
                    "latency_ping",
                    DynamicDataMembersId.Instance.at("latency_ping"));

            ByteSeq bin_data = new ByteSeq();
            msg.get_byte_seq(
                    bin_data,
                    "bin_data",
                    DynamicDataMembersId.Instance.at("bin_data"));
            // message.data is not necesary becasue in the copyFromMessage
            // it is not used. I used ensure_length with the size
            // message.data = new byte[bin_data.length];
            // bin_data already has the size, not necessary to call toArrayByte()
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
            if (!_isKeyed) {
                return new DynamicDataTypeSupport(
                        TestData_tTypeSupport.get_typecode(),
                        DynamicDataTypeProperty_t.DYNAMIC_DATA_TYPE_PROPERTY_DEFAULT);
            }
            else {
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
            _myData.get_byte_seq(
                    bin_data,
                    "bin_data",
                    DynamicDataMembersId.Instance.at("bin_data"));
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
            PrintVersion();

            if (!ParseConfig(argv))
            {
                return;
            }
            ulong _maxPerftestSampleSize = Math.Max(_DataLen,LENGTH_CHANGED_SIZE);
            if (_useUnbounded > 0)
            {
                if (_isKeyed)
                {
                    if (_isDynamicData)
                    {
                        _MessagingImpl = new RTIDDSImpl<DynamicData>(new DynamicDataTypeHelper(TestDataKeyedLarge_t.get_typecode(),_isKeyed,_maxPerftestSampleSize));
                    }
                    else
                    {
                        _MessagingImpl = new RTIDDSImpl<TestDataKeyedLarge_t>(new DataTypeKeyedLargeHelper(_maxPerftestSampleSize));
                    }
                }
                else {
                    if (_isDynamicData)
                    {
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
                    if (_isDynamicData)
                    {
                        _MessagingImpl = new RTIDDSImpl<DynamicData>(new DynamicDataTypeHelper(TestDataKeyed_t.get_typecode(),_isKeyed,_maxPerftestSampleSize));
                    }
                    else
                    {
                        _MessagingImpl = new RTIDDSImpl<TestDataKeyed_t>(new DataTypeKeyedHelper(_maxPerftestSampleSize));
                    }
                }
                else {
                    if (_isDynamicData)
                    {
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

            PrintConfiguration();

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


        // Set the default values into the array _scanDataLenSizes vector
        private void set_default_scan_values(){
            _scanDataLenSizes.Add(32);
            _scanDataLenSizes.Add(64);
            _scanDataLenSizes.Add(128);
            _scanDataLenSizes.Add(256);
            _scanDataLenSizes.Add(512);
            _scanDataLenSizes.Add(1024);
            _scanDataLenSizes.Add(2048);
            _scanDataLenSizes.Add(4096);
            _scanDataLenSizes.Add(8192);
            _scanDataLenSizes.Add(16384);
            _scanDataLenSizes.Add(32768);
            _scanDataLenSizes.Add(63000);

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
                "\t-latencyCount <count>   - Number samples (or batches) to send before a\n" +
                "\t                          latency ping packet is sent, default\n" +
                "\t                          10000 if -latencyTest is not specified,\n" +
                "\t                          1 if -latencyTest is specified\n" +
                "\t-numSubscribers <count> - Number of subscribers running in test,\n" +
                "\t                          default 1\n" +
                "\t-numPublishers <count>  - Number of publishers running in test,\n" +
                "\t                          default 1\n" +
                "\t-scan <size1>:<size2>:...:<sizeN> - Run test in scan mode, traversing\n" +
                "\t                                    a range of sample data sizes from\n" +
                "\t                                    [32,63000] or [63001,2147482620] bytes,\n" +
                "\t                                    in the case that you are using large data or not.\n" +
                "\t                                    The list of sizes is optional.\n" +
                "\t                                    Default values are '32:64:128:256:512:1024:2048:4096:8192:16384:32768:63000'\n" +
                "\t                                    Default: Not set\n" +
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
             * the number of iterations via command-line paramenter. This will
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
                    _MessagingArgv[_MessagingArgc++] = argv[i];
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
                        _useUnbounded = Math.Min(
                                (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE,
                                2 * _DataLen);
                    }
                }
                else if ("-unbounded".StartsWith(argv[i], true, null))
                {
                    _MessagingArgv[_MessagingArgc++] = argv[i];

                    if ((i == (argc - 1)) || argv[i+1].StartsWith("-"))
                    {
                        _useUnbounded = Math.Min(
                                (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE,
                                2 * _DataLen);
                    } else {
                        ++i;
                        _MessagingArgv[_MessagingArgc++] = argv[i];
                        if (!UInt64.TryParse(argv[i], out _useUnbounded))
                        {
                            Console.Error.Write("Bad allocation_threshold value\n");
                            return false;
                        }
                    }

                    if (_useUnbounded < OVERHEAD_BYTES)
                    {
                        Console.Error.WriteLine("_useUnbounded must be >= " + OVERHEAD_BYTES);
                        return false;
                    }
                    if (_useUnbounded > (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE)
                    {
                        Console.Error.WriteLine("_useUnbounded must be <= " +
                                MAX_BOUNDED_SEQ_SIZE.VALUE);
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
                    _isScan = true;
                    _MessagingArgv[_MessagingArgc] = argv[i];

                    if (_MessagingArgv[_MessagingArgc] == null)
                    {
                        Console.Error.Write("Problem allocating memory\n");
                        return false;
                    }

                    _MessagingArgc++;
                    if ((i != (argc - 1)) && !argv[1+i].StartsWith("-")) {
                        ++i;
                        _MessagingArgv[_MessagingArgc] = argv[i];

                        if (_MessagingArgv[_MessagingArgc] == null)
                        {
                            Console.Error.Write("Problem allocating memory\n");
                            return false;
                        }

                        _MessagingArgc++;
                        String[] list_scan = argv[i].Split(':');
                        ulong aux_scan = 0;
                        for( int j = 0; j < list_scan.Length; j++) {
                            if (!UInt64.TryParse(list_scan[j], out aux_scan)) {
                                Console.Error.Write(
                                        "-scan <size> value must have the format '-scan <size1>:<size2>:...:<sizeN>'\n");
                                return false;
                            }
                            _scanDataLenSizes.Add(aux_scan);
                        }
                        if (_scanDataLenSizes.Count < 2) {
                            Console.Error.Write("'-scan <size1>:<size2>:...:<sizeN>' the number of size should be equal or greater then two.\n");
                            return false;
                        }
                        _scanDataLenSizes.Sort();
                        if (_scanDataLenSizes[0] < OVERHEAD_BYTES) {
                            Console.Error.Write("-scan sizes must be >= " +
                                    OVERHEAD_BYTES + "\n");
                            return false;
                        }
                        if (_scanDataLenSizes[_scanDataLenSizes.Count - 1] >
                                (ulong)MAX_PERFTEST_SAMPLE_SIZE.VALUE) {
                            Console.Error.Write("-scan sizes must be <= " +
                                    MAX_PERFTEST_SAMPLE_SIZE.VALUE + "\n");
                            return false;
                        }
                    } else {
                        set_default_scan_values();
                    }
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
                            if ("sleep".Equals(st[1])){
                                _pubRateMethodSpin = false;
                            } else if (!"spin".Equals(st[1])) {
                                Console.Error.Write("<method> for pubRate '" + st[1]
                                        + "' is not valid. It must be 'spin' or 'sleep'.\n");
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
                        Console.Error.Write(
                                "-executionTime value must be a positive number greater than 0\n");
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

            if (_isScan) {
                _DataLen = _scanDataLenSizes[_scanDataLenSizes.Count - 1]; // Max size
                if (_executionTime == 0){
                    _executionTime = 60;
                }
                // Check if large data or small data
                if (_scanDataLenSizes[0] > (ulong)Math.Min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)
                        && _scanDataLenSizes[_scanDataLenSizes.Count - 1] > (ulong)Math.Min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)) {
                    if (_useUnbounded == 0) {
                        _useUnbounded = (ulong)MAX_BOUNDED_SEQ_SIZE.VALUE;
                    }
                } else if (_scanDataLenSizes[0] <= (ulong)Math.Min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)
                        && _scanDataLenSizes[_scanDataLenSizes.Count - 1] <= (ulong)Math.Min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE)) {
                    if (_useUnbounded != 0) {
                        Console.Error.Write("Unbounded will be ignored since -scan is present.\n");
                        _useUnbounded = 0;
                    }
                } else {
                    Console.Error.Write("The sizes of -scan [");
                    for (int i = 0; i < _scanDataLenSizes.Count; i++) {
                        Console.Error.Write(_scanDataLenSizes[i] + " ");
                    }
                    Console.Error.Write(
                            "] should be either all smaller or all bigger than " +
                             Math.Min(MAX_SYNCHRONOUS_SIZE.VALUE,MAX_BOUNDED_SEQ_SIZE.VALUE) +
                             "\n");
                    return false;
                }
            }
            return true;
        }

        private void PrintConfiguration() {

            StringBuilder sb = new StringBuilder();

            // Throughput/Latency mode
            if (_IsPub) {
                sb.Append("\nMode: ");

                if (_LatencyTest) {
                    sb.Append("LATENCY TEST (Ping-Pong test)\n");
                } else {
                    sb.Append("THROUGHPUT TEST\n");
                    sb.Append("      (Use \"-latencyTest\" for Latency Mode)\n");
                }
            }

            sb.Append("\nPerftest Configuration:\n");

            // Reliable/Best Effort
            sb.Append("\tReliability: ");
            if (_isReliable) {
                sb.Append("Reliable\n");
            } else {
                sb.Append("Best Effort\n");
            }

            // Keyed/Unkeyed
            sb.Append("\tKeyed: ");
            if (_isKeyed) {
                sb.Append("Yes\n");
            } else {
                sb.Append("No\n");
            }

            // Publisher/Subscriber and Entity ID
            if (_IsPub) {
                sb.Append("\tPublisher ID: ");
                sb.Append(_PubID);
                sb.Append("\n");
            } else {
                sb.Append("\tSubscriber ID: ");
                sb.Append(_SubID);
                sb.Append("\n");
            }

            if (_IsPub) {

                sb.Append("\tLatency count: 1 latency sample every ");
                sb.Append(_LatencyCount);
                sb.Append("\n");

                // Scan/Data Sizes
                sb.Append("\tData Size: ");
                if (_isScan) {
                    for (int i = 0; i < _scanDataLenSizes.Count; i++ ) {
                        sb.Append(_scanDataLenSizes[i]);
                        if (i == _scanDataLenSizes.Count - 1) {
                            sb.Append("\n");
                        } else {
                            sb.Append(", ");
                        }
                    }
                } else {
                    sb.Append(_DataLen);
                    sb.Append("\n");
                }

                // Batching
                int batchSize = _MessagingImpl.GetBatchSize();

                sb.Append("\tBatching: ");
                if (batchSize > 0) {
                    sb.Append(batchSize);
                    sb.Append(" Bytes (Use \"-batchSize 0\" to disable batching)\n");
                } else if (batchSize == 0) {
                    sb.Append("No (Use \"-batchSize\" to setup batching)\n");
                } else { // < 0 (Meaning, Disabled by RTI Perftest)
                    sb.Append("\"Disabled by RTI Perftest.\"\n");
                    if (batchSize == -1) {
                        if (_LatencyTest) {
                            sb.Append("\t\t  BatchSize disabled for a Latency Test\n");
                        } else {
                            sb.Append("\t\t  BatchSize is smaller than 2 times\n");
                            sb.Append("\t\t  the minimum sample size.\n");
                        }
                    }
                    if (batchSize == -2) {
                        sb.Append("\t\t  BatchSize cannot be used with\n");
                        sb.Append("\t\t  Large Data.\n");
                    }
                }

                // Publication Rate
                sb.Append("\tPublication Rate: ");
                if (_pubRate > 0) {
                    sb.Append(_pubRate);
                    sb.Append(" Samples/s (");
                    if (_pubRateMethodSpin) {
                        sb.Append("Spin)\n");
                    } else {
                        sb.Append("Sleep)\n");
                    }
                } else {
                    sb.Append("Unlimited (Not set)\n");
                }

                // Execution Time or Num Iter
                if (_executionTime > 0) {
                    sb.Append("\tExecution time: ");
                    sb.Append(_executionTime);
                    sb.Append(" seconds\n");
                } else {
                    sb.Append("\tNumber of samples: " );
                    sb.Append(_NumIter);
                    sb.Append("\n");
                }
            } else {
                if (_DataLen > (ulong)MAX_SYNCHRONOUS_SIZE.VALUE) {
                    sb.Append("\tExpecting Large Data Type\n");
                }
            }

            // Listener/WaitSets
            sb.Append("\tReceive using: ");
            if (_UseReadThread) {
                sb.Append("WaitSets\n");
            } else {
                sb.Append("Listeners\n");
            }

            sb.Append(_MessagingImpl.PrintConfiguration());

            Console.Error.WriteLine(sb.ToString());
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
            public bool change_size = false;
            public int   last_data_length = -1;

            // store info for the last data set
            public int   interval_data_length = -1;
            public ulong interval_packets_received = 0;
            public ulong interval_bytes_received = 0;
            public ulong interval_missing_packets = 0;
            public ulong interval_time = 0, begin_time = 0;
            double missing_packets_percent = 0.0;

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
                    _writer.Send(message, false);
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
                        print_summary(message);
                        end_test = true;
                    }
                    return;
                }

                // Send back a packet if this is a ping
                if ((message.latency_ping == _SubID)  ||
                        (_useCft && message.latency_ping != -1)) {
                    _writer.Send(message, false);
                    _writer.Flush();
                }

                // Always check if need to reset internals
                if (message.size == LENGTH_CHANGED_SIZE)
                {
                    print_summary(message);
                    change_size = true;
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
                                        message.seq_num -
                                        _last_seq_num[message.entity_id];
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

            public void print_summary(TestMessage message)
            {
                // store the info for this interval
                ulong now = perftest_cs.GetTimeUsec();

                if (interval_data_length != last_data_length) {
                    if (!_useCft) {
                        // detect missing packets
                        if (message.seq_num != _last_seq_num[message.entity_id]) {
                            // only track if skipped, might have restarted pub
                            if (message.seq_num > _last_seq_num[message.entity_id])
                            {
                                missing_packets +=
                                        message.seq_num -
                                        _last_seq_num[message.entity_id];
                            }
                        }
                    }

                    interval_time = now - begin_time;
                    interval_packets_received = packets_received;
                    interval_bytes_received = bytes_received;
                    interval_missing_packets = missing_packets;
                    interval_data_length = last_data_length;
                    missing_packets_percent = 0.0;

                    // Calculations of missing package percent
                    if (interval_packets_received
                            + interval_missing_packets != 0) {
                        missing_packets_percent =
                                (interval_missing_packets)
                                / (double) (interval_packets_received
                                + interval_missing_packets);
                    }

                    String outputCpu = "";
                    if (_showCpu) {
                        outputCpu = cpu.get_cpu_average();
                    }
                    Console.Write("Length: {0,5}  Packets: {1,8}  Packets/s(ave): {2,7:F0}  " +
                                  "Mbps(ave): {3,7:F1}  Lost: {4,5} ({5,1:p1}){6}\n",
                                  interval_data_length + OVERHEAD_BYTES,
                                  interval_packets_received,
                                  interval_packets_received * 1000000 / interval_time,
                                  interval_bytes_received * 1000000.0 / interval_time * 8.0 / 1000.0 / 1000.0,
                                  interval_missing_packets,
                                  missing_packets_percent,
                                  outputCpu
                    );
                }



                packets_received = 0;
                bytes_received = 0;
                missing_packets = 0;
                // length changed only used in scan mode in which case
                // there is only 1 publisher with ID 0
                _last_seq_num[0] = 0;
                begin_time = now;
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
            writer = _MessagingImpl.CreateWriter(LATENCY_TOPIC_NAME.VALUE);

            if (writer == null) {
                Console.Error.Write("Problem creating latency writer.\n");
                return;
            }

            // Check if using callbacks or read thread
            if (!_UseReadThread)
            {
                // create latency pong reader
                reader_listener = new ThroughputListener(writer, _useCft, _NumPublishers);
                reader = _MessagingImpl.CreateReader(
                        THROUGHPUT_TOPIC_NAME.VALUE,
                        reader_listener);
                if (reader == null)
                {
                    Console.Error.Write("Problem creating throughput reader.\n");
                    return;
                }
            }
            else
            {
                reader = _MessagingImpl.CreateReader(THROUGHPUT_TOPIC_NAME.VALUE, null);
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
            announcement_writer =
                    _MessagingImpl.CreateWriter(ANNOUNCEMENT_TOPIC_NAME.VALUE);

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
            announcement_writer.Send(message, false);
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
            double missing_packets_percent = 0;

            if (_showCpu) {
                reader_listener.cpu.initialize();
            }

            now = GetTimeUsec();
            while (true) {
                prev_time = now;
                System.Threading.Thread.Sleep(1000);
                now = GetTimeUsec();

                if (reader_listener.change_size) { // ACK change_size
                    TestMessage message_change_size = new TestMessage();
                    message_change_size.entity_id = _SubID;
                    message_change_size.data = new byte[1];
                    message_change_size.size = 1;
                    announcement_writer.Send(message_change_size, false);
                    announcement_writer.Flush();
                    reader_listener.change_size = false;
                }
                if (reader_listener.end_test) {
                    TestMessage message_end_test = new TestMessage();
                    message_end_test.entity_id = _SubID;
                    message_end_test.data = new byte[1];
                    message_end_test.size = 1;
                    announcement_writer.Send(message_end_test, false);
                    announcement_writer.Flush();
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

                    // Calculations of missing package percent
                    if (last_msgs + reader_listener.missing_packets == 0) {
                        missing_packets_percent = 0.0;
                    } else {
                        missing_packets_percent =
                                (reader_listener.missing_packets)
                                / (float) (last_msgs
                                + reader_listener.missing_packets);
                    }

                    if (last_msgs > 0)
                    {
                        String outputCpu = "";
                        if (_showCpu) {
                            outputCpu = reader_listener.cpu.get_cpu_instant();
                        }
                        Console.Write("Packets: {0,8}  Packets/s: {1,7}  Packets/s(ave): {2,7:F0}  " +
                                     "Mbps: {3,7:F1}  Mbps(ave): {4,7:F1}  Lost: {5,5} ({6,1:p1}){7}\n",
                                     last_msgs, mps, mps_ave,
                                     bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                                     reader_listener.missing_packets,
                                     missing_packets_percent,
                                     outputCpu);
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
            public int announced_subscriber_replies;
            private List<int> _finished_subscribers;

            public AnnouncementListener() {
                announced_subscriber_replies = 0;
                _finished_subscribers = new List<int>();
            }

            public void ProcessMessage(TestMessage message)
            {
                /*
                 * If the entity_id is not in the list of subscribers
                 * that finished the test, add it.
                 *
                 * Independently, decrease announced_subscriber_replies if a known
                 * writer responds to a message using this channel. We use
                 * this as a way to check that all the readers have received
                 * a message written by the Throughput writer.
                 */
                if (!_finished_subscribers.Contains(message.entity_id)) {
                    _finished_subscribers.Add(message.entity_id);
                    announced_subscriber_replies++;
                } else{
                    announced_subscriber_replies--;
                }
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
            private uint  latency_min = perftest_cs.LATENCY_RESET_VALUE;
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
                        return;

                    // Data length is changing size
                    case LENGTH_CHANGED_SIZE:
                        print_summary_latency();
                        return;

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

                if (latency_min == perftest_cs.LATENCY_RESET_VALUE) {
                    latency_min = latency;
                    latency_max = latency;
                } else {
                    if (latency < latency_min) {
                        latency_min = latency;
                    } else if (latency > latency_max) {
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

            public void print_summary_latency()
            {
                double latency_ave;
                double latency_std;
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
                latency_min = perftest_cs.LATENCY_RESET_VALUE;
                latency_max = 0;
                count = 0;
                clock_skew_count = 0;
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
            int announcementSampleCount = 50;
            int samplesPerBatch = 1;

            // create throughput/ping writer
            writer = _MessagingImpl.CreateWriter(THROUGHPUT_TOPIC_NAME.VALUE);

            if (writer == null)
            {
                Console.Error.Write("Problem creating throughput writer.\n");
                return;
            }

            samplesPerBatch = GetSamplesPerBatch();

            num_latency = (uint)((_NumIter/(ulong)samplesPerBatch)
                    / (ulong)_LatencyCount);

            if ((num_latency / (ulong)samplesPerBatch)
                    % (ulong)_LatencyCount > 0) {
                num_latency++;
            }

            // in batch mode, might have to send another ping
            if (samplesPerBatch > 1) {
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
                    reader = _MessagingImpl.CreateReader(
                            LATENCY_TOPIC_NAME.VALUE,
                            reader_listener);
                    if (reader == null)
                    {
                        Console.Error.Write("Problem creating latency reader.\n");
                        return;
                    }
                }
                else
                {
                    reader = _MessagingImpl.CreateReader(LATENCY_TOPIC_NAME.VALUE, null);
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
            announcement_reader = _MessagingImpl.CreateReader(
                    ANNOUNCEMENT_TOPIC_NAME.VALUE,
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

            Console.Error.Write("Waiting to discover {0} subscribers ...\n", _NumSubscribers);
            writer.WaitForReaders(_NumSubscribers);

            // We have to wait until every Subscriber sends an announcement message
            // indicating that it has discovered every Publisher
            Console.Error.Write("Waiting for subscribers announcement ...\n");
            while (_NumSubscribers > announcement_reader_listener.announced_subscriber_replies) {
                System.Threading.Thread.Sleep(1000);
            }

            if (_showCpu)
            {
                reader_listener.cpu.initialize();
            }

            // Allocate data and set size
            TestMessage message = new TestMessage();
            message.entity_id = _PubID;
            message.data = new byte[Math.Max(_DataLen,LENGTH_CHANGED_SIZE)];

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

            int initializeSampleCount = Math.Max(
                   _MessagingImpl.GetInitializationSampleCount(),
                   _InstanceCount);

            Console.Error.WriteLine(
                    "Sending " + initializeSampleCount + " initialization pings ...");

            for (int i = 0; i < initializeSampleCount; i++)
            {
                // Send test initialization message
                writer.Send(message, true);
            }
            writer.Flush();

            Console.Error.Write("Publishing data ...\n");

            // Set data size, account for other bytes in message
            message.size = (int)_DataLen - OVERHEAD_BYTES;

            // Sleep 1 second, then begin test
            System.Threading.Thread.Sleep(1000);

            int num_pings = 0;
            int scan_count = 0;
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

            if (_executionTime > 0 && !_isScan) {
                setTimeout(_executionTime);
            }
            /********************
             *  Main sending loop
             */

            for (ulong loop = 0; ((_isScan) || (loop < _NumIter)) &&
                                 (!_testCompleted); ++loop)
            {

                if ((_pubRate > 0)
                        && (loop > 0)
                        && (loop % pubRate_sample_period == 0)) {

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
                if ( (_PubID == 0) && (((loop/(ulong)samplesPerBatch)
                        % (ulong)_LatencyCount) == 0) )
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
                        if (_isScan && _testCompletedScan) {
                            _testCompletedScan = false;
                            setTimeout(_executionTime);

                            // flush anything that was previously sent
                            writer.Flush();
                            writer.waitForAck(
                                timeout_wait_for_ack_sec,
                                timeout_wait_for_ack_nsec);

                            announcement_reader_listener.announced_subscriber_replies =
                                    _NumSubscribers;

                            if (scan_count == _scanDataLenSizes.Count) {
                                break; // End of scan test
                            }

                            message.size = LENGTH_CHANGED_SIZE;
                            // must set latency_ping so that a subscriber sends us
                            // back the LENGTH_CHANGED_SIZE message
                            message.latency_ping = num_pings % _NumSubscribers;

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
                            while (announcement_reader_listener.announced_subscriber_replies > 0) {
                                writer.Send(message, true);
                                writer.waitForAck(
                                    timeout_wait_for_ack_sec,
                                    timeout_wait_for_ack_nsec);
                            }
                            message.size = (int)(_scanDataLenSizes[scan_count++] - OVERHEAD_BYTES);
                            /* Reset _SamplePerBatch */
                            samplesPerBatch = GetSamplesPerBatch();

                            ping_index_in_batch = 0;
                            current_index_in_batch = 0;
                        }

                        // Each time ask a different subscriber to echo back
                        pingID = num_pings % _NumSubscribers;
                        ulong now = GetTimeUsec();
                        message.timestamp_sec = (int)((now >> 32) & 0xFFFFFFFF);
                        message.timestamp_usec = (uint)(now & 0xFFFFFFFF);

                        ++num_pings;
                        ping_index_in_batch =
                                (ping_index_in_batch + 1) % samplesPerBatch;
                        sentPing = true;

                        if (_displayWriterStats && _PrintIntervals) {
                            Console.Write("Pulled samples: {0,7}\n", writer.getPulledSampleCount());
                        }
                    }
                }

                current_index_in_batch =
                        (current_index_in_batch + 1) % samplesPerBatch;

                message.seq_num = (uint)loop;
                message.latency_ping = pingID;
                writer.Send(message, false);

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
            int j = 0;
            announcement_reader_listener.announced_subscriber_replies =
                    _NumSubscribers;
            while (announcement_reader_listener.announced_subscriber_replies > 0
                    && j < announcementSampleCount) {
                writer.Send(message, true);
                writer.Flush();
                writer.waitForAck(
                    timeout_wait_for_ack_sec,
                    timeout_wait_for_ack_nsec);
                j++;
            }

            if (_PubID == 0) {
                reader_listener.print_summary_latency();
                reader_listener.end_test = true;
            } else {
                Console.Write("Latency results are only shown when -pidMultiPubTest = 0\n");
            }

            if (_UseReadThread)
            {
                reader.Shutdown();
            }

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

        private static void TimeoutScan(object source, ElapsedEventArgs e)
        {
            _testCompletedScan = true;
        }

        private void setTimeout(ulong executionTime)
        {
            if (timer == null) {
                timer = new System.Timers.Timer();
                if (!_isScan) {
                    timer.Elapsed += new ElapsedEventHandler(Timeout);
                    Console.Error.WriteLine("Setting timeout to "
                            + executionTime + " seconds.");
                } else {
                    timer.Elapsed += new ElapsedEventHandler(TimeoutScan);
                }
                timer.Interval = executionTime * 1000;
                timer.Enabled = true;
            }
        }

        public int GetSamplesPerBatch()
        {
            int batchSize = _MessagingImpl.GetBatchSize();
            int samplesPerBatch;

            if (batchSize > 0)
            {
                samplesPerBatch = batchSize / (int) _DataLen;
                if (samplesPerBatch == 0)
                {
                    samplesPerBatch = 1;
                }
            } else
            {
                samplesPerBatch = 1;
            }

            return samplesPerBatch;
        }

        public ProductVersion_t GetDDSVersion()
        {
            return NDDS.ConfigVersion.get_instance().get_product_version();
        }

        public Perftest_ProductVersion_t GetPerftestVersion()
        {
            return _version;
        }

        public void PrintVersion()
        {
            Perftest_ProductVersion_t perftestV = GetPerftestVersion();
            ProductVersion_t ddsV = GetDDSVersion();

            if (perftestV.major == 9
                    && perftestV.minor == 9
                    && perftestV.release == 9) {
                Console.Write("RTI Perftest Master");
            } else {
                Console.Write(
                        "RTI Perftest "
                        + perftestV.major + "."
                        + perftestV.minor + "."
                        + perftestV.release);
                if (perftestV.revision != 0) {
                    Console.Write("." + perftestV.revision);
                }
                Console.Write(
                        " (RTI Connext DDS: "
                        + ddsV.major + "."
                        + ddsV.minor + "."
                        + ddsV.release + ")\n");
            }
        }

        public struct Perftest_ProductVersion_t
        {
            public uint  major;
            public uint minor;
            public uint release;
            public uint revision;

            public Perftest_ProductVersion_t (
                    uint major,
                    uint minor,
                    uint release,
                    uint revision)
            {
                this.major = major;
                this.minor = minor;
                this.release = release;
                this.revision = revision;
            }
        }

        private ulong  _DataLen = 100;
        private ulong _useUnbounded = 0;

        private ulong _NumIter = 100000000;
        private bool _IsPub = false;
        private bool _isScan = false;
        private List<ulong> _scanDataLenSizes = new List<ulong>();
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
        private static bool _testCompletedScan = true;
        public const int timeout_wait_for_ack_sec = 0;
        public const uint timeout_wait_for_ack_nsec = 10000000;
        public static readonly Perftest_ProductVersion_t _version =
                new Perftest_ProductVersion_t(9, 9, 9, 9);


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

        // Flag used to indicate message is used for initialization only
        private const int INITIALIZE_SIZE = 1234;
        // Flag used to indicate end of test
        private const int FINISHED_SIZE = 1235;
        // Flag used to indicate end of test
        public const int LENGTH_CHANGED_SIZE = 1236;

        /*
         * Value used to compare against to check if the latency_min has
         * been reset.
         */
        public const uint LATENCY_RESET_VALUE = uint.MaxValue;

        static public ulong getMaxPerftestSampleSizeCS(){
            if (MAX_PERFTEST_SAMPLE_SIZE.VALUE > 2147483591){
                return 2147483591; //max value for a buffer in C#
            }else {
                return (ulong) MAX_PERFTEST_SAMPLE_SIZE.VALUE;
            }
        }
    }

} // namespace
