/*
 * (c) 2005-2017  Copyright, Real-Time Innovations, Inc. All rights reserved.
 * Subject to Eclipse Public License v1.0; see LICENSE.md for details.
 */

package com.rti.perftest;


// ===========================================================================

/**
 * Runs the performance test.
 */
public interface IMessaging {
    public boolean initialize(int argc, String[] argv);

    public void printCmdLineHelp();

    String printConfiguration();

    public void shutdown();

    /**
     * If the implementation supports batching and the test scenario is
     * using batching, this function should return the size of the batch
     * in bytes.
     */
    public int getBatchSize();

    /**
     * Get an estimation of the minimum number of samples that need to be send
     * before starting the test to ensure that most memory allocations will be
     * done in the subscriber side (when sending a burst of that data).
     */
    public int getInitializationSampleCount();

    public IMessagingWriter createWriter(String topicName);

    /**
     * Pass null for callback if using IMessagingReader.ReceiveMessage()
     * to get data
     */
    public IMessagingReader createReader(
            String topicName, IMessagingCB callback);

    public void dispose();

}

// ===========================================================================
