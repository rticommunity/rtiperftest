/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

namespace PerformanceTest
{
    public class TestMessage
    {
        public Omg.Types.ISequence<byte> data = new Rti.Types.Sequence<byte>();
        public int Size
        {
            get => data.Count;
            set
            {
                if (value == data.Count)
                {
                    return;
                }
                else if (value < data.Count)
                {
                    data.RemoveRange(value, data.Count - value);
                }
                else
                {
                    for (int i = data.Count; i < value; i++)
                    {
                        data.Add(0);
                    }
                }
            }
        }
        public byte[] key = new byte[4];
        public int entityId;
        public uint seqNum;
        public int timestampSec;
        public uint timestampUsec;
        public int latencyPing;

        public override string ToString()
        {
            return "data[" + data.Count + "]"
                + "\nkey: " + key
                + "\nentityId: " + entityId
                + "\nseqNum: " + seqNum
                + "\ntimestampSec: " + timestampSec
                + "\ntimestampUsec: " + timestampUsec
                + "\nlatencyPing: " + latencyPing + "]\n";
        }
    }
}
