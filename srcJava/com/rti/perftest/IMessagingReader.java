/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
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

