/*
 * (c) 2005-2018  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "ParameterManager.h"
#include "perftest_cpp.h"

ParameterManager::ParameterManager()
{
}

ParameterManager &ParameterManager::GetInstance()
{
    static ParameterManager instance;
    return instance;
}

void ParameterManager::initialize()
{
    // GENERAL PARAMETER
    Parameter<bool> *bestEffort = new Parameter<bool>(false);
    bestEffort->set_command_line_argument(std::make_pair("-bestEffort",""));
    bestEffort->set_description("Run test in best effort mode. Default: reliable");
    bestEffort->set_type(T_BOOL);
    bestEffort->set_extra_argument(NO);
    bestEffort->set_group(GENERAL);
    parameterList["bestEffort"] = AnyParameter(bestEffort);

    Parameter<unsigned long long> *dataLen = new Parameter<unsigned long long>(100);
    dataLen->set_command_line_argument(std::make_pair("-dataLen","<bytes>"));
    dataLen->set_description("Set length of payload for each send.\nDefault: 100");
    dataLen->set_type(T_NUMERIC);
    dataLen->set_extra_argument(YES);
    dataLen->set_range(perftest_cpp::OVERHEAD_BYTES, MAX_PERFTEST_SAMPLE_SIZE);
    dataLen->set_group(GENERAL);
    parameterList["dataLen"] = AnyParameter(dataLen);

    Parameter<unsigned long long> *verbosity = new Parameter<unsigned long long>(1);
    verbosity->set_command_line_argument(std::make_pair("-verbosity","<level>"));
    verbosity->set_description("Run with different levels of verbosity:\n0 - SILENT, 1 - ERROR, 2 - WARNING, 3 - ALL.\nDefault: 1");
    verbosity->set_type(T_NUMERIC);
    verbosity->set_extra_argument(YES);
    verbosity->set_range(0, 3);
    verbosity->set_group(GENERAL);
    parameterList["verbosity"] = AnyParameter(verbosity);

    Parameter<bool> *dynamicData = new Parameter<bool>(false);
    dynamicData->set_command_line_argument(std::make_pair("-dynamicData",""));
    dynamicData->set_description("Makes use of the Dynamic Data APIs instead\nof using the generated types");
    dynamicData->set_type(T_BOOL);
    dynamicData->set_extra_argument(NO);
    dynamicData->set_group(GENERAL);
    parameterList["dynamicData"] = AnyParameter(dynamicData);

    Parameter<unsigned long long> *durability = new Parameter<unsigned long long>(DDS_VOLATILE_DURABILITY_QOS);
    durability->set_command_line_argument(std::make_pair("-durability","<0|1|2|3>"));
    durability->set_description("Set durability QOS: 0 - volatile,\n1 - transient local, 2 - transient,\n3 - persistent. Default: 0");
    durability->set_type(T_NUMERIC);
    durability->set_extra_argument(YES);
    durability->set_group(GENERAL);
    durability->set_range(0, 3);
    parameterList["durability"] = AnyParameter(durability);

    Parameter<unsigned long long> *domain = new Parameter<unsigned long long>(1);
    domain->set_command_line_argument(std::make_pair("-domain","<id>"));
    domain->set_description("RTI DDS Domain. Default: 1");
    domain->set_type(T_NUMERIC);
    domain->set_extra_argument(YES);
    domain->set_range(0, 250);
    domain->set_group(GENERAL);
    parameterList["domain"] = AnyParameter(domain);

    Parameter<unsigned long long> *instances = new Parameter<unsigned long long>(1);
    instances->set_command_line_argument(std::make_pair("-instances","<count>"));
    instances->set_description("Set the number of instances (keys) to iterate\nover when publishing. Default: 1");
    instances->set_type(T_NUMERIC);
    instances->set_extra_argument(YES);
    instances->set_range(0, ULLONG_MAX);
    instances->set_group(GENERAL);
    parameterList["instances"] = AnyParameter(instances);

    Parameter<bool> *keyed = new Parameter<bool>(false);
    keyed->set_command_line_argument(std::make_pair("-keyed",""));
    keyed->set_description("Use keyed data. Default: unkeyed");
    keyed->set_type(T_BOOL);
    keyed->set_extra_argument(NO);
    keyed->set_group(GENERAL);
    parameterList["keyed"] = AnyParameter(keyed);

    Parameter<bool> *noDirectCommunication = new Parameter<bool>(false);
    noDirectCommunication->set_command_line_argument(std::make_pair("-noDirectCommunication",""));
    noDirectCommunication->set_description("Use brokered mode for persistent durability");
    noDirectCommunication->set_type(T_BOOL);
    noDirectCommunication->set_extra_argument(NO);
    noDirectCommunication->set_group(GENERAL);
    parameterList["noDirectCommunication"] = AnyParameter(noDirectCommunication);

    Parameter<bool> *noPositiveAcks = new Parameter<bool>(false);
    noPositiveAcks->set_command_line_argument(std::make_pair("-noPositiveAcks",""));
    noPositiveAcks->set_description("Disable use of positive acks in reliable \nprotocol. Default use positive acks");
    noPositiveAcks->set_type(T_BOOL);
    noPositiveAcks->set_extra_argument(NO);
    noPositiveAcks->set_group(GENERAL);
    parameterList["noPositiveAcks"] = AnyParameter(noPositiveAcks);

    Parameter<bool> *noPrintIntervals = new Parameter<bool>(false);
    noPrintIntervals->set_command_line_argument(std::make_pair("-noPrintIntervals",""));
    noPrintIntervals->set_description("Don't print statistics at intervals during\ntest");
    noPrintIntervals->set_type(T_BOOL);
    noPrintIntervals->set_extra_argument(NO);
    noPrintIntervals->set_group(GENERAL);
    parameterList["noPrintIntervals"] = AnyParameter(noPrintIntervals);

    Parameter<std::string> *qosFile = new Parameter<std::string>("perftest_qos_profiles.xml");
    qosFile->set_command_line_argument(std::make_pair("-qosFile","<filename>"));
    qosFile->set_description("Name of XML file for DDS Qos profiles.\nDefault: perftest_qos_profiles.xml");
    qosFile->set_type(T_STR);
    qosFile->set_extra_argument(YES);
    qosFile->set_group(GENERAL);
    parameterList["qosFile"] = AnyParameter(qosFile);

    Parameter<std::string> *qosLibrary = new Parameter<std::string>("PerftestQosLibrary");
    qosLibrary->set_command_line_argument(std::make_pair("-qosLibrary","<lib name>"));
    qosLibrary->set_description("Name of QoS Library for DDS Qos profiles.\nDefault: PerftestQosLibrary");
    qosLibrary->set_type(T_STR);
    qosLibrary->set_extra_argument(YES);
    qosLibrary->set_group(GENERAL);
    parameterList["qosLibrary"] = AnyParameter(qosLibrary);

    Parameter<bool> *noXmlQos = new Parameter<bool>(false);
    noXmlQos->set_command_line_argument(std::make_pair("-noXmlQos",""));
    noXmlQos->set_description("Skip loading the qos profiles from the xml\nprofile");
    noXmlQos->set_type(T_BOOL);
    noXmlQos->set_extra_argument(NO);
    noXmlQos->set_group(GENERAL);
    parameterList["noXmlQos"] = AnyParameter(noXmlQos);

    Parameter<bool> *useReadThread = new Parameter<bool>(false);
    useReadThread->set_command_line_argument(std::make_pair("-useReadThread",""));
    useReadThread->set_description("Use separate thread instead of callback to\nread data");
    useReadThread->set_type(T_BOOL);
    useReadThread->set_extra_argument(NO);
    useReadThread->set_group(GENERAL);
    parameterList["useReadThread"] = AnyParameter(useReadThread);

    Parameter<unsigned long long> *waitsetDelayUsec = new Parameter<unsigned long long>(100);
    waitsetDelayUsec->set_command_line_argument(std::make_pair("-waitsetDelayUsec","<usec>"));
    waitsetDelayUsec->set_description("UseReadThread related. Allows you to\nprocess incoming data in groups, based on the\ntime rather than individually. It can be used\ncombined with -waitsetEventCount.\nDefault: 100 usec");
    waitsetDelayUsec->set_type(T_NUMERIC);
    waitsetDelayUsec->set_extra_argument(YES);
    waitsetDelayUsec->set_group(GENERAL);
    waitsetDelayUsec->set_range(0, UINT_MAX);
    parameterList["waitsetDelayUsec"] = AnyParameter(waitsetDelayUsec);

    Parameter<unsigned long long> *waitsetEventCount = new Parameter<unsigned long long>(5);
    waitsetEventCount->set_command_line_argument(std::make_pair("-waitsetEventCount","<count>"));
    waitsetEventCount->set_description("UseReadThread related. Allows you to\nprocess incoming data in groups, based on the\nnumber of samples rather than individually. It\ncan be used combined with -waitsetDelayUsec.\nDefault: 5");
    waitsetEventCount->set_type(T_NUMERIC);
    waitsetEventCount->set_extra_argument(YES);
    waitsetEventCount->set_group(GENERAL);
    waitsetEventCount->set_range(1, ULLONG_MAX);
    parameterList["waitsetEventCount"] = AnyParameter(waitsetEventCount);

    Parameter<bool> *asynchronous = new Parameter<bool>(false);
    asynchronous->set_command_line_argument(std::make_pair("-asynchronous",""));
    asynchronous->set_description("Use asynchronous writer.\nDefault: Not set");
    asynchronous->set_type(T_BOOL);
    asynchronous->set_extra_argument(NO);
    asynchronous->set_group(GENERAL);
    parameterList["asynchronous"] = AnyParameter(asynchronous);

    Parameter<std::string> *flowController = new Parameter<std::string>("default");
    flowController->set_command_line_argument(std::make_pair("-flowController", "<flow>"));
    flowController->set_description("In the case asynchronous writer use a specific flow controller.\nThere are several flow controller predefined:\n\t{'default', '1Gbps', '10Gbps'}\nDefault: \"default\" (If using asynchronous)");
    flowController->set_type(T_STR);
    flowController->set_extra_argument(YES);
    flowController->add_valid_str_value("default");
    flowController->add_valid_str_value("1Gbps");
    flowController->add_valid_str_value("10Gbps");
    flowController->set_group(GENERAL);
    parameterList["flowController"] = AnyParameter(flowController);

    Parameter<bool> *cpu = new Parameter<bool>(false);
    cpu->set_command_line_argument(std::make_pair("-cpu",""));
    cpu->set_description("Display the cpu percent use by the process\nDefault: Not set");
    cpu->set_type(T_BOOL);
    cpu->set_extra_argument(NO);
    cpu->set_group(GENERAL);
    parameterList["cpu"] = AnyParameter(cpu);

    Parameter<unsigned long long> *unbounded = new Parameter<unsigned long long>(0);
    unbounded->set_command_line_argument(std::make_pair("-unbounded","<allocation_threshold>"));
    unbounded->set_description("Use unbounded Sequences\n<allocation_threshold> is optional. Default: 63000 Bytes");
    unbounded->set_type(T_NUMERIC);
    unbounded->set_extra_argument(OPTIONAL);
    unbounded->set_range(perftest_cpp::OVERHEAD_BYTES, MAX_BOUNDED_SEQ_SIZE);
    unbounded->set_group(GENERAL);
    parameterList["unbounded"] = AnyParameter(unbounded);

    ////////////////////////////////////////////////////////////////////////////
    //PUBLISHER PARAMETER

    Parameter<unsigned long long> *batching = new Parameter<unsigned long long>(DEFAULT_THROUGHPUT_BATCH_SIZE);
    batching->set_command_line_argument(std::make_pair("-batchsize","<bytes>"));
    batching->set_description("Size in bytes of batched message. Default: 8kB.\n(Disabled for LatencyTest mode or if dataLen > 4kB)");
    batching->set_type(T_NUMERIC);
    batching->set_extra_argument(YES);
    batching->set_range(0, MAX_SYNCHRONOUS_SIZE);
    batching->set_group(PUB);
    parameterList["batching"] = AnyParameter(batching);

    Parameter<bool> *enableAutoThrottle = new Parameter<bool>(false);
    enableAutoThrottle->set_command_line_argument(std::make_pair("-enableAutoThrottle",""));
    enableAutoThrottle->set_description("Enables the AutoThrottling feature in the\nthroughput DataWriter (pub)");
    enableAutoThrottle->set_type(T_BOOL);
    enableAutoThrottle->set_extra_argument(NO);
    enableAutoThrottle->set_group(PUB);
    parameterList["enableAutoThrottle"] = AnyParameter(enableAutoThrottle);

    Parameter<bool> *enableTurboMode = new Parameter<bool>(false);
    enableTurboMode->set_command_line_argument(std::make_pair("-enableTurboMode",""));
    enableTurboMode->set_description("Enables the TurboMode feature in the\nthroughput DataWriter (pub)");
    enableTurboMode->set_type(T_BOOL);
    enableTurboMode->set_extra_argument(NO);
    enableTurboMode->set_group(PUB);
    parameterList["enableTurboMode"] = AnyParameter(enableTurboMode);

    Parameter<bool> * pub = new Parameter<bool>(false);
    pub->set_command_line_argument(std::make_pair("-pub",""));
    pub->set_description("Set test to be a publisher");
    pub->set_type(T_BOOL);
    pub->set_extra_argument(NO);
    pub->set_group(PUB);
    parameterList["pub"] = AnyParameter(pub);

    Parameter<unsigned long long> *latencyCount = new Parameter<unsigned long long>(10000);
    latencyCount->set_command_line_argument(std::make_pair("-latencyCount","<count>"));
    latencyCount->set_description("Number of samples (or batches) to send before\na latency ping packet is sent. Default:\n10000 if -latencyTest is not specified,\n1 if -latencyTest is specified");
    latencyCount->set_type(T_NUMERIC);
    latencyCount->set_extra_argument(YES);
    latencyCount->set_range(0, ULLONG_MAX);
    latencyCount->set_group(PUB);
    parameterList["latencyCount"] = AnyParameter(latencyCount);

    Parameter<unsigned long long> *executionTime = new Parameter<unsigned long long>(0);
    executionTime->set_command_line_argument(std::make_pair("-executionTime","<sec>"));
    executionTime->set_description("Set a maximum duration for the test.\nThe first condition triggered will finish the\ntest: number of samples or execution time.\nDefault 0 (don't set execution time)");
    executionTime->set_type(T_NUMERIC);
    executionTime->set_extra_argument(YES);
    executionTime->set_range(1, ULLONG_MAX);
    executionTime->set_group(PUB);
    parameterList["executionTime"] = AnyParameter(executionTime);

    Parameter<bool> *latencyTest = new Parameter<bool>(false);
    latencyTest->set_command_line_argument(std::make_pair("-latencyTest",""));
    latencyTest->set_description("Run a latency test consisting of a ping-pong\nsynchronous communication");
    latencyTest->set_type(T_BOOL);
    latencyTest->set_extra_argument(NO);
    latencyTest->set_group(PUB);
    parameterList["latencyTest"] = AnyParameter(latencyTest);

    Parameter<unsigned long long> *numIter = new Parameter<unsigned long long>(100000000);
    numIter->set_command_line_argument(std::make_pair("-numIter","<count>"));
    numIter->set_description("Set number of messages to send. Default:\n100000000 for Throughput tests or 10000000\nfor Latency tests. See '-executionTime'");
    numIter->set_type(T_NUMERIC);
    numIter->set_extra_argument(YES);
    numIter->set_range(1, ULLONG_MAX);
    numIter->set_group(PUB);
    parameterList["numIter"] = AnyParameter(numIter);

    Parameter<unsigned long long> *numSubscribers = new Parameter<unsigned long long>(1);
    numSubscribers->set_command_line_argument(std::make_pair("-numSubscribers","<count>"));
    numSubscribers->set_description("Number of subscribers running in test.\nDefault: 1");
    numSubscribers->set_type(T_NUMERIC);
    numSubscribers->set_extra_argument(YES);
    numSubscribers->set_range(1, INT_MAX);
    numSubscribers->set_group(PUB);
    parameterList["numSubscribers"] = AnyParameter(numSubscribers);

    Parameter<unsigned long long> *pidMultiPubTest = new Parameter<unsigned long long>(0);
    pidMultiPubTest->set_command_line_argument(std::make_pair("-pidMultiPubTest","<bytes>"));
    pidMultiPubTest->set_description("Set id of the publisher in a multi-publisher test.\n Only publisher 0 sends \n latency pings. Default: 0");
    pidMultiPubTest->set_type(T_NUMERIC);
    pidMultiPubTest->set_extra_argument(YES);
    pidMultiPubTest->set_range(0, INT_MAX);
    pidMultiPubTest->set_group(PUB);
    parameterList["pidMultiPubTest"] = AnyParameter(pidMultiPubTest);

    ParameterVector<unsigned long long> *cft = new ParameterVector<unsigned long long>();
    cft->set_command_line_argument(std::make_pair("-cft","<start>:<end>"));
    cft->set_description("Use a Content Filtered Topic for the Throughput topic in the subscriber side.\nSpecify 2 parameters: <start> and <end> to receive samples with a key in that range.\nSpecify only 1 parameter to receive samples with that exact key.\nDefault: Not set");
    cft->set_type(T_VECTOR_NUMERIC);
    cft->set_extra_argument(YES);
    cft->set_range(0, MAX_CFT_VALUE);
    cft->setParseMethod(SPLIT);
    cft->set_group(PUB);
    parameterList["cft"] = AnyParameter(cft);

    ParameterPair<unsigned long long, std::string> *pubRate = new ParameterPair<unsigned long long, std::string>(0,"spin");
    pubRate->set_command_line_argument(std::make_pair("-pubRate","<samples/s>:<method>"));
    pubRate->set_description("Limit the throughput to the specified number\nof samples/s. Default 0 (don't limit)\n[OPTIONAL] Method to control the throughput can be:\n'spin' or 'sleep'.\nDefault method: spin");
    pubRate->set_type(T_PAIR_NUMERIC_STR);
    pubRate->set_extra_argument(YES);
    pubRate->set_group(PUB);
    pubRate->set_range(1, 10000000);
    pubRate->add_valid_str_value("sleep");
    pubRate->add_valid_str_value("spin");
    parameterList["pubRate"] = AnyParameter(pubRate);

     std::vector<unsigned long long> _scanDataLenSizes;
    _scanDataLenSizes.push_back(32);
    _scanDataLenSizes.push_back(64);
    _scanDataLenSizes.push_back(128);
    _scanDataLenSizes.push_back(256);
    _scanDataLenSizes.push_back(512);
    _scanDataLenSizes.push_back(1024);
    _scanDataLenSizes.push_back(2048);
    _scanDataLenSizes.push_back(4096);
    _scanDataLenSizes.push_back(8192);
    _scanDataLenSizes.push_back(16384);
    _scanDataLenSizes.push_back(32768);
    _scanDataLenSizes.push_back(63000);
    ParameterVector<unsigned long long> * scan = new ParameterVector<unsigned long long>(_scanDataLenSizes);
    scan->set_command_line_argument(std::make_pair("-scan","<size1>:<size2>:...:<sizeN>"));
    scan->set_description("Run test in scan mode, traversing\na range of sample data sizes from\n[32,63000] or [63001,2147483128] bytes,\nin the case that you are using large data or not.\nThe list of sizes is optional.\nDefault values are '32:64:128:256:512:1024:2048:4096:8192:16384:32768:63000'\nDefault: Not set");
    scan->set_type(T_VECTOR_NUMERIC);
    scan->set_extra_argument(OPTIONAL);
    scan->set_range(perftest_cpp::OVERHEAD_BYTES, MAX_PERFTEST_SAMPLE_SIZE);
    scan->setParseMethod(SPLIT);
    scan->set_group(PUB);
    parameterList["scan"] = AnyParameter(scan);

    Parameter<unsigned long long> *sendQueueSize = new Parameter<unsigned long long>(50);
    sendQueueSize->set_command_line_argument(std::make_pair("-sendQueueSize","<number>"));
    sendQueueSize->set_description("Sets number of samples (or batches) in send\nqueue. Default: 50");
    sendQueueSize->set_type(T_NUMERIC);
    sendQueueSize->set_extra_argument(YES);
    sendQueueSize->set_group(PUB);
    sendQueueSize->set_range(1, INT_MAX);
    parameterList["sendQueueSize"] = AnyParameter(sendQueueSize);

    // TODO convert into NanoSec
    Parameter<unsigned long long> *sleep = new Parameter<unsigned long long>(0);
    sleep->set_command_line_argument(std::make_pair("-sleep","<millisec>"));
    sleep->set_description("Time to sleep between each send. Default: 0");
    sleep->set_type(T_NUMERIC);
    sleep->set_extra_argument(YES);
    sleep->set_range(0, ULLONG_MAX);
    sleep->set_group(PUB);
    parameterList["sleep"] = AnyParameter(sleep);

    Parameter<bool> *writerStats = new Parameter<bool>(false);
    writerStats->set_command_line_argument(std::make_pair("-writerStats",""));
    writerStats->set_description("Display the Pulled Sample count stats for\nreliable protocol debugging purposes.\nDefault: Not set");
    writerStats->set_type(T_BOOL);
    writerStats->set_extra_argument(NO);
    writerStats->set_group(PUB);
    parameterList["writerStats"] = AnyParameter(writerStats);

    Parameter<long> *writeInstance = new Parameter<long>(-1);// (-1) By default use round-robin (-1)
    writeInstance->set_command_line_argument(std::make_pair("-writeInstance","<instance>"));
    writeInstance->set_description("Set the instance number to be sent. \n'-writeInstance' parameter cannot be bigger than the number of instances.\nDefault: 'Round-Robin' schedule");
    writeInstance->set_type(T_NUMERIC);
    writeInstance->set_extra_argument(YES);
    writeInstance->set_range(0, LONG_MAX);
    writeInstance->set_group(PUB);
    parameterList["writeInstance"] = AnyParameter(writeInstance);

    ////////////////////////////////////////////////////////////////////////////
    //SUBSCRIBER PARAMETER
    Parameter<bool> * sub = new Parameter<bool>(false);
    sub->set_command_line_argument(std::make_pair("-sub",""));
    sub->set_description("Set test to be a subscriber");
    sub->set_type(T_BOOL);
    sub->set_extra_argument(NO);
    sub->set_group(SUB);
    parameterList["sub"] = AnyParameter(sub);

    Parameter<unsigned long long> *sidMultiSubTest = new Parameter<unsigned long long>(0);
    sidMultiSubTest->set_command_line_argument(std::make_pair("-sidMultiSubTest","<bytes>"));
    sidMultiSubTest->set_description("Set the id of the subscriber in a\nmulti-subscriber test. Default: 0");
    sidMultiSubTest->set_type(T_NUMERIC);
    sidMultiSubTest->set_extra_argument(YES);
    sidMultiSubTest->set_range(0, INT_MAX);
    sidMultiSubTest->set_group(SUB);
    parameterList["sidMultiSubTest"] = AnyParameter(sidMultiSubTest);

    Parameter<unsigned long long> *numPublishers = new Parameter<unsigned long long>(1);
    numPublishers->set_command_line_argument(std::make_pair("-numPublishers","<count>"));
    numPublishers->set_description("Number of publishers running in test.\nDefault: 1");
    numPublishers->set_type(T_NUMERIC);
    numPublishers->set_extra_argument(YES);
    numPublishers->set_range(1, ULLONG_MAX);
    numPublishers->set_group(SUB);
    parameterList["numPublishers"] = AnyParameter(numPublishers);

    ////////////////////////////////////////////////////////////////////////////
    // TRANSPORT PARAMETER:
    Parameter<std::string> *nic = new Parameter<std::string>();
    nic->set_command_line_argument(std::make_pair("-nic","<ipaddr>"));
    nic->set_description("Use only the nic specified by <ipaddr>.\nIf not specified, use all available interfaces");
    nic->set_type(T_STR);
    nic->set_extra_argument(YES);
    nic->set_group(TRANSPORT);
    parameterList["nic"] = AnyParameter(nic);

    Parameter<std::string> *allowInterfaces = new Parameter<std::string>();
    allowInterfaces->set_command_line_argument(std::make_pair("-allowInterfaces", "<ipaddr>"));
    allowInterfaces->set_description("Use only the nic specified by <ipaddr>.\nIf not specified, use all available interfaces");
    allowInterfaces->set_type(T_STR);
    allowInterfaces->set_extra_argument(YES);
    allowInterfaces->set_group(TRANSPORT);
    allowInterfaces->set_internal(true);
    parameterList["allowInterfaces"] = AnyParameter(allowInterfaces);

    ParameterVector<std::string> * peer = new ParameterVector<std::string>();
    peer->set_command_line_argument(std::make_pair("-peer","<address>"));
    peer->set_description("Adds a peer to the peer host address list.\nThis argument may be repeated to indicate multiple peers");
    peer->set_type(T_VECTOR_STR);
    peer->set_extra_argument(YES);
    peer->set_group(TRANSPORT);
    parameterList["peer"] = AnyParameter(peer);

    Parameter<std::string> * transport = new Parameter<std::string>("Use XML");
    transport->set_command_line_argument(std::make_pair("-transport","<kind>"));
    transport->set_description("Set transport to be used. The rest of\nthe transports will be disabled.\nValues:\nUDPv4\nUDPv6\nSHMEM\nTCP\nTLS\nDTLS\nWAN\nUse XML\nDefault: Use XML (UDPv4|SHMEM)");
    transport->set_type(T_STR);
    transport->set_extra_argument(YES);
    transport->set_group(TRANSPORT);
    transport->add_valid_str_value("UDPv4");
    transport->add_valid_str_value("UDPv6");
    transport->add_valid_str_value("SHMEM");
    transport->add_valid_str_value("TCP");
    transport->add_valid_str_value("TLS");
    transport->add_valid_str_value("DTLS");
    transport->add_valid_str_value("WAN");
    parameterList["transport"] = AnyParameter(transport);

    Parameter<bool> *multicast = new Parameter<bool>(false);
    multicast->set_command_line_argument(std::make_pair("-multicast",""));
    multicast->set_description("Use multicast to send data. Each topic\nwill use a different address\n\tlatency: '239.255.1.2'\n\tthroughput: '239.255.1.1'\n\tannouncement: '239.255.1.100'");
    multicast->set_type(T_BOOL);
    multicast->set_extra_argument(NO);
    multicast->set_group(TRANSPORT);
    parameterList["multicast"] = AnyParameter(multicast);

    // TODO set multicastAddrMap
    /*
     *  multicastAddrMap[THROUGHPUT_TOPIC_NAME] = argv[i];
     *  multicastAddrMap[LATENCY_TOPIC_NAME] = argv[i];
     *  multicastAddrMap[ANNOUNCEMENT_TOPIC_NAME] = argv[i];
     */
    /*
     * TODO: wait for merge the -multicastAddr option update
     * The parse for the input string will be done on the validation. After that
     * parameter manager does not will be called again.
     */
    Parameter<std::string> * multicastAddr = new Parameter<std::string>();
    multicastAddr->set_command_line_argument(std::make_pair("-multicastAddr","<address>"));
    multicastAddr->set_description("Use multicast to send data and set\nthe input <address> as the multicast\naddress for all the topics");
    multicastAddr->set_type(T_STR);
    multicastAddr->set_extra_argument(YES);
    multicastAddr->set_group(TRANSPORT);
    parameterList["multicastAddr"] = AnyParameter(multicastAddr);

    Parameter<std::string> *transportVerbosity = new Parameter<std::string>();
    transportVerbosity->set_command_line_argument(std::make_pair("-transportVerbosity","<level>"));
    transportVerbosity->set_description("Verbosity of the transport.\nDefault: 0 (errors only)");
    transportVerbosity->set_type(T_STR);
    transportVerbosity->set_extra_argument(YES);
    transportVerbosity->set_group(TRANSPORT);
    parameterList["transportVerbosity"] = AnyParameter(transportVerbosity);

    Parameter<std::string> * transportServerBindPort = new Parameter<std::string>("7400");
    transportServerBindPort->set_command_line_argument(std::make_pair("-transportServerBindPort","<p>"));
    transportServerBindPort->set_description("Port used by the transport to accept\nTCP/TLS connections <optional>.\nDefault: 7400");
    transportServerBindPort->set_type(T_STR);
    transportServerBindPort->set_extra_argument(YES);
    transportServerBindPort->set_group(TRANSPORT);
    parameterList["transportServerBindPort"] = AnyParameter(transportServerBindPort);

    Parameter<bool> *transportWan = new Parameter<bool>(false);
    transportWan->set_command_line_argument(std::make_pair("-transportWan",""));
    transportWan->set_description("Use TCP/TLS across LANs and Firewalls.\nDefault: Not Set, LAN mode");
    transportWan->set_type(T_BOOL);
    transportWan->set_extra_argument(NO);
    transportWan->set_group(TRANSPORT);
    parameterList["transportWan"] = AnyParameter(transportWan);

    Parameter<std::string> * transportPublicAddress = new Parameter<std::string>();
    transportPublicAddress->set_command_line_argument(std::make_pair("-transportPublicAddress","<ip>"));
    transportPublicAddress->set_description("Public IP address and port (WAN address\nand port) (separated with ‘:’ ) related\nto the transport instantiation. This is\nrequired when using server mode.\nDefault: Not Set");
    transportPublicAddress->set_type(T_STR);
    transportPublicAddress->set_extra_argument(YES);
    transportPublicAddress->set_group(TRANSPORT);
    parameterList["transportPublicAddress"] = AnyParameter(transportPublicAddress);

    Parameter<std::string> * transportWanServerAddress = new Parameter<std::string>();
    transportWanServerAddress->set_command_line_argument(std::make_pair("-transportWanServerAddress","<a>"));
    transportWanServerAddress->set_description("Address where to find the WAN Server\nDefault: Not Set (Required)\n");
    transportWanServerAddress->set_type(T_STR);
    transportWanServerAddress->set_extra_argument(YES);
    transportWanServerAddress->set_group(TRANSPORT);
    parameterList["transportWanServerAddress"] = AnyParameter(transportWanServerAddress);

    Parameter<std::string> * transportWanServerPort = new Parameter<std::string>("3478");
    transportWanServerPort->set_command_line_argument(std::make_pair("-transportWanServerPort","<p>"));
    transportWanServerPort->set_description("Port where to find the WAN Server.\nDefault: 3478");
    transportWanServerPort->set_type(T_STR);
    transportWanServerPort->set_extra_argument(YES);
    transportWanServerPort->set_group(TRANSPORT);
    parameterList["transportWanServerPort"] = AnyParameter(transportWanServerPort);

    Parameter<std::string> *transportWanId = new Parameter<std::string>();
    transportWanId->set_command_line_argument(std::make_pair("-transportWanId","<id>"));
    transportWanId->set_description("Id to be used for the WAN transport.\nDefault: Not Set (Required)");
    transportWanId->set_type(T_STR);
    transportWanId->set_extra_argument(YES);
    transportWanId->set_group(TRANSPORT);
    parameterList["transportWanId"] = AnyParameter(transportWanId);

    Parameter<bool> *transportSecureWan = new Parameter<bool>(false);
    transportSecureWan->set_command_line_argument(std::make_pair("-transportSecureWan",""));
    transportSecureWan->set_description("Use WAN with security.\nDefault: False");
    transportSecureWan->set_type(T_BOOL);
    transportSecureWan->set_extra_argument(NO);
    transportSecureWan->set_group(TRANSPORT);
    parameterList["transportSecureWan"] = AnyParameter(transportSecureWan);

    Parameter<std::string> * transportCertAuthority = new Parameter<std::string>(TRANSPORT_CERTAUTHORITY_FILE);
    transportCertAuthority->set_command_line_argument(std::make_pair("-transportCertAuthority","<file>"));
    transportCertAuthority->set_description("Certificate authority file <optional>.\nDefault: \"" + TRANSPORT_CERTAUTHORITY_FILE + "\"");
    transportCertAuthority->set_type(T_STR);
    transportCertAuthority->set_extra_argument(YES);
    transportCertAuthority->set_group(TRANSPORT);
    parameterList["transportCertAuthority"] = AnyParameter(transportCertAuthority);

    // TODO assigned a value if it is pub or sub
    Parameter<std::string> * transportCertFile = new Parameter<std::string>(TRANSPORT_CERTIFICATE_FILE_PUB);
    transportCertFile->set_command_line_argument(std::make_pair("-transportCertFile","<file>"));
    transportCertFile->set_description("Certificate file <optional>.\nDefault (Publisher): \"" + TRANSPORT_CERTIFICATE_FILE_PUB + "\"\nDefault (Subscriber): \"" + TRANSPORT_CERTIFICATE_FILE_SUB + "\"\n");
    transportCertFile->set_type(T_STR);
    transportCertFile->set_extra_argument(YES);
    transportCertFile->set_group(TRANSPORT);
    parameterList["transportCertFile"] = AnyParameter(transportCertFile);

    // TODO assigned a value if it is pub or sub
    Parameter<std::string> * transportPrivateKey = new Parameter<std::string>(TRANSPORT_CERTIFICATE_FILE_PUB);
    transportPrivateKey->set_command_line_argument(std::make_pair("-transportPrivateKey","<file>"));
    transportPrivateKey->set_description("Private key file <optional>.\nDefault (Publisher): \"" + TRANSPORT_PRIVATEKEY_FILE_PUB + "\"\nDefault (Subscriber): \"" + TRANSPORT_PRIVATEKEY_FILE_SUB + "\"\n");
    transportPrivateKey->set_type(T_STR);
    transportPrivateKey->set_extra_argument(YES);
    transportPrivateKey->set_group(TRANSPORT);
    parameterList["transportPrivateKey"] = AnyParameter(transportPrivateKey);

    ////////////////////////////////////////////////////////////////////////////
    // SECURE PARAMETER:
  #ifdef RTI_SECURE_PERFTEST
    Parameter<bool> *secureEncryptDiscovery = new Parameter<bool>(false);
    secureEncryptDiscovery->set_command_line_argument(std::make_pair("-secureEncryptDiscovery",""));
    secureEncryptDiscovery->set_description("Encrypt discovery traffic");
    secureEncryptDiscovery->set_type(T_BOOL);
    secureEncryptDiscovery->set_extra_argument(NO);
    secureEncryptDiscovery->set_group(SECURE);
    parameterList["secureEncryptDiscovery"] = AnyParameter(secureEncryptDiscovery);

    Parameter<bool> *secureSign = new Parameter<bool>(false);
    secureSign->set_command_line_argument(std::make_pair("-secureSign",""));
    secureSign->set_description("Sign (HMAC) discovery and user data");
    secureSign->set_type(T_BOOL);
    secureSign->set_extra_argument(NO);
    secureSign->set_group(SECURE);
    parameterList["secureSign"] = AnyParameter(secureSign);

    Parameter<bool> *secureEncryptBoth = new Parameter<bool>(false);
    secureEncryptBoth->set_command_line_argument(std::make_pair("-secureEncryptBoth",""));
    secureEncryptBoth->set_description("Sign (HMAC) discovery and user data");
    secureEncryptBoth->set_type(T_BOOL);
    secureEncryptBoth->set_extra_argument(NO);
    secureEncryptBoth->set_group(SECURE);
    parameterList["secureEncryptBoth"] = AnyParameter(secureEncryptBoth);

    Parameter<bool> *secureEncryptData = new Parameter<bool>(false);
    secureEncryptData->set_command_line_argument(std::make_pair("-secureEncryptData",""));
    secureEncryptData->set_description("Encrypt topic (user) data");
    secureEncryptData->set_type(T_BOOL);
    secureEncryptData->set_extra_argument(NO);
    secureEncryptData->set_group(SECURE);
    parameterList["secureEncryptData"] = AnyParameter(secureEncryptData);

    Parameter<bool> *secureEncryptSM = new Parameter<bool>(false);
    secureEncryptSM->set_command_line_argument(std::make_pair("-secureEncryptSM",""));
    secureEncryptSM->set_description("Encrypt RTPS submessages");
    secureEncryptSM->set_type(T_BOOL);
    secureEncryptSM->set_extra_argument(NO);
    secureEncryptSM->set_group(SECURE);
    parameterList["secureEncryptSM"] = AnyParameter(secureEncryptSM);

    Parameter<std::string> *secureGovernanceFile = new Parameter<std::string>();
    secureGovernanceFile->set_command_line_argument(std::make_pair("-secureGovernanceFile","<file>"));
    secureGovernanceFile->set_description("Governance file. If specified, the authentication,\nsigning, and encryption arguments are ignored. The\ngovernance document configuration will be used instead.\nDefault: built using the secure options");
    secureGovernanceFile->set_type(T_STR);
    secureGovernanceFile->set_extra_argument(YES);
    secureGovernanceFile->set_group(SECURE);
    parameterList["secureGovernanceFile"] = AnyParameter(secureGovernanceFile);

    // TODO assigned a value if it is pub or sub
    Parameter<std::string> *securePermissionsFile = new Parameter<std::string>();
    securePermissionsFile->set_command_line_argument(std::make_pair("-securePermissionsFile","<file>"));
    securePermissionsFile->set_description("Permissions file <optional>.\nDefault: \"./resource/secure/signed_PerftestPermissionsSub.xml\"");
    securePermissionsFile->set_type(T_STR);
    securePermissionsFile->set_extra_argument(YES);
    securePermissionsFile->set_group(SECURE);
    parameterList["securePermissionsFile"] = AnyParameter(securePermissionsFile);

    Parameter<std::string> *secureCertAuthority = new Parameter<std::string>();
    secureCertAuthority->set_command_line_argument(std::make_pair("-secureCertAuthority","<file>"));
    secureCertAuthority->set_description("Certificate authority file <optional>.\nDefault: \"./resource/secure/cacert.pem\"");
    secureCertAuthority->set_type(T_STR);
    secureCertAuthority->set_extra_argument(YES);
    secureCertAuthority->set_group(SECURE);
    parameterList["secureCertAuthority"] = AnyParameter(secureCertAuthority);

    // TODO assigned a value if it is pub or sub
    Parameter<std::string> *secureCertFile = new Parameter<std::string>();
    secureCertFile->set_command_line_argument(std::make_pair("-secureCertFile","<file>"));
    secureCertFile->set_description("Certificate file <optional>.\nDefault: \"./resource/secure/sub.pem\"");
    secureCertFile->set_type(T_STR);
    secureCertFile->set_extra_argument(YES);
    secureCertFile->set_group(SECURE);
    parameterList["secureCertFile"] = AnyParameter(secureCertFile);

    Parameter<std::string> *securePrivateKey = new Parameter<std::string>();
    securePrivateKey->set_command_line_argument(std::make_pair("-securePrivateKey","<file>"));
    securePrivateKey->set_description("Private key file <optional>.\nDefault: \"./resource/secure/subkey.pem\"");
    securePrivateKey->set_type(T_STR);
    securePrivateKey->set_extra_argument(YES);
    securePrivateKey->set_group(SECURE);
    parameterList["securePrivateKey"] = AnyParameter(securePrivateKey);

    Parameter<std::string> *secureLibrary = new Parameter<std::string>();
    secureLibrary->set_command_line_argument(std::make_pair("-secureLibrary","<file>"));
    secureLibrary->set_description("Private key file <optional>.\nDefault: \"./resource/secure/subkey.pem\"");
    secureLibrary->set_type(T_STR);
    secureLibrary->set_extra_argument(YES);
    secureLibrary->set_group(SECURE);
    parameterList["secureLibrary"] = AnyParameter(secureLibrary);

    Parameter<unsigned long long> *secureDebug = new Parameter<unsigned long long>(1);
    secureDebug->set_command_line_argument(std::make_pair("-secureDebug","<level>"));
    secureDebug->set_type(T_NUMERIC);
    secureDebug->set_extra_argument(YES);
    secureDebug->set_range(-1, ULLONG_MAX);
    secureDebug->set_group(SECURE);
    secureDebug->set_internal(true);
    parameterList["secureDebug"] = AnyParameter(secureDebug);

