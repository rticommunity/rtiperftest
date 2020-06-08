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
    long appId;
    unsigned long long packets;
    unsigned long long packetsS; // packets/s
    double pAve;
    double mbps;
    double mbpsAve;
    unsigned long long lost;
    float lostPercent;
    bool interval;
    double outputCpu;

    void set_interval(
            unsigned long long packets,
            unsigned long long packetsS,
            double pAve,
            unsigned long long bps,
            double bpsAve,
            unsigned long long lost);

    void set_summary(
            unsigned long long packets,
            unsigned long long time,
            unsigned long long bytes,
            unsigned long long lost);
};
struct LatencyInfo {
    unsigned long dataLength;
    long appId;
    unsigned long long latency;
    double ave;
    double std;
    unsigned long long min;
    unsigned long long max;
    unsigned long long h50;
    unsigned long long h90;
    unsigned long long h99;
    unsigned long long h9999;
    unsigned long long h999999;
    double serialize;
    double deserialize;
    double total;
    bool interval;
    double outputCpu;

    void set_interval(
            unsigned long latency,
            double ave,
            double std,
            unsigned long min,
            unsigned long max);

    void set_summary(
            double ave,
            double std,
            unsigned long min,
            unsigned long max,
            unsigned long *history,
            unsigned long long count,
            double serialize,
            double deserialize);
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

    PerftestPrinter() : _dataLength(100), _printSummaryHeaders(true) {};
    virtual ~PerftestPrinter() {};

    virtual void initialize(ParameterManager *_PM);

    virtual void print_latency_header() = 0;

    virtual void print_latency_interval(LatencyInfo latInfo) = 0;

    virtual void print_latency_summary(LatencyInfo latInfo) = 0;

    virtual void print_throughput_header() = 0;

    virtual void print_throughput_interval(ThroughputInfo thInfo) = 0;

    virtual void print_throughput_summary(ThroughputInfo thInfo) = 0;

    void print_initial_output() {};

    void print_final_output() {};
};

class PerftestCSVPrinter : public  PerftestPrinter {
public:
    ~PerftestCSVPrinter() {};
    void print_latency_header();
    void print_latency_interval(LatencyInfo latInfo);
    void print_latency_summary(LatencyInfo latInfo);

    void print_throughput_header();
    void print_throughput_interval(ThroughputInfo thInfo);
    void print_throughput_summary(ThroughputInfo thInfo);
};

class PerftestJSONPrinter : public  PerftestPrinter {

private:
    bool _isJsonInitialized;
    bool _controlJsonIntervals;

public:

    void initialize(ParameterManager *_PM) override
    {
        PerftestPrinter::initialize(_PM);
        _isJsonInitialized = false;
    };

    ~PerftestJSONPrinter() {};
    void print_latency_header();
    void print_latency_interval(LatencyInfo latInfo);
    void print_latency_summary(LatencyInfo latInfo);
    void print_throughput_header();
    void print_throughput_interval(ThroughputInfo thInfo);
    void print_throughput_summary(ThroughputInfo thInfo);
    void print_initial_output();
    void print_final_output();
};

class PerftestLegacyPrinter: public PerftestPrinter {

    ~PerftestLegacyPrinter() {};
    void print_latency_header();
    void print_latency_interval(LatencyInfo latInfo);
    void print_latency_summary(LatencyInfo latInfo);

    void print_throughput_header();
    void print_throughput_interval(ThroughputInfo thInfo);
    void print_throughput_summary(ThroughputInfo thInfo);

    void print_initial_output() {};

    void print_final_output() {};
};

#endif  // __PERFTESTPRINTER_H__
