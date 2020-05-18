/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#include "PerftestPrinter.h"

PerftestOuputFormat get_output_format(std::string outputFormat)
{
    if(outputFormat == "legacy"){
        return LEGACY;
    } else {
        return CSV;
    }
}

PerftestPrinter::PerftestPrinter()
{
    _dataLength = 100;
    _outputFormat = CSV;
    _printSummaryHeaders = true;
}

void PerftestPrinter::initialize(bool printIntervals,
        std::string outputFormat,
        bool printHeaders,
        bool printSerialization,
        bool showCpu)
{
    _printIntervals = printIntervals;
    _headerPrinted = printHeaders;
    _printSerialization = printSerialization;
    _showCPU = showCpu;
    if(outputFormat == "csv")
        _outputFormat = CSV;
    else if(outputFormat == "json")
        _outputFormat = JSON;
    else if(outputFormat == "legacy")
        _outputFormat = LEGACY;
}

void PerftestPrinter::set_data_length(unsigned int dataLength)
{
    _dataLength = dataLength;
}

void PerftestPrinter::set_header_printed(bool headerPrinted)
{
    _headerPrinted = headerPrinted;
}

void PerftestPrinter::print_latency_interval_header()
{
    if(_headerPrinted && _printIntervals){
        switch(_outputFormat){
            case CSV:
                printf("\nIntervals One-way Latency for %d Bytes:\n",
                _dataLength);
                printf("Length (Bytes)"
                        ", Latency (" PERFT_TIME_UNIT
                        "), Ave (" PERFT_TIME_UNIT
                        "), Std (" PERFT_TIME_UNIT
                        "), Min (" PERFT_TIME_UNIT
                        "), Max (" PERFT_TIME_UNIT ")");
                if(_showCPU)
                    printf(", CPU %%");

                printf("\n");
                break;
            case JSON:
                break;
            case LEGACY:
                printf("\n\n********** New data length is %d\n",
                        _dataLength);
                break;
        }
    }
}

void PerftestPrinter::print_throughput_header()
{
    if(_headerPrinted && _printIntervals){
        switch(_outputFormat){
            case CSV:
                printf("\nInterval Throughput for %d Bytes:\n",
                _dataLength);
                printf("Length (Bytes), Total Samples,  Samples/s,"
                        " Ave Samples/s,     Mbps,  Ave Mbps"
                        ", Lost Samples, Lost Samples (%%)");
                if(_showCPU)
                    printf(", CPU %%");

                printf("\n");
                break;
            case JSON:
                break;
            case LEGACY:
                printf("\n\n********** New data length is %d\n",
                        _dataLength);
                break;
        }
    }
}

void PerftestPrinter::print_latency_interval(unsigned long latency,
        double latency_ave,
        double latency_std,
        unsigned long latency_min,
        unsigned long latency_max,
        std::string outputCpu)
{
    switch (_outputFormat)
    {
        case CSV :
            printf("%14d,%13lu,%9.0lf,%9.1lf,%9lu,%9lu", _dataLength,
                    latency,
                    latency_ave,
                    latency_std,
                    latency_min,
                    latency_max);
            if(_showCPU){
                printf(",%6s", outputCpu.c_str());
            }
            printf("\n");
            break;

        case JSON :
            break;

        case LEGACY :
            printf("One way Latency: %6lu " PERFT_TIME_UNIT
                    " Ave %6.0lf " PERFT_TIME_UNIT
                    " Std %6.1lf " PERFT_TIME_UNIT
                    " Min %6lu " PERFT_TIME_UNIT
                    " Max %6lu " PERFT_TIME_UNIT,
                    latency,
                    latency_ave,
                    latency_std,
                    latency_min,
                    latency_max
                );
            if(_showCPU){
                printf(" CPU %s %%", outputCpu.c_str());
            }
            printf("\n");
            break;
    }
}

