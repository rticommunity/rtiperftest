package com.rti.perftest.harness;

public abstract class PerftestPrinter {
    public int _dataLength;
    public boolean _printHeaders;
    protected boolean _showCPU;
    protected boolean _printIntervals;
    protected boolean _printSummaryHeaders;
    public final String PERFT_TIME_UNIT = "us";

    public PerftestPrinter()
    {
        _dataLength = 100;
        _printSummaryHeaders = true;
    }
    public void initialize(
            boolean printIntervals,
            boolean printHeaders,
            boolean showCpu)
    {
        _printIntervals = printIntervals;
        _printHeaders = printHeaders;
        _showCPU = showCpu;
    }
    public void set_data_length(int dataLength)
    {
        _dataLength = dataLength;
    }
    public void set_header_printed(boolean printHeaders)
    {
        _printHeaders = printHeaders;
    }
    public abstract void print_latency_header();
    public abstract void print_throughput_header();
    public abstract void print_latency_interval(
        int latency,
        double latencyAve,
        double latencyStd,
        int latencyMin,
        int latencyMax,
        double outputCpu);
    
    public abstract void print_latency_summary(
        double latencyAve,
        double latencyStd,
        long latencyMin,
        long latencyMax,
        int[] latencyHistory,
        long count,
        double outputCpu);

    public abstract void print_throughput_interval(
        long lastMsgs,
        long mps,
        double mpsAve,
        long bps,
        double bpsAve,
        long missingPackets,
        double missingPacketsPercent,
        double outputCpu);

    public abstract void print_throughput_summary(
        int length,
        long intervalPacketsReceived,
        long intervalTime,
        long intervalBytesReceived,
        long intervalMissingPackets,
        double missingPacketsPercent,
        double outputCpu);
    
    public void print_initial_output(){}

    public void print_final_output(){}
}
