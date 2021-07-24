/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System.Linq;

namespace PerformanceTest
{
    public class DataTypeKeyedHelper : ITypeHelper<TestDataKeyed_t>
    {
        protected TestDataKeyed_t dataSample;
        protected TestMessage message = new TestMessage();
        protected ulong maxPerftestSampleSize = Perftest.GetMaxPerftestSampleSize();

        public DataTypeKeyedHelper(ulong maxPerftestSampleSize)
        {
            dataSample = new TestDataKeyed_t();
            this.maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DataTypeKeyedHelper(TestDataKeyed_t dataSample, ulong maxPerftestSampleSize)
        {
            this.dataSample = dataSample;
            this.maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public TestDataKeyed_t MessageToSample(TestMessage message, int keyValue)
        {
            dataSample.key[0] = (byte)(keyValue);
            dataSample.key[1] = (byte)(keyValue >> 8);
            dataSample.key[2] = (byte)(keyValue >> 16);
            dataSample.key[3] = (byte)(keyValue >> 24);
            dataSample.entity_id = message.entityId;
            dataSample.seq_num = message.seqNum;
            dataSample.timestamp_sec = message.timestampSec;
            dataSample.timestamp_usec = message.timestampUsec;
            dataSample.latency_ping = message.latencyPing;

            DataHelper.PopulateData(dataSample.bin_data, message.data);

            return dataSample;
        }

        public TestMessage SampleToMessage(TestDataKeyed_t data_sample)
        {
            message.key = data_sample.key;
            message.entityId = data_sample.entity_id;
            message.seqNum = data_sample.seq_num;
            message.timestampSec = data_sample.timestamp_sec;
            message.timestampUsec = data_sample.timestamp_usec;
            message.latencyPing = data_sample.latency_ping;
            message.Size = data_sample.bin_data.Count;

            return message;
        }

        public ITypeHelper<TestDataKeyed_t> Clone()
        {
            return new DataTypeKeyedHelper(dataSample, maxPerftestSampleSize);
        }

        public long GetSerializedOverheadSize()
        {
            using (var serializer = TestDataKeyed_tSupport.Instance.CreateSerializer())
            {
                return serializer.GetSerializedSampleSize(dataSample) - Perftest.CDR_ENCAPSULATION_HEADER_SIZE;
            }
        }
    }
}