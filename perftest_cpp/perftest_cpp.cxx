/* $Id: perftest_cpp.cxx,v 1.2.2.1 2014/04/01 11:56:52 juanjo Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.    	
 Permission to modify and use for internal purposes granted.   	
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 1.0b,24jul10,jsr Added MAX_BINDATA_SIZE check
 1.0b,13jul10,jsr Removed goto to keep consistency through languages, only
                  release semaphore when it is a latency pong
 1.0b,13jul10,jsr Added waitForPingResponse with timeout
 1.0b,07jul10,jsr Fixing end loop
 1.0b,07jul10,eys Cleanup perftest parameters
 1.0b,19apr10,acr Added latencyTest
 1.0b,06apr10,jsr Fixed RTIOsapiThread_new
 1.0a,14may09,fcs Added instances to INI
 1.0a,14may09,fcs Fixed command-line arguments processing
 1.0a,14may09,fcs Fixed output in scan mode
 1.0a,30apr09,jsr Added fflush to ensure the correct output
 1.0a,29apr09,jsr Added detection of wrong command line parameter
 1.0a,23apr09,jsr Changed to stderr the error and status messages
 1.0a,21apr09,jsr Reformat the help menu
 1.0a,03dec08,jsr Added -HeartbeatPeriod and -FastHeartbeatPeriod options
 1.0a,02oct08,eys Added 99.99% latency
 1.0a,22aug08,eys Flush after every send when initializing the channel
 1.0a,22aug08,eys decrease checking interval on subscriber to gather latency
                  numbers
 1.0a,20aug08,eys initialize channel by writing InstanceCount initially
 1.0a,13aug08,ch  added check to not exceed MAX_BINDATA_SIZE
 1.0a,09aug08,ch  increased sleep after FINISHED_MESSAGE send to make sure
                  it gets through in the multi-instance case
 1.0a,10jun07,fcs Fixed _last_seq_num allocation
 1.0a,12may08,hhw Fixed some length checks to accommodate 32 byte overhead.
 1.0a,09may08,ch  Added keyed type support
 1.0a,04may08,hhw Modified batch processing.
 1.0a,22apr08,fcs Removed sleep from spinning
 1.0a,22apr08,fcs Fixed lost count
 1.0a,21apr08,ch  Output modifications for automation
 1.0a,21apr08,fcs Support for multiple publishers
 1.0a,12apr08,fcs Fixed batch ping
 1.0a,07apr08,hhw Now printing end of test in listener to avoid race condition.
 1.0a,19mar08,hhw Created.

===================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// STL needed for sorting
#include <algorithm>

#include "perftest_cpp.h"
#include "Property.h"

#include "RTIDDSImpl.h"

#ifdef RTI_WIN32
#define STRNCASECMP     _strnicmp
#else
#define STRNCASECMP     strncasecmp
#endif
#define IS_OPTION(str, option) (STRNCASECMP(str, option, strlen(str)) == 0)

int  perftest_cpp::_SubID = 0;
int  perftest_cpp::_PubID = 0;
bool perftest_cpp::_PrintIntervals = true;
bool perftest_cpp::_IsDebug = false;
const char *perftest_cpp::_LatencyTopicName = "Latency";
const char *perftest_cpp::_AnnouncementTopicName = "Announcement";
const char *perftest_cpp::_ThroughputTopicName = "Throughput";

#ifdef RTI_WIN32
LARGE_INTEGER perftest_cpp::_ClockFrequency = {0, 0};
#endif

/*********************************************************
 * Main
 */
int main(int argc, char *argv[])
{
    perftest_cpp app;
    
    return app.Run(argc, argv);
}

int perftest_cpp::Run(int argc, char *argv[])
{ 
    _MessagingImpl = new RTIDDSImpl();

    if (!ParseConfig(argc, argv))
    {
        return -1;
    }

    if ( !_MessagingImpl->Initialize(_MessagingArgc, _MessagingArgv) ) 
    {
        return -1;
    }

    _BatchSize = _MessagingImpl->GetBatchSize();
    _maxBinDataSize = _MessagingImpl->GetMaxBinDataSize();

    if (_BatchSize != 0) {
        _SamplesPerBatch = _BatchSize/_DataLen;
        if (_SamplesPerBatch == 0) {
            _SamplesPerBatch = 1;
        }        
    } else {
        _SamplesPerBatch = 1;
    }

    if (_IsPub) {
        return Publisher();
    } else {
        return Subscriber();
    }
}

/*********************************************************
 * Destructor
 */
perftest_cpp::~perftest_cpp()
{
    for (int i = 0; i< _MessagingArgc; ++i) {
        if (_MessagingArgv[i] != NULL) {
            delete []_MessagingArgv[i];
        }
    }

    if (_MessagingArgv != NULL) {
        delete []_MessagingArgv;
    }

    _MessagingImpl->Shutdown();
    
    fprintf(stderr,"Test ended.\n");
    fflush(stderr);
}

