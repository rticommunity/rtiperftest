/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestPrinter.h"
#include "perftest.h"
#include "perftestSupport.h"

RTIDDSMessageLogger::RTIDDSMessageLogger(int domain)
        : domain(domain),
          topicName("PerftestLogging"),
          logWriter(NULL),
          sample(NULL)
{
}

bool RTIDDSMessageLogger::initialize()
{

    DDS_ReturnCode_t retcode;

    DDS_DomainParticipantQos dpQos;
    DDS_PublisherQos publisherQos;
    DDS_DataWriterQos dwQos;

    DDS_DomainParticipantFactoryQos factory_qos;

    // Setup the QOS profile file to be loaded
    DDSTheParticipantFactory->get_qos(factory_qos);
    factory_qos.profile.url_profile.ensure_length(1, 1);
    factory_qos.profile.url_profile[0] = DDS_String_dup("perftest_qos_profiles.xml");
    DDSTheParticipantFactory->set_qos(factory_qos);

    if (DDSTheParticipantFactory->reload_profiles() != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem opening QOS profiles file\n");
        return false;
    }



    if (DDSTheParticipantFactory->get_participant_qos_from_profile(
            dpQos,
            "PerftestQosLibrary",
            "AnnouncementQos")
            != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem setting QoS Library for .\n");
    }

    if (DDSTheParticipantFactory->get_publisher_qos_from_profile(
        publisherQos,
        "PerftestQosLibrary",
        "LoggingQos")
        != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem setting QoS Library for publisherQos.\n");
    }

    if (DDSTheParticipantFactory->get_datawriter_qos_from_profile(
        dwQos,
        "PerftestQosLibrary",
        "LoggingQos")
        != DDS_RETCODE_OK) {
        fprintf(stderr,
                "Problem setting QoS Library for dwQos.\n");
    }

    participant = DDSTheParticipantFactory->create_participant(
            (DDS_DomainId_t) domain,
            dpQos,
            NULL,
            DDS_STATUS_MASK_NONE);
    if (participant == NULL) {
        fprintf(stderr, "RTIDDSMessageLogger Problem creating participant.\n");
        finalize();
        return false;
    }

    publisher = participant->create_publisher(
        publisherQos,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (publisher == NULL) {
        fprintf(stderr, "RTIDDSMessageLogger create_publisher error\n");
        finalize();
        return false;
    }

    retcode = perftestLogMessage::TypeSupport::register_type(
        participant,
        perftestLogMessage::TypeSupport::get_type_name());
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "RTIDDSMessageLogger register_type error %d\n", retcode);
        finalize();
        return false;
    }

    topic = participant->create_topic(
        topicName.c_str(),
        perftestLogMessage::TypeSupport::get_type_name(),
        DDS_TOPIC_QOS_DEFAULT,
        NULL,
        DDS_STATUS_MASK_NONE);
    if (topic == NULL) {
        fprintf(stderr, "RTIDDSMessageLogger create_topic error\n");
        finalize();
        return false;
    }

    DDSDataWriter *writer = publisher->create_datawriter(
        topic, dwQos, NULL,
        DDS_STATUS_MASK_NONE);
    if (writer == NULL) {
        fprintf(stderr, "RTIDDSMessageLogger create_datawriter error\n");
        finalize();
        return false;
    }

    logWriter = perftestLogMessage::DataWriter::narrow(writer);
    if (logWriter == NULL) {
        fprintf(stderr, "DataWriter narrow error\n");
        finalize();
        return false;
    }

    sample = perftestLogMessage::TypeSupport::create_data();
    if (sample == NULL) {
        fprintf(stderr, "testTypeSupport::create_data error\n");
        finalize();
        return false;
    }

    return true;
}

void RTIDDSMessageLogger::finalize() {

    DDS_ReturnCode_t retcode;
    int status = 0;

    if (perftestLogMessage::TypeSupport::delete_data(sample) != DDS_RETCODE_OK) {
        fprintf(stderr, "testTypeSupport::delete_data error %d\n", retcode);
    }
    if (participant != NULL) {
        retcode = participant->delete_contained_entities();
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_contained_entities error %d\n", retcode);
        }

        retcode = DDSTheParticipantFactory->delete_participant(participant);
        if (retcode != DDS_RETCODE_OK) {
            fprintf(stderr, "delete_participant error %d\n", retcode);
        }
    }
}

