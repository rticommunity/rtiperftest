

/*
 * (c) 2005-2024  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "ParameterManager.h"
#include "perftest_cpp.h"

ParameterManager::ParameterManager() : middleware(Middleware::RTIDDSPRO)
{
}

ParameterManager::ParameterManager(MiddlewareMask middleware) : middleware(middleware)
{
}

void ParameterManager::initialize()
{
    // GENERAL PARAMETER
    Parameter<bool> *bestEffort = new Parameter<bool>(false);
    bestEffort->set_command_line_argument("-bestEffort", "");
    bestEffort->set_description(
            "Run test in best effort mode. Default: reliable");
    bestEffort->set_type(T_BOOL);
    bestEffort->set_extra_argument(NO);
    bestEffort->set_group(GENERAL);
    bestEffort->set_supported_middleware(Middleware::ALLDDS);
    create("bestEffort",  bestEffort);

    Parameter<unsigned long long> *dataLen =
            new Parameter<unsigned long long>(100);
    dataLen->set_command_line_argument("-dataLen", "<bytes>");
    dataLen->set_description(
            "Set length of payload for each send.\nDefault: 100");
    dataLen->set_type(T_NUMERIC_LLU);
    dataLen->set_extra_argument(YES);
    dataLen->set_range(perftest_cpp::OVERHEAD_BYTES, MAX_PERFTEST_SAMPLE_SIZE);
    dataLen->set_group(GENERAL);
    dataLen->set_supported_middleware(Middleware::ALL);
    create("dataLen", dataLen);


    Parameter<int> *verbosity = new Parameter<int>(1);
    verbosity->set_command_line_argument("-verbosity", "<level>");
    verbosity->set_description(
            "Run with different levels of verbosity:\n"
            "0 - SILENT, 1 - ERROR, 2 - WARNING, 3 - ALL.\nDefault: 1");
    verbosity->set_type(T_NUMERIC_D);
    verbosity->set_extra_argument(YES);
    verbosity->set_range(0, 3);
    verbosity->set_group(GENERAL);
    // Everything but CERT
    verbosity->set_supported_middleware(
                Middleware::RTIDDSPRO
                | Middleware::RTIDDSMICRO
                | Middleware::RTITSS
                | Middleware::RAWTRANSPORT);
    create("verbosity", verbosity);

    Parameter<bool> *dynamicData = new Parameter<bool>(false);
    dynamicData->set_command_line_argument("-dynamicData", "");
    dynamicData->set_description(
            "Makes use of the Dynamic Data APIs instead\n"
            "of using the generated types");
    dynamicData->set_type(T_BOOL);
    dynamicData->set_extra_argument(NO);
    dynamicData->set_group(GENERAL);
    dynamicData->set_supported_middleware(Middleware::RTIDDSPRO);
    create("dynamicData", dynamicData);

    Parameter<int> *durability = new Parameter<int>(0);
    durability->set_command_line_argument("-durability", "<0|1|2|3>");
    durability->set_description(
            "Set durability QOS: 0 - volatile,\n"
            "1 - transient local, 2 - transient,\n"
            "3 - persistent. Default: 0");
    durability->set_type(T_NUMERIC_D);
    durability->set_extra_argument(YES);
    durability->set_group(GENERAL);
    durability->set_supported_middleware(Middleware::RTIDDS);
    durability->set_range(0, 3);
    create("durability", durability);

    Parameter<int> *domain = new Parameter<int>(1);
    domain->set_command_line_argument("-domain", "<id>");
    domain->set_description("RTI DDS Domain. Default: 1");
    domain->set_type(T_NUMERIC_D);
    domain->set_extra_argument(YES);
    domain->set_range(0, 250);
    domain->set_group(GENERAL);
    domain->set_supported_middleware(Middleware::ALL);
    create("domain", domain);

    Parameter<long> *instances = new Parameter<long>(1);
    instances->set_command_line_argument("-instances", "<count>");
    instances->set_description(
            "Set the number of instances (keys) to iterate\n"
            "over when publishing. Default: 1");
    instances->set_type(T_NUMERIC_LD);
    instances->set_extra_argument(YES);
    instances->set_range(1, LONG_MAX);
    instances->set_group(GENERAL);
    instances->set_supported_middleware(Middleware::ALLDDS);
    create("instances", instances);

    Parameter<long> *instanceHashBuckets = new Parameter<long>(0);
    instanceHashBuckets->set_command_line_argument(
            "-instanceHashBuckets", "<count>");
    instanceHashBuckets->set_internal(true);
    instanceHashBuckets->set_type(T_NUMERIC_LD);
    instanceHashBuckets->set_extra_argument(YES);
    instanceHashBuckets->set_range(1, 1000000);
    instanceHashBuckets->set_group(GENERAL);
    instanceHashBuckets->set_supported_middleware(Middleware::RTIDDSPRO
                                                  | Middleware::RTITSSPRO);
    create("instanceHashBuckets", instanceHashBuckets);

    Parameter<bool> *keyed = new Parameter<bool>(false);
    keyed->set_command_line_argument("-keyed", "");
    keyed->set_description("Use keyed data. Default: unkeyed");
    keyed->set_type(T_BOOL);
    keyed->set_extra_argument(NO);
    keyed->set_group(GENERAL);
    keyed->set_supported_middleware(Middleware::ALLDDS);
    create("keyed", keyed);

    Parameter<bool> *noDirectCommunication = new Parameter<bool>(false);
    noDirectCommunication->set_command_line_argument(
            "-noDirectCommunication", "");
    noDirectCommunication->set_description(
            "Use brokered mode for persistent durability");
    noDirectCommunication->set_type(T_BOOL);
    noDirectCommunication->set_extra_argument(NO);
    noDirectCommunication->set_group(GENERAL);
    noDirectCommunication->set_supported_middleware(Middleware::RTIDDSPRO
                                                    | Middleware::RTITSSPRO);
    create("noDirectCommunication", noDirectCommunication);

    Parameter<bool> *noPositiveAcks = new Parameter<bool>(false);
    noPositiveAcks->set_command_line_argument("-noPositiveAcks", "");
    noPositiveAcks->set_description(
            "Disable use of positive acks in reliable \n"
            "protocol. Default use positive acks");
    noPositiveAcks->set_type(T_BOOL);
    noPositiveAcks->set_extra_argument(NO);
    noPositiveAcks->set_group(GENERAL);
    noPositiveAcks->set_supported_middleware(Middleware::RTIDDSPRO
                                             | Middleware::RTITSSPRO);
    create("noPositiveAcks", noPositiveAcks);

    Parameter<unsigned long long> *keepDurationUsec =
            new Parameter<unsigned long long>(0);
    keepDurationUsec->set_command_line_argument("-keepDurationUsec", "<usec>");
    keepDurationUsec->set_internal(true);
    keepDurationUsec->set_type(T_NUMERIC_LLU);
    keepDurationUsec->set_extra_argument(YES);
    keepDurationUsec->set_group(GENERAL);
    keepDurationUsec->set_supported_middleware(Middleware::RTIDDSPRO
                                               | Middleware::RTITSSPRO);
    keepDurationUsec->set_range(1,
                (unsigned long long)365 * 24 * 60 * 60 * 1000000);
                // One year in usec
    create("keepDurationUsec", keepDurationUsec);

    Parameter<bool> *noPrintIntervals = new Parameter<bool>(false);
    noPrintIntervals->set_command_line_argument("-noPrintIntervals", "");
    noPrintIntervals->set_description(
            "Don't print statistics at intervals during the test");
    noPrintIntervals->set_type(T_BOOL);
    noPrintIntervals->set_extra_argument(NO);
    noPrintIntervals->set_group(GENERAL);
    noPrintIntervals->set_supported_middleware(Middleware::ALL);
    create("noPrintIntervals", noPrintIntervals);

    Parameter<std::string> *qosFile =
            new Parameter<std::string>("perftest_qos_profiles.xml");
    qosFile->set_command_line_argument("-qosFile", "<filename>");
    qosFile->set_description(
            "Name of XML file for DDS Qos profiles.\n"
            "Default: perftest_qos_profiles.xml");
    qosFile->set_type(T_STR);
    qosFile->set_extra_argument(YES);
    qosFile->set_group(GENERAL);
    qosFile->set_supported_middleware(Middleware::RTIDDSPRO
                                      | Middleware::RTITSSPRO);
    create("qosFile", qosFile);

    Parameter<std::string> *qosLibrary =
            new Parameter<std::string>("PerftestQosLibrary");
    qosLibrary->set_command_line_argument("-qosLibrary", "<lib name>");
    qosLibrary->set_description(
            "Name of QoS Library for DDS Qos profiles.\n"
            "Default: PerftestQosLibrary");
    qosLibrary->set_type(T_STR);
    qosLibrary->set_extra_argument(YES);
    qosLibrary->set_group(GENERAL);
    qosLibrary->set_supported_middleware(Middleware::RTIDDSPRO
                                         | Middleware::RTITSSPRO);
    create("qosLibrary", qosLibrary);

    Parameter<bool> *noXmlQos = new Parameter<bool>(false);
    noXmlQos->set_command_line_argument("-noXmlQos", "");
    noXmlQos->set_description(
            "Skip loading the qos profiles from the xml\nprofile");
    noXmlQos->set_type(T_BOOL);
    noXmlQos->set_extra_argument(NO);
    noXmlQos->set_group(GENERAL);
    noXmlQos->set_supported_middleware(Middleware::RTIDDSPRO
                                       | Middleware::RTITSSPRO);
    create("noXmlQos", noXmlQos);

    Parameter<bool> *useReadThread = new Parameter<bool>(false);
    useReadThread->set_command_line_argument("-useReadThread", "");
    useReadThread->set_description(
            "Use separate thread instead of callback to\nread data");
    useReadThread->set_type(T_BOOL);
    useReadThread->set_extra_argument(NO);
    useReadThread->set_group(GENERAL);
    useReadThread->set_supported_middleware(Middleware::ALLDDS);
    create("useReadThread", useReadThread);

    Parameter<unsigned long long> *waitsetDelayUsec =
            new Parameter<unsigned long long>(100);
    waitsetDelayUsec->set_command_line_argument("-waitsetDelayUsec", "<usec>");
    waitsetDelayUsec->set_description(
            "UseReadThread related. Allows you to\n"
            "process incoming data in groups, based on the\n"
            "time rather than individually. It can be used\n"
            "combined with -waitsetEventCount.\nDefault: 100 usec");
    waitsetDelayUsec->set_type(T_NUMERIC_LLU);
    waitsetDelayUsec->set_extra_argument(YES);
    waitsetDelayUsec->set_group(GENERAL);
    waitsetDelayUsec->set_supported_middleware(Middleware::RTIDDSPRO
                                               | Middleware::RTITSSPRO);
    waitsetDelayUsec->set_range(0, 10000000);
    create("waitsetDelayUsec", waitsetDelayUsec);

    Parameter<long> *waitsetEventCount = new Parameter<long>(5);
    waitsetEventCount->set_command_line_argument("-waitsetEventCount", "<count>");
    waitsetEventCount->set_description(
            "UseReadThread related. Allows you to\n"
            "process incoming data in groups, based on the\n"
            "number of samples rather than individually. It\n"
            "can be used combined with -waitsetDelayUsec.\nDefault: 5");
    waitsetEventCount->set_type(T_NUMERIC_LD);
    waitsetEventCount->set_extra_argument(YES);
    waitsetEventCount->set_group(GENERAL);
    waitsetEventCount->set_supported_middleware(Middleware::RTIDDSPRO
                                                | Middleware::RTITSSPRO);
    waitsetEventCount->set_range(1, LONG_MAX);
    create("waitsetEventCount", waitsetEventCount);

    Parameter<bool> *asynchronous = new Parameter<bool>(false);
    asynchronous->set_command_line_argument("-asynchronous", "");
    asynchronous->set_description("Use asynchronous writer.\nDefault: Not set");
    asynchronous->set_type(T_BOOL);
    asynchronous->set_extra_argument(NO);
    asynchronous->set_group(GENERAL);
    asynchronous->set_supported_middleware(Middleware::RTIDDSPRO
                                           | Middleware::RTITSSPRO);
    create("asynchronous", asynchronous);

    Parameter<bool> *messageSizeMax = new Parameter<bool>(false);
    messageSizeMax->set_command_line_argument("-messageSizeMax", "");
    messageSizeMax->set_description("Set the value of the transport message_size_max.\nDefault: Connext's default");
    messageSizeMax->set_type(T_NUMERIC_LD);
    messageSizeMax->set_extra_argument(YES);
    messageSizeMax->set_range(0, MAX_PERFTEST_SAMPLE_SIZE);
    messageSizeMax->set_group(GENERAL);
    messageSizeMax->set_supported_middleware(Middleware::RTIDDSPRO);
    create("messageSizeMax", messageSizeMax);

    Parameter<std::string> *flowController = new Parameter<std::string>("default");
    flowController->set_command_line_argument("-flowController", "<flow>");
    flowController->set_description(
            "In the case asynchronous writer use a specific flow controller.\n"
            "There are several flow controller predefined:\n"
            "\t{'default', '1Gbps', '10Gbps'}\n"
            "Default: \"default\" (If using asynchronous)");
    flowController->set_type(T_STR);
    flowController->set_extra_argument(YES);
    flowController->add_valid_str_value("default");
    flowController->add_valid_str_value("1Gbps");
    flowController->add_valid_str_value("10Gbps");
    flowController->set_group(GENERAL);
    flowController->set_supported_middleware(Middleware::RTIDDSPRO
                                             | Middleware::RTITSSPRO);
    create("flowController", flowController);

    Parameter<bool> *cpu = new Parameter<bool>(false);
    cpu->set_command_line_argument("-cpu", "");
    cpu->set_description(
            "Display the cpu percent use by the process\nDefault: Not set");
    cpu->set_type(T_BOOL);
    cpu->set_extra_argument(NO);
    cpu->set_group(GENERAL);
    cpu->set_supported_middleware(Middleware::ALL);
    create("cpu", cpu);

    Parameter<int> *unbounded = new Parameter<int>(0);
    unbounded->set_command_line_argument("-unbounded", "<allocation_threshold>");
    unbounded->set_description(
            "Use unbounded Sequences\n"
            "<allocation_threshold> is optional. Default: MAX_BOUNDED_SEQ_SIZE Bytes");
    unbounded->set_type(T_NUMERIC_D);
    unbounded->set_extra_argument(POSSIBLE);
    unbounded->set_range(perftest_cpp::OVERHEAD_BYTES, MAX_BOUNDED_SEQ_SIZE);
    unbounded->set_group(GENERAL);
    unbounded->set_supported_middleware(Middleware::RTIDDSPRO
                                        | Middleware::RTITSSPRO);
    create("unbounded", unbounded);

    Parameter<std::string> *threadPriorities = new Parameter<std::string>("");
    threadPriorities->set_command_line_argument("-threadPriorities", "<X:Y:Z>");
    threadPriorities->set_description(
            "Set the priorities for the application Threads:\n"
            "X - For the Main Thread, which will be the one\n"
            "    sending the data. Also for the Asynchronous \n"
            "    thread if that one is used.\n"
            "Y - For the Receive Threads, If the -useReadThread\n"
            "    is used, also for the thread created to receive\n"
            "    and process data.\n"
            "Z - For the rest of the threads created by the middleware.\n"
            "3 default values: h (high), n (normal) and l (low) can be used\n");
    threadPriorities->set_type(T_STR);
    threadPriorities->set_extra_argument(YES);
    threadPriorities->set_group(GENERAL);
    threadPriorities->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS
            | Middleware::RAWTRANSPORT);
    create("threadPriorities", threadPriorities);

    Parameter<std::string> *outputFormat = new Parameter<std::string>("csv");
    outputFormat->set_command_line_argument("-outputFormat", "<format>");
    outputFormat->set_description(
            "Set the output format.\n"
            "The following formats are available:\n"
            " - 'csv'\n"
            " - 'json'\n"
            " - 'legacy'\n"
            "Default: 'csv'");
    outputFormat->set_type(T_STR);
    outputFormat->set_extra_argument(YES);
    outputFormat->add_valid_str_value("legacy");
    outputFormat->add_valid_str_value("json");
    outputFormat->add_valid_str_value("csv");
    outputFormat->set_group(GENERAL);
    outputFormat->set_supported_middleware(Middleware::ALL);
    create("outputFormat", outputFormat);

    Parameter<bool> *noOutputHeaders = new Parameter<bool>(false);
    noOutputHeaders->set_command_line_argument("-noOutputHeaders", "");
    noOutputHeaders->set_description(
            "Skip displaying the header row with \n"
            "the titles of the tables and the summary.\n"
            "Default: False (it will display titles)");
    noOutputHeaders->set_type(T_BOOL);
    noOutputHeaders->set_extra_argument(NO);
    noOutputHeaders->set_group(GENERAL);
    noOutputHeaders->set_supported_middleware(Middleware::ALL);
    create("noOutputHeaders", noOutputHeaders);

  #ifdef RTI_FLATDATA_AVAILABLE
    Parameter<bool> *flatData = new Parameter<bool>(false);
    flatData->set_command_line_argument("-flatData", "");
    flatData->set_description(
            "Use FlatData language binding\nDefault: Not set");
    flatData->set_type(T_BOOL);
    flatData->set_extra_argument(NO);
    flatData->set_group(GENERAL);
    flatData->set_supported_middleware(Middleware::RTIDDSPRO);
    create("flatdata", flatData);
  #endif

  #if RTI_ZEROCOPY_AVAILABLE
    #if(defined(RTI_CERT) || defined(RTI_FLATDATA_AVAILABLE))
    Parameter<bool> *zerocopy = new Parameter<bool>(false);
    zerocopy->set_command_line_argument("-zeroCopy", "");
    zerocopy->set_description(
            "Use Zero Copy transfer mode. If not Connext CERT,\n"
            "FlatData must be used too\nDefault: Not set");
    zerocopy->set_type(T_BOOL);
    zerocopy->set_extra_argument(NO);
    zerocopy->set_group(GENERAL);
    zerocopy->set_supported_middleware(
        Middleware::RTIDDSPRO
        | Middleware::RTICERT);
    create("zerocopy", zerocopy);
    #endif

    #ifdef RTI_FLATDATA_AVAILABLE
    Parameter<bool> *checkconsistency = new Parameter<bool>(false);
    checkconsistency->set_command_line_argument("-checkConsistency", "");
    checkconsistency->set_description(
            "Check if samples sent with Zero Copy are consistent\nDefault: Not set");
    checkconsistency->set_type(T_BOOL);
    checkconsistency->set_extra_argument(NO);
    checkconsistency->set_group(GENERAL);
    checkconsistency->set_supported_middleware(Middleware::RTIDDSPRO);
    create("checkconsistency", checkconsistency);
    #endif
  #endif

  #if defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_610)
    Parameter<bool> *networkCapture = new Parameter<bool>(false);
    networkCapture->set_command_line_argument("-networkCapture", "");
    networkCapture->set_description(
            "Enable network capture during the test.\n"
            "Default: Disabled");
    networkCapture->set_type(T_BOOL);
    networkCapture->set_extra_argument(NO);
    networkCapture->set_group(GENERAL);
    networkCapture->set_supported_middleware(Middleware::RTIDDSPRO);
    create("networkCapture", networkCapture);

    Parameter<bool> *doNotDropNetworkCapture = new Parameter<bool>(false);
    doNotDropNetworkCapture->set_command_line_argument("-doNotDropNetworkCapture", "");
    doNotDropNetworkCapture->set_description(
            "Do not drop the capture file generated by network capture during the test.\n"
            "Default: Disabled");
    doNotDropNetworkCapture->set_type(T_BOOL);
    doNotDropNetworkCapture->set_extra_argument(NO);
    doNotDropNetworkCapture->set_group(GENERAL);
    doNotDropNetworkCapture->set_supported_middleware(Middleware::RTIDDSPRO);
    create("doNotDropNetworkCapture", doNotDropNetworkCapture);
  #endif

    Parameter<std::string> *outputFile = new Parameter<std::string>("stdout");
    outputFile->set_command_line_argument("-outputFile", "<file>");
    outputFile->set_description(
            "Save the data information (but not the summary info) to a file.\n"
            "Default: Stdout (terminal output)");
    outputFile->set_type(T_STR);
    outputFile->set_extra_argument(YES);
    outputFile->set_group(GENERAL);
    outputFile->set_supported_middleware(Middleware::ALL);
    create("outputFile", outputFile);

    Parameter<bool> *crc = new Parameter<bool>(false);
    crc->set_command_line_argument("-crc", "");
    crc->set_description(
            "Enable CRC checking. Default: Disabled");
    crc->set_type(T_BOOL);
    crc->set_extra_argument(NO);
    crc->set_group(GENERAL);
    crc->set_supported_middleware(Middleware::RTIDDSPRO);
    create("crc", crc);

    Parameter<std::string> *crcKind = new Parameter<std::string>("CRC_32_CUSTOM");
    crcKind->set_command_line_argument("-crcKind", "<value>");
    crcKind->set_description(
            "Modify the default value to compute the CRC.\n"
            "Options: CRC_32_CUSTOM | CRC_32_LEGACY\n"
            "Default: CRC_32_CUSTOM");
    crcKind->set_type(T_STR);
    crcKind->set_extra_argument(YES);
    crcKind->set_group(GENERAL);
    crcKind->set_supported_middleware(Middleware::RTIDDSPRO);
    create("crcKind", crcKind);


    Parameter<bool> *msgLengthHeaderExtension = new Parameter<bool>(false);
    msgLengthHeaderExtension->set_command_line_argument("-enable-message-length", "");
    msgLengthHeaderExtension->set_description(
            "Enable enable_message_length_header_extension. Default: Disabled");
    msgLengthHeaderExtension->set_type(T_BOOL);
    msgLengthHeaderExtension->set_extra_argument(NO);
    msgLengthHeaderExtension->set_group(GENERAL);
    msgLengthHeaderExtension->set_supported_middleware(Middleware::RTIDDSPRO);
    create("enable-message-length", msgLengthHeaderExtension);

  Parameter<bool> *preallocateFragmentation = new Parameter<bool>(false);
    preallocateFragmentation->set_command_line_argument("-preallocateFragmentedSamples", "");
    preallocateFragmentation->set_description(
            "Prevent dynamic allocation of buffer used for storing received fragments\nUseful for data bigger than 5MB\nDefault: Not set");
    preallocateFragmentation->set_type(T_BOOL);
    preallocateFragmentation->set_extra_argument(NO);
    preallocateFragmentation->set_group(GENERAL);
    preallocateFragmentation->set_supported_middleware(Middleware::RTIDDSPRO);
    create("preallocateFragmentedSamples", preallocateFragmentation);

    Parameter<int> *sendQueueSize = new Parameter<int>(50);
    sendQueueSize->set_command_line_argument("-sendQueueSize", "<number>");
    sendQueueSize->set_description(
            "Sets number of samples (or batches) in send\n"
            "queue. Default: 50");
    sendQueueSize->set_type(T_NUMERIC_D);
    sendQueueSize->set_extra_argument(YES);
    sendQueueSize->set_group(GENERAL);
    sendQueueSize->set_supported_middleware(Middleware::ALL);
    sendQueueSize->set_range(1, INT_MAX);
    create("sendQueueSize", sendQueueSize);

    Parameter<int> *receiveQueueSize = new Parameter<int>(128);
    receiveQueueSize->set_command_line_argument("-receiveQueueSize", "<number>");
    receiveQueueSize->set_description(
            "Sets number of samples (or batches) in receive\nqueue. Default: 128");
    receiveQueueSize->set_type(T_NUMERIC_D);
    receiveQueueSize->set_extra_argument(YES);
    receiveQueueSize->set_group(GENERAL);
    receiveQueueSize->set_supported_middleware(
        Middleware::RTIDDSPRO
        | Middleware::RTIDDSMICRO
        | Middleware::RTITSS
        | Middleware::RAWTRANSPORT);
    receiveQueueSize->set_range(1, INT_MAX);
    create("receiveQueueSize", receiveQueueSize);

    Parameter<int> *receiveBufferSize = new Parameter<int>(2097152);
    receiveBufferSize->set_command_line_argument("-receiveBufferSize", "<number>");
    receiveBufferSize->set_description(
            "Sets size of the receive buffer in B\n");
    receiveBufferSize->set_type(T_NUMERIC_D);
    receiveBufferSize->set_extra_argument(YES);
    receiveBufferSize->set_group(GENERAL);
    receiveBufferSize->set_supported_middleware(
        Middleware::RTIDDSMICRO
        | Middleware::RTICERT);
    receiveBufferSize->set_range(1, INT_MAX);
    create("receiveBufferSize", receiveBufferSize);

    Parameter<int> *sendBufferSize = new Parameter<int>(524288);
    sendBufferSize->set_command_line_argument("-sendBufferSize", "<number>");
    sendBufferSize->set_description("Sets size of the send buffer in B\n");
    sendBufferSize->set_type(T_NUMERIC_D);
    sendBufferSize->set_extra_argument(YES);
    sendBufferSize->set_group(GENERAL);
    sendBufferSize->set_supported_middleware(
        Middleware::RTIDDSMICRO
        | Middleware::RTICERT);
    sendBufferSize->set_range(1, INT_MAX);
    create("sendBufferSize", sendBufferSize);

    Parameter<bool> *showResourceLimits = new Parameter<bool>(false);
    showResourceLimits->set_command_line_argument("-showResourceLimits", "");
    showResourceLimits->set_description(
            "Show the resource limits for all different\n"
            "readers and writers. Default: Not Enabled");
    showResourceLimits->set_type(T_BOOL);
    showResourceLimits->set_extra_argument(NO);
    showResourceLimits->set_group(GENERAL);
    showResourceLimits->set_supported_middleware(Middleware::ALL);
    create("showResourceLimits", showResourceLimits);

  #ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    Parameter<bool> *cacheStats = new Parameter<bool>(false);
    cacheStats->set_command_line_argument("-cacheStats", "");
    cacheStats->set_description(
            "Display the reader/writer queue sample count and count_peak.\n"
            "In the Writer side, also display the Pulled Sample count stats for\n"
            "reliable protocol debugging purposes.\nDefault: Not set");
    cacheStats->set_type(T_BOOL);
    cacheStats->set_extra_argument(NO);
    receiveQueueSize->set_group(GENERAL);
    cacheStats->set_supported_middleware(Middleware::RTIDDSPRO);
    create("cacheStats", cacheStats);
  #endif

  #if defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_610)
    Parameter<std::string> *compressionId =
            new Parameter<std::string>("MASK_NONE");
    compressionId->set_command_line_argument("-compressionId", "<kind>");
    compressionId->set_description(
            "Set the compression algorithm to be used.\n"
            "NOTE: Only ZLIB is compatible with batching"
            "\nValues:\nZLIB\nLZ4\nBZIP2\n"
            "Default: Disabled (MASK_NONE)");
    compressionId->set_type(T_STR);
    compressionId->set_extra_argument(YES);
    compressionId->set_group(GENERAL);
    compressionId->set_supported_middleware(Middleware::RTIDDSPRO);
    compressionId->add_valid_str_value("ZLIB");
    compressionId->add_valid_str_value("LZ4");
    compressionId->add_valid_str_value("BZIP2");
    compressionId->add_valid_str_value("MASK_NONE");
    create("compressionId", compressionId);

    Parameter<long> *compressionLevel = new Parameter<long>(10);
    compressionLevel->set_command_line_argument("-compressionLevel", "<level>");
    compressionLevel->set_description(
            "Set the compression level.\n"
            "The value 1 represents the fastest compression time and the\n"
            "lowest compression ratio. The value 10 represents the slowest\n"
            "compression time but the highest compression ratio. A value of 0\n"
            "disables compression.\n"
            "Default: 10 (BEST_COMPRESSION)");
    compressionLevel->set_type(T_NUMERIC_LD);
    compressionLevel->set_extra_argument(YES);
    compressionLevel->set_range(0, 10);
    compressionLevel->set_group(GENERAL);
    compressionLevel->set_supported_middleware(Middleware::RTIDDSPRO);
    create("compressionLevel", compressionLevel);

    Parameter<long> *compressionThreshold = new Parameter<long>(0);
    compressionThreshold->set_command_line_argument(
            "-compressionThreshold",
            "<threshold>");
    compressionThreshold->set_description(
            "Set the compression threshold.\n"
            "The threshold, in bytes, above which a serialized sample will be\n"
            "eligible to be compressed.\n"
            "Default: 0 (Compress all the samples)");
    compressionThreshold->set_type(T_NUMERIC_LD);
    compressionThreshold->set_extra_argument(YES);
    compressionThreshold->set_range(0, LONG_MAX);
    compressionThreshold->set_group(GENERAL);
    compressionThreshold->set_supported_middleware(Middleware::RTIDDSPRO);
    create("compressionThreshold", compressionThreshold);
  #endif //defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_610)


  #if defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_710)
    Parameter<bool> *enableInstanceStateRecovery = new Parameter<bool>(false);
    enableInstanceStateRecovery->set_command_line_argument("-enableInstanceStateRecovery","");
    enableInstanceStateRecovery->set_description("");
    enableInstanceStateRecovery->set_type(T_BOOL);
    enableInstanceStateRecovery->set_extra_argument(NO);
    enableInstanceStateRecovery->set_group(GENERAL);
    enableInstanceStateRecovery->set_supported_middleware(Middleware::RTIDDSPRO);
    create("enableInstanceStateRecovery", enableInstanceStateRecovery);
  #endif // defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_710)

  #if defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_720) && defined(RTI_MONITORING_2)

    Parameter<bool> *enableMonitoring2 = new Parameter<bool>(false);
    enableMonitoring2->set_command_line_argument("-enableMonitoring2","");
    enableMonitoring2->set_description("Enable the use of the rtimonitoring2 library.\n"
                                       "Default is disabled.");
    enableMonitoring2->set_type(T_BOOL);
    enableMonitoring2->set_extra_argument(NO);
    enableMonitoring2->set_group(GENERAL);
    enableMonitoring2->set_supported_middleware(Middleware::RTIDDSPRO);
    create("enableMonitoring2", enableMonitoring2);

    ParameterVector<std::string> *collectorPeer = new ParameterVector<std::string>();
    collectorPeer->set_command_line_argument("-collectorPeer", "<address>");
    collectorPeer->set_description("Address of the Collector Service peer. If provided\n"
                                   "it will enable the use of the rtimonitoring2 library\n"
                                   "This parameter can be used several times for several peers.\n"
                                   "By default no value is provided.");
    collectorPeer->set_type(T_VECTOR_STR);
    collectorPeer->set_extra_argument(YES);
    collectorPeer->set_group(GENERAL);
    collectorPeer->set_supported_middleware(Middleware::RTIDDSPRO);
    create("collectorPeer", collectorPeer);

  #endif // defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_CONNEXT_PRO_720) && defined(RTI_MONITORING_2)


  #ifdef RTI_PERF_TSS
    Parameter<bool> *loaningSendReceive = new Parameter<bool>(false);
    loaningSendReceive->set_command_line_argument("-loaningSendReceive", "");
    loaningSendReceive->set_description(
            "Enables loaning samples when sending and receiving them.\n"
            "Disabled by default.");
    loaningSendReceive->set_type(T_BOOL);
    loaningSendReceive->set_extra_argument(NO);
    loaningSendReceive->set_group(GENERAL);
    loaningSendReceive->set_supported_middleware(Middleware::RTITSS);
    create("loaningSendReceive", loaningSendReceive);
  #endif /* RTI_PERF_TSS */

    ////////////////////////////////////////////////////////////////////////////
    //PUBLISHER PARAMETER

    Parameter<long> *batchSize =
            new Parameter<long>(DEFAULT_THROUGHPUT_BATCH_SIZE);
    batchSize->set_command_line_argument("-batchSize", "<bytes>");
    batchSize->set_description(
            "Size in bytes of batched message. Default: 8kB.\n"
            "(Disabled for LatencyTest mode or if dataLen > 4kB)");
    batchSize->set_type(T_NUMERIC_LD);
    batchSize->set_extra_argument(YES);
    batchSize->set_range(0, MAX_PERFTEST_SAMPLE_SIZE - 1);
    batchSize->set_group(PUB);
    batchSize->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTITSSPRO);
    create("batchSize", batchSize);

    Parameter<bool> *enableAutoThrottle = new Parameter<bool>(false);
    enableAutoThrottle->set_command_line_argument("-enableAutoThrottle", "");
    enableAutoThrottle->set_description(
            "Enables the AutoThrottling feature in the\n"
            "throughput DataWriter (pub)");
    enableAutoThrottle->set_type(T_BOOL);
    enableAutoThrottle->set_extra_argument(NO);
    enableAutoThrottle->set_group(PUB);
    enableAutoThrottle->set_supported_middleware(Middleware::RTIDDSPRO);
    create("enableAutoThrottle", enableAutoThrottle);

    Parameter<bool> *enableTurboMode = new Parameter<bool>(false);
    enableTurboMode->set_command_line_argument("-enableTurboMode", "");
    enableTurboMode->set_description(
            "Enables the TurboMode feature in the\n"
            "throughput DataWriter (pub)");
    enableTurboMode->set_type(T_BOOL);
    enableTurboMode->set_extra_argument(NO);
    enableTurboMode->set_group(PUB);
    enableTurboMode->set_supported_middleware(Middleware::RTIDDSPRO);
    create("enableTurboMode", enableTurboMode);

    Parameter<bool> *pub = new Parameter<bool>(false);
    pub->set_command_line_argument("-pub", "");
    pub->set_description("Set test to be a publisher");
    pub->set_type(T_BOOL);
    pub->set_extra_argument(NO);
    pub->set_group(PUB);
    pub->set_supported_middleware(Middleware::ALL);
    create("pub", pub);

    Parameter<unsigned long long> *latencyCount =
            new Parameter<unsigned long long>(10000);
    latencyCount->set_command_line_argument("-latencyCount", "<count>");
    latencyCount->set_description(
            "Number of samples (or batches) to send before\n"
            "a latency ping packet is sent.\n"
            "Default:10000 if -latencyTest is not specified,\n"
            "1 if -latencyTest is specified");
    latencyCount->set_type(T_NUMERIC_LLU);
    latencyCount->set_extra_argument(YES);
    latencyCount->set_range(1, MAX_ULLONG);
    latencyCount->set_group(PUB);
    latencyCount->set_supported_middleware(Middleware::ALL);
    create("latencyCount", latencyCount);

    Parameter<unsigned long long> *executionTime =
            new Parameter<unsigned long long>(0);
    executionTime->set_command_line_argument("-executionTime", "<sec>");
    executionTime->set_description(
            "Set a maximum duration for the test.\n"
            "The first condition triggered will finish the test:\n"
            "number of samples or execution time.\n"
            "Default 0 (don't set execution time)");
    executionTime->set_type(T_NUMERIC_LLU);
    executionTime->set_extra_argument(YES);
    executionTime->set_range(1, MAX_ULLONG);
    executionTime->set_group(PUB);
    executionTime->set_supported_middleware(Middleware::ALL);
    create("executionTime", executionTime);

    Parameter<long> *initialBurstSize =
            new Parameter<long>(0);
    initialBurstSize->set_command_line_argument("-initialBurstSize", "<samples>");
    initialBurstSize->set_description(
            "Set the initial burst size to initialize the queues.\n"
            "Default Calculated by RTI Perftest");
    initialBurstSize->set_type(T_NUMERIC_LLU);
    initialBurstSize->set_extra_argument(YES);
    initialBurstSize->set_range(0, LONG_MAX);
    initialBurstSize->set_group(PUB);
    initialBurstSize->set_supported_middleware(Middleware::ALL);
    create("initialBurstSize", initialBurstSize);

    Parameter<bool> *latencyTest = new Parameter<bool>(false);
    latencyTest->set_command_line_argument("-latencyTest", "");
    latencyTest->set_description(
            "Run a latency test consisting of a ping-pong\n"
            "synchronous communication");
    latencyTest->set_type(T_BOOL);
    latencyTest->set_extra_argument(NO);
    latencyTest->set_group(PUB);
    latencyTest->set_supported_middleware(Middleware::ALL);
    create("latencyTest", latencyTest);

    Parameter<unsigned long long> *numIter =
            new Parameter<unsigned long long>(100000000);
    numIter->set_command_line_argument("-numIter", "<count>");
    numIter->set_description(
            "Set number of messages to send. Default:\n"
            "100000000 for Throughput tests or 10000000\n"
            "for Latency tests. See '-executionTime'");
    numIter->set_type(T_NUMERIC_LLU);
    numIter->set_extra_argument(YES);
    numIter->set_range(1, MAX_ULLONG);
    numIter->set_group(PUB);
    numIter->set_supported_middleware(Middleware::ALL);
    create("numIter", numIter);

    Parameter<int> *numSubscribers = new Parameter<int>(1);
    numSubscribers->set_command_line_argument("-numSubscribers", "<count>");
    numSubscribers->set_description(
            "Number of subscribers running in test.\nDefault: 1");
    numSubscribers->set_type(T_NUMERIC_D);
    numSubscribers->set_extra_argument(YES);
    numSubscribers->set_range(1, INT_MAX);
    numSubscribers->set_group(PUB);
    numSubscribers->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("numSubscribers", numSubscribers);

    Parameter<int> *pidMultiPubTest = new Parameter<int>(0);
    pidMultiPubTest->set_command_line_argument("-pidMultiPubTest", "<bytes>");
    pidMultiPubTest->set_description(
            "Set id of the publisher in a multi-publisher test.\n"
            "Only publisher 0 sends\nlatency pings. Default: 0");
    pidMultiPubTest->set_type(T_NUMERIC_D);
    pidMultiPubTest->set_extra_argument(YES);
    pidMultiPubTest->set_range(0, INT_MAX);
    pidMultiPubTest->set_group(PUB);
    pidMultiPubTest->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("pidMultiPubTest", pidMultiPubTest);

    ParameterPair<unsigned long long, std::string> *pubRate =
            new ParameterPair<unsigned long long, std::string>(0, "spin");
    pubRate->set_command_line_argument("-pubRate", "<samples/s>:<method>");
    pubRate->set_description(
            "Limit the throughput to the specified number\n"
            "of samples/s. Default 0 (don't limit)\n"
            "[OPTIONAL] Method to control the throughput can be:\n"
            "'spin' or 'sleep'.\nDefault method: spin");
    pubRate->set_type(T_PAIR_NUMERIC_STR);
    pubRate->set_extra_argument(YES);
    pubRate->set_group(PUB);
    pubRate->set_supported_middleware(Middleware::ALL);
    pubRate->set_range(1, 10000000);
    pubRate->add_valid_str_value("sleep");
    pubRate->add_valid_str_value("spin");
    create("pubRate", pubRate);

    Parameter<std::string> *latencyFile = new Parameter<std::string>("latency_samples.csv");
    latencyFile->set_command_line_argument("-latencyFile", "<filename>");
    latencyFile->set_description(
            "Save all the latency samples as a .csv file at the end of the test.\n"
            "Default method: do not save");
    latencyFile->set_type(T_STR);
    latencyFile->set_extra_argument(YES);
    latencyFile->set_group(PUB);
    latencyFile->set_supported_middleware(Middleware::ALL);
    create("latencyFile", latencyFile);

#ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    ParameterPair<unsigned long long, std::string> *pubRatebps =
            new ParameterPair<unsigned long long, std::string>(0, "spin");
    pubRatebps->set_command_line_argument("-pubRatebps", "<bps>:<method>");
    pubRatebps->set_description(
            "Limit the throughput to the specified number\n"
            "of samples/s. Default 0 (don't limit)\n"
            "[OPTIONAL] Method to control the throughput can be:\n"
            "'spin' or 'sleep'.\nDefault method: spin");
    pubRatebps->set_type(T_PAIR_NUMERIC_STR);
    pubRatebps->set_extra_argument(YES);
    pubRatebps->set_group(PUB);
    pubRatebps->set_supported_middleware(Middleware::ALL);
    pubRatebps->set_range(1, 10000000);
    pubRatebps->add_valid_str_value("sleep");
    pubRatebps->add_valid_str_value("spin");
    create("pubRatebps", pubRatebps);
#endif

    Parameter<unsigned long long> *sleep = new Parameter<unsigned long long>(0);
    sleep->set_command_line_argument("-sleep", "<millisec>");
    sleep->set_description(
            "Time to sleep between each send. Default: 0");
    sleep->set_type(T_NUMERIC_LLU);
    sleep->set_extra_argument(YES);
    sleep->set_range(1, MAX_ULLONG);
    sleep->set_group(PUB);
    sleep->set_supported_middleware(Middleware::ALL);
    create("sleep", sleep);

    Parameter<unsigned long long> *spin = new Parameter<unsigned long long>(0);
    spin->set_command_line_argument("-spin", "<count>");
    spin->set_internal(true);
    spin->set_type(T_NUMERIC_LLU);
    spin->set_extra_argument(YES);
    spin->set_range(1, MAX_ULLONG);
    spin->set_group(PUB);
    spin->set_supported_middleware(Middleware::ALL);
    create("spin", spin);

  #ifndef RTI_LANGUAGE_CPP_TRADITIONAL
    Parameter<bool> *writerStats = new Parameter<bool>(false);
    writerStats->set_command_line_argument("-writerStats", "");
    writerStats->set_description(
            "Display the Pulled Sample count stats for\n"
            "reliable protocol debugging purposes.\nDefault: Not set");
    writerStats->set_type(T_BOOL);
    writerStats->set_extra_argument(NO);
    writerStats->set_group(PUB);
    writerStats->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("writerStats", writerStats);
  #endif

  #ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    Parameter<bool> *lowResolutionClock = new Parameter<bool>(false);
    lowResolutionClock->set_command_line_argument("-lowResolutionClock", "");
    lowResolutionClock->set_description(
            "Reports at the end of the test the average\n"
            "latency time to send all the samples.\n"
            "This option should be used if the machine's\n"
            "clock is not precise enough and the latency\n"
            "measurements report 0 most of the times.\n"
            "This should only be used if latencyCount = 1\n"
            "(Latency Test)\n"
            "Default: Not set");
    lowResolutionClock->set_type(T_BOOL);
    lowResolutionClock->set_extra_argument(NO);
    lowResolutionClock->set_group(PUB);
    lowResolutionClock->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("lowResolutionClock", lowResolutionClock);
  #endif

    Parameter<long> *writeInstance =
            new Parameter<long>(-1); // (-1) By default use round-robin (-1)
    writeInstance->set_command_line_argument("-writeInstance", "<instance>");
    writeInstance->set_description(
            "Set the instance number to be sent.\n"
            "'-writeInstance' parameter cannot be bigger than the number\n"
            "of instances. Default: 'Round-Robin' schedule");
    writeInstance->set_type(T_NUMERIC_LD);
    writeInstance->set_extra_argument(YES);
    writeInstance->set_range(0, LONG_MAX);
    writeInstance->set_group(PUB);
    writeInstance->set_supported_middleware(Middleware::ALL);
    create("writeInstance", writeInstance);

  #ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    Parameter<bool> *serializationTime = new Parameter<bool>(false);
    serializationTime->set_command_line_argument("-showSerializationTime", "");
    serializationTime->set_description(
            "Show serialization/Deserialization times for the sample size(s)\n"
            "of the test. This time will be shown after the test concludes\n"
            "Default: Not set");
    serializationTime->set_type(T_BOOL);
    serializationTime->set_extra_argument(NO);
    serializationTime->set_group(PUB);
    serializationTime->set_supported_middleware(
            Middleware::RTIDDSPRO);
    create("serializationTime", serializationTime);
  #endif

  #ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    /* This feature is still just available for Traditional C++ */
    Parameter<std::string> *loadDataFromFile =
            new Parameter<std::string>;
    loadDataFromFile->set_command_line_argument("-loadDataFromFile", "<path>");
    loadDataFromFile->set_description(
            "Set the file used to populate the payload\n"
            "Default: Not set");
    loadDataFromFile->set_type(T_STR);
    loadDataFromFile->set_extra_argument(YES);
    loadDataFromFile->set_group(PUB);
    loadDataFromFile->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("loadDataFromFile", loadDataFromFile);

    Parameter<unsigned long long> *maximumAllocableBufferSize
            = new Parameter<unsigned long long>(1073741824); // 1GB
    maximumAllocableBufferSize->set_command_line_argument(
            "-maximumAllocableBufferSize",
            "<s>");
    maximumAllocableBufferSize->set_description(
            "When loading data from a file, the maximum size of the buffer\n"
            "where the data is going to be stored in memory.\n"
            "Default: 1GB");
    maximumAllocableBufferSize->set_type(T_NUMERIC_LLU);
    maximumAllocableBufferSize->set_extra_argument(YES);
    maximumAllocableBufferSize->set_range(1, MAX_ULLONG);
    maximumAllocableBufferSize->set_group(PUB);
    maximumAllocableBufferSize->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("maximumAllocableBufferSize", maximumAllocableBufferSize);
  #endif //RTI_LANGUAGE_CPP_TRADITIONAL

    ////////////////////////////////////////////////////////////////////////////
    //SUBSCRIBER PARAMETER
    Parameter<bool> *sub = new Parameter<bool>(false);
    sub->set_command_line_argument("-sub", "");
    sub->set_description("Set test to be a subscriber");
    sub->set_type(T_BOOL);
    sub->set_extra_argument(NO);
    sub->set_group(SUB);
    sub->set_supported_middleware(Middleware::ALL);
    create("sub", sub);

    Parameter<int> *sidMultiSubTest = new Parameter<int>(0);
    sidMultiSubTest->set_command_line_argument("-sidMultiSubTest", "<bytes>");
    sidMultiSubTest->set_description(
            "Set the id of the subscriber in a\n"
            "multi-subscriber test. Default: 0");
    sidMultiSubTest->set_type(T_NUMERIC_D);
    sidMultiSubTest->set_extra_argument(YES);
    sidMultiSubTest->set_range(0, INT_MAX);
    sidMultiSubTest->set_group(SUB);
    sidMultiSubTest->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("sidMultiSubTest", sidMultiSubTest);

    Parameter<int> *numPublishers = new Parameter<int>(1);
    numPublishers->set_command_line_argument("-numPublishers", "<count>");
    numPublishers->set_description(
            "Number of publishers running in test.\nDefault: 1");
    numPublishers->set_type(T_NUMERIC_D);
    numPublishers->set_extra_argument(YES);
    numPublishers->set_range(1, INT_MAX);
    numPublishers->set_group(SUB);
    numPublishers->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTIDDSMICRO
            | Middleware::RTITSS);
    create("numPublishers", numPublishers);

    ParameterVector<unsigned long long> *cft =
            new ParameterVector<unsigned long long>();
    cft->set_command_line_argument("-cft", "<start>:<end>");
    cft->set_description(
            "Use a Content Filtered Topic for the Throughput topic in the\n"
            "subscriber side. Specify 2 parameters: <start> and <end> to\n"
            "receive samples with a key in that range.\n"
            "Specify only 1 parameter to receive samples with that exact key.\n"
            "Default: Not set");
    cft->set_type(T_VECTOR_NUMERIC);
    cft->set_extra_argument(YES);
    cft->set_range(0, MAX_CFT_VALUE - 1);
    cft->set_parse_method(SPLIT);
    cft->set_group(SUB);
    cft->set_supported_middleware(Middleware::RTIDDSPRO
                                  | Middleware::RTITSSPRO);
    create("cft", cft);

    ////////////////////////////////////////////////////////////////////////////
    // TRANSPORT PARAMETER:
    Parameter<std::string> *nic = new Parameter<std::string>();
    nic->set_command_line_argument("-nic", "<ipaddr/name>");
    nic->set_description(
            "Use only the NIC specified by <ipaddr> to receive\n"
            "packets. This will be the only address announced\n"
            "at discovery time. If not specified, use all"
            "available interfaces"
          #ifdef PERFTEST_RTI_MICRO
            "\n"
            "When using RTI Connext DDS Micro, always specify the\n"
            "name, not the IP Address."
          #endif
            );
    nic->set_type(T_STR);
    nic->set_extra_argument(YES);
    nic->set_group(TRANSPORT);
    nic->set_supported_middleware(Middleware::ALL);
    create("nic", nic);

    Parameter<std::string> *allowInterfaces = new Parameter<std::string>();
    allowInterfaces->set_command_line_argument("-allowInterfaces", "<ipaddr>");
    allowInterfaces->set_description(
            "Use only the NIC specified by <ipaddr> to receive\n"
            "packets. This will be the only address announced\n"
            "at discovery time. If not specified, use all"
            "available interfaces"
          #ifdef PERFTEST_RTI_MICRO
            "\n"
            "When using RTI Connext DDS Micro, always specify the\n"
            "name, not the IP Address."
          #endif
            );
    allowInterfaces->set_type(T_STR);
    allowInterfaces->set_extra_argument(YES);
    allowInterfaces->set_group(TRANSPORT);
    allowInterfaces->set_supported_middleware(Middleware::ALL);
    allowInterfaces->set_internal(true);
    create("allowInterfaces", allowInterfaces);

    ParameterVector<std::string> *peer = new ParameterVector<std::string>();
    peer->set_command_line_argument("-peer", "<address>");
    peer->set_description(
            "Adds a peer to the peer host address list.\n"
            "If -rawTransport is used, a optional ID of the subscriber could be\n"
            "provided.\n"
            "This argument may be repeated to indicate multiple peers");
    peer->set_type(T_VECTOR_STR);
    peer->set_extra_argument(YES);
    peer->set_group(TRANSPORT);
    peer->set_supported_middleware(Middleware::ALL);
    create("peer", peer);

    Parameter<std::string> *transport = new Parameter<std::string>("Use XML");
    transport->set_command_line_argument("-transport", "<kind>");
    transport->set_description(
            "Set transport to be used. The rest of\n"
            "the transports will be disabled."
          #if defined(PERFTEST_RTI_PRO)
            "\nValues:\n"
            "\tUDPv4\n"
            "\tUDPv6\n"
            "\tSHMEM\n"
            "\tTCP\n"
            "\tTLS\n"
            "\tDTLS\n"
            "\tWAN\n"
          #ifdef PERFTEST_CONNEXT_PRO_610
            "\tUDPv4_WAN\n"
          #endif // PERFTEST_CONNEXT_PRO_610
            "\tUse XML\n"
            "Default: Use XML (UDPv4|SHMEM)"
          #elif defined(PERFTEST_RTI_MICRO)
            "\nValues:\n\tUDPv4\n\tSHMEM\n"
            "Default: UDPv4"
          #elif defined(RTI_CERT)
            "\nValues:\n"
            #ifdef RTI_ZEROCOPY_AVAILABLE
            "\tUDPv4 & SHMEM\n"
            "Default: UDPv4 & SHMEM"
            #else
            "\tUDPv4\n"
            "Default: UDPv4"
            #endif
          #endif
          );
    transport->set_type(T_STR);
    transport->set_extra_argument(YES);
    transport->set_group(TRANSPORT);
    transport->set_supported_middleware(Middleware::ALL);
    transport->add_valid_str_value("UDPv4");
    transport->add_valid_str_value("SHMEM");
  #if defined(PERFTEST_RTI_PRO)
    transport->add_valid_str_value("UDPv6");
    transport->add_valid_str_value("TCP");
    transport->add_valid_str_value("TLS");
    transport->add_valid_str_value("DTLS");
    transport->add_valid_str_value("WAN");
    #ifdef PERFTEST_CONNEXT_PRO_610
    transport->add_valid_str_value("UDPv4_WAN");
    #endif // PERFTEST_CONNEXT_PRO_610
  #endif
    create("transport", transport);

    Parameter<bool> *multicast = new Parameter<bool>(false);
    multicast->set_command_line_argument("-multicast", "");
    std::ostringstream aux;
    aux << "Use multicast to send data. Each topic\n"
        << "will use a different address\n\tlatency: '"
        << TRANSPORT_MULTICAST_ADDR_LATENCY
        << "' \n\tthroughput: '"
        << TRANSPORT_MULTICAST_ADDR_THROUGHPUT
        << "'\n\tannouncement: '"
        <<  TRANSPORT_MULTICAST_ADDR_ANNOUNCEMENT
        << "'";
    multicast->set_description(aux.str());
    multicast->set_type(T_BOOL);
    multicast->set_extra_argument(NO);
    multicast->set_group(TRANSPORT);
    multicast->set_supported_middleware(Middleware::ALL);
    create("multicast", multicast);

    Parameter<std::string> *multicastAddr = new Parameter<std::string>();
    multicastAddr->set_command_line_argument("-multicastAddr", "<address>");
    multicastAddr->set_description(
            "Use multicast to send data and set\n"
            "the input <address>|<addr,addr,addr>\n"
            "as the multicast addresses for the\n"
            "three topics in the application.\n"
            "If only one address is provided, that\n"
            "one and the 2 consecutive ones will be\n"
            "used for the 3 topics used by Perftest.\n"
            "If one address is set, this one must be\n"
            "in multicast range and lower than\n"
            "239.255.255.253 or the equivalent on IPv6");
    multicastAddr->set_type(T_STR);
    multicastAddr->set_extra_argument(YES);
    multicastAddr->set_group(TRANSPORT);
    multicastAddr->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RAWTRANSPORT
            | Middleware::RTITSSPRO);
    create("multicastAddr", multicastAddr);

    Parameter<std::string> *transportVerbosity = new Parameter<std::string>();
    transportVerbosity->set_command_line_argument(
            "-transportVerbosity", "<level>");
    transportVerbosity->set_description(
            "Verbosity of the transport.\n"
            "Default: 0 (errors only)");
    transportVerbosity->set_type(T_STR);
    transportVerbosity->set_extra_argument(YES);
    transportVerbosity->set_group(TRANSPORT);
    transportVerbosity->set_supported_middleware(
            Middleware::RTIDDSPRO | Middleware::RAWTRANSPORT | Middleware::RTITSSPRO);
    create("transportVerbosity", transportVerbosity);

    Parameter<std::string> *transportServerBindPort =
            new Parameter<std::string>("7400");
    transportServerBindPort->set_command_line_argument(
            "-transportServerBindPort", "<p>");
    transportServerBindPort->set_description(
            "Port used by the transport to accept\n"
            "TCP/TLS connections <optional>.\nDefault: 7400");
    transportServerBindPort->set_type(T_STR);
    transportServerBindPort->set_extra_argument(YES);
    transportServerBindPort->set_group(TRANSPORT);
    transportServerBindPort->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportServerBindPort", transportServerBindPort);

    Parameter<bool> *transportWan = new Parameter<bool>(false);
    transportWan->set_command_line_argument("-transportWan", "");
    transportWan->set_description(
            "Use TCP/TLS across LANs and Firewalls.\n"
            "Default: Not Set, LAN mode");
    transportWan->set_type(T_BOOL);
    transportWan->set_extra_argument(NO);
    transportWan->set_group(TRANSPORT);
    transportWan->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportWan", transportWan);

    Parameter<std::string> *transportPublicAddress =
            new Parameter<std::string>();
    transportPublicAddress->set_command_line_argument(
            "-transportPublicAddress", "<ip>");
    transportPublicAddress->set_description(
            "For TCP: Public IP address and port (WAN address\n"
            "and port) (separated with \':\' ) related\n"
            "to the TCP transport instantiation. This is\n"
            "required when using server mode.\n"
            #ifdef PERFTEST_CONNEXT_PRO_610
            "For UDPv4_WAN (Real-Time WAN Transport): Public address of the UDPv4_WAN\n"
            "transport instantiation. Format: <public_ip>:<public_send_port>\n"
            "Default: Not Set\n"
            #endif //PERFTEST_CONNEXT_PRO_610
            );
    transportPublicAddress->set_type(T_STR);
    transportPublicAddress->set_extra_argument(YES);
    transportPublicAddress->set_group(TRANSPORT);
    transportPublicAddress->set_supported_middleware(
            Middleware::RTIDDSPRO | Middleware::RAWTRANSPORT);
    create("transportPublicAddress", transportPublicAddress);

  #ifdef PERFTEST_CONNEXT_PRO_610
    Parameter<std::string> *transportHostPort =
            new Parameter<std::string>();
    transportHostPort->set_command_line_argument(
            "-transportHostPort", "<port>");
    transportHostPort->set_description(
            "Configure host port when using Real-Time WAN Transport\n"
            "Default: set to public port specified as part of -transportPublicAddress\n"
            );
    transportHostPort->set_type(T_STR);
    transportHostPort->set_extra_argument(YES);
    transportHostPort->set_group(TRANSPORT);
    transportHostPort->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportHostPort", transportHostPort);
  #endif //PERFTEST_CONNEXT_PRO_610


    Parameter<std::string> *transportWanServerAddress =
            new Parameter<std::string>();
    transportWanServerAddress->set_command_line_argument(
            "-transportWanServerAddress", "<a>");
    transportWanServerAddress->set_description(
            "Address where to find the WAN Server\n"
            "Default: Not Set (Required)");
    transportWanServerAddress->set_type(T_STR);
    transportWanServerAddress->set_extra_argument(YES);
    transportWanServerAddress->set_group(TRANSPORT);
    transportWanServerAddress->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportWanServerAddress", transportWanServerAddress);

    Parameter<std::string> *transportWanServerPort =
        new Parameter<std::string>("3478");
    transportWanServerPort->set_command_line_argument(
            "-transportWanServerPort", "<p>");
    transportWanServerPort->set_description(
            "Port where to find the WAN Server.\nDefault: 3478");
    transportWanServerPort->set_type(T_STR);
    transportWanServerPort->set_extra_argument(YES);
    transportWanServerPort->set_group(TRANSPORT);
    transportWanServerPort->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportWanServerPort", transportWanServerPort);

    Parameter<std::string> *transportWanId = new Parameter<std::string>();
    transportWanId->set_command_line_argument("-transportWanId", "<id>");
    transportWanId->set_description(
            "Id to be used for the WAN transport.\n"
            "Default: Not Set (Required)");
    transportWanId->set_type(T_STR);
    transportWanId->set_extra_argument(YES);
    transportWanId->set_group(TRANSPORT);
    transportWanId->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportWanId", transportWanId);

    Parameter<bool> *transportSecureWan = new Parameter<bool>(false);
    transportSecureWan->set_command_line_argument("-transportSecureWan", "");
    transportSecureWan->set_description(
            "Use WAN with security.\nDefault: False");
    transportSecureWan->set_type(T_BOOL);
    transportSecureWan->set_extra_argument(NO);
    transportSecureWan->set_group(TRANSPORT);
    transportSecureWan->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportSecureWan", transportSecureWan);

    /*
     * Set this parameter to a empty value by default. This will be set later
     * if it's needed with the corresponding value.
     */
    Parameter<std::string> *transportCertAuthority =
            new Parameter<std::string>("");
    transportCertAuthority->set_command_line_argument(
            "-transportCertAuthority", "<file>");
    transportCertAuthority->set_description(
            "Certificate authority file <optional>.\n"
            "Default: \"" + TRANSPORT_CERTAUTHORITY_FILE + "\"");
    transportCertAuthority->set_type(T_STR);
    transportCertAuthority->set_extra_argument(YES);
    transportCertAuthority->set_group(TRANSPORT);
    transportCertAuthority->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportCertAuthority", transportCertAuthority);

    /*
     * Set this parameter to a empty value by default. This will be set later
     * if it's needed with the corresponding value.
     */
    Parameter<std::string> *transportCertFile = new Parameter<std::string>("");
    transportCertFile->set_command_line_argument("-transportCertFile", "<file>");
    transportCertFile->set_description(
            "Certificate file <optional>.\n"
            "Default (Publisher): \"" + TRANSPORT_CERTIFICATE_FILE_PUB + "\"\n"
            "Default (Subscriber): \"" + TRANSPORT_CERTIFICATE_FILE_SUB + "\"\n");
    transportCertFile->set_type(T_STR);
    transportCertFile->set_extra_argument(YES);
    transportCertFile->set_group(TRANSPORT);
    transportCertFile->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportCertFile", transportCertFile);

    /*
     * Set this parameter to a empty value by default. This will be set later
     * if it's needed with the corresponding value.
     */
    Parameter<std::string> *transportPrivateKey
            = new Parameter<std::string>("");
    transportPrivateKey->set_command_line_argument(
            "-transportPrivateKey", "<file>");
    transportPrivateKey->set_description(
            "Private key file <optional>.\n"
            "Default (Publisher): \"" + TRANSPORT_PRIVATEKEY_FILE_PUB + "\"\n"
            "Default (Subscriber): \"" + TRANSPORT_PRIVATEKEY_FILE_SUB + "\"\n");
    transportPrivateKey->set_type(T_STR);
    transportPrivateKey->set_extra_argument(YES);
    transportPrivateKey->set_group(TRANSPORT);
    transportPrivateKey->set_supported_middleware(Middleware::RTIDDSPRO);
    create("transportPrivateKey", transportPrivateKey);

  #ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    ////////////////////////////////////////////////////////////////////////////
    // RAWTRANSPORT PARAMTER:

    Parameter<bool> *rawTransport = new Parameter<bool>(false);
    rawTransport->set_command_line_argument("-rawTransport", "");
    rawTransport->set_description(
            "Use sockets as a transport instead of DDS protocol.\n"
            "Support UDPv4 and Shared Memory (SHMEM).\n"
            "Many of the parameters are not supported with sockets.\n");
    rawTransport->set_type(T_BOOL);
    rawTransport->set_extra_argument(NO);
    rawTransport->set_group(RAWTRANSPORT);
    rawTransport->set_supported_middleware(Middleware::RAWTRANSPORT);
    create("rawTransport", rawTransport);

    Parameter<bool> *noBlockingSockets = new Parameter<bool>(false);
    noBlockingSockets->set_command_line_argument("-noBlockingSockets", "");
    noBlockingSockets->set_description(
            "Control blocking behavior of send sockets to never block.\n"
            "CHANGING THIS FROM THE DEFAULT CAN CAUSE SIGNIFICANT\n"
            "PERFORMANCE PROBLEMS.\n");
    noBlockingSockets->set_type(T_BOOL);
    noBlockingSockets->set_extra_argument(NO);
    noBlockingSockets->set_group(RAWTRANSPORT);
    noBlockingSockets->set_supported_middleware(Middleware::RAWTRANSPORT);
    create("noBlockingSockets", noBlockingSockets);
  #endif

  #if defined(RTI_LANGUAGE_CPP_TRADITIONAL) && defined(PERFTEST_FAST_QUEUE)
    //  This parameter is just supported in the traditional C++ language.
    Parameter<bool> *fastQueue = new Parameter<bool>(false);
    fastQueue->set_command_line_argument("-fastQueue", "");
    fastQueue->set_description(
            "Private option for measuring using FasQueue.\n");
    fastQueue->set_type(T_BOOL);
    fastQueue->set_extra_argument(NO);
    fastQueue->set_group(GENERAL);
    fastQueue->set_supported_middleware(Middleware::RTIDDSPRO);
    create("fastQueue", fastQueue);
  #endif

    ////////////////////////////////////////////////////////////////////////////
    // SECURE PARAMETER:
  #ifdef RTI_SECURE_PERFTEST

  // These options only make sense when not using LW security, this will
  // happen in Static if we have not defined RTI_LW_SECURE_PERFTEST, and in
  // dynamic if the command line option -lightWeightSecurity was not passed.
  #ifndef RTI_LW_SECURE_PERFTEST

    Parameter<std::string> *secureGovernanceFile = new Parameter<std::string>();
    secureGovernanceFile->set_command_line_argument(
            "-secureGovernanceFile", "<file>");
    secureGovernanceFile->set_description(
            "Governance file to use.");
    secureGovernanceFile->set_type(T_STR);
    secureGovernanceFile->set_extra_argument(YES);
    secureGovernanceFile->set_group(SECURE);
    secureGovernanceFile->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("secureGovernanceFile", secureGovernanceFile);

    Parameter<std::string> *securePermissionsFile = new Parameter<std::string>();
    securePermissionsFile->set_command_line_argument(
            "-securePermissionsFile", "<file>");
    securePermissionsFile->set_description(
            "Permissions file <optional>.\n"
            "Default: \"./resource/secure/signed_PerftestPermissionsSub.xml\"");
    securePermissionsFile->set_type(T_STR);
    securePermissionsFile->set_extra_argument(YES);
    securePermissionsFile->set_group(SECURE);
    securePermissionsFile->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("securePermissionsFile", securePermissionsFile);

    Parameter<std::string> *secureCertAuthority = new Parameter<std::string>();
    secureCertAuthority->set_command_line_argument(
            "-secureCertAuthority", "<file>");
    secureCertAuthority->set_description(
            "Certificate authority file <optional>.\n"
            "Default: \"./resource/secure/cacert.pem\"");
    secureCertAuthority->set_type(T_STR);
    secureCertAuthority->set_extra_argument(YES);
    secureCertAuthority->set_group(SECURE);
    secureCertAuthority->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("secureCertAuthority", secureCertAuthority);

    Parameter<std::string> *secureCertFile = new Parameter<std::string>();
    secureCertFile->set_command_line_argument("-secureCertFile", "<file>");
    secureCertFile->set_description(
            "Certificate file <optional>.\n"
            "Default: \"./resource/secure/sub.pem\"");
    secureCertFile->set_type(T_STR);
    secureCertFile->set_extra_argument(YES);
    secureCertFile->set_group(SECURE);
    secureCertFile->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("secureCertFile", secureCertFile);

    Parameter<std::string> *securePrivateKey = new Parameter<std::string>();
    securePrivateKey->set_command_line_argument("-securePrivateKey", "<file>");
    securePrivateKey->set_description(
            "Private key file <optional>.\n"
            "Default: \"./resource/secure/subkey.pem\"");
    securePrivateKey->set_type(T_STR);
    securePrivateKey->set_extra_argument(YES);
    securePrivateKey->set_group(SECURE);
    securePrivateKey->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("securePrivateKey", securePrivateKey);

    Parameter<std::string> *secureEncryptionAlgo = new Parameter<std::string>();
    secureEncryptionAlgo->set_command_line_argument("-secureEncryptionAlgorithm", "<value>");
    secureEncryptionAlgo->set_description(
            "Set the value for the Encryption Algorithm.\n"
            "Default: \"aes-128-gcm\"");
    secureEncryptionAlgo->set_type(T_STR);
    secureEncryptionAlgo->set_extra_argument(YES);
    secureEncryptionAlgo->set_group(SECURE);
    secureEncryptionAlgo->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("secureEncryptionAlgo", secureEncryptionAlgo);

  #endif // !defined(RTI_LW_SECURE_PERFTEST)

    Parameter<bool> *secureEnableAAD = new Parameter<bool>(false);
    secureEnableAAD->set_command_line_argument("-secureEnableAAD", "");
    secureEnableAAD->set_description("Enable AAD. Default: Not enabled.");
    secureEnableAAD->set_type(T_BOOL);
    secureEnableAAD->set_extra_argument(NO);
    secureEnableAAD->set_group(SECURE);
    secureEnableAAD->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("secureEnableAAD", secureEnableAAD);

  #ifdef RTI_PERFTEST_DYNAMIC_LINKING

    Parameter<std::string> *secureLibrary = new Parameter<std::string>();
    secureLibrary->set_command_line_argument("-secureLibrary", "<file>");
    secureLibrary->set_description(
            "Security Library that will be loaded dynamically.\n"
            "Default: Not used, empty value");
    secureLibrary->set_type(T_STR);
    secureLibrary->set_extra_argument(YES);
    secureLibrary->set_group(SECURE);
    secureLibrary->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    create("secureLibrary", secureLibrary);

    Parameter<bool> *lightWeightSecurity = new Parameter<bool>(false);
    lightWeightSecurity->set_command_line_argument("-lightWeightSecurity", "");
    lightWeightSecurity->set_description(
            "Use the Lightweight security Library.\n"
            "Default: Not enabled.");
    lightWeightSecurity->set_type(T_BOOL);
    lightWeightSecurity->set_extra_argument(NO);
    lightWeightSecurity->set_group(SECURE);
    lightWeightSecurity->set_supported_middleware(Middleware::RTIDDSPRO);
    create("lightWeightSecurity", lightWeightSecurity);

  #endif

    // This one will be used both in Static and Dynamic. In static will be yet
    // another security option. In dynamic this will actually enable LightWeight
    // Security.
    Parameter<std::string> *securePSK = new Parameter<std::string>();
    securePSK->set_command_line_argument("-securePSK", "<seed>");
    securePSK->set_description("Enable PSK security. Default: Not enabled.");
    securePSK->set_type(T_STR);
    securePSK->set_extra_argument(YES);
    securePSK->set_group(SECURE);
    securePSK->set_supported_middleware(Middleware::RTIDDSPRO);
    create("securePSK", securePSK);

    // This one will be used both in Static and Dynamic. In static will be yet
    // another security option. In dynamic this will actually enable LightWeight
    // Security.
    Parameter<std::string> *securePSKAlgorithm = new Parameter<std::string>("AES256+GCM");
    securePSKAlgorithm->set_command_line_argument("-securePSKAlgorithm", "<seed>");
    securePSKAlgorithm->set_description("PSK Algoritm to use. Default: AES256+GCM.");
    securePSKAlgorithm->set_type(T_STR);
    securePSKAlgorithm->set_extra_argument(YES);
    securePSKAlgorithm->set_group(SECURE);
    securePSKAlgorithm->set_supported_middleware(Middleware::RTIDDSPRO);
    create("securePSKAlgorithm", securePSKAlgorithm);


  #ifdef RTI_LANGUAGE_CPP_TRADITIONAL
    Parameter<std::string> *secureRtpsHmacOnly = new Parameter<std::string>();
    secureRtpsHmacOnly->set_command_line_argument("-secureRtpsHmacOnly", "<seed>");
    secureRtpsHmacOnly->set_description("Enable HMAC Only security. Default: Not enabled.");
    secureRtpsHmacOnly->set_type(T_STR);
    secureRtpsHmacOnly->set_extra_argument(YES);
    secureRtpsHmacOnly->set_group(SECURE);
    secureRtpsHmacOnly->set_supported_middleware(Middleware::RTIDDSPRO);
    create("secureRtpsHmacOnly", secureRtpsHmacOnly);
  #endif


    Parameter<int> *secureDebug = new Parameter<int>(1);
    secureDebug->set_command_line_argument("-secureDebug", "<level>");
    secureDebug->set_type(T_NUMERIC_D);
    secureDebug->set_extra_argument(YES);
    secureDebug->set_range(0, 7);
    secureDebug->set_group(SECURE);
    secureDebug->set_supported_middleware(
            Middleware::RTIDDSPRO
            | Middleware::RTIDDSMICRO);
    secureDebug->set_internal(true);
    create("secureDebug", secureDebug);

