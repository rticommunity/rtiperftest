



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
    REGULAR
};

PerftestOuputFormat get_output_format(std::string outputFormat);

class PerftestPrinter {
    private:
        unsigned int _dataLength;
        bool _headerPrinted;
        bool  _showCPU; // Control -showcpu flag?
        bool _printIntervals;       // Control header summary
        bool _printSummaryHeaders;  //
        std::string _latencyIntervalHeader;
        PerftestOuputFormat _outputFormat;

    public:
        PerftestPrinter();
        ~PerftestPrinter(){ }

        // Set and Get members
        void set_data_length(unsigned int dataLength);
        void set_header_printed(bool headerPrinted);
        void set_output_format(std::string outputFormat);
        void set_print_invertals(bool printIntervals);
        // Methods
        void print_pub(unsigned long latency, double latency_ave, double latency_std,
                unsigned long latency_min, unsigned long latency_max, std::string outputCpu);

        void print_pub_sum(int total_sample_size, double latency_ave, double latency_std,
                unsigned long latency_min, unsigned long latency_max,
                unsigned long *_latency_history, unsigned long long count, std::string outputCpu);

        void print_sub(unsigned long long last_msgs, unsigned long long mps, double mps_ave,
                unsigned long long bps, double bps_ave, unsigned long long missing_packets,
                float missing_packets_percent, std::string outputCpu);

        void print_sub_sum(int length, unsigned long long interval_packets_received,
                unsigned long long interval_time, unsigned long long interval_bytes_received,
                unsigned long long interval_missing_packets, float missing_packets_percent,
                std::string outputCpu);

};


/**
 * TODO
 * - Control "-scan -noprint"
 * - Move Serialization/Deserialization to new row
 * - Join noPrintHeaders noPrintSummary to noPrintText?
 * - Change fstream for '>' in exec time "use fprintf(stdout, "text")"
 * - outputFormat:  - Regular   (default form)
 *                  - csv
 *                  - json
 * - Not need another form to print on terminal, just use '>' or not use
 * - Create struct for send parameters
 * - 4 functions for each type ()
 * - Add to headers type data (us)
 * - Types of Heades:
 *      Data Length, Packets, Packets/s, Packets/s(ave), Mbps, Mbps(ave), Lost, Lost(%) -> Sub
 *
 */



#endif // __PERFTESTPRINTER_H__