bool RTIDDSMessageLogger::writeMessage(int datalen, int latency, float thr) {

    DDS_ReturnCode_t retcode;
    sample->datalen = datalen;
    if (latency != -1) {
        sample->latency = latency;
    }
    if (thr != -1) {
        sample->throughput = thr;
    }
    logWriter->write(*sample, DDS_HANDLE_NIL);
    if (retcode != DDS_RETCODE_OK) {
        fprintf(stderr, "write error %d\n", retcode);
    }
    return false;
}


PerftestPrinter::PerftestPrinter()
        : _dataLength(100),
          _printSummaryHeaders(true),
          ddslogger(10),
          _outputFormat(CSV)
{
}

void PerftestPrinter::initialize(
        bool printIntervals,
        std::string outputFormat,
        bool printHeaders,
        bool printSerialization,
        bool showCpu)
{
    _printIntervals = printIntervals;
    _printHeaders = printHeaders;
    _printSerialization = printSerialization;
    _showCPU = showCpu;
    if (outputFormat == "csv") {
        _outputFormat = CSV;
    } else if (outputFormat == "json") {
        _outputFormat = JSON;
        _isJsonInitialized = false;
    } else if (outputFormat == "dds") {
        _outputFormat = DDS;
        ddslogger.initialize();
    } else if (outputFormat == "legacy") {
        _outputFormat = LEGACY;
    }
}

void PerftestPrinter::set_data_length(unsigned int dataLength)
{
    _dataLength = dataLength;
}

void PerftestPrinter::set_header_printed(bool printHeaders)
{
    _printHeaders = printHeaders;
}

void PerftestPrinter::print_latency_header()
{
    switch (_outputFormat) {
    case DDS:
    case CSV:
        if (_printHeaders && _printIntervals) {
            printf("\nIntervals One-way Latency for %d Bytes:\n", _dataLength);
            printf("Length (Bytes)"
                   ", Latency (" PERFT_TIME_UNIT
                   "), Ave (" PERFT_TIME_UNIT
                   "), Std (" PERFT_TIME_UNIT
                   "), Min (" PERFT_TIME_UNIT
                   "), Max (" PERFT_TIME_UNIT ")");
            if (_showCPU) {
                printf(", CPU (%%)");
            }
            printf("\n");
        }
        break;
    case JSON:
        if (_isJsonInitialized) {
            printf(",\n\t\t{\n");
        } else {
            _isJsonInitialized = true;
        }
        printf("\t\t\t\"length\":%d,\n", _dataLength);

        if (_printIntervals && _outputFormat == JSON) {
            printf("\t\t\t\"intervals\":[\n");
            _controlJsonIntervals = true;
        }
        break;
    case LEGACY:
        if (_printHeaders && _printIntervals) {
            printf("\n\n********** New data length is %d\n", _dataLength);
        }
        break;
    }
}

void PerftestPrinter::print_throughput_header()
{
    switch (_outputFormat) {
    case DDS:
    case CSV:
        if (_printHeaders && _printIntervals) {
            printf("\nInterval Throughput for %d Bytes:\n", _dataLength);
            printf("Length (Bytes), Total Samples,  Samples/s,"
                   " Ave Samples/s,     Mbps,  Ave Mbps"
                   ", Lost Samples, Lost Samples (%%)");
            if (_showCPU) {
                printf(", CPU (%%)");
            }
            printf("\n");
        }
        break;
    case JSON:
        if (_isJsonInitialized) {
            printf(",\n\t\t{\n");
        } else {
            _isJsonInitialized = true;
        }
        printf("\t\t\t\"length\":%d,\n", _dataLength);
        if (_printIntervals) {
            printf("\t\t\t\"intervals\":[\n");
            _controlJsonIntervals = true;
        }
        break;
    case LEGACY:
        if (_printHeaders && _printIntervals) {
            printf("\n\n********** New data length is %d\n", _dataLength);
        }
        break;
    }
}

