/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System.Linq;

namespace PerformanceTest
{
    public class DataTypeKeyedLargeHelper : ITypeHelper<TestDataKeyedLarge_t>
    {
        protected TestDataKeyedLarge_t dataSample;
        protected TestMessage message = new TestMessage();
        protected ulong maxPerftestSampleSize = Perftest.GetMaxPerftestSampleSize();

        public DataTypeKeyedLargeHelper(ulong maxPerftestSampleSize)
        {
            dataSample = new TestDataKeyedLarge_t();
            this.maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public DataTypeKeyedLargeHelper(TestDataKeyedLarge_t dataSample, ulong maxPerftestSampleSize)
        {
            this.dataSample = dataSample;
            this.maxPerftestSampleSize = maxPerftestSampleSize;
        }

        public TestDataKeyedLarge_t MessageToSample(TestMessage message, int keyValue)
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

        public TestMessage SampleToMessage(TestDataKeyedLarge_t data_sample)
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

        public ITypeHelper<TestDataKeyedLarge_t> Clone()
        {
            return new DataTypeKeyedLargeHelper(dataSample, maxPerftestSampleSize);
        }

        public long GetSerializedOverheadSize()
        {
            using (var serializer = TestDataKeyedLarge_tSupport.Instance.CreateSerializer())
            {
                return serializer.GetSerializedSampleSize(dataSample) - Perftest.CDR_ENCAPSULATION_HEADER_SIZE;
            }
        }
    }
}