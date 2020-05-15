



/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __PERFTESTPRINTER_H__
#define __PERFTESTPRINTER_H__

#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include "Infrastructure_common.h"
#include <fstream>

enum PerftestOuputFormat {
    LEGACY,
    JSON,
    CSV
};

PerftestOuputFormat get_output_format(std::string outputFormat);

class PerftestPrinter {
    private:
        unsigned int _dataLength;
        bool _headerPrinted;
        bool  _showCPU;
        bool _printIntervals;
        bool _printSummaryHeaders;
        bool _printSerialization;
        // std::string _latencyIntervalHeader;
        PerftestOuputFormat _outputFormat;

    public:
        PerftestPrinter();
        ~PerftestPrinter(){ }

        void initialize(bool printIntervals,
                std::string outputFormat,
                bool printHeaders,
                bool printSerialization,
                bool ShowCpu);

        // Set and Get members
        void set_data_length(unsigned int dataLength);
        void set_header_printed(bool headerPrinted);
        // Methods
        void print_latency_interval(unsigned long latency,
                double latency_ave,
                double latency_std,
                unsigned long latency_min,
                unsigned long latency_max,
                std::string outputCpu);

        void print_latency_summary(int total_sample_size,
                double latency_ave,
                double latency_std,
                unsigned long latency_min,
                unsigned long latency_max,
                unsigned long *_latency_history,
                unsigned long long count,
                std::string outputCpu);

        void print_throughput(unsigned long long last_msgs,
                unsigned long long mps,
                double mps_ave,
                unsigned long long bps,
                double bps_ave,
                unsigned long long missing_packets,
                float missing_packets_percent,
                std::string outputCpu);

        void print_throughput_summary(int length,
                unsigned long long interval_packets_received,
                unsigned long long interval_time,
                unsigned long long interval_bytes_received,
                unsigned long long interval_missing_packets,
                float missing_packets_percent,
                std::string outputCpu);

};
#endif // __PERFTESTPRINTER_H__
