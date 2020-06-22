/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestPrinter.h"

void ThroughputInfo::set_interval(
        unsigned long long packets,
        unsigned long long packetsPerSecond,
        double packetsAverage,
        unsigned long long bps,
        double bpsAve,
        unsigned long long lostPackets)
{
    this->packets      = packets;
    this->packetsPerSecond     = packetsPerSecond;
    this->packetsAverage         = packetsAverage;
    this->mbps         = bps * 8.0 / 1000.0 / 1000.0;
    this->mbpsAve      = bpsAve * 8.0 / 1000.0 / 1000.0;
    this->lostPackets         = lostPackets;
    // Calculations of missing package percent
    if (packets + lostPackets != 0) {
        this->lostPacketsPercent = (float) ((lostPackets * 100.0)
                / (float) (packets + lostPackets));
    } else {
        this->lostPacketsPercent = 0.0;
    }
    this->interval = true;
}

void ThroughputInfo::set_summary(
        unsigned long long packets,
        unsigned long long time,
        unsigned long long bytes,
        unsigned long long lostPackets)
{
    this->packets  = packets;
    this->packetsPerSecond     = packets * 1000000 / time;
    this->mbpsAve  = bytes * 1000000.0 / time * 8.0 / 1000.0
            / 1000.0;
    this->lostPackets     = lostPackets;
    if (packets + lostPackets != 0) {
        this->lostPacketsPercent = (float) ((lostPackets * 100.0)
                / (float) (packets + lostPackets));
    } else {
        this->lostPacketsPercent = 0.0;
    }
    this->interval = false;
}

void LatencyInfo::set_interval(
        unsigned long latency,
        double average,
        double std,
        unsigned long minimum,
        unsigned long maximum)
{
    this->latency  = latency;
    this->average      = average;
    this->std      = std;
    this->minimum      = minimum;
    this->maximum      = maximum;
    this->interval = true;
}

void LatencyInfo::set_summary(
        double average,
        double std,
        unsigned long minimum,
        unsigned long maximum,
        unsigned long *history,
        unsigned long long count,
        double serializeTime,
        double deserializeTime)
{
    this->average      = average;
    this->std      = std;
    this->minimum      = minimum;
    this->maximum      = maximum;
    this->percentile50      = history[count * 50 / 100];
    this->percentile90      = history[count * 90 / 100];
    this->percentile99      = history[count * 99 / 100];
    this->percentile9999    = history[count * 9999 / 10000];
    this->percentile999999  = history[count * 999999 / 1000000];
    this->serializeTime = serializeTime;
    this->deserializeTime = deserializeTime;
    this->interval = false;
}

