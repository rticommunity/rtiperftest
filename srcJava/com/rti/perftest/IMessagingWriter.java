/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest;

import java.util.concurrent.TimeUnit;
import com.rti.dds.infrastructure.Duration_t;

// ===========================================================================

/**
 * Send messages.
 */
public interface IMessagingWriter {
    public void waitForReaders(int numReaders);

    public boolean send(TestMessage message, boolean isCftWildCardKey);
    public boolean waitForPingResponse();
    public boolean waitForPingResponse(long timeout, TimeUnit unit);
    public boolean notifyPingResponse();
    public long getPulledSampleCount();
    public void wait_for_acknowledgments(Duration_t timeout);
    public void flush();

}

// ===========================================================================

