

/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "PerftestPrinter.h"

PerftestOuputFormat get_output_format(std::string outputFormat)
{
    if(outputFormat == "legacy"){
        return PerftestOuputFormat::LEGACY;
    } else {
        return PerftestOuputFormat::REGULAR;
    }
}

PerftestPrinter::PerftestPrinter()
{
    _dataLength = 100;
    _outputFormat = PerftestOuputFormat::REGULAR;
    _printSummaryHeaders = true;
}

void PerftestPrinter::set_header_printed(bool headerPrinted)
{
    _headerPrinted = headerPrinted;
}

void PerftestPrinter::set_print_invertals(bool printIntervals)
{
    _printIntervals = printIntervals;
}

void PerftestPrinter::set_output_format(std::string outputFormat)
{
    _outputFormat = get_output_format(outputFormat);
}

void PerftestPrinter::set_data_length(unsigned int dataLength)
{
    if (_outputFormat == PerftestOuputFormat::LEGACY){
        printf("\n\n********** New data length is %d\n",
                dataLength);
    } else {
        _dataLength = dataLength;
    }
}

void PerftestPrinter::print_pub(unsigned long latency, double latency_ave,
        double latency_std, unsigned long latency_min,
        unsigned long latency_max, std::string outputCpu)
{
    switch (_outputFormat)
    {
        case REGULAR :
            if (_headerPrinted)
            {
                _headerPrinted = !_headerPrinted;
                printf("Length (Bytes), Latency (" PERFT_TIME_UNIT
                        "), Ave (" PERFT_TIME_UNIT
                        "), Std (" PERFT_TIME_UNIT
                        "), Min (" PERFT_TIME_UNIT
                        "), Max (" PERFT_TIME_UNIT ")\n");
            }
            printf("%14d,%13lu,%9.0lf,%9.1lf,%9lu,%9lu\n", _dataLength,
                    latency,
                    latency_ave,
                    latency_std,
                    latency_min,
                    latency_max);
            break;

        case JSON :
            break;

        case LEGACY :
            printf("One way Latency: %6lu " PERFT_TIME_UNIT
                    " Ave %6.0lf " PERFT_TIME_UNIT
                    " Std %6.1lf " PERFT_TIME_UNIT
                    " Min %6lu " PERFT_TIME_UNIT
                    " Max %6lu " PERFT_TIME_UNIT
                    " %s\n",
                    latency,
                    latency_ave,
                    latency_std,
                    latency_min,
                    latency_max,
                    outputCpu.c_str()
                );
            break;
    }
}

void PerftestPrinter::print_pub_sum(int total_sample_size, double latency_ave, double latency_std,
                unsigned long latency_min, unsigned long latency_max,
                unsigned long *_latency_history, unsigned long long count, std::string outputCpu)
{
    switch (_outputFormat)
    {
        case REGULAR :
            if (_printSummaryHeaders) {
                if(!_printIntervals && _printSummaryHeaders)
                    _printSummaryHeaders = _printIntervals;
                printf("Length (Bytes), Ave (" PERFT_TIME_UNIT
                        "), Std (" PERFT_TIME_UNIT
                        "), Min (" PERFT_TIME_UNIT
                        "), Max (" PERFT_TIME_UNIT
                        "), 50%% (" PERFT_TIME_UNIT
                        "), 90%% (" PERFT_TIME_UNIT
                        "), 99%% (" PERFT_TIME_UNIT
                        "), 99.99%% (" PERFT_TIME_UNIT
                        "), 99.9999%% (" PERFT_TIME_UNIT
                        ")\n");
            }
            printf("%14d,%9.0lf,%9.1lf,%9lu,%9lu,%9lu,%9lu,%9lu,%12lu,%14lu\n",
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
                " 99.9999%% %6lu " PERFT_TIME_UNIT
                " %s\n",
                    total_sample_size,
                    latency_ave, latency_std, latency_min, latency_max,
                    _latency_history[count*50/100],
                    _latency_history[count*90/100],
                    _latency_history[count*99/100],
                    _latency_history[(int)(count*(9999.0/10000))],
                    _latency_history[(int)(count*(999999.0/1000000))],
                    outputCpu.c_str());
            break;
    }
}