/*********************************************************
 * Constructor 
 */
perftest_cpp::perftest_cpp() 
{
    _DataLen = 100;
    _BatchSize = 0;
    _maxBinDataSize = TestMessage::MAX_DATA_SIZE;
    _SamplesPerBatch = 1;
    _NumIter = 0;
    _IsPub = false;
    _IsScan = false;
    _UseReadThread = false;
    _SpinLoopCount = 0;
    _SleepMillisec = 0;
    _LatencyCount = -1;
    _NumSubscribers = 1;
    _NumPublishers = 1;
    _InstanceCount = 1;
    _MessagingImpl = NULL;
    _ConfigFile = "perftest.ini";
    _MessagingArgv = NULL;
    _MessagingArgc = 0;
    _LatencyTest = false;
    _IsReliable = true;

#ifdef RTI_WIN32
    QueryPerformanceFrequency(&_ClockFrequency);
#endif
};


/*********************************************************
 * ParseArgs
 */
bool perftest_cpp::ParseConfig(int argc, char *argv[])
{
    _MessagingArgc = 0;
    _MessagingArgv = new char*[argc];

    if (_MessagingArgv == NULL) {
        fprintf(stderr, "Problem allocating memory\n");
        return false;
    }

    for (int i=0; i<argc; i++) {
        _MessagingArgv[i] = NULL;
    }

    const char *usage_string =
        /**************************************************************************/
        "Usage:\n"
        "       perftest_cpp [options]\n"
        "\nWhere [options] are (case insensitive, partial match OK):\n\n"
        "\t-help                   - Print this usage message and exit\n"
        "\t-pub                    "
        "- Set test to be a publisher\n"
        "\t-sub                    - Set test to be a subscriber (default)\n"
        "\t-sidMultiSubTest <id>   - Set id of the subscriber in a multi-subscriber\n"
        "\t                          test, default 0\n"
        "\t-pidMultiPubTest <id>   - Set id of the publisher in a multi-publisher \n"
        "\t                          test, default 0. Only publisher 0 sends \n"   
        "\t                          latency pings\n"
        "\t-configFile <filename>  - Set the name of the .ini configuration file, \n"
        "\t                          default perftest.ini\n"
        "\t-dataLen <bytes>        - Set length of payload for each send\n"
	"\t                          default 100\n"
        "\t-numIter <count>        - Set number of messages to send,\n"
	"\t                          default 0 (infinite)\n"
        "\t-instances <count>      - set the number of instances (keys) to iterate\n"
        "\t                          over when publishing, default 1\n"
        "\t-sleep <millisec>       - Time to sleep between each send, default 0\n"
        "\t-spin <count>           - Number of times to run in spin loop between\n"
        "\t                          each send, default 0\n"
        "\t-latencyCount <count>   - Number of samples (or batches) to send before \n"
        "\t                          a latency ping packet is sent, default\n"
	"\t                          10000 if -latencyTest is not specified,\n"
	"\t                          1 if -latencyTest is specified.\n"
        "\t-numSubscribers <count> - Number of subscribers running in test, \n"
        "\t                          default 1\n"
        "\t-numPublishers <count>  - Number of publishers running in test, \n"
        "\t                          default 1\n"
        "\t-scan                   - Run test in scan mode, traversing a range of \n"
        "\t                          data sizes, 32 - 63000\n"
        "\t-noPrintIntervals       - Don't print statistics at intervals during \n"
        "\t                          test\n"
        "\t-useReadThread          - Use separate thread instead of callback to \n"
        "\t                          read data\n"
	"\t-latencyTest            - Run a latency test consisting of a ping-pong \n"
	"\t                          synchronous communication\n"
        "\t-debug                  - Run in debug mode\n";
    
    if (argc < 1)
    {
        fprintf(stderr,usage_string);
        fflush(stderr);
        _MessagingImpl->PrintCmdLineHelp();
        return false;
    }

    int i;
    // first scan for configFile
    for (i = 1; i < argc; ++i) 
    {
        if (IS_OPTION(argv[i], "-help")) {
            fprintf(stderr, usage_string);
            fflush(stderr);
            _MessagingImpl->PrintCmdLineHelp();
            return false;
        }
        else if (IS_OPTION(argv[i], "-configFile")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <fileName> after -configFile\n");
                return false;
            }
            _ConfigFile = argv[i];
        }
    }

    // now load configuration values from config file
    if (_ConfigFile != NULL)
    {
        QosDictionary *configSource;
        try {
            configSource = new QosDictionary(_ConfigFile);
        } catch (std::logic_error e) {
            fprintf(stderr, "Problem loading configuration file.\n");
            fprintf(stderr, "%s\n", e.what());
            return false;
        }

        QosProfile* config = configSource->get("perftest");
        
        if (config == NULL)
        {
            fprintf(stderr, "Could not find section [perftest] in file %s\n", _ConfigFile);
            return false;
        }

        _NumIter        = config->get_int("num iter", _NumIter);
        _DataLen        = config->get_int("data length", _DataLen);
        _SpinLoopCount  = config->get_int("spin loop count", _SpinLoopCount);
        _SleepMillisec  = config->get_int("sleep millisec", _SleepMillisec);
        _LatencyCount   = config->get_int("latency count", _LatencyCount);
        _NumSubscribers = config->get_int("num subscribers", _NumSubscribers);
        _NumPublishers  = config->get_int("num publishers", _NumPublishers);
        _IsScan         = config->get_bool("scan", _IsScan);
        _PrintIntervals = config->get_bool("print intervals", _PrintIntervals);
        _UseReadThread  = config->get_bool("use read thread", _UseReadThread);
        _IsDebug        = config->get_bool("is debug", _IsDebug);
        _InstanceCount  = config->get_int("instances", _InstanceCount);
	_LatencyTest    = config->get_bool("run latency test", _LatencyTest);
        _IsReliable     = config->get_bool("is reliable", _IsReliable);
    }
    
    // now load everything else, command line params override config file
    for (i = 0; i < argc; ++i) 
    {
        if (IS_OPTION(argv[i], "-pub")) 
        {
            _IsPub = true;
        }
        else if (IS_OPTION(argv[i], "-sub")) 
        {
            _IsPub = false;
        } 
        else if (IS_OPTION(argv[i], "-sidMultiSubTest")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <id> after -sidMultiSubTest\n");
                return false;
            }
            _SubID = strtol(argv[i], NULL, 10);
            if (_SubID < 0)
            {
                fprintf(stderr, "Bad id for subscriber\n");
                return false;
            }
        } 
        else if (IS_OPTION(argv[i], "-pidMultiPubTest")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <id> after -pidMultiPubTest\n");
                return false;
            }
            _PubID = strtol(argv[i], NULL, 10);
            if (_PubID < 0)
            {
                fprintf(stderr, "Bad id for publisher\n");
                return false;
            }
        } 
        else if (IS_OPTION(argv[i], "-numIter")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') {
                fprintf(stderr, "Missing <iter> after -numIter\n");
                return false;
            }
            _NumIter = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-dataLen")) 
        {
            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr, "Missing <length> after -dataLen\n");
                return false;
            }

            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            _DataLen = strtol(argv[i], NULL, 10);

            if (_DataLen < OVERHEAD_BYTES)
            {
                fprintf(stderr, "-dataLen must be >= %d\n", OVERHEAD_BYTES);
                return false;
            }
            if (_DataLen > TestMessage::MAX_DATA_SIZE)
            {
                fprintf(stderr,"-dataLen must be <= %d\n", TestMessage::MAX_DATA_SIZE);
                return false;
            }
        } 
        else if (IS_OPTION(argv[i], "-spin")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr,"Missing <count> after -spin\n");
                return false;
            }
            _SpinLoopCount = strtol(argv[i], NULL, 10);
        } 
        else if (IS_OPTION(argv[i], "-sleep")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr,"Missing <millisec> after -sleep\n");
                return false;
            }
            _SleepMillisec = strtol(argv[i], NULL, 10);
        } 
        else if (IS_OPTION(argv[i], "-latencyCount")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr,"Missing <count> after -latencyCount\n");
                return false;
            }
            _LatencyCount = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-numSubscribers")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr, "Missing <count> after -numSubscribers\n");
                return false;
            }
            _NumSubscribers = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-numPublishers")) 
        {
            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr, "Missing <count> after -numPublishers\n");
                return false;
            }
            _NumPublishers = strtol(argv[i], NULL, 10);
        }
        else if (IS_OPTION(argv[i], "-scan")) 
        {
            _IsScan = true;
        }
        else if (IS_OPTION(argv[i], "-noPrintIntervals") )
        {
            _PrintIntervals = false;
        }
        else if (IS_OPTION(argv[i], "-useReadThread") )
        {
            _UseReadThread = true;
        }
        else if (IS_OPTION(argv[i], "-bestEffort")) 
        {
            _IsReliable = false;

            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;
        }
	else if (IS_OPTION(argv[i], "-latencyTest")) 
        {
            _LatencyTest = true;

            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;
        }
        else if (IS_OPTION(argv[i], "-instances")) 
        {
            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            if ((i == (argc-1)) || *argv[++i] == '-') 
            {
                fprintf(stderr, "Missing <count> after -instances\n");
                return false;
            }

            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

            _InstanceCount = strtol(argv[i], NULL, 10);

            if (_InstanceCount <= 0) 
            {
                fprintf(stderr, "instance count cannot be negative or null\n");
                return false;
            }
        }
        else if (IS_OPTION(argv[i], "-debug")) 
        {
            _IsDebug = true;

            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;

        } else {          
            _MessagingArgv[_MessagingArgc] = StringDup(argv[i]);

            if (_MessagingArgv[_MessagingArgc] == NULL) {
                fprintf(stderr, "Problem allocating memory\n");
                return false;
            }

            _MessagingArgc++;
        }
    }


    if(_LatencyTest) {
	if(_PubID != 0) {
	    fprintf(stderr, "Only the publisher with ID = 0 can run the latency test\n");
	    return false;
	}

	// With latency test, latency should be 1
	if(_LatencyCount == -1) {
	    _LatencyCount = 1;
	}
    }

    if(_LatencyCount == -1) {
	_LatencyCount = 10000;
    }

    if (_IsScan && _NumPublishers > 1) {
        fprintf(stderr,"_IsScan is not supported with more than one publisher\n");
        return false;
    }

    if ((_NumIter > 0) && (_NumIter < _LatencyCount)) {
        fprintf(stderr, "numIter (%d) must be greater than latencyCount (%d).\n",
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
class ThroughputListener : public IMessagingCB
{
  public:

    unsigned long long packets_received;
    unsigned long long bytes_received;
    unsigned long long missing_packets;
    int                last_data_length;

    // store info for the last data set
    int                interval_data_length;
    unsigned long long interval_packets_received;
    unsigned long long interval_bytes_received;
    unsigned long long interval_missing_packets;
    unsigned long long interval_time, begin_time;

    IMessagingWriter *_writer;
    IMessagingReader *_reader;
    unsigned long * _last_seq_num;

    int _finish_count;
    int _num_publishers;

  public: 

    ThroughputListener(IMessagingWriter *writer, IMessagingReader *reader = NULL, int numPublishers = 1)
    {
        packets_received = 0;
        bytes_received = 0;
        missing_packets = 0;
        end_test = false;
        last_data_length = -1;
        interval_data_length = -1;
        interval_packets_received = 0;
        interval_bytes_received = 0;
        interval_missing_packets = 0;
        interval_time = 0;
        begin_time = 0;
        _writer = writer;
        _reader = reader;
        _finish_count = 0;
        _last_seq_num = new unsigned long[numPublishers];

        for (int i=0; i<numPublishers; i++) {
            _last_seq_num[i] = 0;
        }

        _num_publishers = numPublishers;
    }   

    virtual ~ThroughputListener() {
        if (_last_seq_num != NULL) {
            delete []_last_seq_num;
        }
    }
        
    virtual void ProcessMessage(TestMessage &message) 
    {
        // Check for test initialization messages
        if (message.size == perftest_cpp::INITIALIZE_SIZE)
        {
            _writer->Send(message);
            _writer->Flush();
            return;
        } 
        else if (message.size == perftest_cpp::FINISHED_SIZE)
        {
            // only respond to publisher id 0
            if (message.entity_id != 0) 
            {
                return;
            }

            if (end_test == true) 
            {
                return;
            }

            _finish_count ++;

            if (_finish_count >= _num_publishers) 
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

                // store the info for this interval
                unsigned long long now = perftest_cpp::GetTimeUsec();
                interval_time = now - begin_time;
                interval_packets_received = packets_received;
                interval_bytes_received = bytes_received;
                interval_missing_packets = missing_packets;
                interval_data_length = last_data_length;
                end_test = true;
            }

            _writer->Send(message);
            _writer->Flush();

            if (_finish_count >= _num_publishers) 
            {
                printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  " 
                       "Mbps(ave): %7.1lf  Lost: %llu\n",
                       interval_data_length + perftest_cpp::OVERHEAD_BYTES,
                       interval_packets_received,
                       interval_packets_received*1000000/interval_time,
                       interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                       interval_missing_packets);
                fflush(stdout);
            }
            return;
        }
        
        // Send back a packet if this is a ping
        if (message.latency_ping == perftest_cpp::_SubID)
        {
            _writer->Send(message);
            _writer->Flush();
        }
                               
        // Always check if need to reset internals
        if (message.size == perftest_cpp::LENGTH_CHANGED_SIZE)
        {
            unsigned long long now;
            // store the info for this interval
            now = perftest_cpp::GetTimeUsec();

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

                printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  " 
                       "Mbps(ave): %7.1lf  Lost: %llu\n",
                       interval_data_length + perftest_cpp::OVERHEAD_BYTES,
                       interval_packets_received,
                       interval_packets_received*1000000/interval_time,
                       interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                       interval_missing_packets);
                fflush(stdout);
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

            begin_time =  perftest_cpp::GetTimeUsec();

            if (perftest_cpp::_PrintIntervals) {
                printf("\n\n********** New data length is %d\n",
                       message.size + perftest_cpp::OVERHEAD_BYTES);
                fflush(stdout);
            }
        }

        last_data_length = message.size;
        ++packets_received;
        bytes_received += (unsigned long long) (message.size + perftest_cpp::OVERHEAD_BYTES);
        
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
};
    
