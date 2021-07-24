/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Linq;
using System.Threading;
using Omg.Dds.Core;
using Rti.Dds.Publication;
using Rti.Dds.Core.Policy;

namespace PerformanceTest
{
    public class RTIWriter<T> : IMessagingWriter where T : class, IEquatable<T>
    {
        private readonly DataWriter<T> writer;
        protected ITypeHelper<T> dataType;
        protected int numInstances;
        protected long instanceCounter;
        protected Semaphore pongSemaphore;
        protected int instancesToBeWritten = -1;
        protected bool reliable = false;

        public RTIWriter(
                DataWriter<T> writer,
                int numInstances,
                Semaphore pongSemaphore,
                ITypeHelper<T> dataType,
                int instancesToBeWritten)
        {
            this.writer = writer;
            this.dataType = dataType;
            this.numInstances = numInstances;
            this.pongSemaphore = pongSemaphore;
            this.instancesToBeWritten = instancesToBeWritten;
            reliable = this.writer.Qos.Reliability.Kind == ReliabilityKind.Reliable;
        }

        public void Flush()
        {
            writer.Flush();
        }

        public virtual bool Send(TestMessage message, bool isCftWildCardKey)
        {
            int key = 0;
            if (!isCftWildCardKey)
            {
                if (numInstances > 1)
                {
                    if (instancesToBeWritten == -1)
                    {
                        key = (int)(instanceCounter++ % numInstances);
                    }
                    else
                    {
                        key = instancesToBeWritten;
                    }
                }
            }
            else
            {
                key = MAX_CFT_VALUE.Value;
            }

            try
            {
                writer.Write(dataType.MessageToSample(message, key));
            }
            catch (Exception ex)
            {
                Console.Error.Write("Write error {0}\n", ex);
                return false;
            }

            return true;
        }

        public void WaitForReaders(int numSubscribers)
        {
            while (writer.MatchedSubscriptions.Count() < numSubscribers)
            {
                Thread.Sleep(100);
            }
        }

        public bool NotifyPingResponse()
        {
            if (pongSemaphore != null)
            {
                try
                {
                    pongSemaphore.Release();
                }
                catch (System.Exception ex)
                {
                    Console.Error.WriteLine("Exception: " + ex.Message);
                    return false;
                }
            }
            return true;
        }

        public bool WaitForPingResponse()
        {
            if (pongSemaphore != null)
            {
                try
                {
                    pongSemaphore.WaitOne();
                }
                catch (System.Exception ex)
                {
                    Console.Error.WriteLine("Exception: " + ex.Message);
                    return false;
                }
            }
            return true;
        }

        public bool WaitForPingResponse(TimeSpan timeout)
        {
            if (pongSemaphore != null)
            {
                try
                {
                    pongSemaphore.WaitOne(timeout, false);
                }
                catch (System.Exception ex)
                {
                    Console.Error.WriteLine("Exception: " + ex.Message);
                    return false;
                }
            }
            return true;
        }

        public long GetPulledSampleCount()
        {
            return writer.DataWriterProtocolStatus.PulledSampleCount.Value;
        }

        public void WaitForAck(TimeSpan timeSpan)
        {
            if (reliable)
            {
                writer.WaitForAcknowledgments((Duration)timeSpan);
            }
            else
            {
                Thread.Sleep(timeSpan);
            }
        }
    }
}
