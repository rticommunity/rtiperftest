

/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */


#include "PerftestPrinter.h"

PerftestOuputType get_output_format(std::string outputFormat)
{
    if(outputFormat == "csv"){
        return PerftestOuputType::CSV;
    } else {
        return PerftestOuputType::REGULAR;
    }
}

PerftestPrinter::PerftestPrinter()
{
    _dataLength = 100;
    _outputType = PerftestOuputType::REGULAR;
}

void PerftestPrinter::setPerftestPrinter(bool headerPrinted,
        std::string outputType)
{
    _headerPrinted = !headerPrinted;
    _outputType = get_output_format(outputType);
    _filename = "outputData." + outputType;
}

void PerftestPrinter::set_data_length(unsigned int dataLength)
{
    if (_outputType == PerftestOuputType::REGULAR){
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
    switch (_outputType)
    {
        case PerftestOuputType::CSV :
            outputFile.open(_filename, std::ios::out | std::ios::app);

            if (!_headerPrinted)
            {
                _headerPrinted = true;
                outputFile << "Data Length, Latency, Ave, Std, Min, Max" << std::endl;
            }
            outputFile <<  _dataLength << "," << latency << "," << latency_ave << "," << latency_std << "," << latency_min << "," << latency_max << std::endl;
            outputFile.close();
            break;
            
        case PerftestOuputType::JSON :
            break;

        case PerftestOuputType::REGULAR :
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