void PerftestPrinter::print_latency_interval(
        unsigned long latency,
        double latencyAve,
        double latencyStd,
        unsigned long latencyMin,
        unsigned long latencyMax,
        double outputCpu)
{
    switch (_outputFormat) {
    case DDS:
        ddslogger.writeMessage(_dataLength, latency, -1);
    case CSV:
        printf("%14d,%13lu,%9.0lf,%9.1lf,%9lu,%9lu",
               _dataLength,
               latency,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax);
        if (_showCPU) {
            printf(",%8.2f", outputCpu);
        }
        printf("\n");
        break;
    case JSON:
        if (_controlJsonIntervals) {
            _controlJsonIntervals = false;
        } else {
            printf(",");
        }
        printf("\n\t\t\t\t{\n"
               "\t\t\t\t\t\"latency\": %lu,\n"
               "\t\t\t\t\t\"latency_ave\": %1.2lf,\n"
               "\t\t\t\t\t\"latency_std\": %1.2lf,\n"
               "\t\t\t\t\t\"latency_min\": %lu,\n"
               "\t\t\t\t\t\"latency_max\": %lu",
               latency,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax);
        if (_showCPU) {
            printf(",\n\t\t\t\t\t\"cpu\": %1.2f", outputCpu);
        }
        printf("\n\t\t\t\t}");
        break;
    case LEGACY:
        printf("One way Latency: %6lu " PERFT_TIME_UNIT
               " Ave %6.0lf " PERFT_TIME_UNIT
               " Std %6.1lf " PERFT_TIME_UNIT
               " Min %6lu " PERFT_TIME_UNIT
               " Max %6lu " PERFT_TIME_UNIT,
               latency,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax);
        if (_showCPU) {
            printf(" CPU %1.2f (%%)", outputCpu);
        }
        printf("\n");
        break;
    }
}