/*********************************************************
 * Used for receiving data using a thread instead of callback
 *
 */
static void *ThroughputReadThread(void *arg) 
{
    ThroughputListener *listener = (ThroughputListener *) arg;
    TestMessage *message;

    while (!listener->end_test)
    {
        // Receive message should block until a message is received
        message = listener->_reader->ReceiveMessage();

        if (message != NULL)
        {
            listener->ProcessMessage(*message);
        }
    }

    return NULL;
}

/*********************************************************
 * Subscriber
 */
int perftest_cpp::Subscriber()
{
    ThroughputListener *reader_listener = NULL;
    IMessagingReader   *reader;
    IMessagingWriter   *writer;
    IMessagingWriter   *announcement_writer;

    // create latency pong writer
    writer = _MessagingImpl->CreateWriter(_LatencyTopicName);

    if (writer == NULL) {
        fprintf(stderr, "Problem creating latency writer.\n");
        return -1;
    }

    // Check if using callbacks or read thread
    if (!_UseReadThread)
    {
        // create latency pong reader
        reader_listener = new ThroughputListener(writer, NULL, _NumPublishers);
        reader = _MessagingImpl->CreateReader(_ThroughputTopicName, reader_listener);
        if (reader == NULL)
        {
            fprintf(stderr, "Problem creating throughput reader.\n");
            return -1;
        }
    }
    else
    {
        reader = _MessagingImpl->CreateReader(_ThroughputTopicName, NULL);
        if (reader == NULL)
        {
            fprintf(stderr, "Problem creating throughput reader.\n");
            return -1;
        }
        reader_listener = new ThroughputListener(writer, reader, _NumPublishers);

        RTIOsapiThread_new("ReceiverThread",  
                            RTI_OSAPI_THREAD_PRIORITY_DEFAULT, 
                            RTI_OSAPI_THREAD_OPTION_DEFAULT, 
                            RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,                            
                            NULL,
                            ThroughputReadThread,
                            reader_listener);
    }

    // Create announcement writer
    announcement_writer = _MessagingImpl->CreateWriter(_AnnouncementTopicName);

    if (announcement_writer == NULL) {
        fprintf(stderr, "Problem creating announcement writer.\n");
        return -1;
    }

    // Synchronize with publishers
    fprintf(stderr,"Waiting to discover %d publishers ...\n", _NumPublishers);
    fflush(stderr);
    reader->WaitForWriters(_NumPublishers);
    announcement_writer->WaitForReaders(_NumPublishers);

    // Send announcement message
    TestMessage message;
    message.entity_id = _SubID;
    message.data = "";
    message.size = 0;
    announcement_writer->Send(message);
    announcement_writer->Flush();


    fprintf(stderr,"Waiting for data...\n");
    fflush(stderr);
    
    // wait for data
    unsigned long long prev_time = 0, now = 0, delta = 0;
    unsigned long long prev_count = 0;
    unsigned long long prev_bytes = 0;
    unsigned long long ave_count = 0;
    int last_data_length = -1;
    unsigned long long mps = 0, bps = 0;
    double mps_ave = 0.0, bps_ave = 0.0;
    unsigned long long msgsent, bytes, last_msgs, last_bytes;

    now = GetTimeUsec();

    while (true) { 
        prev_time = now;
        MilliSleep(1000);
        now = GetTimeUsec();

        if (reader_listener->end_test)
        {
            break;
        }

        if (_PrintIntervals)
        {
            if (last_data_length != reader_listener->last_data_length)
            {
                last_data_length = reader_listener->last_data_length;
                prev_count = reader_listener->packets_received;
                prev_bytes = reader_listener->bytes_received;
                bps_ave = 0;
                mps_ave = 0;
                ave_count = 0;
                continue;
            }

            last_msgs = reader_listener->packets_received;
            last_bytes = reader_listener->bytes_received;
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

            if (last_msgs > 0) {
                printf("Packets: %8llu  Packets/s: %7llu  Packets/s(ave): %7.0lf  " 
                       "Mbps: %7.1lf  Mbps(ave): %7.1lf  Lost: %llu\n",
                        last_msgs, mps, mps_ave,
                        bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                        reader_listener->missing_packets);
                fflush(stdout);
            }
        }
    }

    if (_UseReadThread) 
    {
        reader->Shutdown();
    }

    perftest_cpp::MilliSleep(1000);
    fprintf(stderr,"Finishing test...\n");
    fflush(stderr);

    return 0;
}

