package com.rti.perftest.harness;

public class PerftestCSVPrinter extends PerftestPrinter {
    @Override
    public void print_latency_header()
    {
        if(_printHeaders && _printIntervals) {
            System.out.printf("\nIntervals One-way Latency for %1d " +
                              "Bytes:\n", _dataLength);
            System.out.printf("Length (Bytes)" +
                              ", Latency (" + PERFT_TIME_UNIT +
                              "), Ave (" + PERFT_TIME_UNIT +
                              "), Std (" + PERFT_TIME_UNIT +
                              "), Min (" + PERFT_TIME_UNIT +
                              "), Max (" + PERFT_TIME_UNIT +
                              ")");
            if (_showCPU) {
                System.out.print(", CPU (%%)");
            }
        }
        System.out.print("\n");
        System.out.flush();
    }
    @Override
    public void print_throughput_header()
    {
        if(_printHeaders && _printIntervals) {
            System.out.printf("\nIntervals Throughput for %1d " +
                              "Bytes:\n", _dataLength);
            System.out.printf("Length (Bytes), Total Samples,  " +
                              "Samples/s, Ave Samples/s,     " +
                              "Mbps,  Ave Mbps" +
                              ", Lost Samples, Lost Samples (%%)");
            if (_showCPU) {
                System.out.printf(", CPU (%%)");
            }
        }
        System.out.printf("\n");
        System.out.flush();
    }
    @Override
    public void print_latency_interval(
        int latency,
        double latencyAve,
        double latencyStd,
        int latencyMin,
        int latencyMax,
        double outputCpu)
    {
        System.out.printf("%1$14d,%2$13d,%3$9.0f,%4$9.1f,%5$9d" +
                          ",%6$9d",
                          _dataLength,
                          latency,
                          latencyAve,
                          latencyStd,
                          latencyMin,
                          latencyMax);
        if (_showCPU) {
            System.out.printf(",%1$8.2f", outputCpu);
        }
        System.out.printf("\n");
        System.out.flush();
    }
    public void print_latency_summary(
        double latencyAve,
        double latencyStd,
        long latencyMin,
        long latencyMax,
        int[] latencyHistory,
        long count,
        double outputCpu)
    {
        if (_printSummaryHeaders && _printHeaders) {
            if (!_printIntervals && _printSummaryHeaders) {
                _printSummaryHeaders = _printIntervals;
            }
            if (_printIntervals) {
                System.out.printf("\nOne-way Latency Summary:\n");
            }
            System.out.printf("Length (Bytes)" +
                              ", Ave (" + PERFT_TIME_UNIT +
                              "), Std (" + PERFT_TIME_UNIT +
                              "), Min (" + PERFT_TIME_UNIT +
                              "), Max (" + PERFT_TIME_UNIT +
                              "), 50%% (" + PERFT_TIME_UNIT +
                              "), 90%% (" + PERFT_TIME_UNIT +
                              "), 99%% (" + PERFT_TIME_UNIT +
                              "), 99.99%% (" + PERFT_TIME_UNIT +
                              "), 99.9999%% (" +PERFT_TIME_UNIT +
                              ")");
            if (_showCPU) {
                System.out.printf(", CPU (%%)");
            }
            System.out.printf("\n");
        }
        System.out.printf("%1$14d,%2$9.0f,%3$9.1f,%4$9d,%5$9d," +
                        "%6$9d,%7$9d,%8$9d,%9$12d,%10$14d",
                        _dataLength,
                        latencyAve,
                        latencyStd,
                        latencyMin,
                        latencyMax,
                        latencyHistory[(int) (count * 50 / 100)],
                        latencyHistory[(int) (count * 90 / 100)],
                        latencyHistory[(int) (count * 99 / 100)],
                        latencyHistory[(int) (count *
                            (9999 / 10000))],
                        latencyHistory[(int) (count *
                            (999999 / 1000000))]);
        if (_showCPU) {
            System.out.printf(",%1$8.2f", outputCpu);
        }
        System.out.printf("\n");
        System.out.flush();
    }
    @Override
    public void print_throughput_interval(
        long lastMsgs,
        long mps,
        double mpsAve,
        long bps,
        double bpsAve,
        long missingPackets,
        double missingPacketsPercent,
        double outputCpu)
    {
        System.out.printf("%1$14d,%2$14d,%3$11d,%4$14.0f,%5$9.1f"+
                          ",%6$10.1f,%7$13d,%8$17.2f",
                          _dataLength,
                          lastMsgs,
                          mps,
                          mpsAve,
                          bps * 8.0 / 1000.0 / 1000.0,
                          bpsAve * 8.0 / 1000.0 / 1000.0,
                          missingPackets,
                          missingPacketsPercent);
        if (_showCPU) {
            System.out.printf(",%1$8.2f", outputCpu);
        }
        System.out.printf("\n");
        System.out.flush();
    }
    @Override
    public void print_throughput_summary(
        int length,
        long intervalPacketsReceived,
        long intervalTime,
        long intervalBytesReceived,
        long intervalMissingPackets,
        double missingPacketsPercent,
        double outputCpu)
    {
        if (_printSummaryHeaders && _printHeaders) {
            if (!_printIntervals && _printSummaryHeaders) {
                _printSummaryHeaders = _printIntervals;
            }
            if (_printIntervals) {
                System.out.printf("\nThroughput Summary:\n");
            }
            System.out.printf("Length (Bytes), Total Samples," +
                              " Ave Samples/s,    Ave Mbps, " +
                              "Lost Samples, Lost Samples (%%)");
            if (_showCPU) {
                System.out.printf(", CPU (%%)");
            }
            System.out.printf("\n");
        }
        System.out.printf("%1$14d,%2$14d,%3$14.0f,%4$12.1f," +
                          "%5$13d,%6$17.2f",
                          length,
                          intervalPacketsReceived,
                          intervalPacketsReceived * 1000000.0
                                  / intervalTime,
                          intervalBytesReceived * 1000000.0
                                  / intervalTime * 8.0 / 1000.0 / 1000.0,
                          intervalMissingPackets,
                          missingPacketsPercent);
        if (_showCPU) {
            System.out.printf(",%1$8.2f", outputCpu);
        }
        System.out.printf("\n");
        System.out.flush();
    }
}
