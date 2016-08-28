/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

package com.rti.perftest;

import java.util.concurrent.TimeUnit;


// ===========================================================================

/**
 * Send messages.
 */
public interface IMessagingWriter {
    public void waitForReaders(int numReaders);

    public boolean send(TestMessage message);
    public boolean waitForPingResponse();
    public boolean waitForPingResponse(long timeout, TimeUnit unit);
    public boolean notifyPingResponse();

    public void flush();

}

// ===========================================================================
// End of $Id$