/*********************************************************
 * Data listener for the Announcement
 *
 * Receives an announcement message from a Subscriber once
 * the subscriber has discovered every Publisher.
 */
class AnnouncementListener : public IMessagingCB 
{
  public:
    int announced_subscribers;
  public:
    AnnouncementListener() {
        announced_subscribers = 0;
    }

    virtual void ProcessMessage(TestMessage& /*message*/)
    {
        announced_subscribers++;
    }
};

/*********************************************************
 * Data listener for the Publisher side.
 *
 * Receives latency ping from Subscriber and does
 * round trip latency calculations
 */
class LatencyListener : public IMessagingCB 
{
  private:

    unsigned long long latency_sum ;
    unsigned long long latency_sum_square;
    unsigned long long count;
    unsigned long      latency_min;
    unsigned long      latency_max;
    int                last_data_length;
    unsigned long     *_latency_history;
    unsigned long      clock_skew_count;
    unsigned int       _num_latency;
    IMessagingWriter *_writer;
 public:
    bool               end_test;
    IMessagingReader *_reader;

  public:

    LatencyListener(unsigned int num_latency, IMessagingReader *reader,
		    IMessagingWriter *writer)
    {
        latency_sum = 0;
        latency_sum_square = 0;
        count = 0;
        latency_min = 0;
        latency_max = 0;
        last_data_length = 0;
        clock_skew_count = 0;

        if (num_latency > 0)
        {
            _latency_history = new unsigned long[num_latency];
            _num_latency = num_latency;
        } else {
            _latency_history = NULL;
            _num_latency = 0;
        }
        
        end_test = false;
        _reader = reader;
	_writer = writer;
    }