void PerftestPrinter::print_latency_summary(
        int totalSampleSize,
        double latencyAve,
        double latencyStd,
        unsigned long latencyMin,
        unsigned long latencyMax,
        unsigned long *latencyHistory,
        unsigned long long count,
        double serializeTime,
        double deserializeTime,
        double outputCpu)
{
    switch (_outputFormat) {
    case DDS:
    case CSV:
        if (_printSummaryHeaders && _printHeaders) {
            if (!_printIntervals && _printSummaryHeaders) {
                _printSummaryHeaders = _printIntervals;
            }
            printf("\nOne-way Latency Summary:\n");
            printf("Length (Bytes)"
                   ", Ave (" PERFT_TIME_UNIT
                   "), Std (" PERFT_TIME_UNIT
                   "), Min (" PERFT_TIME_UNIT
                   "), Max (" PERFT_TIME_UNIT
                   "), 50%% (" PERFT_TIME_UNIT
                   "), 90%% (" PERFT_TIME_UNIT
                   "), 99%% (" PERFT_TIME_UNIT
                   "), 99.99%% (" PERFT_TIME_UNIT
                   "), 99.9999%% (" PERFT_TIME_UNIT
                   ")");
            if (_printSerialization) {
                printf(", Serialization (" PERFT_TIME_UNIT
                       "), Deserialization (" PERFT_TIME_UNIT
                       "), Total (" PERFT_TIME_UNIT ")");
            }
            if (_showCPU) {
                printf(", CPU (%%)");
            }
            printf("\n");
        }
        printf("%14d,%9.0lf,%9.1lf,%9lu,%9lu,%9lu,%9lu,%9lu,%12lu,%14lu",
               totalSampleSize,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax,
               latencyHistory[count * 50 / 100],
               latencyHistory[count * 90 / 100],
               latencyHistory[count * 99 / 100],
               latencyHistory[(int) (count * (9999.0 / 10000))],
               latencyHistory[(int) (count * (999999.0 / 1000000))]);
        if (_printSerialization) {
            printf(",%19.3f,%21.3f,%11.3f",
                   serializeTime,
                   deserializeTime,
                   serializeTime + deserializeTime);
        }
        if (_showCPU) {
            printf(",%8.2f", outputCpu);
        }
        printf("\n");
        break;
    case JSON:
        if (_printIntervals) {
            printf("\n\t\t\t],\n");
        }
        printf("\t\t\t\"summary\":{\n"
               "\t\t\t\t\"latency_ave\": %1.2lf,\n"
               "\t\t\t\t\"latency_std\": %1.2lf,\n"
               "\t\t\t\t\"latency_min\": %lu,\n"
               "\t\t\t\t\"latency_max\": %lu,\n"
               "\t\t\t\t\"latency_50\": %lu,\n"
               "\t\t\t\t\"latency_90\": %lu,\n"
               "\t\t\t\t\"latency_99\": %lu,\n"
               "\t\t\t\t\"latency_99.99\": %lu,\n"
               "\t\t\t\t\"latency_99.9999\": %lu",
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax,
               latencyHistory[count * 50 / 100],
               latencyHistory[count * 90 / 100],
               latencyHistory[count * 99 / 100],
               latencyHistory[(int) (count * (9999.0 / 10000))],
               latencyHistory[(int) (count * (999999.0 / 1000000))]);
        if (_printSerialization) {
            printf(",\n\t\t\t\t\"serialize\": %1.3f,\n"
                   "\t\t\t\t\"deserialize\": %1.3f,\n"
                   "\t\t\t\t\"total_s\": %1.3f",
                   serializeTime,
                   deserializeTime,
                   serializeTime + deserializeTime);
        }
        if (_showCPU) {
            printf(",\n\t\t\t\t\"cpu\": %1.2f", outputCpu);
        }
        printf("\n\t\t\t}\n\t\t}");
        break;
    case LEGACY:
        printf("Length: %5d"
               " Latency: Ave %6.0lf " PERFT_TIME_UNIT
               " Std %6.1lf " PERFT_TIME_UNIT
               " Min %6lu " PERFT_TIME_UNIT
               " Max %6lu " PERFT_TIME_UNIT
               " 50%% %6lu " PERFT_TIME_UNIT
               " 90%% %6lu " PERFT_TIME_UNIT
               " 99%% %6lu " PERFT_TIME_UNIT
               " 99.99%% %6lu " PERFT_TIME_UNIT
               " 99.9999%% %6lu " PERFT_TIME_UNIT,
               totalSampleSize,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax,
               latencyHistory[count * 50 / 100],
               latencyHistory[count * 90 / 100],
               latencyHistory[count * 99 / 100],
               latencyHistory[(int) (count * (9999.0 / 10000))],
               latencyHistory[(int) (count * (999999.0 / 1000000))]);
        if (_showCPU) {
            printf(" CPU %1.2f (%%)", outputCpu);
        }
        printf("\n");
        if (_printSerialization) {
            printf("Serialization/Deserialization: %0.3f us / %0.3f us / "
                   "TOTAL: "
                   "%0.3f us\n",
                   serializeTime,
                   deserializeTime,
                   serializeTime + deserializeTime);
        }
        break;
    }
}

