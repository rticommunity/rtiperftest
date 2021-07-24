/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;

namespace PerformanceTest
{
    /*
     * Receives latency ping from Subscriber and does
     * round trip latency calculations
     */
    public class LatencyListener : IMessagingCallback
    {
        private readonly PerftestPrinter printer;
        private readonly Parameters parameters;
        private ulong latencySum;
        private ulong latencySumSquare;
        private ulong count;
        private uint latencyMin = Perftest.LATENCY_RESET_VALUE;
        private uint latencyMax;
        private int lastDataLength;
        public bool EndTest { get; set; }
        private readonly uint[] latencyHistory;
        private uint clockSkewCount = 0;
        private readonly IMessagingReader reader;
        private readonly IMessagingWriter writer;
        public CpuMonitor cpu = new CpuMonitor();
        private int NumLatency => latencyHistory.Length;

        public LatencyListener(
                IMessagingWriter writer,
                Parameters parameters,
                PerftestPrinter printer,
                uint numLatency)
        {
            this.parameters = parameters;
            this.writer = writer;
            this.printer = printer;
            if (numLatency > 0)
            {
                latencyHistory = new uint[numLatency];
            }
        }

        public LatencyListener(
                IMessagingReader reader,
                IMessagingWriter writer,
                Parameters parameters,
                PerftestPrinter printer,
                uint numLatency)
                : this (writer, parameters, printer, numLatency)
        {
            this.reader = reader;
        }

        public void ProcessMessage(TestMessage message)
        {
            ulong now, sentTime;
            long sec;
            ulong usec;
            uint latency;
            double latencyAve;
            double latencyStd;

            now = Perftest.GetTimeUsec();

            // If message.Size == Perftest.INITIALIZE_SIZE, the message is bein initialized and
            // there's nothing to process
            if (message.Size == Perftest.INITIALIZE_SIZE || message.Size == Perftest.FINISHED_SIZE)
            {
                return;
            }
            else if (message.Size == Perftest.LENGTH_CHANGED_SIZE)
            {
                Console.Error.WriteLine(
                        "[Error]: Received command to change size,"
                        + "this is not supported in C#");
                PrintSummaryLatency();
                return;
            }

            sec = message.timestampSec;
            usec = message.timestampUsec;
            sentTime = ((ulong)sec << 32) | usec;

            if (now >= sentTime)
            {
                latency = (uint)(now - sentTime);

                // keep track of one-way latency
                latency /= 2;
            }
            else
            {
                Console.Error.WriteLine(
                    $"Clock skew suspected: received time {now} usec, sent time {sentTime} usec");
                ++clockSkewCount;
                return;
            }

            // store value for percentile calculations
            if (latencyHistory != null)
            {
                if (count >= (ulong)NumLatency)
                {
                    // Console.Error.WriteLine(
                    //     "Too many latency pongs received.  Do you have more "
                    //     + "than 1 app with -pidMultiPubTest = 0 or -sidMultiSubTest = 0? count: "
                    //     + count + " NumLatency: " + NumLatency + "\n");
                    return;
                }
                else
                {
                    latencyHistory[count] = latency;
                }
            }

            if (latencyMin == Perftest.LATENCY_RESET_VALUE)
            {
                latencyMin = latency;
                latencyMax = latency;
            }
            else
            {
                if (latency < latencyMin)
                {
                    latencyMin = latency;
                }
                else if (latency > latencyMax)
                {
                    latencyMax = latency;
                }
            }

            ++count;
            latencySum += latency;
            latencySumSquare += (ulong)latency * (ulong)latency;

            // if data sized changed
            if (lastDataLength != message.Size)
            {
                lastDataLength = message.Size;

                if (lastDataLength != 0)
                {
                    printer.SetDataSize(lastDataLength
                             + (int)Perftest.OVERHEAD_BYTES);
                    printer.PrintLatencyHeader();
                }
            }
            else
            {
                double outputCpu = 0.0;
                if (parameters.Cpu)
                {
                    outputCpu = cpu.GetCpuInstant();
                }
                if (!parameters.NoPrintIntervals)
                {
                    latencyAve = (double)latencySum / (double)count;
                    latencyStd = System.Math.Sqrt(
                        ((double)latencySumSquare / (double)count) - (latencyAve * latencyAve));
                    printer.PrintLatencyInterval(
                        latency,
                        latencyAve,
                        latencyStd,
                        latencyMin,
                        latencyMax,
                        outputCpu);
                }
            }
            writer?.NotifyPingResponse();
        }

        public void ReadThread()
        {
            while (!EndTest)
            {
                // Receive message should block until a message is received
                reader.ReceiveMessage(this);
            }
        }

        public void PrintSummaryLatency(bool endTest = false)
        {
            double latencyAve;
            double latencyStd;
            if (count == 0)
            {
                if (endTest)
                {
                    Console.Error.Write(
                        "\nNo Pong samples have been received in the Publisher side.\n"
                        + "If you are interested in latency results, you might need to\n"
                        + "increase the Pong frequency (using the -latencyCount option).\n"
                        + "Alternatively you can increase the number of samples sent\n"
                        + "(-numIter) or the time for the test (-executionTime). If you\n"
                        + "are sending large data, make sure you set the data size (-datalen)\n"
                        + "in the Subscriber side.\n\n");
                }
                return;
            }

            if (clockSkewCount != 0)
            {
                Console.Error.Write(
                    "The following latency result may not be accurate because clock skew happens {0} times\n",
                    clockSkewCount);
            }

            // sort the array (in ascending order)
            Array.Sort(latencyHistory, 0, (int)count);
            latencyAve = latencySum / count;
            latencyStd = System.Math.Sqrt((latencySumSquare / count) - (latencyAve * latencyAve));
            double outputCpu = 0.0;
            if (parameters.Cpu)
            {
                outputCpu = cpu.GetCpuAverage();
            }
            printer.PrintLatencySummary(
                    latencyAve,
                    latencyStd,
                    latencyMin,
                    latencyMax,
                    latencyHistory,
                    count,
                    outputCpu);

            latencySum = 0;
            latencySumSquare = 0;
            latencyMin = Perftest.LATENCY_RESET_VALUE;
            latencyMax = 0;
            count = 0;
            clockSkewCount = 0;
        }
    }
} // PerformanceTest Namespace
