



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

enum PerftestOuputType {
    CSV,
    JSON,
    REGULAR
};

PerftestOuputType get_output_format(std::string outputFormat);

class PerftestPrinter {
    private:
        unsigned int _dataLength;
        bool _headerPrinted;
        bool  _showCPU; // Control -showcpu flag?
        std::string _latencyIntervalHeader;
        PerftestOuputType _outputType;
        std::string _filename;
        std::ofstream outputFile;

    public:
        PerftestPrinter();
        void setPerftestPrinter(bool headerPrinted, std::string outputType);
        ~PerftestPrinter(){ }

        // Set and Get members
        void set_data_length(unsigned int dataLength);

        // Methods
        void print_pub(unsigned long latency, double latency_ave, double latency_std,
                unsigned long latency_min, unsigned long latency_max, std::string outputCpu);


};


/**
 * TODO
 * - Control "-scan -noprint"
 *   This print just the summary part
 * - Discuss structure with string headers save in the class
 * - One more enum for select what type (pub, sub, pub_summary, sub_summary)
 * - Move Serialization/Deserialization to new row
 * - Have differents constructs for each case (no name, no type, just use the default one and not have to indicate)
 * - Use enum type?
 * - Add delete file option
 * - Types of Heades:
 *  -
 *    Data Length, Packets, Packets/s, Packets/s(ave), Mbps, Mbps(ave), Lost, Lost(%)
 */



#endif // __PERFTESTPRINTER_H__