#endif
} // End of the initialize.

// Parse the command line parameters and set the value
bool ParameterManager::parse(int argc, char *argv[])
{
    unsigned long long varLLU;
    long varLD;
    int varD;
    bool success = true;
    ParameterBase *p = NULL;
    // Copy all arguments into a vector of strings
    std::vector<std::string> allArgs(argv, argv + argc);
    std::map<std::string, AnyParameter>::iterator it;

    for (unsigned int i = 1; i < allArgs.size(); i++) {
        for (it = _parameterList.begin(); it != _parameterList.end(); it++) {
            if (IS_OPTION(
                    allArgs[i].c_str(),
                    it->second.get()->get_option().c_str())) {
                // Save the variable, it will be used several time.
                p = it->second.get();
                // NumArguments == 0
                if (p->get_extra_argument() == NO) {
                    // Type is T_BOOL
                    if (p->get_type() == T_BOOL) {
                        (static_cast<Parameter<bool>*>(
                                it->second.get<bool>()))->set_value(true);
                    }
                // NumArguments is 1 or optional
                } else { // if (p->get_extra_argument() > NO) {
                    // Check for error in the number of arguments
                    if (i + 1 >= allArgs.size() || allArgs[i + 1].find("-") == 0) {
                        if (p->get_extra_argument() == YES) {
                            fprintf(stderr, "Missing '%s' after '%s'\n",
                                p->get_arg().c_str(),
                                p->get_option().c_str());
                            return false;
                        } else if (p->get_extra_argument() == POSSIBLE) {
                            p->set_isSet(true);
                            break;
                        }
                    }
                    ++i;
                    // Type is T_STR
                    if (p->get_type() == T_STR) {
                        if (!p->validate_str_range(allArgs[i])) {
                            success = false;
                        }
                        (static_cast<Parameter<std::string>*>(
                                it->second.get<std::string>()))->set_value(
                                        allArgs[i]);
                    }
                    // Type is T_NUMERIC_LLU
                    else if (p->get_type() == T_NUMERIC_LLU) {
                        if (sscanf(allArgs[i].c_str(), "%llu", &varLLU) != 1) {
                            fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                    p->get_arg().c_str(),
                                    p->get_option().c_str());
                            success = false;
                        }
                        if (!p->validate_numeric_range(varLLU)) {
                            success = false;
                        }
                        (static_cast<Parameter<unsigned long long>*>(
                                it->second.get<unsigned long long>()))->set_value(
                                        varLLU);
                    }
                    // Type is T_NUMERIC_LD
                    else if (p->get_type() == T_NUMERIC_LD) {
                        if (sscanf(allArgs[i].c_str(), "%ld", &varLD) != 1) {
                            fprintf(stderr, "Cannot parse '%s' '%s', invalid input.\n",
                                    p->get_arg().c_str(),
                                    p->get_option().c_str());
                            success = false;
                        }
                        if (!p->validate_numeric_range(varLD)) {
                            success = false;
                        }
                        (static_cast<Parameter<long>*>(
                                it->second.get<long>()))->set_value(varLD);
                    }
                    // Type is T_NUMERIC_D
                    else if (p->get_type() == T_NUMERIC_D) {
                        if (sscanf(allArgs[i].c_str(), "%d", &varD) != 1) {
                            fprintf(stderr,
                                    "Cannot parse '%s' '%s', invalid input.\n",
                                    p->get_arg().c_str(),
                                    p->get_option().c_str());
                            success = false;
                        }
                        if (!p->validate_numeric_range(varD)) {
                            success = false;
                        }
                        (static_cast<Parameter<int>*>(
                                it->second.get<int>()))->set_value(varD);
                    }
                    // Type is T_VECTOR_STR
                    else if (p->get_type() == T_VECTOR_STR) {
                        if (NO_SPLIT == ((ParameterVector<std::string>*)it->
                                second.get_vector<std::string>())->
                                        get_parse_method()) {
                            if (!p->validate_str_range(allArgs[i])) {
                                success = false;
                            }
                            (static_cast<ParameterVector<std::string>*>(
                                    it->second.get_vector<std::string>()))->
                                        set_value(allArgs[i]);
                        }
                    }
                    // Type is T_VECTOR_NUMERIC
                    else if (p->get_type() == T_VECTOR_NUMERIC) {
                        if (SPLIT == ((ParameterVector<unsigned long long>*)it->
                                second.get_vector<unsigned long long>())->
                                        get_parse_method()) {
                            std::vector<std::string> v = split(allArgs[i]);
                            for (unsigned int j = 0; j < v.size(); j++) {
                                if (sscanf(v[j].c_str(), "%llu", &varLLU) != 1) {
                                    fprintf(stderr,
                                            "Cannot parse '%s' '%s', invalid input.\n",
                                            p->get_arg().c_str(),
                                            p->get_option().c_str());
                                    success = false;
                                }
                                if (!p->validate_numeric_range(varLLU)) {
                                    success = false;
                                }
                                (static_cast<ParameterVector<unsigned long long>*>
                                        (it->second.get_vector
                                                <unsigned long long>()))->
                                                        set_value(varLLU);
                            }
                        }
                    }
                    // Type is T_PAIR_NUMERIC_STR
                    else if (p->get_type() == T_PAIR_NUMERIC_STR) {
                        std::vector<std::string> v = split(allArgs[i]);
                        if (v.size() != 2) {
                            fprintf(stderr, "Missing '%s' after '%s'\n",
                                p->get_arg().c_str(),
                                p->get_option().c_str());
                            return false;
                        } else {
                            if (sscanf(v[0].c_str(), "%llu", &varLLU) != 1) {
                                fprintf(stderr,
                                        "Cannot parse '%s' '%s', invalid input.\n",
                                        p->get_arg().c_str(),
                                        p->get_option().c_str());
                                success = false;
                            }
                            if (!p->validate_numeric_range(varLLU)) {
                                success = false;
                            }
                            if (!p->validate_str_range(v[1])) {
                                success = false;
                            }
                            (static_cast<ParameterPair<unsigned long long,
                                    std::string>*>(it->second.get_pair<
                                            unsigned long long,
                                                    std::string>()))->
                                                            set_value(varLLU,
                                                                    v[1]);
                        }
                    }
                }
                break;
            }
        }
        if (it == _parameterList.end()) {
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
    std::map<Group, std::string> output;
    for (unsigned int i = GENERAL; i != RAWTRANSPORT + 1; i++) {
        switch (static_cast<Group>(i)) {
            case GENERAL:
                output[static_cast<Group>(i)] +=
                    get_center_header_help_line("GENERAL");
                break;
            case PUB:
                output[static_cast<Group>(i)] +=
                        get_center_header_help_line("PUBLISHER");
                break;
            case SUB:
                output[static_cast<Group>(i)] +=
                        get_center_header_help_line("SUBSCRIBER");
                break;
            case TRANSPORT:
                output[static_cast<Group>(i)] +=
                        get_center_header_help_line("TRANSPORT");
                break;
          #ifdef RTI_SECURE_PERFTEST
            case SECURE:
                output[static_cast<Group>(i)] +=
                        get_center_header_help_line("SECURE");
                break;
          #endif
            case RAWTRANSPORT:
                output[static_cast<Group>(i)] +=
                        get_center_header_help_line("RAWTRANSPORT");
                break;
            default:
                break;
        }
    }
    oss << std::string(100, '*') << std::endl;
    oss << "Usage:\t perftest_cpp [options]\n"
        << "Where [options] are:\n";
    output[GENERAL] += "\t-help                           - "
            "Print this usage message and exit\n";

    for (it = _parameterList.begin(); it != _parameterList.end(); it++) {
        if (!it->second.get()->get_internal()) {
            output[it->second.get()->get_group()] +=
                    it->second.get()->print_command_line_parameter();
        }
    }
    std::map<Group, std::string>::iterator itOutput;
    for (itOutput = output.begin(); itOutput != output.end(); itOutput++) {
        oss << itOutput->second;
    }
    oss << std::string(100, '*') << std::endl;
    return oss.str();
}

// check for the '-help' option and display help if requested
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
    _parameterList.clear();
}

