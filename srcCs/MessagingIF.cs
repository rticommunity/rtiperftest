/*
 * Copyright 2016
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
        public const int MAX_SYNCHRONOUS_SIZE = 63000;
        public const int MAX_DATA_SIZE = 131072;
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

        // Used only for scan mode.
        // The maximum size of a message's binary payload. If the size
        // exceeds this during a scan, the test will stop.
        int GetMaxBinDataSize();

        IMessagingWriter CreateWriter(string topic_name);

        // Pass null for callback if using IMessagingReader.ReceiveMessage()
        // to get data
        IMessagingReader CreateReader(string topic_name, IMessagingCB callback);
    }


    public interface ITypeHelper<T> where T : class, DDS.ICopyable<T>
    {
        void fillKey(int value);

        void copyFromMessage(TestMessage message);

        TestMessage copyFromSeqToMessage(Object data_sequence,int index);

        T getData();

        DDS.ByteSeq getBindata();

        DDS.TypedTypeSupport<T> getTypeSupport();

        DDS.LoanableSequence<T> createSequence();

        ITypeHelper<T> clone();

    }
}