void PerftestPrinter::print_latency_summary(
        double latencyAve,
        double latencyStd,
        unsigned long latencyMin,
        unsigned long latencyMax,
        unsigned long *latencyHistory,
        unsigned long long count,
        double outputCpu)
{
    switch (_outputFormat) {
    case DDS:
        // In other cases we set the latency to be printed every sample, that could
        // be hard here.
        break;
    case CSV:
        if (_printSummaryHeaders && _printHeaders) {
            if (!_printIntervals && _printSummaryHeaders) {
                _printSummaryHeaders = _printIntervals;
            }
            printf("\nOne-way Latency Summary:\n");
            printf("Length (Bytes)"
                   ", Ave (" PERFT_TIME_UNIT
                   "), Std (" PERFT_TIME_UNIT
                   "), Min (" PERFT_TIME_UNIT
                   "), Max (" PERFT_TIME_UNIT
                   "), 50%% (" PERFT_TIME_UNIT
                   "), 90%% (" PERFT_TIME_UNIT
                   "), 99%% (" PERFT_TIME_UNIT
                   "), 99.99%% (" PERFT_TIME_UNIT
                   "), 99.9999%% (" PERFT_TIME_UNIT
                   ")");
            if (_showCPU) {
                printf(", CPU (%%)");
            }
            printf("\n");
        }
        printf("%14d,%9.0lf,%9.1lf,%9lu,%9lu,%9lu,%9lu,%9lu,%12lu,%14lu",
               _dataLength,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax,
               latencyHistory[count * 50 / 100],
               latencyHistory[count * 90 / 100],
               latencyHistory[count * 99 / 100],
               latencyHistory[(int) (count * (9999.0 / 10000))],
               latencyHistory[(int) (count * (999999.0 / 1000000))]);
        if (_showCPU) {
            printf(",%8.2f", outputCpu);
        }
        printf("\n");
        break;
    case JSON:
        if (_printIntervals) {
            printf("\n\t\t\t],\n");
        }
        printf("\t\t\t\"summary\":{\n"
               "\t\t\t\t\"latency_ave\": %1.2lf,\n"
               "\t\t\t\t\"latency_std\": %1.2lf,\n"
               "\t\t\t\t\"latency_min\": %lu,\n"
               "\t\t\t\t\"latency_max\": %lu,\n"
               "\t\t\t\t\"latency_50\": %lu,\n"
               "\t\t\t\t\"latency_90\": %lu,\n"
               "\t\t\t\t\"latency_99\": %lu,\n"
               "\t\t\t\t\"latency_99.99\": %lu,\n"
               "\t\t\t\t\"latency_99.9999\": %lu",
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax,
               latencyHistory[count * 50 / 100],
               latencyHistory[count * 90 / 100],
               latencyHistory[count * 99 / 100],
               latencyHistory[(int) (count * (9999.0 / 10000))],
               latencyHistory[(int) (count * (999999.0 / 1000000))]);
        if (_showCPU) {
            printf(",\n\t\t\t\t\"cpu\": %1.2f", outputCpu);
        }
        printf("\n\t\t\t}\n\t\t}");
        break;
    case LEGACY:
        printf("Length: %5d"
               " Latency: Ave %6.0lf " PERFT_TIME_UNIT
               " Std %6.1lf " PERFT_TIME_UNIT
               " Min %6lu " PERFT_TIME_UNIT
               " Max %6lu " PERFT_TIME_UNIT
               " 50%% %6lu " PERFT_TIME_UNIT
               " 90%% %6lu " PERFT_TIME_UNIT
               " 99%% %6lu " PERFT_TIME_UNIT
               " 99.99%% %6lu " PERFT_TIME_UNIT
               " 99.9999%% %6lu " PERFT_TIME_UNIT,
               _dataLength,
               latencyAve,
               latencyStd,
               latencyMin,
               latencyMax,
               latencyHistory[count * 50 / 100],
               latencyHistory[count * 90 / 100],
               latencyHistory[count * 99 / 100],
               latencyHistory[(int) (count * (9999.0 / 10000))],
               latencyHistory[(int) (count * (999999.0 / 1000000))]);
        if (_showCPU) {
            printf(" CPU %1.2f (%%)", outputCpu);
        }
        printf("\n");
        break;
    }
}

void PerftestPrinter::print_throughput_interval(
        unsigned long long lastMsgs,
        unsigned long long mps,
        double mpsAve,
        unsigned long long bps,
        double bpsAve,
        unsigned long long missingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
    switch (_outputFormat) {
    case DDS:
        ddslogger.writeMessage(_dataLength, -1, bpsAve * 8.0 / 1000.0 / 1000.0);
    case CSV:
        printf("%14d,%14llu,%11llu,%14.0lf,%9.1lf,%10.1lf, %12llu, %16.2lf",
               _dataLength,
               lastMsgs,
               mps,
               mpsAve,
               bps * 8.0 / 1000.0 / 1000.0,
               bpsAve * 8.0 / 1000.0 / 1000.0,
               missingPackets,
               missingPacketsPercent);
        if (_showCPU) {
            printf(",%8.2f", outputCpu);
        }
        printf("\n");
        break;

    case JSON:
        if (_controlJsonIntervals) {
            _controlJsonIntervals = false;
        } else {
            printf(",");
        }
        printf("\n\t\t\t\t{\n"
               "\t\t\t\t\t\"length\": %d,\n"
               "\t\t\t\t\t\"packets\": %llu,\n"
               "\t\t\t\t\t\"packets/s\": %llu,\n"
               "\t\t\t\t\t\"packets/s_ave\": %1.2lf,\n"
               "\t\t\t\t\t\"mbps\": %1.1lf,\n"
               "\t\t\t\t\t\"mbps_ave\": %1.1lf,\n"
               "\t\t\t\t\t\"lost\": %llu,\n"
               "\t\t\t\t\t\"lost_percent\": %1.2lf",
               _dataLength,
               lastMsgs,
               mps,
               mpsAve,
               bps * 8.0 / 1000.0 / 1000.0,
               bpsAve * 8.0 / 1000.0 / 1000.0,
               missingPackets,
               missingPacketsPercent);
        if (_showCPU) {
            printf(",\n\t\t\t\t\t\"cpu\": %1.2f", outputCpu);
        }
        printf("\n\t\t\t\t}");
        break;
    case LEGACY:
        printf("Packets: %8llu  Packets/s: %7llu  Packets/s(ave): %7.0lf  "
               "Mbps: %7.1lf  Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%)",
               lastMsgs,
               mps,
               mpsAve,
               bps * 8.0 / 1000.0 / 1000.0,
               bpsAve * 8.0 / 1000.0 / 1000.0,
               missingPackets,
               missingPacketsPercent);
        if (_showCPU) {
            printf(" CPU %1.2f (%%)", outputCpu);
        }
        printf("\n");
        break;
    }
}

