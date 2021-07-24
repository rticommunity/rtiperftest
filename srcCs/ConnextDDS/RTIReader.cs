/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;
using System.Linq;
using Rti.Dds.Core;
using Rti.Dds.Core.Status;
using Rti.Dds.Subscription;
using Omg.Dds.Core;

namespace PerformanceTest
{
    public class RTIReader<T> : IMessagingReader where T : class, IEquatable<T>
    {
        private readonly ITypeHelper<T> dataTypeHelper;
        private readonly DataReader<T> reader;
        private readonly WaitSet waitset;

        public RTIReader(
            DataReader<T> reader,
            ITypeHelper<T> dataType,
            Parameters arguments)
        {
            this.reader = reader;
            dataTypeHelper = dataType;
            if (arguments.UseReadThread)
            {
                WaitSetProperty property = new WaitSetProperty(
                        (int) arguments.WaitsetEventCount,
                        Duration.FromMilliseconds(arguments.WaitsetDelayUsec / 1000));

                waitset = new WaitSet(property);
                StatusCondition readerStatus = reader.StatusCondition;
                readerStatus.EnabledStatuses = StatusMask.DataAvailable;
                waitset.AttachCondition(readerStatus);
            }
        }

        public void ReceiveMessage(IMessagingCallback callback)
        {
            waitset.Wait();
            using var samples = reader.Take();

            foreach (var sample in samples)
            {
                if (sample.Info.ValidData)
                {
                    callback.ProcessMessage(dataTypeHelper.SampleToMessage(sample.Data));
                }
            }
        }

        public void WaitForWriters(int numPublishers)
        {
            while (reader.MatchedPublications.Count() < numPublishers)
            {
                System.Threading.Thread.Sleep(100);
            }
        }
    } // RTISubscriber
} // namespace PerformanceTest
