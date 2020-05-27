/*
 * (c) 2005-2020  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

#ifndef __PERFTESTPRINTER_H__
#define __PERFTESTPRINTER_H__

#ifdef RTI_LANGUAGE_CPP_TRADITIONAL
  #include "Infrastructure_common.h"
#elif defined(RTI_LANGUAGE_CPP_MODERN)
  #define PERFT_TIME_UNIT "us"
#endif
#include <iostream>
#include <stdio.h>
#include <string>
#include "ParameterManager.h"

enum PerftestOuputFormat { LEGACY, JSON, CSV };

class PerftestPrinter {
private:
    unsigned int _dataLength;
    bool _printHeaders;
    bool _showCPU;
    bool _printIntervals;
    bool _printSummaryHeaders;
    bool _printSerialization;
    bool _isJsonInitialized;
    bool _controlJsonIntervals;
    PerftestOuputFormat _outputFormat;

public:
    PerftestPrinter();
    ~PerftestPrinter()
    {
    }

    void initialize(ParameterManager *_PM);


    // Set and Get members
    void set_data_length(unsigned int dataLength);

    void set_header_printed(bool printHeaders);

    // Methods
    void print_latency_header();

    void print_latency_interval(
            unsigned long latency,
            double latencyAve,
            double latencyStd,
            unsigned long latencyMin,
            unsigned long latencyMax,
            double outputCpu);

    void print_latency_summary(
            int totalSampleSize,
            double latencyAve,
            double latencyStd,
            unsigned long latencyMin,
            unsigned long latencyMax,
            unsigned long *latencyHistory,
            unsigned long long count,
            double serializeTime,
            double deserializeTime,
            double outputCpu);

    void print_latency_summary(
            double latencyAve,
            double latencyStd,
            unsigned long latencyMin,
            unsigned long latencyMax,
            unsigned long *latencyHistory,
            unsigned long long count,
            double outputCpu);

    void print_throughput_header();

    void print_throughput_interval(
            unsigned long long lastMsgs,
            unsigned long long mps,
            double mpsAve,
            unsigned long long bps,
            double bpsAve,
            unsigned long long missingPackets,
            float missingPacketsPercent,
            double outputCpu);

    void print_throughput_summary(
            int length,
            unsigned long long intervalPacketsReceived,
            unsigned long long intervalTime,
            unsigned long long intervalBytesReceived,
            unsigned long long intervalMissingPackets,
            float missingPacketsPercent,
            double outputCpu);

    void print_initial_output();

    void print_final_output();
};
#endif  // __PERFTESTPRINTER_H__