#endif
}


// Parse the command line parameters and set the value
bool ParameterManager::parse(int argc, char *argv[])
{
    unsigned long long var;
    bool success = true;
    // Copy all arguments into a container of strings
    std::vector<std::string> allArgs(argv, argv + argc);

    std::map<std::string, AnyParameter>::iterator it;
    for (unsigned int i = 1; i < allArgs.size(); i++) {
        for (it = parameterList.begin(); it != parameterList.end(); it++) {
            if (IS_OPTION(allArgs[i].c_str(), it->second.get()->get_command_line_argument().first.c_str())) {
                // NumArguments == 0
                if (it->second.get()->get_extra_argument() == NO) {
                    // Type is T_BOOL
                    if (it->second.get()->get_type() == T_BOOL) {
                        (static_cast<Parameter<bool>*>(it->second.get<bool>()))->setValue(true);
                    }
                // NumArguments is 1 or optional
                } else if (it->second.get()->get_extra_argument() > NO) {
                    // Check for error in num of arguments
                    if (i+1 >= allArgs.size() || allArgs[i+1].find("-") == 0) {
                        if (it->second.get()->get_extra_argument() == YES) {
                            fprintf(stderr, "Missing '%s' after '%s'\n",
                                it->second.get()->get_command_line_argument().second.c_str(),
                                it->second.get()->get_command_line_argument().first.c_str());
                            return false;
                        } else if (it->second.get()->get_extra_argument() == OPTIONAL) {
                            it->second.get()->set_isSet(true);
                            break;
                        }
                    }
                    ++i;
                    // Type is T_STR
                    if (it->second.get()->get_type() == T_STR) {
                        if (!it->second.get()->validate_str_range(allArgs[i])) {
                            success = false;
                        }
                        (static_cast<Parameter<std::string>*>(it->second.get<std::string>()))->setValue(allArgs[i]);
                    }
                    // Type is T_NUMERIC
                    else if (it->second.get()->get_type() == T_NUMERIC) {
                        if (sscanf(allArgs[i].c_str(), "%llu", &var) != 1) {
                            fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                    it->second.get()->get_command_line_argument().second.c_str(),
                                    it->second.get()->get_command_line_argument().first.c_str());
                            success = false;
                        }
                        if (!it->second.get()->validate_numeric_range(var)) {
                            success = false;
                        }
                        (static_cast<Parameter<unsigned long long>*>(it->second.get<unsigned long long>()))->setValue(var);
                    }
                    // Type is T_VECTOR_STR
                    else if (it->second.get()->get_type() == T_VECTOR_STR) {
                        if (NOSPLIT == ((ParameterVector<std::string>*)it->second.getVector<std::string>())->get_parse_method()) {
                            if (!it->second.get()->validate_str_range(allArgs[i])) {
                                success = false;
                            }
                            (static_cast<ParameterVector<std::string>*>(it->second.getVector<std::string>()))->setValue(allArgs[i]);
                        }
                    }
                    // Type is T_VECTOR_NUMERIC
                    else if (it->second.get()->get_type() == T_VECTOR_NUMERIC) {
                        if (SPLIT == ((ParameterVector<unsigned long long>*)it->second.getVector<unsigned long long>())->get_parse_method()) {
                            std::vector<std::string> v = split(allArgs[i]);
                            for (unsigned int j = 0; j < v.size(); j++) {
                                if (sscanf(v[j].c_str(), "%llu", &var) != 1) {
                                    fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                            it->second.get()->get_command_line_argument().second.c_str(),
                                            it->second.get()->get_command_line_argument().first.c_str());
                                    success = false;
                                }
                                if (!it->second.get()->validate_numeric_range(var)) {
                                    success = false;
                                }
                                (static_cast<ParameterVector<unsigned long long>*>(it->second.getVector<unsigned long long>()))->setValue(var);
                            }
                        }
                    }
                    // Type is T_PAIR_NUMERIC_STR
                    else if (it->second.get()->get_type() == T_PAIR_NUMERIC_STR) {
                        std::vector<std::string> v = split(allArgs[i]);
                        if (v.size() != 2) {
                            fprintf(stderr, "Missing '%s' after '%s'\n",
                                it->second.get()->get_command_line_argument().second.c_str(),
                                it->second.get()->get_command_line_argument().first.c_str());
                            return false;
                        } else {
                            if (sscanf(v[0].c_str(), "%llu", &var) != 1) {
                                fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                        it->second.get()->get_command_line_argument().second.c_str(),
                                        it->second.get()->get_command_line_argument().first.c_str());
                                success = false;
                            }
                            if (!it->second.get()->validate_numeric_range(var)) {
                                success = false;
                            }
                            if (!it->second.get()->validate_str_range(v[1])) {
                                success = false;
                            }
                            (static_cast<ParameterPair<unsigned long long, std::string>*>(it->second.getPair<unsigned long long, std::string>()))->setValue(var, v[1]);
                        }
                    }
                }
                break;
            }
        }
        if (it == parameterList.end()) {
            fprintf(stderr, "Cannot parse '%s', invalid input.\n",
                    allArgs[i].c_str());
            success = false;
        }
    }
    return success;
}

