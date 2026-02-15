/*
 * (c) 2005-2021 Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

using System;

namespace PerformanceTest
{
    public interface IMessagingWriter
    {
        void WaitForReaders(int numReaders);
        bool Send(TestMessage message, bool isCftWildCardKey);
        void Flush();
        bool NotifyPingResponse();
        bool WaitForPingResponse();
        bool WaitForPingResponse(TimeSpan timeout);
        long GetPulledSampleCount();
        void WaitForAck(TimeSpan timeSpan);
    }
}
