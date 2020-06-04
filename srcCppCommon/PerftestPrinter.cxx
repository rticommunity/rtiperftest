/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestPrinter.h"



void PerftestPrinter::initialize(ParameterManager *_PM)
{
    std::string outputFormat = _PM->get<std::string>("outputFormat");
    _printIntervals = !_PM->get<bool>("noPrintIntervals");
    _printHeaders = !_PM->get<bool>("noOutputHeaders");
    _printSerialization = _PM->get<bool>("serializationTime");
    _showCPU = _PM->get<bool>("cpu");
}

/******************************************************************************/
/* CSV Implementation                                                         */
/******************************************************************************/

void PerftestCSVPrinter::print_latency_header()
{
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
}

void PerftestCSVPrinter::print_throughput_header()
{
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
}

void PerftestCSVPrinter::print_latency_interval(
        unsigned long latency,
        double latencyAve,
        double latencyStd,
        unsigned long latencyMin,
        unsigned long latencyMax,
        double outputCpu)
{
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
}

void PerftestCSVPrinter::print_latency_summary(
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
}

void PerftestCSVPrinter::print_throughput_interval(
        unsigned long long lastMsgs,
        unsigned long long mps,
        double mpsAve,
        unsigned long long bps,
        double bpsAve,
        unsigned long long missingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
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
}

void PerftestCSVPrinter::print_throughput_summary(
        int length,
        unsigned long long intervalPacketsReceived,
        unsigned long long intervalTime,
        unsigned long long intervalBytesReceived,
        unsigned long long intervalMissingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
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
}

/******************************************************************************/
/* JSON Implementation                                                        */
/******************************************************************************/

void PerftestJSONPrinter::print_latency_header()
{
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
}

void PerftestJSONPrinter::print_throughput_header()
{
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
}

void PerftestJSONPrinter::print_latency_interval(
        unsigned long latency,
        double latencyAve,
        double latencyStd,
        unsigned long latencyMin,
        unsigned long latencyMax,
        double outputCpu)
{
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
}

void PerftestJSONPrinter::print_latency_summary(
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
}

void PerftestJSONPrinter::print_throughput_interval(
        unsigned long long lastMsgs,
        unsigned long long mps,
        double mpsAve,
        unsigned long long bps,
        double bpsAve,
        unsigned long long missingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
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
}

void PerftestJSONPrinter::print_throughput_summary(
        int length,
        unsigned long long intervalPacketsReceived,
        unsigned long long intervalTime,
        unsigned long long intervalBytesReceived,
        unsigned long long intervalMissingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
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
}

void PerftestJSONPrinter::print_initial_output()
{
    printf("{\"perftest\":\n\t[\n\t\t{\n");
}

void PerftestJSONPrinter::print_final_output()
{
    printf("\n\t]\n}\n");
}


/******************************************************************************/
/* LEGACY Implementation                                                      */
/******************************************************************************/

void PerftestLegacyPrinter::print_latency_header()
{
    if (_printHeaders && _printIntervals) {
        printf("\n\n********** New data length is %d\n", _dataLength);
    }
}

void PerftestLegacyPrinter::print_throughput_header()
{
    if (_printHeaders && _printIntervals) {
        printf("\n\n********** New data length is %d\n", _dataLength);
    }
}

void PerftestLegacyPrinter::print_latency_interval(
        unsigned long latency,
        double latencyAve,
        double latencyStd,
        unsigned long latencyMin,
        unsigned long latencyMax,
        double outputCpu)
{
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
}

void PerftestLegacyPrinter::print_latency_summary(
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
}

void PerftestLegacyPrinter::print_throughput_interval(
        unsigned long long lastMsgs,
        unsigned long long mps,
        double mpsAve,
        unsigned long long bps,
        double bpsAve,
        unsigned long long missingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
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
}

void PerftestLegacyPrinter::print_throughput_summary(
        int length,
        unsigned long long intervalPacketsReceived,
        unsigned long long intervalTime,
        unsigned long long intervalBytesReceived,
        unsigned long long intervalMissingPackets,
        float missingPacketsPercent,
        double outputCpu)
{
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
}