    virtual void ProcessMessage(TestMessage &message)
    {
        unsigned long long now, sentTime;
        unsigned long latency;
        int sec;
        unsigned int usec;
        double latency_ave;
        double latency_std;

        now = perftest_cpp::GetTimeUsec();

        switch (message.size)
        {
            // Initializing message, don't process
            case perftest_cpp::INITIALIZE_SIZE:
                return;

            // Test finished message
            case perftest_cpp::FINISHED_SIZE:
                // may get this message multiple times for 1 to N tests
                if (end_test == true)
                {
                    return;
                }

                end_test = true;
                // fall through...

            // Data length is changing size
            case perftest_cpp::LENGTH_CHANGED_SIZE:

                // will get a LENGTH_CHANGED message on startup before any data
                if (count == 0)
                {
                    return;
                }

                if (clock_skew_count != 0) {
                    fprintf(stderr,"The following latency result may not be accurate because clock skew happens %lu times\n",
                                clock_skew_count);
                    fflush(stderr);
                }

                // sort the array (in ascending order)
                std::sort(_latency_history, _latency_history+count);
                latency_ave = (double)latency_sum / count;
                latency_std = sqrt((double)latency_sum_square / (double)count - (latency_ave * latency_ave));
                printf("Length: %5d  Latency: Ave %6.0lf us  Std %6.1lf us  "
                       "Min %6lu us  Max %6lu us 50%% %6lu us 90%% %6lu us  99%% %6lu us 99.99%% %6lu us\n",
                       last_data_length + perftest_cpp::OVERHEAD_BYTES,
                       latency_ave, latency_std, latency_min, latency_max,
                       _latency_history[count*50/100],
                       _latency_history[count*90/100],
                       _latency_history[count*99/100],
                       _latency_history[(int)(count*(9999.0/10000))]);
                fflush(stdout);
                latency_sum = 0;
                latency_sum_square = 0;
                latency_min = 0;
                latency_max = 0;
                count = 0;
                clock_skew_count = 0;

                return;

            default:
                break;
        }

        if (last_data_length != message.size)
        {
            latency_sum = 0;
            latency_sum_square = 0;
            latency_min = 0;
            latency_max = 0;
            count = 0;
        }

        sec = message.timestamp_sec;
        usec = message.timestamp_usec;
        sentTime = ((unsigned long long)sec << 32) | (unsigned long long)usec;

        if (now >= sentTime) 
        {
            latency = (unsigned long)(now - sentTime);

            // keep track of one-way latency
            latency /= 2;
        } 
        else 
        {
            fprintf(stderr,"Clock skew suspected: received time %llu usec, sent time %llu usec",
                            now, sentTime);
                ++clock_skew_count;
            return;
        }

        // store value for percentile calculations
        if (_latency_history != NULL)
        {
            if (count >= _num_latency)
            {
                fprintf(stderr,"Too many latency pongs received.  Do you have more than 1 app with -pidMultiPubTest = 0 or -sidMultiSubTest 0?\n");
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
        latency_sum_square += ((unsigned long long)latency * (unsigned long long)latency);

        // if data sized changed, print out stats and zero counters
        if (last_data_length != message.size)
        {
            last_data_length = message.size;

            if (perftest_cpp::_PrintIntervals)
            {
                printf("\n\n********** New data length is %d\n",
                       last_data_length + perftest_cpp::OVERHEAD_BYTES);
            }
        }
        else
        {
            if (perftest_cpp::_PrintIntervals)
            {
                latency_ave = (double)latency_sum / (double)count;
                latency_std = sqrt(
                    (double)latency_sum_square / (double)count - (latency_ave * latency_ave));

                printf("One way Latency: %6lu us  Ave %6.0lf us  Std %6.1lf us  Min %6lu us  Max %6lu\n",
                       latency, latency_ave, latency_std, latency_min, latency_max);
            }
        }

	if(_writer != NULL) {
	   _writer->notifyPingResponse();
	}
    }
};

/*********************************************************
 * Used for receiving data using a thread instead of callback
 *
 */
static void *LatencyReadThread(void *arg) 
{
    LatencyListener *listener = (LatencyListener *) arg;
    TestMessage *message;

    while (!listener->end_test)
    {
        // Receive message should block until a message is received
        message = listener->_reader->ReceiveMessage();

        if (message != NULL)
        {
            listener->ProcessMessage(*message);
        }

	/*
	 * TODO:
	 *
	 * To support -latencyTest plus -useReadThread we need to signal 
	 * --HERE-- the internal semaphore used in RTIDDSImpl.cxx as
	 * we now do in the listener on_data_available callback 
	 * inside RTIDDSImpl.cxx
	 *
	 */
    }

    return NULL;
}

/*********************************************************
 * Publisher
 */
int perftest_cpp::Publisher()
{
    LatencyListener  *reader_listener = NULL;
    IMessagingReader *reader;
    IMessagingWriter *writer;
    AnnouncementListener  *announcement_reader_listener = NULL;
    IMessagingReader *announcement_reader;
    unsigned long num_latency;
    int initializeSampleCount = 50;

    // create throughput/ping writer
    writer = _MessagingImpl->CreateWriter(_ThroughputTopicName);

    if (writer == NULL)
    {
        fprintf(stderr,"Problem creating throughput writer.\n");
        return -1;
    }

    // calculate number of latency pings that will be sent per data size
    if (_IsScan)
    {
        num_latency = NUM_LATENCY_PINGS_PER_DATA_SIZE;
    }
    else
    {
        num_latency = (unsigned long)((_NumIter/_SamplesPerBatch) / _LatencyCount);
    }

    if (_SamplesPerBatch > 1) {
        // in batch mode, might have to send another ping
        ++num_latency;
    }

    // Only publisher with ID 0 will send/receive pings
    if (_PubID == 0) 
    {
        // Check if using callbacks or read thread
        if (!_UseReadThread)
        {
            // create latency pong reader
	    // the writer is passed for ping-pong notification in LatencyTest
            reader_listener = new LatencyListener(num_latency, NULL,
		_LatencyTest ? writer : NULL);
            reader = _MessagingImpl->CreateReader(_LatencyTopicName, reader_listener);
            if (reader == NULL)
            {
                fprintf(stderr,"Problem creating latency reader.\n");
                return -1;
            }
        }
        else
        {
            reader = _MessagingImpl->CreateReader(_LatencyTopicName, NULL);
            if (reader == NULL)
            {
                fprintf(stderr,"Problem creating latency reader.\n");
                return -1;
            }
            reader_listener = new LatencyListener(num_latency, reader, 
		_LatencyTest ? writer : NULL);

            RTIOsapiThread_new("ReceiverThread",  
                                RTI_OSAPI_THREAD_PRIORITY_DEFAULT, 
                                RTI_OSAPI_THREAD_OPTION_DEFAULT, 
                                RTI_OSAPI_THREAD_STACK_SIZE_DEFAULT,
                                NULL,
                                LatencyReadThread,
                                reader_listener);
        }
    } 
    else 
    {
        reader = NULL;
    }

    /* Create Announcement reader
     * A Subscriber will send a message on this channel once it discovers
     * every Publisher
     */
    announcement_reader_listener = new AnnouncementListener();
    announcement_reader = _MessagingImpl->CreateReader(_AnnouncementTopicName,
                                                        announcement_reader_listener);
    if (announcement_reader == NULL)
    {
        fprintf(stderr,"Problem creating announcement reader.\n");
        return -1;
    }
            
    fprintf(stderr,"Waiting to discover %d subscribers...\n", _NumSubscribers);
    fflush(stderr);
    writer->WaitForReaders(_NumSubscribers);

    // We have to wait until every Subscriber sends an announcement message
    // indicating that it has discovered every Publisher
    fprintf(stderr,"Waiting for subscribers announcement ...\n");
    fflush(stderr);
    while (_NumSubscribers > announcement_reader_listener->announced_subscribers) {
        MilliSleep(1000);
    }

    // Allocate data and set size
    TestMessage message;
    message.entity_id = _PubID;
    message.data = new char[TestMessage::MAX_DATA_SIZE];

    fprintf(stderr,"Publishing data...\n");
    fflush(stderr);

    // initialize data pathways by sending some initial pings
    if (_InstanceCount > initializeSampleCount) {
        initializeSampleCount = _InstanceCount;
    }

    if (INITIALIZE_SIZE > MAX_BINDATA_SIZE) {
        fprintf(stderr,"Error: INITIALIZE_SIZE > MAX_BINDATA_SIZE\n");
        return -1;
    }

    message.size = INITIALIZE_SIZE;
    for (int i = 0; i < initializeSampleCount; i++)
    {
        // Send test initialization message
        writer->Send(message);
    }
    writer->Flush();

    // Set data size, account for other bytes in message
    message.size = _DataLen - OVERHEAD_BYTES;

    // Sleep 1 second, then begin test
    MilliSleep(1000);

    int num_pings = 0;
    int scan_number = 4; // 4 means start sending with dataLen = 2^5 = 32
    bool last_scan = false;
    int pingID = -1;
    int current_index_in_batch = 0;
    int ping_index_in_batch = 0;
    bool sentPing = false;
    double ad, bd, cd;
    volatile double * a, * b, * c;

    a = &ad;
    b = &bd;
    c = &cd;
    
    /********************
     *  Main sending loop
     */
    for ( unsigned long long loop=0; (_NumIter == 0)||(loop<(unsigned long long)_NumIter); ++loop ) 
    {      
        if ( _SleepMillisec > 0 ) {
            MilliSleep(_SleepMillisec);
        }
        
        if ( _SpinLoopCount > 0 ) {
            // spin, spin, spin
            for (int m=0; m<_SpinLoopCount; ++m) {
                *a = 1.1;
                *b = 3.1415;
                *c = *a/(*b)*m;
            }                    
        }

        pingID = -1;

        // only send latency pings if is publisher with ID 0
        // In batch mode, latency pings are sent once every LatencyCount batches
        if ( (_PubID == 0) && (((loop/_SamplesPerBatch) % (unsigned long long)_LatencyCount) == 0) )
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
            if ( current_index_in_batch == ping_index_in_batch  && !sentPing ) 
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
                        writer->Flush();

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

                        if(new_size > _maxBinDataSize) {
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
                                new_size = TestMessage::MAX_DATA_SIZE - OVERHEAD_BYTES;
                                _LatencyCount /= 2;
                             break;

                            case 9:
                            case 11:
                            case 13:
                            case 14:
                            case 15:
                                _LatencyCount /= 2;
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
                            // sleep to allow packet to be pinged back
                            writer->Send(message);
                            writer->Flush();
                        }

                        // sleep to allow packet to be pinged back
                        MilliSleep(1000);

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
                unsigned long long now = GetTimeUsec();
                message.timestamp_sec = (int)((now >> 32) & 0xFFFFFFFF);
                message.timestamp_usec = (unsigned int)(now & 0xFFFFFFFF);
                
                ++num_pings;
                ping_index_in_batch = (ping_index_in_batch + 1) % _SamplesPerBatch;
                sentPing = true;
            }
        } 
        current_index_in_batch = (current_index_in_batch + 1) % _SamplesPerBatch;

        message.seq_num = (unsigned long) loop;
        message.latency_ping = pingID;
        writer->Send(message);
	if(_LatencyTest && sentPing) {
            if (_IsReliable) {
                writer->waitForPingResponse();
            } 
            else {
                /* time out in milliseconds */
                writer->waitForPingResponse(200);
            }	    
	}

        // come to the beginning of another batch
        if (current_index_in_batch == 0)
        {
            sentPing = false;
        }
    }

