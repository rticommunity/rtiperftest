/*
 * (c) 2005-2016  Copyright, Real-Time Innovations, Inc.  All rights reserved.
 * Permission to modify and use for internal purposes granted.
 * This software is provided "as is", without warranty, express or implied.
 */

package com.rti.perftest;


// ===========================================================================

/**
 * Processes received messages.
 */
public interface IMessagingReader {
    /**
     * Only used for non-callback test.
     */
    public TestMessage receiveMessage();

    public void waitForWriters(int numWriters);

    /**
     * Only used for non-callback test to cleanup the thread.
     */
    public void shutdown();

}

// ===========================================================================
// End of $Id: IMessagingReader.java,v 1.2 2014/01/10 11:19:08 juanjo Exp $
