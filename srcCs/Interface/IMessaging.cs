/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;

namespace PerformanceTest
{
    public interface IMessaging : IDisposable
    {
        bool Initialize(Parameters arguments);

        string PrintConfiguration();

        void Shutdown();

        /*
         * If the implementation supports batching and the test scenario is
         * using batching, this function should return the size of the batch
         * in bytes.
         */
        int BatchSize { get; set; }

        /*
         * Get an estimation of the minimum number of samples that need to be send
         * before starting the test to ensure that most memory allocations will be
         * done in the subscriber side (when sending a burst of that data).
         */
        int InitialBurstSampleCount { get; set; }

        IMessagingWriter CreateWriter(string topicName);

        /*
         * Pass null for callback if using IMessagingReader.ReceiveMessage()
         * to get data
         */
        IMessagingReader CreateReader(string topicName, IMessagingCallback callback);
    }
}
