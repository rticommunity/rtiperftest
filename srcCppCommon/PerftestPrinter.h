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

struct ThroughputInfo {
    unsigned long dataLength;
    unsigned long long packets;
    unsigned long long packetsPerSecond;
    double packetsAverage;
    double mbps;
    double mbpsAve;
    unsigned long long lostPackets;
    float lostPacketsPercent;
    bool interval;
    double outputCpu;

    void set_interval(
            unsigned long long packets,
            unsigned long long packetsPerSecond,
            double packetsAverage,
            unsigned long long bps,
            double bpsAve,
            unsigned long long lostPackets);

    void set_summary(
            unsigned long long packets,
            unsigned long long time,
            unsigned long long bytes,
            unsigned long long lostPackets);
};
struct LatencyInfo {
    unsigned long dataLength;
    unsigned long long latency;
    double average;
    double std;
    unsigned long long minimum;
    unsigned long long maximum;
    unsigned long long percentile50;
    unsigned long long percentile90;
    unsigned long long percentile99;
    unsigned long long percentile9999;
    unsigned long long percentile999999;
    double serializeTime;
    double deserializeTime;
    bool interval;
    double outputCpu;

    void set_interval(
            unsigned long latency,
            double average,
            double std,
            unsigned long minimum,
            unsigned long maximum);

    void set_summary(
            double average,
            double std,
            unsigned long minimum,
            unsigned long maximum,
            unsigned long *history,
            unsigned long long count,
            double serializeTime,
            double deserializeTime);
};

class PerftestPrinter {

protected:
    bool _showCPU;
    bool _printIntervals;
    bool _printSummaryHeaders;
    bool _printSerialization;

public:
    unsigned int _dataLength;
    bool _printHeaders;

    PerftestPrinter() {_dataLength = 100; _printSummaryHeaders = true;}

    virtual ~PerftestPrinter() {};

    virtual void initialize(ParameterManager *_PM);

    virtual void print_latency_header() = 0;

    virtual void print_latency_interval(LatencyInfo latencyInfo) = 0;

    virtual void print_latency_summary(LatencyInfo latencyInfo) = 0;

    virtual void print_throughput_header() = 0;

    virtual void print_throughput_interval(ThroughputInfo throughputInfo) = 0;

    virtual void print_throughput_summary(ThroughputInfo throughputInfo) = 0;

    virtual void print_initial_output() {};

    virtual void print_final_output() {};
};

class PerftestCSVPrinter : public  PerftestPrinter {
public:
    ~PerftestCSVPrinter() {};
    void print_latency_header();
    void print_latency_interval(LatencyInfo latencyInfo);
    void print_latency_summary(LatencyInfo latencyInfo);

    void print_throughput_header();
    void print_throughput_interval(ThroughputInfo throughputInfo);
    void print_throughput_summary(ThroughputInfo throughputInfo);
};

class PerftestJSONPrinter : public  PerftestPrinter {

private:
    bool _isJsonInitialized;
    bool _controlJsonIntervals;

public:

    void initialize(ParameterManager *_PM)
    {
        PerftestPrinter::initialize(_PM);
        _isJsonInitialized = false;
        _controlJsonIntervals = false;
    };

    ~PerftestJSONPrinter() {};
    void print_latency_header();
    void print_latency_interval(LatencyInfo latencyInfo);
    void print_latency_summary(LatencyInfo latencyInfo);
    void print_throughput_header();
    void print_throughput_interval(ThroughputInfo throughputInfo);
    void print_throughput_summary(ThroughputInfo throughputInfo);
    void print_initial_output();
    void print_final_output();
};

class PerftestLegacyPrinter: public PerftestPrinter {

    ~PerftestLegacyPrinter() {};
    void print_latency_header();
    void print_latency_interval(LatencyInfo latencyInfo);
    void print_latency_summary(LatencyInfo latencyInfo);

    void print_throughput_header();
    void print_throughput_interval(ThroughputInfo throughputInfo);
    void print_throughput_summary(ThroughputInfo throughputInfo);

    void print_initial_output() {};

    void print_final_output() {};
};

#endif  // __PERFTESTPRINTER_H__