// Get the help message
std::string ParameterManager::display_help()
{
    std::map<std::string, AnyParameter>::iterator it;
    std::ostringstream oss;
    std::map<GROUP, std::string> output;
    for (int i = GENERAL; i != RAWTRANSPORT+1; i++) {
        switch (static_cast<GROUP>(i)) {
            case GENERAL:
                output[static_cast<GROUP>(i)] += get_center_header_help_line("GENERAL");
                break;
            case PUB:
                output[static_cast<GROUP>(i)] += get_center_header_help_line("PUBLISHER");
                break;
            case SUB:
                output[static_cast<GROUP>(i)] += get_center_header_help_line("SUBSCRIBER");
                break;
            case TRANSPORT:
                output[static_cast<GROUP>(i)] += get_center_header_help_line("TRANSPORT");
                break;
          #ifdef RTI_SECURE_PERFTEST
            case SECURE:
                output[static_cast<GROUP>(i)] += get_center_header_help_line("SECURE");
                break;
          #endif
            case RAWTRANSPORT:
                output[static_cast<GROUP>(i)] += get_center_header_help_line("RAWTRANSPORT");
                break;
            default:
                break;
        }
    }

    oss << "/**********************************************************************************************/\n"
        << "Usage:\t perftest_cpp [options]\n"
        << "Where [options] are:\n";
    output[GENERAL] +="\t-help                           - Print this usage message and exit\n";

    for (it = parameterList.begin(); it != parameterList.end(); it++) {
        if (!it->second.get()->get_internal()) {
            output[it->second.get()->get_group()] +=
                    it->second.get()->print_command_line_parameter();
        }
    }
    std::map<GROUP, std::string>::iterator itOutput;
    for (itOutput = output.begin(); itOutput != output.end(); itOutput++) {
        oss << itOutput->second;
    }
    oss << "/**********************************************************************************************/\n";
    return oss.str();
}

