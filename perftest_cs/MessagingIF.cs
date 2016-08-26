/* $Id: MessagingIF.cs,v 1.7 2014/09/22 16:28:50 jmorales Exp $

 (c) 2005-2012  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 Permission to modify and use for internal purposes granted.
 This software is provided "as is", without warranty, express or implied.

 Modification History
 --------------------
 5.1.0,22sep14,jm  PERFTEST-75 Fixed LargeData + Turbo-Mode. Changing max size to
                   131072.
 5.1.0,16sep14,jm  PERFTEST-60 PERFTEST-65 Large data support 
                   added for perftest.
 5.1.0,11aug14,jm  PERFTEST-68 Added -keyed command line option.
 1.0a,13jul10,jsr Added WaitForPingResponse with timeout
 1.0a,07jul10,jsr Fixed NotifyPingResponse and WaitForPingResponse to
                  return bool instead of void
 1.0a,14may10,jsr Added NotifyPingResponse and WaitForPingResponse
 1.0a,29may09,jsr Added detection of wrong command line parameter
 1.0a,04may08,hhw Added entity_id, WaitForReaders();.
 1.0a,02may08,hhw Added GetBatchSize().
 1.0a,19mar08,hhw Created.
===================================================================== */

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