// check if a variable has been set
bool ParameterManager::is_set(std::string parameterKey)
{
    std::map<std::string, AnyParameter>::iterator it =
            _parameterList.find(parameterKey);
    if (it != _parameterList.end()) {
        return it->second.get()->get_isSet();
    } else {
        return false;
    }
}

bool ParameterManager::check_incompatible_parameters()
{
    bool success = true;
    std::map<std::string, AnyParameter>::iterator it;

    /*
     * RAWTRANSPORT is not actually a middleware, and the desition is made at
     * execution time (it has to be RTI Connext DDS Pro to support this feature)
     */
    if (get<bool>("rawTransport")) {
        this->middleware = Middleware::RAWTRANSPORT;
    }

    // Check if the middleware accepts all the parameters we have in the list
    for (it = _parameterList.begin(); it != _parameterList.end(); it++) {
        if (it->second.get()->get_isSet()
            && !(it->second.get()->get_supported_middleware()
                 & this->middleware)) {
            fprintf(stderr,
                    "[Error] Parser: '%s' Not supported by the Middleware "
                    "implementation.\n",
                    it->second.get()->get_option().c_str());
            return false;
        }
    }

    for (it = _parameterList.begin(); it != _parameterList.end(); it++) {
        if (it->second.get()->get_isSet()) {
            if (it->second.get()->get_group() == PUB && get<bool>("sub")) {
                fprintf(stderr, "Cannot use '%s' while setting '-sub'.\n",
                        it->second.get()->get_option().c_str());
                success = false;
            } else if (it->second.get()->get_group() == SUB && get<bool>("pub")) {
                fprintf(stderr, "Cannot use '%s' while setting '-pub'.\n",
                        it->second.get()->get_option().c_str());
                success = false;
            }
        }
    }

    return success;
}


