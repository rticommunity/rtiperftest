

/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "PerftestPrinter.h"

PerftestOuputFormat get_output_format(std::string outputFormat)
{
    if(outputFormat == "csv"){
        return PerftestOuputFormat::CSV;
    } else {
        return PerftestOuputFormat::REGULAR;
    }
}

PerftestPrinter::PerftestPrinter()
{
    _dataLength = 100;
    _outputFormat = PerftestOuputFormat::REGULAR;
}

void PerftestPrinter::set_header_printed(bool headerPrinted)
{
    _headerPrinted = headerPrinted;
}

void PerftestPrinter::set_output_format(std::string outputFormat)
{
    _outputFormat = get_output_format(outputFormat);
}

void PerftestPrinter::set_data_length(unsigned int dataLength)
{
    if (_outputFormat == PerftestOuputFormat::REGULAR){
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
        case CSV :
            if (_headerPrinted)
            {
                _headerPrinted = !_headerPrinted;
                printf("Data Length, Latency (" PERFT_TIME_UNIT
                        "), Ave (" PERFT_TIME_UNIT
                        "), Std (" PERFT_TIME_UNIT
                        "), Min (" PERFT_TIME_UNIT 
                        "), Max (" PERFT_TIME_UNIT ")\n");
            }
            printf("%11d,%13lu,%9.0lf,%9.1lf,%9lu,%9lu\n", _dataLength, latency, latency_ave, 
                    latency_std, latency_min, latency_max);
            break;
            
        case JSON :
            break;

        case REGULAR :
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