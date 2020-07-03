using System;
using System.Collections.Generic;
using System.Text;

namespace PerformanceTest {
    public abstract class PerftestPrinter
    {
        public int _dataLength;
        public bool _printHeaders;
        protected bool _showCPU;
        protected bool _printIntervals;
        protected bool _printSummaryHeaders;
        public const string PERFT_TIME_UNIT = "us";

        public PerftestPrinter()
        {
            _dataLength = 100;
            _printSummaryHeaders = true;
        }
        public virtual void initialize(
                bool printIntervals,
                bool printHeaders,
                bool showCpu)
        {
            _printIntervals = printIntervals;
            _printHeaders = printHeaders;
            _showCPU = showCpu;
        }
        public abstract void print_latency_header();
        public abstract void print_throughput_header();
        public abstract void print_latency_interval(
            uint latency,
            double latencyAve,
            double latencyStd,
            uint latencyMin,
            uint latencyMax,
            double outputCpu);
        public abstract void print_latency_summary(
            double latencyAve,
            double latencyStd,
            ulong latencyMin,
            ulong latencyMax,
            uint[] latencyHistory,
            ulong count,
            double outputCpu);
        public abstract void print_throughput_interval(
            ulong lastMsgs,
            ulong mps,
            double mpsAve,
            ulong bps,
            double bpsAve,
            ulong missingPackets,
            double missingPacketsPercent,
            double outputCpu);
        public abstract void print_throughput_summary(
            int length,
            ulong intervalPacketsReceived,
            ulong intervalTime,
            ulong intervalBytesReceived,
            ulong intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu);
        public virtual void print_initial_output()
        {
        }
        public virtual void print_final_output()
        {
        }
    }