std::vector<std::string> ParameterManager::split(
        std::string var,
        std::string delimiter)
{
    std::vector<std::string> v;
    char *pch;
    char *str = new char[var.length() + 1];
    strcpy(str, var.c_str());
    pch = strtok (str, delimiter.c_str());
    while (pch != NULL) {
        v.push_back(pch);
        pch = strtok (NULL, delimiter.c_str());
    }
    delete[] str;
    return v;
}

std::string ParameterManager::get_center_header_help_line(std::string name)
{
    name += " Specific Options";
    std::stringstream line;
    unsigned int maxWithLine = 80;
    std::string separatorBar =
            std::string((int) ((maxWithLine - name.length() - 2) / 2), '=');
    line << "\n\n\t"
         << separatorBar
         << " "
         << name
         << " "
         << separatorBar
         << "\n\n";
    if (line.str().length() < maxWithLine) {
        //If name is odd, then add one more '='
        line << '=';
    }
    return line.str();
}

bool ParameterManager::group_is_used(Group group)
{
    std::map<std::string, AnyParameter>::iterator it;
    for (it = _parameterList.begin(); it != _parameterList.end(); it++) {
        if (it->second.get()->get_isSet()
                && it->second.get()->get_group() == group) {
            return true;
        }
    }
    return false;
}
