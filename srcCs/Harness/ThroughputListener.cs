/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Collections.Generic;

namespace PerformanceTest
{
    /*********************************************************
     * Listener for the Subscriber side
     *
     * Keeps stats on data received per second.
     * Returns a ping for latency packets
     */
    public class ThroughputListener : IMessagingCallback
    {
        private readonly PerftestPrinter printer;
        public ulong packetsReceived;
        public ulong bytesReceived;
        public ulong missingPackets;
        public bool endTest;
        public bool changeSize;

        private int currentMessageSize;
        public int lastDataLength = -1;
        private int messageEntityId;
        private uint messageSequenceNumber;

        // store info for the last data set
        public int intervalDataLength = -1;
        public ulong intervalPacketsReceived;
        public ulong intervalBytesReceived;
        public ulong intervalMissingPackets;
        public ulong intervalTime;
        public ulong beginTime;
        private double missingPacketsPercent;

        private readonly IMessagingWriter writer;
        private readonly IMessagingReader reader;
        private readonly ulong[] lastSeqNum;

        private readonly uint numPublishers;
        private readonly List<int> finishedPublishers = new List<int>();
        public CpuMonitor cpu = new CpuMonitor();
        private readonly bool useCft;
        private readonly int subId;
        private readonly bool showCpu;

        public ThroughputListener(IMessagingWriter writer,
                PerftestPrinter printer,
                Parameters parameters)
        {
            this.writer = writer;
            this.printer = printer;
            lastSeqNum = new ulong[parameters.NumPublishers];
            numPublishers = parameters.NumPublishers;
            useCft = !string.IsNullOrEmpty(parameters.Cft);
            subId = parameters.SidMultiSubTest;
            showCpu = parameters.Cpu;
        }

        public ThroughputListener(IMessagingWriter writer,
                IMessagingReader reader,
                PerftestPrinter printer,
                Parameters parameters)
        {
            this.writer = writer;
            this.reader = reader;
            this.printer = printer;
            lastSeqNum = new ulong[parameters.NumPublishers];
            numPublishers = parameters.NumPublishers;
            useCft = !string.IsNullOrEmpty(parameters.Cft);
            subId = parameters.SidMultiSubTest;
            showCpu = parameters.Cpu;
        }

        public void ProcessMessage(TestMessage message)
        {
            currentMessageSize = message.Size;
            messageEntityId = message.entityId;
            messageSequenceNumber = message.seqNum;
            if (messageEntityId >= numPublishers || messageEntityId < 0)
            {
                Console.Error.WriteLine(
                    "ProcessMessage: message content no valid. message.entity_id out of bounds");
                return;
            }
            // Check for test initialization messages
            if (currentMessageSize == Perftest.INITIALIZE_SIZE)
            {
                writer.Send(message, false);
                writer.Flush();
                return;
            }
            else if (currentMessageSize == Perftest.FINISHED_SIZE)
            {
                /*
                 * PERFTEST-97
                 * We check the entity_id of the publisher to see if it has
                 * already send a FINISHED_SIZE message. If he has we ignore
                 * any new one. Else, we add it to a vector. Once that
                 * vector contains all the ids of the publishers the
                 * subscriber is suppose to know, that means that all the
                 * publishers have finished sending data samples, so it is
                 * time to finish the subscriber.
                 */
                if (finishedPublishers.Contains(messageEntityId))
                {
                    return;
                }

                if (endTest)
                {
                    return;
                }

                finishedPublishers.Add(messageEntityId);

                if (finishedPublishers.Count >= numPublishers)
                {
                    PrintSummaryThroughput(message, true);
                    endTest = true;
                }
                return;
            }

            // Send back a packet if this is a ping
            if ((message.latencyPing == subId) ||
                    (useCft && message.latencyPing != -1))
            {
                writer.Send(message, false);
                writer.Flush();
            }

            // // Always check if need to reset internals
            // if (currentMessageSize == Perftest.LENGTH_CHANGED_SIZE)
            // {
            //     PrintSummaryThroughput(message);
            //     changeSize = true;
            //     return;
            // }

            if (currentMessageSize != lastDataLength)
            {
                packetsReceived = 0;
                bytesReceived = 0;
                missingPackets = 0;

                for (int i = 0; i < numPublishers; i++)
                {
                    lastSeqNum[i] = 0;
                }

                beginTime = Perftest.GetTimeUsec();
                printer.SetDataSize(currentMessageSize + (int)Perftest.OVERHEAD_BYTES);
                printer.PrintThroughputHeader();
            }

            lastDataLength = currentMessageSize;
            ++packetsReceived;
            bytesReceived += (ulong)(currentMessageSize + (int)Perftest.OVERHEAD_BYTES);

            // detect missing packets
            if (!useCft)
            {
                if (lastSeqNum[messageEntityId] == 0)
                {
                    lastSeqNum[messageEntityId] = messageSequenceNumber;
                }
                else if (messageSequenceNumber != ++lastSeqNum[messageEntityId])
                {
                    // only track if skipped, might have restarted pub
                    if (messageSequenceNumber > lastSeqNum[messageEntityId])
                    {
                        missingPackets +=
                                messageSequenceNumber -
                                lastSeqNum[messageEntityId];
                    }
                    lastSeqNum[messageEntityId] = messageSequenceNumber;
                }
            }
        }