// check -help option
bool ParameterManager::check_help(int argc, char *argv[])
{
    std::vector<std::string> allArgs(argv, argv + argc);
    for (unsigned int i = 1; i < allArgs.size(); i++) {
        if (allArgs[i] == "-help" || allArgs[i] == "-h") {
            std::cout << display_help() <<'\n';
            return true;
        }
    }
    return false;
}

ParameterManager::~ParameterManager()
{
    parameterList.clear();
}

// check if a variable has been set
bool ParameterManager::is_set(std::string parameterKey)
{
    std::map<std::string, AnyParameter>::iterator it;
    it = parameterList.find(parameterKey);
    if (it != parameterList.end()) {
        return parameterList[parameterKey].get()->get_isSet();
    } else {
        return false;
    }
}


bool ParameterManager::validate_group()
{
    bool success = true;
    std::map<std::string, AnyParameter>::iterator it;
    for (it = parameterList.begin(); it != parameterList.end(); it++) {
        if (it->second.get()->get_isSet()) {
            if (it->second.get()->get_group() == PUB && GetInstance().get<bool>("sub")) {
                fprintf(stderr, "Cannot use '%s' while setting '-sub'.\n",
                        it->second.get()->get_command_line_argument().first.c_str());
                success = false;
            } else if (it->second.get()->get_group() == SUB && GetInstance().get<bool>("pub")) {
                fprintf(stderr, "Cannot use '%s' while setting '-pub'.\n",
                        it->second.get()->get_command_line_argument().first.c_str());
                success = false;
            }
        }
    }
    return success;
}


std::vector<std::string> ParameterManager::split(std::string str, char delimiter)
{
    std::vector<std::string> v;
    std::size_t current, previous = 0;
    current = str.find_first_of(delimiter);
    while (current != std::string::npos) {
        v.push_back(str.substr(previous, current - previous));
        previous = current + 1;
        current = str.find_first_of(delimiter, previous);
    }
    v.push_back(str.substr(previous, current - previous));
    return v;
}

std::string ParameterManager::get_center_header_help_line(std::string name){
    name += "Specific Options";
    std::stringstream line;
    unsigned int maxWithLine = 80;
    std::string separatorBar =
            std::string((int) ((maxWithLine - name.length() - 2) / 2), '=');
    line << "\n\n\t" << separatorBar << " " << name << " " << separatorBar << "\n\n";
    if (line.str().length() < maxWithLine) { //If name is odd, then add one more '='
        line << '=';
    }
    return line.str();
}
/*TODO: */
// bool ParameterManager::group_is_use(){

// }