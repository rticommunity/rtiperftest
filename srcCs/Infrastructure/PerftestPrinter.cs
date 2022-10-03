/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;

namespace PerformanceTest
{
    internal enum PerftestOutputFormat
    {
        LEGACY,
        JSON,
        CSV
    }

    public class PerftestPrinter
    {
        public const string TimeUnit = "μs";
        private readonly Parameters arguments;
        private int dataLength = 100;
        private readonly bool printHeaders;
        private readonly bool showCPU;
        private readonly bool printIntervals;
        private bool printSummaryHeaders = true;
        private bool isJsonInitialized;
        private bool controlJsonIntervals;
        private readonly PerftestOutputFormat outputFormat;

        public PerftestPrinter(Parameters arguments)
        {
            this.arguments = arguments;
            dataLength = (int)this.arguments.DataLen;
            printIntervals = !arguments.NoPrintIntervals;
            printHeaders = !arguments.NoOutputHeaders;
            showCPU = arguments.Cpu;

            if (arguments.OutputFormat.Contains("csv", StringComparison.OrdinalIgnoreCase))
            {
                outputFormat = PerftestOutputFormat.CSV;
            }
            else if (arguments.OutputFormat.Contains("json", StringComparison.OrdinalIgnoreCase))
            {
                outputFormat = PerftestOutputFormat.JSON;
                isJsonInitialized = false;
            }
            else if (arguments.OutputFormat.Contains("legacy", StringComparison.OrdinalIgnoreCase))
            {
                outputFormat = PerftestOutputFormat.LEGACY;
            }
        }
        public void SetDataSize(int dataLength)
        {
            this.dataLength = dataLength;
        }

