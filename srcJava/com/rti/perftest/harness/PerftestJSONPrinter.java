package com.rti.perftest.harness;

public class PerftestJSONPrinter extends PerftestPrinter{

    private boolean _isJsonInitialized;
    private boolean _controlJsonIntervals;
    
    @Override
    public void initialize(
            boolean printIntervals,
            boolean printHeaders,
            boolean showCpu)
    {
        super.initialize(printIntervals, printHeaders, showCpu);
        _isJsonInitialized = false;
        _controlJsonIntervals = false;
    }

    @Override
    public void print_latency_header()
    {
        if (_isJsonInitialized) {
            System.out.print(",\n\t\t{\n");
        } else {
            _isJsonInitialized = true;
        }
        System.out.printf("\t\t\t\"length\":%1d,\n", _dataLength);
        if (_printIntervals) {
            System.out.print("\t\t\t\"intervals\":[\n");
            _controlJsonIntervals = true;
        }
        System.out.flush();
    }

    @Override
    public void print_throughput_header()
    {
        if (_isJsonInitialized) {
            System.out.printf(",\n\t\t{\n");
        } else {
            _isJsonInitialized = true;
        }
        System.out.printf("\t\t\t\"length\":%1d,\n", _dataLength);
        if (_printIntervals) {
            System.out.printf("\t\t\t\"intervals\":[\n");
            _controlJsonIntervals = true;
        }
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
        if (_controlJsonIntervals) {
            _controlJsonIntervals = false;
        } else {
            System.out.printf(",");
        }
        System.out.printf("\n\t\t\t\t{\n" +
                          "\t\t\t\t\t\"latency\": %1d,\n" +
                          "\t\t\t\t\t\"latency_ave\": %2$1.0f,\n" +
                          "\t\t\t\t\t\"latency_std\": %3$1.1f,\n" +
                          "\t\t\t\t\t\"latency_min\": %4$1d,\n" +
                          "\t\t\t\t\t\"latency_max\": %5$1d",
                          latency,
                          latencyAve,
                          latencyStd,
                          latencyMin,
                          latencyMax);
        if (_showCPU) {
            System.out.printf(",\n\t\t\t\t\t\"cpu\": %1$1.2f",
                              outputCpu);
        }
        System.out.printf("\n\t\t\t\t}");
    }
    
    @Override
    public void print_latency_summary(
        double latencyAve,
        double latencyStd,
        long latencyMin,
        long latencyMax,
        int[] latencyHistory,
        long count,
        double outputCpu)
    {
        if (_printIntervals) {
            System.out.printf("\n\t\t\t],\n");
        }
        System.out.printf("\t\t\t\"summary\":{\n" +
                          "\t\t\t\t\"latency_ave\": %1$1.1f,\n" +
                          "\t\t\t\t\"latency_std\": %2$1.1f,\n" +
                          "\t\t\t\t\"latency_min\": %3$1d,\n" +
                          "\t\t\t\t\"latency_max\": %4$1d,\n" +
                          "\t\t\t\t\"latency_50\": %5$1d,\n" +
                          "\t\t\t\t\"latency_90\": %6$1d,\n" +
                          "\t\t\t\t\"latency_99\": %7$1d,\n" +
                          "\t\t\t\t\"latency_99.99\": %8$1d,\n" +
                          "\t\t\t\t\"latency_99.9999\": %9$1d",
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
            System.out.printf(",\n\t\t\t\t\"cpu\": %1$1.1f", outputCpu);
        }
        System.out.printf("\n\t\t\t}\n\t\t}");
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
        if (_controlJsonIntervals) {
            _controlJsonIntervals = false;
        } else {
            System.out.printf(",");
        }
        System.out.printf("\n\t\t\t\t{\n" +
                          "\t\t\t\t\t\"length\": %1d,\n" +
                          "\t\t\t\t\t\"packets\": %2d,\n" +
                          "\t\t\t\t\t\"packets/s\": %3d,\n" +
                          "\t\t\t\t\t\"packets/s_ave\": %4$1.2f,\n" +
                          "\t\t\t\t\t\"mbps\": %5$1.1f,\n" +
                          "\t\t\t\t\t\"mbps_ave\": %6$1.1f,\n" +
                          "\t\t\t\t\t\"lost\": %7$1d,\n" +
                          "\t\t\t\t\t\"lost_percent\": %8$1.2f",
                          _dataLength,
                          lastMsgs,
                          mps,
                          mpsAve,
                          bps * 8.0 / 1000.0 / 1000.0,
                          bpsAve * 8.0 / 1000.0 / 1000.0,
                          missingPackets,
                          missingPacketsPercent);
        if (_showCPU) {
            System.out.printf(",\n\t\t\t\t\t\"cpu\": %1$1.2f",
                              outputCpu);
        }
        System.out.printf("\n\t\t\t\t}");
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
        if (_printIntervals) {
            System.out.printf("\n\t\t\t],\n");
        }
        System.out.printf("\t\t\t\"summary\":{\n" +
                          "\t\t\t\t\"packets\": %1$1d,\n" +
                          "\t\t\t\t\"packets/sAve\": %2$1.0f,\n" +
                          "\t\t\t\t\"mbpsAve\": %3$1.1f,\n" +
                          "\t\t\t\t\"lost\": %4$1d,\n" +
                          "\t\t\t\t\"lostPercent\": %5$1.2f",
                          intervalPacketsReceived,
                          intervalPacketsReceived * 1000000.0
                                  / intervalTime,
                          intervalBytesReceived * 1000000.0
                                  / intervalTime * 8.0 / 1000.0 / 1000.0,
                          intervalMissingPackets,
                          missingPacketsPercent);
        if (_showCPU) {
            System.out.printf(",\n\t\t\t\t\"cpu\": %1$1.2f", outputCpu);
        }
        System.out.printf("\n\t\t\t}\n\t\t}");
    }
    public void print_initial_output()
    {
        System.out.printf("{\"perftest\":\n\t[\n\t\t{\n");
    }
    public void print_final_output()
    {

        System.out.printf("\n\t]\n}\n");

    }
}