void PerftestPrinter::initialize(ParameterManager *_PM)
{
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

void PerftestCSVPrinter::print_latency_interval(LatencyInfo latencyInfo)
{
    printf("%14d,%13llu,%9.0lf,%9.1lf,%9llu,%9llu",
            _dataLength,
            latencyInfo.latency,
            latencyInfo.average,
            latencyInfo.std,
            latencyInfo.minimum,
            latencyInfo.maximum);
    if (_showCPU) {
        printf(",%8.2f", latencyInfo.outputCpu);
    }
    printf("\n");
}

void PerftestCSVPrinter::print_latency_summary(LatencyInfo latencyInfo)
{
    if (_printSummaryHeaders && _printHeaders) {
        if (!_printIntervals && _printSummaryHeaders) {
            _printSummaryHeaders = _printIntervals;
        }
        if (_printIntervals) {
            printf("\nOne-way Latency Summary:\n");
        }
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
    printf("%14d,%9.0lf,%9.1lf,%9llu,%9llu,%9llu,%9llu,%9llu,%12llu,%14llu",
            _dataLength,
            latencyInfo.average,
            latencyInfo.std,
            latencyInfo.minimum,
            latencyInfo.maximum,
            latencyInfo.percentile50,
            latencyInfo.percentile90,
            latencyInfo.percentile99,
            latencyInfo.percentile9999,
            latencyInfo.percentile999999);
    if (_printSerialization) {
        printf(",%19.3f,%21.3f,%11.3f",
                latencyInfo.serializeTime,
                latencyInfo.deserializeTime,
                latencyInfo.serializeTime + latencyInfo.deserializeTime);
    }
    if (_showCPU) {
        printf(",%8.2f", latencyInfo.outputCpu);
    }
    printf("\n");
}

void PerftestCSVPrinter::print_throughput_interval(ThroughputInfo throughputInfo)
{
    printf("%14d,%14llu,%11llu,%14.0lf,%9.1lf,%10.1lf, %12llu, %16.2lf",
            _dataLength,
            throughputInfo.packets,
            throughputInfo.packetsPerSecond,
            throughputInfo.packetsAverage,
            throughputInfo.mbps,
            throughputInfo.mbpsAve,
            throughputInfo.lostPackets,
            throughputInfo.lostPacketsPercent);
    if (_showCPU) {
        printf(",%8.2f", throughputInfo.outputCpu);
    }
    printf("\n");
}

void PerftestCSVPrinter::print_throughput_summary(ThroughputInfo throughputInfo)
{
    if (_printSummaryHeaders && _printHeaders) {
        if (!_printIntervals && _printSummaryHeaders) {
            _printSummaryHeaders = _printIntervals;
        }
        if (_printIntervals) {
            printf("\nThroughput Summary:\n");
        }
        printf("Length (Bytes), Total Samples, Ave Samples/s,"
                "    Ave Mbps, Lost Samples, Lost Samples (%%)");
        if (_showCPU) {
            printf(", CPU (%%)");
        }
        printf("\n");
    }
    printf("%14d,%14llu,%14.0llu,%12.1lf, %12llu, %16.2lf",
            _dataLength,
            throughputInfo.packets,
            throughputInfo.packetsPerSecond,
            throughputInfo.mbpsAve,
            throughputInfo.lostPackets,
            throughputInfo.lostPacketsPercent);
    if (_showCPU) {
        printf(",%8.2f", throughputInfo.outputCpu);
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

void PerftestJSONPrinter::print_latency_interval(LatencyInfo latencyInfo)
{
    if (_controlJsonIntervals) {
        _controlJsonIntervals = false;
    } else {
        printf(",");
    }
    printf("\n\t\t\t\t{\n"
            "\t\t\t\t\t\"latency\": %llu,\n"
            "\t\t\t\t\t\"latency_ave\": %1.2lf,\n"
            "\t\t\t\t\t\"latency_std\": %1.2lf,\n"
            "\t\t\t\t\t\"latency_min\": %llu,\n"
            "\t\t\t\t\t\"latency_max\": %llu",
            latencyInfo.latency,
            latencyInfo.average,
            latencyInfo.std,
            latencyInfo.minimum,
            latencyInfo.maximum);
    if (_showCPU) {
        printf(",\n\t\t\t\t\t\"cpu\": %1.2f", latencyInfo.outputCpu);
    }
    printf("\n\t\t\t\t}");
}

void PerftestJSONPrinter::print_latency_summary(LatencyInfo latencyInfo)
{
    if (_printIntervals) {
        printf("\n\t\t\t],\n");
    }
    printf("\t\t\t\"summary\":{\n"
            "\t\t\t\t\"latency_ave\": %1.2lf,\n"
            "\t\t\t\t\"latency_std\": %1.2lf,\n"
            "\t\t\t\t\"latency_min\": %llu,\n"
            "\t\t\t\t\"latency_max\": %llu,\n"
            "\t\t\t\t\"latency_50\": %llu,\n"
            "\t\t\t\t\"latency_90\": %llu,\n"
            "\t\t\t\t\"latency_99\": %llu,\n"
            "\t\t\t\t\"latency_99.99\": %llu,\n"
            "\t\t\t\t\"latency_99.9999\": %llu",
            latencyInfo.average,
            latencyInfo.std,
            latencyInfo.minimum,
            latencyInfo.maximum,
            latencyInfo.percentile50,
            latencyInfo.percentile90,
            latencyInfo.percentile99,
            latencyInfo.percentile9999,
            latencyInfo.percentile999999);
    if (_printSerialization) {
        printf(",\n\t\t\t\t\"serialize\": %1.3f,\n"
                "\t\t\t\t\"deserialize\": %1.3f,\n"
                "\t\t\t\t\"total_s\": %1.3f",
                latencyInfo.serializeTime,
                latencyInfo.deserializeTime,
                latencyInfo.serializeTime + latencyInfo.deserializeTime);
    }
    if (_showCPU) {
        printf(",\n\t\t\t\t\"cpu\": %1.2f", latencyInfo.outputCpu);
    }
    printf("\n\t\t\t}\n\t\t}");
}

void PerftestJSONPrinter::print_throughput_interval(ThroughputInfo throughputInfo)
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
            throughputInfo.packets,
            throughputInfo.packetsPerSecond,
            throughputInfo.packetsAverage,
            throughputInfo.mbps,
            throughputInfo.mbpsAve,
            throughputInfo.lostPackets,
            throughputInfo.lostPacketsPercent);
    if (_showCPU) {
        printf(",\n\t\t\t\t\t\"cpu\": %1.2f", throughputInfo.outputCpu);
    }
    printf("\n\t\t\t\t}");
}

void PerftestJSONPrinter::print_throughput_summary(ThroughputInfo throughputInfo)
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
            throughputInfo.packets,
            throughputInfo.packetsPerSecond,
            throughputInfo.mbpsAve,
            throughputInfo.lostPackets,
            throughputInfo.lostPacketsPercent);
    if (_showCPU) {
        printf(",\n\t\t\t\t\"cpu\": %1.2f", throughputInfo.outputCpu);
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

void PerftestLegacyPrinter::print_latency_interval(LatencyInfo latencyInfo)
{
    printf("One way Latency: %6llu " PERFT_TIME_UNIT
            " Ave %6.0lf " PERFT_TIME_UNIT
            " Std %6.1lf " PERFT_TIME_UNIT
            " Min %6llu " PERFT_TIME_UNIT
            " Max %6llu " PERFT_TIME_UNIT,
            latencyInfo.latency,
            latencyInfo.average,
            latencyInfo.std,
            latencyInfo.minimum,
            latencyInfo.maximum);
    if (_showCPU) {
        printf(" CPU %1.2f (%%)", latencyInfo.outputCpu);
    }
    printf("\n");
}

void PerftestLegacyPrinter::print_latency_summary(LatencyInfo latencyInfo)
{
    printf("Length: %5d"
            " Latency: Ave %6.0lf " PERFT_TIME_UNIT
            " Std %6.1lf " PERFT_TIME_UNIT
            " Min %6llu " PERFT_TIME_UNIT
            " Max %6llu " PERFT_TIME_UNIT
            " 50%% %6llu " PERFT_TIME_UNIT
            " 90%% %6llu " PERFT_TIME_UNIT
            " 99%% %6llu " PERFT_TIME_UNIT
            " 99.99%% %6llu " PERFT_TIME_UNIT
            " 99.9999%% %6llu " PERFT_TIME_UNIT,
            _dataLength,
            latencyInfo.average,
            latencyInfo.std,
            latencyInfo.minimum,
            latencyInfo.maximum,
            latencyInfo.percentile50,
            latencyInfo.percentile90,
            latencyInfo.percentile99,
            latencyInfo.percentile9999,
            latencyInfo.percentile999999);
    if (_showCPU) {
        printf(" CPU %1.2f (%%)", latencyInfo.outputCpu);
    }
    printf("\n");
    if (_printSerialization) {
        printf("Serialization/Deserialization: %0.3f us / %0.3f us / "
                "TOTAL: "
                "%0.3f us\n",
                latencyInfo.serializeTime,
                latencyInfo.deserializeTime,
                latencyInfo.serializeTime + latencyInfo.deserializeTime);
    }
}

void PerftestLegacyPrinter::print_throughput_interval(ThroughputInfo throughputInfo)
{
    printf("Packets: %8llu  Packets/s: %7llu  Packets/s(ave): %7.0lf  "
            "Mbps: %7.1lf  Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%)",
            throughputInfo.packets,
            throughputInfo.packetsPerSecond,
            throughputInfo.packetsAverage,
            throughputInfo.mbps,
            throughputInfo.mbpsAve,
            throughputInfo.lostPackets,
            throughputInfo.lostPacketsPercent);
    if (_showCPU) {
        printf(" CPU %1.2f (%%)", throughputInfo.outputCpu);
    }
    printf("\n");
}

void PerftestLegacyPrinter::print_throughput_summary(ThroughputInfo throughputInfo)
{
    printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  "
            "Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%)",
            _dataLength,
            throughputInfo.packets,
            throughputInfo.packetsPerSecond,
            throughputInfo.mbpsAve,
            throughputInfo.lostPackets,
            throughputInfo.lostPacketsPercent);
    if (_showCPU) {
        printf(" CPU %1.2f (%%)", throughputInfo.outputCpu);
    }
    printf("\n");
}