    public class PerftestCSVPrinter : PerftestPrinter
    {
        public PerftestCSVPrinter()
        {
            _dataLength = 100;
            _printSummaryHeaders = true;
        }
        public override void print_latency_header()
        {
            if(_printHeaders && _printIntervals) {
                Console.Write("\nIntervals One-way Latency for {0} " +
                              "Bytes:\n", _dataLength);
                Console.Write("Length (Bytes)" +
                              ", Latency ({0}" +
                              "), Ave ({0}" +
                              "), Std ({0}" +
                              "), Min ({0}" +
                              "), Max ({0})",
                              PERFT_TIME_UNIT);
            }
            if (_showCPU) {
                Console.Write(", CPU (%)");
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_throughput_header()
        {
            if(_printHeaders && _printIntervals) {
                Console.Write("\nIntervals Throughput for {0} " +
                              "Bytes:\n", _dataLength);
                Console.Write("Length (Bytes), Total Samples,  " +
                              "Samples/s, Ave Samples/s,     " +
                              "Mbps,  Ave Mbps" +
                              ", Lost Samples, Lost Samples (%)");
            }
            if (_showCPU) {
                Console.Write(", CPU (%)");
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_latency_interval(
            uint latency,
            double latencyAve,
            double latencyStd,
            uint latencyMin,
            uint latencyMax,
            double outputCpu)
        {
            Console.Write("{0,14},{1,13},{2,9:F0},{3,9:F1},{4,9},{5,9}",
                    _dataLength,
                    latency,
                    latencyAve,
                    latencyStd,
                    latencyMin,
                    latencyMax);
            if (_showCPU) {
                Console.Write(",{0,8:F2}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_latency_summary(
            double latencyAve,
            double latencyStd,
            ulong latencyMin,
            ulong latencyMax,
            uint[] latencyHistory,
            ulong count,
            double outputCpu)
        {
            if (_printSummaryHeaders && _printHeaders) {
                if (!_printIntervals && _printSummaryHeaders) {
                    _printSummaryHeaders = _printIntervals;
                }
                if (_printIntervals) {
                    Console.Write("\nOne-way Latency Summary:\n");
                }
                Console.Write("Length (Bytes)" +
                              ", Ave ({0}" +
                              "), Std ({0}" +
                              "), Min ({0}" +
                              "), Max ({0}" +
                              "), 50%% ({0}" +
                              "), 90%% ({0}" +
                              "), 99%% ({0}" +
                              "), 99.99%% ({0}" +
                              "), 99.9999%% ({0})",
                              PERFT_TIME_UNIT);
                if (_showCPU) {
                    Console.Write(", CPU (%)");
                }
                Console.Write("\n");
            }
            Console.Write("{0,14},{1,9:F0},{2,9:F1},{3,9},{4,9}" +
                          "{5,9},{6,9},{7,9},{8,12},{9,14}",
                          _dataLength,
                          latencyAve,
                          latencyStd,
                          latencyMin,
                          latencyMax,
                          latencyHistory[count * 50 / 100],
                          latencyHistory[count * 90 / 100],
                          latencyHistory[count * 99 / 100],
                          latencyHistory[(int) (count *
                                (9999.0 / 10000))],
                          latencyHistory[(int) (count *
                                (999999.0 / 1000000))]);
            if (_showCPU) {
                Console.Write(",{0,8:F2}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_throughput_interval(
            ulong lastMsgs,
            ulong mps,
            double mpsAve,
            ulong bps,
            double bpsAve,
            ulong missingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            Console.Write("{0,14},{1,14},{2,11:F0},{3,14:F0},{4,9:F1}" +
                          ",{5,10:F1},{6,13},{7,17:F2}",
                          _dataLength,
                          lastMsgs,
                          mps,
                          mpsAve,
                          bps * 8.0 / 1000.0 / 1000.0,
                          bpsAve * 8.0 / 1000.0 / 1000.0,
                          missingPackets,
                          missingPacketsPercent);
            if (_showCPU) {
                Console.Write(",{0,8:F2}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_throughput_summary(
            int length,
            ulong intervalPacketsReceived,
            ulong intervalTime,
            ulong intervalBytesReceived,
            ulong intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            if (_printSummaryHeaders && _printHeaders) {
                if (!_printIntervals && _printSummaryHeaders) {
                    _printSummaryHeaders = _printIntervals;
                }
                if (_printIntervals) {
                    Console.Write("\nThroughput Summary:\n");
                }
                Console.Write("Length (Bytes), Total Samples," +
                              " Ave Samples/s,    Ave Mbps, " +
                              "Lost Samples, Lost Samples (%)");
                if (_showCPU) {
                    Console.Write(", CPU (%)");
                }
                Console.Write("\n");
            }
            Console.Write("{0,14},{1,14},{2,14:F0},{3,12:F1}," +
                          "{4,13},{5,17:F2}",
                          length,
                          intervalPacketsReceived,
                          intervalPacketsReceived * 1000000
                                  / intervalTime,
                          intervalBytesReceived * 1000000.0
                                  / intervalTime * 8.0 / 1000.0 / 1000.0,
                          intervalMissingPackets,
                          missingPacketsPercent);
            if (_showCPU) {
                Console.Write(",{0,8:F2}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
    }

    public class PerftestJSONPrinter : PerftestPrinter
    {
        protected bool _isJsonInitialized;
        protected bool _controlJsonIntervals;

        public PerftestJSONPrinter()
        {
            _dataLength = 100;
            _printSummaryHeaders = true;
        }

        public override void initialize(
                bool printIntervals,
                bool printHeaders,
                bool showCpu)
        {
            _printIntervals = printIntervals;
            _printHeaders = printHeaders;
            _showCPU = showCpu;
            _isJsonInitialized = false;
            _controlJsonIntervals = false;
        }
        public override void print_latency_header()
        {
            if (_isJsonInitialized) {
                Console.Write(",\n\t\t{\n");
            } else {
                _isJsonInitialized = true;
            }
            Console.Write("\t\t\t\"length\":{0},\n", _dataLength);
            if (_printIntervals) {
                Console.Write("\t\t\t\"intervals\":[\n");
                _controlJsonIntervals = true;
            }
            Console.Out.Flush();
        }
        public override void print_throughput_header()
        {
            if (_isJsonInitialized) {
                Console.Write(",\n\t\t{\n");
            } else {
                _isJsonInitialized = true;
            }
            Console.Write("\t\t\t\"length\":{0},\n", _dataLength);
            if (_printIntervals) {
                Console.Write("\t\t\t\"intervals\":[\n");
                _controlJsonIntervals = true;
            }
            Console.Out.Flush();
        }
        public override void print_latency_interval(
            uint latency,
            double latencyAve,
            double latencyStd,
            uint latencyMin,
            uint latencyMax,
            double outputCpu)
        {
            if (_controlJsonIntervals) {
                _controlJsonIntervals = false;
            } else {
                Console.Write(",");
            }
            Console.Write("\n\t\t\t\t{{\n" +
                            "\t\t\t\t\t\"latency\": {0},\n" +
                            "\t\t\t\t\t\"latency_ave\": {1:F0},\n" +
                            "\t\t\t\t\t\"latency_std\": {2:F1},\n" +
                            "\t\t\t\t\t\"latency_min\": {3},\n" +
                            "\t\t\t\t\t\"latency_max\": {4}",
                            latency,
                            latencyAve,
                            latencyStd,
                            latencyMin,
                            latencyMax);
            if (_showCPU) {
                Console.Write(",\n\t\t\t\t\t\"cpu\": {0:F1}",
                                outputCpu);
            }
            Console.Write("\n\t\t\t\t}");
            Console.Out.Flush();
        }
        public override void print_latency_summary(
            double latencyAve,
            double latencyStd,
            ulong latencyMin,
            ulong latencyMax,
            uint[] latencyHistory,
            ulong count,
            double outputCpu)
        {
            if (_printIntervals) {
                Console.Write("\n\t\t\t],\n");
            }
            Console.Write("\t\t\t\"summary\":{{\n" +
                            "\t\t\t\t\"latency_ave\": {0:F1},\n" +
                            "\t\t\t\t\"latency_std\": {1:F1},\n" +
                            "\t\t\t\t\"latency_min\": {2},\n" +
                            "\t\t\t\t\"latency_max\": {3},\n" +
                            "\t\t\t\t\"latency_50\": {4},\n" +
                            "\t\t\t\t\"latency_90\": {5},\n" +
                            "\t\t\t\t\"latency_99\": {6},\n" +
                            "\t\t\t\t\"latency_99.99\": {7},\n" +
                            "\t\t\t\t\"latency_99.9999\": {8}",
                            latencyAve,
                            latencyStd,
                            latencyMin,
                            latencyMax,
                            latencyHistory[count * 50 / 100],
                            latencyHistory[count * 90 / 100],
                            latencyHistory[count * 99 / 100],
                            latencyHistory[(int) (count *
                                (9999.0 / 10000))],
                            latencyHistory[(int) (count *
                                (999999.0 / 1000000))]);
            if (_showCPU) {
                Console.Write(",\n\t\t\t\t\"cpu\": {0:F1}", outputCpu);
            }
            Console.Write("\n\t\t\t}\n\t\t}");
        }
        public override void print_throughput_interval(
            ulong lastMsgs,
            ulong mps,
            double mpsAve,
            ulong bps,
            double bpsAve,
            ulong missingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            if (_controlJsonIntervals) {
                _controlJsonIntervals = false;
            } else {
                Console.Write(",");
            }
            Console.Write("\n\t\t\t\t{{\n" +
                            "\t\t\t\t\t\"length\": {0},\n" +
                            "\t\t\t\t\t\"packets\": {1},\n" +
                            "\t\t\t\t\t\"packets/s\": {2},\n" +
                            "\t\t\t\t\t\"packets/s_ave\": {3:F2},\n" +
                            "\t\t\t\t\t\"mbps\": {4:F1},\n" +
                            "\t\t\t\t\t\"mbps_ave\": {5:F1},\n" +
                            "\t\t\t\t\t\"lost\": {6},\n" +
                            "\t\t\t\t\t\"lost_percent\": {7:F2}",
                            _dataLength,
                            lastMsgs,
                            mps,
                            mpsAve,
                            bps * 8.0 / 1000.0 / 1000.0,
                            bpsAve * 8.0 / 1000.0 / 1000.0,
                            missingPackets,
                            missingPacketsPercent);
            if (_showCPU) {
                Console.Write(",\n\t\t\t\t\t\"cpu\": {0:F1}",
                                outputCpu);
            }
            Console.Write("\n\t\t\t\t}");
        }
        public override void print_throughput_summary(
            int length,
            ulong intervalPacketsReceived,
            ulong intervalTime,
            ulong intervalBytesReceived,
            ulong intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            if (_printIntervals) {
                Console.Write("\n\t\t\t],\n");
            }
            Console.Write("\t\t\t\"summary\":{{\n" +
                            "\t\t\t\t\"packets\": {0},\n" +
                            "\t\t\t\t\"packets/sAve\": {1},\n" +
                            "\t\t\t\t\"mbpsAve\": {2:F1},\n" +
                            "\t\t\t\t\"lost\": {3},\n" +
                            "\t\t\t\t\"lostPercent\": {4:F2}",
                            intervalPacketsReceived,
                            intervalPacketsReceived * 1000000
                                    / intervalTime,
                            intervalBytesReceived * 1000000.0
                                    / intervalTime * 8.0 / 1000.0 / 1000.0,
                            intervalMissingPackets,
                            missingPacketsPercent);
            if (_showCPU) {
                Console.Write(",\n\t\t\t\t\"cpu\": {0:F1}", outputCpu);
            }
            Console.Write("\n\t\t\t}\n\t\t}");
        }
        public override void print_initial_output()
        {
            Console.Write("{\"perftest\":\n\t[\n\t\t{\n");
        }
        public override void print_final_output()
        {
            Console.Write("\n\t]\n}\n");
        }
    }

    public class PerftestLegacyPrinter : PerftestPrinter
    {
        public PerftestLegacyPrinter()
        {
            _dataLength = 100;
            _printSummaryHeaders = true;
        }
        public override void print_latency_header()
        {
            if(_printHeaders && _printIntervals) {
                Console.Write("\n\n********** New data length is {0}\n",
                                _dataLength);
                Console.Out.Flush();
            }
        }
        public override void print_throughput_header()
        {
            if(_printHeaders && _printIntervals) {
                Console.Write("\n\n********** New data length is {0}\n",
                                _dataLength);
                Console.Out.Flush();
            }
        }
        public override void print_latency_interval(
            uint latency,
            double latencyAve,
            double latencyStd,
            uint latencyMin,
            uint latencyMax,
            double outputCpu)
        {
            Console.Write("One way Latency: {1,6} {0}" +
                            " Ave {2,6:F0} {0}" +
                            " Std {3,6:F1} {0}" +
                            " Min {4,6} {0}" +
                            " Max {5,6} {0}",
                            PERFT_TIME_UNIT,
                            latency,
                            latencyAve,
                            latencyStd,
                            latencyMin,
                            latencyMax);
            if (_showCPU) {
                Console.Write(" CPU {0:P}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_latency_summary(
            double latencyAve,
            double latencyStd,
            ulong latencyMin,
            ulong latencyMax,
            uint[] latencyHistory,
            ulong count,
            double outputCpu)
        {
            Console.Write("Length: {1,5}" +
                            " Latency: Ave {2,6:F0} {0}" +
                            " Std {3,6:F1} {0}" +
                            " Min {4,6} {0}" +
                            " Max {5,6} {0}" +
                            " 50% {6,6} {0}" +
                            " 90% {7,6} {0}" +
                            " 99% {8,6} {0}" +
                            " 99.99% {9,6} {0}" +
                            " 99.9999% {10,6} {0}",
                            PERFT_TIME_UNIT,
                            _dataLength,
                            latencyAve,
                            latencyStd,
                            latencyMin,
                            latencyMax,
                            latencyHistory[count * 50 / 100],
                            latencyHistory[count * 90 / 100],
                            latencyHistory[count * 99 / 100],
                            latencyHistory[(int) (count
                                    * (9999.0 / 10000))],
                            latencyHistory[(int) (count
                                    * (999999.0 / 1000000))]);
            if (_showCPU) {
                Console.Write(" CPU {0:P}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_throughput_interval(
            ulong lastMsgs,
            ulong mps,
            double mpsAve,
            ulong bps,
            double bpsAve,
            ulong missingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            Console.Write("Packets: {0,8}  Packets/s: {1,7}  " +
                            "Packets/s(ave): {2,7:F0}  Mbps: {3,7:F1}" +
                            "  Mbps(ave): {4,7:F1}  Lost: {5,5} ({6:P})",
                            lastMsgs,
                            mps,
                            mpsAve,
                            bps * 8.0 / 1000.0 / 1000.0,
                            bpsAve * 8.0 / 1000.0 / 1000.0,
                            missingPackets,
                            missingPacketsPercent);
            if (_showCPU) {
                Console.Write(" CPU {0:P}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
        public override void print_throughput_summary(
            int length,
            ulong intervalPacketsReceived,
            ulong intervalTime,
            ulong intervalBytesReceived,
            ulong intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            Console.Write("Length: {0,5}  Packets: {1,8}  " +
                            "Packets/s(ave): {2,7}  " +
                            "Mbps(ave): {3,7:F1}  Lost: {4,5} ({5:P})",
                            length,
                            intervalPacketsReceived,
                            intervalPacketsReceived * 1000000
                                    / intervalTime,
                            intervalBytesReceived * 1000000.0
                                    / intervalTime * 8.0 / 1000.0 / 1000.0,
                            intervalMissingPackets,
                            missingPacketsPercent);
            if (_showCPU) {
                Console.Write(" CPU {0:P}", outputCpu);
            }
            Console.Write("\n");
            Console.Out.Flush();
        }
    }
}