



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
    CSV,
    JSON,
    REGULAR
};

PerftestOuputFormat get_output_format(std::string outputFormat);

class PerftestPrinter {
    private:
        unsigned int _dataLength;
        bool _headerPrinted;
        bool  _showCPU; // Control -showcpu flag?
        std::string _latencyIntervalHeader;
        PerftestOuputFormat _outputFormat;

    public:
        PerftestPrinter();
        ~PerftestPrinter(){ }

        // Set and Get members
        void set_data_length(unsigned int dataLength);
        void set_header_printed(bool headerPrinted);
        void set_output_format(std::string outputFormat);
        // Methods
        void print_pub(unsigned long latency, double latency_ave, double latency_std,
                unsigned long latency_min, unsigned long latency_max, std::string outputCpu);


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