        /*********************************************************
         * Used for receiving data using a thread instead of callback
         *
         */
        public void ReadThread()
        {
            while (!endTest)
            {
                // Receive message should block until a message is received
                reader.ReceiveMessage(this);
            }
        }

        public void PrintSummaryThroughput(TestMessage message, bool endTest = false)
        {
            // store the info for this interval
            ulong now = Perftest.GetTimeUsec();

            if (intervalDataLength != lastDataLength)
            {
                if (!useCft)
                {
                    // detect missing packets
                    if (message.seqNum != lastSeqNum[message.entityId])
                    {
                        // only track if skipped, might have restarted pub
                        if (message.seqNum > lastSeqNum[message.entityId])
                        {
                            missingPackets +=
                                    message.seqNum -
                                    lastSeqNum[message.entityId];
                        }
                    }
                }

                intervalTime = now - beginTime;
                intervalPacketsReceived = packetsReceived;
                intervalBytesReceived = bytesReceived;
                intervalMissingPackets = missingPackets;
                intervalDataLength = lastDataLength;
                missingPacketsPercent = 0.0;

                // Calculations of missing package percent
                if (intervalPacketsReceived
                        + intervalMissingPackets != 0)
                {
                    missingPacketsPercent =
                            intervalMissingPackets
                            / (double)(intervalPacketsReceived
                                + intervalMissingPackets);
                }

                double outputCpu = 0.0;
                if (showCpu)
                {
                    outputCpu = cpu.GetCpuAverage();
                }
                printer.PrintThroughputSummary(
                        intervalDataLength + (int)Perftest.OVERHEAD_BYTES,
                        intervalPacketsReceived,
                        intervalTime,
                        intervalBytesReceived,
                        intervalMissingPackets,
                        missingPacketsPercent,
                        outputCpu);
            }
            else if (endTest)
            {
                Console.WriteLine(
                    "\nNo samples have been received by the Subscriber side,\n"
                    + "however 1 or more Publishers sent the finalization message.\n\n"
                    + "There are several reasons why this could happen:\n"
                    + "- If you are using large data, make sure to correctly adjust your\n"
                    + "  sendQueue, reliability protocol and flowController.\n"
                    + "- Make sure your -executionTime or -numIter in the Publisher side\n"
                    + "  are big enough.\n"
                    + "- Try sending at a slower rate -pubRate in the Publisher side.\n\n");
            }

            packetsReceived = 0;
            bytesReceived = 0;
            missingPackets = 0;
            // length changed only used in scan mode in which case
            // there is only 1 publisher with ID 0
            lastSeqNum[0] = 0;
            beginTime = now;
        }
    }
} // PerformanceTest Namespace