    // In case of batching, flush
    writer->Flush();

    // Test has finished, send end of test message, send multiple
    // times in case of best effort
    if (FINISHED_SIZE > MAX_BINDATA_SIZE) {
        fprintf(stderr,"Error: FINISHED_SIZE < MAX_BINDATA_SIZE\n");
        return -1;
    }
    message.size = FINISHED_SIZE;
    for (int j = 0; j<30; ++j)
    {
        writer->Send(message);
        writer->Flush();
    }

    if (_UseReadThread && reader) 
    {
        reader->Shutdown();
    }

    perftest_cpp::MilliSleep(1000);
    fprintf(stderr,"Finishing test...\n");
    fflush(stderr);

    return 0;
}


/*********************************************************
 * Utility functions
 */
inline unsigned long long perftest_cpp::GetTimeUsec() 
{
#ifdef RTI_WIN32
    LARGE_INTEGER current_count = { 0, 0 };
    QueryPerformanceCounter(&current_count);

    return (unsigned long long) ((current_count.QuadPart * 1000000 / _ClockFrequency.QuadPart));
#else
    struct timeval now;
    gettimeofday(&now, NULL);
    return ((unsigned long long) now.tv_sec * 1000000) + now.tv_usec;
#endif
}


char * perftest_cpp::StringDup(const char * str) 
{
    char * result = NULL;

    result = new char[strlen(str) + 1];

    if (result != NULL) {
        strcpy(result, str);
    }

    return result;
}