        public void PrintLatencyHeader()
        {
            switch (outputFormat)
            {
                case PerftestOutputFormat.CSV:
                    if (printHeaders && printIntervals)
                    {
                        Console.Write("\nIntervals One-way Latency for {0} " +
                                      "Bytes:\n", dataLength);
                        Console.Write("Length (Bytes)" +
                                      ", Latency ({0}" +
                                      "), Ave ({0}" +
                                      "), Std ({0}" +
                                      "), Min ({0}" +
                                      "), Max ({0})",
                                      TimeUnit);
                        if (showCPU)
                        {
                            Console.Write(", CPU (%)");
                        }
                        Console.Write("\n");
                    }

                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.JSON:
                    if (isJsonInitialized)
                    {
                        Console.Write(",\n\t\t{\n");
                    }
                    else
                    {
                        isJsonInitialized = true;
                    }
                    Console.Write("\t\t\t\"length\":{0},\n", dataLength);
                    if (printIntervals)
                    {
                        Console.Write("\t\t\t\"intervals\":[\n");
                        controlJsonIntervals = true;
                    }
                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.LEGACY:
                    if (printHeaders && printIntervals)
                    {
                        Console.Write("\n\n********** New data length is {0}\n",
                                      dataLength);
                        Console.Out.Flush();
                    }
                    break;
            }
        }
        public void PrintThroughputHeader()
        {
            switch (outputFormat)
            {
                case PerftestOutputFormat.CSV:
                    if (printHeaders && printIntervals)
                    {
                        Console.Write("\nIntervals Throughput for {0} " +
                                      "Bytes:\n", dataLength);
                        Console.Write("Length (Bytes), Total Samples,  " +
                                      "Samples/s, Avg Samples/s,     " +
                                      "Mbps,  Avg Mbps" +
                                      ", Lost Samples, Lost Samples (%)");
                        if (showCPU)
                        {
                            Console.Write(", CPU (%)");
                        }
                        Console.Write("\n");
                    }

                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.JSON:
                    if (isJsonInitialized)
                    {
                        Console.Write(",\n\t\t{\n");
                    }
                    else
                    {
                        isJsonInitialized = true;
                    }
                    Console.Write("\t\t\t\"length\":{0},\n", dataLength);
                    if (printIntervals)
                    {
                        Console.Write("\t\t\t\"intervals\":[\n");
                        controlJsonIntervals = true;
                    }
                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.LEGACY:
                    if (printHeaders && printIntervals)
                    {
                        Console.Write("\n\n********** New data length is {0}\n",
                                      dataLength);
                        Console.Out.Flush();
                    }
                    break;
            }
        }
        public void PrintLatencyInterval(
            uint latency,
            double latencyAve,
            double latencyStd,
            uint latencyMin,
            uint latencyMax,
            double outputCpu)
        {
            switch (outputFormat)
            {
                case PerftestOutputFormat.CSV:
                    Console.Write("{0,14},{1,13},{2,9:F0},{3,9:F1},{4,9},{5,9}",
                           dataLength,
                           latency,
                           latencyAve,
                           latencyStd,
                           latencyMin,
                           latencyMax);
                    if (showCPU)
                    {
                        Console.Write(",{0,8:F2}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.JSON:
                    if (controlJsonIntervals)
                    {
                        controlJsonIntervals = false;
                    }
                    else
                    {
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
                    if (showCPU)
                    {
                        Console.Write(",\n\t\t\t\t\t\"cpu\": {0:F1}",
                                      outputCpu);
                    }
                    Console.Write("\n\t\t\t\t}");
                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.LEGACY:
                    Console.Write("One way Latency: {1,6} {0}" +
                                  " Ave {2,6:F0} {0}" +
                                  " Std {3,6:F1} {0}" +
                                  " Min {4,6} {0}" +
                                  " Max {5,6} {0}",
                                  TimeUnit,
                                  latency,
                                  latencyAve,
                                  latencyStd,
                                  latencyMin,
                                  latencyMax);
                    if (showCPU)
                    {
                        Console.Write(" CPU {0:P}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
            }
        }
        public void PrintLatencySummary(
            double latencyAve,
            double latencyStd,
            ulong latencyMin,
            ulong latencyMax,
            uint[] latencyHistory,
            ulong count,
            double outputCpu)
        {
            switch (outputFormat)
            {
                case PerftestOutputFormat.CSV:
                    if (printSummaryHeaders && printHeaders)
                    {
                        if (!printIntervals && printSummaryHeaders)
                        {
                            printSummaryHeaders = printIntervals;
                        }

                        if (printIntervals)
                        {
                            Console.Write("\nOne-way Latency Summary:\n");
                        }
                        Console.Write("Sample Size (Bytes)" +
                                      ", Ave ({0}" +
                                      "), Std ({0}" +
                                      "), Min ({0}" +
                                      "), Max ({0}" +
                                      "), 50% ({0}" +
                                      "), 90% ({0}" +
                                      "), 99% ({0}" +
                                      "), 99.99% ({0}" +
                                      "), 99.9999% ({0})",
                                      TimeUnit);
                        if (showCPU)
                        {
                            Console.Write(", CPU (%)");
                        }
                        Console.Write("\n");
                    }
                    Console.Write("{0,19},{1,9:F0},{2,9:F1},{3,9},{4,9}," +
                                  "{5,9},{6,9},{7,9},{8,12},{9,14}",
                                  dataLength,
                                  latencyAve,
                                  latencyStd,
                                  latencyMin,
                                  latencyMax,
                                  latencyHistory[count * 50 / 100],
                                  latencyHistory[count * 90 / 100],
                                  latencyHistory[count * 99 / 100],
                                  latencyHistory[(int)(count *
                                        (9999.0 / 10000))],
                                  latencyHistory[(int)(count *
                                        (999999.0 / 1000000))]);
                    if (showCPU)
                    {
                        Console.Write(",{0,8:F2}", outputCpu);
                    }
                    Console.Write("\n");
                    break;
                case PerftestOutputFormat.JSON:
                    if (printIntervals)
                    {
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
                                  latencyHistory[(int)(count *
                                        (9999.0 / 10000))],
                                  latencyHistory[(int)(count *
                                        (999999.0 / 1000000))]);
                    if (showCPU)
                    {
                        Console.Write(",\n\t\t\t\t\"cpu\": {0:F1}", outputCpu);
                    }
                    Console.Write("\n\t\t\t}\n\t\t}");
                    break;
                case PerftestOutputFormat.LEGACY:
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
                                  TimeUnit,
                                  dataLength,
                                  latencyAve,
                                  latencyStd,
                                  latencyMin,
                                  latencyMax,
                                  latencyHistory[count * 50 / 100],
                                  latencyHistory[count * 90 / 100],
                                  latencyHistory[count * 99 / 100],
                                  latencyHistory[(int)(count
                                         * (9999.0 / 10000))],
                                  latencyHistory[(int)(count
                                         * (999999.0 / 1000000))]);
                    if (showCPU)
                    {
                        Console.Write(" CPU {0:P}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
            }
        }
        public void PrintThroughputInterval(
            ulong lastMsgs,
            ulong mps,
            double mpsAve,
            ulong bps,
            double bpsAve,
            ulong missingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            switch (outputFormat)
            {
                case PerftestOutputFormat.CSV:
                    Console.Write("{0,14},{1,14},{2,11:F0},{3,14:F0},{4,9:F1}" +
                                  ",{5,10:F1},{6,13},{7,17:F2}",
                                  dataLength,
                                  lastMsgs,
                                  mps,
                                  mpsAve,
                                  bps * 8.0 / 1000.0 / 1000.0,
                                  bpsAve * 8.0 / 1000.0 / 1000.0,
                                  missingPackets,
                                  missingPacketsPercent);
                    if (showCPU)
                    {
                        Console.Write(",{0,8:F2}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.JSON:
                    if (controlJsonIntervals)
                    {
                        controlJsonIntervals = false;
                    }
                    else
                    {
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
                                  dataLength,
                                  lastMsgs,
                                  mps,
                                  mpsAve,
                                  bps * 8.0 / 1000.0 / 1000.0,
                                  bpsAve * 8.0 / 1000.0 / 1000.0,
                                  missingPackets,
                                  missingPacketsPercent);
                    if (showCPU)
                    {
                        Console.Write(",\n\t\t\t\t\t\"cpu\": {0:F1}",
                                      outputCpu);
                    }
                    Console.Write("\n\t\t\t\t}");
                    break;
                case PerftestOutputFormat.LEGACY:
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
                    if (showCPU)
                    {
                        Console.Write(" CPU {0:P}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
            }
        }
        public void PrintThroughputSummary(
            int length,
            ulong intervalPacketsReceived,
            ulong intervalTime,
            ulong intervalBytesReceived,
            ulong intervalMissingPackets,
            double missingPacketsPercent,
            double outputCpu)
        {
            switch (outputFormat)
            {
                case PerftestOutputFormat.CSV:
                    if (printSummaryHeaders && printHeaders)
                    {
                        if (!printIntervals && printSummaryHeaders)
                        {
                            printSummaryHeaders = printIntervals;
                        }

                        if (printIntervals)
                        {
                            Console.Write("\nThroughput Summary:\n");
                        }
                        Console.Write("Sample Size (Bytes), Total Samples," +
                                      " Avg Samples/s,    Avg Mbps, " +
                                      "Lost Samples, Lost Samples (%)");
                        if (showCPU)
                        {
                            Console.Write(", CPU (%)");
                        }
                        Console.Write("\n");
                    }
                    Console.Write("{0,19},{1,14},{2,14:F0},{3,12:F1}," +
                                  "{4,13},{5,17:F2}",
                                  length,
                                  intervalPacketsReceived,
                                  intervalPacketsReceived * 1000000
                                         / intervalTime,
                                  intervalBytesReceived * 1000000.0
                                         / intervalTime * 8.0 / 1000.0 / 1000.0,
                                  intervalMissingPackets,
                                  missingPacketsPercent);
                    if (showCPU)
                    {
                        Console.Write(",{0,8:F2}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
                case PerftestOutputFormat.JSON:
                    if (printIntervals)
                    {
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
                    if (showCPU)
                    {
                        Console.Write(",\n\t\t\t\t\"cpu\": {0:F1}", outputCpu);
                    }
                    Console.Write("\n\t\t\t}\n\t\t}");
                    break;
                case PerftestOutputFormat.LEGACY:
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
                    if (showCPU)
                    {
                        Console.Write(" CPU {0:P}", outputCpu);
                    }
                    Console.Write("\n");
                    Console.Out.Flush();
                    break;
            }
        }
        public void PrintInitialOutput()
        {
            if (outputFormat.Equals(PerftestOutputFormat.JSON))
            {
                Console.Write("{\"perftest\":\n\t[\n\t\t{\n");
            }
        }
        public void PrintFinalOutput()
        {
            if (outputFormat.Equals(PerftestOutputFormat.JSON))
            {
                Console.Write("\n\t]\n}\n");
            }
        }
    }
}