void PerftestPrinter::print_throughput_summary(
        int length,
        unsigned long long intervalPacketsReceived,
        unsigned long long intervalTime,
        unsigned long long intervalBytesReceived,
        unsigned long long intervalMissingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
    switch (_outputFormat) {
    case DDS:
    case CSV:
        if (_printSummaryHeaders && _printHeaders) {
            if (!_printIntervals && _printSummaryHeaders) {
                _printSummaryHeaders = _printIntervals;
            }
            printf("\nThroughput Summary:\n");
            printf("Length (Bytes), Total Samples, Ave Samples/s,"
                   "    Ave Mbps, Lost Samples, Lost Samples (%%)");
            if (_showCPU) {
                printf(", CPU (%%)");
            }
            printf("\n");
        }
        printf("%14d,%14llu,%14.0llu,%12.1lf, %12llu, %16.2lf",
               length,
               intervalPacketsReceived,
               intervalPacketsReceived * 1000000 / intervalTime,
               intervalBytesReceived * 1000000.0 / intervalTime * 8.0 / 1000.0
                       / 1000.0,
               intervalMissingPackets,
               missingPacketsPercent);
        if (_showCPU) {
            printf(",%8.2f", outputCpu);
        }
        printf("\n");
        break;
    case JSON:
        if (_printIntervals) {
            printf("\t\t\t],\n");
        }
        printf("\t\t\t\"summary\":{\n"
               "\t\t\t\t\"packets\": %llu,\n"
               "\t\t\t\t\"packets/sAve\": %llu,\n"
               "\t\t\t\t\"mbpsAve\": %1.1lf,\n"
               "\t\t\t\t\"lost\": %llu,\n"
               "\t\t\t\t\"lostPercent\": %1.2lf",
               intervalPacketsReceived,
               intervalPacketsReceived * 1000000 / intervalTime,
               intervalBytesReceived * 1000000.0 / intervalTime * 8.0 / 1000.0
                       / 1000.0,
               intervalMissingPackets,
               missingPacketsPercent);
        if (_showCPU) {
            printf(",\n\t\t\t\t\"cpu\": %1.2f", outputCpu);
        }
        printf("\n\t\t\t}\n\t\t}");
        break;
    case LEGACY:
        printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  "
               "Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%)",
               length,
               intervalPacketsReceived,
               intervalPacketsReceived * 1000000 / intervalTime,
               intervalBytesReceived * 1000000.0 / intervalTime * 8.0 / 1000.0
                       / 1000.0,
               intervalMissingPackets,
               missingPacketsPercent);
        if (_showCPU) {
            printf(" CPU %1.2f (%%)", outputCpu);
        }
        printf("\n");
        break;
    }
}

void PerftestPrinter::print_initial_output()
{
    if (_outputFormat == JSON) {
        printf("{\"perftest\":\n\t[\n\t\t{\n");
    }
}

void PerftestPrinter::print_final_output()
{
    if (_outputFormat == JSON) {
        printf("\n\t]\n}\n");
    }
    if (_outputFormat == DDS) {
        ddslogger.finalize();
    }
}