void PerftestPrinter::print_latency_summary(int total_sample_size,
        double latency_ave,
        double latency_std,
        unsigned long latency_min,
        unsigned long latency_max,
        unsigned long *_latency_history,
        unsigned long long count,
        double serializeTime,
        double deserializeTime,
        std::string outputCpu)
{
    switch (_outputFormat)
    {
        case CSV :
            if (_printSummaryHeaders && _headerPrinted) {
                if(!_printIntervals && _printSummaryHeaders)
                    _printSummaryHeaders = _printIntervals;
                printf ("\nOne-way Latency Summary:\n");
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
                if(_printSerialization)
                    printf(", Serialization (" PERFT_TIME_UNIT
                            "), Deserialization (" PERFT_TIME_UNIT
                            "), Total (" PERFT_TIME_UNIT
                            ")");
                if(_showCPU)
                    printf(", CPU %%");

                printf("\n");
            }
            printf("%14d,%9.0lf,%9.1lf,%9lu,%9lu,%9lu,%9lu,%9lu,%12lu,%14lu",
                    total_sample_size,
                    latency_ave,
                    latency_std,
                    latency_min,
                    latency_max,
                    _latency_history[count*50/100],
                    _latency_history[count*90/100],
                    _latency_history[count*99/100],
                    _latency_history[(int)(count*(9999.0/10000))],
                    _latency_history[(int)(count*(999999.0/1000000))]);
            if(_printSerialization){
                printf(",%19.3f,%21.3f,%11.3f",
                    serializeTime,
                    deserializeTime,
                    serializeTime+deserializeTime);
            }
            if(_showCPU){
                printf(",%6s", outputCpu.c_str());
            }
            printf("\n");
            break;

        case JSON :
            break;

        case LEGACY :
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
                    total_sample_size,
                    latency_ave, latency_std, latency_min, latency_max,
                    _latency_history[count*50/100],
                    _latency_history[count*90/100],
                    _latency_history[count*99/100],
                    _latency_history[(int)(count*(9999.0/10000))],
                    _latency_history[(int)(count*(999999.0/1000000))]);
            if(_showCPU){
                printf(" CPU %s %%", outputCpu.c_str());
            }
            printf("\n");
            if(_printSerialization){
                printf("Serialization/Deserialization: %0.3f us / %0.3f us / "
                        "TOTAL: "
                        "%0.3f us\n",
                        serializeTime,
                        deserializeTime,
                        serializeTime + deserializeTime
                );
            }

            break;
    }
}

void PerftestPrinter::print_throughput(unsigned long long last_msgs,
        unsigned long long mps,
        double mps_ave,
        unsigned long long bps,
        double bps_ave,
        unsigned long long missing_packets,
        float missing_packets_percent,
        std::string outputCpu)
{
    switch (_outputFormat)
    {
        case CSV :
            printf("%14d,%14llu,%11llu,%14.0lf,%9.1lf,%10.1lf, %12llu, %16.2lf",
                    _dataLength,
                    last_msgs, mps, mps_ave,
                    bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                    missing_packets,
                    missing_packets_percent);
            if(_showCPU){
                printf(",%6s", outputCpu.c_str());
            }
            printf("\n");
            break;

        case JSON :
            break;

        case LEGACY :
            printf("Packets: %8llu  Packets/s: %7llu  Packets/s(ave): %7.0lf  "
                    "Mbps: %7.1lf  Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%)",
                    last_msgs, mps, mps_ave,
                    bps * 8.0 / 1000.0 / 1000.0, bps_ave * 8.0 / 1000.0 / 1000.0,
                    missing_packets,
                    missing_packets_percent
                );
            if(_showCPU){
                printf(" CPU %s %%", outputCpu.c_str());
            }
            printf("\n");
            break;
    }
}

void PerftestPrinter::print_throughput_summary(int length,
        unsigned long long interval_packets_received,
        unsigned long long interval_time,
        unsigned long long interval_bytes_received,
        unsigned long long interval_missing_packets,
        float missing_packets_percent,
        std::string outputCpu)
{
    switch (_outputFormat)
    {
        case CSV :
            if (_printSummaryHeaders && _headerPrinted) {
                if(!_printIntervals && _printSummaryHeaders)
                    _printSummaryHeaders = _printIntervals;
                printf ("\nThroughput Summary:\n");
                printf("Length (Bytes), Total Samples, Ave Samples/s,"
                        "    Ave Mbps, Lost Samples, Lost Samples (%%)");
                if(_showCPU)
                    printf(", CPU %%");

                printf("\n");
            }
            printf("%14d,%14llu,%14.0llu,%12.1lf, %12llu, %16.2lf",
                    length,
                    interval_packets_received,
                    interval_packets_received*1000000/interval_time,
                    interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                    interval_missing_packets,
                    missing_packets_percent
            );
            if(_showCPU){
                printf(",%6s", outputCpu.c_str());
            }
            printf("\n");
            break;

        case JSON :
            break;

        case LEGACY :
            printf("Length: %5d  Packets: %8llu  Packets/s(ave): %7llu  "
                   "Mbps(ave): %7.1lf  Lost: %5llu (%1.2f%%)",
                   length,
                   interval_packets_received,
                   interval_packets_received*1000000/interval_time,
                   interval_bytes_received*1000000.0/interval_time*8.0/1000.0/1000.0,
                   interval_missing_packets,
                   missing_packets_percent
            );
            if(_showCPU){
                printf(" CPU %s %%", outputCpu.c_str());
            }
            printf("\n");
            break;
    }
}