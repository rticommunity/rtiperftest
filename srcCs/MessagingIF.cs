/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using DDS;
using System;
using System.Collections.Generic;
using System.Text;

namespace PerformanceTest
{
    public class TestMessage
    {
        public byte[] data;
        public int    size;
        public byte[] key = new byte[4];
        public int    entity_id;
        public uint   seq_num;
        public int    timestamp_sec;
        public uint   timestamp_usec;
        public int    latency_ping;
    }

    public interface IMessagingCB
    {
        void ProcessMessage(TestMessage message);
    }

    public interface IMessagingReader
    {
        void WaitForWriters(int numWriters);

        // only used for non-callback test
        TestMessage ReceiveMessage();

        // only used for non-callback test to cleanup
        // the thread
        void Shutdown();
    }

    public interface IMessagingWriter
    {
        void WaitForReaders(int numReaders);
        bool Send(TestMessage message);
        void Flush();
        bool NotifyPingResponse();
        bool WaitForPingResponse();
        bool WaitForPingResponse(int timeout);
        long getPulledSampleCount();
    }

    public interface IMessaging : IDisposable
    {
        bool Initialize(int argc, string[] argv);

        void PrintCmdLineHelp();

        void Shutdown();

        // if the implementation supports batching and the test scenario is
        // using batching, this function should return the size of the batch
        // in bytes
        int GetBatchSize();

        IMessagingWriter CreateWriter(string topic_name);

        // Pass null for callback if using IMessagingReader.ReceiveMessage()
        // to get data
        IMessagingReader CreateReader(string topic_name, IMessagingCB callback);
    }


    public interface ITypeHelper<T> where T : class, DDS.ICopyable<T>
    {
        void fillKey(int value);

        void copyFromMessage(TestMessage message);

        TestMessage copyFromSeqToMessage(LoanableSequence<T> data_sequence,int index);

        T getData();

        void setBinDataMax(int newMax);

        void binDataUnloan();

        AbstractTypedTypeSupport<T> getTypeSupport();

        LoanableSequence<T> createSequence();

        ITypeHelper<T> clone();

        int getMAX_PERFTEST_SAMPLE_SIZE();

    }
}